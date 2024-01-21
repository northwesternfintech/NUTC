package analyzer

import (
	"bufio"
	"encoding/json"
	"io"
)

func Analyze(file io.Reader) (string, error) {
	scanner := bufio.NewScanner(file)
	var transactions []Transaction

	for scanner.Scan() {
		var tx Transaction
		if err := json.Unmarshal([]byte(scanner.Text()), &tx); err != nil {
			return "", err
		}
		transactions = append(transactions, tx)
	}

	if err := scanner.Err(); err != nil {
		return "", err
	}

	wrapper := TransactionsWrapper{Transactions: transactions}
	outputJSON, err := json.MarshalIndent(wrapper, "", "  ")
	if err != nil {
		return "", err
	}
	return string(outputJSON), nil
}
