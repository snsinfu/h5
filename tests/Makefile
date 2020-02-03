CXXFLAGS = \
  -std=c++14 \
  -Wpedantic \
  -Wall \
  -Wextra \
  -Wconversion \
  -Wsign-conversion \
  $(INCLUDES) \
  $(DBGFLAGS) \
  $(OPTFLAGS)

INCLUDES = \
  -isystem include \
  -I ../include

DBGFLAGS = \
  -g \
  -fsanitize=address

OPTFLAGS = \
  -O2

ARTIFACTS = \
  main

OBJECTS = \
  main.o


.PHONY: run clean
.SUFFIXES: .cc

run: main
	./main

clean:
	rm -f $(ARTIFACTS)

main: $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS) $(LDFLAGS)

.cc.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

-include: depends.mk

depends.mk:
	for src in $(SOURCES); do \
	$(CXX) $(CXXFLAGS) -MD -MF- -MT ${src%.*}.o ${src}; done > $@
