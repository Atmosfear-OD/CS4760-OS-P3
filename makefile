all: runsim testsim

testsim: testsim.o license.o sem.o
	gcc -g -o testsim testsim.o license.o sem.o -pthread

runsim: runsim.o license.o sem.o
	gcc -g -o runsim runsim.o license.o sem.o -pthread

runsim.o: runsim.c
	gcc -g -c runsim.c -pthread

testsim.o: testsim.c
	gcc -g -c testsim.c -pthread

license.o: license.c
	gcc -g -c license.c

sem.o: sem.c
	gcc -g -c sem.c

clean:
	rm -rf *.o runsim testsim
