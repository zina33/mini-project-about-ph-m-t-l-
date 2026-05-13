#include <bits/stdc++.h>
using namespace std;
int main(){

    ofstream thuong("BotThuong.csv");
    ofstream thongminh("BotThongMinh.csv");
    ofstream lieu("BotLieu.csv");
    ofstream cantrong("BotCanTrong.csv");

    random_device rd;
    mt19937 master_rng(rd());

    cout<<"Nhap so lan chay: \n";
    int n;
    cin>>n;

    while(n--){
        unsigned int seed=master_rng();
        string cmd="Nguon.exe "+to_string(seed);
        system(cmd.c_str());
    }

    cout<<"Hoan thanh mo phong!";
}
