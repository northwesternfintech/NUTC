provider "google" {
  credentials = file("gcp-sa-key.json")
  project     = "nutc-web"
}

resource "google_cloud_run_service" "default" {
  name     = "nutc-linter"
  location = "us-central1"

  template {
    spec {
      containers {
        image = "gcr.io/nutc-web/nutc-linter:latest"
      }
    }
  }

  traffic {
    percent         = 100
    latest_revision = true
  }
}
