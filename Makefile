all:
	gcc -fPIC -shared -DHAVE_CONFIG_H -c -O -g -I../gawk -I../gawk/pc -Ilib/include -Llib/ -llibwayne htrees.c
	gcc -o htrees.so -shared htrees.o