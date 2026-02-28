#include <opencv2/opencv.hpp>
#include "SerialThread.h"
#include "Converter.h"
using namespace std;
using namespace cv;
const string PORT_NAME = "/dev/ttyUSB0";
const int RECT_X_START= 130;
const int RECT_Y_START= 0;
const int RECT_X_WIDTH= 480;
const int RECT_Y_HEIGIT= RECT_X_WIDTH;
const double CAMERA_SCALE=1.9;
int SCALE_WIDTH= CAMERA_SCALE*RECT_X_WIDTH;
int SCALE_HEIGHT= CAMERA_SCALE*RECT_Y_HEIGIT;
const int LEFT_IMAGE_X_POS=0;
const int RIGHT_IMAGE_X_POS=1000;
const int TOP_IMAGE_Y_POS=100;
static int CX=SCALE_WIDTH/2;
static int CY=SCALE_HEIGHT/2;
enum LRUD {LL,RR,UU,DD};
void drawCrossLine(Mat& frame)
{
    line(frame,Point(0,CY),Point(SCALE_WIDTH,CY),CV_RGB( 0, 255, 0 ),1,8);
    line(frame,Point(CX,0),Point(CX,SCALE_HEIGHT),CV_RGB( 0, 255, 0 ),1,8);
     
}
void drawLine(Mat frame,std::vector<double> bu,int lrud,double lbu=0,double rbu=0);
void drawLine(Mat frame,std::vector<double> bu,int lrud,double lbu,double rbu)
{
    for(double a :bu)
    {
        if(lrud<2)
        {
            auto x=lrud==0?CX-a:CX+a;
            auto y=CY-300;
            auto y1=CY+300;
            auto xy=Point2i(x,y);
            auto xy1=Point2i(x,y1);
            line(frame,xy,xy1,CV_RGB( 0, 255, 0 ),1,8);
        }else{
            auto y=lrud==2?CY-a:CY+a;
            auto x=CX-lbu;
            auto x1=CX+rbu;
            auto xy=Point2i(x,y);
            auto xy1=Point2i(x1,y);
            line(frame,xy,xy1,CV_RGB( 0, 255, 0 ),1,8);

        }
       
    }
}
int main(void) {
 
	VideoCapture capture(0);					//カメラ番号１を起動
	Mat reciveFrame;
	Mat frame;
	Mat SueFrame;
    Mat MotoFrame;
    boost::asio::io_context io;
    Converter converter;
    const std::string PORT_NAME = "/dev/ttyUSB0";
    SerialThread serialThread(io,PORT_NAME);
    serialThread.openPort();
   // SerialThread serialThread(io,PORT_NAME);
     int ans=0;
    int ansBk=0;
    std::vector<double> lbu;
    std::vector<double> rbu;
    std::vector<double> ubu;
    std::vector<double> dbu;
	while (1) {
		capture >> reciveFrame;	
        Mat clopFrame=reciveFrame(Rect(RECT_X_START, RECT_Y_START, RECT_X_WIDTH,RECT_Y_HEIGIT));
        resize(clopFrame,frame,Size(),CAMERA_SCALE,CAMERA_SCALE);
        
       // SueFrame = frame.clone();
        flip(frame,SueFrame,1);
        MotoFrame = frame.clone();
        					//カメラ画像の取得。frameに格納
        drawCrossLine(SueFrame);
        drawCrossLine(MotoFrame);
       // ans=serialThread.ctrlRead(1);
        double lpoint=lbu.size()?lbu.at(0):0;    
        double rpoint=rbu.size()?rbu.at(0):0;    
        sleep(0);
       // drawLine(SueFrame,lbu,LL);
       // drawLine(SueFrame,rbu,RR);    
        /*    
        drawLine(SueFrame,ubu,UU,lpoint,rpoint);
        drawLine(SueFrame,dbu,DD,lpoint,rpoint);
        drawLine(MotoFrame,lbu,LL);
        drawLine(MotoFrame,rbu,RR);        
        drawLine(MotoFrame,ubu,UU,lpoint,rpoint);
        drawLine(MotoFrame,dbu,DD,lpoint,rpoint);
*/
    	imshow("末口カメラ", SueFrame);			  //frameに格納されている画像を表示
		imshow("元口カメラ", MotoFrame);	
		moveWindow("末口カメラ", LEFT_IMAGE_X_POS,TOP_IMAGE_Y_POS);			  //frameに格納されている画像を表示
        moveWindow("元口カメラ", RIGHT_IMAGE_X_POS,TOP_IMAGE_Y_POS);			  //frameに格納されている画像を表示
		if (waitKey(1) == 27) break;			//ESCキーが入力されるまで実行

        
        if(ansBk==0 && ans==1) {
            converter.makeGraphBudasis(serialThread.readBudasi());
            lbu=converter.dispHidari();
            rbu=converter.dispMigi();
            ubu=converter.dispUe();
            dbu=converter.dispSita();
        }
        ansBk=ans;
	}
   serialThread.closePort();
    std::cout << "end, from qpmsserial!\n";
	return 0;


}

