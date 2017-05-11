CC = gcc
CFLAGS  = -O2
#LFLAGS = -lncurses -lpthread -lm
LFLAGS = -lpthread

# typing 'make' will invoke the first target entry in the file 
# (in this case the default target entry)
# you can name this target entry anything, but "default" or "all"
# are the most commonly used names by convention
#
default: mr

# To create the executable file count we need the object files
# countwords.o, counter.o, and scanner.o:
#
mr:  mr.o
	$(CC) $(CFLAGS) -o mr mr.o $(LFLAGS)

# To create the object file countwords.o, we need the source
# files countwords.c, scanner.h, and counter.h:
#
mr.o:  mr.c 
	$(CC) $(CFLAGS) -c mr.c

# To start over from scratch, type 'make clean'.  This
# removes the executable file, as well as old .o object
# files and *~ backup files:
#
clean: 
	/bin/rm -f mr *.o *~

