CHARMC=charmc -module liveViz $(OPTS) -g -I/home/anant/winter2024/kale/study/charm/src/libs/ck-libs/liveViz

OBJS = particle.o

all: particle

particle: $(OBJS)
	$(CHARMC) -language charm++ -o particle $(OBJS)

particle.decl.h: particle.ci
	$(CHARMC)  particle.ci

particle.def.h: particle.decl.h

clean:
	rm -f *.decl.h *.def.h conv-host *.o particle charmrun *~

particle.o: particle.C particle.decl.h particle.def.h
	$(CHARMC) -c particle.C
