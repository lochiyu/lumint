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
    //println((col%3)*100);
  for(int i=num-1; i>0; i--)
  {
    for(int fireprop=0;fireprop<12;fireprop++)
    {
      fire[i][fireprop]=fire[i-1][fireprop];
    }
    
    fire[0][0]=1;
    fire[0][1]=x*200;
    fire[0][2]=800;
    fire[0][3]=-random(0,PI);//angle
    fire[0][4]=random(5,10);//size
    fire[0][5]=random(1,3);//speed
    fire[0][6]=random(10,80+intensidad);//maxlife
    fire[0][7]=0;//currentlife
    fire[0][8]=random(0,TWO_PI);
    fire[0][9]=random(200,255);//red
    fire[0][10]=random(50,150)*((col%3)*50);//green
    fire[0][11]=(col%3)*110;
   }
 } 