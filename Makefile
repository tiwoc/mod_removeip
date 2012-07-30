LIBS=
APXS=`which apxs2`

SOURCES=mod_removeip.c
TARGETS=$(SOURCES:.c=.la)


all: $(TARGETS)

%.la: %.c
	$(APXS) -c $< $(LIBS)

install: $(TARGETS)
	$(APXS) -i $(TARGETS)

clean:
	-rm -f $(TARGETS) *~ $(SOURCES:.c=.slo) $(SOURCES:.c=.lo) $(SOURCES:.c=.so) $(SOURCES:.c=.o) 
	-rm -rf .libs
