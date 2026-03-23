/**
 **
 ** (C)2026 akstel.org
 **/
#ifndef EMPBX_RE_H
#define EMPBX_RE_H
#include <re/re.h>
#include <re/rem.h>

#define LIBRE_SUCCESS  0

typedef struct uri uri_t;
typedef struct pl pl_t;
typedef struct mbuf mbuf_t;
typedef struct sa sa_t;
typedef struct ua ua_t;
typedef struct http_sock http_sock_t;
typedef struct http_conn http_conn_t;
typedef struct websock websock_t;
typedef struct sip_msg sip_msg_t;
typedef struct sip_hdr sip_hdr_t;
typedef struct sip_uas_auth sip_uas_auth_t;
typedef struct account account_t;
typedef struct call call_t;
typedef const struct audio audio_t;
typedef const struct video video_t;
typedef struct config_audio config_audio_t;
typedef struct httpauth_digest_resp httpauth_digest_resp_t;
typedef mtx_t  mutex_t;

#define mbuf_clean(mb) mb->pos=0; mbuf_fill(mb, 0x0, mb->end); mb->end=mb->pos=0;

void empbx_ua_destroy(ua_t **ua);
int empbx_re_debug_print_handler(const char *p, size_t size, void *arg);


#endif
