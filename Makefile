PROGNAME := 6502em

all: $(PROGNAME)

$(PROGNAME): main.o 6502.o
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf *.o
	rm -rf $(PROGNAME)
