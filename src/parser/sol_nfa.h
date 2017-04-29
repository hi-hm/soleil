#ifndef _SOL_NFA_H_
#define _SOL_NFA_H_ 1

#include "sol_common.h"
#include "sol_utils.h"
#include "sol_hash.h"
#include "sol_set.h"

typedef struct _SolNfaState {
	void *s;  // state
	SolHash *n; // next states hash {C1: S1, C2: S2, ...}
	SolSet *f; // free moves (S1, S2, S3)
} SolNfaState;

typedef struct _SolNfa {
	SolSet *ss; // starting states (S1, S2, S3, ...)
	SolSet *cs; // current states (S1, S2, S3, ...)
	SolSet *as; // accepting states (Sa, Sb, Sc, ...)
	SolHash *als; // all states {S1: NfaS1, S2: NfaS2, ...}
	sol_f_hash_ptr f_s_hash1; // state hash func1
	sol_f_hash_ptr f_s_hash2; // state hash func2
	sol_f_hash_ptr f_c_hash1; // character hash func1
	sol_f_hash_ptr f_c_hash2; // character hash func2
	sol_f_match_ptr f_sm; // func state match
	sol_f_match_ptr f_cm; // func character match
} SolNfa;

SolNfa* solNfa_new();
void solNfa_init(SolNfa*);
void solNfa_free();
int solNfa_reset(SolNfa*);
int solNfa_add_rule(SolNfa*, void*, void*, void*);
int solNfa_read_character(SolNfa *p, void* c);

int solNfa_add_starting_state(SolNfa*, void*);
int solNfa_add_current_state(SolNfa*, void*);
int solNfa_add_accepting_state(SolNfa*, void*);
int solNfa_add_accepting_states(SolNfa*, SolSet*);
int solNfa_is_accepted(SolNfa*);
int _solNfa_states_free_moves(SolNfa*, SolSet*, SolSet*);
int solNfa_free_moves(SolNfa*);
SolNfaState* solNfa_register_state(SolNfa*, void*);

#define solSet_set_starting_states(p, s) p->ss = s
#define solNfa_set_current_states(p, s) p->cs = s
#define solNfa_set_accepting_states(p, s) p->as = s
#define solNfa_set_all_states(p, ss) p->als = ss

#define solNfa_starting_states(p) p->ss
#define solNfa_current_states(p) p->cs
#define solNfa_accepting_states(p) p->as
#define solNfa_all_states(p) p->als
// wipe, not free
#define solNfa_dup_accepting_states(nfa1, nfa2)							\
	solSet_dup(solNfa_accepting_states(nfa1), solNfa_accepting_states(nfa2))
#define solNfa_wipe_all_states(nfa) solHash_wipe(solNfa_all_states(nfa))

#define solNfa_set_state_hash_func1(p, f) p->f_s_hash1 = f
#define solNfa_set_state_hash_func2(p, f) p->f_s_hash2 = f
#define solNfa_set_character_hash_func1(p, f) p->f_c_hash1 = f
#define solNfa_set_character_hash_func2(p, f) p->f_c_hash2 = f
#define solNfa_set_state_match_func(p, f) p->f_sm = f
#define solNfa_set_character_match_func(p, f) p->f_cm = f

#define solNfa_state_hash_func1(p) p->f_s_hash1
#define solNfa_state_hash_func2(p) p->f_s_hash2
#define solNfa_character_hash_func1(p) p->f_c_hash1
#define solNfa_character_hash_func2(p) p->f_c_hash2
#define solNfa_state_match_func(p) p->f_sm
#define solNfa_character_match_func(p) p->f_cm

SolNfaState* solNfaState_new(void*);
void solNfaState_free(SolNfaState*);
inline int solNfaState_add_rule(SolNfaState*, void*, void*);
void* solNfaState_next_state(SolNfaState*, void*);

#define solNfaState_set_next_state_table(ns, t) ns->n = t
#define solNfaState_state(ns) ns->s
#define solNfaState_next_state_table(ns) ns->n
#define solNfaState_free_moves_set(ns) ns->f
#define solNfaState_has_next_state(ns) solHash_is_not_empty(solNfaState_next_state_table(ns))
#define solNfaState_has_free_moves(ns) solSet_is_not_empty(solNfaState_free_moves_set(ns))
#define solNfaState_next_state_count(ns) solHash_count(solNfaState_next_state_table(ns))
#define solNfaState_free_moves_count(ns) solSet_count(solNfaState_free_moves_set(ns))

#define solNfaState_set_state(ps, s) ps->cs = s
#define solNfa_conv_nfa_state(nfa, s) solHash_get(nfa->als, s)

void _solNfaState_free(void *ps);

size_t solNfaState_hash_func1(void*);
size_t solNfaState_hash_func2(void*);
inline SolSet* _solNfa_nfa_state_set_new(SolNfa*);

#endif