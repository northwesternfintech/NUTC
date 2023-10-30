package jsonutil

import (
	"bufio"
	"encoding/json"
	"fmt"
	"os"
)

func Analyze(filename string) error {
	file, err := os.Open(filename)
	if err != nil {
		return err
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)
	var transactions []Transaction

	for scanner.Scan() {
		var tx Transaction
		if err := json.Unmarshal([]byte(scanner.Text()), &tx); err != nil {
			return err
		}
		transactions = append(transactions, tx)
	}

	if err := scanner.Err(); err != nil {
		return err
	}

	wrapper := TransactionsWrapper{Transactions: transactions}
	outputJSON, err := json.MarshalIndent(wrapper, "", "  ")
	if err != nil {
		return err
	}
	fmt.Println(string(outputJSON))

	return nil
}
