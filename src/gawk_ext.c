#include "htrees.h"
#include "gawk_ext.h"

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

	// no make_bool function in gawk api 3.0
	return make_number((double)ret, result);
}

static awk_value_t* do_tree_insert(const int nargs, awk_value_t* result, struct awk_ext_func* _)
{
	assert(result != NULL);

	awk_value_t awk_query, awk_value;
	foint value;

	if (get_argument(0, AWK_STRING, &awk_query) && get_argument(1, AWK_STRING, &awk_value))
	{
		switch (awk_value.val_type)
		{
			case AWK_STRING:
				value.s = awk_value.str_value.str;
				break;
			case AWK_ARRAY:
				fatal(ext_id, "tree_insert: Attempt to use array value as a scalar");
				break;
			default:
				fatal(ext_id, "tree_insert: Invalid value type given");
		}
		
		tree_insert(awk_query.str_value.str, value);
	}
	else
		fatal(ext_id, "tree_insert: Invalid arguments");

	return make_number(1, result); // assume success if we get to this point
}

static awk_value_t* do_query_tree(const int nargs, awk_value_t* result, struct awk_ext_func* _)
{
	assert(result != NULL);

 	awk_value_t awk_query;   
	foint data;

	if (!get_argument(0, AWK_STRING, &awk_query))
		fatal(ext_id, "query_tree: Invalid arguments");

	query_tree(awk_query.str_value.str, &data);
	return make_const_string(data.s, strlen(data.s), result);
}

static awk_value_t* do_is_tree(const int nargs, awk_value_t* result, struct awk_ext_func* _)
{
	assert(result != NULL);

	awk_value_t awk_query;
	char* query;

	if (get_argument(0, AWK_STRING, &awk_query))
	{
		query = awk_query.str_value.str;
	}
	else 
		fatal(ext_id, "is_tree: Invalid arguments");


	double ret = is_tree(query);
	return make_number(ret, result);
}

static awk_value_t* do_tree_next(const int nargs, awk_value_t* result, struct awk_ext_func* _)
{
	assert(result != NULL);

	awk_value_t awk_name;
	foint _htree, _result;
	char* query;

	if (get_argument(0, AWK_STRING, &awk_name))
		query = awk_name.str_value.str;
	else
		fatal(ext_id, "get_tree_next: Invalid arguments");

	const char* ret = tree_next(query);
	return make_const_string(ret, strlen(ret), result);
}

static awk_value_t* do_tree_iter_done(const int nargs, awk_value_t* result, struct awk_ext_func* _)
{
	assert(result != NULL);

	awk_value_t awk_query;
	char* query;

	if (get_argument(0, AWK_STRING, &awk_query))
		query = awk_query.str_value.str;
	else
		fatal(ext_id, "tree_iter_done: Invalid args");

	const bool force = nargs > 1;
	const bool ret = tree_iter_done(query, force);
	return make_number((double)ret, result);
}

static awk_ext_func_t func_table[] = 
{
	{ "create_tree", do_create_tree, 2, 2, awk_false, NULL },
	{ "tree_insert",  do_tree_insert, 2, 2, awk_false, NULL },
	{ "query_tree",  do_query_tree, 1, 1, awk_false, NULL },
	{ "is_tree",  do_is_tree, 1, 1, awk_false, NULL },
	{ "tree_next", do_tree_next, 1, 1, awk_false, NULL},
	{ "tree_iter_done",  do_tree_iter_done, 2, 1, awk_false, NULL }
};
dl_load_func(func_table, htrees, "");
