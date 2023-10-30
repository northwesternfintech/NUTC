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
