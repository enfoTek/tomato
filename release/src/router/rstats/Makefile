include ../common.mak

CFLAGS	= -O2 -Wall #-mips32
CFLAGS	+= -I$(SRCBASE)/include -I$(TOP)/shared
LDFLAGS	=

OBJS = rstats.o

all: rstats

rstats: $(OBJS)
	@echo " [rstats] CC -o $@"
	@$(CC) $(LDFLAGS) -o $@ $(OBJS) -L../nvram -lnvram -L../shared -lshared

	$(SIZECHECK)
	$(CPTMP)

clean:
	rm -f rstats .*.depend
	rm -f *.o

install: all
	@echo " [rstats] Installing..."
	install -D -m 0500 rstats $(INSTALLDIR)/bin/rstats
	$(STRIP) $(INSTALLDIR)/bin/rstats

%.o: %.c .%.depend
	@echo " [rstats] CC $@"
	@$(CC) $(CFLAGS) -c $<

.depend: $(OBJS:%.o=%.c)
	@$(CC) $(CFLAGS) -M $^ > .depend

.%.depend: %.c
	@$(CC) $(CFLAGS) -M $< > $@

-include $(OBJS:%.o=.%.depend)
