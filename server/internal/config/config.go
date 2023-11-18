package config

import (
	"fmt"
	"os"
	"strconv"

	"github.com/joho/godotenv"
	"server/internal/validator"
)

const (
	keyDBRegion    = "DB_REGION"
	keyDBAccessKey = "DB_ACCESS_KEY"
	keyDBSecretKey = "DB_SECRET_KEY"

	keyEnv           = "ENV"
	keyServerPort    = "SERVER_PORT"
	keyJWTSecret     = "JWT_SIGNING_KEY"
	keyJWTExpiration = "JWT_EXPIRATION"

	ProdEnv = "production"
	DevEnv  = "development"
)

type DatabaseConfig struct {
	Region    string
	AccessKey string
	SecretKey string
}

type Config struct {
	DB            DatabaseConfig
	ServerPort    string
	JwtSecret     string
	JwtExpiration int
}

func Load(file string) (*Config, error) {
	env := os.Getenv(keyEnv)
	if env != ProdEnv {
		// Load .env file if in development
		err := godotenv.Load(file)
		if err != nil {
			return nil, fmt.Errorf("error loading .env file: %w", err)
		}
	}

	databaseConfig, err := getDatabaseConfig()
	if err != nil {
		return nil, fmt.Errorf("error getting database config: %w", err)
	}

	serverPort := os.Getenv(keyServerPort)
	jwtSecret := os.Getenv(keyJWTSecret)
	jwtExpirationStr := os.Getenv(keyJWTExpiration)

	jwtExpiration, err := strconv.Atoi(jwtExpirationStr)
	if err != nil {
		return nil, fmt.Errorf("invalid JWT expiration value: %w", err)
	}

	return &Config{
		DB:            databaseConfig,
		ServerPort:    serverPort,
		JwtSecret:     jwtSecret,
		JwtExpiration: jwtExpiration,
	}, nil

}

func getDatabaseConfig() (DatabaseConfig, error) {
	databaseConfig := DatabaseConfig{
		Region:    os.Getenv(keyDBRegion),
		AccessKey: os.Getenv(keyDBAccessKey),
		SecretKey: os.Getenv(keyDBSecretKey),
	}

	if err := databaseConfig.Validate(); err != nil {
		return DatabaseConfig{}, err
	}

	return databaseConfig, nil
}

func (dbConfig *DatabaseConfig) Validate() error {
	validate := validator.New()
	if err := validate.Struct(dbConfig); err != nil {
		return fmt.Errorf("missing database env var: %v", err)
	}
	return nil
}
