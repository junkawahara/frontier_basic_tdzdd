OPT = -O3 -I. -Wall -fmessage-length=0

main: program.cpp
	g++ $(OPT) program.cpp -o program

clean:
	rm -rf *.o
