# Change these as you see fit
PREFIX=/usr
CONF=/etc/ondirrc

SOURCES=conf.c ondir.c
HEADERS=conf.h ondir.h
OBJS=conf.o ondir.o
TARGET=ondir

VERSION=0.2.3
DESTDIR=

# Add -DUSE_ONENTERLEAVE to CFLAGS to enable support for .onenter/.onleave
# scripts.
# **WARNING** This is not recommended at all.

CC=cc
CFLAGS=-O3 -DVERSION=\"$(VERSION)\" -DGLOBAL_CONF=\"$(CONF)\" -DUSE_ONENTERLEAVE
CFLAGS=-Wall -c -g -DVERSION=\"$(VERSION)\" -DGLOBAL_CONF=\"$(CONF)\"

LD=cc
LDFLAGS=
LDFLAGS=-g

$(TARGET): $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -o $@
	@echo
	@echo "OnDir is built."
	@echo
	@echo "Type 'make DESTDIR=<pkg-root> install' to install."
	@echo

clean:
	rm -f $(OBJS) $(TARGET)

install: $(TARGET)
	install -m 755 -d $(DESTDIR)$(PREFIX)/share/man/man1
	install -m 644 ondir.1 $(DESTDIR)$(PREFIX)/share/man/man1
	install -m 755 -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 ondir $(DESTDIR)$(PREFIX)/bin

package: slackware rpm
	chown athomas:athomas *
	chmod og-rwx *
	chmod a+r ondir-$(VERSION)*
	
slackware: $(TARGET)
	# Make SlackWare package
	rm -rf /tmp/ondir.pkg && \
		make DESTDIR=/tmp/ondir.pkg PREFIX=/usr CONF=/etc/ondirrc install && \
		cd /tmp/ondir.pkg && \
		makepkg -l y -c y ${PWD}/ondir-$(VERSION)-i386-1.tgz && \
		rm -rf /tmp/ondir.pkg

rpm: dist $(TARGET)
	cp ondir-$(VERSION).tar.gz /usr/src/rpm/SOURCES
	rpm -ba ondir.spec
	cp /usr/src/rpm/SRPMS/ondir-$(VERSION)-1.src.rpm ${PWD}
	cp /usr/src/rpm/RPMS/i386/ondir-$(VERSION)-1.i386.rpm ${PWD}

dist: clean
	rm -f ondir-$(VERSION)* && \
		todo -T && \
		cd .. && \
		mv ondir ondir-$(VERSION) && \
		tar -czv --exclude 'old/*' --exclude '.*.swp' -f ondir-$(VERSION).tar.gz ondir-$(VERSION) && \
		mv ondir-$(VERSION) ondir && \
		mv ondir-$(VERSION).tar.gz ondir

dep:
	@makedepend $(CXXFLAGS) $(SOURCES) 2> /dev/null
