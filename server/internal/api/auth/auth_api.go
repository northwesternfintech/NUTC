package auth

import (
	"fmt"
	"net/http"
	"server/internal/config"
	"server/internal/endpoint"
	"server/internal/jwt"
	"server/internal/oauth"
	"server/internal/validator"

	"github.com/go-chi/chi/v5"
)

type API struct {
	validator  validator.Validate
	jwtService jwt.Service
	config     config.GoogleOAuthConfig
}

func NewAPI(validator validator.Validate, jwtService jwt.Service, config config.GoogleOAuthConfig) API {
	return API{
		validator: validator,
		config:    config,
	}
}

func (api *API) HandleGoogleOauthCallback(w http.ResponseWriter, r *http.Request) {
	query := r.URL.Query()
	code := query.Get("code")
	state := query.Get("state")

	if code == "" {
		endpoint.WriteWithError(w, http.StatusInternalServerError, endpoint.ErrMsgInternalServer)
		return
	}
	tokenResponse, err := auth.RequestToken(code, &api.config)
	if err != nil {
		endpoint.WriteWithError(w, http.StatusInternalServerError, endpoint.ErrMsgInternalServer)
		return
	}

	googleUser, err := auth.GetGoogleUser(tokenResponse.AccessToken, tokenResponse.IDToken)
	if err != nil {
		endpoint.WriteWithError(w, http.StatusInternalServerError, endpoint.ErrMsgInternalServer)
		return
	}

	fmt.Println(googleUser)	

	
	api.jwtService.GenerateToken()
}

func (api *API) RegisterHandlers(r chi.Router) {
	r.Route("/auth", func(r chi.Router) {
		r.Post("/google/callback", api.HandleGoogleOauthCallback)
	})
}
