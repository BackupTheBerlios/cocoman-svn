VERSION=0.3.0

CC=gcc
#CFLAGS=-g -Wall -Wpointer-arith -Wstrict-prototypes -W -pedantic -fPIC
CFLAGS=-g -Wall -Wpointer-arith -Wstrict-prototypes -fPIC
#CFLAGS=-g -Werror -fPIC
LD=ld
LDFLAGS=-g

libdir=/lib
sysconfdir=/etc
bindir=/usr/bin

SRC=libsoapbox.c
LIB=libsoapbox.so
BIN=soapbox.sh
SPEC=soapbox.spec

all: $(LIB) $(BIN) $(SPEC)

$(LIB): $(SRC)
#	$(CC) $(CFLAGS) $(SRC) -nostartfiles -shared -Wl,-soname,$(LIB) -o $(LIB) -ldl -lm -lc
	$(CC) $(CFLAGS) $(SRC) -nostartfiles -shared -rdynamic -Wl,-soname,$(LIB) -o $(LIB) -ldl -lm

$(BIN):
	cat $(BIN).in | sed -e 's|@@VERSION@@|$(VERSION)|' > $(BIN)

$(SPEC):
	cat $(SPEC).in | sed -e 's|@@VERSION@@|$(VERSION)|' > $(SPEC)

install: all
	install -d -m0755 $(DESTDIR)$(libdir) $(DESTDIR)$(bindir)
	install -m0755 $(LIB) $(DESTDIR)$(libdir)
	install -m0755 $(BIN) $(DESTDIR)$(bindir)/soapbox

clean:
	rm -f $(LIB) $(BIN) $(SPEC)

test: $(LIB)
	@echo "Create test env"
	-rmdir /var/tmp/soapboxdir* || :
	-rm -f /var/tmp/soapbox* || :
	-touch /var/tmp/soapbox{1,2,3}
	-mkdir -p /var/tmp/soapbox{dir1,dir2,dir3}
	-cd /var/tmp
	@echo -n "TEST 1: cmd = "
	-LD_PRELOAD="./libsoapbox.so" touch /var/tmp/soapboxA
	@echo -n "TEST 2: cmd = "
	-LD_PRELOAD="./libsoapbox.so" touch /etc/soapbox
	@echo -n "TEST 3: cmd = "
	-LD_PRELOAD="./libsoapbox.so" touch /etc/../etc/X11/../soapbox
	@echo -n "TEST 4: cmd = "
	-LD_PRELOAD="./libsoapbox.so" touch /var/tmp/soapboxdir2/../../tmp/file
	@echo -n "TEST 5: cmd = "
#	-LD_PRELOAD="./libsoapbox.so" cp -f /etc/foobar/../passwd /var/tmp
	-LD_PRELOAD="./libsoapbox.so" cp -f /etc/passwd /var/tmp
	@echo -n "TEST 10: cmd = "
	-LD_PRELOAD="./libsoapbox.so" mv -f /var/tmp/soapbox1 /var/tmp/soapboxB
	@echo -n "TEST 20: cmd = "
	-LD_PRELOAD="./libsoapbox.so" ln -f /var/tmp/soapbox3 /var/tmp/soapboxln1
	@echo -n "TEST 25: cmd = "
	-LD_PRELOAD="./libsoapbox.so" ln -sf /var/tmp/soapboxdir /var/tmp/soapboxln2
	@echo -n "TEST 26: cmd = "
	-LD_PRELOAD="./libsoapbox.so" ln -sf /bin/true /var/tmp/soapboxln3
	@echo -n "TEST 27: cmd = "
	-LD_PRELOAD="./libsoapbox.so" ln -sf /bin/true soapboxln4
	@echo -n "TEST 28: cmd = "
	-LD_PRELOAD="./libsoapbox.so" ln -sf /tmp /var/tmp/soapboxln5
	@echo -n "TEST 30: cmd = "
	-LD_PRELOAD="./libsoapbox.so" rm -f /var/tmp/libsoapbox2 soapboxln3
	@echo -n "TEST 31: cmd = "
	-LD_PRELOAD="./libsoapbox.so" rm -f /var/../var/tmp/../tmp/soapbox /var/tmp/passwd /var/tmp/null
	@echo -n "TEST 32: cmd = "
	-LD_PRELOAD="./libsoapbox.so" rm -f soapboxln4
	@echo -n "TEST 35: cmd = "
	-LD_PRELOAD="./libsoapbox.so" chown adm.adm /var/tmp/soapbox2
	@echo -n "TEST 40: cmd = "
	-LD_PRELOAD="./libsoapbox.so" chmod 750 /var/tmp/soapbox3
	@echo -n "TEST 45: cmd = "
	-LD_PRELOAD="./libsoapbox.so" mknod /var/tmp/soapboxdev c 1 3
	@echo -n "TEST 50: cmd = "
	-LD_PRELOAD="./libsoapbox.so" mkdir /var/tmp/foo/../soapboxdirA
	@echo -n "TEST 55: cmd = "
	-LD_PRELOAD="./libsoapbox.so" rmdir /var/tmp/bar/../soapboxdir1
	@echo -n "TEST 56: cmd = "
	-LD_PRELOAD="./libsoapbox.so" rmdir /var/tmp/soapboxln5
	@echo -n "TEST 57: cmd = "
	-LD_PRELOAD="./libsoapbox.so" rmdir /var/tmp/../tmp/soapboxdir1
	@echo -n "TEST 60: cmd = "
	-LD_PRELOAD="./libsoapbox.so" mkfifo -m0755 /var/tmp/soapboxfifo1
	@echo -n "TEST 61: cmd = "
	-LD_PRELOAD="./libsoapbox.so" mkfifo -m0600 /var/log/../tmp/soapboxfifo2
	@echo -n "TEST 65: cmd = "
	-LD_PRELOAD="./libsoapbox.so" install -c -D /etc/passwd /var/log/../tmp/soapboxdir2/blah/passwd
	@echo
