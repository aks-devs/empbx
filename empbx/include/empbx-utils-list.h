/**
 **
 ** (C)2025 akstel.org
 **/
#ifndef EMPBX_UTILS_LIST_H
#define EMPBX_UTILS_LIST_H
#include <empbx-core.h>

typedef struct empbx_list_s  empbx_list_t;
typedef struct {
    void        *data;
    void        *item;
    uint32_t    pos;
} empbx_list_find_t;

#define empbx_list_add_head(l, data, dh) empbx_list_add(l, 0, data, dh)
#define empbx_list_add_tail(l, data, dh) empbx_list_add(l, 0xffffffff, data, dh)

empbx_status_t empbx_list_create(empbx_list_t **list);
empbx_status_t empbx_list_add(empbx_list_t *list, uint32_t pos, void *data, mem_destroy_h *dh);
void *empbx_list_del(empbx_list_t *list, uint32_t pos);
void *empbx_list_get(empbx_list_t *list, uint32_t pos);
void *empbx_list_pop(empbx_list_t *list);
empbx_status_t empbx_list_foreach(empbx_list_t *list, void (*callback)(uint32_t, void *, void *), void *udata);
empbx_status_t empbx_list_clear(empbx_list_t *list, void (*callback)(uint32_t, void *, void *), void *udata);
empbx_status_t empbx_list_find(empbx_list_t *list, empbx_list_find_t *resut, bool (*callback)(uint32_t, void *, void *), void *udata);
uint32_t empbx_list_get_size(empbx_list_t *list);
bool empbx_list_is_empty(empbx_list_t *list);

#endif
