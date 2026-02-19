Usage: convert.py [-v|--verbose] [-u|--universal] [file.awk] [script only if no file]

The output will be the converted version of the provided script, which with you can do whatever you like.

If the output doesn't look correct, you can use -v or --verbose to get more information on the steps the transpiler
is taking and where it goes wrong, but your best bet is probably just to convert your script yourself using the
table provided in the README of the main directory of the repository.

Recognizing which variable names refer to arrays is something the script handles automatically,
but there are certain situations where this cannot traced accurately; for example, if you wanted to
delete an array that may or may not exist as the first part of a function, as in "delete x", this instruction
will be the first time "x" is used in that scope, and therefore not recognized as an array.
To get around this, the -u or --universal option indiscriminately translates any array related commands to
use their gawk-trees counterparts instead. This may lead to undesired behavior, such as getting the length of a
string, e.g. `length(s)`, being translated to the length of a non-existent tree: `tree_length("s")`.
For any translations that do occur (using -u or not), existing variables in the current scope will always take
precedence for the name of a tree, instead of a string of said potential variable name. For example, if the
variable "name" is in the current scope where the script comes across `arr[0]`, it will always be translated
to `tree_query(name, 0)` instead of `tree_query("name", 0)`, even if "name" is not a scalar and cannot be
converted to a string, so take care to avoid using duplicate variable or tree names if you want to prevent this.
