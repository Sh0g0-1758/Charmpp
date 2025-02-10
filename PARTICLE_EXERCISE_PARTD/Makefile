.phony : build

build:
	./../../charm-v7.0.0/bin/charmc particle_1.ci
	./../../charm-v7.0.0/bin/charmc -o particle particle_1.cc -tracemode projections -module EveryLB -module liveViz
	@echo "Build complete"

run:
	./charmrun +p20 ./particle 10000 10 +balancer GreedyRefineLB ++server ++server-port 1234
