CXX = g++
LDLIBS = -lsfml-graphics # -lsfml-window -lsfml-system

all:
	$(CXX) recognizer.cpp  $^ $(LDLIBS) -o recognizer