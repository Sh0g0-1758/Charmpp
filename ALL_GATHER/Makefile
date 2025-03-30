CHARMC=../../charm-v7.0.0/bin/charmc

allGather:
	$(CHARMC) -Ofast allGather.ci
	$(CHARMC) -Ofast -c allGather.cc
	@echo "Built allGather Library"

sim: allGather
	$(CHARMC) -Ofast user.ci
	$(CHARMC) -Ofast -c user.cc
	@echo "Built simulation"
	$(CHARMC) -Ofast -o sim user.o allGather.o

run: sim
	./charmrun +p19 ./sim 64 29 10 10

clean:
	rm -f *.decl.h *.def.h *.o charmrun sim

.phony : run clean sim allGather
