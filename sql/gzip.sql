CREATE EXTENSION gzip;

SELECT 1 AS test, gzip('this is my this is my this is my this is my text'::bytea) AS gzip_bytea;
SELECT 2 AS test, gzip('this is my this is my this is my this is my text'::text) AS gzip_text;
SELECT 3 AS test, gzip(repeat('this is my ', 10000)) AS gzip_long;
SELECT 4 AS test, encode(gunzip(gzip('this is my this is my this is my this is my text')), 'escape') AS gzip_roundtrip;

WITH strs AS (
	SELECT repeat('pack my box with five dozen liquor jugs ', generate_series(0, 1000)) AS str
)
SELECT 5 AS test, sum((str = encode(gunzip(gzip(str)), 'escape'))::integer) AS gzip_sizes
FROM strs;

SELECT 6 AS test, gzip(NULL) AS gzip_null;
SELECT 7 AS test, gzip('') AS gzip_empty;

SELECT 8 AS test, gunzip(NULL) AS gunzip_null;
SELECT 9 AS test, gunzip('') AS gunzip_empty;
SELECT 10 AS test, gunzip('not a gzip bytea'::bytea) AS gunzip_invalid;

