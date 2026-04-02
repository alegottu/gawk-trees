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
| length(name) | tree_length("name") | returns the length of the string argument if no tree for the given name is found |
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
| isarray(name["subscript"]) | is_tree("name", "subscript") | |
| typeof(arr[1][2]) == "array" | is_tree("arr", 1, 2) | |
| for i in name {...} | while (tree_iters_remaining("name") > 0) {i = tree_next("name"); ...} | |
| for i in arr[1][2] {...} | while (tree_iters_remaining("arr", 1, 2) > 0) {i = tree_next("arr", 1, 2); ...} | both `tree_next` and `tree_iters_remaining` will create an internal iterator within the extension if one under the query (for a tree or subtree) doesn't exist yet. The iterator does not hold all the elements in the tree in memory at once, so the return value of `tree_iters_remaining` does not reflect the total number of elements at the query. Once `tree_iters_remaining = 0`, the given iterator will be freed and therefore reset; this can be done forcibly using `tree_iter_break`. The order of the indices returned by `tree_next` are according to a DFS of the tree at the query |
| for i in name {break} | while (tree_iters_remaining("name") > 0) {i = tree_next("name"); tree_iter_break(); break} | | 
| for i in arr[1][2] {break} | while (tree_iters_remaining("arr", 1, 2) > 0) {i = tree_next("arr", 1, 2); tree_iter_break(); break} | if you break out of a while loop using the pattern shown above without using this function, the next time you use that iterator, it will resume from where it was previously. To forcefully reset the iterator at the query, or just to free it from memory, use this function. |

### Performance
In the tables below, memory usage is measured in KB, while speed is measured in seconds.
"x" means repetition of structures, e.g. [100][100]x5 would mean 5 separate structures, each equivalent to
an array of [100][100].
"^" means repetition of a dimension, e.g. [100]^3 to mean a structure equivalent to an array of
[100][100][100].
If you want to see more about how this data was gathered, take a look at [tools/benchmark](https://github.com/alegottu/gawk-trees/tree/main/tools/benchmark).

| Dimensions | gawk_trees Peak Memory Usage | gawk Peak Memory Usage | Memory Decrease | gawk_trees Runtime | gawk Runtime | Slowdown |
|------------|------------------------------|------------------------|-----------------|--------------------|--------------|----------|
| [100000] | 15360 | 23040 | 33.3333% | 0.09 | 0.03 | 200% |
| [250000] | 33024 | 52480 | 37.0732% | 0.27 | 0.07 | 285.714% |
| [500000] | 61696 | 101632 | 39.2947% | 0.48 | 0.13 | 269.231% |
| [750000] | 91392 | 150272 | 39.1823% | 0.71 | 0.23 | 208.696% |
| [900000] | 108544 | 179712 | 39.6011% | 1.01 | 0.28 | 260.714% |
| [10][10][10][1000] | 113152 | 204288 | 44.6115% | 1.27 | 0.31 | 309.677% |
| [1][1000000] | 120576 | 198912 | 39.3822% | 1.27 | 0.33 | 284.848% |
| [10][10][1000][10] | 127488 | 307968 | 58.6035% | 1.30 | 0.34 | 282.353% |
| [1000][10][10][10] | 128512 | 319744 | 59.8078% | 1.50 | 0.40 | 275% |
| [10][1000][10][10] | 128512 | 318464 | 59.6463% | 1.28 | 0.35 | 265.714% |
| [100000][10]x2 | 251392 | 611840 | 58.9121% | 2.13 | 0.63 | 238.095% |
| [1000000][1] | 261120 | 1356032 | 80.7438% | 2.01 | 0.61 | 229.508% |
| [100000][10]x3 | 375040 | 916224 | 59.0668% | 3.16 | 0.93 | 239.785% |
| [100000][10]x5 | 621824 | 1524480 | 59.2107% | 5.31 | 1.52 | 249.342% |
| [25][25][25][5000] | 8551168 | 15533056 | 44.9486% | 103.67 | 24.12 | 329.809% |
| [25][25][5000][25] | 8987648 | 18313472 | 50.9233% | 101.42 | 25.17 | 302.94% |
| [25][5000][25][25] | 9005568 | 18425088 | 51.1233% | 101.31 | 25.31 | 300.277% |
| [5000][25][25][25] | 9006336 | 18429440 | 51.1307% | 113.78 | 25.04 | 354.393% |
| [10]^8 | 12854016 | 31621632 | 59.3506% | 198.29 | 43.08 | 360.283% |
| [15]^7 | 20983808 | 46183168 | 54.5639% | 310.62 | 68.32 | 354.655% |
| [500000000] | 57830912 | 98294016 | 41.1654% | 521.78 | 131.26 | 297.516% |
| [10]^8x5 | 64254720 | 158093824 | 59.3566% | 1056.43 | 216.04 | 388.997% |
| [10]^7x50 | 64254976 | 158094336 | 59.3566% | 1006.27 | 204.46 | 392.16% |
| [10]^6x500 | 64944128 | 158097664 | 58.9215% | 887.05 | 193.17 | 359.207% |
| [750000000] | 86821376 | 147310592 | 41.0624% | 783.22 | 204.91 | 282.226% |
| [900000000] | 104146688 | 176709632 | 41.0634% | 1047.57 | 262.32 | 299.348% |

