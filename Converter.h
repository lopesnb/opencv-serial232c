
#pragma once
#include <string>
#include <boost/asio.hpp>
class Converter
{
public:
    Converter();
    ~Converter();
    void makeGraphBudasis(std::vector<std::string> datas);
    std::vector<double> dispHidari();
    std::vector<double> dispMigi();
    std::vector<double> dispUe();
    std::vector<double> dispSita();
 
private:
    std::vector<double> hidariBudasis;
    std::vector<double> migiBudasis;
    std::vector<double> ueBudasis;
    std::vector<double> sitaBudasis;

    std::vector<double> makeDoubleBudasis(std::vector<std::string> datas,int startp,int endp);

};

