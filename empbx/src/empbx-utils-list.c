/**
 **
 ** (C)2025 akstel.org
 **/
#include <empbx.h>

typedef struct empbx_list_item_s {
    mem_destroy_h           *dh;
    void                    *data;
    struct empbx_list_item_s *next;
    struct empbx_list_item_s *prev;
} empbx_list_item_t;

struct empbx_list_s {
    uint32_t                 size;
    empbx_list_item_t        *head;
    empbx_list_item_t        *tail;
};

static void destructor__empbx_list_t(void *data) {
    empbx_list_t *list = (empbx_list_t *)data;
    empbx_list_item_t *t = NULL, *tt = NULL;

    t = list->head;
    while(t != NULL) {
        tt = t; t = t->next;
        tt = mem_deref(tt);
    }

}

static void destructor__empbx_list_item_t(void *data) {
    empbx_list_item_t *item = (empbx_list_item_t *)data;

    if(!item) { return; }

    if(item->dh) {
        item->dh(item->data);
        item->data = NULL;
    }
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// public
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
empbx_status_t empbx_list_create(empbx_list_t **list) {
    empbx_status_t status = EMPBX_STATUS_SUCCESS;
    empbx_list_t *list_local = NULL;

    list_local = mem_zalloc(sizeof(empbx_list_t), destructor__empbx_list_t);
    if(!list_local) {
        mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
    }

    *list = list_local;
out:
    return status;
}

/**
 * Insert item
 *
 * @param list  - the list
 * @param pos   - 0 = head, >list-size = tail, or certain posiotion
 * @param data  - some data
 * @param dh    - destroy handler
 *
 * @return success or error
 **/
empbx_status_t empbx_list_add(empbx_list_t *list, uint32_t pos, void *data, mem_destroy_h *dh) {
    empbx_status_t status = EMPBX_STATUS_SUCCESS;
    empbx_list_item_t *item = NULL, *target = NULL;

    if(!list || !data) {
        return EMPBX_STATUS_INVALID_ARGUMENT;
    }

    if(list->size == 0) {
        item = mem_zalloc(sizeof(empbx_list_item_t), destructor__empbx_list_item_t);
        if(!item) {
            log_mem_fail();
            return EMPBX_STATUS_MEM_FAIL;
        }

        item->dh = dh;
        item->data = data;
        item->next = NULL;
        item->prev = NULL;
        list->head = item;
        list->tail = item;
        list->size = 1;

        return EMPBX_STATUS_SUCCESS;
    }

    /* head */
    if(pos == 0) {
        item = mem_zalloc(sizeof(empbx_list_item_t), destructor__empbx_list_item_t);
        if(!item) {
            log_mem_fail();
            return EMPBX_STATUS_MEM_FAIL;
        }

        item->dh = dh;
        item->data = data;
        item->next = list->head;
        list->head->prev = item;
        list->head = item;
        list->size = (list->size + 1);

        return EMPBX_STATUS_SUCCESS;
    }

    /* tail */
    if(pos >= list->size) {
        item = mem_zalloc(sizeof(empbx_list_item_t), destructor__empbx_list_item_t);
        if(!item) {
            log_mem_fail();
            return EMPBX_STATUS_MEM_FAIL;
        }

        item->dh = dh;
        item->data = data;
        item->next = NULL;

        list->tail->next = item;
        item->prev = list->tail;
        list->tail = item;
        list->size = (list->size + 1);

        return EMPBX_STATUS_SUCCESS;
    }

    /* certain position */
    target = list->head;
    for(uint32_t i = 0; i < list->size; i++) {
        if(pos == i) break;
        target = target->next;
    }
    if(!target) {
        return EMPBX_STATUS_FALSE;
    }

    item = mem_zalloc(sizeof(empbx_list_item_t), destructor__empbx_list_item_t);
    if(!item) {
        log_mem_fail();
        return EMPBX_STATUS_MEM_FAIL;
    }

    item->dh = dh;
    item->data = data;
    item->next = target;
    item->prev = target->prev;
    target->prev->next = item;
    target->prev = item;

    list->size = (list->size + 1);

    return EMPBX_STATUS_SUCCESS;
}

/**
 * Delete item
 *
 * @param list  - the list
 * @param pod   - the position
 *
 * @return data or NULL
 **/
void *empbx_list_del(empbx_list_t *list, uint32_t pos) {
    empbx_list_item_t *item = NULL;
    void *data = NULL;

    if(!list || pos < 0) {
        return NULL;
    }
    if(!list->size || pos >= list->size) {
        return NULL;
    }

    if(pos == 0) {
        item = list->head;
        list->head = item->next;
        if(list->head) list->head->prev = NULL;

        data = (item->dh ? NULL : item->data);
        item = mem_deref(item);

        list->size = (list->size - 1);
        if(!list->size) {
            list->tail = NULL;
            list->head = NULL;
        }

        return data;
    }

    item = list->head;
    for(uint32_t i = 0; i < list->size; i++) {
        if(i == pos) break;
        item = item->next;
    }
    if(!item) {
        return NULL;
    }

    if(item == list->tail) {
        list->tail = item->prev;
        list->tail->next = NULL;
    } else {
        if(item->prev) { item->prev->next = item->next; }
        if(item->next) { item->next->prev = item->prev; }
    }

    data = (item->dh ? NULL : item->data);
    mem_deref(item);

    list->size = (list->size - 1);
    if(!list->size) {
        list->tail = NULL;
        list->head = NULL;
    }

    return data;
}

/**
 * Get item
 *
 * @param list  - the list
 * @param pos   - the position
 *
 * @return success or error
 **/
void *empbx_list_get(empbx_list_t *list, uint32_t pos) {
    empbx_list_item_t *item = NULL;

    if(!list || pos < 0) {
        return NULL;
    }

    if(list->size == 0 || pos >= list->size) {
        return NULL;
    }

    item = list->head;
    for(uint32_t i = 0; i < list->size; i++) {
        if(i == pos) break;
        item = item->next;
    }

    return(item ? item->data : NULL);
}


/**
 * remove item from list and return
 * result should be destoyed manually!
 *
 * @param list  - the list
 * @return item data
 **/
void *empbx_list_pop(empbx_list_t *list) {
    void *data = NULL;
    empbx_list_item_t *item = NULL;

    if(!list || !list->size) {
        return NULL;
    }

    item = list->head;
    list->head = item->next;
    if(list->head) list->head->prev = NULL;

    data = item->data;

    // to avoid data destroying
    item->dh = NULL;
    item = mem_deref(item);

    list->size = (list->size - 1);
    if(!list->size) {
        list->tail = NULL;
        list->head = NULL;
    }

    return data;
}

/**
 * Foreach list
 * callback will be called for each item
 *
 * @param list      - the list
 * @param callback  - callback (pos, data, udata)
 * @param udata     - user date
 *
 * @return success or error
 **/
empbx_status_t empbx_list_foreach(empbx_list_t *list, void (*callback)(uint32_t, void *, void *), void *udata) {
    empbx_list_item_t *target = NULL;

    if(!list || !callback) {
        return EMPBX_STATUS_FALSE;
    }
    if(list->size == 0) {
        return EMPBX_STATUS_SUCCESS;
    }

    target = list->head;
    for(uint32_t i = 0; i < list->size; i++) {
        if(!target) { break; }
        callback(i, target->data, udata);
        target = target->next;
    }

    return EMPBX_STATUS_SUCCESS;
}

/**
 * Clear list
 * callback will be called before the data be destroyed
 *
 * @param list      - the list
 * @param callback  - callback (pos, data, udata) or NULL
 * @param udata     - user date
 *
 * @return success or error
 **/
empbx_status_t empbx_list_clear(empbx_list_t *list, void (*callback)(uint32_t, void *, void *), void *udata) {
    empbx_list_item_t *next = NULL;
    empbx_list_item_t *curr = NULL;

    if(!list) {
        return EMPBX_STATUS_FALSE;
    }
    if(list->size == 0) {
        return EMPBX_STATUS_SUCCESS;
    }

    curr = list->head;
    for(uint32_t i = 0; i < list->size; i++) {
        next = curr->next;
        if(callback) { callback(i, curr->data, udata); }
        curr = mem_deref(curr);

        if(!next) { break; }
        curr = next;
    }

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;

    return EMPBX_STATUS_SUCCESS;
}

/**
 * Find a certain item
 * callback uses as a compare function
 *
 * @param list      - the list
 * @param result    - the seach result
 * @param callback  - callback (pos, data)
 * @param udata     - user data
 *
 * @return success or some error
 **/
empbx_status_t empbx_list_find(empbx_list_t *list, empbx_list_find_t *result, bool (*callback)(uint32_t, void *, void *), void *udata) {
    empbx_status_t status = EMPBX_STATUS_NOT_FOUND;
    empbx_list_item_t *target = NULL;

    if(!list || !result || !callback) {
        return EMPBX_STATUS_FALSE;
    }
    if(list->size == 0) {
        return EMPBX_STATUS_NOT_FOUND;
    }

    target = list->head;
    for(uint32_t i = 0; i < list->size; i++) {
        if(!target) { break; }

        if(callback(i, target->data, udata)) {
            result->pos = i;
            result->item = target;
            result->data = target->data;
            status = EMPBX_STATUS_SUCCESS;
            break;
        }

        target = target->next;
    }

    return status;
}

/**
 * List size
 *
 * @param list  - the list
 *
 * @return the size
 **/
uint32_t empbx_list_get_size(empbx_list_t *list) {
    if(!list) {
        return 0;
    }
    return list->size;
}

/**
 * Check on empty
 *
 * @param list  - the list
 *
 * @return true/false
 **/
bool empbx_list_is_empty(empbx_list_t *list) {
    if(!list) {
        return false;
    }
    return (list->size > 0 ? false : true);
}
