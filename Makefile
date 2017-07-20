FILE=solar

CFLAGS = -lGL -lGLU -lglut -lGLEW

solar: $(FILE).cpp
	g++ $(CFLAGS) $(FILE).cpp -o $(FILE)

clean:
	rm $(FILE)
