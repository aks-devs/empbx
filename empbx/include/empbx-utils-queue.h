/**
 **
 ** (C)2025 akstel.org
 **/
#ifndef EMPBX_UTILS_QUEUE_H
#define EMPBX_UTILS_QUEUE_H
#include <empbx-core.h>

typedef struct empbx_queue_s empbx_queue_t;

empbx_status_t empbx_queue_create(empbx_queue_t **queue, uint32_t size);
empbx_status_t empbx_queue_push(empbx_queue_t *queue, void *data);
void *empbx_queue_pop(empbx_queue_t *queue);

bool empbx_queue_is_empty(empbx_queue_t *queue);
bool empbx_queue_is_full(empbx_queue_t *queue);


#endif
