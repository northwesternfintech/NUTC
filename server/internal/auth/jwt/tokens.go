package jwt

import (
	"errors"
	"fmt"
	"time"

	"github.com/golang-jwt/jwt/v5"
)

type TokenVerificationService interface {
	GenerateToken(userID string) (string, error)
	VerifyToken(token string) (string, error)
}

type tokenService struct {
	jwtSecret  string
	expiration int
}

func NewService(jwtSecret string, expiration int) *tokenService {
	return &tokenService{jwtSecret, expiration}
}

// GenerateToken takes a user ID and generates a jwt token.
func (s *tokenService) GenerateToken(userID string) (string, error) {
	token := jwt.NewWithClaims(jwt.SigningMethodHS256, jwt.MapClaims{
		"userID": userID,
		"exp":    time.Now().Add(time.Duration(s.expiration) * time.Hour).Unix(),
	})
	return token.SignedString([]byte(s.jwtSecret))
}

// VerifyToken parses and validates a jwt token. It returns the userID if the token is valid.
func (s *tokenService) VerifyToken(tokenString string) (string, error) {
	token, err := jwt.Parse(tokenString, func(token *jwt.Token) (interface{}, error) {
		if _, ok := token.Method.(*jwt.SigningMethodHMAC); !ok {
			return nil, fmt.Errorf("Unexpected signing method: %v", token.Header["alg"])
		}
		return []byte(s.jwtSecret), nil
	})
	if err != nil {
		return "", fmt.Errorf("Issue parsing token: %w", err)
	}

	if claims, ok := token.Claims.(jwt.MapClaims); ok && token.Valid {
		userID := claims["userID"]
		userIDStr, ok := userID.(string)
		if !ok {
			return "", fmt.Errorf("Issue parsing userID: %w", err)
		}
		if userIDStr == "" {
			return "", fmt.Errorf("User id not set: %w", err)
		}
		return userIDStr, nil
	}
	return "", errors.New("Invalid token")
}
