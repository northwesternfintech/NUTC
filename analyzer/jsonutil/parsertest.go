package jsonutil

// import (
// 	"strings"
// 	"testing"
// )

// func TestParseJSONLines(t *testing.T) {
// 	input := `
// {"name": "John", "amount": 100}
// {"name": "Alice", "amount": 200}
// {"name": "Bob", "amount": 300}
// `

// 	reader := strings.NewReader(input)
// 	parsed, err := ParseJSONLines(reader)

// 	if err != nil {
// 		t.Fatalf("Error parsing JSON: %v", err)
// 	}

// 	expected := map[string][]map[string]interface{}{
// 		"transactions": {
// 			{"name": "John", "amount": 100},
// 			{"name": "Alice", "amount": 200},
// 			{"name": "Bob", "amount": 300},
// 		},
// 	}

// 	if !reflect.DeepEqual(parsed, expected) {
// 		t.Fatalf("Parsed JSON does not match expected result")
// 	}
// }
