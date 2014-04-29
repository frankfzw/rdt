# NOTE: Feel free to change the makefile to suit your own need.

# compile and link flags
CCFLAGS = -Wall -g
LDFLAGS = -Wall -g

# make rules
TARGETS = rdt_sim
#TARGETS = test

all: $(TARGETS)

.cc.o:
	g++ $(CCFLAGS) -c -o $@ $<

rdt_sender.o: 	rdt_struct.h rdt_sender.h rdt_help.h

rdt_receiver.o:	rdt_struct.h rdt_receiver.h rdt_help.h

rdt_help.o: rdt_struct.h rdt_help.h

rdt_sim.o: 	rdt_struct.h

rdt_sim: rdt_sim.o rdt_sender.o rdt_receiver.o rdt_help.o
	g++ $(LDFLAGS) -o $@ $^

#clean:
#	rm -f *~ *.o $(TARGETS)

#rdt_help.o: rdt_struct.h rdt_help.h

#test.o: rdt_struct.h rdt_help.h

#test: rdt_help.o test.o
#	g++ $(LDFLAGS) -o $@ $^

clean:
	rm -f *~ *.o $(TARGETS)

