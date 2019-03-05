CFLAGS = -std=gnu99 -pedantic -Wall -Wextra
CCX = gcc


HDRDEP = $(wildcard *.h)


all:
	make ipk-dhcpstarve

clear:
	rm *.o ipk-dhcpstarve

# COMPILE OBJECTS
ipk-dhcpstarve_api.o: $(HDRDEP) ipk-dhcpstarve_api.c
	$(CCX) $(CFLAGS) ipk-dhcpstarve_api.c -c -o ipk-dhcpstarve_api.o

ipk-dhcpstarve.o: $(HDRDEP) ipk-dhcpstarve.c
	$(CCX) $(CFLAGS) ipk-dhcpstarve.c -c -o ipk-dhcpstarve.o

# LINK OBJECTS
ipk-dhcpstarve: $(HDRDEP) ipk-dhcpstarve.o ipk-dhcpstarve_api.o
	$(CCX) $(CFLAGS) ipk-dhcpstarve.o ipk-dhcpstarve_api.o -o ipk-dhcpstarve