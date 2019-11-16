CC = mpicc
CFLAGS = -O2 
LDFLAGS = -L../simple-jpeg/ -lsimplejpeg

PROJ = rgb_main
OBJS = rgb_main.o

all : simple-jpeg $(PROJ)

rgb_main : $(OBJS)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

simple-jpeg :
	$(MAKE) -C ../simple-jpeg

obj-clean:
	$(RM) *.o

exec-clean:
	$(RM) $(PROJ)

autosave-clean:
	$(RM) *~

clean:
	$(MAKE) obj-clean
	$(MAKE) exec-clean
	$(MAKE) autosave-clean
	$(MAKE) -C ../simple-jpeg clean
