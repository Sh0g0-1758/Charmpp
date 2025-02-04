.phony : build run

build:
	./../../charm-v7.0.0/bin/charmc particle.ci
	./../../charm-v7.0.0/bin/charmc -o particle particle.cc -tracemode projections -module EveryLB
	@echo "Build complete"

run:
	./charmrun +p20 ./particle 10000 16 +balancer GreedyLB
