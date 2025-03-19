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

-------------------------------------------------------------------
-- BTREE indexes
-------------------------------------------------------------------
CREATE OR REPLACE FUNCTION grade_lt(grade1 grade, grade2 grade)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'GRADE_lt'
	LANGUAGE 'c' IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION grade_le(grade1 grade, grade2 grade)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'GRADE_le'
	LANGUAGE 'c' IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION grade_gt(grade1 grade, grade2 grade)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'GRADE_gt'
	LANGUAGE 'c' IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION grade_ge(grade1 grade, grade2 grade)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'GRADE_ge'
	LANGUAGE 'c' IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION grade_eq(grade1 grade, grade2 grade)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'GRADE_eq'
	LANGUAGE 'c' IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION grade_neq(grade1 grade, grade2 grade)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'GRADE_neq'
	LANGUAGE 'c' IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION grade_cmp(grade1 grade, grade2 grade)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'GRADE_cmp'
	LANGUAGE 'c' IMMUTABLE STRICT PARALLEL SAFE;

--
-- Sorting operators for Btree
--

CREATE OPERATOR < (
	LEFTARG = grade, RIGHTARG = grade, PROCEDURE = grade_lt,
	COMMUTATOR = '>', NEGATOR = '>=',
	RESTRICT = contsel, JOIN = contjoinsel
);

CREATE OPERATOR <= (
	LEFTARG = grade, RIGHTARG = grade, PROCEDURE = grade_le,
	COMMUTATOR = '>=', NEGATOR = '>',
	RESTRICT = contsel, JOIN = contjoinsel
);

CREATE OPERATOR = (
	LEFTARG = grade, RIGHTARG = grade, PROCEDURE = grade_eq,
	COMMUTATOR = '=', NEGATOR = '<>',
	RESTRICT = contsel, JOIN = contjoinsel, HASHES, MERGES
);

CREATE OPERATOR <> (
	LEFTARG = grade, RIGHTARG = grade, PROCEDURE = grade_neq,
	COMMUTATOR = '<>', NEGATOR = '=',
	RESTRICT = contsel, JOIN = contjoinsel
);

CREATE OPERATOR >= (
	LEFTARG = grade, RIGHTARG = grade, PROCEDURE = grade_ge,
	COMMUTATOR = '<=', NEGATOR = '<',
	RESTRICT = contsel, JOIN = contjoinsel
);

CREATE OPERATOR > (
	LEFTARG = grade, RIGHTARG = grade, PROCEDURE = grade_gt,
	COMMUTATOR = '<', NEGATOR = '<=',
	RESTRICT = contsel, JOIN = contjoinsel
);

CREATE OPERATOR CLASS btree_grade_ops
	DEFAULT FOR TYPE grade USING btree AS
	OPERATOR	1	< ,
	OPERATOR	2	<= ,
	OPERATOR	3	= ,
	OPERATOR	4	>= ,
	OPERATOR	5	> ,
	FUNCTION	1	grade_cmp (grade1 grade, grade2 grade);

CREATE OR REPLACE FUNCTION GradeType(grade)
	RETURNS text
	AS 'MODULE_PATHNAME', 'GRADE_type'
	LANGUAGE 'c' IMMUTABLE STRICT PARALLEL SAFE;
