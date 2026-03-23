/**
 **
 ** (C)2025 akstel.org
 **/
#include <empbx.h>

static empbx_status_t l_dir_create_recursive(const char *dirname) {
    empbx_status_t st = EMPBX_STATUS_SUCCESS;
    char tmp[255] = {0};
    char *p = NULL;
    size_t len;
    int err;

    snprintf(tmp, sizeof(tmp), "%s", dirname);
    len = str_len((char *)tmp);

    if(tmp[len - 1] == '/') {
        tmp[len - 1] = 0;
    }

    for(p = tmp + 1; *p; p++) {
        if(*p == '/') {
            *p = 0;
            err = mkdir(tmp, S_IRWXU);
            if(err && errno != EEXIST) {
                st = (errno == 0 ? EMPBX_STATUS_SUCCESS : errno);
                break;
            }
            *p = '/';
        }
    }

    if(st == EMPBX_STATUS_SUCCESS) {
        err = mkdir(tmp, S_IRWXU);
        if(err && errno != EEXIST) {
            st = (errno == 0 ? EMPBX_STATUS_SUCCESS : errno);
        }
    }

    return st;
}


// ---------------------------------------------------------------------------------------------------------------------------------------------
// public
// ---------------------------------------------------------------------------------------------------------------------------------------------
bool empbx_dir_exists(const char *dirname) {
    struct stat st = {0};

    if(!dirname) {
        return false;
    }
    if(stat(dirname, &st) == -1) {
        return false;
    }

    return(st.st_mode & S_IFDIR ? true : false);
}

empbx_status_t empbx_dir_create(const char *dirname, bool recursive) {
    if(!dirname) {
        return EMPBX_STATUS_FALSE;
    }

    if(!recursive) {
        if(mkdir(dirname, S_IRWXU) == 0) {
            return EMPBX_STATUS_SUCCESS;
        }
        if(errno == EEXIST) {
            return EMPBX_STATUS_SUCCESS;
        }
        return errno;
    }

    return l_dir_create_recursive(dirname);
}

bool empbx_file_exists(const char *filename) {
    struct stat st = {0};

    if(!filename) {
        return false;
    }
    if(stat(filename, &st) == -1) {
        return false;
    }

    return(st.st_mode & S_IFREG ? true : false);
}
