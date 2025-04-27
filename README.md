In order to compile htrees.so with just source and header files, do the following:
1. Clone this repository
2. Use "git submodule update --init"
3. Use "make" or "make debug"

If libwayne.a is recompiled with -fPIC flags enabled in order to contribute to a shared object, it's possible to compile htrees.so using the libwayne.a library file. If you'd prefer this, use the awk_value_t branch, then make.

How to use the extension:
1. Follow the steps above to compile the file "htrees.so" in your current directory
2. Make sure to set the environment variable "AWKLIBPATH" to the gawk-trees/bin directory (or wherever you have your "htrees.so" file)
3. At the top of your awk program, write @load "htrees", or use -lhtrees as an option to the gawk command
4. Refer to the list of extension functions below to use htrees in your awk code; on the left is what you would
see in awk code without the extension, on the right a replacement the extension offers using htrees. The
extension functions behave exactly as standard awk arrays would unless otherwise noted. 

N/A                         create_tree("name", depth)
                            -> creates an empty htree with depth dimensions, held by the extension under "name"

delete name                 delete_tree("name")
                            -> specifically used for deleting the entire tree, see tree_remove for deleting
                            elements

name["subscript"] = value   tree_insert("name", "subscript", value)
arr[1][2] = value           tree_insert("arr", 1, 2, value)

name["subscript"]           query_tree("name", "subscript")
arr[1][2]                   query_tree("arr", 1, 2)

name["subscript"]++         tree_increment("name", "subcript")
arr[1][2]--                 tree_decrement("arr", 1, 2)
test[4]["a"]+=7.93          tree_increment("test", 4, "a", 7.93)
                            -> by default, the value by which the element at the query increments is 1,
                            but you can specify it as the last argument, including as a negative value,
                            effectively changing the operation, and as a real number, as shown above;
                            be aware that in the case that there is no tree that exists under the name
                            given as the first argument, the last argument will always be treated as the
                            amount to increment as long as it is a valid number, since for a non-existent
                            tree it would be impossible to distinguish between a final optional argument
                            and an unknown number of subscripts

name["subscript"]*=2        tree_insert("name", "subscript", tree_query("name", "subscript")*2)
                            -> for now, for any other operations that modify a value already in the tree,
                            you can follow this pattern, although in the future there may be a different
                            interface for each separate operation

delete name["subscript"]    tree_remove("name", "subscript")
delete arr[1][2]            tree_remove("arr", 1, 2)

"subscript" in name         tree_elem_exists("name", "subscript")
2 in arr[1]                 tree_elem_exists("arr", 1, 2)

is_array(name["subscript"]) is_tree("name", "subscript")
is_array(arr[1][2])         is_tree("arr", 1, 2)

for i in name {...}         while (tree_iters_remaining("name") > 0) {i = tree_next("name"); ...}
for i in arr[1][2] {...}    while (tree_iters_remaining("arr", 1, 2) > 0) {i = tree_next("arr", 1, 2); ...}
                            -> both tree_next and tree_iters_remaining will create an internal iterator within
                            the extension if one under the query (for a tree or subtree) doesn't exist yet.
                            The iterator does not hold all the elements in the tree in memory at once, so the
                            return value of tree_iters_remaining does not reflect the total number of elements
                            at the query. Once tree_iters_remaining = 0, the given iterator will be freed and
                            therefore reset; this can be done forcibly using tree_iter_break. The order of the
                            indices returned by tree_next are according to a DFS of the tree at the query
                            

for i in name {break}       while (tree_iters_remaining("name") > 0) 
                            {i = tree_next("name"); tree_iter_break("name")}
for i in arr[1][2] {break}  while (tree_iters_remaining("arr", 1, 2) > 0)
                            {i = tree_next("arr", 1, 2); tree_iter_break("arr", 1, 2)}
                            -> if you break out of a while loop using the pattern shown above without using
                            this function, the next time you use that iterator, it will resume from where it
                            was previously. To forcefully reset the iterator at the query, or just to free it
                            from memory, use this function.
