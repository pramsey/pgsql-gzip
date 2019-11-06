CREATE EXTENSION gzip;

SELECT gzip('this is my this is my this is my this is my text'::bytea) AS gzip_bytea;
SELECT gzip('this is my this is my this is my this is my text'::text) AS gzip_text;
SELECT gzip(repeat('this is my ', 10000)) AS gzip_long;
SELECT encode(gunzip(gzip('this is my this is my this is my this is my text')), 'escape') AS gzip_roundtrip;
SELECT gunzip(gzip('\x00000000000000000000'::bytea)) AS gzip_roundtrip_zero;

WITH strs AS (
	SELECT repeat('pack my box with five dozen liquor jugs ', generate_series(0, 1000)) AS str
)
SELECT sum((str = encode(gunzip(gzip(str)), 'escape'))::integer) AS gzip_sizes
FROM strs;

SELECT gzip(NULL) AS gzip_null;
SELECT gzip('') AS gzip_empty;
SELECT gzip('\x00'::bytea) AS gzip_zero;

SELECT gunzip(NULL) AS gunzip_null;
SELECT gunzip('') AS gunzip_empty;
SELECT gunzip('\x00'::bytea) AS gunzip_zero;
SELECT gunzip('\x0000'::bytea) AS gunzip_zerozero;
SELECT gunzip('not a gzip bytea'::bytea) AS gunzip_invalid;

