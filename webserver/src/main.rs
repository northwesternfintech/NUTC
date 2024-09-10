use actix_cors::Cors;
use actix_web::{http::header, get, post, web, App, HttpResponse, HttpServer, Responder};
use serde::Deserialize;
use serde_json::json;
use reqwest::Client;
use tokio_postgres::{NoTls};

const LINTER_BASE_URL: &str = "http://linter:18081";
const SANDBOX_BASE_URL: &str = "http://sandbox:18080";

// #[repr(i32)]
// #[derive(Deserialize, Debug, PartialEq)]
// pub enum LintingResultOption {
//     Unknown = -1,
//     Failure = 0,
//     Success = 1,
//     Pending = 2,
// }

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

async fn get_algorithms(case: String, uid: Option<String>) -> impl Responder {
    let (postgres_client, connection) = match tokio_postgres::connect("host=localhost user=postgres password=yourpassword dbname=nutc", NoTls).await {
        Ok((client, connection)) => (client, connection),
        Err(e) => {
            eprintln!("Failed to connect to the database: {}", e);
            return HttpResponse::InternalServerError().finish();
        }
    };

    actix_web::rt::spawn(async move {
        if let Err(e) = connection.await {
            eprintln!("connection error: {}", e);
        }
    });

    let query = r#"
    WITH "MostRecentAlgos" AS (
      WITH "FilteredAlgosByCase" AS (
        SELECT a."uid", f."key", f."createdAt" AS "timestamp"
        FROM "algos" AS a
        INNER JOIN "AlgoFile" AS f ON a."algoFileKey" = f."key"
        WHERE a."case" = $1
        AND ($2::text IS NULL OR a."uid" = $2)
      )
      SELECT "uid", MAX("timestamp") AS "timestamp"
      FROM "FilteredAlgosByCase"
      GROUP BY "uid"
    )
    SELECT 
      p."firstName" || ' ' || p."lastName" AS "name", 
      a."uid", 
      a."key"
    FROM "profiles" AS p
    INNER JOIN (
      SELECT u."uid", f."key"
      FROM "MostRecentAlgos" AS u
      INNER JOIN "AlgoFile" AS f ON u."timestamp" = f."createdAt"
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
                .map(|i| row.get::<usize, Option<String>>(i).unwrap_or_else(|| "NULL".to_string()))
                .collect();
            json!(values)
        })
        .collect();

    HttpResponse::Ok().json(response_data)
}

#[tracing::instrument]
#[get("/algorithms/{case}/{uid}")]
async fn get_single_user_algorithm(data: web::Path<(String, String)>) -> impl Responder {
    let (case, uid) = data.into_inner();
    get_algorithms(case, Some(uid)).await
}

#[tracing::instrument]
#[get("/algorithms/{case}")]
async fn get_all_user_algorithms(data: web::Path<String>) -> impl Responder {
    let case = data.into_inner();
    get_algorithms(case, None).await
}

#[actix_web::main]
async fn main() -> std::io::Result<()> {
    tracing_subscriber::fmt()
        .event_format(
            tracing_subscriber::fmt::format()
                .with_file(true)
                .with_line_number(true)
                .with_level(true),
        )
        .init();

    tracing::info!("Starting server.");

    HttpServer::new(|| {
        let cors = Cors::default()
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
            .service(linter)
            .service(get_single_user_algorithm)
            .service(get_all_user_algorithms)
            .wrap(cors)
    })
    .bind(("0.0.0.0", 16124))?
    .run()
    .await
}
