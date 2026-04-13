#include "pg_climb.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int verm_cmp(const Verm *v1, const Verm *v2)
{
	return v1->value - v2->value;
}

uint8_t verm_get_value(const Verm *verm)
{
	return verm->value;
}

void verm_set_value(Verm *verm, uint8_t value)
{
	verm->value = value;
}

int verm_parse(Verm *verm, const char *str)
{
	int	value;

	if (str == NULL || verm == NULL)
		return 1;

	if (strlen(str) < 2)
		return 1;

	if (strncasecmp(str, "v", 1) != 0)
		return 1;

	errno = 0;
	value = strtol(str + 1, NULL, 10);

	if (errno == ERANGE || value < 0 || value > 255)
		return 1;

	verm->value = value;
	return 0;
}

int verm_format(const Verm *verm, char *str, size_t size)
{
	if (verm == NULL || str == NULL || size == 0)
		return -1;

	return snprintf(str, size, "V%d", verm_get_value(verm));
}

int font_cmp(const Font *f1, const Font *f2)
{
	return f1->value - f2->value;
}

uint8_t font_get_value(const Font *font)
{
	return font->value;
}

void font_set_value(Font *font, uint8_t value)
{
	font->value = value;
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

	font->value = value;
	return 0;
}

int font_format(const Font *font, char *str, size_t size)
{
	char	m;
	const char	mods[] = { 'A', 'A', 'B', 'B', 'C', 'C' };
	int	m_i;
	int	p;
	uint8_t	n;
	uint8_t	value;

	if (font == NULL || str == NULL || size == 0)
		return -1;

	value = font->value;

	if (value < 10) {
		n = (value / 2) + 1;
		p = (value % 2);
		return snprintf(str, size, p ? "F%d+" : "F%d", n);
	}

	n = 6 + (value - 10) / 6;

	m_i = (value - 10) % 6;
	m = mods[m_i];
	p = (m_i % 2);

	return snprintf(str, size,
		 p ? "F%d%c+" : "F%d%c",
		 n, m);
}

int yds_cmp(const Yds *y1, const Yds *y2)
{
	return y1->value - y2->value;
}

uint8_t yds_get_value(const Yds *yds)
{
	return yds->value;
}

void yds_set_value(Yds *yds, uint8_t value)
{
	yds->value = value;
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

	yds->value = value;
	return 0;
}

int yds_format(const Yds *yds, char *str, size_t size)
{
	char	m;
	const char	mods[]= {'a', 'b', 'c', 'd'};
	int	m_i;
	uint8_t	n;
	uint8_t	value;

	if (yds == NULL || str == NULL || size == 0)
		return -1;

	value = yds->value;

	if (value < 9) {
		n = value + 1;
		return snprintf(str, size, "5.%d", n);
	} else {
		n = 10 + (value - 9) / 4;
		m_i = (value - 9) % 4;
		m = mods[m_i];
		return snprintf(str, size, "5.%d%c", n, m);
	}
}

int grade_from_string(Grade *g, const char *str, uint32_t type_hint)
{
	if (!g|| !str || !*str)
		return 1;

	if (type_hint == VERMTYPE) {
		if (verm_parse(&g->as.verm, str) == 0) {
			g->type = VERMTYPE;
			return 0;
		}

		return 1;
	}

	if (type_hint == FONTTYPE) {
		if (font_parse(&g->as.font, str) == 0) {
			g->type = FONTTYPE;
			return 0;
		}

		return 1;
	}

	if (type_hint == YDSTYPE) {
		if (yds_parse(&g->as.yds, str) == 0) {
			g->type = YDSTYPE;
			return 0;
		}

		return 1;
	}

	// ANYTYPE
	if (verm_parse(&g->as.verm, str) == 0) {
		g->type = VERMTYPE;
		return 0;
	}

	if (font_parse(&g->as.font, str) == 0) {
		g->type = FONTTYPE;
		return 0;
	}

	if (yds_parse(&g->as.yds, str) == 0) {
		g->type = YDSTYPE;
		return 0;
	}

	return 1;
}

int grade_format(const Grade *grade, char *str, size_t size)
{
	if (grade == NULL || str == NULL || size == 0)
		return -1;

	switch (grade->type) {
		case VERMTYPE:
			return verm_format(&grade->as.verm, str, size);
		case FONTTYPE:
			return font_format(&grade->as.font, str, size);
		case YDSTYPE:
			return yds_format(&grade->as.yds, str, 16);
	}

	return -1;
}

int grade_cmp(const Grade *g1, const Grade *g2)
{
	if (!g1 || !g2)
		return 0;

	// This is senseless
	if (g1->type != g2->type)
		return g1->type - g2->type;

	switch (g1->type) {
		case VERMTYPE:
			return verm_cmp(&g1->as.verm, &g2->as.verm);
		case FONTTYPE:
			return font_cmp(&g1->as.font, &g2->as.font);
		case YDSTYPE:
			return yds_cmp(&g1->as.yds, &g2->as.yds);
		default:
			return 0;
	}
}

Verm *grade_as_verm(Grade *g)
{
	if (!g || g->type != VERMTYPE)
		return NULL;
	return &g->as.verm;
}

Font *grade_as_font(Grade *g)
{
	if (!g || g->type != FONTTYPE)
		return NULL;
	return &g->as.font;
}

Yds *grade_as_yds(Grade *g)
{
	if (!g || g->type != YDSTYPE)
		return NULL;
	return &g->as.yds;
}

size_t grade_serialize(const Grade *g, uint8_t *buf, size_t cap)
{
	uint32_t type = g->type;
	size_t needed = sizeof(uint32_t) + sizeof(uint8_t);

	if (buf == NULL)
		return needed;

	if (cap < needed)
		return 0;

	memcpy(buf, &type, sizeof(uint32_t));

	switch (type) {
	case VERMTYPE:
		buf[4] = verm_get_value(&g->as.verm);
		break;
	case FONTTYPE:
		buf[4] = font_get_value(&g->as.font);
		break;
	case YDSTYPE:
		buf[4] = yds_get_value(&g->as.yds);
		break;
	default:
		return 0;
	}

	return needed;
}

int grade_deserialize(Grade *g, const uint8_t *buf, size_t len, size_t *consumed)
{
	uint32_t type;
	uint8_t value;
	size_t needed = sizeof(uint32_t) + sizeof(uint8_t);

	if (len < needed)
		return -1;

	memcpy(&type, buf, sizeof(uint32_t));

	g->type = type;
	value = buf[4];

	switch (type) {
	case VERMTYPE:
		verm_set_value(&g->as.verm, value);
		break;
	case FONTTYPE:
		font_set_value(&g->as.font, value);
		break;
	case YDSTYPE:
		yds_set_value(&g->as.yds, value);
		break;
	default:
		return -1;
	}

	if (consumed)
		*consumed = needed;

	return 0;
}
