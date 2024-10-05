use actix_cors::Cors;
use actix_web::{get, http::header, post, web, App, HttpResponse, HttpServer, Responder};
use deadpool_postgres::{Manager, ManagerConfig, Pool, RecyclingMethod};
use reqwest::Client;
use serde::Deserialize;
use serde_json::json;
use tokio_postgres::NoTls;

const LINTER_BASE_URL: &str = "http://linter:18081";
const SANDBOX_BASE_URL: &str = "http://sandbox:18080";
// const LINTER_BASE_URL: &str = "http://localhost:18081";

#[derive(Deserialize, Debug)]
pub struct LinterResponse {
    pub linting_status: i32,
}

#[derive(Deserialize, Debug)]
struct LintResult {
    success: bool,
    message: String,
}

async fn set_linter_status(
    algo_id: &String,
    success: bool,
    message: &String,
    pool: &Pool,
) -> Result<(), Box<dyn std::error::Error>> {
    let postgres_cient = pool.get().await?;

    let query = if success {
        r#"
        UPDATE "algos"
        SET "lintResults" = $1, "lintSuccessMessage" = $2
        WHERE "algoFileS3Key" = $3
    "#
    } else {
        r#"
        UPDATE "algos"
        SET "lintResults" = $1, "lintFailureMessage" = $2
        WHERE "algoFileS3Key" = $3
    "#
    };

    let success_message = if success { "success" } else { "failure" };

    postgres_cient
        .execute(query, &[&success_message, &message, &algo_id])
        .await?;

    Ok(())
}

#[tracing::instrument]
#[post("/submit/{algo_id}/{language}")]
async fn handle_algo_submission(
    data: web::Path<(String, String)>,
    db_pool: web::Data<Pool>,
) -> impl Responder {
    dotenv::dotenv().ok();

    let (algo_id, language) = data.into_inner();

    let s3_endpoint =
        std::env::var("S3_ENDPOINT").expect("env variable `S3_ENDPOINT` should be set");
    let algo_url = format!("{}/nutc/{}", s3_endpoint, algo_id);

    let linter_url = format!(
        "{}/?algo_url={}&language={}",
        LINTER_BASE_URL, algo_url, language
    );

    tracing::info!("sending request to linter url {}", linter_url);

    let client = Client::new();
    let linter_response = client.get(&linter_url).send().await;

    if linter_response.is_err() {
        tracing::error!("failed to request linter");
        return HttpResponse::BadGateway().finish();
    }

    let linter_response = linter_response.unwrap();

    let linter_response = if let Ok(linter_response) = linter_response.json::<LintResult>().await {
        linter_response
    } else {
        tracing::error!("failed to decode linter response");
        return HttpResponse::BadGateway().finish();
    };

    if let Err(e) = set_linter_status(&algo_id, true, &linter_response.message, &db_pool).await {
        tracing::error!("failed to update linter status in database, {:?}", e);
        return HttpResponse::BadGateway().finish();
    }
    if linter_response.success {
        tracing::info!("Linting succeeded, now requesting sandbox");
    } else {
        // forward the error message from the linter, and add something in the response
        // to indicate that the linter Failed
        tracing::error!("linting failed: {}", linter_response.message);
        return HttpResponse::Ok()
            .json(json!({"success": false, "message": linter_response.message}));
    }

    // todo: this function shouldnt return a http request
    return request_sandbox(algo_id, algo_url, language).await;
}

async fn request_sandbox(algo_id: String, algo_url: String, language: String) -> HttpResponse {
    let client = Client::new();
    let sandbox_url = format!("{}/sandbox/{}/{}", SANDBOX_BASE_URL, algo_id, language);
    tracing::info!("linting success now requesting sandbox {}", sandbox_url);

    // download the algo file
    let algo_data = client
        .get(algo_url)
        .send()
        .await
        .unwrap()
        .bytes()
        .await
        .unwrap();

    let sandbox_response = client
        .post(&sandbox_url)
        .header("Content-Type", "application/json")
        .body(algo_data)
        .send()
        .await;

    match sandbox_response {
        Ok(response) => {
            let text_body = response.text().await;
            let text = text_body.unwrap();
            tracing::info!("sandbox response: {}", text);
            return HttpResponse::Ok()
                .content_type("application/json")
                .body(text);
        }
        Err(err) => {
            tracing::error!("failed to request sandbox {:#?}", err);
            tracing::error!("someting went wrong requesting to sandbox");
            return HttpResponse::BadGateway()
                .content_type("application/json")
                .body("{\"success\": false}");
        }
    }
}

async fn get_algorithms(case: String, uid: Option<String>, pool: &Pool) -> impl Responder {
    let postgres_client = match pool.get().await {
        Ok(client) => client,
        Err(e) => {
            eprintln!("Failed to connect to the database: {}", e);
            return HttpResponse::InternalServerError().finish();
        }
    };

    let query = r#"
    WITH "MostRecentAlgos" AS (
      WITH "FilteredAlgosByCase" AS (
        SELECT a."uid", f."s3Key", f."createdAt" AS "timestamp"
        FROM "algos" AS a
        INNER JOIN "algo_file" AS f ON a."algoFileS3Key" = f."s3Key"
        WHERE a."case" = $1
        AND ($2::text IS NULL OR a."uid" = $2)
      )
      SELECT "uid", MAX("timestamp") AS "timestamp"
      FROM "FilteredAlgosByCase"
      GROUP BY "uid"
    )
    SELECT 
      p."firstName" || ' ' || p."lastName" AS "name", 
      a."s3Key" 
    FROM "profiles" AS p
    INNER JOIN (
      SELECT u."uid", f."s3Key"
      FROM "MostRecentAlgos" AS u
      INNER JOIN "algo_file" AS f ON u."timestamp" = f."createdAt"
    ) AS a ON p."uid" = a."uid";
    "#;

    let params: Vec<&(dyn tokio_postgres::types::ToSql + Sync)> = vec![&case, &uid];

    let rows = match postgres_client.query(query, &params).await {
        Ok(rows) => rows,
        Err(e) => {
            eprintln!("query failed: {}", e);
            return HttpResponse::InternalServerError().finish();
        }
    };

    let response_data: Vec<serde_json::Value> = rows
        .into_iter()
        .map(|row| {
            let values: Vec<String> = (0..row.len())
                .map(|i| {
                    row.get::<usize, Option<String>>(i)
                        .unwrap_or_else(|| "NULL".to_string())
                })
                .collect();
            json!(values)
        })
        .collect();

    HttpResponse::Ok().json(response_data)
}

#[tracing::instrument]
#[get("/algorithms/{case}/{uid}")]
async fn get_single_user_algorithm(
    data: web::Path<(String, String)>,
    db_pool: web::Data<Pool>,
) -> impl Responder {
    let (case, uid) = data.into_inner();
    get_algorithms(case, Some(uid), &db_pool).await
}

#[tracing::instrument]
#[get("/algorithms/{case}")]
async fn get_all_user_algorithms(
    data: web::Path<String>,
    db_pool: web::Data<Pool>,
) -> impl Responder {
    let case = data.into_inner();
    get_algorithms(case, None, &db_pool).await
}

#[actix_web::main]
async fn main() -> std::io::Result<()> {
    dotenv::dotenv().ok();

    let host = std::env::var("PRISMA_DATABASE_URL")
        .expect("env variable `WEBSERVER_DB_HOST` should be set");
    let host_url = match host.find('?') {
        Some(idx) => &host[..idx],
        None => &host[..],
    };

    let pg_config = host_url
        .parse::<tokio_postgres::Config>()
        .expect("unable to parse DB url");
    let mgr_config = ManagerConfig {
        recycling_method: RecyclingMethod::Fast,
    };

    let pool = Pool::builder(Manager::from_config(pg_config, NoTls, mgr_config))
        .max_size(16)
        .build()
        .expect("Failed to connect to DB");

    tracing_subscriber::fmt()
        .event_format(
            tracing_subscriber::fmt::format()
                .with_file(true)
                .with_line_number(true)
                .with_level(true),
        )
        .init();

    tracing::info!("Starting server.");

    HttpServer::new(move || {
        let cors = Cors::default()
            .allowed_origin("http://localhost:3000")
            .allowed_origin("http://localhost:3001")
            .allowed_origin("http://desktop.tail78d9b.ts.net:3000")
            .allowed_origin("https://nutc.io")
            .allowed_origin("https://www.nutc.io")
            .allowed_methods(vec!["GET", "POST", "DELETE"])
            .allowed_headers(vec![
                header::CONTENT_TYPE,
                header::AUTHORIZATION,
                header::ACCEPT,
                header::UPGRADE,
                header::CONNECTION,
            ])
            .supports_credentials();
        App::new()
            .app_data(web::Data::new(pool.clone()))
            .service(handle_algo_submission)
            .service(get_single_user_algorithm)
            .service(get_all_user_algorithms)
            .wrap(cors)
    })
    .bind(("0.0.0.0", 16124))?
    .run()
    .await
}
