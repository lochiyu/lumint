import processing.net.*; 

Client myClient; 
int dataIn; 
int x=0;
int num = 5000;
float[][] fire = new float [num][12];
int intensidad=0;
int ultimo;
int col=0;

void setup() { 
  size(640, 360);
  noStroke();
  frameRate(60);
  rectMode(CENTER);
  smooth();
  background(0);
  translate(140, 0);
  // Connect to the local machine at port 5204.
  // This example will not run if you haven't
  // previously started a server on this port.
  myClient = new Client(this, "127.0.0.1", 5204); 
} 
 
void draw() { 
  background(0);
  if (myClient.available() > 0) { 
    dataIn = myClient.read(); 
    println(dataIn);
    if(dataIn<10){
      for(int i=0; i<50; i++)
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
  }
  update_fire(); 
  draw_fire();

} 

void update_fire()
{
  for(int flame=0 ; flame<num ; flame++)
  {
    if(fire[flame][0]==1)
    {
      fire[flame][1]=fire[flame][1]+fire[flame][5]*cos(fire[flame][3]);
      fire[flame][2]=fire[flame][2]+fire[flame][5]*sin(fire[flame][3]);
    }
    fire[flame][7]+=1;
    if(fire[flame][7]>fire[flame][6]){
      fire[flame][0]=0;
    }
  }
}
void draw_fire(){
  for(int flame=0 ; flame<num ; flame++){
    if(fire[flame][0]==1){
      fill(fire[flame][9],fire[flame][10],fire[flame][11],40);
      pushMatrix();
      translate(fire[flame][1],fire[flame][2]);
      rotate(fire[flame][8]);
      rect(0,0,fire[flame][4],fire[flame][4]);
      popMatrix();
    }
  }
}
void create_fire()
  {
    println((col%3)*100);
  for(int i=num-1; i>0; i--)
  {
    for(int fireprop=0;fireprop<12;fireprop++)
    {
      fire[i][fireprop]=fire[i-1][fireprop];
    }
    
    fire[0][0]=1;
    fire[0][1]=x*50;
    fire[0][2]=330;
    fire[0][3]=-random(0,PI);//angle
    fire[0][4]=random(5,10);//size
    fire[0][5]=random(1,2);//speed
    fire[0][6]=random(10,60+intensidad);//maxlife
    fire[0][7]=0;//currentlife
    fire[0][8]=random(0,TWO_PI);
    fire[0][9]=random(200,255);//red
    fire[0][10]=random(50,150)*((col%3)*50);//green
    fire[0][11]=(col%3)*110;
   }
 } 