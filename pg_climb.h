#include <stddef.h>
#include <stdint.h>

#ifndef PG_CLIMB_H

// Grade Types - these are defines to avoid confusion about what type the
// compiler decides an enum to be
#define ANYTYPE	0
#define VERMTYPE	1
#define FONTTYPE	2
#define YDSTYPE	3

// Data Structures
typedef struct {
	void *data;
	uint32_t type;
} Grade;

typedef struct {
	uint8_t *value;
	uint32_t type; /* GRADE_TYPE_VERM */
} Verm;

typedef struct {
	uint8_t *value;
	uint32_t type; /* GRADE_TYPE_FONT */
} Font;

typedef struct {
	uint8_t *value;
	uint32_t type; /* GRADE_TYPE_YDS */
} Yds;

// This is a serialized grade. Some header data and flags can be added to the
// structure to determine how it should be interpreted, but for now it's just
// data. Here is how the data should be formatted.
//
// Open-ended, discrete scales (like the V-scale and Font-scale) are just
// represented by a single integer despite appearinging like they contain more
// than one component.
//
// <verm-type>
// [uint8_t]
//
// <font-type>
// [uint8_t]
//
// <yds-type>
// [uint8_t]
typedef struct {
	char data[1];
} SerializedGrade;

// Type Functions
const char *grade_type_name(uint32_t type);
uint32_t grade_type_from_typmod(const char *);
int typmod_string(char **, int32_t typmod);

// Verm Functions
Verm *verm_create(uint8_t initial_value);
void verm_free(Verm *verm);
uint8_t verm_get_value(const Verm *verm);
void verm_set_value(Verm *verm, uint8_t value);
int verm_parse(Verm *verm, const char *str);
Verm *verm_from_string(const char *str);
char *verm_format(const Verm *verm);

// Font Functions
Font *font_create(uint8_t initial_value);
void font_free(Font *font);
uint8_t font_get_value(const Font *font);
void font_set_value(Font *font, uint8_t value);
int font_parse(Font *font, const char *str);
Font *font_from_string(const char *str);
char *font_format(const Font *font);

// YDS Functions
Yds *yds_create(uint8_t initial_value);
void yds_free(Yds *yds);
uint8_t yds_get_value(const Yds *yds);
void yds_set_value(Yds *yds, uint8_t value);
int yds_parse(Yds *yds, const char *str);
Yds *yds_from_string(const char *str);
char *yds_format(const Yds *yds);

// Grade Functions
Grade *grade_from_string(const char *str, uint32_t type_hint);
void grade_free(Grade *grade);
char *grade_to_string(Grade *grade);

// Serialization Functions
void serialized_grade_free(SerializedGrade *grade);
size_t serialized_grade_size_from_verm(void);
SerializedGrade *serialized_grade_from_verm(const Verm *verm, size_t *size);
size_t serialized_grade_size_from_font(void);
SerializedGrade *serialized_grade_from_font(const Font *font, size_t *size);
size_t serialized_grade_size_from_yds(void);
SerializedGrade *serialized_grade_from_yds(const Yds *yds, size_t *size);
SerializedGrade *serialized_grade_from_grade(const Grade *grade, size_t *size);
Grade *grade_from_serialized(const SerializedGrade *serialized);

// Serialization Buffer Functions
Grade *grade_from_serialized_grade_data(uint8_t *buf);
Verm *verm_from_serialized_grade_data(const uint8_t *buf, size_t *size);
Font *font_from_serialized_grade_data(const uint8_t *buf, size_t *size);
Yds *yds_from_serialized_grade_data(const uint8_t *buf, size_t *size);
uint32_t serialized_grade_data_read_uint32_t(const uint8_t *buf);
uint8_t serialized_grade_data_read_uint8_t(const uint8_t *data);
size_t serialized_grade_buffer_write_verm(const Verm *verm, uint8_t *buf);
size_t serialized_grade_buffer_write_font(const Font *font, uint8_t *buf);
size_t serialized_grade_buffer_write_yds(const Yds *yds, uint8_t *buf);

#endif
