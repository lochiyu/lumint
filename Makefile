CFLAGS = `pkg-config --cflags opencv` -O3 -funroll-loops
LIBS = `pkg-config --libs opencv`

all: lumint.cpp
	g++ lumint.cpp -o discreto $(CFLAGS) $(LIBS) -w -lrtmidi 
d2: lumint2.cpp
	g++ lumint2.cpp -o d2 $(CFLAGS) $(LIBS) -w -lrtmidi
test: miditest.cpp
	g++ miditest.cpp -o test $(CFLAGS) $(LIBS) -w -lrtmidi 
continuo: continuo.cpp
	g++ continuo.cpp -o continuo $(CFLAGS) $(LIBS) -w -lrtmidi 

speed:
clean:
	rm -f test discreto continuo d2
