/**
 **
 ** (C)2026 akstel.org
 **/
#ifndef EMPBX_UTILS_SIP_REPLY_H
#define EMPBX_UTILS_SIP_REPLY_H
#include <empbx-re.h>

int empbx_treplyf(struct sip_strans **stp, struct mbuf **mbp, struct sip *sip, const struct sip_msg *msg, bool rec_route, uint16_t scode, const char *reason, const char *fmt, ...);
int empbx_treply(struct sip_strans **stp, struct sip *sip,  const struct sip_msg *msg, uint16_t scode, const char *reason);

int empbx_replyf(struct sip *sip, const struct sip_msg *msg, uint16_t scode, const char *reason, const char *fmt, ...);
int empbx_reply(struct sip *sip, const struct sip_msg *msg, uint16_t scode, const char *reason);


#endif
