
all: aufg42_first aufg42_buddy

aufg42_buddy: aufg42.o memory.c
	$(CC) $(CFLAGS) -g -DBUDDY -c memory.c
	$(CC) $(CFLAGS) -o aufg42_buddy aufg42.o memory.o

aufg42_first: aufg42.o memory.c
	$(CC) $(CFLAGS) -g -DFIRST -c memory.c
	$(CC) $(LDFLAGS) -o aufg42_first aufg42.o memory.o

.PHONY: clean
clean:
	rm -rf *.o
	rm -rf aufg42_first
	rm -rf aufg42_buddy
