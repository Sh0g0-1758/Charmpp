.phony : build

build:
	./../../charm-v7.0.0/bin/charmc particle.ci
	./../../charm-v7.0.0/bin/charmc -o particle particle.cc -tracemode projections
	@echo "Build complete"

run:
	./charmrun +p20 ./particle 10000 16
