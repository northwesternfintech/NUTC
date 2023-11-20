package auth

import (
	"errors"
	"fmt"
	"net/http"
	"server/internal/api/user"
	"server/internal/config"
	"server/internal/endpoint"
	"server/internal/jwt"
	"server/internal/logger"
	"server/internal/models"
	auth "server/internal/oauth"
	"server/internal/validator"
	"time"

	"github.com/go-chi/chi/v5"
	"github.com/guregu/dynamo"
)

type API interface {
	HandleGoogleOAuthCallback(w http.ResponseWriter, r *http.Request)
	RegisterHandlers(r chi.Router)
}

type api struct {
	validator     validator.Validate
	jwtService    jwt.Service
	config        config.GoogleOAuthConfig
	jwtExpiration int
	userRepo      user.Repository
}

func NewAPI(validator validator.Validate, jwtService jwt.Service, userRepo user.Repository, config config.GoogleOAuthConfig, jwtExpiration int) API {
	return &api{
		validator:     validator,
		config:        config,
		jwtService:    jwtService,
		jwtExpiration: jwtExpiration,
		userRepo:      userRepo,
	}
}

func (api *api) HandleGoogleOAuthCallback(w http.ResponseWriter, r *http.Request) {
	ctx := r.Context()
	logger := logger.FromContext(ctx)

	query := r.URL.Query()
	code := query.Get("code")
	state := query.Get("state")

	if code == "" {
		logger.Errorf("handler: missing code query parameter")
		endpoint.WriteWithError(logger, w, http.StatusInternalServerError, endpoint.ErrMsgInternalServer)
		return
	}
	if state == "" {
		logger.Errorf("handler: missing state query parameter")
		endpoint.WriteWithError(logger, w, http.StatusInternalServerError, endpoint.ErrMsgInternalServer)
		return
	}

	tokenResponse, err := auth.RequestToken(code, &api.config)
	if err != nil {
		logger.Errorf("handler: error requesting token: %v", err)
		endpoint.WriteWithError(logger, w, http.StatusInternalServerError, endpoint.ErrMsgInternalServer)
		return
	}

	googleUser, err := auth.GetGoogleUser(tokenResponse.AccessToken, tokenResponse.IDToken)
	if err != nil {
		logger.Errorf("handler: error getting google user: %v", err)
		endpoint.WriteWithError(logger, w, http.StatusInternalServerError, endpoint.ErrMsgInternalServer)
		return
	}

	user, err := api.userRepo.GetUserByID(googleUser.ID)
	if err != nil && !errors.Is(err, dynamo.ErrNotFound) {
		logger.Errorf("handler: error getting user: %v", err)
		endpoint.WriteWithError(logger, w, http.StatusInternalServerError, endpoint.ErrMsgInternalServer)
		return
	}

	if user.ID == "" {
		logger.Infof("Creating new user: %s", googleUser.ID)
		err = api.userRepo.CreateUser(models.User{
			ID:        googleUser.ID,
			Name:      googleUser.Name,
			Email:     googleUser.Email,
			CreatedAt: time.Now(),
		})
		if err != nil {
			logger.Errorf("handler: error creating user: %v", err)
			endpoint.WriteWithError(logger, w, http.StatusInternalServerError, endpoint.ErrMsgInternalServer)
			return
		}
		user.ID = googleUser.ID
	}

	token, err := api.jwtService.GenerateToken(user.ID)
	if err != nil {
		logger.Errorf("handler: error generating token: %v", err)
		endpoint.WriteWithError(logger, w, http.StatusInternalServerError, endpoint.ErrMsgInternalServer)
		return
	}

	cookie := http.Cookie{
		Name:     "token",
		Value:    token,
		Path:     "/",
		MaxAge:   api.jwtExpiration,
		// Secure:   true, secure only send over https, so have to turn this off for local development
	}

	http.SetCookie(w, &cookie)
	redirectURIWithState := fmt.Sprintf("%s?state=%s", "http://localhost:3000", state)

	http.Redirect(w, r, redirectURIWithState, http.StatusFound)
}

func (api *api) RegisterHandlers(r chi.Router) {
	r.Route("/auth", func(r chi.Router) {
		r.Get("/google/callback", api.HandleGoogleOAuthCallback)
	})
}
