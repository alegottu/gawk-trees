#include "htrees.h"
#include <stdlib.h>

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

bool init_trees()
{
	trees = TreeAlloc((pCmpFcn)strcmp, (pFointCopyFcn)strdup, (pFointFreeFcn)free, NULL, (pFointFreeFcn)free_htree);
	current_iterators = TreeAlloc((pCmpFcn)strcmp, (pFointCopyFcn)strdup, (pFointFreeFcn)free, NULL, (pFointFreeFcn)LinkedListFree); 

	on_exit((void*)do_at_exit, NULL); // TODO: possible to use 2nd arg instead of global trees?

	return trees != NULL;
}

// NOTE: Ignoring "discards qualifiers" when it's only because it's being put into a foint;
// e.g. TreeInsert creates a copy of the string/key, so the char* in foint may as well be const char*
HTREE* create_tree(const char* name, const int depth) 
{
	HTREE* array = HTreeAlloc(depth, (pCmpFcn)strcmp, (pFointCopyFcn)strdup, (pFointFreeFcn)free, (pFointCopyFcn)strdup, (pFointFreeFcn)free);
	TreeInsert(trees, (foint){.s=name}, (foint){.v=array});

	return array;
}

const bool delete_tree(const char* name)
{
	return TreeDelete(trees, (foint){.s=name}) != NULL;
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
	foint _htree;
	HTREE* htree;
	
	if (STreeLookup(trees, (foint){.s=tree}, &_htree)) 
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
	foint _htree;
	bool found;

	if (STreeLookup(trees, (foint){.s=tree}, &_htree))
	{
		HTREE* htree = _htree.v;

		if (depth != htree->depth)
		{
			fputs("query_tree: Incorrect number of subcripts given for tree depth; treating array as a scalar value\n", stderr);
			exit(1);
			// May have to changes instances of fputs -> exit to a special return value checked by funcs in gawk_ext.c to then use fatal()
		}

		found = SHTreeLookup(htree, keys, depth, result);
		
		if (!found)
		{
			*result = *HTreeInsert(htree, keys, (foint){.s=""});
		}
	}
	else
	{
		found = false;
		HTREE* htree = create_tree(tree, depth);
		*result = *HTreeInsert(htree, keys, (foint){.s=""});
	}

	return found;
}

static void remove_trailing_zeroes(char* num)
{
	unsigned char first_last_zero_idx = 0;
	const char* from_point = strchr(num, '.');
	
	for (unsigned char i = strlen(from_point)-1; i > 0; --i)
	{
		if (from_point[i] == '0') first_last_zero_idx = i;
		else break;
	}

	unsigned char end_pos;
	if (first_last_zero_idx == 1)
		end_pos = from_point - num;
	else
	 	end_pos = from_point + first_last_zero_idx - num;
	num[end_pos] = '\0';
		
	num = realloc(num, (strlen(num) + 1) * sizeof(char));
}

// NOTE: mult is always -1 or 1
const double increment(const char* tree, const char** args, const unsigned char argc, const char mult)
{
	foint _htree;
	double amount = 1;
	unsigned char amount_digits = 1;
	foint* result;

	if (STreeLookup(trees, (foint){.s=tree}, &_htree))
	{
		HTREE* htree = _htree.v;
		unsigned char depth;

		if (argc == htree->depth)
			depth = argc;
		else
		{
			amount = atof(args[argc-1]);
			amount_digits = strlen(args[argc-1]);
			depth = argc-1;
		}

		foint keys[depth];
		fill_foints(args, keys, depth);

		if (depth != htree->depth)
		{
			fputs("tree_increment: Incorrect number of subcripts given for tree depth; treating array as a scalar value\n", stderr);
			exit(1);
		}

		result = HTreeLookup(htree, keys);
		
		if (result == NULL)
		{
			result = HTreeInsert(htree, keys, (foint){.s="0"});
		}
	}
	else
	{
		amount = atof(args[argc-1]);
		amount_digits = strlen(args[argc-1]);
		unsigned char depth = argc-1;

		if (amount == 0)
		{
			amount = 1;
			amount_digits = 1;
			depth = argc;
		}

		foint keys[depth];
		fill_foints(args, keys, depth);
		HTREE* htree = create_tree(tree, depth);
		result = HTreeInsert(htree, keys, (foint){.s="0"});
	}

	double num = atof(result->s);
	unsigned int len = (num == 0 ? 1 : strlen(result->s)) + amount_digits + 8;
	// + 8 = 1 for \0 and '.', 6 for default precision of %f
	num += amount * mult;
	result->s = realloc(result->s, (len + 8) * sizeof(char)); 
	sprintf(result->s, "%f", num);
	remove_trailing_zeroes(result->s);

	return num;
}

const double tree_increment(const char* tree, const char** args, const unsigned char argc)
{
	return increment(tree, args, argc, 1);
}

const double tree_decrement(const char* tree, const char** args, const unsigned char argc)
{
	return increment(tree, args, argc, -1);
}

const bool tree_elem_exists(const char* tree, const char** subscripts, const unsigned char depth)
{
	foint _htree;
	STreeLookup(trees, (foint){.s=tree}, &_htree);
	HTREE* htree = _htree.v;
	foint _subscripts[depth];
	fill_foints(subscripts, _subscripts, depth);

	return SHTreeLookup(htree, _subscripts, depth, NULL);
}

const bool tree_remove(const char* tree, const char** subscripts, const unsigned char depth)
{
	foint _htree;
	bool result;
	bool found = STreeLookup(trees, (foint){.s=tree}, &_htree);

	if (!found)
		return false;

	HTREE* htree = _htree.v;
	foint _subscripts[depth];
	fill_foints(subscripts, _subscripts, depth);
	
	return HTreeLookDel(htree, _subscripts, depth, true) != NULL;
}

const unsigned short is_tree(const char* tree, const char** subscripts, const unsigned char depth)
{
	foint _subscripts[depth];
	fill_foints(subscripts, _subscripts, depth);
	foint _htree;

	if (!STreeLookup(trees, (foint){.s=tree}, &_htree))
		return 0;

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
		result = realloc(result, (length + 2) * sizeof(char));
		strcat(result, subscripts[i]);
		strcat(result, " "); // space needed to prevent mismatches
	}

	return result;
	// might want to analyze this function and see if one alloc is better (maybe with sprintf or stpcpy), although might be insignificant
}

// Creates an iterator if one is not found for the given query, and the HTREE of that name has valid elements to create one
static LINKED_LIST* get_iterator(const char* tree, const char* query, const char** subscripts, const unsigned char depth)
{
	foint iterator, _htree;
	LINKED_LIST* result;

	if (!STreeLookup(trees, (foint){.s=tree}, &_htree))
		return NULL; // NOTE: should possibly be forced exit

	HTREE* htree = _htree.v;
	const unsigned char htree_depth = htree->depth;
	
	if (depth == htree_depth)
	{
		fputs("Attempt to iterate through a scalar value\n", stderr);
		exit(1);
	}

	foint _query = {.s=query};
			
	if (!STreeLookup(current_iterators, _query, &iterator))
	{
		if (htree->tree->root == NULL)
		{
			return NULL;
		}
		else
		{
			result = LinkedListAlloc(NULL, false);
			foint root_node;

			if (depth == 0)
				root_node.v = htree->tree->root;
			else
			{
				foint _subscripts[depth]; foint result;
				fill_foints(subscripts, _subscripts, depth);
				SHTreeLookup(htree, _subscripts, depth, &result);
				root_node.v = ((TREETYPE*)result.v)->root;
			}

			LinkedListAppend(result, root_node);
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
		node_queue = get_iterator(tree, tree, subscripts, depth);	
	else
	{
		char* query = get_full_query(tree, subscripts, depth);
		node_queue = get_iterator(tree, query, subscripts, depth);
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

static void tree_iters_remaining_exit(char* query)
{
	free(query);	
}

static void tree_iters_remaining_exit_no_free(char* _) { }

// NOTE: Might have to change to larger return type
const unsigned int tree_iters_remaining(const char* tree, const char** subscripts, const unsigned char depth)
{
	LINKED_LIST* node_queue;
	char* query;
	foint result, _query;
	void (*finish)(char*);

	/* NOTE: the stucture of the function is a bit convoluted because
	  we can't safely delete an element from the iterators tree without
	  the TreeLookDel function, even though we previously just found
	  the same element with the same function; if the logic is seperated
	  a bit in libwayne, we can optimize this	*/
	if (depth == 0)
	{
		_query.s = tree;
		node_queue = get_iterator(tree, tree, subscripts, depth);
		finish = &tree_iters_remaining_exit_no_free;
	}
	else
	{
		query = get_full_query(tree, subscripts, depth);
		_query.s = query;
		node_queue = get_iterator(tree, query, subscripts, depth);
		finish = &tree_iters_remaining_exit;
	}

	if (node_queue == NULL) // No elements found to iterate
	{
		finish(query);
		return 0;
	}

	const int remaining = LinkedListSize(node_queue);

	if (remaining == 0)
	{
		TreeDelete(current_iterators, _query);
	}

	finish(query);
	return remaining;
}

const bool tree_iter_break(const char* tree, const char** subscripts, const unsigned char depth)
{
	foint query;

	if (depth == 0)
		query.s = tree;
	else
		query.s = get_full_query(tree, subscripts, depth);

	return TreeDelete(current_iterators, query);
}

void do_at_exit(void* data, int exit_status)
{
	TreeFree(trees);
	TreeFree(current_iterators);
}
