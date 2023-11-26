package db_queries

import (
	"errors"
	"fmt"
	"server/internal/database/schema"

	"github.com/guregu/dynamo"
)

type UserRepository interface {
	CreateUser(user schema.User) error
	GetUserByID(userID string) (schema.User, error)
}

type userRepository struct {
	db *dynamo.DB
}

const (
	userTableName = "User"
)

var (
	ErrUserAlreadyExists = errors.New("repository: user already exists")
	ErrUserNotFound      = errors.New("repository: user not found")
)

func NewRepository(db *dynamo.DB) UserRepository {
	return &userRepository{
		db: db,
	}
}

func (r *userRepository) CreateUser(user schema.User) error {
	err := r.db.Table(userTableName).Put(user).Run()
	if err != nil {
		return fmt.Errorf("repository: error creating user: %v", err)
	}
	return nil
}

func (r *userRepository) GetUserByID(userID string) (schema.User, error) {
	var user schema.User
	err := r.db.Table(userTableName).Get("id", userID).One(&user)
	if err != nil {
		if err == dynamo.ErrNotFound {
			return schema.User{}, ErrUserNotFound
		}
		return schema.User{}, fmt.Errorf("repository: error getting user: %v", err)
	}
	return user, nil
}
