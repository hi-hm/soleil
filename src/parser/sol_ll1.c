#include <assert.h>
#include "sol_ll1.h"
#include "sol_rbtree_iter.h"

SolLL1Parser* solLL1Parser_new()
{
    SolLL1Parser *p = sol_calloc(1, sizeof(SolLL1Parser));
    if (p == NULL) {
        return NULL;
    }
    solLL1Parser_set_stack(p, solStack_new());
    solLL1Parser_set_product_list(p, solList_new());
    solLL1Parser_set_symbol_list(p, solRBTree_new());
    if (solLL1Parser_stack(p) == NULL
        || solLL1Parser_product_list(p) == NULL
        ) {
        solLL1Parser_free(p);
        return NULL;
    }
    solList_set_val_free_func(solLL1Parser_product_list(p), &_solLL1ParserProduct_free);
    solRBTree_set_compare_func(solLL1Parser_symbol_list(p), &_solLL1Parser_symbol_compare);
    solRBTree_set_val_free_func(solLL1Parser_symbol_list(p), &_solLL1ParserSymbol_free);
    return p;
}

void solLL1Parser_free(SolLL1Parser *p)
{
    if (solLL1Parser_stack(p)) {
        solStack_free(solLL1Parser_stack(p));
    }
    if (solLL1Parser_product_list(p)) {
        solList_free(solLL1Parser_product_list(p));
    }
    if (solLL1Parser_symbol_list(p)) {
        solRBTree_free(solLL1Parser_symbol_list(p));
    }
    sol_free(p);
}

int solLL1Parser_reg_product(SolLL1Parser *p, SolLL1ParserProduct *f)
{
    if (p == NULL
        || solLL1Parser_product_list(p) == NULL
        || f == NULL
        ) {
        return -1;
    }
    if (solList_add(solLL1Parser_product_list(p), f)) {
        return 0;
    }
    return 1;
}

int solLL1Parser_reg_symbol(SolLL1Parser *p, SolLL1ParserSymbol *s)
{
    if (p == NULL
        || solLL1Parser_symbol_list(p) == NULL
        || s == NULL
        ) {
        return -1;
    }
    if (solRBTree_insert(solLL1Parser_symbol_list(p), s)) {
        return 0;
    }
    return 1;
}

SolLL1ParserSymbol* solLL1Parser_terminal(SolLL1Parser *p, void *s)
{
    SolLL1ParserSymbol *symbol = solLL1ParserSymbol_new(s, SolLL1ParserSymbolFlag_Terminal);
    if (symbol == NULL) return NULL;
    if (solLL1Parser_reg_symbol(p, symbol) == 0) {
        return symbol;
    }
    solLL1ParserSymbol_free(symbol);
    return NULL;
}

SolLL1ParserSymbol* solLL1Parser_nonterminal(SolLL1Parser *p, void *s)
{
    SolLL1ParserSymbol *symbol = solLL1ParserSymbol_new(s, SolLL1ParserSymbolFlag_Nonterminal);
    if (symbol == NULL) return NULL;
    if (solLL1Parser_reg_symbol(p, symbol) == 0) {
        return symbol;
    }
    solLL1ParserSymbol_free(symbol);
    return NULL;
}

SolLL1ParserSymbol* solLL1Parser_null(SolLL1Parser *p, void *s)
{
    SolLL1ParserSymbol *symbol = solLL1ParserSymbol_new(s, SolLL1ParserSymbolFlag_NULL);
    if (symbol == NULL) return NULL;
    if (solLL1Parser_reg_symbol(p, symbol) == 0) {
        return symbol;
    }
    solLL1ParserSymbol_free(symbol);
    return NULL;
}

SolLL1ParserSymbol* solLL1Parser_symbol_end(SolLL1Parser *p, void *s)
{
    SolLL1ParserSymbol *symbol = solLL1ParserSymbol_new(
        s,
        SolLL1ParserSymbolFlag_END | SolLL1ParserSymbolFlag_Terminal
        );
    if (symbol == NULL) return NULL;
    if (solLL1Parser_reg_symbol(p, symbol) == 0) {
        solLL1Parser_set_end_symbol(p, symbol);
        return symbol;
    }
    solLL1ParserSymbol_free(symbol);
    return NULL;
}
// return 0 when s is nullable, error < 0, finish but not nullable > 0
int solLL1Parser_symbol_compute_nullable(SolLL1Parser *p, SolLL1ParserSymbol *s)
{
    if (p == NULL) return -1;
    if (solLL1Parser_product_list(p) == NULL) return -2;
    if (solList_len(solLL1Parser_product_list(p)) == 0) return -3;
    if (solLL1ParserSymbol_is_nullable(s)) return 0;
    if (solLL1ParserSymbol_is_nullable_computed(s)) return 1;
    SolListNode *nf = solList_head(solLL1Parser_product_list(p));
    SolLL1ParserProductNode *ns;
    SolLL1ParserProduct *f;
    SolLL1ParserSymbol *s1;
    int status = 2;
    do {
        f = solListNode_val(nf);
        ns = solLL1ParserProduct_left(f);
        s1 = solLL1ParserProductNode_symbol(ns);
        if (s == s1) {
            status = 0;
            while ((ns = solLL1ParserProductNode_symbol_next(ns))) {
                s1 = solLL1ParserProductNode_symbol(ns);
                if (solLL1ParserSymbol_is_nullable(s1)) {
                    continue;
                } else if (solLL1ParserSymbol_is_nonterminal(s1)
                           && solLL1Parser_symbol_compute_nullable(p, s1) == 0) {
                    continue;
                }
                status = 2;
                break;
            }
            if (status == 0) {
                solLL1ParserSymbol_set_nullable(s);
                solLL1ParserSymbol_set_nullable_product(s, f);
                break;
            }
        }
    } while ((nf = solListNode_next(nf)));
    solLL1ParserSymbol_set_nullable_computed(s);
    return status;
}

int solLL1Parser_symbol_compute_first(SolLL1Parser *p, SolLL1ParserSymbol *s)
{
    if (p == NULL) return -1;
    if (solLL1Parser_product_list(p) == NULL) return -2;
    if (solList_len(solLL1Parser_product_list(p)) == 0) return -3;
    if (solLL1ParserSymbol_is_first_computed(s)) return 0;
    SolListNode *nf = solList_head(solLL1Parser_product_list(p));
    SolLL1ParserProductNode *ns;
    SolLL1ParserProduct *f;
    SolLL1ParserSymbol *s1;
    do {
        f = solListNode_val(nf);
        ns = solLL1ParserProduct_left(f);
        s1 = solLL1ParserProductNode_symbol(ns);
        if (s1 != s) {
            continue;
        }
        while ((ns = solLL1ParserProductNode_symbol_next(ns))) {
            s1 = solLL1ParserProductNode_symbol(ns);
            if (solLL1ParserSymbol_is_terminal(s1)) {
                solLL1ParserSymbol_add_first(s, s1, f);
                break;
            }
            assert((s != s1) && "has left recursion");
            if (solLL1Parser_symbol_compute_nullable(p, s1) == 0) {
                continue;
            }
            if (solLL1Parser_symbol_compute_first(p, s1) == 0) {
                if (solLL1ParserSymbol_dup_first(s, solLL1ParserSymbol_first(s1), f) != 0) {
                    return -4;
                }
            }
            break;
        }
    } while ((nf = solListNode_next(nf)));
    solLL1ParserSymbol_set_first_computed(s);
    return 0;
}

int solLL1Parser_symbol_compute_follow(SolLL1Parser *p, SolLL1ParserSymbol *s)
{
    if (p == NULL) return -1;
    if (solLL1Parser_product_list(p) == NULL) return -2;
    if (solList_len(solLL1Parser_product_list(p)) == 0) return -3;
    if (solLL1ParserSymbol_is_terminal(s)) return -4;
    if (solLL1ParserSymbol_is_follow_computed(s)) return 0;
    SolListNode *nf = solList_head(solLL1Parser_product_list(p));
    SolLL1ParserSymbol *s1;
    SolLL1ParserSymbol *s2;
    SolLL1ParserProduct *f;
    SolLL1ParserProductNode *ns;
    int status = 1;
    do {
        status = 2;
        f = solListNode_val(nf);
        ns = solLL1ParserProduct_left(f);
        s1 = solLL1ParserProductNode_symbol(ns);
        while ((ns = solLL1ParserProductNode_symbol_next(ns))) {
            s2 = solLL1ParserProductNode_symbol(ns);
            if (s == s2) {
                status = 3;
            } else if (status == 3) {
                if (solLL1ParserSymbol_is_terminal(s2)) {
                    if (solLL1ParserSymbol_add_follow(s, s2, f) != 0) {
                        return -7;
                    }
                } else { // no terminal
                    if (solLL1Parser_symbol_compute_first(p, s2) != 0) {
                        return -5;
                    }
                    if (solLL1ParserSymbol_dup_follow(s, solLL1ParserSymbol_first(s2), f) != 0) {
                        return -6;
                    }
                }
                if (solLL1Parser_symbol_compute_nullable(p, s2) == 0) {
                    continue;
                }
                status = 4;
                break;
            }
        }
        if (status == 3) { // after s are all nullable
            if (s == s1) {
                continue;
            }
            if (solLL1Parser_symbol_compute_follow(p, s1) != 0) {
                return -8;
            }
            if (solLL1ParserSymbol_follow(s1)) {
                if (solLL1ParserSymbol_dup_follow(s, solLL1ParserSymbol_follow(s1), f) != 0) {
                    return -6;
                }
            }
        }
    } while ((nf = solListNode_next(nf)));
    solLL1ParserSymbol_set_follow_computed(s);
    return 0;
}

int solLL1Parser_generate_table(SolLL1Parser *p)
{
    if (p == NULL) return -1;
    if (solLL1Parser_symbol_list(p) == NULL
        || solRBTree_count(solLL1Parser_symbol_list(p)) == 0
        ) return -2;
    if (solLL1Parser_product_list(p) == NULL
        || solList_len(solLL1Parser_product_list(p)) == 0
        ) return -3;
    SolRBTree *t = solLL1Parser_symbol_list(p);
    if (solRBTree_travelsal_preorder(t, solRBTree_root(t), &_solLL1Parser_rbnode_compute_nullable, p)) {
        return 1;
    }
    if (solRBTree_travelsal_preorder(t, solRBTree_root(t), &_solLL1Parser_rbnode_compute_first, p)) {
        return 2;
    }
    if (solRBTree_travelsal_preorder(t, solRBTree_root(t), &_solLL1Parser_rbnode_compute_follow, p)) {
        return 3;
    }
    return 0;
}

void _solLL1ParserProduct_free(void *f)
{
    solLL1ParserProduct_free((SolLL1ParserProduct*)(f));
}

int solLL1ParserProduct_add_symbol(SolLL1ParserProduct *f, SolLL1ParserSymbol *s)
{
    if (f == NULL || s == NULL) {
        return -1;
    }
    if (solLL1ParserProduct_add(f, s)) {
        return 0;
    }
    return 1;
}

SolLL1ParserSymbol* solLL1ParserSymbol_new(void *s, int t)
{
    SolLL1ParserSymbol *symbol = sol_calloc(1, sizeof(SolLL1ParserSymbol));
    if (symbol == NULL) {
        return NULL;
    }
    solLL1ParserSymbol_set_symbol(symbol, s);
    solLL1ParserSymbol_set_type(symbol, t);
    if (solLL1ParserSymbol_is_nonterminal(symbol)) {
        solLL1ParserSymbol_set_first(symbol, solRBTree_new());
        if (solLL1ParserSymbol_first(symbol) == NULL) {
            solLL1ParserSymbol_free(symbol);
            return NULL;
        }
        solRBTree_set_compare_func(solLL1ParserSymbol_first(symbol), &_solLL1Parser_entry_compare);
        solRBTree_set_val_free_func(solLL1ParserSymbol_first(symbol), &_solLL1ParserEntry_free);
    }
    return symbol;
}

void solLL1ParserSymbol_free(SolLL1ParserSymbol *s)
{
    if (solLL1ParserSymbol_first(s)) {
        solRBTree_free(solLL1ParserSymbol_first(s));
    }
    if (solLL1ParserSymbol_follow(s)) {
        solRBTree_free(solLL1ParserSymbol_follow(s));
    }
    sol_free(s);
}

void _solLL1ParserSymbol_free(void *s)
{
    solLL1ParserSymbol_free((SolLL1ParserSymbol*)s);
}

int solLL1ParserSymbol_add_first(SolLL1ParserSymbol *s1, SolLL1ParserSymbol *s2, SolLL1ParserProduct *p)
{
    if (s1 == NULL || s2 == NULL) return -1;
    if (s1 == s2) return -2;
    SolLL1ParserEntry *e = solLL1ParserEntry_new(s2, p);
    return solLL1ParserSymbol_add_first_entry(s1, e);
}

int solLL1ParserSymbol_add_first_entry(SolLL1ParserSymbol *s, SolLL1ParserEntry *e)
{
    if (s == NULL || e == NULL) return -1;
    if (s == solLL1ParserEntry_symbol(e)) return -2;
    if (solLL1ParserSymbol_is_NOT_nonterminal(s)) return -3;
    if (solLL1ParserSymbol_first(s) == NULL) return -4;
    if (solRBTree_insert(solLL1ParserSymbol_first(s), e)) {
        return 0;
    }
    return 1;
}

int solLL1ParserSymbol_dup_first(SolLL1ParserSymbol *s, SolRBTree *t, SolLL1ParserProduct *p)
{
    SolRBTreeIter *i = solRBTreeIter_new(t, solRBTree_root(t), SolRBTreeIterTT_preorder);
    SolLL1ParserEntry *e;
    SolRBTreeNode *n;
    SolLL1ParserSymbol *s1;
    while ((n = solRBTreeIter_next(i))) {
        e = solRBTreeNode_val(n);
        s1 = solLL1ParserEntry_symbol(e);
        if (p == NULL) {
            p = solLL1ParserEntry_product(e);
        }
        solLL1ParserSymbol_add_first(s, s1, p);
    }
    solRBTreeIter_free(i);
    return 0;
}

int solLL1ParserSymbol_add_follow(SolLL1ParserSymbol *s1, SolLL1ParserSymbol *s2, SolLL1ParserProduct *p)
{
    if (s1 == NULL || s2 == NULL) return -1;
    if (s1 == s2) return -2;
    if (solLL1ParserSymbol_is_NOT_nonterminal(s1)) return -3;
    if (solLL1ParserSymbol_follow(s1) == NULL) {
        solLL1ParserSymbol_set_follow(s1, solRBTree_new());
        if (solLL1ParserSymbol_follow(s1) == NULL) {
            return -4;
        }
        solRBTree_set_compare_func(solLL1ParserSymbol_follow(s1), &_solLL1Parser_entry_compare);
        solRBTree_set_val_free_func(solLL1ParserSymbol_follow(s1), &_solLL1ParserEntry_free);
    }
    SolLL1ParserEntry *e = solLL1ParserEntry_new(s2, p);
    if (e == NULL) return -5;
    if (solRBTree_insert(solLL1ParserSymbol_follow(s1), e)) {
        return 0;
    }
    return 1;
}

int solLL1ParserSymbol_dup_follow(SolLL1ParserSymbol *s, SolRBTree *t, SolLL1ParserProduct *p)
{
    if (s == NULL) return -1;
    if (t == NULL) return 0;
    if (solLL1ParserSymbol_is_NOT_nonterminal(s)) return -3;
    if (solLL1ParserSymbol_follow(s) == NULL) {
        solLL1ParserSymbol_set_follow(s, solRBTree_new());
        if (solLL1ParserSymbol_follow(s) == NULL) {
            return -4;
        }
        solRBTree_set_compare_func(solLL1ParserSymbol_follow(s), &_solLL1Parser_entry_compare);
        solRBTree_set_val_free_func(solLL1ParserSymbol_follow(s), &_solLL1ParserEntry_free);
    }
    if (solLL1ParserSymbol_follow(s) == t) return -5;
    SolRBTreeIter *i = solRBTreeIter_new(t, solRBTree_root(t), SolRBTreeIterTT_preorder);
    SolLL1ParserEntry *e;
    SolRBTreeNode *n;
    SolLL1ParserSymbol *s1;
    while ((n = solRBTreeIter_next(i))) {
        e = solRBTreeNode_val(n);
        s1 = solLL1ParserEntry_symbol(e);
        if (p == NULL) {
            p = solLL1ParserEntry_product(e);
        }
        solLL1ParserSymbol_add_follow(s, s1, p);
    }
    solRBTreeIter_free(i);
    return 0;
}

SolLL1ParserEntry* solLL1ParserEntry_new(SolLL1ParserSymbol *s, SolLL1ParserProduct *p)
{
    SolLL1ParserEntry *e = sol_alloc(sizeof(SolLL1ParserEntry));
    if (e == NULL) {
        return NULL;
    }
    solLL1ParserEntry_set_symbol(e, s);
    solLL1ParserEntry_set_product(e, p);
    return e;
}

void solLL1ParserEntry_free(SolLL1ParserEntry *e)
{
    if (e) sol_free(e);
}

int solLL1Parser_parse(SolLL1Parser *p, void *g, void *x)
{
    assert((solLL1Parser_read_symbol_func(p)) && "Parser: no read symbol func");
    assert((solLL1Parser_output_func(p)) && "Parser: no output func");
    assert((solLL1Parser_start_symbol(p)) && "Parser: no start symbol");
    assert((solLL1Parser_end_symbol(p)) && "Parser: no end symbol");
    solStack_push(solLL1Parser_stack(p), solLL1Parser_start_symbol(p));
    int status = 0;
    SolLL1ParserSymbol *sbl1;
    SolLL1ParserSymbol *sbl2;
    SolLL1ParserEntry *e1 = solLL1ParserEntry_new(NULL, NULL);
    SolLL1ParserEntry *e2;
    SolLL1ParserProductNode *n;
    SolRBTreeNode *rbn;
    while ((sbl1 = solLL1Parser_read_symbol(p, g))) {
    parse:
        sbl2 = solStack_pop(solLL1Parser_stack(p));
        if (sbl2 == NULL) {
            status = -5;
            break;
        }
        if (solLL1ParserSymbol_is_end(sbl2)) {
            if (solStack_pop(solLL1Parser_stack(p))) {
                status = -1;
            } else if (solLL1Parser_read_symbol(p, g)) {
                status = -2;
            } else {
                solLL1Parser_output(p, x, NULL, sbl2, NULL);
            }
            break;
        }
        if (solLL1ParserSymbol_is_terminal(sbl2)) {
            solLL1Parser_output(p, x, NULL, sbl2, NULL);
            continue;
        }
        if (solLL1ParserSymbol_first(sbl2) == NULL) break;
        solLL1ParserEntry_set_symbol(e1, sbl1);
        if (solRBTree_count(solLL1ParserSymbol_first(sbl2))) {
            rbn = solRBTree_search_node(solLL1ParserSymbol_first(sbl2), e1);
            e2 = (SolLL1ParserEntry*)solRBTreeNode_val(rbn);
            if (e2 == NULL || solLL1ParserEntry_product(e2) == NULL)
                goto check_nullable;
            if (solList_head(solLL1ParserEntry_product(e2)) == NULL)
                goto check_nullable;
            sbl2 = solLL1ParserProductNode_symbol(solLL1ParserProduct_left(solLL1ParserEntry_product(e2)));
            if (sbl2 == NULL) {
                status = -3;
                break;
            }
            solLL1Parser_output(p, x, solLL1ParserEntry_product(e2), NULL, NULL);
            n = solDlList_tail(solLL1ParserEntry_product(e2));
            do {
                solStack_push(solLL1Parser_stack(p), solDlListNode_val(n));
                n = solDlListNode_pre(n);
            } while ((n != solDlList_head(solLL1ParserEntry_product(e2))) && n);
            goto parse;
        }
    check_nullable:
        if (solLL1ParserSymbol_is_nullable(sbl2)
            && solLL1ParserSymbol_follow(sbl2)) {
            if (solRBTree_count(solLL1ParserSymbol_follow(sbl2))) {
                solLL1Parser_output(p, x, NULL, NULL, sbl2);
                goto parse;
            }
        }
        status = -4;
        break; // error
    }
    solLL1ParserEntry_free(e1);
    return status;
}

void _solLL1ParserEntry_free(void *e)
{
    solLL1ParserEntry_free((SolLL1ParserEntry*)e);
}

int _solLL1Parser_entry_compare(void *s1, void *s2)
{
    if (solLL1ParserEntry_symbol((SolLL1ParserEntry*)s1)
        > solLL1ParserEntry_symbol((SolLL1ParserEntry*)s2)
        ) return 1;
    if (solLL1ParserEntry_symbol((SolLL1ParserEntry*)s1)
        < solLL1ParserEntry_symbol((SolLL1ParserEntry*)s2)
        ) return -1;
    else return 0;
}

int _solLL1Parser_symbol_compare(void *s1, void *s2)
{
    if (s1 > s2) return 1;
    else if (s1 < s2) return -1;
    else return 0;
}

int _solLL1Parser_rbnode_compute_nullable(SolRBTree *t, SolRBTreeNode *n, void *p)
{
    SolLL1ParserSymbol* s = (solRBTreeNode_val(n));
    if (solLL1ParserSymbol_is_nonterminal(s)) {
        if (solLL1Parser_symbol_compute_nullable((SolLL1Parser*)p, s) < 0) {
            return 1;
        }
    }
    return 0;
}

int _solLL1Parser_rbnode_compute_first(SolRBTree *t, SolRBTreeNode *n, void *p)
{
    SolLL1ParserSymbol* s = (solRBTreeNode_val(n));
    if (solLL1ParserSymbol_is_nonterminal(s)) {
        return solLL1Parser_symbol_compute_first((SolLL1Parser*)p, s);
    }
    return 0;
}

int _solLL1Parser_rbnode_compute_follow(SolRBTree *t, SolRBTreeNode *n, void *p)
{
    SolLL1ParserSymbol* s = (solRBTreeNode_val(n));
    if (solLL1ParserSymbol_is_nullable(s)
        && solLL1ParserSymbol_is_nonterminal(s)
        ) {
        return solLL1Parser_symbol_compute_follow((SolLL1Parser*)p, s);
    }
    return 0;
}
