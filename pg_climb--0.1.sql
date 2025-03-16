\echo Use "CREATE EXTENSION pg_climb" to load this file. \quit

CREATE TYPE grade;

-------------------------------------------------------------------
--  GRADE TYPE (grade)
-------------------------------------------------------------------
CREATE OR REPLACE FUNCTION grade_in(cstring)
	RETURNS grade
	AS 'MODULE_PATHNAME','GRADE_in'
	LANGUAGE 'c' IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION grade_out(grade)
	RETURNS cstring
	AS 'MODULE_PATHNAME','GRADE_out'
	LANGUAGE 'c' IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION grade_typmod_in(cstring[])
	RETURNS integer
	AS 'MODULE_PATHNAME', 'GRADE_typmod_in'
	LANGUAGE 'c' IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE grade (
	input = grade_in,
	output = grade_out,
	typmod_in = grade_typmod_in
);
