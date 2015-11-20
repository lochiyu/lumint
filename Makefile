CFLAGS = `pkg-config --cflags opencv`
LIBS = `pkg-config --libs opencv`

all: lumint.cpp
	g++ lumint.cpp -o discreto $(CFLAGS) $(LIBS) -w -lrtmidi 
test: miditest.cpp
	g++ miditest.cpp -o test $(CFLAGS) $(LIBS) -w -lrtmidi 
continuo: continuo.cpp
	g++ continuo.cpp -o continuo $(CFLAGS) $(LIBS) -w -lrtmidi 

speed:
	g++ lumint.cpp -o out $(CFLAGS) $(LIBS) -w -lrtmidi -O3
clean:
	rm -f test discreto continuo
