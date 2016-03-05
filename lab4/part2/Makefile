# CS 111 Lab 1 Makefile

CC = gcc
CFLAGS = -g -Wall -Wextra -Wno-unused -Werror
LAB = 1
DISTDIR = lab1-$(USER)

all: addtest

TESTS = $(wildcard test*.sh)
TEST_BASES = $(subst .sh,,$(TESTS))

ADDTEST_SOURCES = main.c 
ADDTEST_OBJECTS = $(subst .c,.o,$(ADDTEST_SOURCES))

DIST_SOURCES = \
  $(ADDTEST_SOURCES) Makefile \
  $(TESTS) README

addtest: $(ADDTEST_OBJECTS)
	$(CC) -pthread -lrt $(CFLAGS) -o $@ $(ADDTEST_OBJECTS)


dist: $(DISTDIR).tar.gz

$(DISTDIR).tar.gz: $(DIST_SOURCES) 
	rm -fr $(DISTDIR)
	tar -czf $@.tmp --transform='s,^,$(DISTDIR)/,' $(DIST_SOURCES)
	mv $@.tmp $@

check: $(TEST_BASES)

$(TEST_BASES): addtest
	./$@.sh

clean:
	rm -fr *.o *~ *.bak *.tar.gz core *.core *.tmp sltest $(DISTDIR)
	rm *.txt


