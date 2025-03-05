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

void free_htree(const foint tree);

bool init_trees();

HTREE* create_tree(const char* name, const int depth);

const bool delete_tree(const char* name);

void tree_insert(const char* tree, const char** subscripts, const foint value, const unsigned char depth);

const bool query_tree(const char* tree, const char** subscripts, foint* result, const unsigned char depth);

const bool tree_elem_exists(const char* tree, const char** subscripts, const unsigned char depth);

const bool tree_remove(const char* tree, const char** subscripts, const unsigned char depth);

const unsigned short is_tree(const char* tree, const char** subscripts, const unsigned char depth);

const char* tree_next(const char* tree, const char** subscripts, const unsigned char depth);

const unsigned int tree_iters_remaining(const char* tree, const char** subscripts, const unsigned char depth);

const bool tree_iter_break(const char* tree, const char** subscripts, const unsigned char depth);

void do_at_exit(void* data, int exit_status);

#endif // !HTREES_H
