FROM ubuntu:latest

WORKDIR /app
RUN apt-get update && apt-get install -y python3 python3-dev python3-pip python3.11-venv libpython3.11-dev
RUN python3 -m pip install pandas numpy scipy skikit-learn tensorflow
RUN apt-get install -y ca-certificates
RUN ln -s /usr/lib/python3.11/config-3.11-x86_64-linux-gnu/libpython3.11.so /usr/lib/libpython3.11.so.1.0

COPY ./build/NUTC-client ./NUTC
RUN chmod +x ./NUTC

CMD ./NUTC
