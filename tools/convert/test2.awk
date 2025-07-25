# TODO: not up to date with ext-test2.awk
BEGIN { for (i=0;i<10;i++) {
	nodePair=rand();
	edge=rand();
	for (j=0;j<100000;j++) { 
		x = rand(); y = rand();
		op[nodePair][edge][x] = y;
	}
}
}
