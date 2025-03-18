#include "pg_climb.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *grade_type_name(uint32_t type)
{
	switch (type) {
		case VERMTYPE:
			return "V-Scale";
		case FONTTYPE:
			return "Font-Scale";
		case YDSTYPE:
			return "Yosemite Decimal System";
		default:
			return "Unknown";
	}
}

uint32_t grade_type_from_typmod(const char *str)
{
	if (strcasecmp("verm", str) == 0) {
		return VERMTYPE;
	} else if (strcasecmp("font", str) == 0) {
		return FONTTYPE;
	} else if (strcasecmp("yds", str) == 0) {
		return YDSTYPE;
	} else {
		return ANYTYPE;
	}
}

int typmod_string(char **str, int32_t typmod)
{
	int ret;

	switch (typmod) {
		case VERMTYPE:
			*str = strdup("verm");
			ret = 0;
			break;
		case FONTTYPE:
			*str = strdup("font");
			ret = 0;
			break;
		case YDSTYPE:
			*str = strdup("yds");
			ret = 0;
			break;
		default:
			ret = 1;
	}

	return ret;
}

Verm *verm_create(uint8_t initial_value)
{
	Verm *verm;
	uint8_t *value;

	verm = malloc(sizeof(Verm));
	value = malloc(sizeof(uint8_t));

	memcpy(value, &initial_value, sizeof(uint8_t));
	verm->value = value;
	verm->type = VERMTYPE;

	return verm;
}

void verm_free(Verm *verm)
{
	free(verm->value);
	free(verm);
}

uint8_t verm_get_value(const Verm *verm)
{
	return *verm->value;
}

void verm_set_value(Verm *verm, uint8_t value)
{
	memcpy(verm->value, &value, sizeof(uint8_t));
}

int verm_parse(Verm *verm, const char *str)
{
	int	value;

	if (str == NULL || verm == NULL)
		return 1;

	assert(verm->value);

	if (strlen(str) < 2)
		return 1;

	if (strncasecmp(str, "v", 1) != 0)
		return 1;

	errno = 0;
	value = strtol(str + 1, NULL, 10);

	if (errno == ERANGE || value < 0 || value > 255)
		return 1;

	memcpy(verm->value, &value, sizeof(uint8_t));
	return 0;
}

Verm *verm_from_string(const char *str)
{
	Verm *verm = verm_create(0);

	if (verm_parse(verm, str) != 0) {
		verm_free(verm);
		verm = NULL;
	}

	return verm;
}

char *verm_format(const Verm *verm)
{
	char	*str;

	if (verm == NULL)
		return NULL;

	str = malloc(5 * sizeof(char));
	snprintf(str, 5, "V%d", verm_get_value(verm));

	return str;
}

Font *font_create(uint8_t initial_value)
{
	Font *font;
	uint8_t *value;

	font = malloc(sizeof(Font));
	value = malloc(sizeof(uint8_t));

	memcpy(value, &initial_value, sizeof(uint8_t));
	font->value = value;
	font->type = FONTTYPE;

	return font;
}

void font_free(Font *font)
{
	free(font);
}

uint8_t font_get_value(const Font *font)
{
	return *font->value;
}

void font_set_value(Font *font, uint8_t value)
{
	memcpy(font->value, &value, sizeof(uint8_t));
}

static char get_first_char(const char *str, const char **endptr)
{
	if (!strlen(str))
		return '\0';

	if (endptr)
		*endptr = str + 1;
	return str[0];
}

static int calc_font_value(unsigned int n, char m, int p, uint8_t *value)
{
	char mods[] = { 'A', 'B', 'C' };

	if (!value)
		return 1;

	// ensure p is 1 or 0
	p = p ? 1 : 0;

	// TODO protect against overflows
	if (n < 6) {
		*value = 2 * (n - 1) + p;
	} else {
		int m_i = -1;
		for (int i = 0; i < 3; i++) {
			if (mods[i] == m) {
				m_i = 2 * i + p;
				break;
			}
		}

		if (m_i == -1)
			return 1;

		*value = 10 + 6 * (n - 6) + m_i;
	}

	return 0;
}

int font_parse(Font *font, const char *str)
{
	char	*endptr;
	char	m = '\0';
	int	has_plus;
	uint8_t	value;
	unsigned int	n;

	if (str == NULL || font == NULL)
		return 1;

	assert(font->value);

	if (strlen(str) < 2)
		return 1;

	if (strncmp(str, "F", 1) != 0)
		return 1;

	str += 1;

	errno = 0;
	n = strtoul(str, &endptr, 10);

	if (errno)
		return 1;

	str = endptr;

	if (n > 5)
		m = get_first_char(str, (const char **)&endptr);

	str = endptr;

	has_plus = strncmp(str, "+", 1) == 0;

	if (calc_font_value(n, m, has_plus, &value) != 0)
		return 1;

	memcpy(font->value, &value, sizeof(uint8_t));
	return 0;
}

Font *font_from_string(const char *str)
{
	Font *font = font_create(0);

	if (font_parse(font, str) != 0) {
		font_free(font);
		font = NULL;
	}

	return font;
}

char *font_format(const Font *font)
{
	char	*str;
	char	*cur;
	char	m;
	const char	mods[]= {'A', 'A', 'B', 'B', 'C', 'C'};
	int	m_i;
	int	p;
	uint8_t	n;
	uint8_t	value;

	if (font == NULL)
		return NULL;

	value = *font->value;

	if (value < 10) {
		n = (value / 2) + 1;
		p = value % 2 == 1;
	} else {
		n = 6 + (value - 10) / 6;
		m_i = (value - 10) % 6;
		m = mods[m_i];
		p = (m_i % 2 == 1);
	}

	// 6 is the largest potential strlen for a uint8_t
	str = malloc(6 * sizeof(char));

	cur = str;
	cur += snprintf(cur, 4, "F%d", n);

	if (value > 9)
		cur += snprintf(cur, 2, "%c", m);

	if (p)
		snprintf(cur, 2, "+");

	return str;
}

Yds *yds_create(uint8_t initial_value)
{
	Yds *yds;
	uint8_t *value;

	yds = malloc(sizeof(Yds));
	value = malloc(sizeof(uint8_t));

	memcpy(value, &initial_value, sizeof(uint8_t));
	yds->value = value;
	yds->type = YDSTYPE;

	return yds;
}

void yds_free(Yds *yds)
{
	free(yds->value);
	free(yds);
}

uint8_t yds_get_value(const Yds *yds)
{
	return *yds->value;
}

void yds_set_value(Yds *yds, uint8_t value)
{
	memcpy(yds->value, &value, sizeof(uint8_t));
}

static int calc_yds_value(unsigned int n, char m, uint8_t *value)
{
	char mods[] = { 'a', 'b', 'c', 'd' };

	if (!value)
		return 1;

	// TODO protect against overflows
	if (n < 10) {
		*value = n - 1;
	} else {
		int m_i = -1;
		for (int i = 0; i < 4; i++) {
			if (mods[i] == m) {
				m_i = i;
				break;
			}
		}

		if (m_i == -1)
			return 1;

		*value = 9 + (n - 10) * 4 + m_i;
	}

	return 0;
}

int yds_parse(Yds *yds, const char *str)
{
	char	*endptr;
	char	m = '\0';
	uint8_t	value;
	unsigned int	n;

	if (str == NULL || yds == NULL)
		return 1;

	assert(yds->value);

	if (strlen(str) < 3)
		return 1;

	if (strncmp(str, "5.", 2) != 0)
		return 1;

	str += 2;

	errno = 0;
	n = strtoul(str, &endptr, 10);

	if (errno)
		return 1;

	str = endptr;

	if (n > 9)
		m = get_first_char(str, (const char **)&endptr);

	str = endptr;

	if (strlen(str))
		return 1;

	if (calc_yds_value(n, m, &value) != 0)
		return 1;

	memcpy(yds->value, &value, sizeof(uint8_t));
	return 0;
}

Yds *yds_from_string(const char *str)
{
	Yds *yds = yds_create(0);

	if (yds_parse(yds, str) != 0) {
		yds_free(yds);
		yds = NULL;
	}

	return yds;
}

char *yds_format(const Yds *yds)
{
	char	*str;
	char	*cur;
	char	m;
	const char	mods[]= {'a', 'b', 'c', 'd'};
	int	m_i;
	uint8_t	n;
	uint8_t	value;

	if (yds == NULL)
		return NULL;

	value = *yds->value;

	if (value < 9) {
		n = value + 1;
	} else {
		n = 10 + (value - 9) / 4;
		m_i = (value - 9) % 4;
		m = mods[m_i];
	}

	// 6 is the largest potential strlen for a uint8_t
	str = malloc(6 * sizeof(char));

	cur = str;
	cur += snprintf(cur, 5, "5.%d", n);

	if (n > 9) // >5.9
		cur += snprintf(cur, 2, "%c", m);

	return str;
}

Grade *grade_from_string(const char *str, uint32_t type_hint)
{
	Grade	*grade;

	// TODO It'd be great to test this type-hinting
	switch (type_hint) {
		case ANYTYPE: // try all until one hits
		case VERMTYPE:
			if ((grade = (Grade*)verm_from_string(str)) != NULL || type_hint) break;
		case FONTTYPE:
			if ((grade = (Grade*)font_from_string(str)) != NULL || type_hint) break;
		case YDSTYPE:
			if ((grade = (Grade*)yds_from_string(str)) != NULL || type_hint) break;
		default:
			grade = NULL;
	}

	return grade;
}

void grade_free(Grade *grade)
{
	switch (grade->type) {
		case VERMTYPE:
			verm_free((Verm*)grade);
			break;
		case FONTTYPE:
			font_free((Font*)grade);
			break;
		case YDSTYPE:
			yds_free((Yds*)grade);
			break;
	}
}

char *grade_to_string(Grade *grade)
{
	switch (grade->type) {
		case VERMTYPE:
			return verm_format((Verm *)grade);
		case FONTTYPE:
			return font_format((Font *)grade);
		case YDSTYPE:
			return yds_format((Yds *)grade);
		default:
			return NULL;
	}
}

void serialized_grade_free(SerializedGrade *grade)
{
	free(grade);
}

static size_t size_of_uint8_grade()
{
	// type + *->value
	return sizeof(u_int32_t) + sizeof(uint8_t);
}

size_t serialized_grade_size_from_verm(void)
{
	return size_of_uint8_grade();
}

SerializedGrade *serialized_grade_from_verm(const Verm *verm, size_t *size)
{
	SerializedGrade	*grade;
	size_t	actual_size;
	size_t	expected_size;
	uint8_t	*ptr;

	expected_size = serialized_grade_size_from_verm();
	ptr = malloc(expected_size);
	grade = (SerializedGrade *)ptr;

	// TODO here is where flags could be added to ptr

	ptr += serialized_grade_buffer_write_verm(verm, ptr);

	actual_size = ptr - (uint8_t*)grade;

	assert(actual_size == expected_size);

	if (size)
		*size = expected_size;

        return grade;
}

SerializedGrade *serialized_grade_from_font(const Font *font, size_t *size)
{
	SerializedGrade	*grade;
	size_t	actual_size;
	size_t	expected_size;
	uint8_t	*ptr;

	expected_size = serialized_grade_size_from_font();
	ptr = malloc(expected_size);
	grade = (SerializedGrade *)ptr;

	// TODO here is where flags could be added to ptr

	ptr += serialized_grade_buffer_write_font(font, ptr);

	actual_size = ptr - (uint8_t*)grade;

	assert(actual_size == expected_size);

	if (size)
		*size = expected_size;

        return grade;
}

size_t serialized_grade_size_from_font()
{
	return size_of_uint8_grade();
}

size_t serialized_grade_size_from_yds(void)
{
	return size_of_uint8_grade();
}

SerializedGrade *serialized_grade_from_yds(const Yds *yds, size_t *size)
{
	SerializedGrade	*grade;
	size_t	actual_size;
	size_t	expected_size;
	uint8_t	*ptr;

	expected_size = serialized_grade_size_from_yds();
	ptr = malloc(expected_size);
	grade = (SerializedGrade *)ptr;

	// TODO here is where flags could be added to ptr

	ptr += serialized_grade_buffer_write_yds(yds, ptr);

	actual_size = ptr - (uint8_t*)grade;

	assert(actual_size == expected_size);

	if (size)
		*size = expected_size;

        return grade;
}

Grade *grade_from_serialized(const SerializedGrade *serialized)
{
	return grade_from_serialized_grade_data((uint8_t *)serialized->data);
}

SerializedGrade *serialized_grade_from_grade(const Grade *grade, size_t *size)
{
	switch (grade->type) {
		case VERMTYPE:
			return serialized_grade_from_verm((Verm *)grade, size);
		case FONTTYPE:
			return serialized_grade_from_font((Font *)grade, size);
		case YDSTYPE:
			return serialized_grade_from_yds((Yds *)grade, size);
		default:
			return NULL;
	}
}

Grade *grade_from_serialized_grade_data(uint8_t *buf)
{
	uint32_t type;

	type = serialized_grade_data_read_uint32_t(buf);

	switch (type) {
		case VERMTYPE:
			return (Grade *)verm_from_serialized_grade_data(buf, NULL);
		case FONTTYPE:
			return (Grade *)font_from_serialized_grade_data(buf, NULL);
		case YDSTYPE:
			return (Grade *)yds_from_serialized_grade_data(buf, NULL);
		default:
			return NULL;
	}
}

Verm *verm_from_serialized_grade_data(const uint8_t *buf, size_t *size)
{
	const uint8_t *loc;
	Verm *verm;

	verm = verm_create(0);
	verm->type = VERMTYPE;

	loc = buf;
	loc += sizeof(uint32_t); // skip type
	verm_set_value(verm, serialized_grade_data_read_uint8_t(loc));
	loc += sizeof(uint8_t);

	if (size)
		*size = loc - buf;

	return verm;
}

Font *font_from_serialized_grade_data(const uint8_t *buf, size_t *size)
{
	const uint8_t *loc;
	Font *font;

	font = font_create(0);
	font->type = FONTTYPE;

	loc = buf;
	loc += sizeof(uint32_t); // skip type
	font_set_value(font, serialized_grade_data_read_uint8_t(loc));
	loc += sizeof(uint8_t);

	if (size)
		*size = loc - buf;

	return font;
}

Yds *yds_from_serialized_grade_data(const uint8_t *buf, size_t *size)
{
	const uint8_t *loc;
	Yds *yds;

	yds = yds_create(0);
	yds->type = YDSTYPE;

	loc = buf;
	loc += sizeof(uint32_t); // skip type
	yds_set_value(yds, serialized_grade_data_read_uint8_t(loc));
	loc += sizeof(uint8_t);

	if (size)
		*size = loc - buf;

	return yds;
}

uint32_t serialized_grade_data_read_uint32_t(const uint8_t *buf)
{
	return *((uint32_t*)buf);
}

uint8_t serialized_grade_data_read_uint8_t(const uint8_t *data)
{
	return *((uint8_t*)data);
}

static size_t buffer_write_uint8_t(uint8_t *buf, uint8_t data)
{
	memcpy(buf, &data, sizeof(data));
	return sizeof(data);
}

static size_t buffer_write_uint32_t(uint8_t *buf, uint32_t data)
{
	memcpy(buf, &data, sizeof(data));
	return sizeof(data);
}

static size_t buffer_write_uint8_grade(uint8_t *buf, uint32_t type, uint8_t value)
{
	uint8_t *loc = buf;

	loc += buffer_write_uint32_t(loc, type);
	loc += buffer_write_uint8_t(loc, value);

	return loc - buf;
}

size_t serialized_grade_buffer_write_verm(const Verm *verm, uint8_t *buf)
{
	return buffer_write_uint8_grade(buf, VERMTYPE, verm_get_value(verm));
}

size_t serialized_grade_buffer_write_font(const Font *font, uint8_t *buf)
{
	return buffer_write_uint8_grade(buf, FONTTYPE, font_get_value(font));
}

size_t serialized_grade_buffer_write_yds(const Yds *yds, uint8_t *buf)
{
	return buffer_write_uint8_grade(buf, YDSTYPE, yds_get_value(yds));
}
