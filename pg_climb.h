#ifndef PG_CLIMB_H

#include <postgres.h>

#include <fmgr.h>
#include <stddef.h>
#include <stdint.h>

// Grade Types - these are defines to avoid confusion about what type the
// compiler decides an enum to be
#define ANYTYPE	0
#define VERMTYPE	1

// Data Structures
typedef struct {
	void *data;
	uint32_t type;
} Grade;

typedef struct {
	uint8_t *value;
	uint32_t type; /* GRADE_TYPE_VERM */
} Verm;


// This is a serialized grade. Some header data and flags can be added to the
// structure to determine how it should be interpreted, but for now it's just
// data. Here is how the data should be formatted.
//
// <verm-type>
// [uint8_t]
typedef struct {
	char data[1];
} SerializedGrade;

// Type Functions
const char *grade_type_name(uint32_t type);

// Verm Functions
Verm *verm_create(uint8_t initial_value);
void verm_free(Verm *verm);
uint8_t verm_get_value(const Verm *verm);
void verm_set_value(Verm *verm, uint8_t value);
int verm_parse(Verm *verm, const char *str);
Verm *verm_from_string(const char *str);
char *verm_format(const Verm *verm);

// Grade Functions
Grade *grade_from_string(const char *str, uint32_t type_hint);
void grade_free(Grade *grade);
char *grade_to_string(Grade *grade);

// Serialization Functions
size_t serialized_grade_size_from_verm(const Verm *verm);
SerializedGrade *serialized_grade_from_verm(const Verm *verm, size_t *size);
SerializedGrade *serialized_grade_from_grade(const Grade *grade, size_t *size);
Grade *grade_from_serialized(const SerializedGrade *serialized);

// Serialization Buffer Functions
Grade *grade_from_serialized_grade_data(uint8_t *buf);
Verm *verm_from_serialized_grade_data(const uint8_t *buf, size_t *size);
uint32_t serialized_grade_data_read_uint32_t(const uint8_t *buf);
uint8_t serialized_grade_data_read_uint8_t(const uint8_t *data);
size_t serialized_grade_buffer_write_verm(const Verm *verm, uint8_t *buf);

// PostgreSQL Function Interfaces
Datum GRADE_in(PG_FUNCTION_ARGS);
Datum GRADE_out(PG_FUNCTION_ARGS);

#endif
