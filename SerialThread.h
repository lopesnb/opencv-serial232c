
#pragma once
#include <string>
#include <boost/asio.hpp>
#include "SafeQueue.h"
class SerialThread
{
public:
    SerialThread(boost::asio::io_context& io, const std::string& port_name);
    ~SerialThread();
    bool openPort();
    bool closePort();
    std::string readPort(int reciveSize);
    void writePort(std::string message);
    void bitOn(int b);
    void bitOff(int b);
    int ctrlRead(int tusinMode);
    void start();
    std::vector<std::string> readBudasi();

/////////////////////////////////////////
    SafeQueue<EventToWork> toWorker;
    SafeQueue<Event> fromWorker;
    std::thread worker;

private:
    std::string makeDenbun(std::string message);
    bool isFCSMaching(std::string denbun);
    char  makeFCS(std::string message);
    void serialLoop();

// Boostの型をメンバ変数に使う場合は、フルネームで記述する
    boost::asio::serial_port port_;
    std::string port_name_;
    unsigned int ctrl;
    using BaudRate = boost::asio::serial_port_base::baud_rate;
    using CharactorSize = boost::asio::serial_port_base::character_size;
    using StopBits = boost::asio::serial_port_base::stop_bits;
    using Parity   = boost::asio::serial_port_base::parity;
    using FlowControl   = boost::asio::serial_port_base::flow_control;

};

