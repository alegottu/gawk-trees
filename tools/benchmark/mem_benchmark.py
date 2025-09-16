import sys
from functools import reduce
import os
import subprocess
from pathlib import Path

if "benchmark" not in os.getcwd():
    print("Please run from the benchmark directory")
    exit()

def concat_args(arg1, arg2):
    return arg1 + ", " + arg2

def create_nested_fors(dimensions, name, command, args=[""], use_ext=True):
    n = len(dimensions)
    loops =""
    tab = "    "
    body = f'{command}("{name}", ' if use_ext else command + f" {name}"

    for i in range(1, n+1):
        iter_var = chr(96+i)
        loops += tab * i
        loops += f"for ({iter_var}=0; {iter_var}<{dimensions[i-1]}; {iter_var}++)\n"
        loops += tab * i
        loops += "{\n"
        body += f"{iter_var}, " if use_ext else f"[{iter_var}]"
    else:
        body += f"{reduce(concat_args, args)})\n" if use_ext else reduce(str.__add__, args) + "\n"
        body = (tab * (n+1)) + body
        loops += body

    for i in range(n, 0, -1):
        loops += tab * i
        loops += "}\n"

    return loops

def create_nested_whiles(n, name):
    tab = "    "
    iter_vars = [chr(97)]
    loops = tab + f'while(tree_iters_remaining("{name}") > 0)\n'
    loops += tab + "{\n"
    loops += (tab * 2) + f'{iter_vars[0]}=tree_next("{name}")\n'
    body = f'print query_tree("{name}", '

    for i in range(2, n+1):
        iter_vars.append(chr(96+i))
        var_args = reduce(concat_args, iter_vars[:-1])
        loops += tab * i
        loops += f'while (tree_iters_remaining("{name}", {var_args}) > 0)\n'
        loops += tab * i
        loops += "{\n"
        loops += tab * (i+1)
        loops += f'{iter_vars[i-1]}=tree_next("{name}", {var_args})\n'
    else:
        body += f"{reduce(concat_args, iter_vars)})\n"
        body = tab * (n+1) + body
        loops += body

    for i in range(n, 0, -1):
        loops += tab * i
        loops += "}\n"

    return loops


if __name__ == "__main__":
    args = " -lhtrees"
    iteration = False
    massif = False
    print_script = 0
    out = ("ext", "normal")

    if len(sys.argv) == 1:
        with open("README.md") as file:
            print(file.read())
        exit()

    for arg in sys.argv[1:]:
        if 'i' in arg:
            iteration = True
        elif 'v' in arg:
            args = " -lvhtrees"
        elif 'm' in arg:
            massif = True
        elif 'b' in arg:
            args = " -lbinhtrees"
            out = ("bin", "avl")
        elif 'p' in arg:
            sys.argv.pop(1)
            print_script = int(sys.argv[1])
        else:
            break
        sys.argv.pop(1)

    num_trees = len(sys.argv) - 1
    name = reduce(lambda a, b: a + "+" + b, sys.argv[1:])
    dims = []

    for dim in sys.argv[1:]:
        if 'x' in dim:
            n, times = dim.split('x')
            for i in range(int(times)):
                dims.append(n.split('-'))
            continue
        dims.append(dim.split('-'))

    def write_script(loop_writers) -> str:
        loops = ""
        for i, dim in enumerate(dims):
            loops += loop_writers[0](dim, i)
        code = "BEGIN {\n" + loops + "}\n"

        if iteration:
            loops = ""
            for i, dim in enumerate(dims):
                loops += loop_writers[1](dim, i)
            code += "BEGIN {\n" + loops + "}"

        return code

    def do_test(command, n):
        if massif:
            dirs = f"logs/{name}/"
            Path(dirs).mkdir(parents=True, exist_ok=True)
            subprocess.run(f'valgrind --tool=massif --pages-as-heap=yes --massif-out-file={dirs}{out[n]}.massif {command}', shell=True, check=True)
        else:
            p1 = subprocess.run(f"command time -v {command}", stderr=subprocess.PIPE, shell=True)
            process = subprocess.run("grep -E 'wall|Max|Command'", input=p1.stderr, stdout=subprocess.PIPE, shell=True)
            mode = 'w' if n == 0 else 'a'
            with open(f"{name}.data", mode) as data:
                data.write(f"{out[n]}:\n")
                data.write(process.stdout.decode())
                data.flush()

    with open('.mem-benchmark.awk', 'w') as file:
        loop_writers = (
            lambda dim, i: create_nested_fors(dim, f"test{i}", "tree_insert", ["rand()"]),
            lambda dim, i: create_nested_whiles(len(dim), f"test{i}")
            )

        if print_script == 0:
            file.write(write_script(loop_writers));
            file.flush()
            do_test(f"gawk{args} -f {file.name}", 0)
        elif print_script == 1:
            print(write_script(loop_writers))

    if "bin" in args:
        script = ".mem-benchmark.awk"
        args = " -lhtrees"
    else:
        script = ".mem-benchmark-noext.awk"
        args = ""

        with open(script, 'w') as file:
            loop_writers = (
                lambda dim, i: create_nested_fors(dim, f"test{i}", "", ["=rand()"], False),
                lambda dim, i: create_nested_fors(dim, f"test{i}", "print", use_ext=False)
                )

            if print_script == 0:
                file.write(write_script(loop_writers))
                file.flush()
            elif print_script == 2:
                print(write_script(loop_writers))
                exit()

    with open(script, 'r') as file:
        do_test(f"gawk{args} -f {file.name}", 1)

