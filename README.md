<h1 align="center">Metrics Service</h1>

<p align="center"><b>Measure the host. Send it over HTTP. Query it back</b></p>

<p align="center">
Простой self-hosted сервис мониторинга на C++: агент читает загрузку CPU и RAM из <code>/proc</code>, отправляет её по HTTP в API, а API сохраняет данные в MySQL. Есть веб-дашборд с графиком по каждому хосту. Весь стек запускается одной командой <code>docker compose up</code>.
</p>

<p align="center">
  <img src="https://img.shields.io/badge/platform-Linux%20%7C%20Docker-2496ED?logo=docker&logoColor=white" alt="platform">
  <img src="https://img.shields.io/badge/agent-C%2B%2B17-00599C?logo=cplusplus&logoColor=white" alt="agent">
  <img src="https://img.shields.io/badge/api-C%2B%2B%20(httplib)-00599C?logo=cplusplus&logoColor=white" alt="api">
  <img src="https://img.shields.io/badge/storage-MySQL%208-4479A1?logo=mysql&logoColor=white" alt="storage">
  <img src="https://img.shields.io/badge/build-CMake-064F8C?logo=cmake&logoColor=white" alt="build">
  <img src="https://github.com/Maksimuson/metrics-service/actions/workflows/ci.yml/badge.svg" alt="CI">
</p>

<p align="center">
  <img src="docs/dashboard.gif" alt="Metrics Dashboard" width="800">
</p>

## Возможности

- Сбор метрик CPU (в процентах) и памяти (использовано / всего, МБ) каждые N секунд
- Приём измерений по HTTP (`POST /ingest`) с валидацией входных данных
- Хранение в MySQL 8 с индексом по хосту и времени
- Выборка измерений по хосту и диапазону времени
- Веб-дашборд, который открывается в браузере по адресу `http://localhost:8080/`
- Unit-тесты парсинга `/proc` и расчёта загрузки CPU, запускаются в GitHub Actions

## Архитектура

```
+---------+   POST /ingest   +---------+     SQL     +---------+
|  agent  | ---------------> |   api   | ----------> |  MySQL  |
| (C++)   |      (JSON)      | (C++)   |             |    8    |
+---------+                  +---------+             +---------+
 читает /proc                 GET /hosts
                              GET /metrics
                              GET /  (дашборд)
```

- **agent** читает `/proc/stat` и `/proc/meminfo` и отправляет значения в API.
- **api** проверяет данные, пишет их в MySQL и отдаёт обратно, а также раздаёт дашборд из папки `web/`.
- **mysql** хранит данные в двух таблицах: `hosts` (имена хостов) и `metrics` (измерения).

## Стек

C++17, CMake, cpp-httplib, nlohmann/json, MySQL Client Library, MySQL 8, Docker Compose, GoogleTest, GitHub Actions.

## Быстрый старт

Нужны Docker и Docker Compose.

```bash
git clone https://github.com/Maksimuson/Metrics-Service.git
cd Metrics-Service
cp .env.example .env      # затем задайте свои пароли в .env
docker compose up -d --build
```

Через несколько секунд агент начнёт отправлять данные:

```bash
docker compose logs -f agent
curl "localhost:8080/metrics?host=docker-agent&limit=3"
```

Дашборд открывается в браузере: <http://localhost:8080/>

Остановить стек: `docker compose down`. Чтобы также удалить сохранённые данные, добавьте `-v`.

## Конфигурация

Все настройки задаются через переменные окружения. Шаблон лежит в `.env.example`.

| Переменная | Для кого | По умолчанию | Описание |
|------------|----------|--------------|----------|
| `DB_HOST` | api | `127.0.0.1` | Адрес MySQL |
| `DB_PORT` | api | `3306` | Порт MySQL |
| `DB_USER` | api | `app` | Пользователь MySQL |
| `DB_PASS` | api | — (обязательна) | Пароль MySQL |
| `DB_NAME` | api | `metrics` | Имя базы данных |
| `PORT` | api | `8080` | Порт HTTP API |
| `API_HOST` | agent | `127.0.0.1` | Адрес API |
| `API_PORT` | agent | `8080` | Порт API |
| `INTERVAL_SEC` | agent | `5` | Интервал отправки, секунд |
| `HOST_NAME` | agent | имя системы | Имя хоста, под которым сохраняются метрики |

В `docker-compose.yml` также используются переменные `DB_ROOT_PASS` и `DB_PORT` (порт MySQL на хост-машине).

## API

| Метод | Путь | Описание |
|-------|------|----------|
| GET | `/health` | Проверка работоспособности, отвечает `ok` |
| POST | `/ingest` | Сохранить одно измерение |
| GET | `/hosts` | Список имён хостов |
| GET | `/metrics` | Измерения одного хоста, от новых к старым |
| GET | `/` | Веб-дашборд |

### POST /ingest

```bash
curl -X POST localhost:8080/ingest \
  -H "Content-Type: application/json" \
  -d '{"host":"srv1","cpu_percent":12.5,"ram_used_mb":2048,"ram_total_mb":8192}'
```

Ответы:

- `201` — измерение сохранено (`{"status":"saved"}`)
- `400` — невалидный JSON, отсутствует поле или неверный тип, либо значения вне диапазона: `cpu_percent` от 0 до 100, `ram_used_mb` не больше `ram_total_mb`, имя хоста от 1 до 255 символов
- `500` — ошибка базы данных

### GET /metrics

| Параметр | Обязательный | Описание |
|----------|--------------|----------|
| `host` | да | Имя хоста |
| `from`, `to` | нет | Диапазон времени в формате `2026-10-04 10:00:00` (UTC) |
| `limit` | нет | Максимум строк: по умолчанию 100, максимум 1000 |

Пример ответа:

```json
[
  {"ts": "2026-10-04 10:00:05", "cpu_percent": 12.5, "ram_used_mb": 2048, "ram_total_mb": 8192}
]
```

## Локальная разработка без Docker

Нужен Linux. Установка зависимостей на Ubuntu/Debian:

```bash
sudo apt install build-essential cmake libcpp-httplib-dev \
  nlohmann-json3-dev libmysqlclient-dev libgtest-dev
```

Сборка:

```bash
cmake -S api -B api/build && cmake --build api/build
cmake -S agent -B agent/build -DBUILD_TESTS=ON && cmake --build agent/build
```

Запуск API требует переменных окружения с доступом к MySQL. Например, если `.env` лежит в корне проекта:

```bash
cd api
set -a; source ../.env; set +a
./build/api
```

Запуск тестов:

```bash
./agent/build/test_collector
```

Тесты покрывают парсинг `/proc/stat` и `/proc/meminfo` и расчёт загрузки CPU.

## Структура проекта

```
metrics-service/
├── agent/                 C++ агент: сбор метрик и отправка в API
│   ├── src/               collector, sender, config, main
│   ├── tests/             unit-тесты (GoogleTest)
│   ├── CMakeLists.txt
│   └── Dockerfile
├── api/                   C++ HTTP API
│   ├── src/               маршруты (main), работа с MySQL (db), конфигурация
│   ├── web/               веб-дашборд (index.html, chart.umd.js)
│   ├── CMakeLists.txt
│   └── Dockerfile
├── docs/
│   ├── dashboard.gif      анимация дашборда для README
│   └── dashboard.png      исходный скриншот
├── db/
│   ├── init.sql           схема базы данных (создаётся при первом запуске MySQL)
│   └── playground.sql     примеры запросов для ручной проверки
├── .github/workflows/ci.yml   сборка и тесты при push и pull request
├── docker-compose.yml
└── .env.example
```

## Ограничения

- Эндпоинт `/ingest` не требует авторизации, поэтому сервис рассчитан на использование во внутренней сети. Перед открытием наружу добавьте проверку токена или закройте порт.
- Агент измеряет систему, в которой запущен. В Docker-контейнере это виртуальная машина Docker (WSL или Linux-VM), а не физический хост.
- Метки времени ставит API в момент получения данных, хранятся они в UTC.
- Для каждого запроса в API открывается новое соединение с MySQL. Для учебного проекта это приемлемо, но под нагрузкой стоит добавить пул соединений.

## Что можно доработать

- Авторизация для `/ingest` (API-ключ или токен)
- Пул соединений с MySQL
- Больше метрик: диск, сеть, загрузка по процессам
- Очистка старых данных по расписанию (retention)