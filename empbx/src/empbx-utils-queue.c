/**
 **
 ** (C)2025 akstel.org
 **/
#include <empbx.h>

struct empbx_queue_s {
    mutex_t    *mutex;
    void       **data;
    uint32_t   size;
    uint32_t   in;
    uint32_t   out;
};

static void destructor__empbx_queue_t(void *data) {
    empbx_queue_t *queue = (empbx_queue_t *)data;

    if(!queue) { return; }

    mtx_lock(queue->mutex);
    if(queue->in) {
        for(uint32_t i = 0; i <= queue->in; i++) {
            void *dp = queue->data[i];
            if(dp) { mem_deref(dp); }
        }
    }
    mtx_unlock(queue->mutex);

    queue->data = mem_deref(queue->data);
    queue->mutex = mem_deref(queue->mutex);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// public
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
empbx_status_t empbx_queue_create(empbx_queue_t **queue, uint32_t size) {
    empbx_status_t status = EMPBX_STATUS_SUCCESS;
    empbx_queue_t *qlocal = NULL;

    if(!queue || !size) {
        return EMPBX_STATUS_FALSE;
    }

    qlocal = mem_zalloc(sizeof(empbx_queue_t), destructor__empbx_queue_t);
    if(!qlocal) {
        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
    }

    qlocal->data = mem_zalloc(sizeof(void *) * (size + 1), NULL);
    if(!qlocal->data) {
        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
    }

    if(mutex_alloc(&qlocal->mutex) != LIBRE_SUCCESS) {
        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
    }

    qlocal->in = 0;
    qlocal->out = 0;
    qlocal->size = size;

    *queue = qlocal;
out:
    if(status != EMPBX_STATUS_SUCCESS) {
        mem_deref(qlocal);
        *queue = NULL;
    }
    return status;
}

empbx_status_t empbx_queue_push(empbx_queue_t *queue, void *data) {
    empbx_status_t status = EMPBX_STATUS_QUEUE_FULL;

    if(!queue || !data) {
        return EMPBX_STATUS_FALSE;
    }

    mtx_lock(queue->mutex);
    if(queue->in < queue->size) {
        queue->data[queue->in] = data;
        queue->in++;

        status = EMPBX_STATUS_SUCCESS;
    }
    mtx_unlock(queue->mutex);

    return status;
}

void *empbx_queue_pop(empbx_queue_t *queue) {
    void *result = NULL;

    if(!queue) {
        return NULL;
    }

    mtx_lock(queue->mutex);
    if(queue->in > 0) {
        result = queue->data[queue->out];

        queue->data[queue->out] = NULL;
        queue->out++;

        if(queue->out > queue->in || queue->out >= queue->size) {
            queue->in = 0;
            queue->out = 0;
        }

    }
    mtx_unlock(queue->mutex);

    return result;
}

bool empbx_queue_is_empty(empbx_queue_t *queue) {
    bool result = false;

    if(!queue) {
        return false;
    }

    mtx_lock(queue->mutex);
    result = (queue->in == 0);
    mtx_unlock(queue->mutex);

    return result;
}

bool empbx_queue_is_full(empbx_queue_t *queue) {
    bool result = false;

    if(!queue) {
        return false;
    }

    mtx_lock(queue->mutex);
    result = (queue->in >= queue->size);
    mtx_unlock(queue->mutex);

    return result;
}
