CXX = h5c++

CXXFLAGS = \
  -std=c++2a \
  -Wpedantic \
  -Wall \
  -Wextra \
  -Wconversion \
  -Wsign-conversion \
  $(INCLUDES) \
  $(DBGFLAGS) \
  $(OPTFLAGS) \
  $(PKGFLAGS)

INCLUDES = \
  -isystem ../../include

DBGFLAGS = \
  -g \
  -fsanitize=address

OPTFLAGS = \
  -Og

PKGFLAGS = \
  $$(pkg-config --cflags eigen3 | sed 's/-I/-isystem/g')

ARTIFACTS = \
  main \
  main.o \
  dump.h5


.PHONY: run clean

run: main
	./main
	h5ls -rlv dump.h5

clean:
	rm -f $(ARTIFACTS)

main: main.cc
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)
