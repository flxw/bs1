
aufg42_first: aufg42.o
	$(CC) $(CFLAGS) -g -DFIRST -c memory.c
	$(CC) $(LDFLAGS) -o aufg42_first aufg42.o memory.o

.PHONY: clean
clean:
	rm -rf *.o
	rm -rf aufg42_first
	rm -rf aufg42_buddy
