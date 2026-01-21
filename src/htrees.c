#include "htrees.h"
#include "stack.h"

#include <string.h>
#include <tinyexpr.h>

#include <stdlib.h>

/* NOTE from gawk API documentation:
 * All pointers filled in by gawk point to memory managed by gawk and should be treated by the extension as read-only.

Memory for all strings passed into gawk from the extension must come from calling one of gawk_malloc(), gawk_calloc(), or gawk_realloc(), and is managed by gawk from then on */


// HTrees, found by their name in a gawk program, are contained here
TREETYPE* trees = NULL; 

// Stack of node queues (LL*)
STACK* current_iterators = NULL;

void free_htree(foint tree)
{
	HTreeFree((HTREE*)tree.v);
}

bool init_trees()
{
	trees = TreeAlloc((pCmpFcn)strcmp, (pFointCopyFcn)strdup, (pFointFreeFcn)free, NULL, (pFointFreeFcn)free_htree);
	current_iterators = StackAlloc(4);
	// NOTE: 4 seems like a reasonable maximum (before the stack has to resize) of for-in loops going at once

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

void delete_tree(const char* name)
{
	TreeDelete(trees, (foint){.s=name});
}

const double tree_length(const char* name)
{
	foint* _htree = TreeLookup(trees, (foint){.s=name});

	if (_htree != NULL)
	{
		HTREE* htree = _htree->v;
		return htree->n;
	}
	else
	{
		return strlen(name);
	}
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

static foint* get_element(const char* tree, const char** subscripts, const unsigned char depth)
{
	foint _htree;
	foint keys[depth];
	fill_foints(subscripts, keys, depth);
	foint* result;

	if (STreeLookup(trees, (foint){.s=tree}, &_htree))
	{
		HTREE* htree = _htree.v;

		if (depth != htree->depth)
		{
			fputs("query_tree: Incorrect number of subscripts given for tree depth; treating array as a scalar value\n", stderr);
			exit(1);
			// May have to changes instances of fputs -> exit to a special return value checked by funcs in gawk_ext.c to then use fatal()
		}

		result = HTreeLookup(htree, keys);
		
		if (result == NULL)
		{
			result = HTreeInsert(htree, keys, (foint){.s=""});
		}
	}
	else
	{
		HTREE* htree = create_tree(tree, depth);
		result = HTreeInsert(htree, keys, (foint){.s=""});
	}

	return result;
}

const foint query_tree(const char* tree, const char** subscripts, const unsigned char depth)
{
	return *get_element(tree, subscripts, depth);
}

static char* remove_trailing_zeroes(char* num)
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
		
	return realloc(num, (strlen(num) + 1) * sizeof(char));
}

const double tree_modify(const char* tree, const char** subscripts, const unsigned char depth, const char* expr)
{
	foint* result = get_element(tree, subscripts, depth); 
	double x = atof(result->s);

	// TODO: compare speed / mem if we do branch into just te_interp
	// if (strchr(expr, 'x') == NULL)
	
	te_variable vars[] = { {"x", &x} };
	int err;
	char* _expr = malloc((strlen(result->s) + strlen(expr) + 1) * sizeof(char));
	strcpy(_expr, result->s);
	strcat(_expr, expr);
	te_expr* te = te_compile(_expr, vars, 1, &err);

	if (te != NULL)
	{
		x = te_eval(te);
		te_free(te);
		free(_expr);

		// TODO: 64 max for now, see if we can figure out exact size from mantissa digits * max_10_exp in float.h
		result->s = realloc(result->s, 64 * sizeof(char)); 
		sprintf(result->s, "%f", x);
		result->s = remove_trailing_zeroes(result->s);

		return x;
	}
	else
	{
		fputs("tree_modify: Invalid expression given; parse error\n", stderr);
		exit(1);
	}
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
			fputs("tree_increment: Incorrect number of subscripts given for tree depth; treating array as a scalar value\n", stderr);
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
	// + 8 = 1 for '.' and \0, 6 for default precision of %f
	num += amount * mult;
	result->s = realloc(result->s, len * sizeof(char)); 
	sprintf(result->s, "%f", num);
	result->s = remove_trailing_zeroes(result->s);

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

void tree_remove(const char* tree, const char** subscripts, const unsigned char depth)
{
	foint _htree;
	bool result;
	bool found = STreeLookup(trees, (foint){.s=tree}, &_htree);

	if (found)
	{
		HTREE* htree = _htree.v;
		foint _subscripts[depth];
		fill_foints(subscripts, _subscripts, depth);
		HTreeLookDel(htree, _subscripts, depth, true);
	}
}

const unsigned short is_tree(const char* tree, const char** subscripts, const unsigned char depth)
{
	foint _subscripts[depth];
	fill_foints(subscripts, _subscripts, depth);
	foint _htree;

	if (!STreeLookup(trees, (foint){.s=tree}, &_htree))
		return 0;

	HTREE* htree = _htree.v;

	if (!SHTreeLookup(htree, _subscripts, depth, NULL))
		return 0;

	return depth < htree->depth;
}

typedef struct _iterator
{
	unsigned int hash; // Needed to validate if a certain iterator already exists or not
	LINKED_LIST* current;
} ITERATOR;

static const unsigned int hash_query(const char* tree, const char** subscripts, const unsigned char depth)
{
	unsigned int length = strlen(tree);
	char* query = malloc((length + 1) * sizeof(char));
	strcpy(query, tree);

	for (unsigned char i = 0; i < depth; ++i)
	{
		length += strlen(subscripts[i]) + 1;
		query = realloc(query, (length + 1) * sizeof(char)); // TODO: analyze this function and see if one alloc is better (maybe with sprintf or stpcpy)
		strcat(query, " "); // Delimiter needed to prevent mismatches
		strcat(query, subscripts[i]);
	}

	const unsigned char p = 67;
	const unsigned int m = 1e9 + 9;
	unsigned int result = 0;
	unsigned int p_raised_i = 1;

	for (unsigned int i = 0; i < length; ++i)
	{
		result = (result + query[i] * p_raised_i) % m;
		p_raised_i = (p_raised_i * p) % m;
	}

	free(query);
	return result;
}

// Creates an iterator if one is not found for the given query, and the HTREE of that name has valid elements to create one
static LINKED_LIST* get_iterator(const char* tree, const char** subscripts, const unsigned char depth)
{
	foint _htree;

	if (!STreeLookup(trees, (foint){.s=tree}, &_htree))
		return NULL;

	HTREE* htree = _htree.v;
	const unsigned char htree_depth = htree->depth;
	
	if (depth == htree_depth)
	{
		fputs("Attempt to iterate through a scalar value\n", stderr);
		exit(1);
	}

	ITERATOR* last;
	unsigned int hash;

	if (StackSize(current_iterators) == 0 ||
	(hash = hash_query(tree, subscripts, depth)) != (last = StackTop(current_iterators).v)->hash)
	{
		if (htree->tree->root == NULL)
		{
			return NULL;
		}
		else
		{
			ITERATOR* new = malloc(sizeof(ITERATOR));
			new->current = LinkedListAlloc(NULL, false);
			new->hash = hash;
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

			LinkedListAppend(new->current, root_node);
			foint iterator = {.v=new};
			StackPush(current_iterators, iterator);

			return new->current;
		}
	}
	else
	{
		return last->current;
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
	foint result;
	LINKED_LIST* node_queue = get_iterator(tree, subscripts, depth);

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
	{
		fputs("get_tree_next: No items in tree\n", stderr); 
		return "";
	}

}

static void iterators_pop()
{
	ITERATOR* item = StackPop(current_iterators).v;
	LinkedListFree(item->current);
	free(item);
}

// NOTE: Might have to change to larger return type
const unsigned int tree_iters_remaining(const char* tree, const char** subscripts, const unsigned char depth)
{
	foint result;
	LINKED_LIST* node_queue = get_iterator(tree, subscripts, depth);
	
	if (node_queue == NULL) // No elements found to iterate
		return 0;

	const int remaining = LinkedListSize(node_queue);

	if (remaining == 0)
		iterators_pop();

	return remaining;
}

void tree_iter_break()
{
	if (StackSize(current_iterators) == 0)
	{
		fputs("tree_iter_break: Cannot break if the corresponding iterator does not exist", stderr);
		exit(1);
	}

	iterators_pop();
}

void do_at_exit(void* data, int exit_status)
{
	while(StackSize(current_iterators) > 0)
		iterators_pop();

	TreeFree(trees);
	StackFree(current_iterators);
}
