INSERT INTO hosts (name) VALUES ('srv1');

INSERT INTO metrics (host_id, ts, cpu_percent, ram_used_mb, ram_total_mb)
VALUES (1, NOW(), 12.5, 2048, 8192);

SELECT h.name, m.ts, m.cpu_percent, m.ram_used_mb
FROM metrics m
JOIN hosts h ON m.host_id = h.id;

INSERT INTO hosts (name) VALUES ('srv1');