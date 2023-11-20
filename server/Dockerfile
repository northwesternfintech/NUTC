FROM golang:latest

WORKDIR /app/src

COPY go.* ./
RUN go mod download && go mod verify

COPY . .

RUN go build -o /app/main ./cmd/main.go

EXPOSE 8080

CMD ["/app/main"]