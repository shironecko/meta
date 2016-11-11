/*
 * Block comment at the top of the file
 */

#include <stdio.h>

#define meta(params)

meta(introspect);
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
meta(introspect);
typedef struct {
	double double_real;
	int dword;
	char byte_a;
	char byte_b;
} test_struct_b;

int main() {
	return 0;
}
