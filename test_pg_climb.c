#include <check.h>
#include <stdlib.h>
#include "pg_climb.h"

START_TEST(test_grade_type_name)
{
	ck_assert_str_eq(grade_type_name(0), "Unknown");
	ck_assert_str_eq(grade_type_name(1), "V-Scale");
	ck_assert_str_eq(grade_type_name(2), "Font-Scale");
	ck_assert_str_eq(grade_type_name(3), "Unknown");
}
END_TEST

START_TEST(test_grade_type_from_typmod)
{
	ck_assert_uint_eq(grade_type_from_typmod("verm"), VERMTYPE);
	ck_assert_uint_eq(grade_type_from_typmod("font"), FONTTYPE);
	ck_assert_uint_eq(grade_type_from_typmod("nothing"), ANYTYPE);
}
END_TEST

START_TEST(test_grade_strings)
{
	Grade *grade;
	Verm *verm;
	Font *font;
	char *string;

	grade = grade_from_string("", ANYTYPE);
	ck_assert_ptr_null(grade);

	grade = grade_from_string("V6", ANYTYPE);
	ck_assert_ptr_nonnull(grade);
	ck_assert_uint_eq(grade->type, VERMTYPE);

	// TODO a casting/function may be nice here
	verm = (Verm*)grade;
	ck_assert_uint_eq(verm_get_value(verm), 6);

	string = grade_to_string(grade);
	ck_assert_ptr_nonnull(string);
	ck_assert_str_eq(string, "V6");
	free(string);

	grade = grade_from_string("F7C+", ANYTYPE);
	ck_assert_ptr_nonnull(grade);
	ck_assert_uint_eq(grade->type, FONTTYPE);

	// TODO a casting/function may be nice here
	font = (Font*)grade;
	ck_assert_uint_eq(font_get_value(font), 21);

	string = grade_to_string(grade);
	ck_assert_ptr_nonnull(string);
	ck_assert_str_eq(string, "F7C+");
	free(string);

	free(grade);
}
END_TEST

START_TEST(test_verm_basic)
{
	Verm *verm;

	verm = verm_create(0);
	ck_assert_ptr_nonnull(verm);
	ck_assert_uint_eq(verm_get_value(verm), 0);
	verm_set_value(verm, 7);
	ck_assert_uint_eq(verm_get_value(verm), 7);

	verm_free(verm);
}
END_TEST

START_TEST(test_verm_parse)
{
	Verm *verm;
	int ret;

	ret = verm_parse(NULL, NULL);
	ck_assert_uint_ne(ret, 0);
	ret = verm_parse(NULL, "");
	ck_assert_uint_ne(ret, 0);

	// invalid strings
	verm = verm_from_string("");
	ck_assert_ptr_null(verm);
	verm = verm_from_string("v");
	ck_assert_ptr_null(verm);
	verm = verm_from_string("b0");
	ck_assert_ptr_null(verm);
	verm = verm_from_string("v256");
	ck_assert_ptr_null(verm);

	// valid strings
	verm = verm_from_string("v1");
	ck_assert_ptr_nonnull(verm);
	ck_assert_uint_eq(verm_get_value(verm), 1);

	ret = verm_parse(verm, "V5");
	ck_assert_int_eq(ret, 0);
	ck_assert_uint_eq(verm_get_value(verm), 5);
	verm_free(verm);
}
END_TEST

START_TEST(test_verm_format)
{
	Verm *verm;
	char *string;

	ck_assert_ptr_null(verm_format(NULL));

	// valid strings
	verm = verm_create(0);
	string = verm_format(verm);
	ck_assert_str_eq(string, "V0");
	free(string);

	verm_set_value(verm, 8);
	string = verm_format(verm);
	ck_assert_str_eq(string, "V8");
	free(string);

	verm_set_value(verm, 12);
	string = verm_format(verm);
	ck_assert_str_eq(string, "V12");
	free(string);

	verm_free(verm);
}


START_TEST(test_font_basic)
{
	Font *font;

	font = font_create(0);
	ck_assert_ptr_nonnull(font);
	ck_assert_uint_eq(font_get_value(font), 0);
	font_set_value(font, 7);
	ck_assert_uint_eq(font_get_value(font), 7);

	font_free(font);
}
END_TEST

START_TEST(test_font_parse)
{
	Font *font;
	int ret;

	ret = font_parse(NULL, NULL);
	ck_assert_uint_ne(ret, 0);
	ret = font_parse(NULL, "");
	ck_assert_uint_ne(ret, 0);

	// invalid strings
	font = font_from_string("");
	ck_assert_ptr_null(font);
	font = font_from_string("F");
	ck_assert_ptr_null(font);
	font = font_from_string("v0");
	ck_assert_ptr_null(font);
	font = font_from_string("F6D");
	ck_assert_ptr_null(font);

	// valid strings
	font = font_from_string("F1");
	ck_assert_ptr_nonnull(font);
	ck_assert_uint_eq(font_get_value(font), 0);

	ret = font_parse(font, "F3+");
	ck_assert_int_eq(ret, 0);
	ck_assert_uint_eq(font_get_value(font), 5);

	ret = font_parse(font, "F6A");
	ck_assert_int_eq(ret, 0);
	ck_assert_uint_eq(font_get_value(font), 10);

	ret = font_parse(font, "F6A+");
	ck_assert_int_eq(ret, 0);
	ck_assert_uint_eq(font_get_value(font), 11);

	font_free(font);
}
END_TEST

START_TEST(test_font_format)
{
	Font *font;
	char *string;

	ck_assert_ptr_null(font_format(NULL));

	// valid strings
	font = font_create(0);
	string = font_format(font);
	ck_assert_str_eq(string, "F1");
	free(string);

	font_set_value(font, 5);
	string = font_format(font);
	ck_assert_str_eq(string, "F3+");
	free(string);

	font_set_value(font, 10);
	string = font_format(font);
	ck_assert_str_eq(string, "F6A");
	free(string);

	font_set_value(font, 11);
	string = font_format(font);
	ck_assert_str_eq(string, "F6A+");
	free(string);

	font_free(font);
}
END_TEST

START_TEST(test_serial_verm)
{
	Verm *verm;
	SerializedGrade *ser;
	u_int8_t *data;
	size_t size;

	// constant size 4(type)+1(value)
	ck_assert_uint_eq(serialized_grade_size_from_verm(), 5);

	// serialize
	verm = verm_create(5);
	ser = serialized_grade_from_verm(verm, &size);
	ck_assert_ptr_nonnull(ser);
	ck_assert_uint_eq(size, 5);
	data = (u_int8_t*)ser->data;
	ck_assert_uint_eq(serialized_grade_data_read_uint32_t(data), VERMTYPE);
	ck_assert_uint_eq(serialized_grade_data_read_uint8_t(data+4), 5);

	verm_free(verm);
	verm = NULL;

	// deserialize
	verm = verm_from_serialized_grade_data((u_int8_t*)ser->data, &size);
	ck_assert_ptr_nonnull(verm);
	ck_assert_uint_eq(verm_get_value(verm), 5);

	serialized_grade_free(ser);
	verm_free(verm);
}
END_TEST

START_TEST(test_serial_font)
{
	Font *font;
	SerializedGrade *ser;
	u_int8_t *data;
	size_t size;

	// constant size 4(type)+1(value)
	ck_assert_uint_eq(serialized_grade_size_from_font(), 5);

	// serialize
	font = font_create(12);
	ser = serialized_grade_from_font(font, &size);
	ck_assert_ptr_nonnull(ser);
	ck_assert_uint_eq(size, 5);
	data = (u_int8_t*)ser->data;
	ck_assert_uint_eq(serialized_grade_data_read_uint32_t(data), FONTTYPE);
	ck_assert_uint_eq(serialized_grade_data_read_uint8_t(data+4), 12);

	font_free(font);
	font = NULL;

	// deserialize
	font = font_from_serialized_grade_data((u_int8_t*)ser->data, &size);
	ck_assert_ptr_nonnull(font);
	ck_assert_uint_eq(font_get_value(font), 12);

	serialized_grade_free(ser);
	font_free(font);
}
END_TEST

START_TEST(test_serial_grade)
{
	Grade *grade;
	SerializedGrade *ser;
	Verm *verm;
	Font *font;
	u_int8_t *data;
	size_t size;

	// verm
	// serialize
	grade = grade_from_string("V6", ANYTYPE);
	ser = serialized_grade_from_grade(grade, &size);
	ck_assert_ptr_nonnull(ser);
	ck_assert_uint_eq(size, 5);
	data = (u_int8_t*)ser->data;
	ck_assert_uint_eq(serialized_grade_data_read_uint32_t(data), VERMTYPE);
	ck_assert_uint_eq(serialized_grade_data_read_uint8_t(data+4), 6);

	grade_free(grade);
	grade = NULL;

	// deserialize
	grade = grade_from_serialized(ser);
	ck_assert_ptr_nonnull(grade);
	ck_assert_uint_eq(grade->type, VERMTYPE);
	verm = (Verm*)grade;
	ck_assert_uint_eq(verm_get_value(verm), 6);

	serialized_grade_free(ser);
	grade_free(grade);

	// font
	// serialize
	grade = grade_from_string("F8A", ANYTYPE);
	ser = serialized_grade_from_grade(grade, &size);
	ck_assert_ptr_nonnull(ser);
	ck_assert_uint_eq(size, 5);
	data = (u_int8_t*)ser->data;
	ck_assert_uint_eq(serialized_grade_data_read_uint32_t(data), FONTTYPE);
	ck_assert_uint_eq(serialized_grade_data_read_uint8_t(data+4), 22);

	grade_free(grade);
	grade = NULL;

	// deserialize
	grade = grade_from_serialized(ser);
	ck_assert_ptr_nonnull(grade);
	ck_assert_uint_eq(grade->type, FONTTYPE);
	font = (Font*)grade;
	ck_assert_uint_eq(font_get_value(font), 22);

	serialized_grade_free(ser);
	grade_free(grade);
}

static Suite* pg_climb_suite(void)
{
	Suite *s;
	TCase *tc_core;
	TCase *tc_verm;
	TCase *tc_font;
	TCase *tc_serial;

	s = suite_create("pg_climb");
	tc_core = tcase_create("Core");
	tc_verm = tcase_create("V-Scale");
	tc_font = tcase_create("Font-Scale");
	tc_serial = tcase_create("Serialization");

	tcase_add_test(tc_core, test_grade_type_name);
	tcase_add_test(tc_core, test_grade_type_from_typmod);
	tcase_add_test(tc_core, test_grade_strings);
	suite_add_tcase(s, tc_core);

	tcase_add_test(tc_verm, test_verm_basic);
	tcase_add_test(tc_verm, test_verm_parse);
	tcase_add_test(tc_verm, test_verm_format);
	suite_add_tcase(s, tc_verm);

	tcase_add_test(tc_font, test_font_basic);
	tcase_add_test(tc_font, test_font_parse);
	tcase_add_test(tc_font, test_font_format);
	suite_add_tcase(s, tc_font);

	tcase_add_test(tc_serial, test_serial_verm);
	tcase_add_test(tc_serial, test_serial_font);
	tcase_add_test(tc_serial, test_serial_grade);
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
