/*
 * Block comment at the top of the file
 */

#include <stdio.h>

#define meta(params)

#define define_min(name, type) \
	type name(type a, type b) { \
		return a < b ? a : b; \
	}

define_min(mini, int);

meta(introspect)
typedef struct {
	int number;
	float real_number;
} test_struct_a;

// line comment before ignored struct
typedef struct {
	int bogus;
	char trash;
} this_should_be_ignored;

// line comment before introspected struct
meta(introspect)
typedef struct {
	double double_real;
	int dword;
	char byte_a;
	char byte_b;
} test_struct_b;

#include "test.meta.c"

int main() {
	test_struct_a a = { 42, 3.14f };
	saveb_test_struct_a(&a, "a.dat");
	test_struct_a ax = {0};
	loadb_test_struct_a(&ax, "a.dat");
	printf("number = %d\nreal_number = %f\n", ax.number, ax.real_number);
	return 0;
}
