#ifndef PG_CLIMB_H
#define PG_CLIMB_H

#include <stddef.h>
#include <stdint.h>

// Grade Types - these are defines to avoid confusion about what type the
// compiler decides an enum to be
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
int grade_parse(Grade *, const char *);
int grade_format(const Grade *grade, char *, size_t);
int grade_cmp(const Grade *g1, const Grade *g2);
Verm *grade_as_verm(Grade *g);
Font *grade_as_font(Grade *g);
Yds *grade_as_yds(Grade *g);

// ## Serialization
//
// Below describes how different grades are serialized.
//
// ### Verm grade
// ```
// VERMTYPE
// [uint8_t]
// ```
//
// ### Font grade
// ```
// FONTTYPE
// [uint8_t]
// ```
//
// ### YDS grade
// ```
// YDSTYPE
// [uint8_t]
// ```

size_t grade_serialize(const Grade *, uint8_t *, size_t);
int grade_deserialize(Grade *, const uint8_t *, size_t, size_t *);

#endif
