import sys
from functools import reduce
import os
import subprocess
from pathlib import Path

# usage mem-benchmark.py dimension1 [dimension2] [...]
# make sure to have set the AWKLIBPATH env var correctly

if "benchmark" not in os.getcwd():
    print("Please run from the benchmark directory")
    exit()

def concat_args(arg1, arg2):
    return arg1 + ", " + arg2

def create_nested_fors(n, command, args=[""], use_ext=True):
    loops =""
    tab = "    "
    body = f'{command}("test", ' if use_ext else command + " test"

    for i in range(1, n+1):
        iter_var = chr(96+i)
        loops += tab * i
        loops += f"for ({iter_var}=0; {iter_var}<{sys.argv[i]}; {iter_var}++)\n"
        loops += tab * i
        loops += "{\n"
        body += f"{iter_var}, " if use_ext else f"[{iter_var}]"
    else:
        body += f"{reduce(concat_args, args)})\n" if use_ext else reduce(str.__add__, args) + "\n"
        body = (tab * len(sys.argv)) + body
        loops += body

    for i in range(n, 0, -1):
        loops += tab * i
        loops += "}\n"

    return loops

def create_nested_whiles(n):
    tab = "    "
    iter_vars = [chr(97)]
    loops = tab + 'while(tree_iters_remaining("test") > 0)\n'
    loops += tab + "{\n"
    loops += (tab * 2) + f'{iter_vars[0]}=tree_next("test")\n'
    body = 'print query_tree("test", '

    for i in range(2, n+1):
        iter_vars.append(chr(96+i))
        var_args = reduce(concat_args, iter_vars[:-1])
        loops += tab * i
        loops += f'while (tree_iters_remaining("test", {var_args}) > 0)\n'
        loops += tab * i
        loops += "{\n"
        loops += tab * (i+1)
        loops += f'{iter_vars[i-1]}=tree_next("test", {var_args})\n'
    else:
        body += f"{reduce(concat_args, iter_vars)})\n"
        body = (tab * len(sys.argv)) + body
        loops += body

    for i in range(n, 0, -1):
        loops += tab * i
        loops += "}\n"

    return loops


args = " -lhtrees"
massif = False
out = ("ext", "normal")

if len(sys.argv) == 1:
    print("""usage: mem-benchmark.py [-v|--verbose] [-m|--massif] [-b|--bintree] dimensions
    dimensions refer to the number of elements for each depth of the tree structure, e.g. 100 100 = htree[100][100]
    --verbose: also print info about the number of steps needed to traverse AVL trees during key operations
    --massif: use valgrind --tool=massif to profile instead of time -v
    --bintree: test the AVL tree version of htrees against the regular binary tree version""")
    exit()
else:
    for arg in sys.argv[1:]:
        if 'v' in sys.argv[1]:
            args = " -lvhtrees"
            sys.argv.pop(1)
        elif 'm' in sys.argv[1]:
            massif = True
            sys.argv.pop(1)
        elif 'b' in sys.argv[1]:
            args = " -lbinhtrees"
            out = ("bin", "avl")
            sys.argv.pop(1)

num_dims = len(sys.argv) - 1
name = reduce(lambda a, b: a + "-" + b, sys.argv[1:])
dirs = f"logs/{name}/"
Path(dirs).mkdir(parents=True, exist_ok=True)

with open('mem-benchmark.awk', 'w') as file:
    loops = create_nested_fors(num_dims, "tree_insert", ["rand()"])
    code = 'BEGIN {\n' + loops + "}\n"
    loops = create_nested_whiles(num_dims)
    code += "BEGIN {\n" + loops + "}"
    file.write(code)
    file.flush()
    command = f"gawk{args} -f {file.name}"

    if massif:
        process = subprocess.run(f'valgrind --tool=massif --pages-as-heap=yes --massif-out-file={dirs}{out[0]}.massif {command}', shell=True, check=True)
    else:
        process = subprocess.run(f'command time -o {dirs}{out[0]}.time -v {command}', shell=True, check=True)

if "bin" in args:
    script = "mem-benchmark.awk"
    args = " -lhtrees"
else:
    script = "mem-benchmark-noext.awk"
    args = ""

    with open(script, 'w') as file:
        loops = create_nested_fors(num_dims, "", ["=rand()"], False)
        code = "BEGIN {\n" + loops + "}\n"
        loops = create_nested_fors(num_dims, "print", use_ext=False)
        code += "BEGIN {\n" + loops + "}"
        file.write(code)
        file.flush()

with open(script, 'r') as file:
    command = f"gawk{args} -f {file.name}"

    if massif:
        process = subprocess.run(f'valgrind --tool=massif --pages-as-heap=yes --massif-out-file={dirs}{out[1]}.massif {command}', shell=True, check=True)
    else:
        process = subprocess.run(f'command time -o {dirs}{out[1]}.time -v {command}', shell=True, check=True)
