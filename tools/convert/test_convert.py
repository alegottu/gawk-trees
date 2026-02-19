import unittest
import convert

class TestTranslations(unittest.TestCase):
    
    def setUp(self) -> None:
        convert.universal = True
        convert.trees = set()
        convert.current_vars = dict()
        self.maxDiff = None

    def test_brackets(self):
        test = "list[x][y]"
        target = '"list", x, y'
        result = convert.process_brackets(test)
        self.assertEqual(result, target)

        test = "tree_name[1]"
        target = '"tree_name", 1, '
        result = convert.process_brackets(test, True)
        self.assertEqual(result, target)

        test = "sortTb[sortTc[i]]"
        target = '"sortTb", query_tree("sortTc", i)'
        result = convert.process_brackets(test)
        self.assertEqual(result, target)

        test = "sortTb[sortTc[i][j]]"
        target = '"sortTb", query_tree("sortTc", i, j)'
        result = convert.process_brackets(test)
        self.assertEqual(result, target)

    def test_for_in(self):
        test = "for (x in list)"
        target = 'while (tree_iters_remaining("list") > 0) { x = tree_next("list"); '
        result = convert.process_for_in(test)
        self.assertEqual(result, target)

        test = 'for (x in list[y][2]["abc"])'
        target = 'while (tree_iters_remaining("list", y, 2, "abc") > 0) { x = tree_next("list", y, 2, "abc"); '
        results = convert.process_for_in(test)
        self.assertEqual(results, target)

    # NOTE: process_in is a smaller part of process_expression
    def test_in(self):
        test = "now[0][a+b] in list"
        target = 'tree_elem_exists("list", query_tree("now", 0, a+b))'
        result = convert.process_expression(test)
        self.assertEqual(result, target)

        test = '43 in example["3sys4"][0]'
        target = 'tree_elem_exists("example", "3sys4", 0, 43)'
        result = convert.process_expression(test)
        self.assertEqual(result, target)

        test = "if((v in edge) && (u in edge[v]))"
        target = 'if((tree_elem_exists("edge", v)) && (tree_elem_exists("edge", v, u)))'
        result = convert.process_expression(test)
        self.assertEqual(result, target)

    def test_query(self):
        test = "ARGV[0]"
        result = convert.process_query(test, True)
        self.assertEqual(result, test)

        test = "random[32][variable]"
        target = 'query_tree("random", 32, variable)'
        result = convert.process_query(test, True)
        self.assertEqual(result, target)

    def test_assignment(self):
        test = ["tree[1][x]", "457"]
        target = 'tree_insert("tree", 1, x, 457)'
        result = convert.process_assignment(test)
        self.assertEqual(result, target)

        test = ["decently_long_var", "random[52]"]
        target = 'decently_long_var = query_tree("random", 52)'
        result = convert.process_assignment(test)
        self.assertEqual(result, target)

        test = ["seed", 'systime()+PROCINFO["gid"]+PROCINFO["uid"]+PROCINFO["pgrpid"]+PROCINFO["ppid"]+PROCINFO["pid"]']
        target = 'seed = systime()+PROCINFO["gid"]+PROCINFO["uid"]+PROCINFO["pgrpid"]+PROCINFO["ppid"]+PROCINFO["pid"]'
        result = convert.process_assignment(test)
        self.assertEqual(result, target)


    def test_increment(self):
        test = ["tree[40][$1]", "25.2"]
        target = 'tree_increment("tree", 40, $1, 25.2)'
        result = convert.process_increment(test)
        self.assertEqual(result, target)

        test = ["hx[$1][$2][$3]", "hy[$4]"]
        target = 'tree_increment("hx", $1, $2, $3, query_tree("hy", $4))'
        result = convert.process_increment(test)
        self.assertEqual(result, target)

    def test_modify(self):
        test = ["cual[$1][$2][$3]", "$4*$5"]
        target = 'tree_modify("cual", $1, $2, $3, "*"$4"*"$5)'
        result = convert.process_modify(test, '*')
        self.assertEqual(result, target)

        test = ["gato[0][$3]", "gato[0][$3]+3/7"]
        target = 'tree_modify("gato", 0, $3, "*x+3/7")'
        result = convert.process_modify(test, '*')
        self.assertEqual(result, target)

    def test_delete_element(self):
        test = 'to_delete[78]["here a string"]'
        target = 'tree_remove("to_delete", 78, "here a string")'
        result = convert.process_delete_element(test)
        self.assertEqual(result, target)

        test = 'name[1]'
        target = 'tree_remove("name", 1)'
        result = convert.process_delete_element(test)
        self.assertEqual(result, target)

    def test_expression(self):
        test = "print x * big_tree[11][37][444]"
        target = 'print x * query_tree("big_tree", 11, 37, 444)'
        result = convert.process_expression(test)
        self.assertEqual(result, target)

        test = "random_func(small[1] * z + another[x++])"
        target = 'random_func(query_tree("small", 1) * z + query_tree("another", x++))'
        result = convert.process_expression(test)
        self.assertEqual(result, target)

        test = "unit[12][54]"
        target = 'query_tree("unit", 12, 54)'
        result = convert.process_expression(test)
        self.assertEqual(result, target)

        test = "sortTb[sortTc[i]]"
        target = 'query_tree("sortTb", query_tree("sortTc", i))'
        result = convert.process_expression(test)
        self.assertEqual(result, target)

        test = "n[inner[another[2][1]]]"
        target = 'query_tree("n", query_tree("inner", query_tree("another", 2, 1)))'
        result = convert.process_expression(test)
        self.assertEqual(result, target)

    def test_delete_array(self):
        test = "delete delete_this_tree_next781462"
        target = 'delete_tree("delete_this_tree_next781462")'
        result = convert.process_delete_array(test)
        self.assertEqual(result, target)

        test = "delete simple"
        target = 'delete_tree("simple")'
        result = convert.process_delete_array(test)
        self.assertEqual(result, target)

    def test_is_array(self):
        test = "isarray(a[0][y])"
        target = 'is_tree("a", 0, y)'
        result = convert.process_is_array(test)
        self.assertEqual(result, target)

        test = 'typeof(b[7+1/2%0][x])=="array"'
        target = 'is_tree("b", 7+1/2%0, x)'
        result = convert.process_is_array(test)
        self.assertEqual(result, target)

### All the tests below use process_statements ###

    def test_inline_for(self):
        test = "for(c=3; c<NF; c+=2) op[nodePair][edge][$c]=$(c+1)\n"
        target = 'for(c=3; c<NF; c+=2) tree_insert("op", nodePair, edge, $c, $(c+1))\n'
        result = convert.process_statements(test)
        self.assertEqual(result, target)

    def test_for_in_in_inline_for_in(self):
        test = 'for(u in edge)for(v in edge[u]){if(u""==v""){++self; print(u,": ",v); continue;} if((v in edge) && (u in edge[v]))++both}\n'
        target = 'while (tree_iters_remaining("edge") > 0) { u = tree_next("edge"); while (tree_iters_remaining("edge", u) > 0) { v = tree_next("edge", u); if(u""==v"") { ++self; print(u,": ",v); continue;  }  } if((tree_elem_exists("edge", v)) && (tree_elem_exists("edge", v, u))) ++both } \n'
        result = convert.process_statements(test)
        self.assertEqual(result, target)

    def test_double_inline_for_in_and_length(self):
        test = "delete res;for(g in T1)res[g]=1;for(g in T2)res[g]=1; return length(res)\n"
        target = 'delete_tree("res"); while (tree_iters_remaining("T1") > 0) { g = tree_next("T1"); tree_insert("res", g, 1);  } while (tree_iters_remaining("T2") > 0) { g = tree_next("T2"); tree_insert("res", g, 1);  } return tree_length("res")\n'
        result = convert.process_statements(test)
        self.assertEqual(result, target)

    def test_for_in_with_statement(self):
        test = "for (i in indices) { op[i]++; print(op[i]) }\n"
        target = 'while (tree_iters_remaining("indices") > 0) { i = tree_next("indices"); tree_increment("op", i, 1); print(query_tree("op", i)) } \n'
        result = convert.process_statements(test)
        self.assertEqual(result, target)

    def test_comments(self):
        test = 'for(u in edge) for(v in edge[u]) { print(edge[v]) } # This is a comment; no[x][y] = z; if (true) { print(list[9]) }\n'
        target = 'while (tree_iters_remaining("edge") > 0) { u = tree_next("edge"); while (tree_iters_remaining("edge", u) > 0) { v = tree_next("edge", u); print(query_tree("edge", v)) }  } # This is a comment; no[x][y] = z; if (true) { print(list[9]) }\n'
        result = convert.process_statements(test)
        self.assertEqual(result, target)

    def test_inline_if_with_many_parentheses(self):
        test = "if(u[i]==log(v[i]))res[i]+=36; else res[i]-=log(ABS(u[i]-v[i]));"
        target = 'if(query_tree("u", i)==log(query_tree("v", i))) tree_increment("res", i, 36); else tree_decrement("res", i, log(ABS(query_tree("u", i)-query_tree("v", i)))); '
        result = convert.process_statements(test)
        self.assertEqual(result, target)

    def test_if_else(self):
        test = "if(u[i]==log(v[i])) { res[i]+=36; } else { res[i]-=log(ABS(u[i]-v[i])) }"
        target = 'if(query_tree("u", i)==log(query_tree("v", i))) { tree_increment("res", i, 36);  } else { tree_decrement("res", i, log(ABS(query_tree("u", i)-query_tree("v", i)))) } '
        result = convert.process_statements(test)
        self.assertEqual(result, target)
        
    def test_triple_nested_inline_for_in(self):
        test = "for (a in b) for (x in y) for (i in j) print a+x+i;"
        target = 'while (tree_iters_remaining("b") > 0) { a = tree_next("b"); while (tree_iters_remaining("y") > 0) { x = tree_next("y"); while (tree_iters_remaining("j") > 0) { i = tree_next("j"); print a+x+i;  }  }  } '
        result = convert.process_statements(test)
        self.assertEqual(result, target)

    def test_function_and_inline_if_else_in_inline_for_in(self):
        test = "function GeoMeanDist(u,v,    i,res) { for(i in u) if(u[i]==v[i])res+=log(1e-16); else res+=log(ABS(u[i]-v[i])); return exp(res/length(u)); }"
        target = 'function GeoMeanDist(u,v,    i,res) { while (tree_iters_remaining(u) > 0) { i = tree_next(u); if(query_tree(u, i)==query_tree(v, i)) res+=log(1e-16); else res += log(ABS(query_tree(u, i)-query_tree(v, i)));  } return exp(res/tree_length(u));  } '
        convert.universal = False
        result = convert.process_statements(test)
        self.assertEqual(result, target)

    def test_assignment_with_nested_brackets_in_inline_for_in(self):
        test = "for(i=1;i<=NsortTc;i++)ai[i]=sortTb[sortTc[i]];"
        target = 'for(i=1; i<=NsortTc; i++) tree_insert("ai", i, query_tree("sortTb", query_tree("sortTc", i))); '
        result = convert.process_statements(test)
        self.assertEqual(result, target)

    # Also tests retaining indentation, and () surrounding a statement
    def test_return_of_array_assignment(self):
        test = "    return (_memAccLog1[x]=sum);"
        target = '    return (tree_insert("_memAccLog1", x, sum)); '
        result = convert.process_statements(test)
        self.assertEqual(result, target)

    def test_quoting(self):
        test = '    ASSERT(M>=m,"BUG: M is not greater than m in LogSumLogs"); \n'
        result = convert.process_statements(test)
        self.assertEqual(result, test)

    def test_return_from_inline_if_else(self):
        test = "function floor(x) {if(x>=0) return int(x); else return int(x)-1}"
        target = "function floor(x) { if(x>=0) return int(x); else return int(x)-1 } "
        result = convert.process_statements(test)
        self.assertEqual(result, target)

    def test_multiple_ins_in_expression(self):
        test = "if(n in _memLogChoose && k in _memLogChoose[n]) return _memLogChoose[n][k];"
        target = 'if(tree_elem_exists("_memLogChoose", n) && tree_elem_exists("_memLogChoose", n, k)) return query_tree("_memLogChoose", n, k); '
        result = convert.process_statements(test)
        self.assertEqual(result, target)

    # Also tests correctly closing scope from combo of inline / regular if
    def test_length_with_string_vars(self):
        test = "{delete res;if(length(T1)<length(T2)){for(g in T1)if(g in T2)res[g]=1}"
        target = ' { delete_tree("res"); if(tree_length("T1")<tree_length("T2")) { while (tree_iters_remaining("T1") > 0) { g = tree_next("T1"); if(tree_elem_exists("T2", g)) tree_insert("res", g, 1) }  } '
        result = convert.process_statements(test)
        self.assertEqual(result, target)

    def test_multiple_assignment(self):
        test = "_statN[name] = _statSum[name] = _statSum2[name] = 0;"
        target = 'tree_insert("_statN", name, 0); tree_insert("_statSum", name, 0); tree_insert("_statSum2", name, 0); '
        result = convert.process_statements(test)
        self.assertEqual(result, target)

    def test_string_escaping(self):
        test = 'ASSERT(x==0|| (x in _statHistCDF[name]), "oops, x "x" is not in _statHistCDF["name"]");'
        target = 'ASSERT(x==0|| (tree_elem_exists("_statHistCDF", name, x)), "oops, x "x" is not in _statHistCDF["name"]"); '
        result = convert.process_statements(test)
        self.assertEqual(result, target)

    def test_standard_for_no_translate(self):
        test = "for(i=1;i<=k;i++){term*=l/i;sum+=term}"
        target = "for(i=1; i<=k; i++) { term*=l/i; sum+=term } "
        result = convert.process_statements(test)
        self.assertEqual(result, target)

    def test_assign_result_of_increment(self):
        test = "_SpN=(_Spearman_N[name]++);"
        target = '_SpN = (tree_increment("_Spearman_N", name, 1)); '
        result = convert.process_statements(test)
        self.assertEqual(result, target)

    def test_multiple_expressions_in_parentheses_in_one_expression(self):
        test = "function IsEdge(edge,u,v) {return (u in edge)&&(v in edge[u])&&edge[u][v];}"
        target = 'function IsEdge(edge,u,v) { return (tree_elem_exists(edge, u))&&(tree_elem_exists(edge, u, v))&&query_tree(edge, u, v);  } '
        convert.universal = False
        result = convert.process_statements(test)
        self.assertEqual(result, target)

    def test_inline_for_in_inline_if(self):
        test = "if(k>=n/2) for(i=n;i>=k;i--) sum+=BinomialPMF(p,n,i);"
        target = "if(k>=n/2) for(i=n; i>=k; i--) sum+=BinomialPMF(p,n,i); "
        result = convert.process_statements(test)
        self.assertEqual(result, target)

    def test_assertion_of_tree_element_with_quoting(self):
        test = 'for(u in T) { ASSERT(D[u]%2==0, "InducedEdges: D["u"]="D[u]); D[u]/=2; }'
        target = 'while (tree_iters_remaining(T) > 0) { u = tree_next(T); ASSERT(query_tree(D, u)%2==0, "InducedEdges: D["u"]="query_tree(D, u)); tree_modify(D, u, "/2");  } '
        convert.universal = False
        convert.current_vars.update([ ("T", 1), ("D", 1) ])
        result = convert.process_statements(test)
        self.assertEqual(result, target)

    def test_prefix_decrement(self):
        test = "if(--_PQ_[name][p][element]==0) {"
        target = 'if(tree_decrement("_PQ_", name, p, element, 1)==0) { '
        result = convert.process_statements(test)
        self.assertEqual(result, target)

if __name__ == '__main__':
    unittest.main()
