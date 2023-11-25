package jsonutil

type Message struct {
	Ticker    string `json:"ticker"`
	BuyerUID  string `json:"buyer_uid"`
	SellerUID string `json:"seller_uid"`
	Side      int    `json:"side"`
	Price     int    `json:"price"`
	Quantity  int    `json:"quantity"`
}

type Transaction struct {
	Time    string  `json:"time"`
	Type    int     `json:"type"`
	Message Message `json:"message"`
}

type TransactionsWrapper struct {
	Transactions []Transaction `json:"transactions"`
}

type Match struct {
	Ticker    string  `json:"ticker"`
	BuyerUID  string  `json:"buyer_uid"`
	SellerUID string  `json:"seller_uid"`
	Side      int     `json:"side"`
	Price     float64 `json:"price"`
	Quantity  float64 `json:"quantity"`
}

type MarketOrder struct {
	ClientUID string  `json:"client_uid"`
	Side      int     `json:"side"`
	Ticker    string  `json:"ticker"`
	Quantity  float64 `json:"quantity"`
	Price     float64 `json:"price"`
}

type LogEntry struct {
	Data      interface{} `json:"data"`
	Timestamp string      `json:"timestamp"`
}