<h1 align="center">Metrics Service</h1>

<p align="center"><b>Measure the host. Send it over HTTP. Query it back.</b></p>

<p align="center">
A tiny self-hosted metrics pipeline written in C++. An agent reads CPU and RAM usage straight from <code>/proc</code>, ships it as JSON to a small HTTP API, and the API keeps everything in MySQL, ready to be queried by host and time range. The whole stack starts with a single <code>docker compose up</code>.
</p>

<p align="center">
  <img src="https://img.shields.io/badge/platform-Linux%20%7C%20Docker-2496ED?logo=docker&logoColor=white" alt="platform">
  <img src="https://img.shields.io/badge/agent-C%2B%2B17-00599C?logo=cplusplus&logoColor=white" alt="agent">
  <img src="https://img.shields.io/badge/api-C%2B%2B%20(httplib)-00599C?logo=cplusplus&logoColor=white" alt="api">
  <img src="https://img.shields.io/badge/storage-MySQL%208-4479A1?logo=mysql&logoColor=white" alt="storage">
  <img src="https://img.shields.io/badge/build-CMake-064F8C?logo=cmake&logoColor=white" alt="build">
  <img src="https://github.com/Maksimuson/metrics-service/actions/workflows/ci.yml/badge.svg" alt="CI">
</p>

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