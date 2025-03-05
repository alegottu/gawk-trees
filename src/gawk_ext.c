#include "htrees.h"
#include "gawkapi.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "gawk_ext.h"

extern TREETYPE* trees;
extern TREETYPE* current_iterators;

static foint copy_str(foint info)
{
	foint ret;
	ret.s = gawk_malloc((strlen(info.s) + 1) * sizeof(char));
	strcpy(ret.s, info.s);
	return ret;
}

static awk_bool_t do_at_init()
{
	trees = TreeAlloc((pCmpFcn)strcmp, (pFointCopyFcn)strdup, (pFointFreeFcn)free, NULL, (pFointFreeFcn)free_htree); 
	current_iterators = TreeAlloc((pCmpFcn)strcmp, (pFointCopyFcn)strdup, (pFointFreeFcn)free, NULL, (pFointFreeFcn)LinkedListFree); 

	awk_atexit((void*)do_at_exit, NULL); // possible to use 2nd arg instead of global trees

	return trees != NULL;
}

static awk_value_t* do_create_tree(const int nargs, awk_value_t* result, struct awk_ext_func* _)
{
	assert(result != NULL);

	awk_value_t awk_name, awk_depth;
	bool ret;

	if (get_argument(0, AWK_STRING, &awk_name) && get_argument(1, AWK_NUMBER, &awk_depth))
	{
		char* name = awk_name.str_value.str;
		int depth = (int)awk_depth.num_value;
		ret = create_tree(name, depth) != NULL;
	}
	else
		fatal(ext_id, "create_tree: Invalid arguments");

	return make_number((double)ret, result);
}

static awk_value_t* do_delete_tree(const int nargs, awk_value_t* result, struct awk_ext_func* _)
{
	assert(result != NULL);

	awk_value_t awk_name;
	bool ret;

	if (get_argument(0, AWK_STRING, &awk_name))
	{
		char* name = awk_name.str_value.str;
		ret = delete_tree(name);
	}
	else
		fatal(ext_id, "delete_tree: Invalid arguments");

	return make_number((double)ret, result);
}

static query_t get_query()
{
	awk_value_t arg;
	char* name;
	char** subscripts;
	subscripts = malloc(1 * sizeof(char*));
	unsigned char i = 0;

	if (get_argument(0, AWK_STRING, &arg))
	{
		name = strdup(arg.str_value.str);

		while (get_argument(i+1, AWK_STRING, &arg))
		{
			i++;
			subscripts = realloc(subscripts, i * sizeof(char*));
			subscripts[i-1] = strdup(arg.str_value.str);
		}
	}
	else
		fatal(ext_id, "htrees: No name given");

	return (query_t){name, subscripts, i};
}

static void free_query(query_t query)
{
	for (unsigned char i = 0; i < query.num_subs; ++i)
	{
		free(query.subscripts[i]);
	}

	free(query.subscripts);
	free(query.name);
}

static awk_value_t* do_tree_insert(const int nargs, awk_value_t* result, struct awk_ext_func* _)
{
	assert(result != NULL);

	query_t query = get_query();
	const char** subscripts = query.subscripts;
	const unsigned char num_subs = query.num_subs;
	
	tree_insert(query.name, subscripts, (foint){.s=subscripts[num_subs - 1]}, num_subs - 1);
	free_query(query);
	return make_number(1, result); // assume success if we get to this point
}

static awk_value_t* do_query_tree(const int nargs, awk_value_t* result, struct awk_ext_func* _)
{
	assert(result != NULL);

	query_t query = get_query();
	foint data;
	query_tree(query.name, query.subscripts, &data, query.num_subs);

	free_query(query);
	return make_const_string(data.s, strlen(data.s), result);
}

static awk_value_t* do_tree_remove(const int nargs, awk_value_t* result, struct awk_ext_func* _)
{
	assert(result != NULL);

	query_t query = get_query();
	double ret = tree_remove(query.name, query.subscripts, query.num_subs);

	free_query(query);
	return make_number(ret, result);
}

static awk_value_t* do_tree_elem_exists(const int nargs, awk_value_t* result, struct awk_ext_func* _)
{
	assert(result != NULL);

	query_t query = get_query();
	double ret = tree_elem_exists(query.name, query.subscripts, query.num_subs);

	free_query(query);
	return make_number(ret, result);
}

static awk_value_t* do_is_tree(const int nargs, awk_value_t* result, struct awk_ext_func* _)
{
	assert(result != NULL);

	query_t query = get_query();
	double ret = is_tree(query.name, query.subscripts, query.num_subs);

	free_query(query);
	return make_number(ret, result);
}

static awk_value_t* do_tree_next(const int nargs, awk_value_t* result, struct awk_ext_func* _)
{
	assert(result != NULL);

	query_t query = get_query();
	const char* ret = tree_next(query.name, query.subscripts, query.num_subs);

	free_query(query);
	return make_const_string(ret, strlen(ret), result);
}

static awk_value_t* do_tree_iters_remaining(const int nargs, awk_value_t* result, struct awk_ext_func* _)
{
	assert(result != NULL);

	query_t query = get_query();
	bool ret = tree_iters_remaining(query.name, query.subscripts, query.num_subs);
	
	free_query(query);
	return make_number((double)ret, result);
}

static awk_value_t* do_tree_iter_break(const int nargs, awk_value_t* result, struct awk_ext_func* _)
{
	assert(result != NULL);

	query_t query = get_query();
	bool ret = tree_iter_break(query.name, query.subscripts, query.num_subs);
	
	free_query(query);
	return make_number((double)ret, result);
}

static awk_ext_func_t func_table[] = 
{
	{ "create_tree", do_create_tree, 2, 2, awk_false, NULL },
	{ "delete_tree", do_delete_tree, 1, 1, awk_false, NULL },
	{ "tree_insert", do_tree_insert, 0, 2, awk_true, NULL },
	{ "query_tree", do_query_tree, 0, 2, awk_true, NULL },
	{ "tree_remove", do_tree_remove, 0, 2, awk_true, NULL },
	{ "tree_elem_exists", do_tree_elem_exists, 0, 2, awk_true, NULL },
	{ "is_tree", do_is_tree, 0, 2, awk_true, NULL },
	{ "tree_next", do_tree_next, 0, 1, awk_true, NULL},
	{ "tree_iters_remaining", do_tree_iters_remaining, 0, 1, awk_true, NULL },
	{ "tree_iter_break", do_tree_iter_break, 0, 1, awk_true, NULL }
};
dl_load_func(func_table, htrees, "");
