/**
 **
 ** (C)2026 akstel.org
 **/
#include <empbx.h>
#include "empbx-service-sip-pvt.h"

extern empbx_global_t *global;
extern sipd_runtime_t sipd_runtime;

static void destructor__empbx_session_t(void *arg) {
    empbx_session_t *obj = (empbx_session_t *)arg;

    log_debug("delete session (%p) [id=%s, refs=%d, from=%s, to=%s]", obj, obj->id, obj->refs, obj->lega_aor, obj->legb_aor);

    if(obj->refs && obj->mutex) {
        bool flw = true;
        while(flw) {
            mtx_lock(obj->mutex);
            flw = obj->refs > 0;
            mtx_unlock(obj->mutex);
            sys_msleep(250);
        }
    }

    if(!global->fl_shutdown) {
        mem_deref(obj->lega);
        mem_deref(obj->legb);
    }

    mem_deref(obj->id);
    mem_deref(obj->legb_aor);
    mem_deref(obj->lega_aor);
    mem_deref(obj->lega_contact);
    mem_deref(obj->mutex);

    log_debug("session deleted (%p)", obj);
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// public
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
empbx_session_t *empbx_session_lookup(char *sid) {
    empbx_session_t *entry = NULL;

    if(zstr(sid) || !sipd_runtime.regmap_sessions) {
        return NULL;
    }

    mtx_lock(sipd_runtime.regmap_sessions_mutex);
    entry = empbx_hash_find(sipd_runtime.regmap_sessions, sid);
    if(entry && entry->mutex) {
        mtx_lock(entry->mutex);
        entry->refs++;
        mtx_unlock(entry->mutex);
    }
    mtx_unlock(sipd_runtime.regmap_sessions_mutex);

    return entry;
}

bool empbx_session_take(empbx_session_t *sess) {
    if(!sess || !sess->mutex) return false;
    mtx_lock(sess->mutex);
    sess->refs++;
    mtx_unlock(sess->mutex);
    return true;
}

void empbx_session_release(empbx_session_t *sess) {
    if(sess) {
        mtx_lock(sess->mutex);
        if(sess->refs > 0) sess->refs--;
        mtx_unlock(sess->mutex);
    }
}

void empbx_session_delete(char *sid) {
    if(zstr(sid) || !sipd_runtime.regmap_sessions) {
        return;
    }

    mtx_lock(sipd_runtime.regmap_sessions_mutex);
    empbx_hash_delete(sipd_runtime.regmap_sessions, sid);
    mtx_unlock(sipd_runtime.regmap_sessions_mutex);
}


empbx_status_t empbx_session_inbound_new(empbx_session_t **nsess, call_t *lega, empbx_registration_user_t *to_user) {
    empbx_status_t status = EMPBX_STATUS_SUCCESS;
    empbx_session_t *sess = NULL;
    const char *peername;

    if(!lega || !to_user) {
        return EMPBX_STATUS_FALSE;
    }

    sess = mem_zalloc(sizeof(empbx_session_t), destructor__empbx_session_t);
    if(!sess) {
        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
    }
    if(mutex_alloc(&sess->mutex) != LIBRE_SUCCESS) {
        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
    }

    sess->refs = 0;
    sess->type = EMPBX_SESS_INBOUND;

    re_sdprintf(&sess->id, "%08x-%04x-%04x-%04x-%08x%04x", rand_u32(), rand_u16(), rand_u16(), rand_u16(), rand_u32(), rand_u16());
    if(zstr(sess->id)) {
        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
    }


    /* leg-a */
    sess->lega = lega;
    if(str_dup(&sess->lega_aor, call_peeruri(lega))) {
        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
    }

    peername = call_peername(lega);
    if(!zstr(peername)) {
        re_sdprintf(&sess->lega_contact, "\"%s\" %s", peername, sess->lega_aor);
    } else {
        str_dup(&sess->lega_contact, sess->lega_aor);
    }
    if(zstr(sess->lega_aor)) {
        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
    }


    /* leg-b */
    if(str_dup(&sess->legb_aor, to_user->contact)) {
        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
    }

    if(ua_connect(to_user->ua, &sess->legb, sess->lega_contact, sess->legb_aor, call_has_video(sess->lega) ? VIDMODE_ON : VIDMODE_OFF) != 0) {
        log_error("Unable to create leg-b (%s)", sess->legb_aor);
        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
    }

    sess->started = empbx_time_epoch_now();

    mtx_lock(sipd_runtime.regmap_sessions_mutex);
    status = empbx_hash_insert_ex(sipd_runtime.regmap_sessions, sess->id, sess, true);
    mtx_unlock(sipd_runtime.regmap_sessions_mutex);

    sess->refs++;
    *nsess = sess;
out:
    if(status == EMPBX_STATUS_SUCCESS) {
        log_debug("Session created (%s) [%s] => [%s] (%s)", sess->id, sess->lega_aor, sess->legb_aor, sess->lega_contact);
    } else {
        mem_deref(sess);
    }
    return status;
}


empbx_status_t empbx_session_intercom_new(empbx_session_t **nsess, call_t *lega, empbx_registration_user_t *to_user) {
    empbx_status_t status = EMPBX_STATUS_SUCCESS;
    empbx_session_t *sess = NULL;
    const char *peername;

    if(!lega || !to_user) {
        return EMPBX_STATUS_FALSE;
    }

    sess = mem_zalloc(sizeof(empbx_session_t), destructor__empbx_session_t);
    if(!sess) {
        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
    }
    if(mutex_alloc(&sess->mutex) != LIBRE_SUCCESS) {
        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
    }

    sess->refs = 0;
    sess->type = EMPBX_SESS_INTERCOM;

    re_sdprintf(&sess->id, "%08x-%04x-%04x-%04x-%08x%04x", rand_u32(), rand_u16(), rand_u16(), rand_u16(), rand_u32(), rand_u16());
    if(zstr(sess->id)) {
        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
    }


    /* leg-a */
    sess->lega = lega;
    if(str_dup(&sess->lega_aor, call_peeruri(lega))) {
        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
    }

    peername = call_peername(lega);
    if(!zstr(peername)) {
        re_sdprintf(&sess->lega_contact, "\"%s\" %s", peername, sess->lega_aor);
    } else {
        str_dup(&sess->lega_contact, sess->lega_aor);
    }
    if(zstr(sess->lega_aor)) {
        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
    }


    /* leg-b */
    if(str_dup(&sess->legb_aor, to_user->contact)) {
        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
    }

    if(ua_connect(to_user->ua, &sess->legb, sess->lega_contact, sess->legb_aor, call_has_video(sess->lega) ? VIDMODE_ON : VIDMODE_OFF) != 0) {
        log_error("Unable to create leg-b (%s)", sess->legb_aor);
        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
    }

    sess->started = empbx_time_epoch_now();

    mtx_lock(sipd_runtime.regmap_sessions_mutex);
    status = empbx_hash_insert_ex(sipd_runtime.regmap_sessions, sess->id, sess, true);
    mtx_unlock(sipd_runtime.regmap_sessions_mutex);

    sess->refs++;
    *nsess = sess;
out:
    if(status == EMPBX_STATUS_SUCCESS) {
        log_debug("Session created (%s) [%s] => [%s] (%s)", sess->id, sess->lega_aor, sess->legb_aor, sess->lega_contact);
    } else {
        mem_deref(sess);
    }
    return status;
}


empbx_status_t empbx_session_outbound_new(empbx_session_t **nsess, call_t *lega, empbx_registration_gateway_t *to_gw, const char *dst_number) {
    empbx_status_t status = EMPBX_STATUS_SUCCESS;
    empbx_session_t *sess = NULL;
    const char *peername;

    if(!lega || !to_gw) {
        return EMPBX_STATUS_FALSE;
    }

    sess = mem_zalloc(sizeof(empbx_session_t), destructor__empbx_session_t);
    if(!sess) {
        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
    }
    if(mutex_alloc(&sess->mutex) != LIBRE_SUCCESS) {
        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
    }

    sess->refs = 0;
    sess->type = EMPBX_SESS_OUTBOUND;

    re_sdprintf(&sess->id, "%08x-%04x-%04x-%04x-%08x%04x", rand_u32(), rand_u16(), rand_u16(), rand_u16(), rand_u32(), rand_u16());
    if(zstr(sess->id)) {
        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
    }


    /* leg-a */
    sess->lega = lega;
    if(str_dup(&sess->lega_aor, call_peeruri(lega))) {
        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
    }

    peername = call_peername(lega);
    if(!zstr(peername)) {
        re_sdprintf(&sess->lega_contact, "\"%s\" %s", peername, sess->lega_aor);
    } else {
        str_dup(&sess->lega_contact, sess->lega_aor);
    }
    if(zstr(sess->lega_aor)) {
        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
    }


    /* leg-b */
    account_t *acc = ua_account(to_gw->ua);
    uri_t *acc_luri = account_luri(acc);

    if(acc_luri) {
        if(re_sdprintf(&sess->legb_aor, "%r:%s@%r", &acc_luri->scheme, dst_number, &acc_luri->host)) {
            mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
        }
    } else {
        if(str_dup(&sess->legb_aor, call_localuri(lega))) {
            mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
        }
    }

    if(ua_connect(to_gw->ua, &sess->legb, sess->lega_contact, sess->legb_aor, call_has_video(sess->lega) ? VIDMODE_ON : VIDMODE_OFF) != 0) {
        log_error("Unable to create leg-b (%s)", sess->legb_aor);
        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
    }

    sess->started = empbx_time_epoch_now();

    mtx_lock(sipd_runtime.regmap_sessions_mutex);
    status = empbx_hash_insert_ex(sipd_runtime.regmap_sessions, sess->id, sess, true);
    mtx_unlock(sipd_runtime.regmap_sessions_mutex);

    sess->refs++;
    *nsess = sess;
out:
    if(status == EMPBX_STATUS_SUCCESS) {
        log_debug("Session created (%s) [%s] => [%s] (%s)", sess->id, sess->lega_aor, sess->legb_aor, sess->lega_contact);
    } else {
        mem_deref(sess);
    }
    return status;
}

