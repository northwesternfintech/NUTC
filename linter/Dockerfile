#build stage
FROM python:3.11-slim as build

RUN pip install conan numpy pandas polars scipy scikit-learn \
    # c++ stuff
    && apt update \
    && apt install -y --no-install-recommends build-essential libssl-dev cmake git

WORKDIR /app
COPY ./conanfile.py /app/
COPY ../.github/scripts/conan-profile.sh /app/

RUN cat conan-profile.sh | bash \
    && conan install . -b missing

COPY . /app
RUN cmake --preset=ci-docker \
    && cmake --build build --config Release -j


# Main stage
FROM python:3.11-slim

RUN pip install numpy pandas polars scipy scikit-learn

COPY --from=build /app/build/NUTC-linter /bin/NUTC-linter
RUN chmod +x /bin/NUTC-linter

CMD NUTC-linter
