package config

import (
	"fmt"
	"os"
	"strconv"

	"server/internal/validator"

	"github.com/joho/godotenv"
)

const (
	keyDBRegion    = "DB_REGION"
	keyDBAccessKey = "DB_ACCESS_KEY"
	keyDBSecretKey = "DB_SECRET_KEY"

	keyEnv           = "ENV"
	keyServerPort    = "SERVER_PORT"
	keyJWTSecret     = "JWT_SIGNING_KEY"
	keyJWTExpiration = "JWT_EXPIRATION"

	keyGoogleOAuthRedirectURL  = "GOOGLE_OAUTH_REDIRECT_URL"
	keyGoogleOAuthClientID     = "GOOGLE_OAUTH_CLIENT_ID"
	keyGoogleOAuthClientSecret = "GOOGLE_OAUTH_CLIENT_SECRET"

	ProdEnv = "production"
	DevEnv  = "development"
)

type GoogleOAuthConfig struct {
	RedirectURL  string `validate:"required,url"`
	ClientID     string `validate:"required"`
	ClientSecret string `validate:"required"`
}

type DatabaseConfig struct {
	Region    string `validate:"required"`
	AccessKey string `validate:"required"`
	SecretKey string `validate:"required"`
}

type Config struct {
	DB            DatabaseConfig
	GoogleOAuth   GoogleOAuthConfig
	ServerPort    string `validate:"required"`
	JwtSecret     string `validate:"required"`
	JwtExpiration int    `validate:"required"`
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

	GoogleOAuthConfig, err := getGoogleOAuthConfig()
	if err != nil {
		return nil, fmt.Errorf("error getting Google OAuth config: %w", err)
	}

	serverPort := os.Getenv(keyServerPort)
	jwtSecret := os.Getenv(keyJWTSecret)
	jwtExpirationStr := os.Getenv(keyJWTExpiration)

	jwtExpiration, err := strconv.Atoi(jwtExpirationStr)
	if err != nil {
		return nil, fmt.Errorf("invalid JWT expiration value: %w", err)
	}

	config := &Config{
		DB:            databaseConfig,
		GoogleOAuth:   GoogleOAuthConfig,
		ServerPort:    serverPort,
		JwtSecret:     jwtSecret,
		JwtExpiration: jwtExpiration,
	}
	config.Validate()

	return config, nil
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

func getGoogleOAuthConfig() (GoogleOAuthConfig, error) {
	googleOAuthConfig := GoogleOAuthConfig{
		RedirectURL:  os.Getenv(keyGoogleOAuthRedirectURL),
		ClientID:     os.Getenv(keyGoogleOAuthClientID),
		ClientSecret: os.Getenv(keyGoogleOAuthClientSecret),
	}

	if err := googleOAuthConfig.Validate(); err != nil {
		return GoogleOAuthConfig{}, err
	}

	return googleOAuthConfig, nil
}

func (googleOAuthConfig *GoogleOAuthConfig) Validate() error {
	validate := validator.New()
	if err := validate.Struct(googleOAuthConfig); err != nil {
		return fmt.Errorf("missing Google OAuth env var: %v", err)
	}
	return nil
}

func (cfg *Config) Validate() error {
	validate := validator.New()
	if err := validate.Struct(cfg); err != nil {
		return fmt.Errorf("error validating config: %v", err)
	}
	return nil
}
