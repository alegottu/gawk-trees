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

    result = f"{var_name} = tree_next({all_params}); "
    result += f"while (tree_iters_remaining({all_params}) > 0)"
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

# TODO: account for shortcuts like +=
def process_assignment(tokens: list) -> str:
    if '[' in tokens[0]:
        all_params = process_brackets(tokens[0], True)
        return f"tree_insert({all_params}{tokens[1]})"
    else:
        all_params = process_brackets(tokens[1])
        return f"{tokens[0]} = query_tree({all_params})"

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
    if "for(" in statement or "for (" in statement:
        if " in " in statement:
            end = statement.find(')')
            tokens = statement[statement.find('(')+1:end].split(" in ", maxsplit=1)
            result, breaker = process_for_in(tokens)
            breakers.append(breaker)

            if end != len(statement)-1: # accounts for if body of for loop is attached
                return result + " { " + process_statement(statement[end+1:], depth+1) + " } "
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
            return process_assignment(tokens)
        elif "++" in statement:
            # TODO: next
            pass
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

if __name__ == "__main__":
    with open(argv[1]) as file:
        print(file.read())
        print("---------")
        print()
        file.seek(0)
        result = ""

        for line in file:
            statements = re.split("[;{}]", line)
            current_pos = 0
            missing_brace = False

            for statement in statements:
                current_pos += max(len(statement), 1) # 1 default to track two delimiters right next to each other

                if "while(" in statement or "while (" in statement:
                    breakers.append(None)
                elif current_pos < len(line) and line[current_pos] == '}':
                    if len(breakers) > 0:
                        breakers.pop()

                    if missing_brace:
                        line = line[:current_pos] + " } " + line[current_pos:]
                        missing_brace = False

                translated = process_statement(statement.strip())
                if len(translated) >= 4 and translated[-4] == ';': # See note in process_statement
                    translated = translated[:-4]
                    missing_brace = True

                line = line.replace(statement, translated, 1)
            else:
                result += line # line is translated by this point

        print(result)
        with open(f"ext-{argv[1]}", 'w') as converted:
            converted.write(result)
