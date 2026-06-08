#include "gawk_ext.h"
#include "htrees.h"

static awk_bool_t do_at_init()
{
	init_trees();
	return true;
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

	if (get_argument(0, AWK_STRING, &awk_name))
	{
		char* name = awk_name.str_value.str;
		delete_tree(name);
	}
	else
		fatal(ext_id, "delete_tree: Invalid arguments");

	// `delete` can't be used in expressions, but we need to fill "result" with something to avoid memory errors
	return make_null_string(result);
}

static awk_value_t* do_tree_length(const int nargs, awk_value_t* result, struct awk_ext_func* _)
{
	assert(result != NULL);

	awk_value_t awk_name;
	double ret;

	if (get_argument(0, AWK_STRING, &awk_name))
	{
		ret = tree_length(awk_name.str_value.str);
	}
	else
		ret = 0;

	return make_number(ret, result);
}

static query_t _get_query(const int nargs)
{
	awk_value_t arg;
	char* name;
	const unsigned char num_subs = nargs - 1;
	char** subscripts = malloc(num_subs * sizeof(char*));

	if (get_argument(0, AWK_STRING, &arg))
		name = strdup(arg.str_value.str);
	else
		fatal(ext_id, "htrees: No name given");

	for (int i = 0; i < num_subs; ++i)
	{
		if (get_argument(i+1, AWK_STRING, &arg))
			subscripts[i] = strdup(arg.str_value.str);
		else
			fatal(ext_id, "htrees: Bad argument");
	}

	return (query_t){name, subscripts, num_subs};
}
#define get_query() _get_query(nargs)
#define full_query query.name, query.subscripts, query.num_subs

// static const unsigned int get_query_with_size(const int nargs)

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
	const unsigned char num_subs = query.num_subs - 1;
	
	const foint value = {.s=subscripts[num_subs]};
	tree_insert(query.name, subscripts, value, num_subs);
	free_query(query);
	return make_number(atof(value.s), result);
}

static awk_value_t* do_query_tree(const int nargs, awk_value_t* result, struct awk_ext_func* _)
{
	assert(result != NULL);

	query_t query = get_query();
	const char* data = query_tree(full_query);

	free_query(query);
	return make_const_string(data, strlen(data), result);
}

static awk_value_t* do_tree_modify(const int nargs, awk_value_t* result, struct awk_ext_func* _)
{
	assert(result != NULL);

	query_t query = get_query();
	const unsigned char num_subs = query.num_subs - 1;
	double ret = tree_modify(query.name, query.subscripts, num_subs, query.subscripts[num_subs]);

	free_query(query);
	return make_number(ret, result);
}

static awk_value_t* do_tree_increment(const int nargs, awk_value_t* result, struct awk_ext_func* _)
{
	assert(result != NULL);

	query_t query = get_query();
	double ret = tree_increment(full_query);

	free_query(query);
	return make_number(ret, result);
}

static awk_value_t* do_tree_decrement(const int nargs, awk_value_t* result, struct awk_ext_func* _)
{
	assert(result != NULL);

	query_t query = get_query();
	double ret = tree_decrement(full_query);

	free_query(query);
	return make_number(ret, result);
}

static awk_value_t* do_tree_remove(const int nargs, awk_value_t* result, struct awk_ext_func* _)
{
	assert(result != NULL);

	query_t query = get_query();
	tree_remove(full_query);

	free_query(query);
	return make_null_string(result);
	// `delete` can't be used in expressions, but we need to fill "result" with something to avoid memory errors
}

static awk_value_t* do_tree_elem_exists(const int nargs, awk_value_t* result, struct awk_ext_func* _)
{
	assert(result != NULL);

	query_t query = get_query();
	double ret = tree_elem_exists(full_query);

	free_query(query);
	return make_number(ret, result);
}

static awk_value_t* do_is_tree(const int nargs, awk_value_t* result, struct awk_ext_func* _)
{
	assert(result != NULL);

	query_t query = get_query();
	double ret = is_tree(full_query);

	free_query(query);
	return make_number(ret, result);
}

static awk_value_t* do_tree_next(const int nargs, awk_value_t* result, struct awk_ext_func* _)
{
	assert(result != NULL);

	query_t query = get_query();
	const char* ret = tree_next(full_query);

	free_query(query);
	return make_const_string(ret, strlen(ret), result);
}

static awk_value_t* do_tree_iters_remaining(const int nargs, awk_value_t* result, struct awk_ext_func* _)
{
	assert(result != NULL);

	query_t query = get_query();
	bool ret = tree_iters_remaining(full_query);
	
	free_query(query);
	return make_number((double)ret, result);
}

static awk_value_t* do_tree_iter_break(const int nargs, awk_value_t* result, struct awk_ext_func* _)
{
	assert(result != NULL);
	tree_iter_break();
	return make_null_string(result);
	// `break` can't be used in expressions, but we need to fill "result" with something to avoid memory errors
}

static awk_ext_func_t func_table[] =
{
	{ "create_tree", do_create_tree, 2, 2, awk_false, NULL },
	{ "delete_tree", do_delete_tree, 1, 1, awk_false, NULL },
	{ "tree_length", do_tree_length, 1, 1, awk_false, NULL },
	{ "tree_insert", do_tree_insert, 0, 2, awk_true, NULL },
	{ "query_tree", do_query_tree, 0, 2, awk_true, NULL },
	{ "tree_modify", do_tree_modify, 0, 3, awk_true, NULL },
	{ "tree_increment", do_tree_increment, 0, 2, awk_true, NULL },
	{ "tree_decrement", do_tree_decrement, 0, 2, awk_true, NULL },
	{ "tree_remove", do_tree_remove, 0, 2, awk_true, NULL },
	{ "tree_elem_exists", do_tree_elem_exists, 0, 2, awk_true, NULL },
	{ "is_tree", do_is_tree, 0, 2, awk_true, NULL },
	{ "tree_next", do_tree_next, 0, 1, awk_true, NULL},
	{ "tree_iters_remaining", do_tree_iters_remaining, 0, 1, awk_true, NULL },
	{ "tree_iter_break", do_tree_iter_break, 0, 0, awk_false, NULL }
};
#if HTREE_USES_AVL
dl_load_func(func_table, htrees, "");
#else
dl_load_func(func_table, binhtrees, "");
#endif
