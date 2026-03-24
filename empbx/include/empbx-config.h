/**
 **
 ** (C)2026 akstel.org
 **/
#ifndef EMPBX_CONFIG_H
#define EMPBX_CONFIG_H
#include <empbx-core.h>
#include <empbx-utils-list.h>
#include <empbx-utils-htable.h>

typedef struct {
    mutex_t         *mutex;
    mbuf_t          *bsip_config;
    empbx_hash_t    *gateways_map;          // name -> empbx_gateway_entry_t
    mutex_t         *gateways_map_mutex;
    empbx_hash_t    *users_map;       	    // userId => empbx_user_entry_t
    mutex_t         *users_map_mutex;
    empbx_list_t    *dialplan_rules;        // empbx_dialplan_rule_t
    char            *instance_id;
    char            *sip_server_ident;
    char            *path_home;
    char            *path_config;
    char            *path_tmp;
    char            *path_var;
    char            *path_www;
    char            *path_sounds;
    char            *file_config;
    char            *file_pid;
    char            *xuser;
    char            *xgroup;
    uint32_t        active_threds;
    bool            fl_ready;
    bool            fl_shutdown;
    bool            fl_debug_enabled;
    bool            fl_sip_debug_enabled;
} empbx_global_t;

typedef struct {
    char        *id;
    char        *sip_password;
    char        *group;
} empbx_user_entry_t;

typedef struct {
    char        *name;
    char        *user_id;
    char        *realm;
    char        *config;
} empbx_gateway_entry_t;

typedef struct {
    char        *name;
    char        *field;
    char        *expr;
    char        *app_name;
    char        *app_data;
} empbx_dialplan_rule_t;

empbx_status_t empbx_init(empbx_global_t **global);
empbx_status_t empbx_load_config(const char *fname);

empbx_user_entry_t *empbx_config_user_lookup(char *user_id);
empbx_gateway_entry_t *empbx_config_gateway_lookup(char *name);

#endif
