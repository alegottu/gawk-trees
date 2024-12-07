In order to compile htrees.so with just source and header files, do the following:
1. Clone this repository
2. Use "git submodule update --init"
3. Use "make" or "make debug"

If libwayne.a needs is recompiled with -fPIC flags enabled in order to contribute to a shared object, it's possible to compile htrees.so using the libwayne.a library like the following:
```
    gcc -fPIC -shared -DHAVE_CONFIG_H -c -O -g -Igawk -Ilibwayne/include -Iinclude src/htrees.c
    gcc -Wl,--no-undefined -shared -o htrees.so htrees.o -Llibwayne -lwayne
```

How to use the extension:
1. Follow the steps above to compile the file "htrees.so" in your current directory
2. Make sure to set the environment vairable "AWKLIBPATH" to your current directory (or wherever you have your "htrees.so" file)
3. At the top of your awk program, write @load "htrees"
4. Refer to the list of extension functions below to use htrees in your awk code

create_tree("name", depth)

delete_tree("name")

tree_insert("name", "subscript1", [...], value)
- each subscript allows its own parameter
- creates a tree called "name" if one doesn't yet exist
- creates a default value under the subscripts if one doesn't exit yet

query_tree("name", "subscript1", [...])
- as with tree_insert, each subscript allows its own parameter
- instantiates the value for the subscript(s) if there isn't one already
- instantiates the entire tree as well if there isn't one named that already

tree_remove("name", "subscript1", [...])
- removes the element at the query

tree_elem_exists("name", subscript1", [...])
- returns 1 if the element at the query exists, 0 otherwise

is_tree("name", "subscript1", [...])
- checks whether the element within a given tree is an array (bintree) or scalar value
- for example, is_tree("htree", "x") would return 1 if "htree" represents a 2D array

tree_next("name", "subscript1", [...])
- use in a loop to iterate through all the indices for the immediate
depth of the query; works the same as using "for index in array"
in gawk, where array would be a standard gawk array

tree_iter_done("name", "subscript1", [...], [force])
- returns 0 if the iterator for the binary tree
at the query has more elements remaining, or 1 if the final element was returned with the
most recent call of "get_tree_next"
- if the binary tree at the query is not currently being iterated on, an iterator will be created
- if the optional "force" argument is supplied, the iterator will be freed
forcefully and this will return 1; this argument must be supplied as a number to differentiate
it from all of the subscript parameters (may be changed later)
- otherwise, an iterator will be freed once each element has been iterated through
(once this function returns 1)
