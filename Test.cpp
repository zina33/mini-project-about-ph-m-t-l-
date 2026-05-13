#include <bits/stdc++.h>
using namespace std;
int main(){
ifstream thuong("BotThuong.csv");
ifstream thongminh("BotThongMinh.csv");
ifstream lieu("BotLieu.csv");
ifstream cantrong("BotCanTrong.csv");
    int i, t = 0;
    while(thuong >> i) t+=i;
    while(thongminh >> i) t+=i;
    while(lieu >> i) t+=i;
    while(cantrong >> i) t+=i;
    cout<<t;
}
