use actix_cors::Cors;
use actix_web::{get, http::header, post, web, App, HttpResponse, HttpServer, Responder};
use deadpool_postgres::{Manager, ManagerConfig, Pool, RecyclingMethod};
use reqwest::Client;
use serde::Deserialize;
use serde_json::json;
use tokio_postgres::NoTls;

const LINTER_BASE_URL: &str = "http://linter:18081";
const SANDBOX_BASE_URL: &str = "http://sandbox:18080";

#[derive(Deserialize, Debug)]
pub struct LinterResponse {
    pub linting_status: i32,
}

#[tracing::instrument]
#[post("/submit/{uid}/{algo_id}")]
async fn linter(data: web::Path<(String, String)>) -> impl Responder {
    let (uid, algo_id) = data.into_inner();
    let client = Client::new();

    let linter_url = format!("{}/?uid={}&algo_id={}", LINTER_BASE_URL, uid, algo_id);

    tracing::info!("sending request to linter url {}", linter_url);

    let linter_response = client.get(&linter_url).send().await;

    match linter_response {
        Ok(response) => {
            if response.status().is_success() {
                match response.json::<LinterResponse>().await {
                    Ok(linting_response) => {
                        // only call sandbox if linting is successful
                        if linting_response.linting_status != 1 {
                            tracing::info!("linting failed");
                            return HttpResponse::BadRequest().finish();
                        }

                        tracing::info!("linting success, now requesting sandbox");
                        let sandbox_url =
                            format!("{}/sandbox/{}/{}", SANDBOX_BASE_URL, uid, algo_id);
                        tracing::info!("linting success now requesting sandbox {}", sandbox_url);

                        let sandbox_response = client.get(&sandbox_url).send().await;

                        match sandbox_response {
                            Ok(response) => {
                                let body = response.text().await;
                                tracing::info!(
                                    "sandbox response: {}",
                                    body.unwrap_or("failed to decode sandbox response".into())
                                );
                                HttpResponse::Ok().finish()
                            }
                            Err(err) => {
                                tracing::error!("failed to request sandbox {:#?}", err);
                                tracing::error!("someting went wrong requesting to sandbox");
                                HttpResponse::BadGateway().finish()
                            }
                        }
                    }
                    Err(err) => {
                        tracing::error!("failed to decode linter response {:#?}", err);
                        return HttpResponse::BadRequest().finish();
                    }
                }
            } else {
                tracing::error!("linter response status isn't 200");
                HttpResponse::BadGateway().finish()
            }
        }
        Err(err) => {
            tracing::error!("someting went wrong requesting to linter {:#?}", err);
            HttpResponse::BadGateway().finish()
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

    let host =
        std::env::var("WEBSERVER_DB_HOST").expect("env variable `WEBSERVER_DB_HOST` should be set");
    let user =
        std::env::var("WEBSERVER_DB_USER").expect("env variable `WEBSERVER_DB_USER` should be set");
    let password = std::env::var("WEBSERVER_DB_PASSWORD")
        .expect("env variable `WEBSERVER_DB_PASSWORD` should be set");
    let dbname =
        std::env::var("WEBSERVER_DB_NAME").expect("env variable `WEBSERVER_DB_NAME` should be set");

    let mut pg_config = tokio_postgres::Config::new();
    pg_config.host(host);
    pg_config.user(user);
    pg_config.password(password);
    pg_config.dbname(dbname);
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
            .service(linter)
            .service(get_single_user_algorithm)
            .service(get_all_user_algorithms)
            .wrap(cors)
    })
    .bind(("0.0.0.0", 16124))?
    .run()
    .await
}
