/////////////////////////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <RtMidi.h>
#include "opencv2/opencv.hpp"
#include <unistd.h>
#include <iostream>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#define TOLERANCIA 0.001
using namespace cv;
using namespace std;
int threshold_value = 0;
int threshold_type = 3;
int tipo_modalidad = 0;
int const max_value = 255;
int const max_type = 4;
int const max_BINARY_value = 255;
bool processing=false;
bool cromatica=false;

//coordenadas de x y para el crop
//hay 4 lineas:
//2 horizontales :h1, h2
//2 verticales :v1, v2
//cada uno tiene su x o su y
int h1_y=0;
int h2_y=400;
int v1_x=0;
int v2_x=400;
int Hcropped, Wcropped;

char* modalidad = "detección de oscuridad \ndetección de linea";
char* trackbar_value = "Umbral: O para -, P para +";

char* window_name = "Panel de Control";
char n[5];


Mat gray_image,dst, cropped;
Mat nota(470,820,CV_8UC3, Scalar(0,0,0));
Mat templ2; //plantilla para el lumint
Mat result; //imagen con la mancha procesada
int camara=0; //default es la camara de la compu

bool haylinea(Mat roi);
bool negro(Mat roi);
void Threshold_Demo( int, void* );
void cambio_modalidad( int, void* );
void draw_lines(double dWidth, double dHeight);
bool isInteger(const std::string & s);

//relacionado con MIDI
RtMidiOut *midiout;
int xviejo;
bool cont=false;
bool linea=false;
void init_midi();
void play(int nota);
void play_bend(int nota,int msb, int lsb, bool nueva);
void callar(int nota);
std::vector<unsigned char> message;
int semitonos=15;//en los que se va a dividir
void dibujar_semitonos(int numero, int ancho, int alto);
void tocar_nota(int x, int ancho);
void continuo(int x, int ancho, bool nuevo);
int notas_midi[15]={60,62,64,65,67,69,71,72,74,76,77,79,81,83,84};//escala inicial por defecto
int nota_actual;
void aumentar_octava();
void disminuir_octava();
void callar_todo();
void aumentar_escala();
void disminuir_escala();
string imprimir_nota(int nota_midi);
void imprimir_escala_actual();
void fijarNota(int nota);
//fin relacionado con MIDI

//relacionado con openCV
void MyLine( Mat img, Point start, Point end );
void inicializar_pantallas(void);
void scroll(void);
//fin relacionado con openCV

int sockfd, newsockfd, portno;
char * buf = new char(1);
void leer_teclas(int tecla,bool &bandera, double &dWidth, double &dHeight);

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

void abrir(int puerto){
	socklen_t clilen;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
	error("ERROR opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = puerto;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
	      sizeof(serv_addr)) < 0) 
	      error("ERROR on binding");
	listen(sockfd,5);
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd, 
		 (struct sockaddr *) &cli_addr, 
		 &clilen);
	if (newsockfd < 0) 
	  error("ERROR on accept");
}

void enviar(int xpos){
	int n;
	buf[0]=xpos;
	n = write(newsockfd,buf,1);
	if (n < 0) error("ERROR writing to socket");
}

void cerrar(){
	close(newsockfd);
	close(sockfd);

}
int main(int argc, char* argv[]){

	init_midi();
	for (int j=0;j<argc;j++){
		if (strcmp(argv[j],"help")==0){
			cout<<"opciones: "<<endl;
			cout<<"  webcam: si hay otra cámara, aveces se enreda entre la de usb y del laptop"<<endl;
			cout<<"  continuo: modo continuo, hay pulgas pues toca pocos semitonos"<<endl;
			cout<<"  número: fija el número de notas totals, siendo el máximo 15"<<endl;
			cout<<"  cromatica: fija una escala cromatica y no diatonica"<<endl;
			cout<<endl<<"Ejemplo: ./d2 12 noscroll     <--causa 12 notas máximo, sin scroll"<<endl;
			return 0;
		}
		if (strcmp(argv[j],"cromatica")==0){ // quiero una escala cromatica
			notas_midi[0]=60;
			notas_midi[1]=61;
			notas_midi[2]=62;
			notas_midi[3]=63;
			notas_midi[4]=64;
			notas_midi[5]=65;
			notas_midi[6]=66;
			notas_midi[7]=67;
			notas_midi[8]=68;
			notas_midi[9]=69;
			notas_midi[10]=70;
			notas_midi[11]=71;
			notas_midi[12]=72;
			notas_midi[13]=73;
			notas_midi[14]=74;
			cromatica=true;
			cout<<argv[j]<<" activada"<<endl;
		}
		if (strcmp(argv[j],"webcam")==0){ // de lo contrario es pintar
			camara=1;//usaremos la webcam, siempre es bueno tener dos opciones
			cout<<argv[j]<<" activada"<<endl;
		}
		if (strcmp(argv[j],"linea")==0){ // deteccion de linea
			linea=true;//usaremos el modo continuo
			cout<<"modo linea activado"<<endl;
		}
		if (strcmp(argv[j],"continuo")==0){ // de lo contrario es pintar
			cont=true;//usaremos el modo continuo
			cout<<"modo continuo activado"<<endl;
		}
		if (strcmp(argv[j],"fx")==0){ // no habrá efectos de processing
			processing=true;
			abrir(5204);
			cout<<"animación de processing desactivada"<<endl;
		}
		if (isInteger(argv[j])){
			semitonos=atoi(argv[j]);
			if (semitonos>15){
				cout<<"El máximo de notas es 15"<<endl;
			}else{
				cout<<"Cambiando el número de notas a "<<semitonos<<endl;
			}
		}
	}//end for
	inicializar_pantallas();//ventanas de openCV

	VideoCapture cap(camara); // open the video camera no. 0
	if (!cap.isOpened())  // if not success, exit program
	{
		cout << "Cannot open the video cam" << endl;
		return -1;
	}

	cap.set(CV_CAP_PROP_FRAME_WIDTH, 176);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 144);
	double dWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
	double dHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video

	h2_y=int(dHeight)-1;
	v2_x=int(dWidth)-1;
	cout << "Frame size : " << dWidth << " x " << dHeight << endl;
	
	namedWindow( window_name, CV_WINDOW_NORMAL );

	Mat frame;
	//loop principal
	while (1)
	{
		bool bSuccess = cap.read(frame); // read a new frame from video

		if (!bSuccess) //if not success, break loop
		{
		     cout << "Cannot read a frame from video stream" << endl;
		     break;
		}

		cvtColor( frame, gray_image, CV_BGR2GRAY );
		createTrackbar( modalidad,window_name, &tipo_modalidad,1, cambio_modalidad );
		createTrackbar( trackbar_value,window_name, &threshold_value,max_value, Threshold_Demo );

		//opciones para cambiar configuración
		  
		Threshold_Demo(0,0);
		int key=waitKey(30);
		bool end=false;
		leer_teclas(key,end,dWidth,dHeight);
		
		if (end) break;

		cv::Mat source = dst;
		// Setup a rectangle to define your region of interest
		//cout<<v1_x<<","<< h1_y<<","<< v2_x<<","<< h2_y<<endl;
		cv::Rect myROI(v1_x, h1_y, v2_x-v1_x, h2_y-h1_y);
		// Crop the full image to that image contained by the rectangle myROI
		// Note that this doesn't copy the data
		cv::Mat croppedRef(source, myROI);
		// Copy the data into new matrix
		croppedRef.copyTo(cropped);
		///////////////////////// DRAW LINES

		draw_lines(dWidth, dHeight);

		///////////////////////// PATTERN MATCHING
		
		if ((cropped.depth() == CV_8U || cropped.depth() == CV_32F) && cropped.type() == templ2.type()){
			matchTemplate(cropped, templ2, result, 0);
			//tomado de http://docs.opencv.org/doc/tutorials/imgproc/histograms/template_matching/template_matching.html
			normalize( result, result, 0, 1, NORM_MINMAX, -1, Mat() );
			double minVal; double maxVal; Point minLoc; Point maxLoc;
			Point matchLoc;
			minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );
			
			matchLoc = minLoc;
			Size s = cropped.size();
			Hcropped = s.height;
			Wcropped = s.width;
			//dibujar las líneas para mostrar las fronteras de los tonos discretos
			//if (!cont) dibujar_semitonos(semitonos,Wcropped,Hcropped);
			if (!cont) dibujar_semitonos(semitonos,v2_x-v1_x,gray_image.size().height);

			Mat roi(cropped, Rect(matchLoc.x , matchLoc.y, templ2.cols , templ2.rows));
			
			imshow("template encontrado", roi);
			if (negro(roi)&&!linea || (linea&&haylinea(roi))){ //encontré la mancha, mandar coordenada
				//cout<<Wcropped<<" ";
				//cout<<"detected:"<<matchLoc.x<<","<<matchLoc.y<<endl;
				tocar_nota(matchLoc.x,Wcropped);
				//muestra la nota actual
				putText(nota, n, Point2f(10,430), FONT_HERSHEY_DUPLEX, 18, Scalar(255,255,255,255),7);
			}else{ //no se encontro nada, callar
				//cout<<"callando"<<endl;
				callar(notas_midi[nota_actual]);
				nota=Scalar(0,0,0);
				nota_actual=-1;
			}
			
			rectangle( cropped, matchLoc, Point( matchLoc.x + templ2.cols , matchLoc.y + templ2.rows ), Scalar(255,0,0), 2, 8, 0 );
			rectangle( gray_image, Point(matchLoc.x+v1_x,matchLoc.y+h1_y), Point( matchLoc.x + templ2.cols+v1_x , matchLoc.y + templ2.rows +h1_y), Scalar(255,0,0), 2, 8, 0 );
		}
		//Image to projector
		imshow("cortado", cropped);

		resize(gray_image,gray_image,Size(1280,400));
		imshow("Gris", gray_image);
		
		imshow("Nota actual", nota);
			
	}//end while
	callar_todo();
	cerrar();
	delete midiout;
	return 0;

}//end main
//////////////////////////////////////////////////////////////////////////////////////////////////////////
bool haylinea(Mat roi){
	//float threshold=0.36; //cuánto negro es suficiente
	float threshold=0.02;
	Size s=roi.size();
	float numero=0;
	for (int i=0;i<s.height;i++){
		for (int j=0;j<s.width;j++){
			uchar item=roi.at<uchar>(i,j);
			if (item>200 ) numero++;
			//cout<<(int)item<<",";
		}
		//cout<<endl;
	}
//	cout<<"  "<<numero<<" de "<< s.width*s.height<<endl;
//	cout<<roi<<endl;
	if (numero/float(s.width*s.height)>threshold) return true;
	return false;
}

bool negro(Mat roi){
	float threshold=0.96; //cuánto negro es suficiente
	Size s=roi.size();
	float numero=0;
	for (int i=0;i<s.height;i++){
		for (int j=0;j<s.width;j++){
			uchar item=roi.at<uchar>(i,j);
			if ((item<1 && item >-1) || (item !=item)) numero++;
			//cout<<item<<",";
		}
		//cout<<endl;
	}
	//cout<<"  "<<numero<<" de "<< s.width*s.height<<endl;
	//cout<<roi<<endl;
	if (numero/float(s.width*s.height)>threshold) return true;
	return false;
}

void draw_lines(double dWidth, double dHeight){
	//INDIVIDUAL LINES
	//horizontal 1
	Point p1 = Point (0,h1_y);
	Point p2 = Point (dWidth, h1_y);
	MyLine(gray_image,p1,p2);
	//horizontal 2
	p1 = Point (0,h2_y);
	p2 = Point (dWidth, h2_y);
	MyLine(gray_image,p1,p2);
	//vertical 1
	p1 = Point (v1_x,0);
	p2 = Point (v1_x,dHeight);
	MyLine(gray_image,p1,p2);
	//horizontal 2
	p1 = Point (v2_x,0);
	p2 = Point (v2_x,dHeight);
	MyLine(gray_image,p1,p2);
	
}

void dibujar_semitonos(int numero, int ancho, int alto){
	int x;
	int i=0;
	//dibuja las lineas
	Point p1,p2;
	char m[5];
	for (x=v1_x;x<ancho+v1_x;x+=ancho/numero){
		p1=Point(x,0);
		p2=Point(x,alto);
		//MyLine(cropped,p1,p2);//cropped es para la imagen pequeña
		MyLine(gray_image,p1,p2);
		//escribe las notas en gray_image
		string nota=imprimir_nota(notas_midi[i++]);	
		strcpy(m,nota.c_str());
		putText(gray_image, m, Point(x+1,alto-1), FONT_HERSHEY_PLAIN, 1, Scalar(255,255,255,255),1);
	}
	
}

void Threshold_Demo( int, void* ){
	  threshold( gray_image, dst, threshold_value, max_BINARY_value,threshold_type );
	  imshow( window_name, dst );
}


void init_midi()
{
	  midiout = new RtMidiOut();
	  // Check available ports.
	  unsigned int nPorts = midiout->getPortCount();
	  if ( nPorts == 0 ) {
	    std::cout << "No ports available!\n";
	    goto cleanup;
	  }
	  cout<<"Inicializando puerto MIDI"<<endl;
	  // Open first available port.
	  midiout->openPort( 0 );
	  // Send out a series of MIDI messages.
	  // Program change: 192, 5
	  message.push_back( 192 );
	  message.push_back( 5 );
	  midiout->sendMessage( &message );
	  // Control Change: 176, 7, 100 (volume)
	  message[0] = 176;
	  message[1] = 7;
	  message.push_back( 127 );
	  midiout->sendMessage( &message );
	  return;
	 cleanup:
	  delete midiout;
}

void play(int lanota){
	message[0] = 144;//encender nota en canal 1
	message[1] = lanota;
	message[2] = 90;
	midiout->sendMessage( &message );
	cout<<"tocando la nota "<<lanota<<"  "<<endl;
	fijarNota(lanota);
}
void fijarNota(int nota){
	switch(nota%12){
		case 0:
			sprintf(n,"C");
			break;	
		case 1:
			sprintf(n,"C#");
			break;	
		case 2:
			sprintf(n,"D");
			break;	
		case 3:
			sprintf(n,"D#");
			break;	
		case 4:
			sprintf(n,"E");
			break;	
		case 5:
			sprintf(n,"F");
			break;	
		case 6:
			sprintf(n,"F#");
			break;	
		case 7:
			sprintf(n,"G");
			break;	
		case 8:
			sprintf(n,"G#");
			break;	
		case 9:
			sprintf(n,"A");
			break;	
		case 10:
			sprintf(n,"A#");
			break;	
		case 11:
			sprintf(n,"B");
			break;	
	}

}

void play_bend(int nota,int msb, int lsb, bool nueva){
	//primero encender la nota en canal 1
	nota=notas_midi[nota];
	message[0] = 144;
	message[1] = nota;
	message[2] = 90;
	if (1){
		midiout->sendMessage( &message ); //solo tocar de nuevo si es nueva, de lo contrario, solo hacer el bend
		cout<<" notabend nueva"<<endl;
	}
	//hacer nota en canal 1 con bend
	message[0] = 224;
	message[1] = lsb; //7 LSB
	message[2] = msb; //7 MSB, recordar que 0x2000 es la nota pura sin bend
	midiout->sendMessage( &message );
	cout<<"tocando la nota con bend:"<<nota<<"  "<<msb<<":"<<lsb<<endl;
}

void callar(int lanota){
  	message[0] = 128;//apagar nota en canal 1
  	message[1] = lanota;
  	message[2] = 40;
  	midiout->sendMessage( &message );
	//cout<<"callando la nota "<<nota<<"  "<<endl;
	nota=Scalar(0,0,0);
	sprintf(n,"-");
}

void MyLine( Mat img, Point start, Point end ){
  int thickness = 2;
  int lineType = 8;
  line( img, start, end, Scalar( 255,255,255 ), thickness, lineType );
}


void inicializar_pantallas(void){
	
	//plantilla para el lumint mancha oscura
	if (!linea){
		templ2=imread("template4.png",1);
	}else{//buscar la linea
		templ2=imread("punto.png",1);
	}

	//imshow("template2", templ2);
	cvtColor( templ2, templ2, CV_BGR2GRAY );
	
}
void leer_teclas(int tecla, bool &bandera, double &dWidth, double &dHeight){
	switch(tecla){
		//teclas verticales
		case 113:
			//cout << "q" << endl;
			//sube el h1, osea, le resta 1 pixel
			h1_y-=5;
			if (h1_y<0) h1_y=0;
			//cout<<"v1_x="<<v1_x<<",h1_y="<<h1_y<<endl;
			break; 
		case 97:
			//cout << "a" << endl;
			//baja el h1, osea, le suma 1 pixel
			h1_y+=5;
			if (h1_y>int(dHeight)) h1_y=int(dHeight);
			if (h2_y-h1_y<templ2.size().height) h1_y-=5;
			//cout<<"v1_x="<<v1_x<<",h1_y="<<h1_y<<endl;
			break; 
		case 119:
	//		cout << "w" << endl;
			//sube el h2, osea, le resta 1 pixel
			h2_y-=5;
			if (h2_y<0) h2_y=0;
			if (h2_y-h1_y<templ2.size().height) h2_y+=5;
			//cout<<"v2_x="<<v2_x<<",h2_y="<<h2_y<<endl;
			break; 
		case 115:
			//cout << "s" << endl;
			//baja el h2, osea, le suma 1 pixel
			h2_y+=5;
			if (h2_y>int(dHeight)) h2_y=int(dHeight);
			//cout<<"v2_x="<<v2_x<<",h2_y="<<h2_y<<endl;
			break; 
		//teclas horizontales
		case 101:
			//cout << "e" << endl;
			//mueve a la izquierda el v1, osea, le resta 1 pixel
			v1_x-=5;
			if (v1_x<0) v1_x=0;
			//cout<<"v1_x="<<v1_x<<",h1_y="<<h1_y<<endl;
			break; 
		case 114:
			//cout << "r" << endl;
			//mueve a la derecha el v1, osea, le suma 1 pixel
			v1_x+=5;
			if (v1_x>int(dWidth)) v1_x=int(dWidth);
			//cout<<"v1_x="<<v1_x<<",h1_y="<<h1_y<<endl;
			break; 
		case 100:
			//cout << "d" << endl;
			//mueve a la izquierda el v2, osea, le resta 1 pixel
			v2_x-=5;
			if (v2_x<0) v2_x=0;
			//cout<<"v2_x="<<v2_x<<",h2_y="<<h2_y<<endl;
			break; 
		case 102:
			//cout << "f" << endl;
			//mueve a la derecha el v2, osea, le suma 1 pixel
			v2_x+=5;
			if (v2_x>int(dWidth)) v2_x=int(dWidth);
			//cout<<"v2_x="<<v2_x<<",h2_y="<<h2_y<<endl;
			break; 
		case 27:
			cout << "Se apretó el ESC, saliendo del programa" << endl;
			bandera=true;
			break;
		case 43: //tecla +
			aumentar_octava();
			cout<< "Incrementando una octava"<< endl;
			break;
		case 45: //tecla -
			disminuir_octava();
			cout<< "Decrementando una octava"<< endl;
			break;
		case 109: //tecla m
			aumentar_escala();
			cout<< "Incrementando escala en un semitono"<<endl;
			break;
		case 110: //tecla n
			disminuir_escala();
			cout<< "Decrementando escala en un semitono"<<endl;
			break;
		case 32: //barra espaciadora
			imprimir_escala_actual();
			callar_todo();
			break;
		case 112: //subir umbral
			threshold_value++;
			break;
		case 111: //bajar umbral
			threshold_value--;
			break;
	}//end switch
}
void tocar_nota(int x, int ancho){
	int ancho_nota=ancho/semitonos;
	int num_nota=x/ancho_nota;
	if (processing) enviar(num_nota);
	//si es continuo, ejecutar otra cosa
	if (cont) {
		//primero, ver si es la misma nota.  Si la es, revisar si cambio suficiente
		float tol=(float)x-(float)xviejo;
		tol=abs(tol)/xviejo;
		cout<<num_nota<<","<<x<<"  dif="<<tol<<endl;
		if (nota_actual!=num_nota){ 
			//no es la misma nota
			cout<<"nota nueva"<<endl;
			callar(notas_midi[nota_actual]);
			continuo(x,ancho,true);//true es que es una nota nueva
			nota_actual=num_nota;
			xviejo=x;
		}else{
			//es la misma nota, revisar si el bend cambió más que TOLERANCIA
			float tol=(float)x-(float)xviejo;
			tol=abs(tol)/xviejo;
			if (tol>TOLERANCIA){
				cout<<"bend   ";
				callar(notas_midi[nota_actual]);
				continuo(x,ancho,false);//es nota vieja, solo cambiar el bend
				xviejo=x;
			}else{
				//la diferencia en x no es tanta, siga con la misma nota
				//cout<<"misma nota"<<endl;
			}
		}
	}else{ //es discreto
		//tocar la nota
		if (nota_actual!=num_nota){
			//cout<<"--"<<x<<"--";
			callar(notas_midi[nota_actual]);
			play(notas_midi[num_nota]);
			nota_actual=num_nota;
		}
	}
}
void aumentar_octava(){
	callar_todo();
	int i;
	if (notas_midi[1]>109) return; //máxima octava
	for (i=0;i<semitonos;i++){
		notas_midi[i]+=12;  //aumente una octava
	}
	system("clear");
}
void disminuir_octava(){
	callar_todo();
	int i;
	if (notas_midi[1]<12) return; //mínima octava
	for (i=0;i<semitonos;i++){
		notas_midi[i]-=12;  //disminuya una octava
	}
	system("clear");
}
void callar_todo(){
	for (int i=0;i<semitonos;i++){ callar(notas_midi[i]);}
}
void aumentar_escala(){
	system("clear");
	callar_todo();
	if (notas_midi[0]!=127){
		notas_midi[0]=notas_midi[0]+1; //aumento un semitono
	}
	cout<<"Escala en "<<imprimir_nota(notas_midi[0])<<" mayor"<<endl;
	//incremento las demás según la primera
	//escala mayor nada más
	if (cromatica!=true){//diatonica
		notas_midi[1]=notas_midi[0]+2;
		notas_midi[2]=notas_midi[1]+2;
		notas_midi[3]=notas_midi[2]+1;
		notas_midi[4]=notas_midi[3]+2;
		notas_midi[5]=notas_midi[4]+2;
		notas_midi[6]=notas_midi[5]+2;
		notas_midi[7]=notas_midi[6]+1;
		notas_midi[8]=notas_midi[7]+2;
		notas_midi[9]=notas_midi[8]+2;
		notas_midi[10]=notas_midi[9]+1;
		notas_midi[11]=notas_midi[10]+2;
		notas_midi[12]=notas_midi[11]+2;
		notas_midi[13]=notas_midi[12]+2;
		notas_midi[14]=notas_midi[13]+1;
	}else{ //es escala cromatica
		notas_midi[1]=notas_midi[0]+1;
		notas_midi[2]=notas_midi[1]+1;
		notas_midi[3]=notas_midi[2]+1;
		notas_midi[4]=notas_midi[3]+1;
		notas_midi[5]=notas_midi[4]+1;
		notas_midi[6]=notas_midi[5]+1;
		notas_midi[7]=notas_midi[6]+1;
		notas_midi[8]=notas_midi[7]+1;
		notas_midi[9]=notas_midi[8]+1;
		notas_midi[10]=notas_midi[9]+1;
		notas_midi[11]=notas_midi[10]+1;
		notas_midi[12]=notas_midi[11]+1;
		notas_midi[13]=notas_midi[12]+1;
		notas_midi[14]=notas_midi[13]+1;
	}
}
void disminuir_escala(){
	system("clear");
	callar_todo();
	if (notas_midi[0]!=0){
		notas_midi[0]=notas_midi[0]-1; //aumento un semitono
	}
	cout<<"Escala en "<<imprimir_nota(notas_midi[0])<<" mayor"<<endl;
	//incremento las demás según la primera
	//escala mayor nada más
	if (cromatica!=true){//diatonica
		notas_midi[1]=notas_midi[0]+2;
		notas_midi[2]=notas_midi[1]+2;
		notas_midi[3]=notas_midi[2]+1;
		notas_midi[4]=notas_midi[3]+2;
		notas_midi[5]=notas_midi[4]+2;
		notas_midi[6]=notas_midi[5]+2;
		notas_midi[7]=notas_midi[6]+1;
		notas_midi[8]=notas_midi[7]+2;
		notas_midi[9]=notas_midi[8]+2;
		notas_midi[10]=notas_midi[9]+1;
		notas_midi[11]=notas_midi[10]+2;
		notas_midi[12]=notas_midi[11]+2;
		notas_midi[13]=notas_midi[12]+2;
		notas_midi[14]=notas_midi[13]+1;
	}else{ //es escala cromatica
		notas_midi[1]=notas_midi[0]+1;
		notas_midi[2]=notas_midi[1]+1;
		notas_midi[3]=notas_midi[2]+1;
		notas_midi[4]=notas_midi[3]+1;
		notas_midi[5]=notas_midi[4]+1;
		notas_midi[6]=notas_midi[5]+1;
		notas_midi[7]=notas_midi[6]+1;
		notas_midi[8]=notas_midi[7]+1;
		notas_midi[9]=notas_midi[8]+1;
		notas_midi[10]=notas_midi[9]+1;
		notas_midi[11]=notas_midi[10]+1;
		notas_midi[12]=notas_midi[11]+1;
		notas_midi[13]=notas_midi[12]+1;
		notas_midi[14]=notas_midi[13]+1;
	}
}
string imprimir_nota(int nota_midi){
	switch(nota_midi%12){
		case 0: return "C";
		case 1: return "C#";
		case 2: return "D";
		case 3: return "D#";
		case 4: return "E";
		case 5: return "F";
		case 6: return "F#";
		case 7: return "G";
		case 8: return "G#";
		case 9: return "A";
		case 10: return "A#";
		case 11: return "B";
	}//end switch
}
void imprimir_escala_actual(){
	system("clear");
	cout<<"Escala en "<<imprimir_nota(notas_midi[0])<<" mayor"<<endl;
}
void continuo(int x, int ancho, bool nueva){
	int ancho_nota=ancho/semitonos;
	int nota_central=x/ancho_nota;
	int residuo=x%ancho_nota;
	float porcentaje=(float) residuo/ (float) ancho_nota;
	int bend=porcentaje*16384; //el bend toma 14 bits.  8192 es la nota pura
	int lsb=bend%128;
	int msb=bend/128;
	//cout<<nota_central<<"="<<porcentaje<<"%"<<","<<bend<<"="<<msb<<"-"<<lsb<<endl;
	play_bend(nota_central,msb,lsb,nueva);
}
bool isInteger(const std::string & s)
{
   if(s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false ;

   char * p ;
   strtol(s.c_str(), &p, 10) ;

   return (*p == 0) ;
}

void cambio_modalidad( int, void* ){
	//cambia de detección de oscuridad a detección de un punto luminoso
	linea=!linea;
	if (linea){
		cout<<"deteccion de linea"<<endl;
	}else{
		cout<<"deteccion de oscuridad"<<endl;
	}
	inicializar_pantallas();
}
