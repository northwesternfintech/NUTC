package jsonutil

import (
	"bufio"
	"encoding/json"
	"encoding/csv"
	"fmt"
	"os"
	"time"
	"strconv"
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

func CountMarketOrders(filename string) error {
	file, err := os.Open(filename)
	if err != nil {
		return err
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)
	var log_entries []LogEntry

	for scanner.Scan() {
		var entry LogEntry
		if err := json.Unmarshal([]byte(scanner.Text()), &entry); err != nil {
			return err
		}
		log_entries = append(log_entries, entry)
	}

	if err := scanner.Err(); err != nil {
		return err
	}

	num_entries := len(log_entries)
	if num_entries == 0 {
		fmt.Println("empty file")
		return nil
	}

	//get start and end time to calculate the timespan of the logger
	start_time, err := time.Parse("2006-01-02 15:04:05.999", log_entries[0].Timestamp)
    if err != nil {
        fmt.Println("Error parsing start time:", err)
        return nil
	}

	end_time, err := time.Parse("2006-01-02 15:04:05.999", log_entries[num_entries-1].Timestamp)
    if err != nil {
        fmt.Println("Error parsing end time:", err)
        return nil
	}

	time_difference := int(end_time.Sub(start_time).Seconds())

	//count up the market orders for every second
	cumulative_orders := make([]int, time_difference+1)

	for _, entry := range log_entries {
		fmt.Println(isMarketOrder(entry.Data))
		if !isMarketOrder(entry.Data) {
			continue
		}

		cur_time, err := time.Parse("2006-01-02 15:04:05.999", entry.Timestamp)
		if err != nil {
			fmt.Println("Error parsing end time:", err)
			return nil
		}

		seconds_diff := int(cur_time.Sub(start_time).Seconds())
		
		cumulative_orders[seconds_diff]++
	}

	//create CSV file
	csv_file, err := os.Create("output.csv")
	if err != nil {
		return err
	}
	defer csv_file.Close()

	writer := csv.NewWriter(csv_file)
	defer writer.Flush()

	if err := writer.Write([]string{"time", "cum_orders"}); err != nil {
		return err
	}

	for seconds, cum_orders := range cumulative_orders {
		if err := writer.Write([]string{fmt.Sprintf("%ds", seconds), strconv.Itoa(cum_orders)}); err != nil {
			return err
		}
	}

	fmt.Println("CSV file 'output.csv' generated successfully.")
	return nil
}


func isMarketOrder(data interface{}) bool {
	// Type assert the interface to a map
	dataMap, ok := data.(map[string]interface{})
	if !ok {
		// Not a map
		return false
	}

	// Check if the required fields are present
	_, clientUIDPresent := dataMap["client_uid"]
	_, sidePresent := dataMap["side"]
	_, tickerPresent := dataMap["ticker"]
	_, quantityPresent := dataMap["quantity"]
	_, pricePresent := dataMap["price"]

	// Check if all required fields are present
	return clientUIDPresent && sidePresent && tickerPresent && quantityPresent && pricePresent
}
