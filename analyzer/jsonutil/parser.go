package jsonutil

import (
	// "encoding/json"
	"strings"
	// "fmt"
)

// ParseJSONLines reads a JSON array with objects on separate lines from the input reader
// and returns a structured JSON object with a "transactions" field.

// ParseJSONStrings takes a string containing single-line JSON objects and returns
// a structured JSON object with a "transactions" field.
func ParseJSONStrings(input string) (map[string][]string, error) {
	// fmt.Print(input)
	// transactions := make([]map[string]interface{}, 0)
	
	lines := strings.Split(input, "\n")
	// fmt.Print(len(lines))
	// for _, line := range lines {
	// 	fmt.Print(line)
	// 	var item map[string]interface{}
	// 	if err := json.Unmarshal([]byte(line), &item); err != nil {
	// 		return nil, err
	// 	}
	// 	transactions = append(transactions, item)
	// }

	result := map[string][]string{"transactions": lines}
	return result, nil
}