@load "htrees"

BEGIN {
    for (a=0; a<10000000000; a++)
    {
		tree_increment("test", a)
    }
}
# BEGIN {
#     while(tree_iters_remaining("test") > 0)
#     {
#         a=tree_next("test")
#         while (tree_iters_remaining("test", a) > 0)
#         {
#             b=tree_next("test", a)
#             print query_tree("test", a, b)
#         }
#     }
# }
