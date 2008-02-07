#ifndef ASSERT_H
#define ASSERT_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define assert(expression, error_message) \
	_assert(expression, __LINE__, __FILE__, error_message)

#define assert_exit(expression, error_message) \
	_assert_exit(expression, __LINE__, __FILE__, error_message)

#define error(error_message) \
	_error(__LINE__, __FILE__, error_message)

void _assert_exit(int expression, int line_number, char *file, char *error_message);

void _assert(int expression, int line_number, char *file, char *error_message);

void _error(int line_number, char *file, char *error_message);

#endif
