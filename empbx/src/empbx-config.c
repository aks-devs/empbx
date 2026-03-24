/**
 **
 ** (C)2026 akstel.org
 **/
#include <empbx.h>

extern empbx_global_t *global;

static void destructor__empbx_global_t(void *data) {
    empbx_global_t *ptr = (empbx_global_t *)data;

    mem_deref(ptr->file_config);
    mem_deref(ptr->file_pid);

    mem_deref(ptr->path_home);
    mem_deref(ptr->path_config);
    mem_deref(ptr->path_sounds);
    mem_deref(ptr->path_var);
    mem_deref(ptr->path_www);
    mem_deref(ptr->path_tmp);

    mem_deref(ptr->xuser);
    mem_deref(ptr->xgroup);

    mem_deref(ptr->bsip_config);
    mem_deref(ptr->instance_id);
    mem_deref(ptr->sip_server_ident);

    mem_deref(ptr->dialplan_rules);
    mem_deref(ptr->gateways_map);
    mem_deref(ptr->gateways_map_mutex);
    mem_deref(ptr->users_map);
    mem_deref(ptr->users_map_mutex);

    mem_deref(ptr->mutex);
}

static void destructor__empbx_user_entry_t(void *data) {
    empbx_user_entry_t *ptr = (empbx_user_entry_t *)data;

    mem_deref(ptr->id);
    mem_deref(ptr->group);
    mem_deref(ptr->sip_password);
}

static void destructor__empbx_gateway_entry_t(void *data) {
    empbx_gateway_entry_t *ptr = (empbx_gateway_entry_t *)data;

    mem_deref(ptr->name);
    mem_deref(ptr->user_id);
    mem_deref(ptr->realm);
    mem_deref(ptr->config);
}

static void destructor__empbx_dialplan_rule_t(void *arg) {
    empbx_dialplan_rule_t *ptr = (empbx_dialplan_rule_t *)arg;

    mem_deref(ptr->name);
    mem_deref(ptr->field);
    mem_deref(ptr->expr);
    mem_deref(ptr->app_name);
    mem_deref(ptr->app_data);
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------
// public
// -------------------------------------------------------------------------------------------------------------------------------------------------------
empbx_status_t empbx_init(empbx_global_t **tglobal) {
    empbx_status_t status = EMPBX_STATUS_SUCCESS;
    empbx_global_t *glocal = NULL;

    glocal = mem_zalloc(sizeof(empbx_global_t), destructor__empbx_global_t);
    if(!glocal) {
        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
    }

    if(mutex_alloc(&glocal->mutex) != LIBRE_SUCCESS) {
        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
    }

    if(mutex_alloc(&glocal->gateways_map_mutex) != LIBRE_SUCCESS) {
        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
    }
    if(empbx_hash_create(&glocal->gateways_map) != EMPBX_STATUS_SUCCESS) {
        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
    }

    if(mutex_alloc(&glocal->users_map_mutex) != LIBRE_SUCCESS) {
        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
    }
    if(empbx_hash_create(&glocal->users_map) != EMPBX_STATUS_SUCCESS) {
        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
    }

    if(empbx_list_create(&glocal->dialplan_rules) != EMPBX_STATUS_SUCCESS) {
        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
    }


    *tglobal = glocal;
out:
    if(status != EMPBX_STATUS_SUCCESS) {
        if(glocal) {
            mem_deref(glocal);
        }
    }
    return status;
}

empbx_status_t empbx_load_config(const char *fname) {
    empbx_status_t status = EMPBX_STATUS_SUCCESS;
    const char *conf_namespace = NULL;
    ezxml_t xml, xelem, xparams, xusers, xgateways, xdialplan;
    mbuf_t *tmp_mbuf = NULL;

    if(!empbx_file_exists(fname)) {
        log_error("File not found (%s)", fname)    ;
        return EMPBX_STATUS_FALSE;
    }

    if((xml = ezxml_parse_file(fname)) == NULL) {
        log_error("Unable to parese XML");
        empbx_goto_status(EMPBX_STATUS_FALSE, out);
    }

    conf_namespace = ezxml_attr(xml, "xmlns:empbx");
    if(zstr(conf_namespace)) {
        log_error("Missing namespace attribure (xmlns:empbx)");
        empbx_goto_status(EMPBX_STATUS_FALSE, out);
    }
    if(str_casecmp(conf_namespace, APP_CONFIG_NAMESPACE)) {
        log_error("Invalid namespace (expected: %s)", APP_CONFIG_NAMESPACE);
        empbx_goto_status(EMPBX_STATUS_FALSE, out);
    }

    if(str_casecmp(ezxml_name(xml), "empbx:configuration") != 0) {
        log_error("Missing root element: <configuration>");
        empbx_goto_status(EMPBX_STATUS_FALSE, out);
    }

    /* tmp buf */
    tmp_mbuf = mbuf_alloc(4096);
    if(!tmp_mbuf) {
        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
    }

    /* core-settings */
    if((xparams = ezxml_child(xml, "core-settings")) != NULL) {
        for(ezxml_t xparam = ezxml_child(xparams, "param"); xparam; xparam = xparam->next) {
            const char *name = ezxml_attr(xparam, "name");
            const char *val = ezxml_attr(xparam, "value");

            if(!name) { continue; }

            if(!str_casecmp(name, "instance-id")) {
                if(zstr(global->instance_id)) str_dup(&global->instance_id, val);
            }
        }
    }
    if(zstr(global->instance_id)) {
        str_dup(&global->instance_id, "0000");
    }

    /* mgmt-service */
    if((xelem = ezxml_child(xml, "mgmt-service")) != NULL) {
        const char *enabled = ezxml_attr(xelem, "enabled");
        if(str_casecmp(enabled, "true") == 0) {
            if((xparams = ezxml_child(xelem, "settings")) != NULL) {
                for(ezxml_t xparam = ezxml_child(xparams, "param"); xparam; xparam = xparam->next) {
                    const char *name = ezxml_attr(xparam, "name");
                    const char *val = ezxml_attr(xparam, "value");

                    if(!name) { continue; }

                    //
                    // todo
                    //
                }
            }
        }
    }

    /* sip-service */
    if((xelem = ezxml_child(xml, "sip-service")) != NULL) {
        if((xusers = ezxml_child(xelem, "users")) != NULL) {
            for(ezxml_t xuser = ezxml_child(xusers, "user"); xuser; xuser = xuser->next) {
                const char *user_id = ezxml_attr(xuser, "id");
                const char *password = ezxml_attr(xuser, "sip-password");
                const char *group = ezxml_attr(xuser, "group");
                empbx_user_entry_t *entry = NULL;

                if(!user_id) { continue; }

                entry = mem_zalloc(sizeof(empbx_user_entry_t), destructor__empbx_user_entry_t);
                if(!entry) {
                    mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
                }

                str_dup(&entry->id, user_id);
                str_dup(&entry->sip_password, password);
                str_dup(&entry->group, group);

                if(empbx_hash_insert_ex(global->users_map, entry->id, entry, true) != EMPBX_STATUS_SUCCESS) {
                    mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
                }
            }
        }

        /* gateways */
        if((xgateways = ezxml_child(xelem, "gateways")) != NULL) {
            for(ezxml_t xgateway = ezxml_child(xgateways, "gateway"); xgateway; xgateway = xgateway->next) {
                const char *gw_name = ezxml_attr(xgateway, "name");
                char *gw_username = NULL, *gw_password = NULL, *gw_domain = NULL, *gw_transport = NULL;
                uint8_t *gw_conf_str = NULL, *gw_opt_ptr = NULL;
                uint32_t gw_conf_buf_len = 0, sas_ofs = 0;
                empbx_gateway_entry_t *gw_entry = NULL;

                mbuf_clean(tmp_mbuf);
                for(ezxml_t xparam = ezxml_child(xgateway, "param"); xparam; xparam = xparam->next) {
                    const char *name = ezxml_attr(xparam, "name");
                    const char *val = ezxml_attr(xparam, "value");
                    if(name) {
                        if(!str_casecmp(name, "username")) {
                            if(gw_username == NULL) str_dup(&gw_username, val);
                        } else if(!str_casecmp(name, "password")) {
                            if(gw_password == NULL) str_dup(&gw_password, val);
                        } else if(!str_casecmp(name, "domain")) {
                            if(gw_domain == NULL) str_dup(&gw_domain, val);
                        } else if(!str_casecmp(name, "transport")) {
                            if(gw_transport == NULL) str_dup(&gw_transport, val);
                        } else {
                            mbuf_printf(tmp_mbuf, ";%s=%s", name, (val ? val : ""));
                        }
                    }
                }

                sas_ofs = tmp_mbuf->pos;

                if(!zstr(gw_name) && !zstr(gw_username) && !zstr(gw_domain)) {
                    mbuf_printf(tmp_mbuf, "<sip:%s@%s", gw_username, gw_domain);
                    if(!zstr(gw_transport)) mbuf_printf(tmp_mbuf, ";transport=%s", gw_transport);
                    mbuf_printf(tmp_mbuf, ">;auth_user=%s", gw_username);
                    if(!zstr(gw_password)) mbuf_printf(tmp_mbuf, ";auth_pass=%s", gw_password);
                    //mbuf_write_str(tmp_mbuf, ">;check_origin=yes");

                    gw_conf_buf_len = tmp_mbuf->pos;

                    if((gw_conf_str = mem_alloc(gw_conf_buf_len + 1, NULL)) == NULL) {
                        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
                    }
                    tmp_mbuf->pos = sas_ofs;
                    mbuf_read_mem(tmp_mbuf, gw_conf_str, mbuf_get_left(tmp_mbuf));

                    tmp_mbuf->pos = 0;
                    gw_opt_ptr = (void *)(gw_conf_str + (gw_conf_buf_len - sas_ofs));
                    mbuf_read_mem(tmp_mbuf, gw_opt_ptr, sas_ofs);
                    gw_conf_str[gw_conf_buf_len] = 0x0;

                    gw_entry = mem_zalloc(sizeof(empbx_gateway_entry_t), destructor__empbx_gateway_entry_t);
                    if(!gw_entry) {
                        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
                    }

                    str_dup(&gw_entry->name, gw_name);
                    str_dup(&gw_entry->user_id, gw_username);
                    str_dup(&gw_entry->realm, gw_domain);
                    gw_entry->config = gw_conf_str;

                    if(empbx_hash_insert_ex(global->gateways_map, gw_entry->name, gw_entry, true) != EMPBX_STATUS_SUCCESS) {
                        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
                    }
                } else {
                    log_warn("Gateway (%s) was ignored", gw_name);
                }

                mem_deref(gw_domain);
                mem_deref(gw_username);
                mem_deref(gw_password);
                mem_deref(gw_transport);
            }
        }

        /* dialplan */
        if((xdialplan = ezxml_child(xelem, "dialplan")) != NULL) {
            for(ezxml_t xrule = ezxml_child(xdialplan, "rule"); xrule; xrule = xrule->next) {
                const char *name = ezxml_attr(xrule, "name");
                const char *field = ezxml_attr(xrule, "field");
                const char *expr = ezxml_attr(xrule, "expression");
                const char *app_name = ezxml_attr(xrule, "app");
                const char *app_data = ezxml_attr(xrule, "data");
                empbx_dialplan_rule_t *rule = NULL;

                if(zstr(name) || zstr(field) || zstr(app_name)) {
                    continue;
                }

                rule = mem_zalloc(sizeof(empbx_dialplan_rule_t), destructor__empbx_dialplan_rule_t);
                if(!rule) {
                    mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
                }

                str_dup(&rule->name, name);
                str_dup(&rule->field, field);
                str_dup(&rule->expr, expr);
                str_dup(&rule->app_name, app_name);
                str_dup(&rule->app_data, app_data);

                if(empbx_list_add_tail(global->dialplan_rules, rule, (mem_destroy_h *)mem_deref) != EMPBX_STATUS_SUCCESS) {
                    mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
                }
            }
        }

        /* baresip config */
        mbuf_clean(tmp_mbuf);
        if((xparams = ezxml_child(xelem, "settings")) != NULL) {
            for(ezxml_t xparam = ezxml_child(xparams, "param"); xparam; xparam = xparam->next) {
                const char *name = ezxml_attr(xparam, "name");
                const char *val = ezxml_attr(xparam, "value");

                if(name) {
                    if(strstr(name, "x_")) {
                        if(!str_casecmp(name, "x_server_ident")) {
                            if(zstr(global->sip_server_ident)) str_dup(&global->sip_server_ident, val);
                        } else if(!str_casecmp(name, "x_sip_debug")) {
                            global->fl_sip_debug_enabled = empbx_str2bool(val);
                        }

                        continue;
                    } else {
                        mbuf_printf(tmp_mbuf, "%s \t %s\n", name, (val ? val : ""));
                    }
                }
            }
        }

        if(tmp_mbuf->pos > 0) {
            global->bsip_config = mbuf_dup(tmp_mbuf);
            if(!global->bsip_config) {
                mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
            }
        }
    }

out:
    if(tmp_mbuf) {
        mem_deref(tmp_mbuf);
    }
    if(xml) {
        ezxml_free(xml);
    }
    return status;
}


empbx_user_entry_t *empbx_config_user_lookup(char *user_id) {
    empbx_user_entry_t *result = NULL;

    if(zstr(user_id)) return NULL;

    mtx_lock(global->users_map_mutex);
    result = empbx_hash_find(global->users_map, user_id);
    mtx_unlock(global->users_map_mutex);

    return result;
}

empbx_gateway_entry_t *empbx_config_gateway_lookup(char *name) {
    empbx_gateway_entry_t *result = NULL;

    if(zstr(name)) return NULL;

    mtx_lock(global->gateways_map_mutex);
    result = empbx_hash_find(global->gateways_map, name);
    mtx_unlock(global->gateways_map_mutex);

    return result;
}
