#build stage
FROM python:3.11-slim as build

RUN pip install conan \
    && sudo apt update \
    && sudo apt install -y --no-install-recommends build-essential libssl-dev

WORKDIR /app
COPY . /app

RUN cat .github/scripts/conan-profile.sh | bash \
    && conan install . -b missing \
    && cmake --preset=ci-docker \
    && cmake --build build --config Release -j 2


# Main stange
FROM python:3.11-slim

RUN pip install numpy pandas polars scipy scikit-learn

COPY --from=build /app/build/NUTC-client /bin/NUTC-linter
RUN chmod +x /bin/NUTC-linter

CMD NUTC-linter
