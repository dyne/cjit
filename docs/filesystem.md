# Filesystem operations

The [dmon](https://github.com/septag/dmon/) header is designed for
monitoring changes in the filesystem: it simplifies the process of
tracking file modifications, deletions, and creations, allowing you to
respond to these events in real time within your C scripts.

Here is
[examples/dmon.c](https://github.com/dyne/cjit/blob/main/examples/dmon.c)
which is tested to run on all CJIT platforms:

```c
#include <stdio.h>
#include <dmon.h>

static void watch_callback(dmon_watch_id watch_id,
                           dmon_action action,
                           const char* rootdir,
                           const char* filepath,
                           const char* oldfilepath,
                           void* user) {
    (void)(user);
    (void)(watch_id);
    switch (action) {
    case DMON_ACTION_CREATE:
        fprintf(stderr,"CREATE: [%s]%s\n", rootdir, filepath);
        break;
    case DMON_ACTION_DELETE:
        fprintf(stderr,"DELETE: [%s]%s\n", rootdir, filepath);
        break;
    case DMON_ACTION_MODIFY:
        fprintf(stderr,"MODIFY: [%s]%s\n", rootdir, filepath);
        break;
    case DMON_ACTION_MOVE:
        fprintf(stderr,"MOVE: [%s]%s -> [%s]%s\n",
          rootdir, oldfilepath, rootdir, filepath);
        break;
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        dmon_init();
        puts("waiting for changes ..");
        dmon_watch(argv[1], watch_callback,
          DMON_WATCHFLAGS_RECURSIVE, NULL);
        getchar();
        dmon_deinit();
    } else {
        puts("usage: test dirname");
    }
    return 0;
}
```

# ⏩ [Next: Terminal User Interface](tui.md)

Go to the next chapter of this tutorial.
