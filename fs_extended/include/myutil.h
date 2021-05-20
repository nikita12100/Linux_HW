#pragma once

#include <stddef.h>

typedef struct Path {
	const char* line;
	const char* const* parts;
	int count;
} Path;

Path split_path(const char*);
void free_path(Path path);
void die(const char*);
char warn(const char*);
void die_fatal(const char*);
int get_page_size();

void intercept_errors(void (*interceptor) (const char*));
void configure_error_logging(char to_stderr, char to_syslog);


typedef struct String {
    int length;
    char* string;
} String;

String to_string(const char* s);
void free_string(String string);
