.phony : build run

build:
	./../../charm-v7.0.0/bin/charmc sort.ci
	./../../charm-v7.0.0/bin/charmc -o sort sort.cc
	@echo "Build complete"

run:
	./charmrun +p10 ./sort 100
