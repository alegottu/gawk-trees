FNR>10{ exit }
{ nodePair=$1;
edge=$2;
for(c=3; c<NF; c+=2) { 
	op[nodePair][edge][$c]=$(c+1);
	print nodePair, edge, $c, $(c+1);
}
}
