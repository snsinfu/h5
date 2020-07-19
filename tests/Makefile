CXX = h5c++

CXXFLAGS = \
  -std=c++14 \
  -Wpedantic \
  -Wall \
  -Wextra \
  -Wconversion \
  -Wsign-conversion \
  $(INCLUDES) \
  $(DBGFLAGS) \
  $(OPTFLAGS) \
  $(EXTRA_CXXFLAGS)

INCLUDES = \
  -isystem include \
  -I ../include

DBGFLAGS = \
  -g \
  -fsanitize=address

OPTFLAGS = \
  -Og

ARTIFACTS = \
  main \
  $(OBJECTS)

OBJECTS = \
  main.o \
  test_unique_hid.o \
  test_shape.o \
  test_file.o \
  test_dataset.o \
  test_buffer.o \
  test_enums.o


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
test_shape.o: test_shape.cc ../include/h5.hpp
test_file.o: test_file.cc utils.hpp ../include/h5.hpp
test_dataset.o: test_dataset.cc utils.hpp ../include/h5.hpp
test_buffer.o: test_buffer.cc utils.hpp ../include/h5.hpp
test_enums.o: test_enums.cc utils.hpp ../include/h5.hpp
