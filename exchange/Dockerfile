#should be run from root
#NOT DESIGNED TO BE RUN IN GITHUB ACTIONS (TODO)

# Build stage for wrapper
FROM python:3.11-slim as build

RUN pip install conan numpy pandas polars scipy scikit-learn \
    # c++ stuff
    && apt update \
    && apt install -y --no-install-recommends build-essential libssl-dev cmake git

WORKDIR /app/wrapper
COPY ./wrapper/conanfile.py /app/wrapper
COPY ./.github/scripts/conan-profile.sh /app/wrapper

RUN cat conan-profile.sh | bash \
    && conan install . -b missing

COPY ./wrapper /app/wrapper
RUN cmake --preset=ci-docker \
    && cmake --build build --config Release -j

# Main stage for wrapper
FROM python:3.11-slim

RUN pip install numpy pandas polars scipy scikit-learn

COPY --from=build /app/wrapper/build/NUTC-client /bin/NUTC-wrapper

RUN chmod +x /bin/NUTC-wrapper

CMD NUTC-wrapper
