package main

import (
	"bytes"
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"net/http"
	"net/url"
	"os"

	"github.com/aws/aws-lambda-go/events"
	"github.com/aws/aws-lambda-go/lambda"
)

const errMsgMissingCode = "Missing code query parameter"
const errMsgMissingState = "Missing state query parameter"
const errMsgInternalServer = "Internal server error"

func HandleGoogleOAuthCallback(ctx context.Context, request events.APIGatewayProxyRequest) (events.APIGatewayProxyResponse, error) {
	code := request.QueryStringParameters["code"]
	state := request.QueryStringParameters["state"]

	if code == "" {
		return events.APIGatewayProxyResponse{
			StatusCode: http.StatusInternalServerError,
			Body:       errMsgMissingCode,
		}, nil
	}

	if state == "" {
		return events.APIGatewayProxyResponse{
			StatusCode: http.StatusInternalServerError,
			Body:       errMsgMissingState,
		}, nil
	}

	googleOauthConfig := &GoogleOAuthConfig{
		RedirectURL:  os.Getenv("GOOGLE_OAUTH_REDIRECT_URL"),
		ClientID:     os.Getenv("GOOGLE_OAUTH_CLIENT_ID"),
		ClientSecret: os.Getenv("GOOGLE_OAUTH_CLIENT_SECRET"),	
	}

	tokenResponse, err := RequestToken(code, googleOauthConfig)
	if err != nil {
		return events.APIGatewayProxyResponse{
			StatusCode: http.StatusInternalServerError,
			Body:       errMsgInternalServer,
		}, nil
	}

	googleUser, err := GetGoogleUser(tokenResponse.AccessToken, tokenResponse.IDToken)
	if err != nil {
		return events.APIGatewayProxyResponse{
			StatusCode: http.StatusInternalServerError,
			Body:       errMsgInternalServer,
		}, nil
	}
	
	return events.APIGatewayProxyResponse{
		StatusCode: http.StatusOK,
		Body:       fmt.Sprintf("Hello, %s", googleUser.Name),
	}, nil
}

func main() {
	lambda.Start(HandleGoogleOAuthCallback)
}

type GoogleOAuthConfig struct {
	RedirectURL  string `validate:"required,url"`
	ClientID     string `validate:"required"`
	ClientSecret string `validate:"required"`
}

type OAuthResponse struct {
	AccessToken string `json:"access_token"`
	IDToken     string `json:"id_token"`
}

func RequestToken(authorizationCode string, config *GoogleOAuthConfig) (*OAuthResponse, error) {
	client := &http.Client{}
	data := url.Values{
		"grant_type":    {"authorization_code"},
		"redirect_uri":  {config.RedirectURL},
		"client_id":     {config.ClientID},
		"code":          {authorizationCode},
		"client_secret": {config.ClientSecret},
	}

	req, err := http.NewRequest("POST", "https://oauth2.googleapis.com/token", bytes.NewBufferString(data.Encode()))
	if err != nil {
		return nil, err
	}
	req.Header.Add("Content-Type", "application/x-www-form-urlencoded")

	resp, err := client.Do(req)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, errors.New("error requesting token")
	}

	var oauthResponse OAuthResponse
	err = json.NewDecoder(resp.Body).Decode(&oauthResponse)
	if err != nil {
		return nil, err
	}

	return &oauthResponse, nil
}

type GoogleUserResult struct {
    ID            string `json:"id"`
    Email         string `json:"email"`
    VerifiedEmail bool   `json:"verified_email"`
    Name          string `json:"name"`
    GivenName     string `json:"given_name"`
    FamilyName    string `json:"family_name"`
    Picture       string `json:"picture"`
    Locale        string `json:"locale"`
}


func GetGoogleUser(accessToken string, idToken string) (*GoogleUserResult, error) {
    client := &http.Client{}
    reqURL := "https://www.googleapis.com/oauth2/v1/userinfo?alt=json&access_token=" + accessToken

    req, err := http.NewRequest("GET", reqURL, nil)
    if err != nil {
        return nil, err
    }
    req.Header.Add("Authorization", "Bearer "+idToken)

    resp, err := client.Do(req)
    if err != nil {
        return nil, err
    }
    defer resp.Body.Close()

    if resp.StatusCode != http.StatusOK {
        return nil, errors.New("error retrieving user information")
    }

    var userInfo GoogleUserResult
    err = json.NewDecoder(resp.Body).Decode(&userInfo)
    if err != nil {
    return nil, err
    }

    return &userInfo, nil
}
