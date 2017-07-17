FILE=sphere

CFLAGS =
CFLAGS += -lGL -lGLU -lglut -lGLEW -D _DEBUG

gasket: $(FILE).cpp
	g++ -lGL -lGLU -lglut -lGLEW $(FILE).cpp -o $(FILE)

clean:
	rm $(FILE)
