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

type Message struct {
	Data      Transaction `json:"data"`
	Timestamp Timestamp   `json:"timestamp"`
}

type Transaction struct {
	Security string  `json:"security"`
	Side     int     `json:"side"`
	Price    float64 `json:"price"`
	Quantity int     `json:"quantity"`
}

type TransactionsWrapper struct {
	Transactions []Transaction `json:"transactions"`
}
