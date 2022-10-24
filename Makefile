IDIR = ../include
CC = g++
CFLAGS = -I.
LIBS = -lsfml-graphics -lsfml-window -lsfml-system -lm
DEPS = 
OBJ = main.o

#DEPS = $(pathsubst %,$(IDIR)/%,$(_DEPS))
#OBJ = $(pathsubst %,$(ODIR)/%,$(_OBJ))

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

main: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f *.o main
