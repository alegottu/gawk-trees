import re
from collections import deque
from itertools import pairwise
from typing import Callable
from sys import argv

current_vars = dict()
trees = set()
breakers = deque()

# pos = index of matching symbol to be found in s, returns index of match or -1 upon failure
def find_matching(s: str, pos: int) -> int:
    if pos >= len(s)-1:
        return -1

    same = s[pos]
    MATCHES = { '[': ']', '{': '}', '(': ')' }

    if same not in MATCHES.keys():
        return -1

    find = MATCHES[same]
    remaining = 1

    for i, c in enumerate(s[pos+1:]):
        # Counts up if another pair starts, counts down if a pair ends
        remaining += int(c == same) - int(c == find)

        if remaining == 0:
            return pos+i+1

    return -1

# Continues if a new pair is opened directly after one closing
def find_brackets(s: str, pos: int) -> int:
    remaining = 1

    for i, pair in enumerate(pairwise(s[pos+1:])):
        c = pair[0]
        remaining += int(c == '[') - int(c == ']')

        if remaining == 0 and pair[1] != '[':
            return pos+i+1

    return len(s)-1

def process_function(statement: str) -> None:
    first_pos = statement.find('(') + 1
    close = statement.find(')', first_pos+1)
    vars = [ (v.lstrip(), 1) for v in statement[first_pos:close].split(',') ]
    # 1 = number of '}'s to be seen before that var falls out of scope, updated dynamically
    current_vars.update(vars)

def process_brackets(token: str, trailing_sep: bool = False, separate: bool = False) -> str | tuple[str, str]:
    start = token.find('[')
    end = find_matching(token, start)
    tree_name = token[:start]
    parameters = ""

    while end != -1:
        parameter = token[start+1:end]

        if '[' in parameter:
            parameter = process_query(parameter, True)

        parameters += f"{parameter}, "
        start = end+1
        end = find_matching(token, start)
    
    if not trailing_sep: parameters = parameters[:-2]
    return f"{get_tree_name(tree_name)}, {parameters}" if not separate else (tree_name, parameters)

def process_for_in(statement: str) -> str:
    end = statement.find(')')
    tokens = statement[statement.find('(')+1:end].split(" in ", maxsplit=1)

    var_name = tokens[0]
    current_vars[var_name] = 1
    container = tokens[1]
    all_params = get_tree_name(container)
    
    if '[' in container:
        all_params = process_brackets(container)

    result = f"while (tree_iters_remaining({all_params}) > 0) " + "{ "
    result += f"{var_name} = tree_next({all_params}); "
    breakers.append("tree_iter_break()")

    return result

# In case of an inline if
def process_if(statement: str) -> tuple[str, str]:
    open_paren_pos = statement.find('(')
    close_paren_pos = find_matching(statement, open_paren_pos)

    condition = statement[open_paren_pos+1:close_paren_pos]
    body = statement[close_paren_pos+1:].lstrip()
    result = f"{statement[:open_paren_pos]}({process_expression(condition)})"

    return (result, body)

def valid_tree(name: str) -> bool:
    return universal or name in trees or name in current_vars

# <name> could be a variable; if so, don't put quotes around it
def get_tree_name(name: str) -> str:
    if universal or name in trees or name not in current_vars:
        return f'"{name}"'
    else:
        return name

def process_in(statement: str) -> str:
    pattern = r'\w+(:?\[(?:\S*|".*")\])* in \w+\[?'
    # Makes sure to account for expressions or strings inside brackets
    result = statement

    # Could be part of an expression, so we have to repeat the process
    for match in re.finditer(pattern, statement):
        end = match.end()-1
        if statement[end] == '[': end = find_brackets(statement, end)
        full = statement[match.start():end+1]
        if not valid_token(full, statement): continue
        tokens = full.split(" in ", 1)
        value = tokens[0]
        tree_name = tokens[1]
        all_params = ""

        if '[' in tree_name:
            tree_name, subscripts = process_brackets(tree_name, True, True)
            all_params = f"{get_tree_name(tree_name)}, {subscripts}{value}"
        else: # NOTE: we can't guarantee if a variable is a string or the name of a tree here
            if not valid_tree(tree_name): continue
            all_params = f"{get_tree_name(tree_name)}, {value}"

        translated = f"tree_elem_exists({all_params})"
        result = result.replace(full, translated, 1)

    return result

def process_length(statement: str) -> str:
    tree_name = statement[statement.find('(')+1:-1]

    if valid_tree(tree_name):
        return f"tree_length({get_tree_name(tree_name)})"
    else: return statement

# Includes processing typeof() == "array"
def process_is_array(statement: str) -> str:
    start = statement.find('(')
    end = statement.find(')', start+1)
    tree_name = statement[start+1:end]
    subscripts = ""

    if '[' in tree_name:
        tree_name, subscripts = process_brackets(tree_name, False, True)
        subscripts = ", " + subscripts

    if universal or tree_name in trees:
        return f'is_tree("{tree_name}"{subscripts})'
    else: return statement

def process_equals(statement: str) -> str:
    add_parens = False
    
    # NOTE: could improve this in cases like ( ( x + y ) ), but only affects spacing; same with process_exp
    if statement[0] == '(':
        statement = statement[1:-1]
        add_parens = True

    tokens = statement.split('=', maxsplit=1)
    tokens[0] = tokens[0].rstrip(' ')
    tokens[1] = tokens[1].lstrip(' ')
    result = ""

    if tokens[0][-1] == '+':
        tokens[0] = tokens[0].rstrip(" +")
        result = process_increment(tokens)
    elif tokens[0][-1] == '-':
        tokens[0] = tokens[0].rstrip(" -")
        result = process_increment(tokens, True)
    elif tokens[1][0] == '=':
        result = process_expression(statement)
    elif re.match("[%/*^]", tokens[0][-1]) != None:
        op = tokens[0][-1]
        tokens[0] = tokens[0][:-1].rstrip(' ')
        result = process_modify(tokens, op)
    else:
        result = process_assignment(tokens)

    return f"({result})" if add_parens else result

def log_tree(tree_name: str) -> None:
    if tree_name not in current_vars:
        trees.add(tree_name)

def process_query(token: str, has_bracket: bool) -> str:
    BUILTINS = ["ARGV", "ENVIRON", "FUNCTAB", "PROCINFO", "SYMTAB"]

    if has_bracket and not token[:token.find('[')] in BUILTINS:
        tree_name, subsripts = process_brackets(token, False, True)
        log_tree(tree_name)
        return f"query_tree({get_tree_name(tree_name)}, {subsripts})"
    else:
        return token

ASSIGNMENT = r"[^!<>=%/*^+-]=[^=]"

def process_assignment(tokens: list) -> str:
    if re.search(ASSIGNMENT, tokens[1]) != None:
        result = ""
        tokens.extend([ t.strip() for t in tokens.pop().split('=') ])

        for t in tokens[:-1]:
            result += process_assignment([t, tokens[-1]]) + "; "
        else:
            return result[:-2]

    if '[' in tokens[0]:
        value = process_expression(tokens[1])
        tree_name, subscripts = process_brackets(tokens[0], True, True)
        log_tree(tree_name)
        return f"tree_insert({get_tree_name(tree_name)}, {subscripts}{value})"
    else:
        return f"{tokens[0]} = {process_expression(tokens[1])}"

def process_increment(tokens: list, decrement: bool = False) -> str:
    if '[' in tokens[0]:
        value = process_expression(tokens[1])
        tree_name, subscripts = process_brackets(tokens[0], True, True)
        log_tree(tree_name)
        func = "tree_increment" if not decrement else "tree_decrement"
        return f"{func}({get_tree_name(tree_name)}, {subscripts}{value})"
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

        tree_name, subscripts = process_brackets(tokens[0], True, True)
        log_tree(tree_name)
        return f"tree_modify({get_tree_name(tree_name)}, {subscripts}{exp})"
    else:
        return f"{tokens[0]} {op}= {process_expression(tokens[1])}"

def process_delete_element(token: str) -> str:
    all_params = process_brackets(token)
    return f"tree_remove({all_params})"

def process_pattern(pattern: str, statement: str, processor: Callable[[str], str]) -> str:
    result = statement

    for match in re.finditer(pattern, statement):
        start = match.start()
        if statement[start] not in ['l', 'i']: start += 1
        end = find_matching(statement, statement.find('(', start+6)) # 6 = len("length")
        part = statement[start:end+1]

        value = processor(part)
        result = result.replace(part, value)

    return result

IS_ARRAY = r'(?:is_array|typeof\(.*\)\s*==\s*"array")'

def process_expression(statement: str) -> str:
    result = process_in(statement)

    pattern = gen_keyword_pattern("length")
    result = process_pattern(pattern, result, process_length)

    pattern = gen_keyword_pattern(IS_ARRAY)
    result = process_pattern(pattern, result, process_is_array)

    statement = result
    pattern = r"\w+\["
    end = 0

    for match in re.finditer(pattern, statement):
        start = match.start()
        if start < end: continue
        end = find_brackets(statement, statement.find('[', start+1))
        token = statement[start:end+1]
        if not valid_token(token, statement): continue
        last = len(statement) if statement[0] != '(' else -1

        # end+1:end+4 makes sure any assignment is directly after '['
        if search := re.search(ASSIGNMENT, statement[end+1:end+4]):
            value = statement[search.start()+1:]
            translated = process_assignment([token, value])
            return result.replace(statement[start:last], translated, 1)
        elif search := re.search(r"([+-])([=+-])", statement[end+1:end+4]):
            value = "1" if search.group(2) != '=' else statement[search.end()+1:].lstrip()
            translated = process_increment([token, value], search.group(1)=='-')
            return result.replace(statement[start:last], translated, 1)
        elif search := re.search(r"[-+]{2}", statement[start-2:start]):
            translated = process_increment([token, "1"], search.group(0)[0]=='-')
            result = result.replace(search.group(0) + token, translated, 1)
        elif search := re.search(r"([*/%^])([=*])", statement[end+1:end+4]):
            value = statement[search.end()+1:].lstrip()
            op = search.group(1) if search.group(2) != '*' else '^'
            translated = process_modify([token, value], op)
            return result.replace(statement[start:last], translated, 1)
        else:
            translated = process_query(token, True)
            result = result.replace(token, translated, 1)

    return result

def process_delete_array(statement: str) -> str:
    token = statement[7:] # Assuming delete is right at the start of the statement
    if universal or token in trees:
        return f'delete_tree("{token}")'
    else:
        return statement

# Check if a substring exists within the statement, and that it's not within quotes
def valid_token(substr: str, statement: str) -> bool:
    i = statement.find(substr)
    quote_start = statement.find('"')

    if i == -1:
        return False
    elif quote_start == -1:
        return True
    else:
        quote_end = statement.find('"', quote_start+1)
        while quote_start != -1 and quote_end != -1:
            if i > quote_start and i < quote_end: return False
            quote_start = statement.find('"', quote_end+1)
            quote_end = statement.find('"', quote_start+1)

    return True

# TODO: this doesn't account for being within quotes
def gen_keyword_pattern(keyword: str) -> str:
    return r"(^|[^\w])\s*" + keyword + r"\s*\("

# Check that the given keyword is present (by itself, i.e. with whitespace surrounding it) followed by a '('
def keyword_present(keyword: str, statement: str) -> bool:
    match = re.search(gen_keyword_pattern(keyword), statement)
    return match != None and valid_token(match.group(0), statement)

# TODO: use this for more than just checking else / if conditions
def contains_expression(statement: str) -> bool:
    return valid_token('[', statement) or \
        valid_token(" in ", statement) or \
        keyword_present("length", statement) or \
        keyword_present(IS_ARRAY, statement)

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

def process_statements(line: str, line_num: int = 0) -> str:
    nonspace = re.search(r"\S", line)
    result = ""
    if nonspace == None: return result
    indentation = sum([ 1 if c == ' ' else 4 for c in line[:nonspace.start()] ])
    statements, delims = parse_statements(line[nonspace.start():])
    del nonspace
    global close_scope_timer; close_scope_timer = -1
    global new_scopes_opened; new_scopes_opened = 0

    def process_statement(statement, delim) -> tuple[str, str]:
        translated = statement
        global close_scope_timer; close_scope_timer -= 1
        global new_scopes_opened

        if statement[:9] == "function ":
            process_function(statement)
        elif keyword_present("for", statement) and statement[0] == 'f':
            if valid_token(" in ", statement):
                match = find_matching(statement, statement.find('('))
                parts = [statement[:match], statement[match+1:]]
                statement = parts.pop(0) + ')'
                translated = process_for_in(statement)

                if len(parts[0].strip()) == 0:
                    delim = '' # Remove extra opening brace
                else: # Inline for-in
                    statements.insert(i+1, parts[0].lstrip())
                    if delim != " { ":
                        close_scope_timer = 1; new_scopes_opened += 1 # Replace next delimiter with '}'
                        delims.insert(i+1, delim) 
                        delim = ''
                    else: # Special case where an inline for-in is followed by a block
                        delims.insert(i+1, " { ")
                        delim = '' # Remove extra opening brace

                        try:
                            j = delims.index(" } ", i+2)
                            statements.insert(j+1, '')
                            delims.insert(j+1, " } ") # Add missing closing brace
                        except ValueError:
                            statements.append('')
                            delims.append(" } ") # Add missing closing brace
            else: # Standard for
                # TODO: could be expression to process for each part between ';'
                temp = translated + f"; {statements.pop(i+1)}; {statements[i+1]}"
                match = find_matching(temp, temp.find('('))
                translated, next = (temp[:match].rstrip(), temp[match+1:].lstrip())
                translated += ')'
                delims.pop(i+1)
                delim = ' '

                if len(next) == 0:
                    breakers.append(None)
                    statements.pop(i+1)
                    delim = delims.pop(i+1)
                else:
                    statements[i+1] = next
        elif keyword_present("while", statement) and statement[0] == 'w':
            breakers.append(None)
            # TODO: could be inline while loop, maybe make common function for splitting inline body stuff
        elif keyword_present("if", statement) and statement[0] == 'i':
            if delim != " { ":
                close_scope_timer = 1
                if i+1 < len(statements) and valid_token("else ", statements[i+1]): close_scope_timer = 2
                translated, body = process_if(statement)
                statements.insert(i+1, body)
                delims.insert(i+1, delim)
                delim = ' '
            elif contains_expression(statement):
                translated = process_expression(statement)
        # TODO: maybe stricter check for this
        elif valid_token("else ", statement) and statement[0] == 'e':
            if delim != " { ": # In case of an inline else
                close_scope_timer = 1
                body = statement[statement.find(' ')+1:].lstrip()
                translated = f"else "
                statements.insert(i+1, body)
                delims.insert(i+1, delim)
                delim = ''
            elif contains_expression(statement):
                translated = process_expression(statement)
        elif valid_token("return ", statement):
            close_scope_timer = 1
            next = statement[statement.find(' ')+1:].lstrip()
            statements.insert(i+1, next)
            delims.insert(i+1, delim)
            delim = ' '
            translated = "return"
        elif valid_token(" in ", statement) or \
            keyword_present("length", statement) or \
            keyword_present(IS_ARRAY, statement):
                translated = process_expression(statement)
        elif statement == "break" and len(breakers) != 0:
            breaker = breakers.pop()
            if breaker != None:
                translated = f"{breaker}; break"
        elif valid_token('[', statement):
            if valid_token('=', statement):
                # TODO: == needs priority if there's a situation where its used besides "if"
                translated = process_equals(statement)
            elif valid_token("++", statement):
                tokens = [statement.strip('+')]
                tokens.append("1")
                translated = process_increment(tokens)
            elif valid_token("--", statement):
                tokens = [statement.strip('-')]
                tokens.append("1")
                translated = process_increment(tokens, True)
            elif valid_token("delete ", statement):
                token = statement[7:] # Assuming delete is right at the start of the statement
                translated = process_delete_element(token)
            else: # "[ ]" used within an expression
                translated = process_expression(statement)
        elif valid_token("delete ", statement):
            translated = process_delete_array(statement)

        if close_scope_timer == 0:
            delim += " } " * new_scopes_opened
            new_scopes_opened = 0

        return (translated, delim)

    for i, statement in enumerate(statements):
        delim = delims[i]
        translated = statement
        global current_vars

        if len(statement) > 0:
           translated, delim = process_statement(statement, delim)

        if '}' in delim:
            for i in range(delim.count('}')):
                if len(breakers) != 0:
                    breakers.pop()
                if len(current_vars) != 0:
                    current_vars.update([ (key, val+1) for key, val in current_vars.items() ])
            else:
                current_vars = dict( filter(lambda i: i[1] > 0, current_vars.items()) )

        result += translated + delim

        if verbose and statement != translated:
            print(f"Token {i+1} of line {line_num}: {statement}\n", line, "---\n", result)

        if delim == "# ":
            result += statements[i+1] + '\n'
            break

    return ' ' * indentation + result

verbose = False
universal = False

if __name__ == "__main__":
    result = ""

    for arg in argv[1:]:
        if "-v" in arg:
            verbose = True
        elif "-u" in arg:
            universal = True
        else:
            break
        argv.pop(1)
    
    if len(argv) == 1:
        with open("README.md", 'r') as file:
            print(file.read())
        exit(0)
    elif '{' in argv[1]:
        for i, line in enumerate(argv[1].split('\n')):
            result += process_statements(line+'\n', i+1)
    else:
        with open(argv[1], newline='\n') as file:
            for i, line in enumerate(file):
                result += process_statements(line, i+1)

    print(result)
