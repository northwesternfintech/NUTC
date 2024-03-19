FROM golang:1.21 as build

WORKDIR /

COPY go.mod go.sum ./
RUN go mod download

COPY *.go ./
COPY /analyzer ./analyzer

RUN CGO_ENABLED=0 GOOS=linux go build -o /bin/sandbox .

FROM scratch

COPY --from=build /bin/sandbox /bin/sandbox

EXPOSE 12687

CMD ["/bin/sandbox"]
