package user_api

import (
	"net/http"
	"server/internal/auth/jwt"
	"server/internal/http"
	"server/internal/logger"
	"server/internal/validator"

	"github.com/go-chi/chi/v5"
)

type API struct {
	validator validator.Validate
	userRepo  Repository
}

func NewAPI(validator validator.Validate, userRepo Repository) API {
	return API{
		validator: validator,
		userRepo:  userRepo,
	}
}

func (api *API) GetUser(w http.ResponseWriter, r *http.Request) {
	ctx := r.Context()
	logger := logger.FromContext(ctx)

	userID := chi.URLParam(r, "userID")
	if userID == "" {
		logger.Errorf("handler: missing userID path parameter")
		http_utils.WriteWithError(logger, w, http.StatusBadRequest, http_utils.ErrMsgBadRequest)
		return
	}

	user, err := api.userRepo.GetUserByID(userID)
	if err != nil {
		logger.Errorf("handler: error getting user: %v", err)
		http_utils.WriteWithError(logger, w, http.StatusInternalServerError, http_utils.ErrMsgInternalServer)
		return
	}
	http_utils.WriteWithStatus(logger, w, http.StatusOK, user)
}

func (api *API) HandleGetMe(w http.ResponseWriter, r *http.Request) {
	ctx := r.Context()
	logger := logger.FromContext(ctx)
	userID := jwt.UserIDFromContext(ctx)

	if userID == "" {
		logger.Errorf("handler: missing userID path parameter")
		http_utils.WriteWithError(logger, w, http.StatusBadRequest, http_utils.ErrMsgBadRequest)
		return
	}

	user, err := api.userRepo.GetUserByID(userID)
	if err != nil {
		logger.Errorf("handler: error getting user: %v", err)
		http_utils.WriteWithError(logger, w, http.StatusInternalServerError, http_utils.ErrMsgInternalServer)
		return
	}

	http_utils.WriteWithStatus(logger, w, http.StatusOK, user)
}

func (api *API) RegisterHandlers(r chi.Router, authHandler func(http.Handler) http.Handler) {
	r.Route("/users", func(r chi.Router) {
		r.Get("/{userID}", api.GetUser)
		r.Group(func(r chi.Router) {
			r.Group(func(r chi.Router) {
				r.Use(authHandler)
				r.Get("/me", api.HandleGetMe)
			})
		})
	})
}
