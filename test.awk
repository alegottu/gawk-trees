@load "htrees"

BEGIN {
	create_tree("test", 1)
	tree_insert("test[x][y][z]", 72)
	print query_tree("test[x][y][z]")
}
