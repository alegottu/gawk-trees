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
#else
#include "bintree.h"
#define NODETYPE BINTREENODE
#endif

void free_htree(const foint tree);

bool init_trees();

HTREE* create_tree(const char* name, const int depth);

void delete_tree(const char* name);

const double tree_length(const char* name);

void tree_insert(const char* tree, const char** subscripts, const foint value, const unsigned char depth);

const foint query_tree(const char* tree, const char** subscripts, const unsigned char depth);

const double tree_modify(const char* tree, const char** subscripts, const unsigned char depth, const char* expr);

const double tree_increment(const char* tree, const char** args, const unsigned char argc);

const double tree_decrement(const char* tree, const char** args, const unsigned char argc);

const bool tree_elem_exists(const char* tree, const char** subscripts, const unsigned char depth);

void tree_remove(const char* tree, const char** subscripts, const unsigned char depth);

const unsigned short is_tree(const char* tree, const char** subscripts, const unsigned char depth);

const char* tree_next(const char* tree, const char** subscripts, const unsigned char depth);

const unsigned int tree_iters_remaining(const char* tree, const char** subscripts, const unsigned char depth);

void tree_iter_break();

void do_at_exit(void* data, int exit_status);

#endif // !HTREES_H
