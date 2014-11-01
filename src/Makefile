# build udp_led executable when user executes "make" 

udp_led: udp_led.o
	$(CC) $(LDFLAGS) udp_led.o -o udp_led

udp_led.o: udp_led.c 
	$(CC) $(CFLAGS) -c udp_led.c 

# remove object files and executable when user executes "make clean"
clean:
	rm *.o udp_led 
