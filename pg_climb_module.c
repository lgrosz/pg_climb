#include <postgres.h>

#include "lib/stringinfo.h"
#include "pg_climb.h"
#include "utils/builtins.h"
#include "utils/elog.h"
#include "utils/palloc.h"

#include <catalog/pg_type_d.h>
#include <fmgr.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <utils/array.h>
#include <varatt.h>

PG_MODULE_MAGIC;

static inline SerializedGrade *
DatumGetSergradeP(Datum X)
{
	return (SerializedGrade *) (DatumGetPointer(X) + VARHDRSZ);
}
static inline Datum
SergradePGetDatum(const SerializedGrade *X)
{
	return PointerGetDatum(X);
}
#define PG_GETARG_SERGRADE_P(n) DatumGetSergradeP(PG_GETARG_DATUM(n))
#define PG_RETURN_SERGRADE_P(x) return SergradePGetDatum(x)

PG_FUNCTION_INFO_V1(GRADE_in);

Datum
GRADE_in(PG_FUNCTION_ARGS)
{
	Grade	grade;
	SerializedGrade	*serialized = NULL;
	SerializedGrade	*pg_serialized = NULL;
	char	*input = PG_GETARG_CSTRING(0);
	int32_t	typmod = -1;
	size_t	size;

	if (PG_NARGS() > 2 && !PG_ARGISNULL(2)) {
		typmod = PG_GETARG_INT32(2);
	}

	if (input[0] == '\0') {
		ereport(ERROR,(errmsg("parse error - invalid grade")));
		PG_RETURN_NULL();
	}

	;

	if (grade_from_string(&grade, input, typmod < 0 ? ANYTYPE : (uint32_t)typmod) != 0) {
		ereport(ERROR,(errmsg("parse error - invalid grade")));
		PG_RETURN_NULL();
	}

	serialized = serialized_grade_from_grade(&grade, &size);

	// copy to pg context
	pg_serialized = (SerializedGrade *) palloc(size + VARHDRSZ);
	SET_VARSIZE(pg_serialized, size + VARHDRSZ);
	// TODO memmove is unecessary _if_ the memory won't overlap
	memmove((uint8_t*)pg_serialized + VARHDRSZ, serialized, size);

	free(serialized);

	PG_RETURN_SERGRADE_P(pg_serialized);
}

PG_FUNCTION_INFO_V1(GRADE_out);

Datum
GRADE_out(PG_FUNCTION_ARGS)
{
	SerializedGrade	*serialized = PG_GETARG_SERGRADE_P(0);
	Grade grade;
	char raw[16];
	char *pgstr;

	if (grade_from_serialized(&grade, serialized) != 0)
		ereport(ERROR,(errmsg("Failed to deserialized grade data")));

	if (grade_format(&grade, raw, sizeof(raw)) < 0)
		ereport(ERROR, (errmsg("Failed to stringify grade")));

	pgstr = pstrdup(raw);

	PG_RETURN_CSTRING(pgstr);
}

PG_FUNCTION_INFO_V1(GRADE_typmod_in);

Datum
GRADE_typmod_in(PG_FUNCTION_ARGS)
{
	ArrayType	*arr = (ArrayType *) DatumGetPointer(PG_GETARG_DATUM(0));
	Datum	*values;
	const char	*str;
	int	size;
	uint32_t	typmod;

	deconstruct_array(arr, CSTRINGOID, -2, false, 'c', &values, NULL, &size);

	if (size != 1) {
		ereport(ERROR,
				(errcode(ERRCODE_DATA_EXCEPTION),
				 errmsg("typmod array must contain exactly one value")));
		PG_RETURN_INT32(0);
	}

	str = DatumGetCString(values[0]);
	typmod = grade_type_from_typmod(str);

	if (typmod == ANYTYPE) {
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				 errmsg("parameter value not a valid typmod")));
		PG_RETURN_INT32(0);
	}

	PG_RETURN_INT32(typmod);
}

PG_FUNCTION_INFO_V1(GRADE_typmod_out);

Datum
GRADE_typmod_out(PG_FUNCTION_ARGS)
{
	StringInfoData	si;
	const char	*typmod_str;
	int32_t	typmod = PG_GETARG_INT32(0);

	if (typmod < 0)
		PG_RETURN_CSTRING(pstrdup(""));

	initStringInfo(&si);
	appendStringInfoChar(&si, '(');

	typmod_str = typmod_string(typmod);

	if (typmod_str) {
		appendStringInfoString(&si, typmod_str);
	} else {
		appendStringInfo(&si, "%d", typmod);
	}

	appendStringInfoChar(&si, ')');

	PG_RETURN_CSTRING(si.data);
}

PG_FUNCTION_INFO_V1(GRADE_enforce_typmod);

Datum
GRADE_enforce_typmod(PG_FUNCTION_ARGS)
{
	Grade grade;
	SerializedGrade *serialized;
	int32_t typmod;
	void *ret;

	// TODO this is a little ugly, but it gets the job done for now
	ret = ((char *)PG_GETARG_SERGRADE_P(0) - VARHDRSZ);
	serialized = (SerializedGrade *)((char *)ret + VARHDRSZ);

	typmod = PG_GETARG_INT32(1);

	;

	if (grade_from_serialized(&grade, serialized) != 0)
		ereport(ERROR, errmsg("failed to deserialize grade"));

        // TODO there could be a useful conversion here, like converting between
        // alike grade types (e.g. verm -> font), though this likely should be
        // explicitly request by the user.

        if (typmod != grade.type)
		ereport(ERROR, errmsg("typmod mismatched"));

	PG_RETURN_SERGRADE_P(ret);
}

PG_FUNCTION_INFO_V1(GRADE_lt);

Datum
GRADE_lt(PG_FUNCTION_ARGS)
{
	PG_RETURN_BOOL(serialized_grade_cmp(PG_GETARG_SERGRADE_P(0), PG_GETARG_SERGRADE_P(1)) < 0);
}

PG_FUNCTION_INFO_V1(GRADE_le);

Datum
GRADE_le(PG_FUNCTION_ARGS)
{
	PG_RETURN_BOOL(serialized_grade_cmp(PG_GETARG_SERGRADE_P(0), PG_GETARG_SERGRADE_P(1)) <= 0);
}

PG_FUNCTION_INFO_V1(GRADE_eq);

Datum
GRADE_eq(PG_FUNCTION_ARGS)
{
	PG_RETURN_BOOL(serialized_grade_cmp(PG_GETARG_SERGRADE_P(0), PG_GETARG_SERGRADE_P(1)) == 0);
}

PG_FUNCTION_INFO_V1(GRADE_neq);

Datum
GRADE_neq(PG_FUNCTION_ARGS)
{
	PG_RETURN_BOOL(serialized_grade_cmp(PG_GETARG_SERGRADE_P(0), PG_GETARG_SERGRADE_P(1)) != 0);
}

PG_FUNCTION_INFO_V1(GRADE_ge);

Datum
GRADE_ge(PG_FUNCTION_ARGS)
{
	PG_RETURN_BOOL(serialized_grade_cmp(PG_GETARG_SERGRADE_P(0), PG_GETARG_SERGRADE_P(1)) >= 0);
}

PG_FUNCTION_INFO_V1(GRADE_gt);

Datum
GRADE_gt(PG_FUNCTION_ARGS)
{
	PG_RETURN_BOOL(serialized_grade_cmp(PG_GETARG_SERGRADE_P(0), PG_GETARG_SERGRADE_P(1)) > 0);
}

PG_FUNCTION_INFO_V1(GRADE_cmp);

Datum
GRADE_cmp(PG_FUNCTION_ARGS)
{
	PG_RETURN_INT32(serialized_grade_cmp(PG_GETARG_SERGRADE_P(0), PG_GETARG_SERGRADE_P(1)));
}

PG_FUNCTION_INFO_V1(GRADE_type);

Datum
GRADE_type(PG_FUNCTION_ARGS)
{
	Grade grade;
	SerializedGrade *serialized;
	const char *type_str;
	text *type_text;
	char *bytes;

	// TODO this is a little ugly, but it gets the job done for now
	bytes = (char *)PG_GETARG_SERGRADE_P(0) - VARHDRSZ;
	serialized = (SerializedGrade *)(bytes + VARHDRSZ);

	grade_from_serialized(&grade, serialized);

        // NOTE Grade.type _is_ typmod for valid types (for now)
	type_str = typmod_string(grade.type);
        if (type_str) {
		type_text = cstring_to_text(type_str);
	} else {
		type_text = cstring_to_text("");
	}

	PG_FREE_IF_COPY(bytes, 0);
	PG_RETURN_TEXT_P(type_text);
}
