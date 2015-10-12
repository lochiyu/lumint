CFLAGS = `pkg-config --cflags opencv`
LIBS = `pkg-config --libs opencv`

all:
	g++ lumint.cpp -o out $(CFLAGS) $(LIBS) -w -lrtmidi 
test:
	g++ miditest.cpp -o test $(CFLAGS) $(LIBS) -w -lrtmidi 

speed:
	g++ lumint.cpp -o out $(CFLAGS) $(LIBS) -w -lrtmidi -O3
tclean:
	rm -f test out core
