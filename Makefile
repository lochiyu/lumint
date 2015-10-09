CFLAGS = `pkg-config --cflags opencv`
LIBS = `pkg-config --libs opencv`

all:
	g++ lumint.cpp -o lumint $(CFLAGS) $(LIBS) -w

	
