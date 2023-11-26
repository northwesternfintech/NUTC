package jwt

import (
	"context"
	"net/http"
	"server/internal/http"
	"server/internal/logger"
)

type userID int

const (
	keyUserID userID = -1

	errMsgMissingCookie = "Missing authentication token."
	errMsgInvalidToken  = "Token is invalid."
)

// Auth creates a middleware function that retrieves a bearer token and validates the token.
// The middleware sets the userID in the jwt payload into the request context. If the token is
// invalid, it will write an Unauthorized response.
func AuthenticateToken(jwtService TokenVerificationService) func(next http.Handler) http.Handler {
	return func(next http.Handler) http.Handler {
		return http.HandlerFunc(func(w http.ResponseWriter, req *http.Request) {
			ctx := req.Context()
			logger := logger.FromContext(ctx)

			cookie, err := req.Cookie("token")
			if err != nil {
				logger.Errorf("handler: issue getting cookie: %v\n", err)
				http_utils.WriteWithError(logger, w, http.StatusUnauthorized, errMsgMissingCookie)
				return
			}
			token := cookie.Value

			// Verify the JWT token
			userID, err := jwtService.VerifyToken(token)
			if err != nil {
				logger.Errorf("handler: issue verifying token: %v\n", err)
				http_utils.WriteWithError(logger, w, http.StatusUnauthorized, errMsgInvalidToken)
				return
			}

			// Add user information to the context and proceed
			req = req.WithContext(withUser(ctx, userID))
			next.ServeHTTP(w, req)
		})
	}
}

// UserIDFromContext returns a user ID from context
func UserIDFromContext(ctx context.Context) string {
	if userID, ok := ctx.Value(keyUserID).(string); ok {
		return userID
	}
	return ""
}

// withUser adds the userID to a context object and returns that context
func withUser(ctx context.Context, userID string) context.Context {
	return context.WithValue(ctx, keyUserID, userID)
}
