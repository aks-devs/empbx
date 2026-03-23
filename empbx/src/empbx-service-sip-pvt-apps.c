/**
 **
 ** (C)2026 akstel.org
 **/
#include <empbx.h>
#include "empbx-service-sip-pvt.h"

extern empbx_global_t *global;
extern sipd_runtime_t sipd_runtime;

static void destructor__empbx_dialplan_app_t(void *arg) {
    empbx_dialplan_app_t *obj = (empbx_dialplan_app_t *)arg;

    log_debug("delete application (%p) [name=%s, refs=%d]", obj, obj->name, obj->refs);

    if(obj->refs && obj->mutex) {
        bool flw = true;
        while(flw) {
            mtx_lock(obj->mutex);
            flw = obj->refs > 0;
            mtx_unlock(obj->mutex);
            sys_msleep(250);
        }
    }

    if(obj->unload_h) {
        obj->unload_h();
    }

    mem_deref(obj->name);
    mem_deref(obj->mutex);

    log_debug("application deleted (%p)", obj);
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// public
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
empbx_status_t empbx_dialplan_application_register(const char *name, empbx_dialplan_app_unload_h *uh, empbx_dialplan_app_perform_h *ph) {
    empbx_status_t status = EMPBX_STATUS_SUCCESS;
    empbx_dialplan_app_t *entry = NULL;

    if(zstr(name) || !ph) {
        return EMPBX_STATUS_FALSE;
    }

    entry = mem_zalloc(sizeof(empbx_dialplan_app_t), destructor__empbx_dialplan_app_t);
    if(!entry) {
        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
    }

    if(mutex_alloc(&entry->mutex) != LIBRE_SUCCESS) {
        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
    }
    if(str_dup(&entry->name, name)) {
        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
    }

    entry->unload_h = uh;
    entry->perform_h = ph;

    mtx_lock(sipd_runtime.apps_map_mutex);
    status = empbx_hash_insert_ex(sipd_runtime.apps_map, entry->name, entry, true);
    mtx_unlock(sipd_runtime.apps_map_mutex);

    log_debug("Application registered (%s)", entry->name);

out:
    if(status != EMPBX_STATUS_SUCCESS)  {
        mem_deref(entry);
    }
    return status;
}

empbx_dialplan_app_t *empbx_dialplan_application_lookup(char *name) {
    empbx_dialplan_app_t *entry = NULL;

    if(zstr(name) || !sipd_runtime.apps_map) {
        return NULL;
    }

    mtx_lock(sipd_runtime.apps_map_mutex);
    entry = empbx_hash_find(sipd_runtime.apps_map, name);
    if(entry && entry->mutex) {
        mtx_lock(entry->mutex);
        entry->refs++;
        mtx_unlock(entry->mutex);
    }
    mtx_unlock(sipd_runtime.apps_map_mutex);

    return entry;
}

bool empbx_dialplan_application_take(empbx_dialplan_app_t *app) {
    if(!app || !app->mutex) return false;
    mtx_lock(app->mutex);
    app->refs++;
    mtx_unlock(app->mutex);
    return true;
}

void empbx_dialplan_application_release(empbx_dialplan_app_t *app) {
    if(app) {
        mtx_lock(app->mutex);
        if(app->refs > 0) app->refs--;
        mtx_unlock(app->mutex);
    }
}
