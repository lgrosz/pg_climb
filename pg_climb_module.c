#include <postgres.h>

#include "c.h"
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

static inline uint8_t *
pg_grade_data(struct varlena *varlena)
{
	return (uint8_t *) VARDATA(varlena);
}

static inline size_t
pg_grade_size(struct varlena *varlena)
{
	return VARSIZE(varlena) - VARHDRSZ;
}

#define GRADEVERMSTR	"verm"
#define GRADEFONTSTR	"font"
#define GRADEYDSSTR	"yds"

#define GRADEVERMTYPE	1
#define GRADEFONTTYPE	2
#define GRADEYDSTYPE	3

/*
 * The typmod is an int32_t as follows:
 * RESERVED = Top 24 bits.
 * TYPE = Bottom 8 bits.
 */
#define GRADE_TYPMOD_GET_TYPE(typmod)	((uint8_t)((typmod) & 0xFF))
#define GRADE_TYPMOD_SET_TYPE(typmod, type) \
	((typmod) = (((typmod) & ~0xFF) | ((int32_t)(type) & 0xFF)))

static inline int
grade_typmod_type_from_string(const char *s, uint8_t *type)
{
	if (strcasecmp(GRADEVERMSTR, s) == 0) {
		*type = GRADEVERMTYPE;
	} else if (strcasecmp(GRADEFONTSTR, s) == 0) {
		*type = GRADEFONTTYPE;
	} else if (strcasecmp(GRADEYDSSTR, s) == 0) {
		*type = GRADEYDSTYPE;
	} else {
		return 1;
	}

	return 0;
}

static inline const char *
grade_typmod_type_name(uint8_t type)
{
	switch (type) {
		case GRADEVERMTYPE:
			return GRADEVERMSTR;
		case GRADEFONTTYPE:
			return GRADEFONTSTR;
		case GRADEYDSTYPE:
			return GRADEYDSSTR;
	}

	return NULL;
}

PG_FUNCTION_INFO_V1(GRADE_in);

Datum
GRADE_in(PG_FUNCTION_ARGS)
{
	Grade	grade;
	char	*input = PG_GETARG_CSTRING(0);
	struct varlena	*varlena;
	uint8_t	*buf;
	size_t	size;

	if (input[0] == '\0') {
		ereport(ERROR,(errmsg("parse error - invalid grade")));
		PG_RETURN_NULL();
	}

	if (grade_from_string(&grade, input, ANYTYPE) != 0) {
		ereport(ERROR,(errmsg("parse error - invalid grade")));
		PG_RETURN_NULL();
	}

	size = grade_serialize(&grade, NULL, 0);

	varlena = palloc(size + VARHDRSZ);
	SET_VARSIZE(varlena, size + VARHDRSZ);

	buf = (uint8_t *) VARDATA(varlena);
	grade_serialize(&grade, buf, size);

	PG_RETURN_POINTER(varlena);
}

PG_FUNCTION_INFO_V1(GRADE_out);

Datum
GRADE_out(PG_FUNCTION_ARGS)
{
	struct varlena *varlena = PG_GETARG_VARLENA_P(0);
	uint8_t *data = pg_grade_data(varlena);
	size_t len = pg_grade_size(varlena);

	Grade grade;
	size_t consumed;
	char raw[16];

	if (grade_deserialize(&grade, data, len, &consumed) != 0)
		ereport(ERROR, (errmsg("Failed to deserialize grade")));

	if (grade_format(&grade, raw, sizeof(raw)) < 0)
		ereport(ERROR, (errmsg("Failed to stringify grade")));

	PG_RETURN_CSTRING(pstrdup(raw));
}

PG_FUNCTION_INFO_V1(GRADE_typmod_in);

Datum
GRADE_typmod_in(PG_FUNCTION_ARGS)
{
	ArrayType	*arr = (ArrayType *) DatumGetPointer(PG_GETARG_DATUM(0));
	Datum	*values;
	const char	*str;
	int	i;
	int	size;
	uint8_t	type;
	int32_t	typmod = 0;

	deconstruct_array(arr, CSTRINGOID, -2, false, 'c', &values, NULL, &size);

	if (size != 1) {
		ereport(ERROR,
				(errcode(ERRCODE_DATA_EXCEPTION),
				 errmsg("typmod array must contain exactly one value")));
		PG_RETURN_INT32(0);
	}

	for (i = 0; i < size; i++) {
		if (i == 0) { /* TYPE */
			str = DatumGetCString(values[i]);

			if (grade_typmod_type_from_string(str, &type) == 0) {
				GRADE_TYPMOD_SET_TYPE(typmod, type);
			} else {
				ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("parameter value not a valid typmod")));
			}
		}
		/* future typmod fields handled here */
	}

	pfree(values);

	PG_RETURN_INT32(typmod);
}

PG_FUNCTION_INFO_V1(GRADE_typmod_out);

Datum
GRADE_typmod_out(PG_FUNCTION_ARGS)
{
	StringInfoData	si;
	int32_t	typmod = PG_GETARG_INT32(0);
	int8_t	type = GRADE_TYPMOD_GET_TYPE(typmod);

	if (!type || typmod < 0) {
		PG_RETURN_CSTRING(pstrdup(""));
	}

	initStringInfo(&si);
	appendStringInfoChar(&si, '(');
	appendStringInfoString(&si, grade_typmod_type_name(GRADE_TYPMOD_GET_TYPE(typmod)));
	appendStringInfoChar(&si, ')');

	PG_RETURN_CSTRING(si.data);
}

PG_FUNCTION_INFO_V1(GRADE_enforce_typmod);

Datum
GRADE_enforce_typmod(PG_FUNCTION_ARGS)
{
	struct varlena *varlena = PG_GETARG_VARLENA_P(0);
	int32_t typmod = PG_GETARG_INT32(1);

	Grade grade;
	size_t consumed;

	if (grade_deserialize(&grade, pg_grade_data(varlena), pg_grade_size(varlena), &consumed) != 0)
		ereport(ERROR, errmsg("Failed to deserialize grade"));

	if (typmod != grade.type)
		ereport(ERROR, errmsg("typmod mismatched"));

	PG_RETURN_POINTER(varlena);
}

static inline int
pg_grade_cmp(struct varlena *a, struct varlena* b)
{
	Grade g1, g2;
	size_t c1, c2;
	uint8_t *d1, *d2;
	size_t l1, l2;

	d1 = pg_grade_data(a);
	d2 = pg_grade_data(b);

	l1 = pg_grade_size(a);
	l2 = pg_grade_size(b);

	if (grade_deserialize(&g1, d1, l1, &c1) != 0)
		ereport(ERROR, errmsg("Failed to deserialize grade argument 1"));

	if (grade_deserialize(&g2, d2, l2, &c2) != 0)
		ereport(ERROR, errmsg("Failed to deserialize grade argument 2"));

	return grade_cmp(&g1, &g2);
}

PG_FUNCTION_INFO_V1(GRADE_lt);

Datum
GRADE_lt(PG_FUNCTION_ARGS)
{
	struct varlena *a = PG_GETARG_VARLENA_P(0);
	struct varlena *b = PG_GETARG_VARLENA_P(1);

	PG_RETURN_BOOL(pg_grade_cmp(a, b) < 0);
}

PG_FUNCTION_INFO_V1(GRADE_le);

Datum
GRADE_le(PG_FUNCTION_ARGS)
{
	struct varlena *a = PG_GETARG_VARLENA_P(0);
	struct varlena *b = PG_GETARG_VARLENA_P(1);

	PG_RETURN_BOOL(pg_grade_cmp(a, b) <= 0);
}

PG_FUNCTION_INFO_V1(GRADE_eq);

Datum
GRADE_eq(PG_FUNCTION_ARGS)
{
	struct varlena *a = PG_GETARG_VARLENA_P(0);
	struct varlena *b = PG_GETARG_VARLENA_P(1);

	PG_RETURN_BOOL(pg_grade_cmp(a, b) == 0);
}

PG_FUNCTION_INFO_V1(GRADE_neq);

Datum
GRADE_neq(PG_FUNCTION_ARGS)
{
	struct varlena *a = PG_GETARG_VARLENA_P(0);
	struct varlena *b = PG_GETARG_VARLENA_P(1);

	PG_RETURN_BOOL(pg_grade_cmp(a, b) != 0);
}

PG_FUNCTION_INFO_V1(GRADE_ge);

Datum
GRADE_ge(PG_FUNCTION_ARGS)
{
	struct varlena *a = PG_GETARG_VARLENA_P(0);
	struct varlena *b = PG_GETARG_VARLENA_P(1);

	PG_RETURN_BOOL(pg_grade_cmp(a, b) >= 0);
}

PG_FUNCTION_INFO_V1(GRADE_gt);

Datum
GRADE_gt(PG_FUNCTION_ARGS)
{
	struct varlena *a = PG_GETARG_VARLENA_P(0);
	struct varlena *b = PG_GETARG_VARLENA_P(1);

	PG_RETURN_BOOL(pg_grade_cmp(a, b) > 0);
}

PG_FUNCTION_INFO_V1(GRADE_cmp);

Datum
GRADE_cmp(PG_FUNCTION_ARGS)
{
	struct varlena *a = PG_GETARG_VARLENA_P(0);
	struct varlena *b = PG_GETARG_VARLENA_P(1);

	PG_RETURN_INT32(pg_grade_cmp(a, b));
}

static const char *
grade_type_string(uint32_t type)
{
	switch (type) {
	case VERMTYPE:
		return GRADEVERMSTR;
	case FONTTYPE:
		return GRADEFONTSTR;
	case YDSTYPE:
		return GRADEYDSSTR;
	}

	return NULL;
}

PG_FUNCTION_INFO_V1(GRADE_type);

Datum
GRADE_type(PG_FUNCTION_ARGS)
{
	struct varlena *varlena = PG_GETARG_VARLENA_P(0);
	uint8_t *data = pg_grade_data(varlena);
	size_t len = pg_grade_size(varlena);
	const char *type_str;
	text *type_text;

	Grade grade;
	size_t consumed;

	if (grade_deserialize(&grade, data, len, &consumed) != 0)
		ereport(ERROR, (errmsg("Failed to deserialize grade")));

	type_str = grade_type_string(grade.type);

	type_text = cstring_to_text(type_str);
	PG_RETURN_TEXT_P(type_text);
}
