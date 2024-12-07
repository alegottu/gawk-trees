@load "htrees"

{
        print "testing inserting many values"
        name=$1; len=$2+0;
        create_tree(name,2)

        for(i=0;i<len;i++) for(j=0;j<len;j++) tree_insert(name,i,j,len*i+j)

        for(i=tree_next(name);!tree_iter_done(name);i=tree_next(name)) {
            for(j=tree_next(name,i);!tree_iter_done(name,i);j=tree_next(name,i))
                printf " %s", query_tree(name,i,j);
            print ""
        }
}

