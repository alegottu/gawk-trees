@include "temp"

BEGIN { for (i in SYMTAB) if (i ~ /[a-z]+/) print i }
