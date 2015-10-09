/////////////////////////////////////////////////////////////////////////////////////////////////////
#include <RtMidi.h>
#include "opencv2/opencv.hpp"
#include <unistd.h>
#define SLEEP( milliseconds ) usleep( (unsigned long) (milliseconds * 1000.0) )
#include <iostream>


using namespace cv;
using namespace std;
int threshold_value = 0;
int threshold_type = 3;;
int const max_value = 255;
int const max_type = 4;
int const max_BINARY_value = 255;


//coordenadas de x y para el crop
//hay 4 lineas
//2 horizontales
//h1, h2
//2 verticales
//v1, v2
//cada uno tiene su x o su y
int h1_y=0;
int h2_y=400;
int v1_x=0;
int v2_x=400;
int H,W;
Point prev,curr;

char* trackbar_type = "Type: \n 0: Binary \n 1: Binary Inverted \n 2: Truncate \n 3: To Zero \n 4: To Zero Inverted";
char* trackbar_value = "Value";

char* window_name = "Threshold Demo";
Mat gray_image,dst, cropped;
Mat templ; //mancha blanca a buscar
Mat templ2; //mancha negra para el lumint
Mat result; //imagen con la mancha procesada
Mat proj; //imagen a proyectar en el edificio
Mat comparar;
int camara=0; //default es la camara de la compu

bool lumint=true;

bool negro(Mat roi);
void Threshold_Demo( int, void* );
void draw_lines(double dWidth, double dHeight);
bool chooseMidiPort( RtMidiOut *rtmidi );

void MyLine( Mat img, Point start, Point end ){
  int thickness = 2;
  int lineType = 8;
  line( img,
        start,
        end,
        Scalar( 255,255,255 ),
        thickness,
        lineType );
}


int main(int argc, char* argv[]){
	
	int option=0;
	if (argc>1){
		if (strcmp(argv[1],"webcam")==0){ // de lo contrario es pintar
			camara=1;
			cout<<argv[1]<<" activated"<<endl;
		}
	}
	
	
	//preparar la plantilla
	templ=imread("template2.png",1);
	namedWindow("template", CV_WINDOW_AUTOSIZE);imshow("template", templ);
	cvtColor( templ, templ, CV_BGR2GRAY );
	
	//plantilla para el lumint mancha oscura
	templ2=imread("template4.png",1);
	namedWindow("template2", CV_WINDOW_AUTOSIZE);imshow("template2", templ2);
	cvtColor( templ2, templ2, CV_BGR2GRAY );
	
	namedWindow("template encontrado", CV_WINDOW_AUTOSIZE);
	//preparar el proyector
	cvNamedWindow("edificio", CV_WINDOW_NORMAL);
	cvSetWindowProperty("edificio", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
	proj = Mat(400,720, CV_64F, cvScalar(0.));

	
	VideoCapture cap(camara); // open the video camera no. 0

	if (!cap.isOpened())  // if not success, exit program
	{
	cout << "Cannot open the video cam" << endl;
	return -1;
	}

	double dWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
	double dHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video

	h2_y=int(dHeight)-1;
	v2_x=int(dWidth)-1;
    cout << "Frame size : " << dWidth << " x " << dHeight << endl;

    while (1)
    {
        Mat frame;

        bool bSuccess = cap.read(frame); // read a new frame from video

         if (!bSuccess) //if not success, break loop
        {
             cout << "Cannot read a frame from video stream" << endl;
             break;
        }


		cvtColor( frame, gray_image, CV_BGR2GRAY );
		namedWindow( window_name, CV_WINDOW_AUTOSIZE );
		createTrackbar( trackbar_type,
                  window_name, &threshold_type,
                  max_type, Threshold_Demo );

		createTrackbar( trackbar_value,
                  window_name, &threshold_value,
                  max_value, Threshold_Demo );
                  
		Threshold_Demo(0,0);
		int key=waitKey(30);
		bool end=false;
		bool crop=false;
		switch(key){
			//teclas verticales
			case 113:
				cout << "q" << endl;
				//sube el h1, osea, le resta 1 pixel
				h1_y-=5;
				if (h1_y<0) h1_y=0;
				cout<<"v1_x="<<v1_x<<",h1_y="<<h1_y<<endl;
				break; 
			case 97:
				cout << "a" << endl;
				//baja el h1, osea, le suma 1 pixel
				h1_y+=5;
				if (h1_y>int(dHeight)) h1_y=int(dHeight);
				if (h2_y-h1_y<templ2.size().height) h1_y-=5;
				cout<<"v1_x="<<v1_x<<",h1_y="<<h1_y<<endl;
				break; 
			case 119:
				cout << "w" << endl;
				//sube el h2, osea, le resta 1 pixel
				h2_y-=5;
				if (h2_y<0) h2_y=0;
				if (h2_y-h1_y<templ2.size().height) h2_y+=5;
				cout<<"v2_x="<<v2_x<<",h2_y="<<h2_y<<endl;
				break; 
			case 115:
				cout << "s" << endl;
				//baja el h2, osea, le suma 1 pixel
				h2_y+=5;
				if (h2_y>int(dHeight)) h2_y=int(dHeight);
				cout<<"v2_x="<<v2_x<<",h2_y="<<h2_y<<endl;
				break; 
			//teclas horizontales
			case 101:
				cout << "e" << endl;
				//mueve a la izquierda el v1, osea, le resta 1 pixel
				v1_x-=5;
				if (v1_x<0) v1_x=0;
				cout<<"v1_x="<<v1_x<<",h1_y="<<h1_y<<endl;
				break; 
			case 114:
				cout << "r" << endl;
				//mueve a la derecha el v1, osea, le suma 1 pixel
				v1_x+=5;
				if (v1_x>int(dWidth)) v1_x=int(dWidth);
				cout<<"v1_x="<<v1_x<<",h1_y="<<h1_y<<endl;
				break; 
			case 100:
				cout << "d" << endl;
				//mueve a la izquierda el v2, osea, le resta 1 pixel
				v2_x-=5;
				if (v2_x<0) v2_x=0;
				cout<<"v2_x="<<v2_x<<",h2_y="<<h2_y<<endl;
				break; 
			case 102:
				cout << "f" << endl;
				//mueve a la derecha el v2, osea, le suma 1 pixel
				v2_x+=5;
				if (v2_x>int(dWidth)) v2_x=int(dWidth);
				cout<<"v2_x="<<v2_x<<",h2_y="<<h2_y<<endl;
				break; 
			case 99:
				cout << "Limpiando pantalla" << endl;
				proj = Mat(400,720, CV_64F, cvScalar(0.));
				break;
			case 112:
				cout << "tamaño del crop" << endl;
				cout<<W<<","<<H<<endl;
				break;
			case 27:
				cout << "esc key is pressed by user" << endl;
				end=true;
				break;

		}
		
		
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
		imshow("Gris", gray_image);
	
		///////////////////////// PATTERN MATCHING
		
		if ((cropped.depth() == CV_8U || cropped.depth() == CV_32F) && cropped.type() == templ.type()){
			if (lumint){
				matchTemplate(cropped, templ2, result, 0);
			}else{
				matchTemplate(cropped, templ, result, 0);
			}
			//tomado de http://docs.opencv.org/doc/tutorials/imgproc/histograms/template_matching/template_matching.html
			normalize( result, result, 0, 1, NORM_MINMAX, -1, Mat() );
			double minVal; double maxVal; Point minLoc; Point maxLoc;
			Point matchLoc;
			minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );
			
			matchLoc = minLoc;
			Size s = cropped.size();
			H = s.height;
			W = s.width;
			
			if (!lumint){
				rectangle( cropped, matchLoc, Point( matchLoc.x + templ.cols , matchLoc.y + templ.rows ), 		Scalar::all(0), 2, 8, 0 );
				curr=Point(720*matchLoc.x/W,400*matchLoc.y/H);
				circle(proj, curr, 1,Scalar(255,0,0),1,8,0);
				//line(proj, prev, curr,Scalar(255,0,0),1,8,0);  //no funciona bien
				prev=curr;
			}else if (lumint){
				Size s = proj.size();
				H = s.height;
				W = s.width;
				//line(proj,Point(100,H-50),Point(W-99,H-50),Scalar(255,255,255),3,8,0);
				Mat roi(cropped, Rect(matchLoc.x , matchLoc.y, templ2.cols , templ2.rows));
				
				imshow("template encontrado", roi);
				if (negro(roi)){ //encontré la mancha, mandar coordenada
					cout<<"detected:"<<matchLoc.x<<","<<matchLoc.y<<endl;
				}
				
				rectangle( cropped, matchLoc, Point( matchLoc.x + templ2.cols , matchLoc.y + templ2.rows ), Scalar(255,0,0), 2, 8, 0 );
				//		
			}
		}
		//PRINT COORDINATES
		
		//Image to projector
		imshow("cortado", cropped);

		imshow("edificio", proj);
    }//end while
    return 0;

}
bool negro(Mat roi){
	
	float threshold=0.8; //70% negro es suficiente
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
	/*DIAGONAL
	Point pt1 =Point (v1_x,h1_y);
	Point pt2 =Point (v2_x,h2_y);
	MyLine(gray_image,pt1,pt2);
	*/
	//INDIVIDUAL LINES
	//4 lines
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
void Threshold_Demo( int, void* ){
  /* 0: Binary
     1: Binary Inverted
     2: Threshold Truncated
     3: Threshold to Zero
     4: Threshold to Zero Inverted
   */

  threshold( gray_image, dst, threshold_value, max_BINARY_value,threshold_type );

  imshow( window_name, dst );
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////


