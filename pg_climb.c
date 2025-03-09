#include <postgres.h>

#include <fmgr.h>
#include <stdio.h>
#include <varatt.h>

// TODO
// - [ ] support many types
//   - [ ] parse typmod `(grade_type)`
//   - [ ] serialized structure for arbitrary "grade", this should be a flat allocation. It will contain...
//     - size, total for the struct (including this field), for postgres
//     - flags+ext flags? I'm not sure if these would be necessary or what
//       they'd be used for in this context, but PostGIS has them for its
//       serialize-geo data
//     - data
//       - first byte is the type
//       - the rest is the serialized data for the type

PG_MODULE_MAGIC;

// TODO expand on this idea VERM_parse, VERM_parse_result, VERM_free, etc.
struct VERM {
	uint8_t	value;
};

static inline struct VERM *
DatumGetVermP(Datum X)
{
	return (struct VERM *) DatumGetPointer(X);
}
static inline Datum
VermPGetDatum(const struct VERM *X)
{
	return PointerGetDatum(X);
}
#define PG_GETARG_VERM_P(n) DatumGetVermP(PG_GETARG_DATUM(n))
#define PG_RETURN_VERM_P(x) return VermPGetDatum(x)

static int
parse_verm(struct VERM *verm, const char *str)
{
	int	value;

	if (verm == NULL)
		return 1;

	if (strncasecmp(str, "v", 1) != 0)
		return 1;

	value = strtol(str + 1, NULL, 10);

	if (errno == ERANGE || value < 0 || value > 255)
		return 1;

	verm->value = value;
	return 0;
}

static char *
format_verm(struct VERM *verm)
{
	char	*str;

	if (verm == NULL)
		return NULL;

	str = malloc(5 * sizeof(char));
	snprintf(str, 5, "V%d", verm->value);

	return str;
}

PG_FUNCTION_INFO_V1(GRADE_in);

Datum
GRADE_in(PG_FUNCTION_ARGS)
{
	char	*input = PG_GETARG_CSTRING(0);
	struct VERM	*verm;
	unsigned int	size;

	if (input[0] == '\0')
	{
		ereport(ERROR,(errmsg("parse error - invalid grade")));
		PG_RETURN_NULL();
	}

	size = sizeof(struct VERM *);
	verm = (struct VERM *) palloc(size + VARHDRSZ);
	SET_VARSIZE(verm, size + VARHDRSZ);

	if (parse_verm(verm + VARHDRSZ, input) == 0)
		PG_RETURN_VERM_P(verm);
	else
	{
		pfree(verm);
		ereport(ERROR,(errmsg("parse error - invalid grade")));
		PG_RETURN_NULL();
	}
}

PG_FUNCTION_INFO_V1(GRADE_out);

Datum
GRADE_out(PG_FUNCTION_ARGS)
{
	struct VERM *verm = PG_GETARG_VERM_P(0);
	PG_RETURN_CSTRING(format_verm(verm + VARHDRSZ));
}

