FROM rust:latest as builder

WORKDIR /app

COPY webserver/Cargo.toml webserver/Cargo.lock ./

RUN mkdir src \
    && echo "// dummy file" > src/lib.rs \
    && cargo build --release

COPY webserver/src src

RUN cargo build --release

COPY webserver/.env .env

EXPOSE 16124

CMD ["./target/release/rust-server"]
