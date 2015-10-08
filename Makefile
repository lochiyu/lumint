all:
	g++ captura_imagen.cpp -o lumint -lopencv_core -lopencv_imgproc -lopencv_objdetect -lopencv_highgui `sdl2-config --cflags --libs` -w -lrtmidi 
	
gen:	lumint2.cpp
	g++ lumint2.cpp -o sonidos `sdl2-config --cflags --libs`
	
