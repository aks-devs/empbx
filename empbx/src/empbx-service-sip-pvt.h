/**
 **
 ** (C)2025 akstel.org
 **/
#ifndef EMPBX_SERVICE_SIP_PVT_H
#define EMPBX_SERVICE_SIP_PVT_H
#include <empbx-core.h>

#define SIP_AUTH_HDR_NAME           "WWW-Authenticate"
#define REGS_CHECK_INTERVAL_SEC     60  // 1min
#define REG_DEFAULT_LIFETIME_SEC    300 // 5min
#define AUTH_NONCE_LIFETIME_SEC     300
#define AUTH_NONCE_LMIN_LEN         30
#define SESS_DTMF_QUEUE_SIZE        128


#define HC_NORMAL_CLEARING          "NORMAL_CLEARING"
#define HC_SUBSCRIBER_ABSENT        "SUBSCRIBER_ABSENT"
#define HC_CALL_REJECTED            "CALL_REJECTED"
#define HC_NO_ROUTE_DESTINATION     "NO_ROUTE_DESTINATION"
#define HC_USER_BUSY                "USER_BUSY"
#define HC_NORMAL_FALURE            "NORMAL_TEMPORARY_FAILURE"

typedef struct {
    mutex_t         *baresip_mutex;
    mutex_t         *regmap_users_mutex;
    mutex_t         *regmap_gateways_mutex;
    mutex_t         *regmap_sessions_mutex;
    mutex_t         *apps_map_mutex;
    empbx_hash_t    *regmap_users;         	    // userId => empbx_registration_user_t
    empbx_hash_t    *regmap_gateways;           // userId => empbx_registration_gateway_t
    empbx_hash_t    *regmap_sessions;           // sid => empbx_session_t
    empbx_hash_t    *apps_map;                  // name => empbx_dialplan_app_t
    bool            fl_ua_init_ok;
} sipd_runtime_t;

/* registrar */
empbx_status_t empbx_authentication_challenge_start(const char *realm, const sip_msg_t *msg);
empbx_status_t empbx_authentication_challenge_check(empbx_user_entry_t *user,  const sip_hdr_t *ahdr, const sip_msg_t *msg);

/* users */
void empbx_registration_user_delete(char *user_id);
empbx_status_t empbx_registration_user_update(empbx_registration_user_t *reg, char *contact, const sip_msg_t *sip_msg);
empbx_status_t empbx_registration_user_new(char *user_id, char *realm, char *contact, empbx_user_entry_t *user, const sip_msg_t *sip_msg);

/* gateways */
empbx_status_t empbx_registration_gateway_add(empbx_gateway_entry_t *entry);

/* sessions */
empbx_status_t empbx_session_intercom_new(empbx_session_t **nsess, call_t *lega, empbx_registration_user_t *to_user);
empbx_status_t empbx_session_inbound_new(empbx_session_t **nsess, call_t *lega, empbx_registration_user_t *to_user);
empbx_status_t empbx_session_outbound_new(empbx_session_t **nsess, call_t *lega, empbx_registration_gateway_t *to_gw, const char *dst_number);
void empbx_session_delete(char *sid);

/* applications */
empbx_status_t empbx_application_register__bridge();


#endif
