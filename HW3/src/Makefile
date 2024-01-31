CXX = g++
CXXFLAGS = -O3 -std=c++11 -I /usr/local/include/boost/
SOURCES = main.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = hw3

all: $(SOURCES) ../bin/$(EXECUTABLE)

../bin/$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o:  %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
 


clean:
	rm -rf ../bin/$(EXECUTABLE)