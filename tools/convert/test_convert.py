import unittest
import convert

class TestTranslations(unittest.TestCase):
    
    def test_brackets(self):
        test = "list[x][y]"
        target = '"list", x, y'
        result = convert.process_brackets(test)
        self.assertEqual(result, target)

        test = "tree_name[1]"
        target = '"tree_name", 1, '
        result = convert.process_brackets(test, True)
        self.assertEqual(result, target)

    def test_for_in(self):
        test = "for (x in list)"
        targets = ('while (tree_iters_remaining("list") > 0) { x = tree_next("list"); ', 
            'tree_iter_break("list")')
        result = convert.process_for_in(test)
        self.assertEqual(result, targets[0])
        self.assertEqual(convert.breakers[0], targets[1])

        test = 'for (x in list[y][2]["abc"])'
        targets = ('while (tree_iters_remaining("list", y, 2, "abc") > 0) { x = tree_next("list", y, 2, "abc"); ', 
                   'tree_iter_break("list", y, 2, "abc")')
        results = convert.process_for_in(test)
        self.assertEqual(results, targets[0])
        self.assertEqual(convert.breakers[-1], targets[1])

    def test_in(self):
        test = "now[0][a+b] in list"
        target = 'tree_elem_exists("list", query_tree("now", 0, a+b))'
        result = convert.process_in(test)
        self.assertEqual(result, target)

        test = '43 in example["3sys4"][0]'
        target = 'tree_elem_exists("example", "3sys4", 0, 43)'
        result = convert.process_in(test)
        self.assertEqual(result, target)

        test = "if((v in edge) && (u in edge[v]))"
        target = 'if((tree_elem_exists("edge", v)) && (tree_elem_exists("edge", v, u)))'
        result = convert.process_in(test)
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

    def test_delete_array(self):
        test = "delete_this_tree_next781462"
        target = 'delete_tree("delete_this_tree_next781462")'
        result = convert.process_delete_array(test)
        self.assertEqual(result, target)

        test = "simple"
        target = 'delete_tree("simple")'
        result = convert.process_delete_array(test)
        self.assertEqual(result, target)

    def test_is_array(self):
    # TODO: next
        pass

    def test_statements(self):
        # Test for-in edge cases
        test = "for(c=3; c<NF; c+=2) op[nodePair][edge][$c]=$(c+1)\n"
        target = 'for(c=3; c<NF; c+=2) tree_insert("op", nodePair, edge, $c, $(c+1))\n'
        result = convert.process_statements(test, False, 0)
        self.assertEqual(result, target)

        test = 'for(u in edge)for(v in edge[u]){if(u""==v""){++self; print(u,": ",v); continue;} if((v in edge) && (u in edge[v]))++both}\n'
        target = 'while (tree_iters_remaining("edge") > 0) { u = tree_next("edge"); while (tree_iters_remaining("edge", u) > 0) { v = tree_next("edge", u); if(u""==v"") { ++self; print(u,": ",v); continue;  }  } if((tree_elem_exists("edge", v)) && (tree_elem_exists("edge", v, u)))++both } \n'
        result = convert.process_statements(test, False, 0)
        self.maxDiff = None
        self.assertEqual(result, target)

        test = "delete res;for(g in T1)res[g]=1;for(g in T2)res[g]=1; return length(res)\n"
        # TODO: technically res should be a tree but would need to do a lookup for that,
        # also no length function yet
        target = 'delete res; while (tree_iters_remaining("T1") > 0) { g = tree_next("T1"); tree_insert("res", g, 1) } while (tree_iters_remaining("T2") > 0) { g = tree_next("T2"); tree_insert("res", g, 1) } return length(res)\n'
        result = convert.process_statements(test, False, 0)
        self.assertEqual(result, target)

        # Test standard for-in
        test = "for (i in indices) { op[i]++; print(op[i]) }\n"
        target = 'while (tree_iters_remaining("indices") > 0) { i = tree_next("indices"); tree_increment("op", i, 1); print(query_tree("op", i)) } \n'
        result = convert.process_statements(test, False, 0)
        self.assertEqual(result, target)

        # Test edge case with comments
        test = 'for(u in edge) for(v in edge[u]) { print(edge[v]) } # This is a comment; no[x][y] = z; if (true) { print(list[9]) }\n'
        target = 'while (tree_iters_remaining("edge") > 0) { u = tree_next("edge"); while (tree_iters_remaining("edge", u) > 0) { v = tree_next("edge", u); print(query_tree("edge", v)) }  } # This is a comment; no[x][y] = z; if (true) { print(list[9]) }\n'
        result = convert.process_statements(test, False, 0)
        self.assertEqual(result, target)

        # Test inline if, including with multiple sets of parentheses for the condition
        test = "if(u[i]==log(v[i]))res[i]+=36; else res[i]-=log(ABS(u[i]-v[i]));"
        target = 'if(query_tree("u", i)==log(query_tree("v", i))) tree_increment("res", i, 36); else tree_decrement("res", i, log(ABS(query_tree("u", i)-query_tree("v", i)))); '
        result = convert.process_statements(test, False, 0)
        self.assertEqual(result, target)

        # Test regular if-else
        test = "if(u[i]==log(v[i])) { res[i]+=36; } else { res[i]-=log(ABS(u[i]-v[i])) }"
        target = 'if(query_tree("u", i)==log(query_tree("v", i))) { tree_increment("res", i, 36);  } else { tree_decrement("res", i, log(ABS(query_tree("u", i)-query_tree("v", i)))) } '
        result = convert.process_statements(test, False, 0)
        self.assertEqual(result, target)

if __name__ == '__main__':
    unittest.main()
