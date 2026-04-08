#include <check.h>
#include <stdlib.h>
#include "pg_climb.h"

START_TEST(test_grade_type_name)
{
	ck_assert_str_eq(grade_type_name(0), "Unknown");
	ck_assert_str_eq(grade_type_name(1), "V-Scale");
	ck_assert_str_eq(grade_type_name(2), "Font-Scale");
	ck_assert_str_eq(grade_type_name(3), "Yosemite Decimal System");
	ck_assert_str_eq(grade_type_name(4), "Unknown");
}
END_TEST

START_TEST(test_grade_type_from_typmod)
{
	ck_assert_uint_eq(grade_type_from_typmod("verm"), VERMTYPE);
	ck_assert_uint_eq(grade_type_from_typmod("font"), FONTTYPE);
	ck_assert_uint_eq(grade_type_from_typmod("yds"), YDSTYPE);
	ck_assert_uint_eq(grade_type_from_typmod("nothing"), ANYTYPE);
}
END_TEST

START_TEST(test_grade_strings)
{
	Grade grade;
	Verm *verm;
	Font *font;
	Yds *yds;
	char string[16];

	ck_assert_int_ne(grade_from_string(&grade, "", ANYTYPE), 0);

	ck_assert_int_eq(grade_from_string(&grade, "V6", ANYTYPE), 0);
	ck_assert_uint_eq(grade.type, VERMTYPE);

	verm = grade_as_verm(&grade);
	ck_assert_ptr_nonnull(verm);
	ck_assert_uint_eq(verm_get_value(verm), 6);

	ck_assert_int_eq(grade_format(&grade, string, sizeof(string)), 2);
	ck_assert_str_eq(string, "V6");

	ck_assert_int_eq(grade_from_string(&grade, "F7C+", ANYTYPE), 0);
	ck_assert_uint_eq(grade.type, FONTTYPE);

	font = grade_as_font(&grade);
	ck_assert_ptr_nonnull(font);
	ck_assert_uint_eq(font_get_value(font), 21);

	ck_assert_int_eq(grade_format(&grade, string, sizeof(string)), 4);
	ck_assert_str_eq(string, "F7C+");

	ck_assert_int_eq(grade_from_string(&grade, "5.13b", ANYTYPE), 0);
	ck_assert_uint_eq(grade.type, YDSTYPE);

	yds = grade_as_yds(&grade);
	ck_assert_ptr_nonnull(yds);
	ck_assert_uint_eq(yds_get_value(yds), 22);

	ck_assert_int_eq(grade_format(&grade, string, sizeof(string)), 5);
	ck_assert_str_eq(string, "5.13b");

	// These should fail because of the hint, even though they are valid for other types
	ck_assert_int_ne(grade_from_string(&grade, "F7C", VERMTYPE), 0);
	ck_assert_int_ne(grade_from_string(&grade, "5.11", FONTTYPE), 0);
	ck_assert_int_ne(grade_from_string(&grade, "V7", YDSTYPE), 0);
}
END_TEST

START_TEST(test_grade_cmp)
{
	Grade g1;
	Grade g2;

	// I'm not totally sure what these should be, but they should at least be not equal
	ck_assert_int_eq(grade_from_string(&g1, "V1", VERMTYPE), 0);
	ck_assert_int_eq(grade_from_string(&g2, "F5", FONTTYPE), 0);
	ck_assert_int_ne(grade_cmp(&g1, &g2), 0);

	ck_assert_int_eq(grade_from_string(&g1, "V1", VERMTYPE), 0);
	ck_assert_int_eq(grade_from_string(&g2, "V2", VERMTYPE), 0);
	ck_assert_int_le(grade_cmp(&g1, &g2), 0);

	ck_assert_int_eq(grade_from_string(&g1, "F7A+", FONTTYPE), 0);
	ck_assert_int_eq(grade_from_string(&g2, "F7B", FONTTYPE), 0);
	ck_assert_int_le(grade_cmp(&g1, &g2), 0);

	ck_assert_int_eq(grade_from_string(&g1, "5.9", YDSTYPE), 0);
	ck_assert_int_eq(grade_from_string(&g2, "5.10a", YDSTYPE), 0);
	ck_assert_int_le(grade_cmp(&g1, &g2), 0);
}
END_TEST

START_TEST(test_typmod_string)
{
	ck_assert_ptr_null(typmod_string(ANYTYPE));
	ck_assert_str_eq(typmod_string(VERMTYPE), "verm");
	ck_assert_str_eq(typmod_string(FONTTYPE), "font");
	ck_assert_str_eq(typmod_string(YDSTYPE), "yds");
}
END_TEST

START_TEST(test_verm_basic)
{
	Verm verm;

	verm_set_value(&verm, 7);
	ck_assert_uint_eq(verm_get_value(&verm), 7);
}
END_TEST

START_TEST(test_verm_parse)
{
	Verm verm;

	ck_assert(verm_parse(NULL, NULL));
	ck_assert(verm_parse(NULL, ""));

	ck_assert(verm_parse(&verm, ""));
	ck_assert(verm_parse(&verm, "v"));
	ck_assert(verm_parse(&verm, "b0"));
	ck_assert(verm_parse(&verm, "v256"));

	ck_assert(!verm_parse(&verm, "v1"));
	ck_assert_uint_eq(verm_get_value(&verm), 1);

	ck_assert(!verm_parse(&verm, "v5"));
	ck_assert_uint_eq(verm_get_value(&verm), 5);
}
END_TEST

START_TEST(test_verm_format)
{
	Verm verm;
	char string[16];

	verm_set_value(&verm, 0);
	ck_assert_int_eq(verm_format(&verm, string, sizeof(string)), 2);
	ck_assert_str_eq(string, "V0");

	verm_set_value(&verm, 8);
	ck_assert_int_eq(verm_format(&verm, string, sizeof(string)), 2);
	ck_assert_str_eq(string, "V8");

	verm_set_value(&verm, 12);
	ck_assert_int_eq(verm_format(&verm, string, sizeof(string)), 3);
	ck_assert_str_eq(string, "V12");
}

START_TEST(test_verm_cmp)
{
	Verm v1;
	Verm v2;

	verm_set_value(&v1, 4);
	verm_set_value(&v2, 4);
	ck_assert_int_eq(verm_cmp(&v1, &v2), 0);

	verm_set_value(&v2, 3);
	ck_assert_int_ge(verm_cmp(&v1, &v2), 0);

	verm_set_value(&v2, 5);
	ck_assert_int_le(verm_cmp(&v1, &v2), 0);
}
END_TEST

START_TEST(test_font_basic)
{
	Font font;

	font_set_value(&font, 0);
	ck_assert_uint_eq(font_get_value(&font), 0);
	font_set_value(&font, 7);
	ck_assert_uint_eq(font_get_value(&font), 7);
}
END_TEST

START_TEST(test_font_parse)
{
	Font font;

	ck_assert(font_parse(NULL, NULL));
	ck_assert(font_parse(NULL, ""));

	ck_assert(font_parse(&font, ""));
	ck_assert(font_parse(&font, "F"));
	ck_assert(font_parse(&font, "v0"));
	ck_assert(font_parse(&font, "F6D"));

	ck_assert(!font_parse(&font, "F1"));
	ck_assert_uint_eq(font_get_value(&font), 0);

	ck_assert(!font_parse(&font, "F3+"));
	ck_assert_uint_eq(font_get_value(&font), 5);

	ck_assert(!font_parse(&font, "F6A"));
	ck_assert_uint_eq(font_get_value(&font), 10);

	ck_assert(!font_parse(&font, "F6A+"));
	ck_assert_uint_eq(font_get_value(&font), 11);
}
END_TEST

START_TEST(test_font_format)
{
	Font font;
	char string[16];

	font_set_value(&font, 0);
	ck_assert_int_eq(font_format(&font, string, sizeof(string)), 2);
	ck_assert_str_eq(string, "F1");

	font_set_value(&font, 5);
	ck_assert_int_eq(font_format(&font, string, sizeof(string)), 3);
	ck_assert_str_eq(string, "F3+");

	font_set_value(&font, 10);
	ck_assert_int_eq(font_format(&font, string, sizeof(string)), 3);
	ck_assert_str_eq(string, "F6A");

	font_set_value(&font, 11);
	ck_assert_int_eq(font_format(&font, string, sizeof(string)), 4);
	ck_assert_str_eq(string, "F6A+");
}
END_TEST

START_TEST(test_font_cmp)
{
	Font f1;
	Font f2;

	font_set_value(&f1, 4);
	font_set_value(&f2, 4);
	ck_assert_int_eq(font_cmp(&f1, &f2), 0);

	font_set_value(&f2, 3);
	ck_assert_int_ge(font_cmp(&f1, &f2), 0);

	font_set_value(&f2, 5);
	ck_assert_int_le(font_cmp(&f1, &f2), 0);
}
END_TEST

START_TEST(test_yds_basic)
{
	Yds yds;

	yds_set_value(&yds, 0);
	ck_assert_uint_eq(yds_get_value(&yds), 0);
	yds_set_value(&yds, 7);
	ck_assert_uint_eq(yds_get_value(&yds), 7);
}
END_TEST

START_TEST(test_yds_parse)
{
	Yds yds;

	ck_assert(yds_parse(NULL, NULL));
	ck_assert(yds_parse(NULL, ""));

	ck_assert(yds_parse(&yds, ""));
	ck_assert(yds_parse(&yds, "5."));
	ck_assert(yds_parse(&yds, "f7a"));
	ck_assert(yds_parse(&yds, "5.9a"));

	ck_assert(!yds_parse(&yds, "5.1"));
	ck_assert_uint_eq(yds_get_value(&yds), 0);

	ck_assert(!yds_parse(&yds, "5.9"));
	ck_assert_uint_eq(yds_get_value(&yds), 8);

	ck_assert(!yds_parse(&yds, "5.10a"));
	ck_assert_uint_eq(yds_get_value(&yds), 9);

	ck_assert(!yds_parse(&yds, "5.10c"));
	ck_assert_uint_eq(yds_get_value(&yds), 11);

	ck_assert(!yds_parse(&yds, "5.11a"));
	ck_assert_uint_eq(yds_get_value(&yds), 13);
}
END_TEST

START_TEST(test_yds_format)
{
	Yds yds;
	char string[16];

	yds_set_value(&yds, 0);
	ck_assert_int_eq(yds_format(&yds, string, sizeof(string)), 3);
	ck_assert_str_eq(string, "5.1");

	yds_set_value(&yds, 5);
	ck_assert_int_eq(yds_format(&yds, string, sizeof(string)), 3);
	ck_assert_str_eq(string, "5.6");

	yds_set_value(&yds, 9);
	ck_assert_int_eq(yds_format(&yds, string, sizeof(string)), 5);
	ck_assert_str_eq(string, "5.10a");

	yds_set_value(&yds, 11);
	ck_assert_int_eq(yds_format(&yds, string, sizeof(string)), 5);
	ck_assert_str_eq(string, "5.10c");

	yds_set_value(&yds, 13);
	ck_assert_int_eq(yds_format(&yds, string, sizeof(string)), 5);
	ck_assert_str_eq(string, "5.11a");
}
END_TEST

START_TEST(test_yds_cmp)
{
	Yds y1;
	Yds y2;

	yds_set_value(&y1, 4);
	yds_set_value(&y2, 4);
	ck_assert_int_eq(yds_cmp(&y1, &y2), 0);

	yds_set_value(&y2, 3);
	ck_assert_int_ge(yds_cmp(&y1, &y2), 0);

	yds_set_value(&y2, 5);
	ck_assert_int_le(yds_cmp(&y1, &y2), 0);
}
END_TEST

START_TEST(test_serial_verm)
{
	Verm verm;
	SerializedGrade *ser;
	uint8_t *data;
	size_t size;

	// constant size 4(type)+1(value)
	ck_assert_uint_eq(serialized_grade_size_from_verm(), 5);

	// serialize
	verm_set_value(&verm, 5);
	ser = serialized_grade_from_verm(&verm, &size);
	ck_assert_ptr_nonnull(ser);
	ck_assert_uint_eq(size, 5);
	data = (uint8_t*)ser->data;
	ck_assert_uint_eq(serialized_grade_data_read_uint32_t(data), VERMTYPE);
	ck_assert_uint_eq(serialized_grade_data_read_uint8_t(data+4), 5);

	serialized_grade_free(ser);
}
END_TEST

START_TEST(test_serial_font)
{
	Font font;
	SerializedGrade *ser;
	uint8_t *data;
	size_t size;

	// constant size 4(type)+1(value)
	ck_assert_uint_eq(serialized_grade_size_from_font(), 5);

	// serialize
	font_set_value(&font, 12);
	ser = serialized_grade_from_font(&font, &size);
	ck_assert_ptr_nonnull(ser);
	ck_assert_uint_eq(size, 5);
	data = (uint8_t*)ser->data;
	ck_assert_uint_eq(serialized_grade_data_read_uint32_t(data), FONTTYPE);
	ck_assert_uint_eq(serialized_grade_data_read_uint8_t(data+4), 12);

	serialized_grade_free(ser);
}
END_TEST

START_TEST(test_serial_yds)
{
	Yds yds;
	SerializedGrade *ser;
	uint8_t *data;
	size_t size;

	// constant size 4(type)+1(value)
	ck_assert_uint_eq(serialized_grade_size_from_yds(), 5);

	// serialize
	yds_set_value(&yds, 12);
	ser = serialized_grade_from_yds(&yds, &size);
	ck_assert_ptr_nonnull(ser);
	ck_assert_uint_eq(size, 5);
	data = (uint8_t*)ser->data;
	ck_assert_uint_eq(serialized_grade_data_read_uint32_t(data), YDSTYPE);
	ck_assert_uint_eq(serialized_grade_data_read_uint8_t(data+4), 12);

	serialized_grade_free(ser);
}
END_TEST

START_TEST(test_serial_grade)
{
	Grade grade;
	SerializedGrade *ser;
	Verm *verm;
	Font *font;
	Yds *yds;
	uint8_t *data;
	size_t size;

	// verm
	// serialize
	ck_assert_int_eq(grade_from_string(&grade, "V6", ANYTYPE), 0);
	ser = serialized_grade_from_grade(&grade, &size);
	ck_assert_ptr_nonnull(ser);
	ck_assert_uint_eq(size, 5);
	data = (uint8_t*)ser->data;
	ck_assert_uint_eq(serialized_grade_data_read_uint32_t(data), VERMTYPE);
	ck_assert_uint_eq(serialized_grade_data_read_uint8_t(data+4), 6);

	// deserialize
	ck_assert_int_eq(grade_from_serialized(&grade, ser), 0);
	ck_assert_uint_eq(grade.type, VERMTYPE);
	verm = grade_as_verm(&grade);
	ck_assert_ptr_nonnull(verm);
	ck_assert_uint_eq(verm_get_value(verm), 6);

	serialized_grade_free(ser);

	// font
	// serialize
	ck_assert_int_eq(grade_from_string(&grade, "F8A", ANYTYPE), 0);
	ser = serialized_grade_from_grade(&grade, &size);
	ck_assert_ptr_nonnull(ser);
	ck_assert_uint_eq(size, 5);
	data = (uint8_t*)ser->data;
	ck_assert_uint_eq(serialized_grade_data_read_uint32_t(data), FONTTYPE);
	ck_assert_uint_eq(serialized_grade_data_read_uint8_t(data+4), 22);

	// deserialize
	ck_assert_int_eq(grade_from_serialized(&grade, ser), 0);
	ck_assert_uint_eq(grade.type, FONTTYPE);
	font = grade_as_font(&grade);
	ck_assert_ptr_nonnull(font);
	ck_assert_uint_eq(font_get_value(font), 22);

	serialized_grade_free(ser);

	// yds
	// serialize
	ck_assert_int_eq(grade_from_string(&grade, "5.11b", ANYTYPE), 0);
	ser = serialized_grade_from_grade(&grade, &size);
	ck_assert_ptr_nonnull(ser);
	ck_assert_uint_eq(size, 5);
	data = (uint8_t*)ser->data;
	ck_assert_uint_eq(serialized_grade_data_read_uint32_t(data), YDSTYPE);
	ck_assert_uint_eq(serialized_grade_data_read_uint8_t(data+4), 14);

	// deserialize
	ck_assert_int_eq(grade_from_serialized(&grade, ser), 0);
	ck_assert_uint_eq(grade.type, YDSTYPE);
	yds = grade_as_yds(&grade);
	ck_assert_ptr_nonnull(yds);
	ck_assert_uint_eq(yds_get_value(yds), 14);

	serialized_grade_free(ser);
}

START_TEST(test_serial_cmp)
{
	Grade g1;
	Grade g2;
	SerializedGrade *sg1;
	SerializedGrade *sg2;

	ck_assert_int_eq(grade_from_string(&g1, "V6", ANYTYPE), 0);
	ck_assert_int_eq(grade_from_string(&g2, "V7", ANYTYPE), 0);
	sg1 = serialized_grade_from_grade(&g1, NULL);
	sg2 = serialized_grade_from_grade(&g2, NULL);

	ck_assert_int_le(serialized_grade_cmp(sg1, sg2), 0);
	serialized_grade_free(sg1);
	serialized_grade_free(sg2);
}
END_TEST

static Suite* pg_climb_suite(void)
{
	Suite *s;
	TCase *tc_core;
	TCase *tc_verm;
	TCase *tc_font;
	TCase *tc_yds;
	TCase *tc_serial;

	s = suite_create("pg_climb");
	tc_core = tcase_create("Core");
	tc_verm = tcase_create("V-Scale");
	tc_font = tcase_create("Font-Scale");
	tc_yds = tcase_create("Yosemite Decimal System");
	tc_serial = tcase_create("Serialization");

	tcase_add_test(tc_core, test_grade_type_name);
	tcase_add_test(tc_core, test_grade_type_from_typmod);
	tcase_add_test(tc_core, test_grade_strings);
	tcase_add_test(tc_core, test_grade_cmp);
	tcase_add_test(tc_core, test_typmod_string);
	suite_add_tcase(s, tc_core);

	tcase_add_test(tc_verm, test_verm_basic);
	tcase_add_test(tc_verm, test_verm_parse);
	tcase_add_test(tc_verm, test_verm_format);
	tcase_add_test(tc_verm, test_verm_cmp);
	suite_add_tcase(s, tc_verm);

	tcase_add_test(tc_font, test_font_basic);
	tcase_add_test(tc_font, test_font_parse);
	tcase_add_test(tc_font, test_font_format);
	tcase_add_test(tc_font, test_font_cmp);
	suite_add_tcase(s, tc_font);

	tcase_add_test(tc_yds, test_yds_basic);
	tcase_add_test(tc_yds, test_yds_parse);
	tcase_add_test(tc_yds, test_yds_format);
	tcase_add_test(tc_yds, test_yds_cmp);
	suite_add_tcase(s, tc_yds);

	tcase_add_test(tc_serial, test_serial_verm);
	tcase_add_test(tc_serial, test_serial_font);
	tcase_add_test(tc_serial, test_serial_yds);
	tcase_add_test(tc_serial, test_serial_grade);
	tcase_add_test(tc_serial, test_serial_cmp);
	suite_add_tcase(s, tc_serial);

	return s;
}

int main(void)
{
	int number_failed;
	Suite *s;
	SRunner *sr;

	s = pg_climb_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	return (number_failed == 0) ? 0 : 1;
}
