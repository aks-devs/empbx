/**
 **
 ** (C)2026 akstel.org
 **/
#include <empbx.h>

static bool fl_syslog = false;

// ---------------------------------------------------------------------------------------------------------------------------------------------
// public
// ---------------------------------------------------------------------------------------------------------------------------------------------
empbx_status_t empbx_log_init(bool use_syslog) {
    fl_syslog = use_syslog;

    if(!fl_syslog) {
        openlog(SYSLOG_NAME, LOG_PID | LOG_NDELAY, LOG_DAEMON);
    }

    return EMPBX_STATUS_SUCCESS;
}

void empbx_log_shutdown() {
    if(fl_syslog) {
        closelog();
    }
}

void empbx_log_notice(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    if(fl_syslog) {
        vsyslog(LOG_NOTICE, fmt, ap);
    } else {
        re_vfprintf(stderr, fmt, ap);
    }
    va_end(ap);
}

void empbx_log_debug(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    if(fl_syslog) {
        vsyslog(LOG_DEBUG, fmt, ap);
    } else {
        re_vfprintf(stderr, fmt, ap);
    }
    va_end(ap);
}

void empbx_log_err(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    if(fl_syslog) {
        vsyslog(LOG_ERR, fmt, ap);
    } else {
        re_vfprintf(stderr, fmt, ap);
    }
    va_end(ap);
}

void empbx_log_warn(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    if(fl_syslog) {
        vsyslog(LOG_WARNING, fmt, ap);
    } else {
        re_vfprintf(stderr, fmt, ap);
    }
    va_end(ap);
}
