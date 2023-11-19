package auth

import (
	"bytes"
	"encoding/json"
	"errors"
	"net/http"
	"net/url"
	"server/internal/config"
)

// OAuthResponse represents the OAuth response.
type OAuthResponse struct {
    AccessToken string `json:"access_token"`
    IDToken     string `json:"id_token"`
}

// GoogleUserResult represents the Google user information.
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

// RequestToken requests an OAuth token.
func RequestToken(authorizationCode string, config *config.GoogleOAuthConfig) (*OAuthResponse, error) {
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

// GetGoogleUser retrieves user information from Google.
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
