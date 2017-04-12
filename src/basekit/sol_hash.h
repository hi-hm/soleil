#ifndef _SOL_HASH_H_
#define _SOL_HASH_H_ 1
#include <stddef.h>
#include "sol_common.h"

#define SOL_HASH_INIT_SIZE 8
#define SOL_HASH_RESIZE_MAX_LOOP 100

#define solHash_record_at_offset(r, o) (SolHashRecord*)(r + o)
#define solHash_grow(h) solHash_resize(h, h->size * 2)
#define solHash_record_extend(r) //

typedef size_t (*SolHashFunc)(void*);
typedef int (*SolHashEqualFunc)(void*, void*);

typedef struct SolHashRecord {
	void *k;
	void *v;
} SolHashRecord;

typedef struct SolHash {
	size_t size;
	size_t count;
	size_t mask;
	SolHashRecord *records;
	SolHashFunc hash_func1;
	SolHashFunc hash_func2;
	SolHashEqualFunc equal_func;
	void (*f_free_k)(void*);
	void (*f_free_v)(void*);
	int is_resizing;
} SolHash;

typedef struct SolHashIter {
	SolHash *hash;
	SolHashRecord *record;
	size_t c;
} SolHashIter;

SolHash* solHash_new();
void solHash_free(SolHash*);
int solHash_set_size(SolHash*, size_t);
int solHash_try_to_put(SolHash*, void*, void*);
int solHash_resize(SolHash*, size_t);
SolHashRecord* solHash_find_record_by_key(SolHash*, void *);

#define solHash_size(h) h->size
#define solHash_count(h) h->count
#define solHash_update_mask(h) h->mask = h->size - 1

#define solHash_put(h, k, v) solHash_put_key_and_val(h, k, v)
#define solHash_get(h, k) solHash_find_value(h, k)

int solHash_put_key_and_val(SolHash*, void*, void*);
void solHash_remove_by_key(SolHash*, void*);
int solHash_has_key(SolHash*, void*);
void* solHash_find_value(SolHash*, void *);

SolHashIter* solHashIter_new(SolHash*);
void solHashIter_free(SolHashIter*);
void solHashIter_rewind(SolHashIter*);
SolHashRecord* solHashIter_current_record(SolHashIter *iter);
void solHashIter_next(SolHashIter*);

#define solHash_set_hash_func1(h, f) h->hash_func1 = f
#define solHash_set_hash_func2(h, f) h->hash_func2 = f
#define solHash_set_equal_func(h, f) h->equal_func = f
#define solHash_hash_func1(h) h->hash_func1
#define solHash_hash_func2(h) h->hash_func2
#define solHash_equal_func(h) h->equal_func

inline void solHash_records_free(SolHashRecord*);
inline SolHashRecord* solHash_record1_of_key(SolHash *hash, void *k);
inline SolHashRecord* solHash_record2_of_key(SolHash *hash, void *k);
inline void solHash_record_switch(SolHashRecord *r1, SolHashRecord *r2);
inline int solHash_add_records(SolHash *hash, SolHashRecord *records, size_t size);

#endif
