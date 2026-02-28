#include "Converter.h"
#include <algorithm>
#include <iterator>
Converter::Converter()
{
}

Converter::~Converter()

{
}

void Converter::makeGraphBudasis(std::vector<std::string> data)
{
    hidariBudasis=makeDoubleBudasis(data,0,10);
    migiBudasis=makeDoubleBudasis(data,10,20);
    ueBudasis=makeDoubleBudasis(data,20,30);
    sitaBudasis=makeDoubleBudasis(data,30,40);
    int a=1;
}

std::vector<double> Converter::dispHidari()
{
    return hidariBudasis;
}

std::vector<double> Converter::dispMigi()
{
    return migiBudasis;
}

std::vector<double> Converter::dispUe()
{
    return ueBudasis;
}
std::vector<double> Converter::dispSita()
{
    return sitaBudasis;
}

std::vector<double> Converter::makeDoubleBudasis(std::vector<std::string> datas,int startp,int endp)
{
    std::vector<int> ldata;
    std::vector<double> ans;
    std::vector<std::string> datal(datas.begin()+startp, datas.begin() + endp);

    std::transform(datal.begin(),datal.end(),std::back_inserter(ldata), [] (std::string s){return stoi(s);});
    //int count = ldata.at(0);
    std::transform(ldata.begin()+1,ldata.begin()+ldata.at(0)+1,std::back_inserter(ans), [] (int s){return 0.1*s;});

    return ans;
}
