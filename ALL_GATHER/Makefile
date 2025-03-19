CHARMC=../../charm-v7.0.0/bin/charmc

allGather:
	$(CHARMC) allGather.ci
	$(CHARMC) -c allGather.cc
	@echo "Built allGather Library"

sim: allGather
	$(CHARMC) user.ci
	$(CHARMC) -c user.cc
	@echo "Built simulation"
	$(CHARMC) -o sim user.o allGather.o

clean:
	rm -f *.decl.h *.def.h *.o charmrun sim

.phony : build
