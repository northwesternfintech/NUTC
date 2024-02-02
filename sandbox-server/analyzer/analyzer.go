package analyzer

import (
	"bufio"
	"encoding/json"
	"io"
	"reflect"
)

func extractKeysFromStruct(v interface{}) []string {
	val := reflect.ValueOf(v)
	if val.Kind() == reflect.Ptr {
		val = val.Elem()
	}
	keys := []string{}
	for i := 0; i < val.Type().NumField(); i++ {
		keys = append(keys, val.Type().Field(i).Tag.Get("json"))
	}
	return keys
}

func isType(data map[string]interface{}, v interface{}) bool {
	keys := extractKeysFromStruct(v)
	for _, key := range keys {
		if _, ok := data[key]; !ok {
			return false
		}
	}
	return true
}

func Analyze(file io.Reader, user_id string) (string, error) {
	scanner := bufio.NewScanner(file)
	var log_entries []LogEntry

	for scanner.Scan() {
		var entry LogEntry
		if err := json.Unmarshal([]byte(scanner.Text()), &entry); err != nil {
			return "", err
		}
		log_entries = append(log_entries, entry)
	}

	if err := scanner.Err(); err != nil {
		return "", err
	}

	var matches []LogEntry
	var updates []LogEntry

	for _, entry := range log_entries {
		var data = entry.Data.(map[string]interface{})

		if isType(data, Match{}) {
			if data["buyer_id"] == user_id || data["seller_id"] == user_id {
				matches = append(matches, entry)
			}
		} else if isType(data, ObUpdate{}) {
			updates = append(updates, entry)
		}
	}

	wrapper := LogResult{Matches: matches, ObUpdates: updates}
	outputJSON, err := json.MarshalIndent(wrapper, "", "  ")
	if err != nil {
		return "", err
	}
	return string(outputJSON), nil
}
