CC = gcc
LDFLAGS = -lGL -lGLEW -lglfw -lm -lsndfile
CFLAGS = -Wall -Ofast -fstrict-aliasing -march=native
OBJECTS = occo.o

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

occo: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

debug:	CFLAGS= -Wall -O0 -g  -fstrict-aliasing -march=native
debug: 	$(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o occo-$@ $(LDFLAGS)


clean:
	@echo cleaning oop
	@rm -f $(OBJECTS)
purge:
	@echo purging oop
	@rm -f $(OBJECTS)
	@rm -f occo
	@rm -f occo-debug
