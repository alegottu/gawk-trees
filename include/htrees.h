#ifndef HTREES_H
#define HTREES_H

#include <string.h>
#include <stdio.h>

#include <stdbool.h>
#include <sys/stat.h>

#include "htree.h"
#include "linked-list.h"

#if HTREE_USES_AVL
#include "avltree.h"
#define NODETYPE AVLTREENODE
#define TreeDelete(T,k) AvlTreeLookDel((T),(k),(foint*)1)
#define AvlTreeDelete(T,k) AvlTreeLookDel((T),(k),(foint*)1)
#else
#include "bintree.h"
#define NODETYPE BINTREENODE
#define TreeDelete(t,k) BinTreeLookDel((t), (k), (foint*)1)
#endif

#define MAX_SUBSCRIPTS 8 

void free_htree(foint tree);

bool init_trees();

HTREE* create_tree(char* name, const int depth);

void tree_insert(const char* query, const foint value);

const bool query_tree(const char* query, foint* result);

// const bool delete_tree(const char* name);

const unsigned short is_tree(const char* query);

const char* tree_next(char* query);

const bool tree_iter_done(char* query, const bool force);

void do_at_exit(void* data, int exit_status);

#endif // !HTREES_H
