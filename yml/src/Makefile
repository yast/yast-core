# minimal Makefile for YMLparser
#
#                                 by jdsn

INCL  = -I /usr/include/libxml2 -I ../../libycp/src/include  -I ../../libycp/src/include/ycp  -I ../../liby2/src/include  -I ../../../liby2util/src/include/
LINK  = -l xml2 -lstdc++ -lycp -ly2
DIR   = -L .
DEBUG = -ggdb

YML: YMLparser.cc
	gcc -o $@ $(INCL) $< $(LINK) $(DIR)

YMLdebug: YMLparser.cc
	gcc -o $@ $(INCL) $< $(LINK) $(DEBUG) $(DIR)
    

all: YML

clean:
	rm YML YMLdebug test.ycp

test: YML
	./YML > test.ycp && y2base ./test.ycp qt
