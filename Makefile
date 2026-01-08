CXX = g++
CXXFLAGS = -std=c++17 -Wall -g
LIBS = -lglfw3 -lglew32 -lopengl32
TARGET = main.exe

SRC = main.cpp Engine/Renderer/VertexBuffer.cpp Engine/Renderer/IndexBuffer.cpp Engine/Renderer/VertexArray.cpp \
Engine/Math/Math.cpp Engine/Shader/Shader.cpp ObjReader.cpp Engine/InputHandler.cpp  \
 Engine/Engine.cpp Engine/ECS/Systems/BatchedRenderer.cpp Engine/Renderer/BufferedBatch.cpp \
 Engine/Renderer/SSBOBuffer.cpp Engine/Camera.cpp Game.cpp
OBJ = $(SRC:.cpp=.o)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)