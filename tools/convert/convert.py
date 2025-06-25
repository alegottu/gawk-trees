from collections import deque
import re
from sys import argv

trees = []
breakers = deque()

def process_brackets(token: str, trailing_sep: bool = False) -> str:
    split = token.split('[')
    tree_name = split.pop(0)
    parameters = ""
    
    while len(split) > 0:
        parameters += f"{split.pop(0).rstrip(']')}, "
    
    if not trailing_sep: parameters = parameters[:-2]
    return f'"{tree_name}", {parameters}'

def process_for_in(tokens: list) -> tuple[ str, str ]:
    var_name = tokens[0]
    container = tokens[1]
    all_params = f'"{container}"'
    
    if '[' in container:
        all_params = process_brackets(container)

    result = f"while (tree_iters_remaining({all_params}) > 0)" + '{'
    result += f"{var_name} = tree_next({all_params}); "
    breaker = f"tree_iter_break({all_params})"

    return (result, breaker)

def process_in(statement: str) -> str:
    pattern = r"\w+(?:\[.+\])* in \w+(?:\[.+\])*"
    match = re.search(pattern, statement)
    # Could be part of an expression, so we have to repeat the process

    while match != None:
        tokens = match.group(0).split(" in ", 1)
        value = process_expression(tokens[0])

        container = tokens[1]
        all_params = f'"{container}", '
        if '[' in container:
            all_params = process_brackets(container, True)

        result = f"tree_elem_exists({all_params}{value})"
        statement = statement.replace(match.group(0), result, 1)
        match = re.search(pattern, statement)

    return statement

def process_assignment(tokens: list) -> str:
    if '[' in tokens[0]:
        value = f"query_tree({process_brackets(tokens[1])})" \
            if '[' in tokens[1] else tokens[1]
        subscripts = process_brackets(tokens[0], True)
        return f"tree_insert({subscripts}{value})"
    else:
        all_params = process_brackets(tokens[1])
        return f"{tokens[0]} = query_tree({all_params})"

def process_increment(tokens: list, decrement: bool = False) -> str:
    if '[' in tokens[0]:
        value = f"query_tree({process_brackets(tokens[1])})" \
            if '[' in tokens[1] else tokens[1]
        subscripts = process_brackets(tokens[0], True)
        func = "tree_increment" if not decrement else "tree_decrement"
        return f"{func}({subscripts}{value})"
    else:
        all_params = process_brackets(tokens[1])
        symbol = '+' if not decrement else '-'
        return f"{tokens[0]} {symbol}= query_tree({all_params})"

def process_modify(tokens: list, op: str) -> str:
    if '[' in tokens[0]:
        exp = tokens[1]

        if '[' in exp:
            if exp.find(tokens[0]) != -1:
                exp = exp.replace(tokens[0], 'x')
            else:
                exp = process_expression(exp)

        matches = re.findall(r"\$\d+", exp)
        match_at_end = len(matches) != 0 and \
            exp.find(matches[-1]) == len(exp) - len(matches[-1])

        for match in matches:
            exp = exp.replace(match, f'"{match}"')

        exp = f'"{op}{exp}"'
        if match_at_end:
            exp = exp[:-2]

        subscripts = process_brackets(tokens[0], True)
        return f"tree_modify({subscripts}{exp})"
    else:
        all_params = process_brackets(tokens[1])
        return f"{tokens[0]} {op}= query_tree({all_params})"

def process_delete_element(token: str) -> str:
    all_params = process_brackets(token)
    return f"tree_remove({all_params})"

def process_expression(statement: str) -> str:
    pattern = r"\w+(?:\[[^\s\[]+\])+"
    match = re.search(pattern, statement)

    while match != None:
        token = match.group(0)
        query = f"query_tree({process_brackets(token)})"
        statement = statement.replace(token, query, 1)

        match = re.search(pattern, statement)

    return statement

def process_delete_array(token: str) -> str:
    return f'delete_tree("{token}")'

def process_is_array(token: str) -> str:
    container = ""
    if '[' in token:
        container = process_brackets(token)

    return f"is_array({container})"

def process_statement(statement: str, depth: int = 0) -> str: 
    if "for" in statement and '(' in statement:
        if " in " in statement:
            end = statement.find(')')
            tokens = statement[statement.find('(')+1:end].split(" in ", maxsplit=1)
            result, breaker = process_for_in(tokens)
            breakers.append(breaker)

            if end != len(statement)-1: # accounts for if body of for loop is attached
                return result + process_statement(statement[end+1:], depth+1) + " } "
            elif depth > 0:
                return result + ';'
                # NOTE: ';' Denotes a special meaning here as it is a delimiter for
                # statements and could not otherwise be part of the string;
                # specifically, it means that the body of an inline for-in
                # (from the past recursive call) ends with a non-inline for-in,
                # which would ruin the structure of the braces created by
                # the recursion without special behavior

            return result
        else:
            breakers.append(None)
    elif " in " in statement:
        return process_in(statement)
    elif '[' in statement:
        if '=' in statement:
            tokens = statement.split('=', maxsplit=1)
            tokens[0] = tokens[0].rstrip(' ')
            tokens[1] = tokens[1].lstrip(' ')

            if tokens[0][-1] == '+':
                tokens[0] = tokens[0].rstrip(" +")
                return process_increment(tokens)
            elif tokens[0][-1] == '-':
                tokens[0] = tokens[0].rstrip(" -")
                return process_increment(tokens, True)
            elif re.match("[%/*^]", tokens[0][-1]) != None:
                op = tokens[0][-1]
                tokens[0] = tokens[0][:-1].rstrip(' ')
                return process_modify(tokens, op)
            else:
                return process_assignment(tokens)
        elif "++" in statement:
            tokens = [statement.strip('+')]
            tokens.append("1")
            return process_increment(tokens)
        elif "--" in statement:
            tokens = [statement.strip('-')]
            tokens.append("1")
            return process_increment(tokens, True)
        elif "delete " in statement:
            token = statement[7:]
            return process_delete_element(token)
        else: # "[ ]" used within an expression
            return process_expression(statement)
    elif "delete " in statement:
        token = statement[7:]
        if token in trees:
            return process_delete_array(token)
    elif "is_array" in statement: 
        start, end = statement.find('('), statement.find(')')
        token = statement[start+1:end]
        return process_is_array(token)
    elif statement == "break" and len(breakers) != 0:
        breaker = breakers.pop()
        if breaker != None:
            return f"{breaker}; break"

    return statement

def first_sig_char(s: str) -> int:
    for idx, c in enumerate(s):
        if not c.isspace():
            return idx
    return 0

def process_statements(line: str, verbose: bool, line_num = 1) -> str:
    statements = re.split("[;{}]", line)
    statements = [s for s in statements if len(s) > 0 and not s.isspace()]
    current_pos = 0
    missing_brace = False
    check_for = False # handles special case where a standard for loop uses no braces

    for i, statement in enumerate(statements):
        current_pos = line.find(statement)+len(statement)

        if check_for and ')' in statement:
            if line[current_pos-1] != ')' or line[current_pos-2] != ')':
                correct_split = statement.split(')', 1)
                statement = correct_split[0]
                statements.insert(i+1, correct_split[1])
            check_for = False

        if "while" in statement and '(' in statement:
            breakers.append(None)
        elif "for" in statement and '(' in statement:
            check_for = True
        elif current_pos < len(line) and line[current_pos] == '}':
            if len(breakers) > 0:
                breakers.pop()

            if missing_brace:
                line = line[:current_pos] + " } } " + line[current_pos:]
                missing_brace = False

        translated = process_statement(statement.strip())
        # TODO: only accounts for one inner loop (depth of 1)
        if len(translated) >= 4 and translated[-4] == ';': # See note in process_statement
            translated = translated[:-4]
            missing_brace = True

        before = line
        line = line.replace(statement, translated, 1)
        if verbose and before != line: print(f"Token {i+1} of line {line_num} ({statement}):\n", before, " -> ", line)
    else:
        return line # line is translated by this point

if __name__ == "__main__":
    verbose = len(argv) > 2
    result = ""
    
    if len(argv) == 1:
        print("usage: python convert.py [file.awk] [script only if no file] [-verbose]")
        exit(0)
    elif '{' in argv[1]:
        result = process_statements(argv[1], verbose)
    else:
        with open(argv[1]) as file:
            for i, line in enumerate(file):
                result += process_statements(line, verbose, i+1)

    print(result)
