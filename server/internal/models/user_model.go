package models

import "time"

type User struct {
	ID        string    `dynamo:"id"`
	Name      string    `dynamo:"name"`
	Email     string    `dynamo:"email"`
	CreatedAt time.Time `dynamo:"created_at"`
}
