

# Target to compile CPLEX
build:
	g++ orienteering-problem-exact.cpp -std=c++11 \
	-I/home/josue/CPLEX_Studio129/cplex/include \
	-I/home/josue/CPLEX_Studio129/concert/include \
	-L/home/josue/CPLEX_Studio129/cplex/lib/x86-64_linux/static_pic \
	-L/home/josue/CPLEX_Studio129/concert/lib/x86-64_linux/static_pic \
	-DIL_STD -lilocplex -lconcert -lcplex -lm -o a.out

run: 
	./a.out
	rm -f *.o a.out
	
clean:
	rm -f *.o a.out

read: 
	g++ read.cpp -o read.out
	./read.out
	rm -f *.o read.out

heuristic:
	g++ orienteering-problem-heuristic.cpp -o heuristic.out
	time ./heuristic.out
	

heuristic-bug:
	g++ orienteering-problem-heuristic.cpp -g -o a.out
