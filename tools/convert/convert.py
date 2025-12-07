from collections import deque
import re
from sys import argv

trees = []
breakers = deque()

def process_brackets(token: str, trailing_sep: bool = False) -> str:
    split = token.split('[')
    tree_name = split.pop(0)
    parameters = ""
    
    for subscript in split:
        parameters += f"{subscript.rstrip(']')}, "
    
    if not trailing_sep: parameters = parameters[:-2]
    return f'"{tree_name}", {parameters}'

def process_for_in(statement: str) -> str:
    end = statement.find(')')
    tokens = statement[statement.find('(')+1:end].split(" in ", maxsplit=1)

    var_name = tokens[0]
    container = tokens[1]
    all_params = f'"{container}"'
    
    if '[' in container:
        all_params = process_brackets(container)

    result = f"while (tree_iters_remaining({all_params}) > 0) " + "{ "
    result += f"{var_name} = tree_next({all_params}); "
    breaker = f"tree_iter_break({all_params})"
    breakers.append(breaker)

    return result

def process_in(statement: str) -> str:
    pattern = r"\w+(?:\[.+\])* in \w+(?:\[.+\])*"

    # Could be part of an expression, so we have to repeat the process
    for match in re.finditer(pattern, statement):
        tokens = match.group(0).split(" in ", 1)
        value = process_expression(tokens[0])

        container = tokens[1]
        all_params = f'"{container}", '
        if '[' in container:
            all_params = process_brackets(container, True)

        result = f"tree_elem_exists({all_params}{value})"
        statement = statement.replace(match.group(0), result, 1)

    return statement

# In case of an inline if
def process_if(statement: str) -> tuple[str, str]:
    open_paren_pos = statement.find('(')
    close_paren_pos = statement.find(')')
    next = statement.find('(', open_paren_pos+1)

    # TODO: can utilize something similar for other places that might have multiple sets of parens
    while next != -1 and next < close_paren_pos:
        close_paren_pos = statement.find(')', close_paren_pos+1)
        next = statement.find('(', next+1)

    condition = statement[open_paren_pos+1:close_paren_pos]
    body = statement[close_paren_pos+1:]
    result = f"{statement[:open_paren_pos]}({process_expression(condition)})"
    return (result, body)

def process_equals(statement: str) -> str:
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

def process_query(token: str, has_bracket: bool) -> str:
    BUILTINS = ["ARGV", "ENVIRON", "FUNCTAB", "PROCINFO", "SYMTAB"]

    if has_bracket and not token[:token.find('[')] in BUILTINS:
        return f"query_tree({process_brackets(token)})"
    else:
        return token

def process_assignment(tokens: list) -> str:
    if '[' in tokens[0]:
        value = process_expression(tokens[1])
        subscripts = process_brackets(tokens[0], True)
        return f"tree_insert({subscripts}{value})"
    else:
        return f"{tokens[0]} = {process_expression(tokens[1])}"

def process_increment(tokens: list, decrement: bool = False) -> str:
    if '[' in tokens[0]:
        value = process_expression(tokens[1])
        subscripts = process_brackets(tokens[0], True)
        func = "tree_increment" if not decrement else "tree_decrement"
        return f"{func}({subscripts}{value})"
    else:
        symbol = '+' if not decrement else '-'
        return f"{tokens[0]} {symbol}= {process_expression(tokens[1])}"

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
        return f"{tokens[0]} {op}= {process_expression(tokens[1])}"

def process_delete_element(token: str) -> str:
    all_params = process_brackets(token)
    return f"tree_remove({all_params})"

def process_expression(statement: str) -> str:
    pattern = r"\w+(?:\[[^\s\[]+\])+"

    for match in re.finditer(pattern, statement):
        token = match.group(0)
        query = process_query(token, True)
        statement = statement.replace(token, query, 1)

    return statement

def process_delete_array(token: str) -> str:
    return f'delete_tree("{token}")'

def process_is_array(token: str) -> str:
    container = ""
    if '[' in token:
        container = process_brackets(token)

    return f"is_array({container})"

# Check that the given keyword is present (by itself, i.e. with whitespace surrounding it) followed by a '('
def keyword_present(keyword: str, statement: str) -> bool:
    return re.search(r"(^|\s)\s*" + keyword + r"\s*\(", statement) != None

def parse_statements(line: str) -> tuple[list, list]:
    delim_spacing = { ';': "; ", '#': "# ", '{': " { ", '}': " } ", '\n': '\n' }
    statements, delims = ([], [])
    current_pos = 0

    while current_pos < len(line):
        last_pos = current_pos
        while line[current_pos] not in ";{}#\n":
            current_pos += 1
            if current_pos == len(line):
                current_pos -= 1
                break

        statement = line[last_pos:current_pos].strip()
        statements.append(statement)

        if line[current_pos] == '#':
            delims.append("# ")
            statements.append(line[current_pos+1:].strip())
            break

        delims.append(delim_spacing[line[current_pos]])
        current_pos += 1

    return statements, delims

def process_statements(line: str, verbose: bool, line_num: int) -> str:
    result = ""
    statements, delims = parse_statements(line)

    for i, statement in enumerate(statements):
        delim = delims[i]
        translated = statement

        if len(statement) > 0: 
            if keyword_present("for", statement):
                if " in " in statement:
                    # TODO: same concern with extra set of parens
                    parts = statement.split(')', 1)
                    statement = parts.pop(0) + ')'
                    translated = process_for_in(statement)

                    if len(parts[0].strip()) == 0:
                        delim = '' # Remove extra opening brace
                    else: # Inline for-in
                        statements.insert(i+1, parts[0])
                        if delim == "; ":
                            delim = ''
                            delims.insert(i+1, " } ") # Replace ending ';' with '}'
                        else: # Special case where an inline for-in is followed by a regular loop
                            delims.insert(i+1, " { ")
                            delim = '' # Remove extra opening brace
                            j = delims.index(" } ", i+2)
                            statements.insert(j+1, '')
                            delims.insert(j+1, " } ") # Add missing closing brace
                else:
                    # TODO: could be expression to process here, also need to account for inline?
                    temp, statements[i+2] = [s.strip() for s in statements[i+2].split(')', 1)]
                    # TODO: common func for splitting by matching parens
                    statement += f"; {statements.pop(i+1)}; {temp})"
                    translated = statement
                    delims.pop(i+1)
                    delim = ' '
                    breakers.append(None)
            elif " in " in statement:
                translated = process_in(statement)
            elif keyword_present("while", statement):
                breakers.append(None)
                # TODO: could be inline while loop, maybe make common function for splitting inline body stuff
            elif '[' in statement:
                if keyword_present("if", statement):
                    if delim != " { ":
                        translated, body = process_if(statement)
                        statements.insert(i+1, body)
                        delims.insert(i+1, "; ")
                        delim = ' '
                    else:
                        translated = process_expression(statement)
                # TODO: maybe stricter check for this
                elif "else " in statement:
                    if delim != " { ": # In case of an inline else
                        body = statement[statement.find(' ')+1:]
                        translated = f"else "
                        statements.insert(i+1, body)
                        delims.insert(i+1, "; ")
                        delim = ''
                    else:
                        translated = process_expression(statement)
                elif '=' in statement:
                    translated = process_equals(statement)
                elif "++" in statement:
                    tokens = [statement.strip('+')]
                    tokens.append("1")
                    translated = process_increment(tokens)
                elif "--" in statement:
                    tokens = [statement.strip('-')]
                    tokens.append("1")
                    translated = process_increment(tokens, True)
                elif "delete " in statement:
                    token = statement[7:] # Assuming delete is right at the start of the statement
                    translated = process_delete_element(token)
                else: # "[ ]" used within an expression
                    translated = process_expression(statement)
            elif "delete " in statement:
                token = statement[7:] # Assuming delete is right at the start of the statement
                if token in trees: # TODO: not tracking trees
                    translated = process_delete_array(token)
            elif keyword_present("is_array", statement):
                # TODO: also check if the name given is a valid tree, otherwise do so in the appropiate extension func
                start, end = statement.find('('), statement.find(')')
                token = statement[start+1:end]
                translated = process_is_array(token)
            elif statement == "break" and len(breakers) != 0:
                breaker = breakers.pop()
                if breaker != None:
                    translated = f"{breaker}; break"

        if delim == " } " and len(breakers) > 0:
            breakers.pop()
       
        result += translated + delim

        if verbose and statement != translated: 
            print(f"Token {i+1} of line {line_num}: {statement}\n", line, "---\n", result)

        if delim == "# ":
            result += statements[i+1] + '\n'
            break

    return result

if __name__ == "__main__":
    verbose = False
    result = ""

    for arg in argv[1:]:
        if '-v' in arg:
            verbose = True
        else:
            break
        argv.pop(1)
    
    if len(argv) == 1:
        with open("README.md", 'r') as file:
            print(file.read())
        exit(0)
    elif '{' in argv[1]:
        for i, line in enumerate(argv[1].split('\n')):
            result += process_statements(line+'\n', verbose, i+1)
    else:
        with open(argv[1]) as file:
            for i, line in enumerate(file):
                result += process_statements(line, verbose, i+1)

    print(result)
