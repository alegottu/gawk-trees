all:
	@# gcc -fPIC -shared -DHAVE_CONFIG_H -c -O -g -I../gawk -Ilib/include htrees.c
	@# gcc -Wl,--no-undefined -shared -o htrees.so htrees.o -Llib -lwayne
	@# config.h in include/
	gcc -fPIC -shared -DHAVE_CONFIG_H -c -O -I../gawk -Iinclude src/*
	gcc -Wl,--no-undefined -shared -o htrees.so *.o -lm
	rm *.o

debug:
	gcc -fPIC -shared -DHAVE_CONFIG_H -c -O0 -g -I../gawk -Iinclude src/*
	gcc -Wl,--no-undefined -shared -o htrees.so *.o -lm
	rm *.o
