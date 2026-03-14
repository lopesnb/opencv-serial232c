////////////////////////////////////////////////////
//  
//
//  
//
////////////////////////////////////////////////////
//  作成時
//  ハード構成
//　　　PC：MouseComputer Co.,Ltd. NG-N-i5540　
//          Intel® Core™ i7-7700HQ × 8
//          GPU GForce gtx1060
//          OS Linux Ubuntu 24.04.3 LTS
//   
//  ソフト構成
//　　　IDE　VSCODE　1.109.0
//      コンパイラ　GCC　13.3.0 デバッガ　GDB 
//      Boost-1.83.0
//      Opencv　4.10
//      PLC 通信Format Fins(OMRON)

//
////////////////////////////////////////////////////

#include <opencv2/opencv.hpp>
#include "SerialThread.h"
#include "Converter.h"
//#include "common.h"
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
    Event ev;
	VideoCapture capture(2);					//カメラ番号１を起動
	VideoCapture capture2(4);					//カメラ番号１を起動
	Mat reciveFrame;
	Mat reciveFrame2;
	Mat frame;
	Mat frame2;
	Mat SueFrame;
    Mat MotoFrame;
    Mat combinedFrame;
 
    boost::asio::io_context io;
    Converter converter;
    const std::string PORT_NAME = "/dev/ttyUSB0";
    SerialThread serialThread(io,PORT_NAME);
    serialThread.openPort();
  //  SerialThread serialThread(io,PORT_NAME);
     int ans=0;
    int ansBk=0;
    std::vector<double> lbu;
    std::vector<double> rbu;
    std::vector<double> ubu;
    std::vector<double> dbu;
    serialThread.start();
	while (1) {
		capture >> reciveFrame;	
        Mat clopFrame=reciveFrame(Rect(RECT_X_START, RECT_Y_START, RECT_X_WIDTH,RECT_Y_HEIGIT));
        resize(clopFrame,frame,Size(),CAMERA_SCALE,CAMERA_SCALE);

        capture2 >> reciveFrame2;	
        Mat clopFrame2=reciveFrame2(Rect(RECT_X_START, RECT_Y_START, RECT_X_WIDTH,RECT_Y_HEIGIT));
        resize(clopFrame2,frame2,Size(),CAMERA_SCALE,CAMERA_SCALE);
        
        flip(frame,SueFrame,1);
        MotoFrame = frame2.clone();
        					//カメラ画像の取得。frameに格納
        drawCrossLine(SueFrame);
        drawCrossLine(MotoFrame);
       // ans=serialThread.ctrlRead(1);
        double lpoint=lbu.size()?lbu.at(0):0;    
        double rpoint=rbu.size()?rbu.at(0):0;    
        sleep(0);
        drawLine(SueFrame,lbu,LL);
        drawLine(SueFrame,rbu,RR);    
            
        drawLine(SueFrame,ubu,UU,lpoint,rpoint);
        drawLine(SueFrame,dbu,DD,lpoint,rpoint);
        drawLine(MotoFrame,lbu,LL);
        drawLine(MotoFrame,rbu,RR);        
        drawLine(MotoFrame,ubu,UU,lpoint,rpoint);
        drawLine(MotoFrame,dbu,DD,lpoint,rpoint);
        hconcat(SueFrame, MotoFrame, combinedFrame);
        line(combinedFrame, Point(SueFrame.cols, 0), Point(SueFrame.cols, combinedFrame.rows), Scalar(0, 255, 0), 2);

        imshow("Combined Camera", combinedFrame);
    	//imshow("末口カメラ", SueFrame);			  //frameに格納されている画像を表示
		//imshow("元口カメラ", MotoFrame);	
		//moveWindow("末口カメラ", LEFT_IMAGE_X_POS,TOP_IMAGE_Y_POS);			  //frameに格納されている画像を表示
        //moveWindow("元口カメラ", RIGHT_IMAGE_X_POS,TOP_IMAGE_Y_POS);			  //frameに格納されている画像を表示
		if (waitKey(1) == 27) 
        {
            EventToWork evAns;
            evAns.type=WorkerCommand::END;
            serialThread.toWorker.push(evAns);
            break;			//ESCキーが入力されるまで実行
        }
        ev.type=DeviceCommand::IDLE;
        if(serialThread.fromWorker.try_pop(ev)) 
        {
            if(ev.type == DeviceCommand::DATA_RECEIVED)
            {
                converter.makeGraphBudasis(ev.message);
                lbu=converter.dispHidari();
                rbu=converter.dispMigi();
                ubu=converter.dispUe();
                dbu=converter.dispSita();
            }
            if(ev.type == DeviceCommand::ERROR_REPORT)
            {
                break;
            }
        }
        ansBk=ans;
	}
    if (serialThread.worker.joinable()) {
        serialThread.worker.join(); 
    }
   serialThread.closePort();
    std::cout << "end, from qpmsserial!\n";
	return 0;


}

