#ifndef PG_CLIMB_H
#define PG_CLIMB_H

#include <stddef.h>
#include <stdint.h>

// Grade Types - these are defines to avoid confusion about what type the
// compiler decides an enum to be
#define ANYTYPE	0
#define VERMTYPE	1
#define FONTTYPE	2
#define YDSTYPE	3

// Data Structures
typedef struct {
	uint8_t value;
} Verm;

typedef struct {
	uint8_t value;
} Font;

typedef struct {
	uint8_t value;
} Yds;

typedef struct {
    uint32_t type;

    union {
        Verm verm;
        Font font;
        Yds yds;
    } as;
} Grade;

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
int verm_cmp(const Verm *v1, const Verm *v2);
uint8_t verm_get_value(const Verm *verm);
void verm_set_value(Verm *verm, uint8_t value);
int verm_parse(Verm *verm, const char *str);
int verm_format(const Verm *verm, char *, size_t);

// Font Functions
int font_cmp(const Font *f1, const Font *f2);
uint8_t font_get_value(const Font *font);
void font_set_value(Font *font, uint8_t value);
int font_parse(Font *font, const char *str);
int font_format(const Font *font, char *, size_t);

// YDS Functions
int yds_cmp(const Yds *y1, const Yds *y2);
uint8_t yds_get_value(const Yds *yds);
void yds_set_value(Yds *yds, uint8_t value);
int yds_parse(Yds *yds, const char *str);
int yds_format(const Yds *yds, char *, size_t);

// Grade Functions
Grade *grade_from_string(const char *str, uint32_t type_hint);
void grade_free(Grade *grade);
int grade_format(const Grade *grade, char *, size_t);
int grade_cmp(const Grade *g1, const Grade *g2);
Verm *grade_as_verm(Grade *g);
Font *grade_as_font(Grade *g);
Yds *grade_as_yds(Grade *g);

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
int serialized_grade_cmp(const SerializedGrade *sg1, const SerializedGrade *sg2);

// Serialization Buffer Functions
Grade *grade_from_serialized_grade_data(uint8_t *buf);
uint32_t serialized_grade_data_read_uint32_t(const uint8_t *buf);
uint8_t serialized_grade_data_read_uint8_t(const uint8_t *data);
size_t serialized_grade_buffer_write_verm(const Verm *verm, uint8_t *buf);
size_t serialized_grade_buffer_write_font(const Font *font, uint8_t *buf);
size_t serialized_grade_buffer_write_yds(const Yds *yds, uint8_t *buf);

#endif
