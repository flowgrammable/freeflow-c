#include "arrival.h" // also includes types.h
#include "dataplane.h" // also includes types.h and util.h
#include "departure.h"
#include "packet.h" // contains np_context, np_packet and also includes types.h
#include "instruction.h" // don't think I need this in here
#include "port.h" // also includes util.h

unit_testing:
	cc -o unit_testing arrival.o dataplane.o departure.o packet.o port.o
arrival.o: arrival.c arrival.h types.h
	cc -o arrival.c
dataplane.o: dataplane.c dataplane.h types.h util.h
	cc -o dataplane.c
departure.o: departure.c departure.h 
	cc -o departure.c
packet.o: packet.c packet.h types.h
	cc -o packet.c
port.o: port.c port.h util.h
	cc -o port.c
clean:
	rm -rf *.o core
