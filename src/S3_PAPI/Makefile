GCC	= g++
FLAGS	= -Wall -pthread -msse4
EXE	= main
OBJ	= main.o 

default: $(EXE) clean

main.o: main.cpp  
	$(GCC) -O3 -c -o $@ main.cpp $(FLAGS)

$(EXE): $(OBJ)
	$(GCC) -O3 $(OBJ) -o $(EXE) $(FLAGS)

clean:
	rm -rf *.o 
