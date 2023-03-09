#include <dlfcn.h>
#include <stdio.h>

int main() {
    void* dlHandle = dlopen("lib_wc_repl_lib.so", RTLD_LAZY);
    void (*start_repl_loop)();
    start_repl_loop = (void (*)())dlsym(dlHandle, "start_repl_loop");

    if (dlerror() != NULL) {
        printf("dlerror\n");
        dlclose(dlHandle);
        return 1;
    }

    (*start_repl_loop)();

    dlclose(dlHandle);
    return 0;
}
