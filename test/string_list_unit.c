#include <stdio.h>
#include <string.h>

#include "support/string_list.h"

static int failures;

static void expect(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        failures++;
    }
}

int main(void)
{
    StringList *list;
    char mutable_value[] = "first";

    string_list_free(NULL);
    list = NULL;
    string_list_free(&list);

    expect(string_list_count(NULL) == 0, "NULL list has zero entries");
    expect(string_list_get(NULL, 0) == NULL, "NULL list get is NULL");
    expect(!string_list_add(NULL, "value"), "cannot add to NULL list");

    list = string_list_new();
    expect(list != NULL, "list allocation succeeds");
    if (!list) {
        return 1;
    }

    expect(string_list_count(list) == 0, "new list is empty");
    expect(string_list_get(list, 0) == NULL, "empty list get is NULL");
    expect(!string_list_add(list, NULL), "NULL value is rejected");
    expect(string_list_add(list, mutable_value), "first value is appended");
    mutable_value[0] = 'F';
    expect(string_list_add(list, ""), "empty string is appended");
    expect(string_list_add(list, "repeat"), "first repeated value is appended");
    expect(string_list_add(list, "repeat"), "second repeated value is appended");

    expect(string_list_count(list) == 4, "append order count is retained");
    expect(strcmp(string_list_get(list, 0), "first") == 0,
           "stored value is copied instead of borrowed");
    expect(strcmp(string_list_get(list, 1), "") == 0, "empty value is retained");
    expect(strcmp(string_list_get(list, 2), "repeat") == 0, "first repeated value is retained");
    expect(strcmp(string_list_get(list, 3), "repeat") == 0, "second repeated value is retained");
    expect(string_list_get(list, 4) == NULL, "one-past-end get is NULL");
    expect(string_list_get(list, (size_t)-1) == NULL, "maximum index get is NULL");

    string_list_free(&list);
    expect(list == NULL, "free clears caller pointer");
    string_list_free(&list);

    return failures != 0;
}
