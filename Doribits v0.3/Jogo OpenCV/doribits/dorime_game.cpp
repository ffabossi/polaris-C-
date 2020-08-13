#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <fstream>


using namespace std;
using namespace cv;



 Mat img2;
 int recorde = 0;

void detectAndDraw( Mat img, CascadeClassifier& cascade,
                    CascadeClassifier& nestedCascade,
                    double scale);

int lerArquivo(){
    ifstream file;
    int pontuacao;

    file.open("pontuacao.txt");

    if(!file.is_open()){
        std::cout << "Nao foi possivel abrir o arquivo.";
        return 0;
    }
    
    file >> pontuacao;
    
    file.close();
    return pontuacao;
}

void salvaArquivo(int p){
    ofstream file;

    file.open("pontuacao.txt");

    if(!file.is_open()){
        std::cout << "Nao foi possivel abrir o arquivo.";
        return ;
    }
    
    
    file << p;
    
    file.close();
}
string cascadeName;
string nestedCascadeName;
Mat dorimesafe;
Mat pedra;
/**
 * @brief Draws a transparent image over a frame Mat.
 * 
 * @param frame the frame where the transparent image will be drawn
 * @param transp the Mat image with transparency, read from a PNG image, with the IMREAD_UNCHANGED flag
 * @param xPos x position of the frame image where the image will start.
 * @param yPos y position of the frame image where the image will start.
 */
void drawTransparency(Mat frame, Mat transp, int xPos, int yPos) {
    Mat mask;
    vector<Mat> layers;
    
    split(transp, layers); // seperate channels
    Mat rgb[3] = { layers[0],layers[1],layers[2] };
    mask = layers[3]; // png's alpha channel used as mask
    merge(rgb, 3, transp);  // put together the RGB channels, now transp insn't transparent 
    transp.copyTo(frame.rowRange(yPos, yPos + transp.rows).colRange(xPos, xPos + transp.cols), mask);
}

void drawTransparency2(Mat frame, Mat transp, int xPos, int yPos) {
    Mat mask;
    vector<Mat> layers;
    
    split(transp, layers); // seperate channels
    Mat rgb[3] = { layers[0],layers[1],layers[2] };
    mask = layers[3]; // png's alpha channel used as mask
    merge(rgb, 3, transp);  // put together the RGB channels, now transp insn't transparent 
    Mat roi1 = frame(Rect(xPos, yPos, transp.cols, transp.rows));
    Mat roi2 = roi1.clone();
    transp.copyTo(roi2.rowRange(0, transp.rows).colRange(0, transp.cols), mask);
    double alpha = 0.9;
    addWeighted(roi2, alpha, roi1, 1.0 - alpha, 0.0, roi1);
}

int main( int argc, const char** argv )
{
    recorde = lerArquivo();
    namedWindow("Doribits", 19);
    img2 = imread("imagens/dorimefundo.png");

    VideoCapture capture;
    Mat frame, image;
    string inputName;
    CascadeClassifier cascade, nestedCascade;
    double scale = 2;

    dorimesafe = cv::imread("imagens/dorimesafe.png", IMREAD_UNCHANGED);
    pedra = cv::imread("imagens/pedra.png", IMREAD_UNCHANGED);
    if (dorimesafe.empty())
        printf("Error opening file dorimesafe.png\n");

    if (pedra.empty())
        printf("Error opening file pedra.png\n");

    string folder = "/home/phellipe/Downloads/opencv-4.1.2/data/haarcascades/";
    cascadeName = folder + "haarcascade_frontalface_alt.xml";
    nestedCascadeName = folder + "haarcascade_righteye_2splits.xml";
    inputName = "/dev/video0";

    if (!nestedCascade.load(samples::findFileOrKeep(nestedCascadeName)))
       cerr << "WARNING: Could not load classifier cascade for nested objects" << endl;
    if (!cascade.load(samples::findFile(cascadeName)))
    {
        cerr << "ERROR: Could not load classifier cascade" << endl;
        return -1;
    }

    if(!capture.open(inputName))
    {
        cout << "Capture from camera #" <<  inputName << " didn't work" << endl;
        return 1;
    }

    if( capture.isOpened() )
    {
        cout << "Video capturing has been started ..." << endl;
        
        for(;;)
        {
            capture >> frame;
            if( frame.empty() )
                break;

            //Mat frame1 = frame.clone();
            
            detectAndDraw( frame, cascade, nestedCascade, scale);

            char c = (char)waitKey(10);
            if( c == 27 || c == 'q' || c == 'Q' )
                break;
        }
    }

    return 0;
}

void detectAndDraw( Mat img, CascadeClassifier& cascade,
                    CascadeClassifier& nestedCascade,
                    double scale)
{
    static int frames = 0;
    double t = 0;
    vector<Rect> faces, faces2;
    const static Scalar colors[] =
    {
        Scalar(255,255,0),
        Scalar(255,128,0),
        Scalar(255,255,0),
        Scalar(0,255,0),
        Scalar(0,128,255),
        Scalar(0,255,255),
        Scalar(0,0,255),
        Scalar(255,0,255)
    };
    Mat gray, smallImg;

    cvtColor( img, gray, COLOR_BGR2GRAY );
    double fx = 1 / scale;
    resize( gray, smallImg, Size(), fx, fx, INTER_LINEAR_EXACT );
    equalizeHist( smallImg, smallImg );

    t = (double)getTickCount();
    cascade.detectMultiScale( smallImg, faces,
        1.2, 2, 0
        //|CASCADE_FIND_BIGGEST_OBJECT
        //|CASCADE_DO_ROUGH_SEARCH
        |CASCADE_SCALE_IMAGE,
        Size(30, 30) );

    frames++;


    t = (double)getTickCount() - t;
     
    for ( size_t i = 0; i < faces.size(); i++ )
    {
        Rect r = faces[i];
      
        Mat smallImgROI;
        vector<Rect> nestedObjects;
        Point center;
        Scalar color = colors[i%8];
        int radius;

       /* rectangle( img, Point(cvRound(r.x*scale), cvRound(r.y*scale)),
                   Point(cvRound((r.x + r.width-1)*scale), cvRound((r.y + r.height-1)*scale)),
                   color, 3, 8, 0);*/
        if( nestedCascade.empty() )
            continue;
        smallImgROI = smallImg( r );
        nestedCascade.detectMultiScale( smallImgROI, nestedObjects,
            1.1, 2, 0
            //|CASCADE_FIND_BIGGEST_OBJECT
            //|CASCADE_DO_ROUGH_SEARCH
            //|CASCADE_DO_CANNY_PRUNING
            |CASCADE_SCALE_IMAGE,
            Size(30, 30) );
        
        static int time;
        static int retro = 620;
        static bool colisao = false;
        
        for ( size_t j = 0; j < nestedObjects.size(); j++ )
        {
            int tamanhoY;
            if(colisao){
                salvaArquivo(recorde);
                waitKey(5000);
                exit(0);
                
                //exit(0);
                break;
            }
            string str = "SCORE: ";
            string strscore= "HIGH SCORE: ";
            time =time + 1;
            string conver = to_string(time/45);
            str+=conver;
            if((time/45) > recorde){
                recorde = time/45;
            }
            strscore = strscore + to_string(recorde);
            if(time/45 >= 2)
                retro = retro - 10;
            if(retro == 0)
                retro = 620;
            
            Rect nr = nestedObjects[0];
            img2 = imread("imagens/dorimefundo.png");
            
                drawTransparency2(img2, pedra, retro, 490);
            
            if((r.y+250) > 350){
                    tamanhoY = 350;
            }else{
                    tamanhoY = r.y+250;
            }
            // PEDRA Y 60, X 106
            // RATO  y 200 X 141
            drawTransparency2(img2, dorimesafe, 30, tamanhoY);

            if(retro > 30 && retro < 140 && tamanhoY >= 290){  //COLISAO
            cv::putText(img2,"GAME OVER", cv::Point(100, 200), cv::FONT_HERSHEY_DUPLEX,3,CV_RGB(255, 0, 0), 2);
            cv::putText(img2,str , cv::Point(210, 300), cv::FONT_HERSHEY_DUPLEX,2,CV_RGB(255, 0, 0), 2);
            cv::putText(img2,strscore, cv::Point(110, 400), cv::FONT_HERSHEY_DUPLEX,2,CV_RGB(255, 0, 0), 2);
            imshow("Doribits", img2);
                colisao = true;
                salvaArquivo(recorde);
                
                break;
                
            }
           
            

            cv::putText(img2,str , cv::Point(300, 18), cv::FONT_HERSHEY_DUPLEX,0.5,CV_RGB(255, 0, 0), 2);
            cv::putText(img2,strscore, cv::Point(580, 18), cv::FONT_HERSHEY_DUPLEX,0.5,CV_RGB(255, 0, 0), 2);


           
            imshow("Doribits", img2);
            center.x = cvRound((r.x + nr.x + nr.width*0.5)*scale);
            center.y = cvRound((r.y + nr.y + nr.height*0.5)*scale);
            radius = cvRound((nr.width + nr.height)*0.25*scale);
            circle( img, center, radius, color, 3, 8, 0 );
        }
        
    }



    

   
    
    imshow( "WEBCAM", img );
    
    
    
}
