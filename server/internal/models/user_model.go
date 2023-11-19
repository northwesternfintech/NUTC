package models

import "time"

type User struct {
	ID        string    `dynamo:"id" json:"id"`
	Name      string    `dynamo:"name" json:"name"`
	Email     string    `dynamo:"email" json:"email"`
	CreatedAt time.Time `dynamo:"created_at" json:"created_at"`
}
