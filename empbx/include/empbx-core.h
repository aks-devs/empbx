/**
 **
 ** (C)2026 akstel.org
 **/
#ifndef EMPBX_CORE_H
#define EMPBX_CORE_H

#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <syslog.h>
#include <stdarg.h>
#include <stdbool.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/time.h>
#include <cJSON.h>
#include <cJSON_Utils.h>
#include <ezxml.h>

#include <empbx-re.h>
#include <empbx-baresip.h>

#define DEFAULT_HOME            "/opt/empbx"
#define SYSLOG_NAME             "empbxd"
#define APP_CONFIG_NAME         "empbxd-conf.xml"
#define APP_PID_NAME            "empbxd.pid"
#define APP_VERSION             "v0.0.1"
#define APP_CONFIG_NAMESPACE    "http://org.akstel.empbx#1.0"

#define print_mem_fail() do{re_fprintf(stderr, "FAIL [%s:%d]: Not enough memory!\n", __FILE__, __LINE__);} while (0)
#define empbx_goto_status(_status, _label) status = _status; goto _label

static inline int zstr(const char *s) {
    if (!s) return 1;
    if (*s == '\0') return 1;
    return 0;
}

typedef enum {
    D_ACTION_NONE = 0,
    D_ACTION_START,
    D_ACTION_STOP
} daemon_action_t;

typedef enum {
    EMPBX_STATUS_SUCCESS = 0,
    EMPBX_STATUS_FALSE,
    EMPBX_STATUS_MEM_FAIL,
    EMPBX_STATUS_QUEUE_FULL,
    EMPBX_STATUS_NOT_FOUND,
    EMPBX_STATUS_EXPIRED,
    EMPBX_STATUS_INVALID_ARGUMENT
} empbx_status_t;


#endif
