CXX = g++
CXXFLAGS = -std=c++17 -Wall -g -O3
LIBS = -lglfw3 -lglew32 -lopengl32
TARGET = main.exe

# Recursively find all .cpp files
SRC = $(shell find . -name "*.cpp")
OBJ = $(SRC:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) -g $(OBJ) -o $(TARGET) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean remake

remake: clean all