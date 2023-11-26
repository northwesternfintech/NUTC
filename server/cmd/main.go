package main

import (
	"context"
	"net/http"
	"os"
	"os/signal"
	"server/api/auth"
	"server/api/database"
	"server/internal/auth/jwt"
	"server/internal/config"
	"server/internal/database"
	"server/internal/http"
	"server/internal/logger"
	"server/internal/validator"
	"syscall"
	"time"

	"github.com/go-chi/chi/v5"
	"github.com/guregu/dynamo"
)

func main() {
	logger := logger.New()
	validator := validator.New()

	cfg, err := config.Load(".env")
	if err != nil {
		logger.Fatalf("Error loading config: %v", err)
	}

	db, err := database.New(cfg.DB)
	if err != nil {
		logger.Fatalf("Error connecting to database: %v", err)
	}

	mux := chi.NewRouter()
	r := setupHandler(mux, validator, cfg, db, logger)

	server := http.Server{
		Addr:    cfg.ServerPort,
		Handler: r,
	}

	// Graceful shutdown
	stop := make(chan os.Signal, 1)
	signal.Notify(stop, os.Interrupt, syscall.SIGTERM)

	go func() {
		logger.Infof("Server listening on %s", cfg.ServerPort)
		if err := server.ListenAndServe(); err != nil && err != http.ErrServerClosed {
			logger.Fatalf("Could not start server: %s", err)
		}
	}()

	<-stop

	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()

	if err := server.Shutdown(ctx); err != nil {
		logger.Fatalf("Server forced shutdown: %s", err)
	}
	logger.Infof("Server gracefully stopped")
}

func setupHandler(
	r chi.Router,
	v validator.Validate,
	cfg *config.Config,
	db *dynamo.DB,
	logger logger.Logger,
) chi.Router {
	r.Use(http_utils.RequestLogger(logger))
	r.Use(http_utils.Cors())

	userRepo := user_api.NewRepository(db)
	userAPI := user_api.NewAPI(v, userRepo)

	jwtService := jwt.NewService(cfg.JwtSecret, cfg.JwtExpiration)
	authHandler := jwt.AuthenticateToken(jwtService)

	authAPI := auth_api.NewAPI(v, jwtService, userRepo, cfg.GoogleOAuth, cfg.JwtExpiration)

	userAPI.RegisterHandlers(r, authHandler)
	authAPI.RegisterHandlers(r)

	return r
}
