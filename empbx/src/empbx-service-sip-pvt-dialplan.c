/**
 **
 ** (C)2026 akstel.org
 **/
#include <empbx.h>
#include "empbx-service-sip-pvt.h"

#define RE_BUF_SIZE    1024

extern empbx_global_t *global;
extern sipd_runtime_t sipd_runtime;

typedef struct {
    empbx_dialplan_lookup_prams_t   *lprams;
    char                            *dst_number_new;
} find_cb_prams_t;

static bool rlist_fnd_callback(uint32_t idx, void *idata, void *udata) {
    empbx_dialplan_rule_t *rule = (empbx_dialplan_rule_t *)idata;
    find_cb_prams_t *fparams = (find_cb_prams_t *)udata;
    char *rebuf = NULL, *tmp_target = NULL;
    bool found = false;

    if(!str_casecmp(rule->field, "dst-number")) {
        if(zstr(rule->expr)) goto out;

        if(!str_casecmp(rule->expr, fparams->lprams->dst_number)) {
            found = true;
        } else {
            int ok = 0, ovector[30] = {0};
            char substituted[RE_BUF_SIZE] = {0};
            empbx_regex_t *re = NULL;

            if(!(rebuf = mem_zalloc(RE_BUF_SIZE, NULL))) {
                log_mem_fail();
                goto out;
            }

            memcpy(rebuf, fparams->lprams->dst_number, MIN(RE_BUF_SIZE, strlen(fparams->lprams->dst_number)));

            if((ok = empbx_regex_perform(&re, rebuf, rule->expr, ovector, RE_ARRAY_SIZE(ovector)))) {
                if(!zstr(rule->app_data)) {
                    char *ptr = (char *)substituted;
                    empbx_perform_substitution(re, ok, rule->app_data, rebuf, ptr, sizeof(substituted), ovector);
                    if(ptr) str_dup(&tmp_target, ptr);
                }
                found = true;
                empbx_regex_free(re);
            }
        }
    }

    if(found) {
        if(!zstr(tmp_target)) {
            str_dup(&fparams->dst_number_new, tmp_target);
        } else {
            str_dup(&fparams->dst_number_new, rule->app_data);
        }
    }

out:
    mem_deref(rebuf);
    mem_deref(tmp_target);
    return found;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// public
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
empbx_status_t empbx_dialplan_lookup(empbx_dialplan_lookup_result_t *result, empbx_dialplan_lookup_prams_t *lparams) {
    empbx_status_t status = EMPBX_STATUS_NOT_FOUND;
    empbx_list_find_t fresult = {0};
    find_cb_prams_t fparams = {0};

    if(!result || !lparams) {
        return EMPBX_STATUS_FALSE;
    }

    fparams.lprams = lparams;
    if(empbx_list_find(global->dialplan_rules, &fresult, rlist_fnd_callback, (void *)&fparams) == EMPBX_STATUS_SUCCESS) {
        result->rule = (empbx_dialplan_rule_t *)fresult.data;
        result->dst_number = fparams.dst_number_new;
        status = EMPBX_STATUS_SUCCESS;
    }

    return status;
}

