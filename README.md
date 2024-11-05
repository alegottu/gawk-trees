It's possible to compile htrees.so using the libwayne.a library like the following:
```
    gcc -fPIC -shared -DHAVE_CONFIG_H -c -O -g -Igawk -Ilibwayne/include -Iinclude src/htrees.c
    gcc -Wl,--no-undefined -shared -o htrees.so htrees.o -Llibwayne -lwayne
```
but libwayne.a needs to be recompiled with -fPIC flags enabled in order to contribute to a shared object.

Otherwise, in order to compile htrees.so with just source and header files, do the following:
1. Clone this repository
2. Use "git submodule update --init"
3. Use "make" or "make debug"

How to use the extension:
1. Follow the steps above to compile the file "htrees.so" in your current directory
2. Make sure to set the environment vairable "AWKLIBPATH" to your current directory (or wherever you have your "htrees.so" file)
3. At the top of your awk program, write @load "htrees"
4. Refer to the list of extension functions below to make proper use of htrees in your awk code

create_tree("name", depth)

tree_insert("name+subscripts", value)
	the first parameter is the name of the tree concatenated with the desired
		subscripts, e.g. "name[x][y]"
	creates a tree called "name" if one doesn't yet exist
	creates a default value under the subscripts if one doesn't exit yet

query_tree("name+subscripts")
	as with tree_insert, the first and only parameter is the name of the tree
		concatenated with the desired subscripts, e.g. "name[x][y]"
 	instantiates the value for the subscript(s) if there isn't one already
 	instantiates the entire tree as well if there isn't one named that already

delete_tree("name")

is_tree("query")
	checks whether the element within a given tree is an array (bintree) or scalar value
	for example, is_tree("htree[x]") would return 1 if "htree" represents a 2D array

tree_next("query")
	use in a loop to iterate through all the indices for the immdiate
	depth of the query; works the same as using "for index in array"
	in gawk, where array would be a standard gawk array

tree_iter_done("query", [force])
    returns 0 if the iterator for the binary tree
 	at "query" has more elements remaining, or 1 if the final element was returned with the
	most recent call of "get_tree_next"
	if the binary tree at "query" is not currently being iterated on, an iterator will be created
	if the optional "force" argument is supplied (in any manner), the iterator will be freed
	forcefully and this will return 1
