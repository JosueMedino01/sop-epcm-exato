run: 
	g++ app.cpp iterated-local-search/IteratedLocalSearch.cpp greedy-algorithm/GreedyAlgorithm.cpp -o app.out
	time ./app.out -f ../instancias/quality/instances/berlin52FSTCII_q2_g4_p40_r20_s20_rs15.pop -C 0.001 -K 2 -T 10 -maxNotImproviment 1000 -maxIterTabu 100 -seed 13
	rm -f *.o app.out

bug: 
