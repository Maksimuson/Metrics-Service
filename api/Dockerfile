FROM ubuntu:26.04

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake \
    libcpp-httplib-dev nlohmann-json3-dev libmysqlclient-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY CMakeLists.txt .
COPY src ./src
RUN cmake -S . -B build && cmake --build build

CMD ["./build/api"]
