### Building
1. Clone this repository
2. Use `make`

### Usage
1. Follow the steps above to compile the file "htrees.so" in your current directory
2. Make sure to set the environment variable "AWKLIBPATH" to the gawk-trees/bin directory (or wherever you have your "htrees.so" file)
3. If you have an existing program that you want to use htrees instead of standard gawk arrays, see [tools/convert](https://github.com/alegottu/gawk-trees/tree/main/tools/convert) which can help you convert your script automatically
4. Otherwise, refer to the table of examples below to use the extension in your awk code; on the left is what you would see in awk code without the extension, on the right a replacement the extension offers using htrees. The
extension functions behave exactly as standard awk arrays would unless otherwise noted
3. To load the extension before runtime and allow the script to run properly, you can either write `@load "htrees"` as the first line of your awk program, or use `-lhtrees` as an option to the gawk command

Note: if you encounter the error `version mismatch with gawk!` upon running, it's likely because you have a
version of the gawk API on your machine that's incompatible with the one used to compile this extension.
If you don't want to install a different version of the API, you can try building the extension with a different
version of the API instead using the following:
1. `cd gawk` then `git branch -r` to see possible versions
2. checkout the branch closest to your gawk API version (check with `gawk --version`)
3. `cd ..` then `make` again

| Standark AWK code | Using the "htrees" extension  | Extra Notes |
|-------------------|-------------------------------|-------------|
| N/A | create_tree("name", depth) | creates an empty htree with `depth` dimensions, held by the extension under "name"; htrees under the extension can't store elements at variable depths, and they also cannot expand or shrink their depth dynamically, as either of these would cost significant extra memory to upkeep |
| delete name | delete_tree("name") | specifically used for deleting the entire tree, see tree_remove for deleting elements |
| name["subscript"] = value | tree_insert("name", "subscript", value) | |
| arr[1][2] = value | tree_insert("arr", 1, 2, value) | |
| name["subscript"] | query_tree("name", "subscript") | |
| arr[1][2] | query_tree("arr", 1, 2) | |
| name["subscript"]++ | tree_increment("name", "subscript") | |
| arr[1][2]-- | tree_decrement("arr", 1, 2) | |
| test[4]["a"]+=7.93 | tree_increment("test", 4, "a", 7.93) | by default, the value by which the element at the query increments is 1, but you can specify it as the last argument, as shown above; be aware that in the case that there is no tree that exists under the name given as the first argument, the last argument will always be treated as the amount to increment as long as it is a valid number, since for a non-existent tree it would be impossible to distinguish between a final optional argument and an unknown number of subscripts |
| name["subscript"]*=2 | tree_modify("name", "subscript", "*2") | |
| arr[1]+=2*arr[1] | tree_modify("name", "subscript", "+2*x") | To replicate the behavior of all other assignment shortcuts, use this function; note that "x" in the expression represents the current value of the element, which, just like awk, defaults to 0 if there is no valid value currently |
| delete name["subscript"] | tree_remove("name", "subscript") | |
| delete arr[1][2] | tree_remove("arr", 1, 2) | |
| "subscript" in name | tree_elem_exists("name", "subscript") | |
| 2 in arr[1] | tree_elem_exists("arr", 1, 2) | |
| is_array(name["subscript"]) | is_tree("name", "subscript") | |
| is_array(arr[1][2]) | is_tree("arr", 1, 2) | |
| for i in name {...} | while (tree_iters_remaining("name") > 0) {i = tree_next("name"); ...} | |
| for i in arr[1][2] {...} | while (tree_iters_remaining("arr", 1, 2) > 0) {i = tree_next("arr", 1, 2); ...} | both `tree_next` and `tree_iters_remaining` will create an internal iterator within the extension if one under the query (for a tree or subtree) doesn't exist yet. The iterator does not hold all the elements in the tree in memory at once, so the return value of `tree_iters_remaining` does not reflect the total number of elements at the query. Once `tree_iters_remaining = 0`, the given iterator will be freed and therefore reset; this can be done forcibly using `tree_iter_break`. The order of the indices returned by `tree_next` are according to a DFS of the tree at the query |
| for i in name {break} | while (tree_iters_remaining("name") > 0) {i = tree_next("name"); tree_iter_break("name")} | | 
| for i in arr[1][2] {break} | while (tree_iters_remaining("arr", 1, 2) > 0) {i = tree_next("arr", 1, 2); tree_iter_break("arr", 1, 2)} | if you break out of a while loop using the pattern shown above without using this function, the next time you use that iterator, it will resume from where it was previously. To forcefully reset the iterator at the query, or just to free it from memory, use this function. |

### Performance (WIP; see tools/benchmark)
The dimensions in the tables are how many elements per depth, sometimes asymetrical.
If you want to see more about how this data was gathered, take a look at [tools/benchmark](https://github.com/alegottu/gawk-trees/tree/main/tools/benchmark)

### Memory usage
| Dimensions | gawk_trees Peak Memory Usage | gawk Peak Memory Usage | Memory Decrease | 
|------------|------------------------------|------------------------|-----------------|
| 100,000,000 | | | |
| 10,000 x 10,000 | | | |

### Speed
| Dimensions | gawk_trees Runtime | gawk Runtime | Slowdown | 
|------------|--------------------|--------------|----------|
| 100,000,000 | | | |
| 10,000 x 10,000 | | | |
