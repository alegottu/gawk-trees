BEGIN { for (i=0;i<2;i++) {
	nodePair="855736:"i;
	edge=i;
	for (j=0;j<200000;j++) { 
		x="8:10030:"j;
		y=rand();
		tree_insert("op", nodePair, edge, x, y);
		print nodePair, edge, x, y
	}
}
}
