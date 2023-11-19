package models

import "time"

type User struct {
	ID        string    `dynamo:"id,hash" index:"Seq-ID-index"`
	CreatedAt time.Time `dynamo:"created_at"`
	Name      string    `dynamo:"name"`
	Email     string    `dynamo:"email"`
}