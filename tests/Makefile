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

LDFLAGS = \
  -lhdf5

ARTIFACTS = \
  main \
  $(OBJECTS)

OBJECTS = \
  main.o \
  test_unique_hid.o


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

test_unique_hid.o: test_unique_hid.cc ../include/h5.hpp
