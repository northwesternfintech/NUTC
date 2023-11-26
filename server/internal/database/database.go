package database

import (
	"server/internal/config"

	"github.com/aws/aws-sdk-go/aws"
	"github.com/aws/aws-sdk-go/aws/credentials"
	"github.com/aws/aws-sdk-go/aws/session"
	"github.com/guregu/dynamo"
)

func New(cfg config.DatabaseConfig) (*dynamo.DB, error) {
	sess, err := session.NewSession(&aws.Config{
		Region:      aws.String(cfg.Region), // Use region from config
		Credentials: credentials.NewStaticCredentials(cfg.AccessKey, cfg.SecretKey, ""), // Use credentials from config
	})
	if err != nil {
		return nil, err // Handle potential errors during session creation
	}

	db := dynamo.New(sess)
	return db, nil
}
