/**
 **
 ** (C)2026 akstel.org
 **/
#ifndef EMPBX_LOG_H
#define EMPBX_LOG_H
#include <empbx-core.h>

empbx_status_t empbx_log_init(bool use_stdout);
void empbx_log_shutdown();

void empbx_log_notice(const char *fmt, ...);
void empbx_log_debug(const char *fmt, ...);
void empbx_log_err(const char *fmt, ...);
void empbx_log_warn(const char *fmt, ...);

#define log_notice(fmt, ...)    do{empbx_log_notice("NOTICE [%s:%d]: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);} while (0)
#define log_error(fmt, ...)     do{empbx_log_err("ERROR [%s:%d]: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);} while (0)
#define log_warn(fmt, ...)      do{empbx_log_warn("WARN [%s:%d]: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);} while (0)
#define log_debug(fmt, ...)     do{empbx_log_debug("DEBUG [%s:%d]: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);} while (0)

#define log_mem_fail()          do{empbx_log_err("FAIL [%s:%d]: Not enough memory!\n", __FILE__, __LINE__);} while (0)
#define mem_fail_goto_status(_status, _label) do{empbx_log_err("FAIL [%s:%d]: Not enough memory!\n", __FILE__, __LINE__); status = _status; goto _label; } while (0)


#endif
