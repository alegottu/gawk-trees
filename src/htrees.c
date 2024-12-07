#include "htrees.h"

/* NOTE from gawk API documentation:
 * All pointers filled in by gawk point to memory managed by gawk and should be treated by the extension as read-only.

Memory for all strings passed into gawk from the extension must come from calling one of gawk_malloc(), gawk_calloc(), or gawk_realloc(), and is managed by gawk from then on */


// HTrees, found by their name in a gawk program, are contained here
TREETYPE* trees = NULL; 

// keys = tree or sub-tree name, value = node queue (LL*)
TREETYPE* current_iterators = NULL; 

void free_htree(foint tree)
{
	HTreeFree((HTREE*)tree.v);
}

static foint copy_str(foint info)
{
	foint ret;
	ret.s = malloc((strlen(info.s) + 1) * sizeof(char));
	strcpy(ret.s, info.s);
	return ret;
}

bool init_trees()
{
	trees = TreeAlloc((pCmpFcn)strcmp, (pFointCopyFcn)strdup, (pFointFreeFcn)free, NULL, (pFointFreeFcn)free_htree); 
	current_iterators = TreeAlloc((pCmpFcn)strcmp, (pFointCopyFcn)strdup, (pFointFreeFcn)free, NULL, (pFointFreeFcn)LinkedListFree); 

	on_exit((void*)do_at_exit, NULL); // possible to use 2nd arg instead of global trees

	return trees != NULL;
}

// NOTE: Ignoring "discards qualifiers" when it's only because it's being put into a foint;
// e.g. TreeInsert creates a copy of the string/key, so the char* in foint may as well be const char*
HTREE* create_tree(const char* name, const int depth) 
{
	// possibly need free foint fcn
	HTREE* array = HTreeAlloc(depth, (pCmpFcn)strcmp, (pFointCopyFcn)strdup, (pFointFreeFcn)free, (pFointCopyFcn)copy_str, (pFointFreeFcn)free);
	TreeInsert(trees, (foint){.s=name}, (foint){.v=array});

	return array;
}

const bool delete_tree(const char* name)
{
	return TreeDelete(trees, (foint){.s=name});
}

static void fill_foints(const char** strs, foint* result, const unsigned char count)
{
	for (unsigned char i = 0; i < count; ++i)
	{
		result[i].s = strs[i];
	}
}

void tree_insert(const char* tree, const char** subscripts, const foint value, const unsigned char depth)
{
	foint _htree, key;
	key.s = tree;
	HTREE* htree;
	
	if (TreeLookup(trees, key, &_htree)) 
	{
		htree = _htree.v;
		foint keys[depth];
		fill_foints(subscripts, keys, depth);
		HTreeInsert(htree, keys, value);
	}
	else 
	{
		foint keys[depth];
		fill_foints(subscripts, keys, depth);
		htree = create_tree(tree, depth);
		HTreeInsert(htree, keys, value);
	}
}

const bool query_tree(const char* tree, const char** subscripts, foint* result, const unsigned char depth)
{
	foint keys[depth];
	fill_foints(subscripts, keys, depth);
	foint _htree, _name; _name.s = tree;
	bool found = false;

	if (TreeLookup(trees, _name, &_htree))
	{
		HTREE* htree = _htree.v;

		if (depth != htree->depth)
		{
			fputs("query_tree: Incorrect number of subcripts given for tree depth; returning arrays not yet implemented\n", stderr);
			exit(1);
			// May have to changes instances of fputs -> exit to a special return value checked by funcs in gawk_ext.c to then use fatal()
		}

		found = HTreeLookup(htree, keys, result);
		
		if (!found)
		{
			result->s = "";
			HTreeInsert(htree, keys, *result);
		}
	}
	else
	{
		found = false;
		HTREE* htree = create_tree(tree, depth);
		result->s = "";
		HTreeInsert(htree, keys, *result); 
	}

	return found;
}

// TODO: add depth parameter to test if non-final elem exists
const bool tree_elem_exists(const char* tree, const char** subscripts)
{
	foint _htree;
	TreeLookup(trees, (foint){.s=tree}, &_htree);
	HTREE* htree = _htree.v;
	const unsigned char depth = htree->depth;
	foint _subscripts[depth];
	fill_foints(subscripts, _subscripts, depth);

	foint result;
	return HTreeLookup(htree, _subscripts, &result);
}

const bool tree_remove(const char* tree, const char** subscripts, const unsigned char depth)
{
	foint _htree;
	TreeLookup(trees, (foint){.s=tree}, &_htree);
	HTREE* htree = _htree.v;
	const unsigned char htree_depth = htree->depth;
	foint _subscripts[depth];
	fill_foints(subscripts, _subscripts, depth);

	if (depth < htree_depth)
	{
		htree->depth = depth; // tricks HTreeLookDel to finish early	
		foint tree;
		HTreeLookDel(htree, _subscripts, &tree);
		TreeFree(tree.v);
		bool result = HTreeLookDel(htree, _subscripts, (foint*)1); // inefficient, but stops other code from breaking; revisit this and the other use like it
		htree->depth = htree_depth;
		return result;
	}

	return HTreeLookDel(htree, _subscripts, (foint*)1);
}

const unsigned short is_tree(const char* tree, const char** subscripts, const unsigned char depth)
{
	foint _subscripts[depth];
	fill_foints(subscripts, _subscripts, depth);
	foint _htree;

	if (!TreeLookup(trees, (foint){.s=tree}, &_htree))
	{
		fputs("Invalid tree name\n", stderr);
		exit(1);
	}

	HTREE* htree = _htree.v;

	// could possibly use htree->depth - depth instead
	if (depth < htree->depth)
		return 1;
	else if (depth == htree->depth)
		return 0;

	fputs("Excess subscripts given for this tree's depth\n", stderr);
	exit(1);
}

// NOTE: Make sure to free the result of this function
static const char* get_full_query(const char* tree, const char** subscripts, const unsigned char depth)
{
	unsigned int length = strlen(tree) + strlen(subscripts[0]);
	char* result = malloc((length + 1) * sizeof(char));
	strcpy(result, tree);
	strcat(result, subscripts[0]);

	for (unsigned char i = 1; i < depth; ++i)
	{
		length += strlen(subscripts[i]);
		result = realloc(result, (length + 1) * sizeof(char));
		strcat(result, subscripts[i]);
	}

	return result;
	// might want to analyze this function and see if one alloc is better (maybe with sprintf or stpcpy), although might be insignificant
}

// Creates an iterator if one is not found for the given query, and the HTREE of that name has valid elements to create one
static LINKED_LIST* get_iterator(const char* tree, const char* query, const unsigned char depth)
{
	foint iterator, _htree;
	LINKED_LIST* result;

	if (!TreeLookup(trees, (foint){.s=tree}, &_htree))
	{
		fputs("No tree found for an iterator\n", stderr);
		exit(1);
	}

	HTREE* htree = _htree.v;
	
	if (depth == htree->depth)
	{
		fputs("Attempt to iterate through a scalar value\n", stderr);
		exit(1);
	}

	foint _query = {.s=query};
			
	if (!TreeLookup(current_iterators, _query, &iterator))
	{
		result = LinkedListAlloc(NULL, false);

		if (htree->tree->root == NULL)
		{
			return NULL;
		}
		else
		{
			LinkedListAppend(result, (foint){.v=htree->tree->root});
			iterator.v = result;
			TreeInsert(current_iterators, _query, iterator);

			return result;
		}
	}
	else
	{
		return iterator.v;
	}
}

static Boolean visit_next_node(LINKED_LIST* iterator, foint* result)
{
	if (LinkedListSize(iterator) == 0)
		return false;

	NODETYPE* current_node = LinkedListPop(iterator).v;

	if (current_node->left != NULL)
		LinkedListAppend(iterator, (foint){.v=current_node->left});
	if (current_node->right != NULL)
		LinkedListAppend(iterator, (foint){.v=current_node->right});

	*result = current_node->key;
	return true;
}

// Returns the next index, not the next element
const char* tree_next(const char* tree, const char** subscripts, const unsigned char depth)
{
	LINKED_LIST* node_queue;
	foint result;

	if (depth == 0)
		node_queue = get_iterator(tree, tree, depth);	
	else
	{
		char* query = get_full_query(tree, subscripts, depth);
		node_queue = get_iterator(tree, query, depth);
		free(query);
	}

	if (node_queue != NULL)
	{
		if (visit_next_node(node_queue, &result))
		{
			return result.s;
		}
		else
		{
			fputs("get_tree_next: Attempt to infinitely iterate through the given tree\n", stderr);
			exit(1);
		}
	}
	else
		fputs("get_tree_next: No items in tree\n", stderr); 

	return "ERROR";
}

static const bool tree_iter_done_exit(char* query, const bool result)
{
	free(query);	
	return result;
}

static const bool tree_iter_done_exit_no_free(char* _, const bool result) { return result; }

const bool tree_iter_done(const char* tree, const char** subscripts, const unsigned char depth, const bool force)
{
	LINKED_LIST* node_queue;
	char* query;
	foint result, _query;
	const bool (*finish)(char*, const bool);

	if (depth == 0)
	{
		_query.s = tree;
		node_queue = get_iterator(tree, tree, depth);
		finish = &tree_iter_done_exit_no_free;
	}
	else
	{
		query = get_full_query(tree, subscripts, depth);
		_query.s = query;
		node_queue = get_iterator(tree, query, depth);
		finish = &tree_iter_done_exit;
	}

	if (node_queue == NULL) // No elements found to iterate
		return finish(query, true);

	if (force)
	{
		TreeDelete(current_iterators, _query);
		return finish(query, true);
	}

	if (LinkedListSize(node_queue) == 0)
	{
		TreeDelete(current_iterators, _query);
		return finish(query, true);
	}

	return finish(query, false);
}

void do_at_exit(void* data, int exit_status)
{
	TreeFree(trees);
	TreeFree(current_iterators);
}
