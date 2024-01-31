CXX = g++
CXXFLAGS = -O3 -std=c++11 -I /usr/local/include/boost/
SOURCES = main.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = hw2

all: $(SOURCES) ../bin/$(EXECUTABLE)

../bin/$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o:  %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: ../bin/$(EXECUTABLE)
	$^ ../testcase/public3.txt test.hgr ../output/public3.out

clean:
	rm -rf ../bin/$(EXECUTABLE)
