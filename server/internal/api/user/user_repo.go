package user

import (
	"errors"
	"fmt"
	"server/internal/models"

	"github.com/guregu/dynamo"
)

type Repository interface {
	CreateUser(user models.User) error
	GetUserByID(userID string) (models.User, error)
}

type repository struct {
	db *dynamo.DB
}

const (
	userTableName = "User"
)

var (
	ErrUserAlreadyExists = errors.New("repository: user already exists")
	ErrUserNotFound      = errors.New("repository: user not found")
)

func NewRepository(db *dynamo.DB) Repository {
	return &repository{
		db: db,
	}
}

func (r *repository) CreateUser(user models.User) error {
	err := r.db.Table(userTableName).Put(user).Run()
	if err != nil {
		return fmt.Errorf("repository: error creating user: %v", err)
	}
	return nil
}

func (r *repository) GetUserByID(userID string) (models.User, error) {
	var user models.User
	err := r.db.Table(userTableName).Get("id", userID).One(&user)
	if err != nil {
		if err == dynamo.ErrNotFound {
			return models.User{}, ErrUserNotFound
		}
		return models.User{}, fmt.Errorf("repository: error getting user: %v", err)
	}
	return user, nil
}
