/**
 **
 ** (C)2025 akstel.org
 **/
#include <empbx.h>

extern empbx_global_t *global;

empbx_status_t empbx_thread_launch(thrd_start_t func, void *udata) {
    thrd_t thr;
    int err = 0;

    if((err = thrd_create(&thr, func, udata)) != thrd_success) {
        return EMPBX_STATUS_FALSE;
    }

    mtx_lock(global->mutex);
    global->active_threds++;
    mtx_unlock(global->mutex);

    return EMPBX_STATUS_SUCCESS;
}

void empbx_thread_finished() {
    mtx_lock(global->mutex);
    if(global->active_threds > 0) global->active_threds--;
    mtx_unlock(global->mutex);
}

