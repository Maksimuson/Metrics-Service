# Metrics Service

![CI](https://github.com/Maksimuson/metrics-service/actions/workflows/ci.yml/badge.svg)

A small system for collecting host metrics (CPU and RAM usage): a C++ agent measures them and sends them over HTTP to a C++ API, which stores them in MySQL.

## Architecture

```
+---------+   POST /ingest   +---------+    SQL     +---------+
|  agent  | ---------------> |   api   | ---------> |  MySQL  |
| (C++)   |      (JSON)      | (C++)   |            |         |
+---------+                  +---------+            +---------+
 reads /proc                  GET /hosts
                              GET /metrics
```

- **agent** reads `/proc/stat` and `/proc/meminfo` every few seconds and sends the values to the API
- **api** validates incoming data, stores it in MySQL and serves it back
- **mysql** keeps two tables: `hosts` and `metrics`

## Tech stack

C++17, CMake, cpp-httplib, nlohmann/json, MySQL client library, MySQL 8, Docker Compose, GoogleTest, GitHub Actions.

## Quick start

Requires Docker with Docker Compose.

```
git clone https://github.com/YOUR_USERNAME/metrics-service.git
cd metrics-service
cp .env.example .env      # then set your own passwords in .env
docker compose up -d --build
```

After a few seconds the agent starts sending metrics:

```
docker compose logs -f agent
curl "localhost:8080/metrics?host=docker-agent&limit=3"
```

Stop everything with `docker compose down` (add `-v` to also delete the stored data).

## API

| Method | Path | Description |
|--------|------|-------------|
| GET | `/health` | Liveness check, returns `ok` |
| POST | `/ingest` | Store one measurement |
| GET | `/hosts` | List of known host names |
| GET | `/metrics` | Measurements of one host, newest first |

### POST /ingest

```
curl -X POST localhost:8080/ingest \
  -H "Content-Type: application/json" \
  -d '{"host":"srv1","cpu_percent":12.5,"ram_used_mb":2048,"ram_total_mb":8192}'
```

Returns `201` on success and `400` if the JSON is invalid, a field is missing or a value is out of range (CPU must be 0-100, used RAM must not exceed total RAM).

### GET /metrics

| Parameter | Required | Description |
|-----------|----------|-------------|
| `host` | yes | Host name |
| `from`, `to` | no | Time range, format `2026-10-04 10:00:00` (UTC) |
| `limit` | no | Max rows, default 100, maximum 1000 |

## Configuration

All settings come from environment variables (see `.env.example`).

| Variable | Used by | Default |
|----------|---------|---------|
| `DB_HOST`, `DB_PORT`, `DB_USER`, `DB_PASS`, `DB_NAME` | api | `127.0.0.1`, `3306`, `app`, none (required), `metrics` |
| `PORT` | api | `8080` |
| `API_HOST`, `API_PORT` | agent | `127.0.0.1`, `8080` |
| `INTERVAL_SEC` | agent | `5` |
| `HOST_NAME` | agent | system host name |

## Development

Build and run without Docker (Linux):

```
sudo apt install build-essential cmake libcpp-httplib-dev \
  nlohmann-json3-dev libmysqlclient-dev libgtest-dev

cmake -S api -B api/build && cmake --build api/build
cmake -S agent -B agent/build -DBUILD_TESTS=ON && cmake --build agent/build
```

Run the unit tests:

```
./agent/build/test_collector
```

Tests cover the parsing of `/proc/stat` and `/proc/meminfo` and the CPU usage calculation. The same build and tests run on every push in GitHub Actions.

## Project structure

```
metrics-service/
├── agent/          C++ agent (collector, sender, tests)
├── api/            C++ HTTP API (routes, MySQL access)
├── db/init.sql     Database schema
├── docker-compose.yml
├── .env.example
└── .github/workflows/ci.yml
```

## Notes

- The agent measures the system it runs in. Inside a container this is the Docker virtual machine, not necessarily the physical host.
- Timestamps are set by the API when the data is received and stored in UTC.