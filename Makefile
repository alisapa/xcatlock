# Use these to pass extra CFLAGS/LDFLAGS for debugging
EXTRA_CFLAGS=
EXTRA_LDFLAGS=

CFLAGS=-O2 $(EXTRA_CFLAGS)
LDFLAGS=-lX11 $(EXTRA_LDFLAGS)

OBJDIR=obj
SRCDIR=src

_DEPS=unicode.h
_OBJS=unicode.o main.o
_SRCS=unicode.c main.c
DEPS=$(patsubst %,$(SRCDIR)/%,$(_DEPS))
OBJS=$(patsubst %,$(OBJDIR)/%,$(_OBJS))

INSTALL_PREFIX=/usr/local
INSTALL_BIN=$(INSTALL_PREFIX)/bin
INSTALL_MAN=$(INSTALL_PREFIX)/share/man

xcatlock: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	@mkdir -p $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o
	rm -f xcatlock

.PHONY: install
install:
	mkdir -p $(INSTALL_BIN)
	cp xcatlock $(INSTALL_BIN)/xcatlock
#	mkdir -p $(INSTALL_MAN)
#	cp man/xcatlock.1 $(INSTALL_MAN)/xcatlock
.PHONY: uninstall
uninstall:
	rm -f $(INSTALL_BIN)/xcatlock
#	rm -f $(INSTALL_MAN)/xcatlock.1
