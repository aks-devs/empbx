/**
 **
 ** (C)2025 akstel.org
 **/
#include <empbx.h>

pid_t empbx_pid_read(const char *filename) {
    FILE *f;
    pid_t pid;

    if(!(f = fopen(filename, "r"))) {
        return 0;
    }
    int r=fscanf(f, "%d", &pid);
    fclose(f);

    return pid;
}

empbx_status_t empbx_pid_write(const char *filename, pid_t pid) {
    FILE *f;
    int fd;

    if(((fd = open(filename, O_RDWR | O_CREAT, 0644)) == -1) || ((f = fdopen(fd, "r+")) == NULL)) {
        return EMPBX_STATUS_FALSE;
    }
    if(!fprintf(f, "%d\n", pid)) {
        close(fd);
        return EMPBX_STATUS_FALSE;
    }
    fflush(f);
    close(fd);

    return EMPBX_STATUS_SUCCESS;
}

empbx_status_t empbx_pid_delete(const char *filename) {
    return unlink(filename) == 0 ? EMPBX_STATUS_SUCCESS : EMPBX_STATUS_FALSE;
}
