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

### Performance
In the tables below, memory usage is measured in KB, while speed is measured in seconds.
"x" means repetition of structures, e.g. [100][100]x5 would mean 5 separate structures, each equivalent to
an array of [100][100].
"^" means repetition of a dimension, e.g. 100^3 to mean a tree structure equivalent to an array of
[100][100][100].
If you want to see more about how this data was gathered, take a look at [tools/benchmark](https://github.com/alegottu/gawk-trees/tree/main/tools/benchmark).

| Dimensions | gawk_trees Peak Memory Usage | gawk Peak Memory Usage | Memory Decrease | gawk_trees Runtime | gawk Runtime | Slowdown |
|------------|------------------------------|------------------------|-----------------|--------------------|--------------|----------|
| [100000] | 15872 | 23808 | 33.3333% | 0.17 | 0.05 | 240% |
| [250000] | 33280 | 52736 | 36.8932% | 0.43 | 0.12 | 258.333% |
| [500000] | 62208 | 101888 | 38.9447% | 0.86 | 0.22 | 290.909% |
| [750000] | 91648 | 150784 | 39.219% | 1.31 | 0.35 | 274.286% |
| [900000] | 109056 | 180224 | 39.4886% | 1.57 | 0.41 | 282.927% |
| [10][10][10][1000] | 113664 | 204544 | 44.4305% | 2.16 | 0.67 | 222.388% |
| [1][1000000] | 121088 | 199680 | 39.359% | 1.95 | 0.51 | 282.353% |
| [10][10][1000][10] | 127744 | 308480 | 58.5892% | 2.37 | 0.80 | 196.25% |
| [10][1000][10][10] | 129024 | 318976 | 59.5506% | 2.35 | 0.80 | 193.75% |
| [1000][10][10][10] | 129280 | 320000 | 59.6% | 2.36 | 0.80 | 195% |
| [100000][10]x2 | 251648 | 612352 | 58.9047% | 4.28 | 1.41 | 203.546% |
| [1000000][1] | 261632 | 1356544 | 80.7133% | 3.33 | 1.63 | 104.294% |
| [100000][10]x3 | 375296 | 916480 | 59.0503% | 6.18 | 2.15 | 187.442% |
| [100000][10]x5 | 622336 | 1524992 | 59.1909% | 10.58 | 3.58 | 195.531% |
| [25][25][25][5000] | 8551424 | 15533056 | 44.9469% | 188.08 | 53.79 | 249.656% |
| [25][25][5000][25] | 8988672 | 18313984 | 50.9191% | 194.55 | 55.57 | 250.099% |
| [25][5000][25][25] | 9006080 | 18425088 | 51.1206% | 196.82 | 55.81 | 252.661% |
| [5000][25][25][25] | 9007104 | 18429696 | 51.1272% | 191.27 | 56.62 | 237.813% |
| [10]^8 | 12854016 | 31621888 | 59.3509% | 260.53 | 82.56 | 215.564% |
| [15]^7 | 20984832 | 46183680 | 54.5622% | 579.91 | 145.95 | 297.335% |
| [500000000] | 57831424 | 98294272 | 41.165% | 912.08 | 235.80 | 286.802% |
| [10]^7x50 | 64255232 | 137075968 | 53.1244% | 1719.67 | 460.20 | 273.679% |
| [10]^8x5 | 64255744 | 137173248 | 53.1572% | 1777.91 | 328.92 | 440.53% |
| [10]^6x500 | 64944896 | 137070080 | 52.6192% | 1620.93 | 435.31 | 272.362% |
| [750000000] | 86820608 | 137100288 | 36.6737% | 1400.41 | 384.56 | 264.159% |
| [900000000] | 104148224 | 137223680 | 24.1033% | 1705.18 | 460.42 | 270.353% |
| [1500000000] | 137172736 | 137636096 | 0.336656% | 2883.84 | 876.75 | 228.924% |
| [1750000000] | 137195520 | 139059712 | 1.34057% | 3679.77 | 1021.00 | 260.408% |
| [20]^7 | 137251072 | 137478656 | 0.165541% | 4007.83 | 1156.16 | 246.65% |
| [1250000000] | 137271040 | 137636096 | 0.265233% | 2412.20 | 730.05 | 230.416% |

