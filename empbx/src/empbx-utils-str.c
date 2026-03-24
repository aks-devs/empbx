/**
 **
 ** (C)2025 akstel.org
 **/
#include <empbx.h>

char *empbx_strndup(char *src, size_t sz) {
    char *lstr = NULL;

    if(zstr(src) || !sz) {
        return NULL;
    }
    if(!(lstr = mem_zalloc(sz, NULL))) {
        return NULL;
    }
    memcpy(lstr, src, sz);

    return lstr;
}
