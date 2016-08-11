import processing.net.*; 

Client myClient; 
int dataIn; 
int x=0;
int num = 5000;
float[][] fire = new float [num][12];
int intensidad=0;
int ultimo;
int col=0;
int contador=0;
int ciclar=30;
int turno=0;
int contadorfx=0;
int numeroefectos=2;

void setup() { 
  size(1024, 800);
  noStroke();
  frameRate(60);
  rectMode(CENTER);
  smooth();
  background(0);
  translate(140, 0);
  // Connect to the local machine at port 5204.
  // This example will not run if you haven't
  // previously started a server on this port.
  myClient = new Client(this, "127.0.0.1", 5200); 
  
  flock = new Flock();
  
rcolores[0]=255;
gcolores[0]=255;
bcolores[0]=255;

rcolores[1]=0;
gcolores[1]=255;
bcolores[1]=0;

rcolores[2]=255;
gcolores[2]=0;
bcolores[2]=0;

rcolores[3]=0;
gcolores[3]=0;
bcolores[3]=255;

rcolores[4]=0;
gcolores[4]=255;
bcolores[4]=255;

rcolores[5]=255;
gcolores[5]=255;
bcolores[5]=0;

rcolores[6]=102;
gcolores[6]=0;
bcolores[6]=204;

rcolores[7]=204;
gcolores[7]=102;
bcolores[7]=255;

} 
 
void draw() { 
  background(0);
  if (myClient.available() > 0) { 
    dataIn = myClient.read(); 
    //println(dataIn);
    
    if(dataIn<10){ //es una nota de la octava
      contador++;
      if (contador%ciclar==0){
        //hora de cambiar de efecto
        contadorfx++;
        println("Cambiando de efecto");
        if (contadorfx==numeroefectos) {
          contadorfx=0;
        }
      }
      if (contadorfx==1) fuego();
      if (contadorfx==0) dibujarBoid();
    }
  }
  
  //mantenimiento de fuego
  update_fire(); 
  draw_fire();
  
  //mantenimiento de aves
  flock.run();

} 

void fuego(){
      for(int i=0; i<100; i++)
      {
        x=dataIn;
        if (ultimo==dataIn) intensidad+=5;
        else {
          intensidad=0;
          col++;
        }
        ultimo=dataIn;
        create_fire();
      }  
}