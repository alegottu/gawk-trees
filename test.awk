@load "htrees"

BEGIN {
	create_tree("pred", 2)
	elem = "pred[a][b]"
	tree_insert(elem, 1)
	print elem
	print query_tree(elem)
	print elem
	print query_tree(elem)
	print elem

	create_tree("test", 2)
	tree_insert("test[x][y]", 864)
	print query_tree("test[x][y]")
	tree_insert("test[x][z]", 229)
	print query_tree("test[y][z]")
	print query_tree("test[x][z]")
	# print query_tree("test[x][y][z]") # produces fatal error
	# print query_tree("test[x]") # produces fatal error
	
	tree_insert("example[sub]", "hello")
	print query_tree("example[sub]")
	# print query_tree("example[sub][new]") # produces fatal error

	print query_tree("another[1]")
	tree_insert("another[1]", 1)
	print query_tree("another[1]")
}
