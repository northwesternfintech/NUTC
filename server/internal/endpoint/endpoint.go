package endpoint

import (
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"server/internal/validator"
)

const (
	errMsgInvalidReq     = "Invalid request"
	errMsgJSONDecode     = "Failed to decode json request"
	ErrMsgInternalServer = "Internal server error"
)

type errResponse struct {
	Status  int    `json:"status"`
	Message string `json:"message"`
}

type successResponse struct {
	Message string `json:"message"`
}

// WriteWithError sets the response header to application/json, writes the header
// with a status code, and returns an error response with a status and mesage
func WriteWithError(w http.ResponseWriter, statusCode int, errMsg string) {
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(statusCode)

	errResponse := errResponse{
		Status:  statusCode,
		Message: errMsg,
	}
	if err := json.NewEncoder(w).Encode(errResponse); err != nil {
		log.Printf("Failed to encode error response into JSON: %v", err)
		w.WriteHeader(http.StatusInternalServerError)
	}
}

// WriteWithStatus sets the response header to application/json, write the header
// with a status code, and encodes and writes the data json.NewEncoder()
func WriteWithStatus(w http.ResponseWriter, statusCode int, data interface{}) {
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(statusCode)

	if data != nil {
		if err := json.NewEncoder(w).Encode(data); err != nil {
			log.Printf("Failed to encode API response into JSON: %v", err)
			w.WriteHeader(http.StatusInternalServerError)
		}
	} else if statusCode == http.StatusOK {
		if err := json.NewEncoder(w).Encode(successResponse{Message: "Success."}); err != nil {
			log.Printf("Failed to encode API response into JSON: %v", err)
			w.WriteHeader(http.StatusInternalServerError)
		}
	}
}

// buildDecodeErrorMsg formats the decode error and returns it as a string
func buildDecodeErrorMsg(field string, want string, got string) string {
	return fmt.Sprintf("Expected %s to be %s, got %s", field, want, got)
}

// HandleDecodeErr responds with the appropriate decode error msg and sets
// the http status to 400
func HandleDecodeErr(w http.ResponseWriter, err error) {
	errMsg := errMsgJSONDecode
	if err, ok := err.(*json.UnmarshalTypeError); ok {
		errMsg = buildDecodeErrorMsg(err.Field, err.Type.String(), err.Value)
	}

	WriteWithError(w, http.StatusBadRequest, errMsg)
}

// WriteValidationErr responds with the appropriate validation error msg and
// sets the http status to 400
func WriteValidationErr(w http.ResponseWriter, s interface{}, err error) {
	errMsg := errMsgInvalidReq
	validationErrMsg := validator.GetValidationErrMsg(s, err)
	if validationErrMsg != "" {
		errMsg = validationErrMsg
	}

	WriteWithError(w, http.StatusBadRequest, errMsg)
}
