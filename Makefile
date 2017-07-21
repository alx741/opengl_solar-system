FILE=solar

CFLAGS = -I/usr/include -lGL -lGLU -lglut -lGLEW -std=c++14

solar: $(FILE).cpp
	g++ $(CFLAGS) $(FILE).cpp -o $(FILE)

clean:
	rm $(FILE)
