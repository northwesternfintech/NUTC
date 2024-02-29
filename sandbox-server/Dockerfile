FROM golang:1.21

WORKDIR /app

COPY go.mod go.sum ./

RUN go mod download

COPY *.go ./
COPY /analyzer ./analyzer

RUN CGO_ENABLED=0 GOOS=linux go build -o /sandbox

EXPOSE 8081

CMD ["/sandbox"]