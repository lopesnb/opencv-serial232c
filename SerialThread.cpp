
#include <iostream>
//#include <exception>>
#include "SerialThread.h"
#include "boost/format.hpp"
using namespace std;
using namespace boost::asio;


static string PLC_READ = "@00FA000000000010182";
static string PLC_WRITE	= "@00FA000000000010282";

static string CTRL_WRITE = "138800";   //D5000
static string CTRL_READ	= "139200";   //D5010

static string BUDASI_READ = "13EC00";   //D5100
//static string SAISYOU_HABA_ADR	= "145000";   //D5200

//--------------------------------------


static int KIDORI =	0x01;

//--------------------------------------

static int PC_RUN =	0x01;
static int  KIDORI_END = 0x02;
static int  KIDORI_POINT_NG_END =  0x10 + KIDORI_END;
static int  KIDORI_ERR = 0x04;
static int  TABLE_TENSO_END = 0x08;

//--------------------------------------
static string LEN_1 = "0001";  //CTRL READ
static string  LEN_40 = "0028"; //BUDASI ADRES
//static string LEN_100 = "0064"; //chosin tenso
//--------------------------------------







//using namespace boost::system;
SerialThread::SerialThread(io_context& io, const std::string& port_name)
:port_(io), port_name_(port_name),ctrl(0)
{
}

SerialThread::~SerialThread()
{
}

bool SerialThread::openPort()
{
    try {
        port_.open(port_name_);
        
        // ★★★ using namespace boost::asio; のおかげで短く書ける ★★★
        port_.set_option(BaudRate(38400));
        port_.set_option(CharactorSize(7));
        port_.set_option(StopBits( StopBits::two));
        port_.set_option(Parity(Parity::even));
        port_.set_option(FlowControl(FlowControl::none));

        return true;
    } catch (std::exception& e) {
        cerr << "Port open error: " << e.what() << endl;
        return false;
    }
}

bool SerialThread::closePort()
{
    port_.close();
    return false;
}

string SerialThread::readPort(int reciveSize)
{

    array<char, 200> buffer_rx;
   // size_t bytes_read ;
        // 受信データが改行文字 (\n) に達するまで読み取る (ストリーム志向の読み取り)
   /*    // この処理は、データが来るまでブロックされます。
    do{
        bytes_read = port_.read_some(buffer(buffer_rx));
    }while(bytes_read < 31);
    string received_data(buffer_rx.data(), bytes_read);
   // cout << "受信 (" << bytes_read << " bytes): " << received_data << endl;
    return received_data;
    */


boost::system::error_code ec;

    // 31バイト読み取るまでブロック（待機）する
    size_t bytes_read = boost::asio::read(port_, boost::asio::buffer(buffer_rx), 
                                          boost::asio::transfer_exactly(reciveSize), ec);

    if (ec) {
        // エラー処理（タイムアウトや切断など）
        return "";
    }

    return string(buffer_rx.data(), bytes_read);




}

void SerialThread::writePort(string message)
{
    // boost::asio::write関数を使用
    boost::asio::write(port_, buffer(message.c_str(), message.length()));
 //   cout << "送信: " << message;
}

std::string SerialThread::makeDenbun(string message)
{
    int fcs=makeFCS(message);
    return (boost::format("%04X%02X*\r") % message % fcs).str(); 
}

bool SerialThread::isFCSMaching(string denbun)
{
    string fcsString=denbun.substr(denbun.size()-4,2);
    int fcs=stoi(fcsString,nullptr,16);
    string messageString=denbun.substr(0,denbun.size()-4);
    int calcFcs=makeFCS(messageString);

    return fcs==calcFcs?true:false;
}

char SerialThread::makeFCS(string message)
{
    char c[1500];
    char b=0;
    strcpy(c,message.c_str());
    for(int i=0;i<message.size();i++)
    {
        b ^= c[i];
    }
    return b;
}
void SerialThread::bitOn(int b)
{
	char recv[100];
	int ret =0;

	string sd;
	sd=PLC_WRITE;
	sd+=CTRL_WRITE;
	sd+=LEN_1;
	ctrl |= b;
	string bitData;
    bitData=(boost::format("%04X") % ctrl).str(); 
	sd=sd+bitData;
	sd=makeDenbun(sd);
	writePort(sd);
    readPort(27);


}

void SerialThread::bitOff(int b)
{

	char recv[100];
	int ret =0;

	string sd;
	sd=PLC_WRITE;
	sd+=CTRL_WRITE;
	sd+=LEN_1;
    ctrl &= ~b;
	string bitData;
    bitData=(boost::format("%04X") % ctrl).str(); 
	sd=sd+bitData;
	sd=makeDenbun(sd);
	writePort(sd);
    readPort(27);

}

int SerialThread::ctrlRead(int tusinMode)
{
	int ret=0,ret1=0;
	char recv[100];
	string sd;
	sd=PLC_READ;
	sd+=CTRL_READ;
	sd+=LEN_1;
	sd=makeDenbun(sd);
    string denbun;
    do{
        writePort(sd);
        denbun=readPort(31);
    }while(!isFCSMaching(denbun));

	if(tusinMode!=999)
	{
        ret=stoi(denbun.substr(26,1));
        
    }
	return ret;
}
std::vector<std::string>SerialThread::readBudasi()
{
	int ret=0,ret1=0;
	char recv[100];
	string sd;
	sd=PLC_READ;
	sd+=BUDASI_READ;
	sd+=LEN_40;
	sd=makeDenbun(sd);
    string denbun;
    do{
        writePort(sd);
        denbun=readPort(187);
    }while(!isFCSMaching(denbun));
    std::vector<std::string> budasis;
    for(int i=0;i<40;i++)
    {
        budasis.push_back(denbun.substr(23+i*4,4));
    }
	return budasis;
}
