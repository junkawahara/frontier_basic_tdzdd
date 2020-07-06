OPT = -O3 -I.

main: program.cpp
	g++ $(OPT) program.cpp -o program

clean:
	rm -rf *.o
