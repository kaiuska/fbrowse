CFLAGS = -g 
LINKLIBS = -lncurses
DEPS = fbrowse.h fbutils.h

browse: fbrowse.c $(DEPS)
	$(CC) $(CFLAGS) $^ -o $@ $(LINKLIBS)

clean:
	rm browse
