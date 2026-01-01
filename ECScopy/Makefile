CXX = g++
CXXFLAGS = -std=c++17 -Wall -g
LIBS = -lglfw3 -lglew32 -lopengl32
TARGET = main.exe

SRC = Test.cpp ECS.cpp Entity.cpp
OBJ = $(SRC:.cpp=.o)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)