package main

import (
	"context"
	"net/http"
	"os"
	"os/signal"
	"server/internal/api/auth"
	"server/internal/api/user"
	"server/internal/config"
	"server/internal/db"
	"server/internal/jwt"
	"server/internal/logger"
	"server/internal/middleware"
	"syscall"
	"time"

	"server/internal/validator"

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

	db, err := db.New(cfg.DB)
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
	r.Use(middleware.RequestLogger(logger))
	r.Use(middleware.Cors())

	userRepo := user.NewRepository(db)
	userAPI := user.NewAPI(v, userRepo)

	jwtService := jwt.NewService(cfg.JwtSecret, cfg.JwtExpiration)
	authHandler := middleware.Auth(jwtService)

	authAPI := auth.NewAPI(v, jwtService, userRepo, cfg.GoogleOAuth, cfg.JwtExpiration)

	userAPI.RegisterHandlers(r, authHandler)
	authAPI.RegisterHandlers(r)

	return r
}
