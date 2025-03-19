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
	Grade	*grade;
	SerializedGrade	*serialized = NULL;
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

	grade = grade_from_string(input, typmod < 0 ? ANYTYPE : (uint32_t)typmod);

	if (grade) {
		serialized = serialized_grade_from_grade(grade, &size);

		// insert postgres size header
		serialized = realloc(serialized, size + VARHDRSZ);
		memmove((uint8_t*)serialized + VARHDRSZ, serialized, size);
		SET_VARSIZE(serialized, size + VARHDRSZ);

		grade_free(grade);
	} else {
		ereport(ERROR,(errmsg("parse error - invalid grade")));
		PG_RETURN_NULL();
	}

	PG_RETURN_SERGRADE_P(serialized);
}

PG_FUNCTION_INFO_V1(GRADE_out);

Datum
GRADE_out(PG_FUNCTION_ARGS)
{
	SerializedGrade	*serialized = PG_GETARG_SERGRADE_P(0);
	// TODO is this leaky?
	Grade *grade = grade_from_serialized(serialized);

	if (!grade)
		ereport(ERROR,(errmsg("Failed to deserialized grade data")));

	// TODO would it be a better API to allocate the string here and just
	// format it?
	PG_RETURN_CSTRING(grade_to_string(grade));
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
	char	*typmod_str;
	int32_t	typmod = PG_GETARG_INT32(0);

	if (typmod < 0)
		PG_RETURN_CSTRING(pstrdup(""));

	initStringInfo(&si);
	appendStringInfoChar(&si, '(');

	if (typmod_string(&typmod_str, typmod) != 0) {
		ereport(WARNING, (errmsg("failed to stringify typmod")));
		typmod_str = malloc(12); // enough
		sprintf(typmod_str, "%d", typmod);
	}

	appendStringInfoString(&si, typmod_str);
	appendStringInfoChar(&si, ')');

	free(typmod_str);

	PG_RETURN_CSTRING(si.data);
}

PG_FUNCTION_INFO_V1(GRADE_enforce_typmod);

Datum
GRADE_enforce_typmod(PG_FUNCTION_ARGS)
{
	Grade *grade;
	SerializedGrade *serialized;
	int32_t typmod;
	void *ret;

	// TODO this is a little ugly, but it gets the job done for now
	ret = PG_GETARG_SERGRADE_P(0) - VARHDRSZ;
	serialized = (SerializedGrade *)ret + VARHDRSZ;

	typmod = PG_GETARG_INT32(1);

	grade = grade_from_serialized(serialized);

	if (!grade)
		ereport(ERROR, errmsg("failed to deserialize grade"));

        // TODO there could be a useful conversion here, like converting between
        // alike grade types (e.g. verm -> font)

        if (grade && typmod != grade->type)
		ereport(ERROR, errmsg("typmod mismatched"));

	if (grade)
		grade_free(grade);

	PG_RETURN_SERGRADE_P(ret);
}

PG_FUNCTION_INFO_V1(GRADE_lt);

Datum
GRADE_lt(PG_FUNCTION_ARGS)
{
	int cmp;
	void *g1;
	void *g2;

	// TODO this is a little ugly, but it gets the job done for now
	g1 = PG_GETARG_SERGRADE_P(0) - VARHDRSZ;
	g2 = PG_GETARG_SERGRADE_P(1) - VARHDRSZ;
	cmp = serialized_grade_cmp((SerializedGrade*)g1 + VARHDRSZ, (SerializedGrade*)g2 + VARHDRSZ);

	PG_FREE_IF_COPY(g1, 0);
	PG_FREE_IF_COPY(g2, 1);
	PG_RETURN_BOOL(cmp < 0);
}

PG_FUNCTION_INFO_V1(GRADE_le);

Datum
GRADE_le(PG_FUNCTION_ARGS)
{
	int cmp;
	void *g1;
	void *g2;

	// TODO this is a little ugly, but it gets the job done for now
	g1 = PG_GETARG_SERGRADE_P(0) - VARHDRSZ;
	g2 = PG_GETARG_SERGRADE_P(1) - VARHDRSZ;
	cmp = serialized_grade_cmp((SerializedGrade*)g1 + VARHDRSZ, (SerializedGrade*)g2 + VARHDRSZ);

	PG_FREE_IF_COPY(g1, 0);
	PG_FREE_IF_COPY(g2, 1);
	PG_RETURN_BOOL(cmp <= 0);
}

PG_FUNCTION_INFO_V1(GRADE_eq);

Datum
GRADE_eq(PG_FUNCTION_ARGS)
{
	int cmp;
	void *g1;
	void *g2;

	// TODO this is a little ugly, but it gets the job done for now
	g1 = PG_GETARG_SERGRADE_P(0) - VARHDRSZ;
	g2 = PG_GETARG_SERGRADE_P(1) - VARHDRSZ;
	cmp = serialized_grade_cmp((SerializedGrade*)g1 + VARHDRSZ, (SerializedGrade*)g2 + VARHDRSZ);

	PG_FREE_IF_COPY(g1, 0);
	PG_FREE_IF_COPY(g2, 1);
	PG_RETURN_BOOL(cmp == 0);
}

PG_FUNCTION_INFO_V1(GRADE_neq);

Datum
GRADE_neq(PG_FUNCTION_ARGS)
{
	int cmp;
	void *g1;
	void *g2;

	// TODO this is a little ugly, but it gets the job done for now
	g1 = PG_GETARG_SERGRADE_P(0) - VARHDRSZ;
	g2 = PG_GETARG_SERGRADE_P(1) - VARHDRSZ;
	cmp = serialized_grade_cmp((SerializedGrade*)g1 + VARHDRSZ, (SerializedGrade*)g2 + VARHDRSZ);

	PG_FREE_IF_COPY(g1, 0);
	PG_FREE_IF_COPY(g2, 1);
	PG_RETURN_BOOL(cmp != 0);
}

PG_FUNCTION_INFO_V1(GRADE_ge);

Datum
GRADE_ge(PG_FUNCTION_ARGS)
{
	int cmp;
	void *g1;
	void *g2;

	// TODO this is a little ugly, but it gets the job done for now
	g1 = PG_GETARG_SERGRADE_P(0) - VARHDRSZ;
	g2 = PG_GETARG_SERGRADE_P(1) - VARHDRSZ;
	cmp = serialized_grade_cmp((SerializedGrade*)g1 + VARHDRSZ, (SerializedGrade*)g2 + VARHDRSZ);

	PG_FREE_IF_COPY(g1, 0);
	PG_FREE_IF_COPY(g2, 1);
	PG_RETURN_BOOL(cmp >= 0);
}

PG_FUNCTION_INFO_V1(GRADE_gt);

Datum
GRADE_gt(PG_FUNCTION_ARGS)
{
	int cmp;
	void *g1;
	void *g2;

	// TODO this is a little ugly, but it gets the job done for now
	g1 = PG_GETARG_SERGRADE_P(0) - VARHDRSZ;
	g2 = PG_GETARG_SERGRADE_P(1) - VARHDRSZ;
	cmp = serialized_grade_cmp((SerializedGrade*)g1 + VARHDRSZ, (SerializedGrade*)g2 + VARHDRSZ);

	PG_FREE_IF_COPY(g1, 0);
	PG_FREE_IF_COPY(g2, 1);
	PG_RETURN_BOOL(cmp > 0);
}

PG_FUNCTION_INFO_V1(GRADE_cmp);

Datum
GRADE_cmp(PG_FUNCTION_ARGS)
{
	int cmp;
	void *g1;
	void *g2;

	// TODO this is a little ugly, but it gets the job done for now
	g1 = PG_GETARG_SERGRADE_P(0) - VARHDRSZ;
	g2 = PG_GETARG_SERGRADE_P(1) - VARHDRSZ;
	cmp = serialized_grade_cmp((SerializedGrade*)g1 + VARHDRSZ, (SerializedGrade*)g2 + VARHDRSZ);

	PG_FREE_IF_COPY(g1, 0);
	PG_FREE_IF_COPY(g2, 1);
	PG_RETURN_INT32(cmp);
}

PG_FUNCTION_INFO_V1(GRADE_type);

Datum
GRADE_type(PG_FUNCTION_ARGS)
{
	Grade *grade;
	SerializedGrade *serialized;
	char *type_str;
	text *type_text;
	void *bytes;

	// TODO this is a little ugly, but it gets the job done for now
	bytes = PG_GETARG_SERGRADE_P(0) - VARHDRSZ;
	serialized = (SerializedGrade *)bytes + VARHDRSZ;
	grade = grade_from_serialized(serialized);

        // NOTE Grade.type _is_ typmod for valid types (for now)
        if (typmod_string(&type_str, grade->type) == 0) {
		type_text = cstring_to_text(type_str);
		free(type_str);
	} else {
		type_text = cstring_to_text("");
	}

	grade_free(grade);
	PG_FREE_IF_COPY(bytes, 0);
	PG_RETURN_TEXT_P(type_text);
}
