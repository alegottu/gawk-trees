@load "htrees"

BEGIN { create_tree("pred", 2); EVERY=1 }
{
    for(c=3;c<NF;c+=2) {
	elem="pred["$1"]["$c"]";
	tree_insert(elem, $(c+1));
    }
    if(FNR % EVERY==0) { # now check to see everything was assigned correctly.
	print "***", $0, "***"; # the correct line
	printf "line %d, first index is \"%s\"\n", FNR, $1;
	for(c=3;c<NF;c+=2) {
	    elem="pred["$1"]["$c"]";
	    printf "\telem is \"%s\", value should be \"%s\"\n", elem, $(c+1)
	    printf "\tretrieved value is \"%s\"\n", query_tree(elem);
	    if(query_tree(elem) != $(c+1)) print "\tWRONG VALUE";
	}
    }
}
