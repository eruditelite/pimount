%.o : %.c
	gcc $(CFLAGS) -c -o $*.o $*.c
	gcc $(CFLAGS) -MM $*.c > $*.d
