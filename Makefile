CXX = g++
STD = -std=c++11
INC = -Isrc
CFLAGS = -g -O0 -Wpedantic -Wextra -Wall
DEFS = -DGLM_FORCE_RADIANS -D_USE_MATH_DEFINES -DGLEW_STATIC
DEPFLAGS = -MMD -MP
EXE = game
SRC = $(wildcard src/*.cpp)
OBJ = $(SRC:.cpp=.o)
DEP = $(SRC:.cpp=.d)

GLEW_CFLAGS = `pkg-config --cflags glew`
GLEW_LIBS = `pkg-config --libs glew`
GLFW3_CFLAGS = `pkg-config --cflags glfw3`
GLFW3_LIBS = `pkg-config --libs glfw3`
GLM_CFLAGS =
GLM_LIBS = -lglmf32
JPEG_CFLAGS =
JPEG_LIBS = -ljpeg
PNG_CFLAGS = `pkg-config --cflags libpng`
PNG_LIBS = `pkg-config --libs libpng`
BOOST_CFLAGS =
BOOST_LIBS = -lboost_system-mt -lboost_filesystem-mt
ASSIMP_CFLAGS = `pkg-config --cflags assimp`
ASSIMP_LIBS = `pkg-config --libs assimp`
GL_CFLAGS =
GL_LIBS = -lgdi32 -lwinmm -lopengl32

ALL_CFLAGS = $(CFLAGS) $(GLM_CFLAGS) $(JPEG_CFLAGS) $(PNG_CFLAGS) $(BOOST_CFLAGS) $(ASSIMP_CFLAGS) $(GLEW_CFLAGS) $(GLFW3_CFLAGS) $(GL_CFLAGS)
ALL_LIBS = $(LIBS) $(GLM_LIBS) $(JPEG_LIBS) $(PNG_LIBS) $(BOOST_LIBS) $(ASSIMP_LIBS) $(GLEW_LIBS) $(GLFW3_LIBS) $(GL_LIBS)

all: game

-include $(DEP)

%.o: %.cpp
	$(CXX) $(STD) $(INC) -c $(CPPFLAGS) $(DEPFLAGS) $(ALL_CFLAGS) $(DEFS) -o $@ $<

$(EXE): $(OBJ)
	$(CXX) $(LDFLAGS) $(OBJ) $(ALL_LIBS) -o $(EXE)

clean:
	-rm -rf $(OBJ) $(DEP) $(EXE)

.PHONY: all clean
