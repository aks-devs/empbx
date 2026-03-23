/**
 **
 ** (C)2025 akstel.org
 **/
#ifndef EMPBX_UTILS_HTABLE_H
#define EMPBX_UTILS_HTABLE_H
#include <empbx-core.h>

typedef enum {
    EMPBX_HASHTABLE_FLAG_NONE        = 0,
    EMPBX_HASHTABLE_FLAG_FREE_KEY    = (1 << 0),
    EMPBX_HASHTABLE_FLAG_FREE_VALUE  = (1 << 1),
    EMPBX_HASHTABLE_DUP_CHECK        = (1 << 2)
} empbx_hashtable_flag_t;

typedef struct empbx_hashtable empbx_hash_t;
typedef struct empbx_hashtable empbx_inthash_t;
typedef struct empbx_hashtable_iterator empbx_hash_index_t;
typedef void (*empbx_hashtable_destructor_t)(void *ptr);

#define empbx_hash_create(_hash) empbx_hash_create_case(_hash, true)
#define empbx_hash_create_nocase(_hash) empbx_hash_create_case(_hash, false)
#define empbx_hash_insert(_h, _k, _d) empbx_hash_insert_destructor(_h, _k, _d, NULL)

empbx_status_t empbx_hash_create_case(empbx_hash_t **hash, bool case_sensitive);
empbx_status_t empbx_hash_insert_destructor(empbx_hash_t *hash, const char *key, const void *data, empbx_hashtable_destructor_t destructor);
empbx_status_t empbx_hash_insert_ex(empbx_hash_t *hash, const char *key, const void *data, bool destroy_val);

unsigned int empbx_hash_size(empbx_hash_t *hash);
void *empbx_hash_delete(empbx_hash_t *hash, const char *key);
void *empbx_hash_find(empbx_hash_t *hash, const char *key);
bool empbx_hash_is_empty(empbx_hash_t *hash);
empbx_hash_index_t *empbx_hash_first(empbx_hash_t *hash);
empbx_hash_index_t *empbx_hash_first_iter(empbx_hash_t *hash, empbx_hash_index_t *hi);
empbx_hash_index_t *empbx_hash_next(empbx_hash_index_t **hi);
void empbx_hash_this(empbx_hash_index_t *hi, const void **key, size_t *klen, void **val);
void empbx_hash_this_val(empbx_hash_index_t *hi, void *val);

void empbx_hash_iter_free(empbx_hash_index_t *hi);

#define empbx_inthash_insert(_h, _k, _d) empbx_inthash_insert_ex(_h, _k, _d, false)

empbx_status_t empbx_inthash_init(empbx_inthash_t **hash);
empbx_status_t empbx_inthash_insert_ex(empbx_inthash_t *hash, uint32_t key, const void *data, bool destroy_val);
void *empbx_core_inthash_delete(empbx_inthash_t *hash, uint32_t key);
void *empbx_core_inthash_find(empbx_inthash_t *hash, uint32_t key);


#endif
