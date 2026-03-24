/**
 **
 ** (C)2026 akstel.org
 **/
#ifndef EMPBX_UTILS_H
#define EMPBX_UTILS_H
#include <empbx-core.h>

/* common */
empbx_status_t empbx_switch_ug(char *user, char *group);
empbx_status_t empbx_dir_create_ifne(char *dir);
bool empbx_str2bool(const char *str);

/* pid */
pid_t empbx_pid_read(const char *filename);
empbx_status_t empbx_pid_delete(const char *filename);
empbx_status_t empbx_pid_write(const char *filename, pid_t pid);

/* file */
bool empbx_file_exists(const char *filename);
bool empbx_dir_exists(const char *dirname);
empbx_status_t empbx_dir_create(const char *dirname, bool recursive);

/* threads */
empbx_status_t empbx_thread_launch(thrd_start_t func, void *udata);
void empbx_thread_finished();

/* time */
uint32_t empbx_time_epoch_now();
uint64_t empbx_time_micro_now();

/* str */
char *empbx_strndup(char *src, size_t sz);

#endif
