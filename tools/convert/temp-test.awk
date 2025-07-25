@load "vhtrees"
BEGIN { for (i=0;i<1;i++) {
	for (j=0;j<10000000;j++) { 
		tree_insert("op", i, j, i + j);
	}
}
}
BEGIN { for (i=0;i<1;i++) {
	for (j=0;j<10000000;j++) { 
		if (j%1==0) { tree_remove("op", i, j); print("remove ", i, j); }
		else {
			x = query_tree("op", i, j);
			if (x == i + j) print "yes";
		}
	}
}
}
BEGIN { tree_insert("op", 0, 0, 0); tree_insert("op", 1, 1, 1); tree_remove("op", 0, 0); }
