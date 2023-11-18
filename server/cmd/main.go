package main

import (
	"log"
	"server/internal/config"
	"server/internal/db"
	"time"

	"github.com/matoous/go-nanoid/v2"
	// "server/internal/validator"
)

type user struct {
	ID        string    `dynamo:"id,hash" index:"Seq-ID-index"`
	CreatedAt time.Time `dynamo:"created_at"`
	Name      string    `dynamo:"name"`
	Email     string    `dynamo:"email"`
}

func main() {
	// validator := validator.New()

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
	user := user{
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
}
