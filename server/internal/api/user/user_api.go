package user

import (
	"net/http"
	"server/internal/endpoint"
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

func (api *API) RegisterHandlers(r chi.Router) {
	r.Route("/users", func(r chi.Router) {
		r.Get("/{userID}", api.GetUser)
	})
}
