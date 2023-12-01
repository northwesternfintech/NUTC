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
	log_entries, err := readAndParseLogEntries(filename)
	if err != nil {
		fmt.Println("Error reading and parsing log entries:", err)
		return err
	}

	num_entries := len(log_entries)
	if num_entries == 0 {
		fmt.Println("empty file")
		return nil
	}

	cumulative_orders, err := calculateCumulativeCounts(log_entries, isMarketOrder)
	if err != nil {
		fmt.Println("Error calculating cumulative counts for market orders:", err)
		return err
	}

	csv_file, err := os.Create("market_order.csv")
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

	fmt.Println("CSV file 'market_order.csv' generated successfully.")
	return nil
}

func CountMatch(filename string) error {
	log_entries, err := readAndParseLogEntries(filename)
	if err != nil {
		fmt.Println("Error reading and parsing log entries:", err)
		return err
	}

	num_entries := len(log_entries)
	if num_entries == 0 {
		fmt.Println("empty file")
		return nil
	}

	cumulative_matches, err := calculateCumulativeCounts(log_entries, isMatch)
	if err != nil {
		fmt.Println("Error calculating cumulative counts for matches:", err)
		return err
	}

	//create CSV file
	csv_file, err := os.Create("matches.csv")
	if err != nil {
		return err
	}
	defer csv_file.Close()

	writer := csv.NewWriter(csv_file)
	defer writer.Flush()

	if err := writer.Write([]string{"time", "cum_matches"}); err != nil {
		return err
	}

	for seconds, cum_matches := range cumulative_matches {
		if err := writer.Write([]string{fmt.Sprintf("%ds", seconds), strconv.Itoa(cum_matches)}); err != nil {
			return err
		}
	}

	fmt.Println("CSV file 'matches.csv' generated successfully.")
	return nil
}

func isMarketOrder(data interface{}) bool {
	// Type assert the interface to a map
	data_map, ok := data.(map[string]interface{})
	if !ok {
		// Not a map
		return false
	}

	// Check if the required fields are present
	_, clientUID_present := data_map["client_uid"]
	_, side_present := data_map["side"]
	_, ticker_present := data_map["ticker"]
	_, quantity_present := data_map["quantity"]
	_, price_present := data_map["price"]

	// Check if all required fields are present
	return clientUID_present && side_present && ticker_present && quantity_present && price_present
}

func isMatch(data interface{}) bool {
	// Type assert the interface to a map
	data_map, ok := data.(map[string]interface{})
	if !ok {
		// Not a map
		return false
	}

	// Check if the required fields are present
	_, ticker_present := data_map["ticker"]
	_, buyerUID_present := data_map["buyer_uid"]
	_, sellerUID_present := data_map["seller_uid"]
	_, side_present := data_map["side"]
	_, price_present := data_map["price"]
	_, quantity_present := data_map["quantity"]


	// Check if all required fields are present
	return ticker_present && buyerUID_present && sellerUID_present && side_present && quantity_present && price_present
}

func readAndParseLogEntries(filename string) ([]LogEntry, error) {
	file, err := os.Open(filename)
	if err != nil {
		return nil, err
	}
	defer file.Close()

	var log_entries []LogEntry
	scanner := bufio.NewScanner(file)

	for scanner.Scan() {
		var entry LogEntry
		if err := json.Unmarshal([]byte(scanner.Text()), &entry); err != nil {
			return nil, err
		}
		log_entries = append(log_entries, entry)
	}

	if err := scanner.Err(); err != nil {
		return nil, err
	}

	return log_entries, nil
}

func calculateCumulativeCounts(log_entries []LogEntry, isEntryValid func(interface{}) bool) ([]int, error) {
	//get start and end time to calculate the timespan of the logger
	startTime, err := time.Parse("2006-01-02 15:04:05.999", log_entries[0].Timestamp)
	if err != nil {
		return nil, fmt.Errorf("Error parsing start time: %v", err)
	}

	numEntries := len(log_entries)
	endTime, err := time.Parse("2006-01-02 15:04:05.999", log_entries[numEntries-1].Timestamp)
	if err != nil {
		return nil, fmt.Errorf("Error parsing end time: %v", err)
	}

	timeDifference := int(endTime.Sub(startTime).Seconds())

	//count up the entries for every second
	cumulativeCounts := make([]int, timeDifference+1)

	count := 0
	lastCount := 0

	for _, entry := range log_entries {
		if !isEntryValid(entry.Data) {
			continue
		}

		curTime, err := time.Parse("2006-01-02 15:04:05.999", entry.Timestamp)
		if err != nil {
			return nil, fmt.Errorf("Error parsing end time: %v", err)
		}

		secondsDiff := int(curTime.Sub(startTime).Seconds())

		// Update count for all seconds between lastCount and secondsDiff
		for i := lastCount + 1; i <= secondsDiff; i++ {
			cumulativeCounts[i] = count
		}

		count++
		cumulativeCounts[secondsDiff] = count
		lastCount = secondsDiff
	}

	// Fill in the remaining seconds if any
	for i := lastCount + 1; i <= timeDifference; i++ {
		cumulativeCounts[i] = count
	}

	return cumulativeCounts, nil
}
