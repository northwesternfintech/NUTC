package auth

import (
	"fmt"
	"net/http"
	"server/internal/api/user"
	"server/internal/config"
	"server/internal/endpoint"
	"server/internal/jwt"
	"server/internal/models"
	"server/internal/oauth"
	"server/internal/validator"
	"time"

	"github.com/go-chi/chi/v5"
)

type API struct {
	validator  validator.Validate
	jwtService jwt.Service
	config     config.GoogleOAuthConfig
	jwtMaxAge  int
	userRepo   user.Repository
}

func NewAPI(validator validator.Validate, jwtService jwt.Service, userRepo user.Repository, config config.GoogleOAuthConfig, jwtMaxAge int) API {
	return API{
		validator:  validator,
		config:     config,
		jwtService: jwtService,
		jwtMaxAge:  jwtMaxAge,
		userRepo:   userRepo,
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

	err = api.userRepo.CreateUser(models.User{
		ID:        googleUser.ID,
		Name:      googleUser.Name,
		Email:     googleUser.Email,
		CreatedAt: time.Now(),
	})
	if err != nil {
		endpoint.WriteWithError(w, http.StatusInternalServerError, endpoint.ErrMsgInternalServer)
		return
	}

	user, err := api.userRepo.GetUserByID(googleUser.ID)
	if err != nil {
		endpoint.WriteWithError(w, http.StatusInternalServerError, endpoint.ErrMsgInternalServer)
		return
	}

	if user.ID == "" {
		err = api.userRepo.CreateUser(models.User{
			ID:        googleUser.ID,
			Name:      googleUser.Name,
			Email:     googleUser.Email,
			CreatedAt: time.Now(),
		})
		if err != nil {
			endpoint.WriteWithError(w, http.StatusInternalServerError, endpoint.ErrMsgInternalServer)
			return
		}
		user.ID = googleUser.ID
	}

	token, err := api.jwtService.GenerateToken(user.ID)
	if err != nil {
		endpoint.WriteWithError(w, http.StatusInternalServerError, endpoint.ErrMsgInternalServer)
		return
	}
	
	cookie := http.Cookie{
		Name:     "token",
		Value:    token,
		Path:     "/",
		MaxAge:   60 * api.jwtMaxAge,
		HttpOnly: true,
		Secure:   true,
	}

	http.SetCookie(w, &cookie)

	http.Redirect(w, r, state, http.StatusFound)
}

func (api *API) RegisterHandlers(r chi.Router) {
	r.Route("/auth", func(r chi.Router) {
		r.Post("/google/callback", api.HandleGoogleOauthCallback)
	})
}
