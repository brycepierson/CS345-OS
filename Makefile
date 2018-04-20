#*******VARIABLES*******

cc=gcc
compflags=-c -g

#***********************

all: 345os

345os: os345.o os345fat.o os345interrupts.o os345lc3.o os345mmu.o os345p1.o os345p2.o os345p3.o os345p4.o os345p5.o os345p6.o os345signals.o os345park.o os345tasks.o os345semaphores.o
	$(cc) os345.o os345fat.o os345interrupts.o os345lc3.o os345mmu.o os345p1.o os345p2.o os345p3.o os345p4.o os345p5.o os345p6.o os345signals.o os345park.o os345tasks.o os345semaphores.o -o 345os
os345.o: os345.c os345.h os345signals.c os345signals.h os345config.h os345lc3.c os345lc3.h os345fat.c os345fat.h
	$(cc) $(compflags) os345.c

os345fat.o: os345.c os345.h os345fat.c os345fat.h
	$(cc) $(compflags) os345fat.c

os345interrupts.o: os345interrupts.c os345config.h os345signals.c os345signals.h
	$(cc) $(compflags) os345interrupts.c

os345lc3.o: os345lc3.c os345lc3.h os345.c os345.h os345fat.c os345fat.h
	$(cc) $(compflags) os345lc3.c

os345mmu.o: os345mmu.c os345.c os345.h os345lc3.c os345lc3.h
	$(cc) $(compflags) os345mmu.c

os345p1.o: os345.c os345.h os345signals.c os345signals.h os345p1.c
	$(cc) $(compflags) os345p1.c

os345p2.o: os345.c os345.h os345signals.c os345signals.h os345p2.c
	$(cc) $(compflags) os345p2.c

os345p3.o: os345.c os345.h os345park.c os345park.h os345p3.c
	$(cc) $(compflags) os345p3.c

os345p4.o: os345.c os345.h os345lc3.c os345lc3.h os345p4.c
	$(cc) $(compflags) os345p4.c

os345p5.o: os345.c os345.h os345p5.c
	$(cc) $(compflags) os345p5.c

os345p6.o: os345.c os345.h os345fat.c os345fat.h os345p6.c
	$(cc) $(compflags) os345p6.c

os345park.o: os345.c os345.h os345park.c os345park.h
	$(cc) $(compflags) os345park.c

os345semaphores.o: os345.c os345.h os345semaphores.c
	$(cc) $(compflags) os345semaphores.c

os345signals.o: os345.c os345.h os345signals.c os345signals.h
	$(cc) $(compflags) os345signals.c

os345tasks.o: os345.c os345.h os345signals.c os345signals.h os345tasks.c os345lc3.h
	$(cc) $(compflags) os345tasks.c

clean:
	rm *.o 345os
