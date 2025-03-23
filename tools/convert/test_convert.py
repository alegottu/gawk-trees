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
        test = ["x", "list"]
        targets = ('x = tree_next("list"); while (tree_iters_remaining("list") > 0)', 
                   'tree_iter_break("list")')
        results = convert.process_for_in(test)
        self.assertEqual(results, targets)

        test = ["x", 'list[y][2]["abc"]']
        targets = ('x = tree_next("list", y, 2, "abc"); while (tree_iters_remaining("list", y, 2, "abc") > 0)', 
                   'tree_iter_break("list", y, 2, "abc")')
        results = convert.process_for_in(test)
        self.assertEqual(results, targets)

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

    def test_assignment(self):
        test = ["tree[1][x]", "457"]
        target = 'tree_insert("tree", 1, x, 457)'
        result = convert.process_assignment(test)
        self.assertEqual(result, target)

        test = ["decently_long_var", "random[52]"]
        target = 'decently_long_var = query_tree("random", 52)'
        result = convert.process_assignment(test)
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

    def test_statement(self):
    # TODO: test process_statement
        pass

if __name__ == '__main__':
    unittest.main()
