#include "sol_hash.h"

SolHash* solHash_new()
{
	SolHash *hash = sol_calloc(1, sizeof(SolHash));
	if (hash == NULL) {
		return NULL;
	}
	if (solHash_set_size(hash, SOL_HASH_INIT_SIZE)) {
		return NULL;
	}
	return hash;
}

void solHash_free(SolHash *hash)
{
	solHash_free_records(hash->records, hash->size, hash->f_free_k, hash->f_free_v);
	sol_free(hash);
}

inline void solHash_free_records(SolHashRecord *r, size_t s, void (*fk)(void *k), void (*fv)(void *v))
{
	SolHashRecord *cr;
	if (fk || fv) {
		size_t o = 0;
		while(o < s) {
			cr = solHash_record_at_offset(r, o);
			if (fk && cr->k) {
				(*fk)(cr->k);
			}
			if (fv && cr->v) {
				(*fv)(cr->v);
			}
			o++;
		}
	}
	sol_free(r);
}

int solHash_set_size(SolHash *hash, size_t size)
{
	hash->records = sol_calloc(size, sizeof(SolHashRecord));
	if (hash->records == NULL) {
		return 1;
	}
	hash->size = size;
	solHash_update_mask(hash);
	return 0;
}

SolHashRecord* solHash_find_record_by_key(SolHash *hash, void *k)
{
	SolHashRecord *r = solHash_record1_of_key(hash, k);
	if (r->k != NULL && solHash_match(hash, k, r->k) == 0) {
		return r;
	}
	r = solHash_record2_of_key(hash, k);
	if (r->k != NULL && solHash_match(hash, k, r->k) == 0) {
		return r;
	}
	return NULL;
}

void* solHash_find_value(SolHash *hash, void *k)
{
	SolHashRecord *r = solHash_find_record_by_key(hash, k);
	if (r == NULL) {
		return NULL;
	}
	return r->v;
}

int solHash_has_key(SolHash *hash, void *k)
{
	return solHash_find_value(hash, k) != NULL;
}

int solHash_put_key_and_val(SolHash *hash, void *k, void *v)
{
	SolHashRecord* r;
	r = solHash_record1_of_key(hash, k);
	if (r->k == NULL) {
		solHash_record_extend(r);
		r->k = k;
		r->v = v;
		hash->count++;
		return 0;
	}
	if (solHash_match(hash, k, r->k) == 0) {
		r->v = v;
		return 0;
	}
	r = solHash_record2_of_key(hash, k);
	if (r->k == NULL) {
		solHash_record_extend(r);
		r->k = k;
		r->v = v;
		hash->count++;
		return 0;
	}
	if (solHash_match(hash, k, r->k) == 0) {
		r->v = v;
		return 0;
	}
	// no place to put
	// adjust and resize
	return solHash_try_to_put(hash, k, v);
}

int solHash_try_to_put(SolHash *hash, void *k, void *v)
{
	SolHashRecord *r, rs;
	rs.k = k;
	rs.v = v;
	int i = 0;
	for (; i < hash->size * 2; i++) {
		// get conflict hash record
		r = solHash_record1_of_key(hash, k);
		// try to put record
		if (r->k == NULL) {
			solHash_record_extend(r);
			r->k = k;
			r->v = v;
			hash->count++;
			return 0;
		}
		// conflict exists
		// switch rs -> record, conflict record switched to rs
		solHash_record_switch(r, &rs);
		// second hash step
		r = solHash_record2_of_key(hash, rs.k);
		if (r->k == NULL) {
			solHash_record_extend(r);
			r->k = rs.k;
			r->v = rs.v;
			hash->count++;
			return 0;
		}
		solHash_record_switch(r, &rs);
	}
	if (hash->is_resizing == SOL_HASH_RESIZING_Y) {
		return 1;
	}
	if (solHash_grow(hash)) {
		return 1;
	}
	return solHash_put_key_and_val(hash, rs.k, rs.v);
}

void solHash_remove_by_key(SolHash *hash, void *k)
{
	SolHashRecord *r = solHash_find_record_by_key(hash, k);
	if (r && r->k != NULL) {
		r->k = NULL;
		r->v = NULL;
	}
}

int solHash_resize(SolHash *hash, size_t size)
{
	SolHashRecord *records = hash->records;
	size_t old_size = hash->size;
	int loop_limit = SOL_HASH_RESIZE_MAX_LOOP;
	hash->is_resizing = SOL_HASH_RESIZING_Y;
	do {
		if (solHash_set_size(hash, size) != 0) {
			hash->records = records;
			hash->size = old_size;
			hash->is_resizing = SOL_HASH_RESIZING_N;
			return 1;
		}
		hash->count = 0;
		if (solHash_add_records(hash, records, old_size) == 0) {
			hash->is_resizing = SOL_HASH_RESIZING_N;
		} else {
			size = size * 2;
			solHash_free_records(hash->records, hash->size, hash->f_free_k, hash->f_free_v);
		}
	} while (loop_limit-- && hash->is_resizing == SOL_HASH_RESIZING_Y);
	if (hash->is_resizing == SOL_HASH_RESIZING_Y) {
		hash->records = records;
		hash->size = old_size;
		hash->is_resizing = SOL_HASH_RESIZING_N;
		return 1;
	} else {
		solHash_free_records(records, old_size, hash->f_free_k, hash->f_free_v);
		return 0;
	}
}

inline int solHash_add_records(SolHash *hash, SolHashRecord *records, size_t size)
{
	SolHashRecord *r;
	size_t offset = 0;
	while(offset < size) {
		r = solHash_record_at_offset(records, offset);
		if (r->k) {
			if (solHash_put_key_and_val(hash, r->k, r->v)) {
				return 1;
			}
		}
		offset++;
	}
	return 0;
}

inline void solHash_record_switch(SolHashRecord *r1, SolHashRecord *r2)
{
	SolHashRecord tmp = *r1;
	*r1 = *r2;
	*r2 = tmp;
}

inline SolHashRecord* solHash_record1_of_key(SolHash *hash, void *k)
{
	size_t offset = (solHash_hash1(hash, k) & hash->mask);
	return solHash_record_at_offset(hash->records, offset);
}

inline SolHashRecord* solHash_record2_of_key(SolHash *hash, void *k)
{
	size_t offset = (solHash_hash2(hash, k) & hash->mask);
	SolHashRecord *record = solHash_record_at_offset(hash->records, offset);
	return record;
}

SolHashIter* solHashIter_new(SolHash *hash)
{
	SolHashIter *iter = sol_alloc(sizeof(SolHashIter));
	if (iter == NULL) {
		return NULL;
	}
	iter->hash = hash;
	solHashIter_rewind(iter);
	return iter;
}

void solHashIter_free(SolHashIter *iter)
{
	sol_free(iter);
}

SolHashRecord* solHashIter_current_record(SolHashIter *iter)
{
	if (iter->record->k) {
		return iter->record;
	}
	return NULL;
}

void solHashIter_rewind(SolHashIter *iter)
{
	iter->record = iter->hash->records;
	iter->c = 1;
}

void solHashIter_next(SolHashIter *iter)
{
	if (iter->c < iter->hash->size) {
		iter->record++;
		iter->c++;
	}
}
