package user

import (
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
	userTableName        = "User"
	ErrUserAlreadyExists = "user already exists"
)

func (r *repository) CreateUser(user models.User) error {
	return r.db.Table(userTableName).Put(user).Run()
}

func (r *repository) GetUserByID(userID string) (models.User, error) {
	var user models.User
	err := r.db.Table(userTableName).Get("ID", userID).One(&models.User{})
	if err != nil {
		return models.User{}, err
	}
	return user, nil
}
