#include "opencv2/highgui/highgui.hpp"
#include <iostream>

using namespace std;
using namespace cv;
Mat img2 = imread("imagens/creditos2.png");
Mat img = imread("imagens/dorime2.png");
bool creditos;


void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
     if  ( event == EVENT_LBUTTONDOWN && x > 283 && x < 484 && y > 246 && y < 308 )
     {
          system("./dorime_game");
	
          waitKey(1);
     }
     else if  ( event == EVENT_LBUTTONDOWN && x > 283 && x < 484 && y > 359 && y < 472 )
     {
          //creditos
          imshow("Doribits", img2);
          creditos = true;
          
     }
     
     if  ( event == EVENT_LBUTTONDOWN && x > 53 && x < 132 && y > 490 && y < 520 && creditos == true)
     {
          imshow("Doribits", img);
          creditos = false;
          cout << "estou aqui\n";
          
     }
}

int main(int argc, char** argv)
{

     //if fail to read the image
     if ( img.empty() ) 
     { 
          cout << "Error loading the image" << endl;
          return -1; 
     }
    

     //Create a window
     namedWindow("Doribits", 5);

     //set the callback function for any mouse event
     setMouseCallback("Doribits", CallBackFunc, NULL);

     //show the image
     imshow("Doribits", img);

	
	

     // Wait until user press some key
     waitKey(0);

     return 0;

}
