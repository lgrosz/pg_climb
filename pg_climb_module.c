#include <postgres.h>

#include "pg_climb.h"

#include <catalog/pg_type_d.h>
#include <fmgr.h>
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

