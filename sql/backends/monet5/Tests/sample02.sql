

START TRANSACTION;

CREATE TABLE vals(id DOUBLE, type INTEGER);

INSERT INTO vals VALUES (1, 100), (0, 50);

SELECT SUM(id) FROM vals SAMPLE 1 USING WEIGHTS id;

ROLLBACK;

