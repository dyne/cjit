#ifndef CJIT_SUPPORT_STRING_LIST_H
#define CJIT_SUPPORT_STRING_LIST_H

#include <stddef.h>

typedef struct StringList StringList;

/**
 * Create an owned list of copied C strings.
 */
StringList *string_list_new(void);

/**
 * Free the list and all copied strings it owns.
 */
void string_list_free(StringList **list);

/**
 * Append one string copy to the list.
 *
 * Returns true on success.
 */
int string_list_add(StringList *list, const char *value);

/**
 * Return the number of stored strings.
 */
size_t string_list_count(const StringList *list);

/**
 * Return the string at `index`, or NULL when out of bounds.
 */
char *string_list_get(const StringList *list, size_t index);

#endif
