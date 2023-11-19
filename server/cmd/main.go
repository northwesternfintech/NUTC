package main

import (
	"log"
	"net/http"
	"server/internal/config"
	"server/internal/db"
	"server/internal/models"
	"time"

	"server/internal/validator"

	"github.com/go-chi/chi/v5"
	"github.com/matoous/go-nanoid/v2"
)


func main() {
	validator := validator.New()

	cfg, err := config.Load(".env")
	if err != nil {
		log.Fatalf("Could not load config: %v", err)
	}

	db, err := db.New(cfg.DB)
	if err != nil {
		log.Fatalf("Error connecting to database: %v", err)
	}

	table := db.Table("User")

	userID, err := gonanoid.New()
	if err != nil {
		log.Fatalf("Error generating ID: %v", err)
	}
	user := models.User{
		ID:        userID,
		CreatedAt: time.Now(),
		Name:      "John Doe",
		Email:     "john@doe",
	}

	err = table.Put(user).Run()
	if err != nil {
		log.Fatalf("Error putting user: %v", err)
	} else {
		log.Println("User added successfully")
	}

	mux := chi.NewRouter()
	r, handlers := setupHandler(mux, validator, cfg)
	server := http.Server{
		Addr:    cfg.ServerPort,
		Handler: r,
	}

}

func setupHandler(
	r chi.Router,
	v validator.Validate,
	cfg *config.Config,
) {

}