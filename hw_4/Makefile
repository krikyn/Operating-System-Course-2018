project: main.o libstatic.a libdynamic1.so
	gcc -o project main.o -L. -lstatic -ldl -ldynamic1 -Wl,-rpath,.

main.o: main.c
	gcc -c main.c

libstatic.a: libstatic.o
	ar cr libstatic.a libstatic.o

libstatic.o: libstatic.c
	gcc -c libstatic.c

libdynamic1.so: libdynamic1.o
	gcc -shared -o libdynamic1.so libdynamic1.o

libdynamic1.o: libdynamic1.c
	gcc -c -fPIC libdynamic1.c

clean:
	rm -f *.o *.so *.a project
	gcc -c -fPIC libdynamic2.c
	gcc -shared -o libdynamic2.so libdynamic2.o
	rm *.o