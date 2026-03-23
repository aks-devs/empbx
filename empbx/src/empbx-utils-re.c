/**
 **
 ** (C)2026 akstel.org
 **/
#include <empbx.h>

void empbx_ua_destroy(ua_t **ua) {
    ua_t *uap = ua ? *ua : NULL;
    if(uap) {
        ua_destroy(uap);
        *ua = NULL;
    }
}

int empbx_re_debug_print_handler(const char *p, size_t size, void *arg) {
    fwrite(p, size, 1, stderr);
    return 0;
}
