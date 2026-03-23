/**
 **
 ** (C)2025 akstel.org
 **/
#include <empbx.h>
#include <math.h>

/*
 * Copyright (c) 2002, Christopher Clark
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * * Neither the name of the original author; nor the names of any contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
  Credit for primes table: Aaron Krowne
  http://br.endernet.org/~akrowne/
  http://planetmath.org/encyclopedia/GoodHashTablePrimes.html
 */
struct empbx_hashtable_entry {
    void                        *k, *v;
    unsigned int                 h;
    empbx_hashtable_flag_t        flags;
    empbx_hashtable_destructor_t  destructor;
    struct empbx_hashtable_entry  *next;
};
struct empbx_hashtable_iterator {
    unsigned int                pos;
    struct empbx_hashtable_entry  *e;
    struct empbx_hashtable        *h;
};
struct empbx_hashtable {
    unsigned int                tablelength;
    struct empbx_hashtable_entry  **table;
    unsigned int                entrycount;
    unsigned int                loadlimit;
    unsigned int                primeindex;
    unsigned int (*hashfn) (void *k);
    int (*eqfn) (void *k1, void *k2);
};

static const unsigned int primes[] = {
    53, 97, 193, 389,
    769, 1543, 3079, 6151,
    12289, 24593, 49157, 98317,
    196613, 393241, 786433, 1572869,
    3145739, 6291469, 12582917, 25165843,
    50331653, 100663319, 201326611, 402653189,
    805306457, 1610612741
};
const unsigned int prime_table_length = (sizeof (primes) / sizeof (primes[0]));
const float max_load_factor = 0.65f;
typedef struct empbx_hashtable empbx_hashtable_t;
typedef struct empbx_hashtable_iterator empbx_hashtable_iterator_t;

static inline unsigned int hash(struct empbx_hashtable *h, void *k) {
    /* Aim to protect against poor hash functions by adding logic here - logic taken from java 1.4 hashtable source */
    unsigned int i = h->hashfn(k);
    i += ~(i << 9);
    i ^= ((i >> 14) | (i << 18)); /* >>> */
    i += (i << 4);
    i ^= ((i >> 10) | (i << 22)); /* >>> */
    return i;
}

static __inline__ unsigned int indexFor(unsigned int tablelength, unsigned int hashvalue) {
    return (hashvalue % tablelength);
}


/* https://code.google.com/p/stringencoders/wiki/PerformanceAscii
   http://www.azillionmonkeys.com/qed/asmexample.html
 */
static uint32_t c_tolower(uint32_t eax) {
    uint32_t ebx = (0x7f7f7f7ful & eax) + 0x25252525ul;
    ebx = (0x7f7f7f7ful & ebx) + 0x1a1a1a1aul;
    ebx = ((ebx & ~eax) >> 2) & 0x20202020ul;
    return eax + ebx;
}

static void desctuctor__empbx_hashtable_t(void *ptr) {
    struct empbx_hashtable *ht = (struct empbx_hashtable *)ptr;
    struct empbx_hashtable_entry **table = ht->table;
    struct empbx_hashtable_entry *e=NULL, *f=NULL;
    uint32_t i = 0;

//    log_debug("destroying hastable: htable=%p (tablelength=%d)", ht, ht->tablelength);

    for(i = 0; i < ht->tablelength; i++) {
        e = table[i];
        while(NULL != e) {
            f = e;
            e = e->next;

            if(f->flags & EMPBX_HASHTABLE_FLAG_FREE_KEY) {
                f->k = mem_deref(f->k);
            }

            if(f->flags & EMPBX_HASHTABLE_FLAG_FREE_VALUE) {
                f->v = mem_deref(f->v);
            } else if (f->destructor) {
                f->destructor(f->v);
                f->v = NULL;
            }

            f = mem_deref(f);
        }
    }

    ht->table = mem_deref(ht->table);

//    log_debug("hastable destroyed: htable=%p", ht);
}

// ----------------------------------------------------------------------------------------------------------------------------------------------
/**
 **
 **/
empbx_status_t empbx_hashtable_create(empbx_hashtable_t **hp, unsigned int minsize, unsigned int (*hashf) (void*), int (*eqf) (void*, void*)) {
    unsigned int pindex = 0, size = primes[0];
    empbx_hashtable_t *h = NULL;

    /* Check requested hashtable isn't too large */
    if(minsize > (1u << 30)) {
        *hp = NULL;
        return EMPBX_STATUS_FALSE;
    }

    /* Enforce size as prime */
    for(pindex = 0; pindex < prime_table_length; pindex++) {
        if(primes[pindex] > minsize) {
            size = primes[pindex];
            break;
        }
    }

    h = mem_zalloc(sizeof(empbx_hashtable_t), desctuctor__empbx_hashtable_t);
    if(!h) { log_mem_fail(); return EMPBX_STATUS_MEM_FAIL; }

    h->table = mem_zalloc((size * sizeof(struct wstk_hashtable_entry *)), NULL);
    if(!h->table) { log_mem_fail(); return EMPBX_STATUS_MEM_FAIL; }

    h->tablelength = size;
    h->primeindex = pindex;
    h->entrycount = 0;
    h->hashfn = hashf;
    h->eqfn = eqf;
    h->loadlimit = (unsigned int) ceil(size * max_load_factor);

    *hp = h;

    return EMPBX_STATUS_SUCCESS;
}

/**
 **
 **/
static int hashtable_expand(empbx_hashtable_t *h) {
    /* Double the size of the table to accommodate more entries */
    struct empbx_hashtable_entry **newtable = NULL;
    struct empbx_hashtable_entry *e = NULL;
    struct empbx_hashtable_entry **pE = NULL;
    unsigned int newsize, i, index;

    /* Check we're not hitting max capacity */
    if(h->primeindex == (prime_table_length - 1)) {
        return 0;
    }

    newsize = primes[++(h->primeindex)];
    newtable = mem_zalloc(newsize * sizeof(struct empbx_hashtable_entry *), NULL);
    if(newtable != NULL) {
        /* This algorithm is not 'stable'. ie. it reverses the list when it transfers entries between the tables */
        for(i = 0; i < h->tablelength; i++) {
            while(NULL != (e = h->table[i])) {
                h->table[i] = e->next;
                index = indexFor(newsize, e->h);
                e->next = newtable[index];
                newtable[index] = e;
            }
        }
        mem_deref(h->table);
        h->table = newtable;
    } else {
        /* Plan B: realloc instead */
        newtable = h->table;
        newtable = mem_realloc(newtable, (newsize * sizeof(struct empbx_hashtable_entry *)));
        if(!newtable) {
            log_mem_fail();
            (h->primeindex)--;
            return 0;
        }

        h->table = newtable;
        memset(&newtable[h->tablelength], 0x0, ((newsize - h->tablelength) * sizeof(struct empbx_hashtable_entry *)));

        for(i = 0; i < h->tablelength; i++) {
            for(pE = &(newtable[i]), e = *pE; e != NULL; e = *pE) {
                index = indexFor(newsize, e->h);
                if(index == i) {
                    pE = &(e->next);
                } else {
                    *pE = e->next;
                    e->next = newtable[index];
                    newtable[index] = e;
                }
            }
        }
    }

    h->tablelength = newsize;
    h->loadlimit = (unsigned int) ceil(newsize * max_load_factor);

    return -1;
}

/**
 **
 **/
unsigned int empbx_hashtable_count(empbx_hashtable_t *h) {
    return h->entrycount;
}

/**
 **
 **/
static void * _empbx_hashtable_remove(empbx_hashtable_t *h, void *k, unsigned int hashvalue, unsigned int index) {
    /* TODO: consider compacting the table when the load factor drops enough, or provide a 'compact' method. */

    struct empbx_hashtable_entry *e=NULL;
    struct empbx_hashtable_entry **pE=NULL;
    void *v;

    pE = &(h->table[index]);
    e = *pE;
    while (NULL != e) {
        /* Check hash value to short circuit heavier comparison */
        if ((hashvalue == e->h) && (h->eqfn(k, e->k))) {
            *pE = e->next;
            h->entrycount--;
            v = e->v;
            if(e->flags & EMPBX_HASHTABLE_FLAG_FREE_KEY) {
                e->k = mem_deref(e->k);
            }
            if(e->flags & EMPBX_HASHTABLE_FLAG_FREE_VALUE) {
                v = mem_deref(e->v);
            } else if(e->destructor) {
                e->destructor(e->v);
                v = e->v = NULL;
            }
            e = mem_deref(e);
            return v;
        }
        pE = &(e->next);
        e = e->next;
    }
    return NULL;
}

/**
 **
 **/
int empbx_hashtable_insert_destructor(empbx_hashtable_t *h, void *k, void *v, empbx_hashtable_flag_t flags, empbx_hashtable_destructor_t destructor) {
    struct empbx_hashtable_entry *e=NULL;
    unsigned int hashvalue = hash(h, k);
    unsigned index = indexFor(h->tablelength, hashvalue);

    if (flags & EMPBX_HASHTABLE_DUP_CHECK) {
        _empbx_hashtable_remove(h, k, hashvalue, index);
    }

    if (++(h->entrycount) > h->loadlimit) {
        /* Ignore the return value. If expand fails, we should
         * still try cramming just this value into the existing table
         * -- we may not have memory for a larger table, but one more
         * element may be ok. Next time we insert, we'll try expanding again.*/
        hashtable_expand(h);
        index = indexFor(h->tablelength, hashvalue);
    }

    e = mem_zalloc(sizeof(struct empbx_hashtable_entry), NULL);
    if(!e) {
        log_mem_fail();
        --(h->entrycount);
        return 0;
    }

    e->h = hashvalue;
    e->k = k;
    e->v = v;
    e->flags = flags;
    e->destructor = destructor;
    e->next = h->table[index];
    h->table[index] = e;

    return -1;
}

/**
 ** returns value associated with key
 **/
void *empbx_hashtable_search(empbx_hashtable_t *h, void *k) {
    struct empbx_hashtable_entry *e = NULL;
    unsigned int hashvalue, index;

    hashvalue = hash(h, k);
    index = indexFor(h->tablelength, hashvalue);
    e = h->table[index];

    while (NULL != e) {
        /* Check hash value to short circuit heavier comparison */
        if((hashvalue == e->h) && (h->eqfn(k, e->k))) {
            return e->v;
        }
        e = e->next;
    }

    return NULL;
}

/**
 ** returns value associated with key
 **/
void *empbx_hashtable_remove(empbx_hashtable_t *h, void *k) {
    unsigned int hashvalue = hash(h, k);
    return _empbx_hashtable_remove(h, k, hashvalue, indexFor(h->tablelength, hashvalue));
}

/**
 **
 **/
empbx_hashtable_iterator_t *empbx_hashtable_next(empbx_hashtable_iterator_t **iP) {
    empbx_hashtable_iterator_t *i = *iP;

    if(i->e) {
        if((i->e = i->e->next) != 0) {
            return i;
        } else {
            i->pos++;
        }
    }
    while(i->pos < i->h->tablelength && !i->h->table[i->pos]) {
        i->pos++;
    }
    if(i->pos >= i->h->tablelength) {
        goto end;
    }
    if((i->e = i->h->table[i->pos]) != 0) {
        return i;
    }

end:
    mem_deref(i);
    *iP = NULL;

    return NULL;
}

/**
 **
 **/
empbx_hashtable_iterator_t *empbx_hashtable_first_iter(empbx_hashtable_t *h, empbx_hashtable_iterator_t *it) {
    empbx_hashtable_iterator_t *iterator = NULL;

    if(it) {
        iterator = it;
    } else {
        iterator = mem_zalloc(sizeof(empbx_hashtable_iterator_t), NULL);
        if(!iterator) {
            log_mem_fail();
            return NULL;
        }
    }

    iterator->pos = 0;
    iterator->e = NULL;
    iterator->h = h;

    return empbx_hashtable_next(&iterator);
}

/**
 **
 **/
void empbx_hashtable_this_val(empbx_hashtable_iterator_t *i, void *val) {
    if(!i) {
        return;
    }
    if(i->e) {
        i->e->v = val;
    }
}

/**
 **
 **/
void empbx_hashtable_this(empbx_hashtable_iterator_t *i, const void **key, size_t *klen, void **val) {
    if(!i) {
        if(key) *key = NULL;
        if(klen) *klen = 0;
        if(val) *val = NULL;
        return;
    }

    if (i->e) {
        if (key) {
            *key = i->e->k;
        }
        if (klen) {
            *klen = (int) strlen(i->e->k);
        }
        if (val) {
            *val = i->e->v;
        }
    } else {
        if (key) {
            *key = NULL;
        }
        if (klen) {
            *klen = 0;
        }
        if (val) {
            *val = NULL;
        }
    }
}

// ===================================================================================================================================================================
// PUB
// ===================================================================================================================================================================

static inline uint32_t empbx_hash_default_int(void *ky) {
    uint32_t x = *((uint32_t *) ky);
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x);
    return x;
}

static inline int empbx_hash_equalkeys_int(void *k1, void *k2) {
    return *(uint32_t *) k1 == *(uint32_t *) k2;
}

static inline int empbx_hash_equalkeys(void *k1, void *k2) {
    return strcmp((char *) k1, (char *) k2) ? 0 : 1;
}

static inline int empbx_hash_equalkeys_ci(void *k1, void *k2) {
    return strcasecmp((char *) k1, (char *) k2) ? 0 : 1;
}

static inline uint32_t empbx_hash_default(void *ky) {
    unsigned char *str = (unsigned char *) ky;
    uint32_t hash = 0;
    int c;

    while ((c = *str)) {
        str++;
        hash = c + (hash << 6) + (hash << 16) - hash;
    }

    return hash;
}

static inline uint32_t empbx_hash_default_ci(void *ky) {
    unsigned char *str = (unsigned char *) ky;
    uint32_t hash = 0;
    int c;

    while ((c = c_tolower(*str))) {
        str++;
        hash = c + (hash << 6) + (hash << 16) - hash;
    }
    return hash;
}

// ===================================================================================================================================================================

empbx_status_t empbx_hash_create_case(empbx_hash_t **hash, bool case_sensitive) {
    if (case_sensitive) {
        return empbx_hashtable_create(hash, 16, empbx_hash_default, empbx_hash_equalkeys);
    } else {
        return empbx_hashtable_create(hash, 16, empbx_hash_default_ci, empbx_hash_equalkeys_ci);
    }
}

empbx_status_t empbx_hash_insert_destructor(empbx_hash_t *hash, const char *key, const void *data, empbx_hashtable_destructor_t destructor) {
    int r = 0;
    char *lkey = NULL;

    if(str_dup(&lkey, key)) {
        log_mem_fail();
        return EMPBX_STATUS_FALSE;
    }

    r = empbx_hashtable_insert_destructor(hash, (void *)lkey, (void *)data, EMPBX_HASHTABLE_FLAG_FREE_KEY | EMPBX_HASHTABLE_DUP_CHECK, destructor);
    return (r ? EMPBX_STATUS_SUCCESS : EMPBX_STATUS_FALSE);
}

empbx_status_t empbx_hash_insert_ex(empbx_hash_t *hash, const char *key, const void *data, bool destroy_val) {
    int r = 0;
    char *lkey = NULL;
    int flags = (EMPBX_HASHTABLE_FLAG_FREE_KEY | EMPBX_HASHTABLE_DUP_CHECK);

    if(destroy_val) {
        flags |= EMPBX_HASHTABLE_FLAG_FREE_VALUE;
    }
    if(str_dup(&lkey, key)) {
        log_mem_fail();
        return EMPBX_STATUS_FALSE;
    }
    r = empbx_hashtable_insert_destructor(hash, (void *)lkey, (void *)data, flags, NULL);
    return (r ? EMPBX_STATUS_SUCCESS : EMPBX_STATUS_FALSE);
}

void *empbx_hash_delete(empbx_hash_t *hash, const char *key) {
    return empbx_hashtable_remove(hash, (void *) key);
}

unsigned int empbx_hash_size(empbx_hash_t *hash) {
    return empbx_hashtable_count(hash);
}

void *empbx_hash_find(empbx_hash_t *hash, const char *key) {
    return empbx_hashtable_search(hash, (void *) key);
}

bool empbx_hash_is_empty(empbx_hash_t *hash) {
    empbx_hash_index_t *hi = empbx_hash_first(hash);
    if(hi) {
        hi = mem_deref(hi);
        return false;
    }
    return true;
}

empbx_hash_index_t *empbx_hash_first(empbx_hash_t *hash) {
    return empbx_hashtable_first_iter(hash, NULL);
}

empbx_hash_index_t *empbx_hash_first_iter(empbx_hash_t *hash, empbx_hash_index_t *hi) {
    return empbx_hashtable_first_iter(hash, hi);
}

empbx_hash_index_t *empbx_hash_next(empbx_hash_index_t **hi) {
    return empbx_hashtable_next(hi);
}

void empbx_hash_this(empbx_hash_index_t *hi, const void **key, size_t *klen, void **val) {
    empbx_hashtable_this(hi, key, klen, val);
}

void empbx_hash_this_val(empbx_hash_index_t *hi, void *val) {
    empbx_hashtable_this_val(hi, val);
}

void empbx_hash_iter_free(empbx_hash_index_t *hi) {
    mem_deref(hi);
}
// -----------------------------------------------------------------------------------------------------------------------------

empbx_status_t empbx_inthash_create(empbx_inthash_t **hash) {
    return empbx_hashtable_create(hash, 16, empbx_hash_default_int, empbx_hash_equalkeys_int);
}

empbx_status_t empbx_inthash_insert_ex(empbx_inthash_t *hash, uint32_t key, const void *data, bool destroy_val) {
    int flags = (EMPBX_HASHTABLE_FLAG_FREE_KEY | EMPBX_HASHTABLE_DUP_CHECK);
    uint32_t *k = NULL;
    int r = 0;

    if(destroy_val) { flags |= EMPBX_HASHTABLE_FLAG_FREE_VALUE; }

    k = mem_zalloc(sizeof(*k), NULL);
    if(!k) {
        log_mem_fail();
        return EMPBX_STATUS_MEM_FAIL;
    }

    *k = key;
    r = empbx_hashtable_insert_destructor(hash, k, (void *)data, flags, NULL);
    return (r ? EMPBX_STATUS_SUCCESS : EMPBX_STATUS_FALSE);
}

void *empbx_core_inthash_delete(empbx_inthash_t *hash, uint32_t key) {
    return empbx_hashtable_remove(hash, (void *)&key);
}

void *empbx_core_inthash_find(empbx_inthash_t *hash, uint32_t key) {
    return empbx_hashtable_search(hash, (void *)&key);
}
