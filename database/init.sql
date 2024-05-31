CREATE TABLE temperature (
    id SERIAL PRIMARY KEY,
    reported_value FLOAT NOT NULL,
    reported_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE fall_detection (
    id SERIAL PRIMARY KEY,
    reported_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE panic_alert (
    id SERIAL PRIMARY KEY,
    reported_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE spo2 (
    id SERIAL PRIMARY KEY,
    reported_value FLOAT NOT NULL,
    reported_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE bpm (
    id SERIAL PRIMARY KEY,
    reported_value INT NOT NULL,
    reported_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

INSERT INTO spo2 (reported_value, reported_at) VALUES
(98.5, NOW() - INTERVAL '10 minutes'),
(99.0, NOW() - INTERVAL '9 minutes'),
(97.8, NOW() - INTERVAL '8 minutes'),
(98.3, NOW() - INTERVAL '7 minutes'),
(99.2, NOW() - INTERVAL '6 minutes'),
(97.5, NOW() - INTERVAL '5 minutes'),
(98.7, NOW() - INTERVAL '4 minutes'),
(99.1, NOW() - INTERVAL '3 minutes'),
(97.9, NOW() - INTERVAL '2 minutes'),
(98.6, NOW() - INTERVAL '1 minute');

INSERT INTO bpm (reported_value, reported_at) VALUES
(70, NOW() - INTERVAL '10 minutes'),
(75, NOW() - INTERVAL '9 minutes'),
(80, NOW() - INTERVAL '8 minutes'),
(72, NOW() - INTERVAL '7 minutes'),
(77, NOW() - INTERVAL '6 minutes'),
(82, NOW() - INTERVAL '5 minutes'),
(74, NOW() - INTERVAL '4 minutes'),
(78, NOW() - INTERVAL '3 minutes'),
(84, NOW() - INTERVAL '2 minutes'),
(76, NOW() - INTERVAL '1 minute');

INSERT INTO temperature (reported_value, reported_at) VALUES
(36.5, NOW() - INTERVAL '10 minutes'),
(36.7, NOW() - INTERVAL '9 minutes'),
(37.0, NOW() - INTERVAL '8 minutes'),
(36.4, NOW() - INTERVAL '7 minutes'),
(36.8, NOW() - INTERVAL '6 minutes'),
(37.2, NOW() - INTERVAL '5 minutes'),
(36.6, NOW() - INTERVAL '4 minutes'),
(36.9, NOW() - INTERVAL '3 minutes'),
(37.1, NOW() - INTERVAL '2 minutes'),
(36.3, NOW() - INTERVAL '1 minute');

-- Inserções na tabela panic_alert com timestamps variados
INSERT INTO panic_alert (reported_at) VALUES (DEFAULT);
INSERT INTO panic_alert (reported_at) VALUES (DEFAULT);
INSERT INTO panic_alert (reported_at) VALUES (DEFAULT);
INSERT INTO panic_alert (reported_at) VALUES (DEFAULT);
INSERT INTO panic_alert (reported_at) VALUES (DEFAULT);
INSERT INTO panic_alert (reported_at) VALUES (DEFAULT);
INSERT INTO panic_alert (reported_at) VALUES (DEFAULT);
INSERT INTO panic_alert (reported_at) VALUES (DEFAULT);
INSERT INTO panic_alert (reported_at) VALUES (DEFAULT);
INSERT INTO panic_alert (reported_at) VALUES (DEFAULT);

-- Inserções na tabela fall_detection com timestamps variados
INSERT INTO fall_detection (reported_at) VALUES (DEFAULT);
INSERT INTO fall_detection (reported_at) VALUES (DEFAULT);
INSERT INTO fall_detection (reported_at) VALUES (DEFAULT);
INSERT INTO fall_detection (reported_at) VALUES (DEFAULT);
INSERT INTO fall_detection (reported_at) VALUES (DEFAULT);
INSERT INTO fall_detection (reported_at) VALUES (DEFAULT);
INSERT INTO fall_detection (reported_at) VALUES (DEFAULT);
INSERT INTO fall_detection (reported_at) VALUES (DEFAULT);
INSERT INTO fall_detection (reported_at) VALUES (DEFAULT);
INSERT INTO fall_detection (reported_at) VALUES (DEFAULT);
