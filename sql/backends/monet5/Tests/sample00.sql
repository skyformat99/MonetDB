
START TRANSACTION;

CREATE TABLE vals(w DOUBLE, value INTEGER);
INSERT INTO vals VALUES (1, 100), (0, 50);

SELECT * FROM vals SAMPLE 1 USING WEIGHTS w;
SELECT * FROM vals SAMPLE 0.5 USING WEIGHTS w;
SELECT * FROM vals SAMPLE 0.5 USING WEIGHTS (1 - w);

ROLLBACK;

# different types of weights
START TRANSACTION;

CREATE TABLE vals(w DECIMAL(10,3), value INTEGER);
INSERT INTO vals VALUES (10.77, 100), (0, 50);

SELECT * FROM vals SAMPLE 1 USING WEIGHTS w;

ROLLBACK;
