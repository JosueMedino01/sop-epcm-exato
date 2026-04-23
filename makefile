compile:
	arch -x86_64 g++ app.cpp -std=c++11 \
	-I/Applications/CPLEX_Studio129/cplex/include \
	-I/Applications/CPLEX_Studio129/concert/include \
	-L/Applications/CPLEX_Studio129/cplex/lib/x86-64_osx/static_pic \
	-L/Applications/CPLEX_Studio129/concert/lib/x86-64_osx/static_pic \
	-DIL_STD -lilocplex -lconcert -lcplex -lm -o app.out
	
run:
	./app.out instances/large/large_56_1.txt 0 0 3000 0.5 1800 > CPLEX_LOG/large_56_1.3000.05.1800.txt

clear:
	rm -f *.o app.out

bug:
	arch -x86_64 g++ sop-epcm.cpp -std=c++11 \
	-I/Applications/CPLEX_Studio129/cplex/include \
	-I/Applications/CPLEX_Studio129/concert/include \
	-L/Applications/CPLEX_Studio129/cplex/lib/x86-64_osx/static_pic \
	-L/Applications/CPLEX_Studio129/concert/lib/x86-64_osx/static_pic \
	-DIL_STD -lilocplex -lconcert -lcplex -lm  -g -o a.out



tsp:
	g++ tsp.cpp -std=c++11 \
	-I/Applications/CPLEX_Studio129/cplex/include \
	-I/Applications/CPLEX_Studio129/concert/include \
	-L/Applications/CPLEX_Studio129/cplex/lib/x86-64_osx/static_pic\
	-L/Applications/CPLEX_Studio129/concert/lib/x86-64_osx/static_pic\
	-DIL_STD -lilocplex -lconcert -lcplex -lm -o a.out
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
	time ./heuristic.out -f ./instancias/quality/instances/berlin52FSTCII_q2_g4_p40_r20_s20_rs15.pop -C 0.001 -K 2 -T 10 -maxNotImproviment 1000 -maxIterTabu 100 -seed 13

heuristic-bug:
	g++ orienteering-problem-heuristic.cpp -g -o a.out

trouble: 
	g++ -std=c++11 calc_path_cost.cpp -o calc_path_cost.out
	./calc_path_cost.out Artificial_POP_Instance_04.txt "0 12 21 41 16 43 51 45 2 0"

compile-generate: 
	g++ -std=c++11 generate.cpp -o generate.out