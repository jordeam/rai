MDS = $(shell ls *.md)

ALL = $(MDS:.md=.html)

ASCIIDOC = asciidoctor

all: $(ALL)

clean:
	rm -f *~

distclean: clean
	rm -f $(ALL)

%.html: %.md
	$(ASCIIDOC) -b html $<
