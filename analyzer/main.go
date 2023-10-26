/*
Copyright © 2023 NAME HERE <EMAIL ADDRESS>
*/
package main

// import "analyzer/cmd"

import (
	"fmt"
	"encoding/json"
	"analyzer/jsonutil"
)

func main() {
	input := `{ "time": "2023-10-19T08:39:07.258711Z", "type": 1, "message": {"ticker":"A","buyer_uid":"vzZV25SctsUZOFPpptE8XXYFP6h2","seller_uid":"SIMULATED","side":0,"price":100,"quantity":333} }
	{ "time": "2023-10-19T08:39:07.260756Z", "type": 1, "message": {"ticker":"B","buyer_uid":"vzZV25SctsUZOFPpptE8XXYFP6h2","seller_uid":"SIMULATED","side":0,"price":200,"quantity":166} }
	{ "time": "2023-10-19T08:39:07.261519Z", "type": 1, "message": {"ticker":"C","buyer_uid":"vzZV25SctsUZOFPpptE8XXYFP6h2","seller_uid":"SIMULATED","side":0,"price":300,"quantity":111} }
	{ "time": "2023-10-19T08:39:07.412135Z", "type": 1, "message": {"ticker":"A","buyer_uid":"HPl7qB7JXVPr8whA5CdeHkiIJgy2","seller_uid":"SIMULATED","side":0,"price":100,"quantity":1} }`

	parsed, err := jsonutil.ParseJSONStrings(input)

	if err != nil {
		fmt.Printf("Error parsing JSON: %v\n", err)
		return
	}

	jsonString, err := json.MarshalIndent(parsed, "", "    ")
	if err != nil {
		fmt.Printf("Error marshalling JSON: %v\n", err)
		return
	}

	// Print the JSON string.
	fmt.Println(string(jsonString))

	// fmt.Printf("Parsed JSON: %+v\n", parsed)
}
