CXX = g++
CXXFLAGS = -std=c++17 -Wall -g
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
	rm -f $(OBJ) $(TARGET) gmon.out analysis.txt

profile: $(TARGET)
	./$(TARGET)
	gprof $(TARGET) gmon.out > analysis.txt
	@echo "Profile analysis written to analysis.txt"

.PHONY: all clean profile