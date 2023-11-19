package user

import (
	"log"
	"net/http"
	"server/internal/endpoint"
	"server/internal/middleware"
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
	userID := chi.URLParam(r, "userID")
	if userID == "" {
		endpoint.WriteWithError(w, http.StatusBadRequest, endpoint.ErrMsgBadRequest)
		return
	}

	user, err := api.userRepo.GetUserByID(userID)
	if err != nil {
		endpoint.WriteWithError(w, http.StatusInternalServerError, endpoint.ErrMsgInternalServer)
		return
	}
	endpoint.WriteWithStatus(w, http.StatusOK, user)
}

func (api *API) HandleGetMe(w http.ResponseWriter, r *http.Request) {
	ctx := r.Context()
	userID := middleware.UserIDFromContext(ctx)
	log.Printf("userID: %s", userID)

	if userID == "" {
		log.Printf("Error getting userID from context")
		endpoint.WriteWithError(w, http.StatusBadRequest, endpoint.ErrMsgBadRequest)
		return
	}
	
	user, err := api.userRepo.GetUserByID(userID)
	if err != nil {
		log.Printf("Error getting user: %v", err)
		endpoint.WriteWithError(w, http.StatusInternalServerError, endpoint.ErrMsgInternalServer)
		return
	}
	endpoint.WriteWithStatus(w, http.StatusOK, user)
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
