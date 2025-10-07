import sys
import matplotlib.pyplot as plot
from numpy import array

# NOTE: Currently works with tables provided by running make-table.sh

if len(sys.argv) == 1 and sys.stdin.isatty():
   print("Usage: make_graph [memory analysis table] [time analysis table]") # TODO: maybe make README
   exit()

x = [] # Total number of elements in each array / tree
y = [] # Data savings
MARKERSIZE = 10

def get_line_data(line: str, x: list, y: list):
    line = line.strip("| \n")
    data = line.split(" | ")
    dimensions, savings = data[0], data[3]

    dimensions = dimensions.replace('^', "**")
    dimensions = dimensions.replace('x', '*')
    dimensions = dimensions.replace('-', '*')
    x.append(eval(dimensions))
    y.append(float(savings.rstrip('%')))

if len(sys.argv) > 1:
    for file in sys.argv[1:]:
        with open(file, 'r') as table:
            x.append([])
            y.append([])

            for line in table.readlines()[2:]:
                get_line_data(line, x[-1], y[-1])

        label = "Memory Decrease %" if len(x) == 1 else "Time Slowdown %"
        plot.plot(x[-1], y[-1], marker='.', markersize=MARKERSIZE, label=label)
        plot.legend()
else:
    for line in sys.stdin.readlines()[2:]:
        get_line_data(line, x, y)

    plot.plot(x, y, marker='.', markersize=MARKERSIZE)
    plot.ylabel("Memory Decrease (%)")

plot.xlabel("Total Elements")
plot.savefig("graphs/graph.jpg")
