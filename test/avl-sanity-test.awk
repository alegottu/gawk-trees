@load "vhtrees"

{
	for (i=0;i<$1+0;i++) {
	for (j=0;j<$2+0;j++) {
		tree_insert("op", i, j, i+j);
	}
}
}

{
	test=1
	if ($3!="") { test=$3+0 }
	for (i=0;i<$1+0;i++) {
	for (j=0;j<$2+0;j++) {
		if (j%test==0) { tree_remove("op", i, j); print("remove ", i, j); }
		else {
			x = query_tree("op", i, j);
			if (x != i+j) {
				print "failed! op["i"]["j"]!="i+j
				exit 1
			}
		}
	}
}
}

END { tree_insert("op", 0, 0, 0); tree_insert("op", 1, 1, 1); tree_remove("op", 0, 0); }
