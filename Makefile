CFLAGS = `pkg-config --cflags opencv`
LIBS = `pkg-config --libs opencv`

all:
	g++ lumint.cpp -o out $(CFLAGS) $(LIBS) -w -lrtmidi -g
test:
	g++ miditest.cpp -o test $(CFLAGS) $(LIBS) -w -lrtmidi -g

clean:
	rm -f test out core
