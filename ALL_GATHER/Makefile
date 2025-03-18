.phony : build

build:
	./../../charm-v7.0.0/bin/charmc allGather.ci
	./../../charm-v7.0.0/bin/charmc -o sim allGather.cc
	@echo "Build complete"

run:
	./charmrun +p20 ./sim 3 2 10 20 10 1
