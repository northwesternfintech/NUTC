package analyzer

import (
	"strings"
	"time"
)

type Timestamp struct {
	time.Time
}

func (ct *Timestamp) UnmarshalJSON(b []byte) error {
	const layout = "2006-01-02 15:04:05.000"
	s := strings.Trim(string(b), `"`)
	t, err := time.Parse(layout, s)
	if err != nil {
		return err
	}
	ct.Time = t
	return nil
}

type Match struct {
	Ticker   string  `json:"ticker"`
	BuyerId  string  `json:"buyer_id"`
	SellerId string  `json:"seller_id"`
	Side     int     `json:"side"`
	Price    float64 `json:"price"`
	Quantity float64 `json:"quantity"`
}

type ObUpdate struct {
	Security string  `json:"security"`
	Side     int     `json:"side"`
	Price    float64 `json:"price"`
	Quantity float64 `json:"quantity"`
}

type LogEntry struct {
	Timestamp Timestamp   `json:"timestamp"`
	Data      interface{} `json:"data"`
}

type LogResult struct {
	Matches   []LogEntry `json:"matches"`
	ObUpdates []LogEntry `json:"ob_updates"`
}
