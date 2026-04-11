#include "support/string_list.h"

#include <stddef.h>
#include <string.h>

#include "array.h"

struct StringList {
    xarray_t *items;
};

StringList *string_list_new(void)
{
    StringList *list = (StringList *)malloc(sizeof(StringList));
    if (!list) {
        return NULL;
    }
    list->items = XArray_New(2, 0);
    if (!list->items) {
        free(list);
        return NULL;
    }
    return list;
}

void string_list_free(StringList **list)
{
    if (!list || !*list) {
        return;
    }
    XArray_Free(&(*list)->items);
    free(*list);
    *list = NULL;
}

int string_list_add(StringList *list, const char *value)
{
    if (!list || !value) {
        return 0;
    }
    return XArray_AddData(list->items, (void *)value, strlen(value) + 1) == XARRAY_SUCCESS;
}

size_t string_list_count(const StringList *list)
{
    if (!list || !list->items) {
        return 0;
    }
    return XArray_Used(list->items);
}

char *string_list_get(const StringList *list, size_t index)
{
    if (!list || !list->items) {
        return NULL;
    }
    return (char *)XArray_GetData(list->items, index);
}
