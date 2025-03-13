#include "pg_climb.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *grade_type_name(uint32_t type)
{
	switch (type) {
		case VERMTYPE:
			return "V-Scale";
		default:
			return "Unknown";
	}
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

Grade *grade_from_string(const char *str, uint32_t type_hint)
{
	Grade	*grade;

	switch (type_hint) {
		case ANYTYPE: // try all until one hits
		case VERMTYPE:
			if ((grade = (Grade*)verm_from_string(str)) != NULL || type_hint) break;
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
	}
}

char *grade_to_string(Grade *grade)
{
	switch (grade->type) {
		case VERMTYPE:
			return verm_format((Verm *)grade);
		default:
			return NULL;
	}
}

void serialized_grade_free(SerializedGrade *grade)
{
	free(grade);
}

size_t serialized_grade_size_from_verm(void)
{
	size_t size = sizeof(uint32_t); // for type
	size += sizeof(uint8_t); // for verm->value content
	return size;
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

Grade *grade_from_serialized(const SerializedGrade *serialized)
{
	return grade_from_serialized_grade_data((uint8_t *)serialized->data);
}

SerializedGrade *serialized_grade_from_grade(const Grade *grade, size_t *size)
{
	SerializedGrade *serialized = NULL;

	switch (grade->type) {
		case VERMTYPE:
			serialized = serialized_grade_from_verm((Verm *)grade, size);
	}

	return serialized;

}

Grade *grade_from_serialized_grade_data(uint8_t *buf)
{
	uint32_t type;

	type = serialized_grade_data_read_uint32_t(buf);

	switch (type) {
		case VERMTYPE:
			return (Grade *)verm_from_serialized_grade_data(buf, NULL);
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

uint32_t serialized_grade_data_read_uint32_t(const uint8_t *buf)
{
	return *((uint32_t*)buf);
}

uint8_t serialized_grade_data_read_uint8_t(const uint8_t *data)
{
	return *((uint8_t*)data);
}

size_t serialized_grade_buffer_write_verm(const Verm *verm, uint8_t *buf)
{
	uint8_t *loc;
	uint32_t type = VERMTYPE;
	uint8_t value = verm_get_value(verm);

	loc = buf;

	memcpy(loc, &type, sizeof(uint32_t));
	loc += sizeof(uint32_t);
	memcpy(loc, &value, sizeof(uint8_t));
	loc += sizeof(uint8_t);

	return loc - buf;
}
