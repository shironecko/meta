#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

char* load_text_file(const char* path) {
	FILE* file = fopen(path, "rb");
	if (!file)
		return NULL;

	fseek(file, 0, SEEK_END);
	int file_size = ftell(file);
	fseek(file, 0, SEEK_SET);
	char* contents = (char*)malloc(file_size + 1);
	fread(contents, 1, file_size, file);
	contents[file_size] = 0;
	fclose(file);
	return contents;
}

typedef enum {
	TK_EOF = 0,
	TK_IDENTIFIER,
	TK_SEMICOLON,
	TK_OPEN_BRACE,
	TK_CLOSED_BRACE,
	TK_OPEN_PAREN,
	TK_CLOSED_PAREN,
	TK_OPEN_BRACKET,
	TK_CLOSED_BRACKET,
	TK_LINE_COMMENT,
	TK_BLOCK_COMMENT,
	TK_UNKNOWN = -1,
} TOKEN_TYPE;

typedef enum {
	AT_INTROSPECT,
	AT_UNKNOWN = -1,
} ATTRIBUTE_TYPE;

const char* get_token_type_str(TOKEN_TYPE token_type) {
	switch (token_type) {
		case TK_EOF:
			return "EOF";
		case TK_IDENTIFIER:
			return "INDENTIFIER";
		case TK_SEMICOLON:
			return "SEMICOLON";
		case TK_OPEN_BRACE:
			return "OPEN_BRACE";
		case TK_CLOSED_BRACE:
			return "CLOSED_BRACE";
		case TK_OPEN_PAREN:
			return "OPEN_PAREN";
		case TK_CLOSED_PAREN:
			return "CLOSED_PAREN";
		case TK_OPEN_BRACKET:
			return "OPEN_BRACKET";
		case TK_CLOSED_BRACKET:
			return "CLOSED_BRACKET";
		case TK_LINE_COMMENT:
			return "LINE_COMMENT";
		case TK_BLOCK_COMMENT:
			return "BLOCK_COMMENT";
		case TK_UNKNOWN:
			return "UNKNOWN";
	}

	return "unexpected token type!";
}

const char* get_attribute_type_str(ATTRIBUTE_TYPE attr_type) {
	switch (attr_type) {
		case AT_INTROSPECT:
			return "INTROSPECT";
		case AT_UNKNOWN:
			return "UNKNOWN";
	}

	return "unexpected attribute type!";
}

typedef struct {
	char* at;
	int len;
} str_slice;

typedef struct {
	str_slice str;
	TOKEN_TYPE type;
	int line;
} token;

typedef struct {
	ATTRIBUTE_TYPE type;
} attribute;

int is_whitespace(char c) {
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

int is_identifier_first_char(char c) {
	return c == '_' ||
		(c >= 'A' && c <= 'Z') ||
		(c >= 'a' && c <= 'z');
}

int is_identifier_char(char c) {
	return is_identifier_first_char(c) ||
		(c >= '0' && c <= '9');
}

typedef struct {
	char* text;
	char* at;
	int line;
} tokenizer_ctx;

void eat_whitespace(tokenizer_ctx* ctx) {
	while (is_whitespace(*ctx->at)) {
		if (*ctx->at == '\n') {
			++ctx->line;
		}
		++ctx->at;
	}
}

token next_token(tokenizer_ctx* ctx) {
	assert(ctx);
	assert(ctx->text);
	assert(ctx->at);

	eat_whitespace(ctx);
	
	token result = { .str = { .at = ctx->at, .len= 1 }, .line = ctx->line, .type = TK_UNKNOWN };
	if (!*ctx->at) {
		result.type = TK_EOF;
	} else if (*ctx->at == ';') {
		result.type = TK_SEMICOLON;
	} else if (*ctx->at == '{') {
		result.type = TK_OPEN_BRACE;
	} else if (*ctx->at == '}') {
		result.type = TK_CLOSED_BRACE;
	} else if (*ctx->at == '(') {
		result.type = TK_OPEN_PAREN;
	} else if (*ctx->at == ')') {
		result.type = TK_CLOSED_PAREN;
	} else if (*ctx->at == '[') {
		result.type = TK_OPEN_BRACKET;
	} else if (*ctx->at == ']') {
		result.type = TK_CLOSED_BRACKET;
	} else if (*ctx->at == '/' && ctx->at[1] == '/') {
		result.type = TK_LINE_COMMENT;
		ctx->at += 2;
		while (*ctx->at != '\n') {
			++ctx->at;
		}
		++ctx->line;
		result.str.len = ctx->at - result.str.at + 1;
	} else if (*ctx->at == '/' && ctx->at[1] == '*') {
		result.type = TK_BLOCK_COMMENT;
		ctx->at += 2;
		while (!(ctx->at[0] == '*' && ctx->at[1] == '/')) {
			if (*ctx->at == '\n') {
				++ctx->line;
			}
			++ctx->at;
		}
		++ctx->at;
		result.str.len = ctx->at - result.str.at + 1;
	} else if (is_identifier_first_char(*ctx->at)) {
		result.type = TK_IDENTIFIER;
		do { ++ctx->at; } while (is_identifier_char(*ctx->at));
		result.str.len = ctx->at - result.str.at;
		--ctx->at;
	}

	++ctx->at;
	return result;
}

typedef struct {
	unsigned char* data;
	int size;
	int capacity;
	int item_size;
} array;

array new_array(void* memory, int memory_size, int item_size) {
	assert(memory);
	assert(memory_size);
	assert(item_size);

	return (array){ .data = (unsigned char*)memory, .item_size = item_size, .capacity = memory_size / item_size };
}

array malloc_array(int item_size, int items_count) {
	int array_data_size = item_size * items_count;
	unsigned char* array_data = malloc(array_data_size);
	assert(array_data);
	array result = new_array(array_data, array_data_size, item_size);
	assert(result.capacity == items_count);
	return result;
}

void free_array(array* arr) {
	free(arr->data);
	*arr = (array){0};
}

void assert_array_valid(array* arr) {
	assert(arr);
	assert(arr->data);
	assert(arr->capacity);
	assert(arr->item_size);
	assert(arr->size <= arr->capacity);
}

void* arr_push(array* arr) {
	assert_array_valid(arr);
	assert(arr->size < arr->capacity);
	void* new_item_ptr = arr->data + arr->size * arr->item_size;
	++arr->size;
	return new_item_ptr;
}

void arr_clear(array* arr) {
	assert_array_valid(arr);
	arr->size = 0;
}

void* arr_get(array* arr, int i) {
	assert_array_valid(arr);
	assert(i < arr->size);
	return arr->data + arr->item_size * i;
}

void arr_copy(array* dst, array* src) {
	assert_array_valid(dst);
	assert_array_valid(src);
	assert(dst->item_size == src->item_size);
	assert(dst->capacity >= src->size);
	memcpy(dst->data, src->data, src->size * src->item_size);
	dst->size = src->size;
}

typedef struct {
	str_slice type;
	str_slice name;
} variable_def;

#define MAX_STRUCT_MEMBERS 64
#define MAX_ATTRIBUTES 16

typedef struct {
	str_slice name;
	array members;
	array attributes;
	unsigned char members_data[MAX_STRUCT_MEMBERS * sizeof(variable_def)];
	unsigned char attributes_data[MAX_ATTRIBUTES * sizeof(attribute)];
} struct_def;

void construct_struct_def(struct_def* s) {
	s->name = (str_slice){0};
	s->members = new_array(s->members_data, sizeof(s->members_data), sizeof(variable_def));
	s->attributes = new_array(s->attributes_data, sizeof(s->attributes_data), sizeof(attribute));
	assert(s->members.capacity == MAX_STRUCT_MEMBERS);
}

void printf_struct_def(struct_def* s) {
	assert(s);
	if (s->attributes.size) {
		printf("meta(");
		for (int i = 0; i < s->attributes.size; ++i) {
			attribute* attr = arr_get(&s->attributes, i);
			printf("%s", get_attribute_type_str(attr->type));
			if (i != s->attributes.size - 1) {
				printf(", ");
			}
		}
		printf(")\n");
	}
	printf("struct %.*s {\n", s->name.len, s->name.at);
	for (int i = 0; i < s->members.size; ++i) {
		variable_def* member = arr_get(&s->members, i);
		printf("\t%.*s %.*s;\n", member->type.len, member->type.at, member->name.len, member->name.at);
	}
	printf("}\n");
}

int mini(int a, int b) {
	return a < b ? a : b;
}

int strcmpslice(const char* str, str_slice slice) {
	return strncmp(str, slice.at, mini(slice.len, strlen(str)));
}

#define MAX_STRUCTS_PER_FILE 32

int main() {
	char* file = load_text_file("test.c");
	assert(file);

	tokenizer_ctx tk_ctx = { .text = file, .at = file, .line = 1 };
	token tk = {0};
	array struct_defs = malloc_array(sizeof(struct_def), MAX_STRUCTS_PER_FILE);
	array attributes = malloc_array(sizeof(attribute), MAX_ATTRIBUTES);
	do {
		tk = next_token(&tk_ctx);
		if (tk.type == TK_LINE_COMMENT) {
			printf("line comment at line %d:\n%.*s\n", tk.line, tk.str.len, tk.str.at);
		} else if (tk.type == TK_BLOCK_COMMENT) {
			printf("block comment at line %d:\n%.*s\n", tk.line, tk.str.len, tk.str.at);
		} else if (tk.type == TK_IDENTIFIER) {
			if (!strcmpslice("meta", tk.str)) {
				tk = next_token(&tk_ctx);
				assert(tk.type == TK_OPEN_PAREN);
				while (1) {
					tk = next_token(&tk_ctx);
					if (tk.type == TK_CLOSED_PAREN) {
						break;
					} else if (tk.type == TK_IDENTIFIER) {
						if (!strcmpslice("introspect", tk.str)) {
							attribute* attr = arr_push(&attributes);
							attr->type = AT_INTROSPECT;
						} else {
							printf("unexpected attribute type at line %d: %.*s\n", tk.line, tk.str.len, tk.str.at);
						}
					} else {
						printf("unexpected token at line %d: type(%s), str(%.*s), line(%d)\n", tk.line, get_token_type_str(tk.type), tk.str.len, tk.str.at, tk.line);
					}
				}
				tk = next_token(&tk_ctx);
				if (tk.type == TK_SEMICOLON) {
					tk = next_token(&tk_ctx);
				}
			}

			if (!strcmpslice("struct", tk.str)) {
				struct_def* s = arr_push(&struct_defs);
				construct_struct_def(s);
				arr_copy(&s->attributes, &attributes);
				arr_clear(&attributes);
				tk = next_token(&tk_ctx);
				if (tk.type == TK_IDENTIFIER) {
					s->name = tk.str;
					tk = next_token(&tk_ctx);
				}
				assert(tk.type == TK_OPEN_BRACE);
				tk = next_token(&tk_ctx);
				while (tk.type != TK_CLOSED_BRACE) {
					assert(tk.type == TK_IDENTIFIER);
					variable_def* member = arr_push(&s->members);
					member->type = tk.str;
					tk = next_token(&tk_ctx);
					assert(tk.type == TK_IDENTIFIER);
					member->name = tk.str;
					tk = next_token(&tk_ctx);
					assert(tk.type == TK_SEMICOLON);
					tk = next_token(&tk_ctx);
				}
				tk = next_token(&tk_ctx);
				if (tk.type == TK_IDENTIFIER) {
					s->name = tk.str;
					tk = next_token(&tk_ctx);
				}
				assert(tk.type == TK_SEMICOLON);
			}
		}
		/* printf("%4d:%-15s %.*s\n", tk.line, get_token_type_str(tk.type), tk.str.len, tk.str.at); */
	} while (tk.type != TK_EOF);

	for (int i = 0; i < struct_defs.size; ++i) {
		struct_def* s = arr_get(&struct_defs, i);
		printf_struct_def(s);
	}

	return 0;
}
