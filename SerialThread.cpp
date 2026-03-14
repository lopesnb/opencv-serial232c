
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
:port_(io), port_name_(port_name),ctrl(0),io(io), timer(io)
{
}

SerialThread::~SerialThread()
{
    closePort();
    if (worker.joinable()) worker.join();
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
void SerialThread::serialLoop()
{
    int ans=0;
    int ansBk=0;
    EventToWork evWk;
    try{
        do{
            if(toWorker.try_pop(evWk))
            {
                if(evWk.type== WorkerCommand::END) break;

            }
            int ans=ctrlRead(1);
            if(ansBk==0 && ans==1)
            {
                Event ev;
                ev.type= DeviceCommand::DATA_RECEIVED;
                ev.message=readBudasi();

                fromWorker.push(ev);
            }
            ansBk=ans;
        }while(1);

    }catch(std::runtime_error e)
    {
        Event ev;
        ev.type= DeviceCommand::ERROR_REPORT;
        
        fromWorker.push(ev);
 
    }
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
	string denbun=  PLCSendRecieve(sd);


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
	string denbun=  PLCSendRecieve(sd);

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
    string denbun=  PLCSendRecieve(sd);
	if(tusinMode!=999)
	{
        ret=stoi(denbun.substr(26,1));
        
    }
	return ret;
}
void SerialThread::start()
{
    worker = std::thread([this]() { serialLoop(); });
}
const int MAX_RETRY = 3;
std::vector<std::string> SerialThread::readBudasi()
{
	int ret=0,ret1=0;
	char recv[100];
	string sd;
	sd=PLC_READ;
	sd+=BUDASI_READ;
	sd+=LEN_40;
	sd=makeDenbun(sd);
    string denbun=  PLCSendRecieve(sd);
    std::vector<std::string> budasis;
    for(int i=0;i<40;i++)
    {
        budasis.push_back(denbun.substr(23+i*4,4));
    }
	return budasis;
}
std::string SerialThread::PLCSendRecieve(string sd)
{
    string denbun;
    int retry_count =0;
    bool success= false;
    do{
        writePort(sd);
        if(readWithTimeout(denbun, 30)&& isFCSMaching(denbun))
        {
            success= true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }while(retry_count <MAX_RETRY );
    if (!success) throw std::runtime_error("Serial communication failed after max retries.");
 	return denbun;
}
bool SerialThread::readWithTimeout(std::string& data, int timeout_ms) {
    bool topped = false;
    boost::system::error_code ec_read = boost::asio::error::would_block;
    boost::asio::streambuf buf;
    // 1. 非同期読み込みを開始
    boost::asio::async_read_until(port_, buf, "\n", 
        [&](const error_code& ec, size_t length) {
            ec_read = ec; // 読み終わったらここに入る
            if (!ec) {
                // length 分だけ取り出して string に変換
                std::istream is(&buf);
                std::getline(is, data); // 改行を除いて data に格納
            }
        });

    // 2. タイマーを設定
    timer.expires_from_now(std::chrono::milliseconds(timeout_ms));
    timer.async_wait([&](const error_code& ec) {
        if (!ec) port_.cancel(); // タイムアウトしたら通信を強制キャンセル！
    });

    // 3. イベントが完了するまで待機（ここがポイント）
    io.reset();
    while (io.run_one()) {
        if (ec_read != boost::asio::error::would_block) {
            timer.cancel(); // 読み込めたらタイマーを止める
        }
    }

    return (ec_read == boost::system::errc::success);
}