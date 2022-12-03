#include <iostream>
#include <functional>
#include <algorithm>
#include <map>
#include "DataStream.hpp"

class A : public Yazi::serialize::serializable { 
public:
    A(const int i = 0, const double d = 0, const std::string s = "non-string", 
        const std::set<int, std::greater<int>>& set = std::set<int, std::greater<int>>())
        :
        _i(i),
        _d(d),
        _s(s),
        _set(set) {}
    void display() const {
        std::cout << _i << " " << _d << " " << _s << std::endl;
        for (auto& item : _set) {
            std::cout << item << " ";
        }
    }
    
    STREAM_FOR_WARD()
    SERIALIZE(_i, _d, _s, _set)

private:
    int _i;
    double _d;
    std::string _s;
    std::set<int, std::greater<int>> _set;
};

int main() {
    Yazi::serialize::DataStream ds;
    int tmp1 = 666;
    double tmp2 = 100.5;
    std::string tmp3 = "success";
    std::set<int, std::greater<int>> tmp4 {5, 9, 555, 111, 10000000};
    std::vector<double> tmp5 {6.6, 4.4, 20.88, 83};
    A tmp6 {555, 666.66 , "ok", std::set<int, std::greater<int>>{666,555,111,493,10000000}};
    ds << tmp1 << tmp2 << tmp3 << tmp4 << tmp5 << tmp6.for_ward();
    int rec1;
    double rec2;
    std::string rec3;
    std::set<int, std::greater<int>> rec4;
    std::vector<double> rec5;
    A rec6;
    ds >> rec1 >> rec2 >> rec3 >> rec4 >> rec5 >> rec6.for_ward();
    for (auto& item : rec5) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
    rec6.display();
}