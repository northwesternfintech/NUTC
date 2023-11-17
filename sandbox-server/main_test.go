package main

import (
	"fmt"
	"net/http"
	"net/http/httptest"
	"testing"
)

const validUserID = "123"
const validAlgoID = "123"

const invalidUserID = "123$"
const invalidAlgoID = "123$"

func MakeRequest(t *testing.T, userID, algoID string) (int, string) {
	var url string
	if userID == "" {
		url = fmt.Sprintf("/algo_id=%s", algoID)
	} else if algoID == "" {
		url = fmt.Sprintf("/user_id=%s", userID)
	} else {
		url = fmt.Sprintf("/?user_id=%s&algo_id=%s", userID, algoID)
	}

	req, err := http.NewRequest("GET", url, nil)
	if err != nil {
		t.Errorf("Failed to create request: %v", err)
	}
	rr := httptest.NewRecorder()
	handler := http.HandlerFunc(algoTestingHandler)

	// Test valid request
	handler.ServeHTTP(rr, req)
	return rr.Code, rr.Body.String()
}

func TestValidRequest(t *testing.T) {
	statusCode, responseMsg := MakeRequest(t, validUserID, validAlgoID)
	if statusCode != http.StatusOK {
		t.Errorf("Expected status OK, got %v with message %s", statusCode, responseMsg)
	}
}

func TestMissingParameters(t *testing.T) {
	statusCode, responseMsg := MakeRequest(t, "", "")
	if statusCode != http.StatusBadRequest {
		t.Errorf("Expected status BadRequest, got %v with message %s", statusCode, responseMsg)
	}

	statusCode, responseMsg = MakeRequest(t, validUserID, "")
	if statusCode != http.StatusBadRequest {
		t.Errorf("Expected status BadRequest, got %v with message %s", statusCode, responseMsg)
	}

	statusCode, responseMsg = MakeRequest(t, "", validAlgoID)
	if statusCode != http.StatusBadRequest {
		t.Errorf("Expected status BadRequest, got %v with message %s", statusCode, responseMsg)
	}
}

func TestInvalidIDs(t *testing.T) {
	statusCode, responseMsg := MakeRequest(t, invalidUserID, invalidAlgoID)
	if statusCode != http.StatusBadRequest {
		t.Errorf("Expected status BadRequest, got %v with message %s", statusCode, responseMsg)
	}

	statusCode, responseMsg = MakeRequest(t, validUserID, invalidAlgoID)
	if statusCode != http.StatusBadRequest {
		t.Errorf("Expected status BadRequest, got %v with message %s", statusCode, responseMsg)
	}
}

func TestTimeout(t *testing.T) {
}
