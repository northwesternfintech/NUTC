package user

import (
	"log"
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

func NewRepository(db *dynamo.DB) Repository {
	return &repository{
		db: db,
	}
}

func (r *repository) CreateUser(user models.User) error {
	return r.db.Table(userTableName).Put(user).Run()
}

func (r *repository) GetUserByID(userID string) (models.User, error) {
    var user models.User
    err := r.db.Table(userTableName).Get("id", userID).One(&user)
    if err != nil {
        if err == dynamo.ErrNotFound {
            log.Printf("User not found: %v", err)
            return models.User{}, dynamo.ErrNotFound
        }
        log.Printf("Error getting user in repo: %v", err)
        return models.User{}, err
    }
    return user, nil
}