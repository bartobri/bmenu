CC=gcc
ODIR=obj

_OBJ = bmenu.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c
	$(CC) -c -o $@ $<

bmenu: $(OBJ)
	$(CC) -o $@ $^

clean:
	rm -f $(ODIR)/*.o bmenu
