FILE=solar

CFLAGS = -lGL -lGLU -lglut -lGLEW

gasket: $(FILE).cpp
	g++ $(CFLAGS) $(FILE).cpp -o $(FILE)

clean:
	rm $(FILE)
