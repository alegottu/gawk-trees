#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gawkapi.h"
#include "gettext.h"
#define _(msgid)  gettext(msgid)
#define N_(msgid) msgid

#include "gawkfts.h"
#include "htree.h"

#define MAX_SUBSCRIPTS 8 

static const gawk_api_t *api;    /* for convenience macros to work */
static awk_ext_id_t ext_id;
static const char *ext_version = "bintree extension: version 1.0";

int plugin_is_GPL_compatible;

// HTrees, found by their name in a gawk program, are contained here
BINTREE* trees = BinTreeAlloc((pCmpFcn) strcmp, (pFointCopyFcn) strdup, (pFointFreeFcn) free, NULL, NULL); 

static Boolean create_tree(char* name, int depth) 
{
    HTREE* array = HTreeAlloc(depth);
    foint data; foint.v = array;
    BinTreeInsert(trees, name, data);

    return true;
}

static awk_value_t* do_create_tree(int nargs, awk_value_t* result, struct awk_ext_func* _)
{
    awk_value_t awk_name, awk_depth;

    assert(result != NULL);

    if (get_argument(0, AWK_STRING, &awk_name) && get_argument(1, AWK_INT, &awk_depth))
    {
        char* name = awk_name->str_value.str;
        int depth = (int) awk_depth->num_value;
        create_tree(name, depth);
        // report error from create_tree
    }
    else
    {
        // report error
    }
}

// usage: tree_insert(name, subscripts, value, [split]);
// where subscripts are a string split by ',' e.g. x,y = [x][y]
// a custom token can be used to split the string through the optional 'split' parameter
static awk_value_t* do_tree_insert(int nargs, awk_value_t* result, struct awk_ext_func* _)
{
    awk_value_t awk_split, awk_name, awk_subscripts, awk_value;
    char* name, subscripts;
    foint value;
    char* split = ","; 

    assert(result != NULL);

    if (get_arugment(3, AWK_STRING, &awk_split))
    {
       split = awk_split->str_value.str; 
    }

    // handle value separately to discern between string or double
    if (get_argument(2, AWK_STRING, &awk_value))
    {
        value.s = awk_value->str_value.str;
    }
    else if (awk_value->val_type == AWK_NUMBER)
    {
        value.d = awk_value->num_value;
    }
    else
    {
        // report error, array illegal
    }
    
    if (get_arugment(0, AWK_STRING, &awk_name) &&
        get_argument(1, AWK_STRING, &awk_subscripts))
    {
        name = awk_name->str_value.str;
        subscripts = awk_subscripts->str_value.str;
        
        char* token = strtok(subscripts, split);
        int num_subscripts = 1;
        char* keys[MAX_SUBSCRIPTS];
        subs_arr[0] = token;

        while(token)
        { 
            token = strtok(NULL, split);
            subs_arr[num_subscripts++] = token;
            // possibly report error if more than max subs used
        }

        foint tree, key;
        key.s = name;

        if (!(BinTreeLookup(trees, key, &tree) && HTreeInsert(tree.v, keys, value))
        {
            // report error
        }
    }
    else
    {
        // report error
    }
}

static Boolean query_tree(char* name, char* subscripts[], int num_subscripts, foint* result)
{
    foint _name, _tree; _name.s = name; 
    Boolean found = false;

    if (BinTreeQuery(trees, _name, &_tree))
    {
        HTREE* tree = _tree.v;
        found = HTreeLookup(tree, subscripts, result);
        
        if (!found)
        {
            memset(result->s, '\0', 1);
            HTreeInsert(tree, subscripts, *result);
        }
    }
    else
    {
        HTREE* tree = HTreeAlloc(num_subscripts);
        _tree.v = tree;
        BinTreeInsert(trees, _name, _tree);
        memset(result->s, '\0', 1);
        HTreeInsert(tree, subscripts, result); 
    }

    return found;
}

// usage: query_tree(name, subscripts, [split]);
// where subscripts are a string split by ',' e.g. x,y = [x][y]
// a custom token can be used to split the string through the optional 'split' parameter
// instantiates the node for the subscript if there isn't one already
// instantiates the entire tree as well if there isn't one named that already
static awk_value_t* do_query_tree(int nargs, awk_value_t* result, struct awk_ext_func* _)
{
    
}

