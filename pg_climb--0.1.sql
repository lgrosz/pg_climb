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

CREATE OR REPLACE FUNCTION grade_typmod_out(integer)
	RETURNS cstring
	AS 'MODULE_PATHNAME', 'GRADE_typmod_out'
	LANGUAGE 'c' IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE grade (
	input = grade_in,
	output = grade_out,
	typmod_in = grade_typmod_in,
	typmod_out = grade_typmod_out
);

CREATE OR REPLACE FUNCTION grade(grade, integer, boolean)
	RETURNS grade
	AS 'MODULE_PATHNAME','GRADE_enforce_typmod'
	LANGUAGE 'c' IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (grade AS grade) WITH FUNCTION grade(grade, integer, boolean) AS IMPLICIT;
