#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <ctime>
#include <fstream>

using namespace std;

// 1. Khai báo một biến toàn cục (global) để lưu con số seed truyền từ ngoài vào
unsigned int global_seed = 0;

// Lớp đại diện cho một lá bài
class Pocket {
private:
    int So;   // Số của lá bài (1-13 tương ứng A, 2..10, J, Q, K)
    int Chat; // Chất của lá bài (1: Bích, 2: Chuồn, 3: Rô, 4: Cơ)
public:
    // Constructor mặc định (khởi tạo lá bài rỗng)
    Pocket() {
        So = 0;
        Chat = 0;
    }
    // Constructor khởi tạo lá bài với số và chất cụ thể
    Pocket(int So, int Chat) {
        this->So = So;
        this->Chat = Chat;
    }
    // Toán tử so sánh khác nhau
    bool operator != (Pocket p) {
        return (So != p.So || Chat != p.Chat);
    }
    // Toán tử so sánh bằng nhau
    bool operator == (Pocket p) {
        return (So == p.So && Chat == p.Chat);
    }
    int getSo() const { return So; }
    int getChat() const { return Chat; }

    // Chuyển đổi thông tin lá bài thành chuỗi (String) để in ra màn hình
    string getTenLaBai() const {
        if (So == 0) return "Null"; // Lá bài không tồn tại
        string strSo;
        if (So == 1) strSo = "A";
        else if (So == 11) strSo = "J";
        else if (So == 12) strSo = "Q";
        else if (So == 13) strSo = "K";
        else strSo = to_string(So);

        string strChat;
        if (Chat == 1) strChat = "Bich";
        else if (Chat == 2) strChat = "Chuon";
        else if (Chat == 3) strChat = "Ro";
        else if (Chat == 4) strChat = "Co";

        return strSo + " " + strChat;
    }
};

// Lớp đại diện cho bộ bài (Nọc)
class bobai {
private:
    int SoLuong = 52;
    vector<Pocket> BoBai;

    // Viết hàm random dựa trên global_seed
    int random(int lo, int hi) {
        // CỰC KỲ QUAN TRỌNG: Phải có chữ 'static'
        // 'static' giúp bộ sinh số rng chỉ khởi tạo ĐÚNG 1 LẦN bằng global_seed
        // Các lần gọi hàm random() tiếp theo sẽ tiếp tục xáo trộn dựa trên trạng thái trước đó
        static std::mt19937 rng(global_seed);

        std::uniform_int_distribution<int> dist(lo, hi);
        return dist(rng);
    }
public:
    bobai() {
        BoBai.push_back(Pocket(0, 0));
        for (int i = 1; i <= 4; i++) {
            for (int j = 1; j <= 13; j++) {
                BoBai.push_back(Pocket(j, i));
            }
        }
    }
    Pocket BocBai() {
        int ViTri = random(1, SoLuong);
        Pocket ans = BoBai[ViTri];
        for (int i = ViTri + 1; i <= SoLuong; i++)
            BoBai[i - 1] = BoBai[i];
        SoLuong = SoLuong - 1;
        return ans;
    }
};

// Biến toàn cục lưu trữ tất cả các Phỏm đã được hạ
vector<vector<Pocket>> PhomChungToanBan;

// Lớp đại diện cho một người chơi (bot)
class Player {
private:
    int id;
    int loaiBot = 1; // 1: Thường, 2: Thông minh, 3: Liều, 4: Cẩn trọng

    vector<Pocket> BaiNamTrenSan;
    vector<Pocket> BaiTrenTay;
    vector<vector<Pocket>> PhomDaHa;
    vector<Pocket> CacLaBaiDaAn;
    vector<vector<Pocket>> PhomTuViecAn;

    bool isPhom(vector<Pocket> nhom) {
        if (nhom.size() < 3) return false;
        sort(nhom.begin(), nhom.end(), [](Pocket& a, Pocket& b) {
            if (a.getSo() == b.getSo()) return a.getChat() < b.getChat();
            return a.getSo() < b.getSo();
        });

        bool sameSo = true;
        for (size_t i = 1; i < nhom.size(); ++i) {
            if (nhom[i].getSo() != nhom[0].getSo()) sameSo = false;
        }
        if (sameSo) return true;

        bool isRun = true;
        for (size_t i = 1; i < nhom.size(); ++i) {
            if (nhom[i].getChat() != nhom[0].getChat() ||
                nhom[i].getSo() != nhom[i - 1].getSo() + 1) {
                isRun = false; break;
            }
        }
        return isRun;
    }

    bool isCa(Pocket a, Pocket b) {
        if (a.getSo() == b.getSo()) return true;
        if (a.getChat() == b.getChat() && abs(a.getSo() - b.getSo()) <= 2) return true;
        return false;
    }

    void checkPhom(vector<Pocket>& hand, vector<vector<Pocket>> currentPhoms,
                   vector<vector<Pocket>>& bestPhoms, vector<Pocket>& bestRem, int& minScore) {
        int currentScore = 0;
        for (Pocket p : hand) currentScore += p.getSo();

        if (currentScore < minScore) {
            minScore = currentScore;
            bestPhoms = currentPhoms; bestRem = hand;
        }

        int n = hand.size();
        for (int i = 1; i < (1 << n); ++i) {
            vector<Pocket> subset, remaining;
            for (int j = 0; j < n; ++j) {
                if ((i & (1 << j)) != 0) subset.push_back(hand[j]);
                else remaining.push_back(hand[j]);
            }
            if (isPhom(subset)) {
                vector<vector<Pocket>> nextPhoms = currentPhoms;
                nextPhoms.push_back(subset);
                checkPhom(remaining, nextPhoms, bestPhoms, bestRem, minScore);
            }
        }
    }

public:
    void setId(int i) { id = i; }
    void setLoaiBot(int type) { loaiBot = type; }

    string getTen() {
        string baseName = "Bot thu " + to_string(id);
        if (loaiBot == 1) return baseName + " (Thuong)";
        if (loaiBot == 2) return baseName + " (Thong minh)";
        if (loaiBot == 3) return baseName + " (Lieu)";
        if (loaiBot == 4) return baseName + " (Can trong)";
        return baseName;
    }

//    void inBaiTrenTay() {
//        cout << "   [BAI TREN TAY]: ";
//        for (Pocket p : BaiTrenTay) {
//            cout << "[" << p.getTenLaBai() << "] ";
//        }
//        cout << "\n";
//    }

    void KhoiTaoBoBai(int n, bobai& BoBai) {
        for (int i = 0; i < n; i++) BaiTrenTay.push_back(BoBai.BocBai());
    }

    void Add(Pocket p) { BaiTrenTay.push_back(p); }

    int SoLuongBaiNamTrenSan() { return BaiNamTrenSan.size(); }

    bool return_pick(Pocket p) {
        if (p.getSo() == 0) return false;
        int n = BaiTrenTay.size();
        for (int i = 0; i < n - 1; i++) {
            for (int j = i + 1; j < n; j++) {
                vector<Pocket> testnhom = {BaiTrenTay[i], BaiTrenTay[j], p};
                if (isPhom(testnhom)) {
                    PhomTuViecAn.push_back(testnhom);
//                    cout << "   >>> [" << getTen() << "] DA AN LA BAI: [" << p.getTenLaBai() << "]\n";
                    return true;
                }
            }
        }
        return false;
    }

    Pocket return_discard() {
        if (BaiTrenTay.empty()) return Pocket();

        // 1. Lọc bài bị khóa bởi phỏm ăn
        vector<Pocket> danhSachCamDanh;
        for (auto& phom : PhomTuViecAn) {
            for (auto& la : phom) danhSachCamDanh.push_back(la);
        }

        vector<Pocket> baiCoTheDanh;
        for (auto& laTay : BaiTrenTay) {
            bool biKhoa = false;
            for (auto& laCam : danhSachCamDanh) {
                if (laTay == laCam) { biKhoa = true; break; }
            }
            if (!biKhoa) baiCoTheDanh.push_back(laTay);
        }
        if (baiCoTheDanh.empty()) baiCoTheDanh = BaiTrenTay;

        // 2. Lấy bộ rác tốt nhất
        vector<vector<Pocket>> bestPhoms;
        vector<Pocket> bestRem;
        int minScore = 1e9;
        checkPhom(baiCoTheDanh, {}, bestPhoms, bestRem, minScore);

        Pocket danhLaRac;

        // Nếu đã lên phỏm hết, bắt buộc đánh lá cuối
        if (bestRem.empty()) {
            danhLaRac = baiCoTheDanh.back();
        } else {
            // Tách các lá rác thành 2 nhóm: Cạ và Rác hoàn toàn
            vector<Pocket> racHoanToan;
            vector<Pocket> giuCa;
            for (size_t i = 0; i < bestRem.size(); i++) {
                bool inCa = false;
                for (size_t j = 0; j < bestRem.size(); j++) {
                    if (i == j) continue;
                    if (isCa(bestRem[i], bestRem[j])) { inCa = true; break; }
                }
                if (inCa) giuCa.push_back(bestRem[i]);
                else racHoanToan.push_back(bestRem[i]);
            }

            // Hàm Lambda tìm lá to nhất trong 1 mảng
            auto getLargest = [](vector<Pocket>& v) {
                Pocket maxP = v[0];
                for (Pocket p : v) {
                    if (p.getSo() > maxP.getSo()) maxP = p;
                }
                return maxP;
            };

            // === TÁCH BIỆT CHIẾN THUẬT ===
            int soBaiDaDanh = BaiNamTrenSan.size();

            if (loaiBot == 1) {
                // BOT THƯỜNG: Luôn đánh lá rác to nhất, bất chấp có phải Cạ hay không
                danhLaRac = getLargest(bestRem);

            } else if (loaiBot == 2) {
                // BOT THÔNG MINH: Sau 2 lượt (đã đánh >= 2 lá) thì xé cạ đánh lá to nhất
                if (soBaiDaDanh >= 2) {
                    danhLaRac = getLargest(bestRem);
                } else {
                    if (!racHoanToan.empty()) danhLaRac = getLargest(racHoanToan);
                    else danhLaRac = getLargest(giuCa); // Hết rác hoàn toàn bắt buộc phải xé cạ
                }

            } else if (loaiBot == 3) {
                // BOT LIỀU: Luôn ưu tiên đánh rác hoàn toàn to nhất. KHÔNG BAO GIỜ xé cạ (trừ khi trên tay chỉ toàn Cạ bắt buộc phải đánh)
                if (!racHoanToan.empty()) danhLaRac = getLargest(racHoanToan);
                else danhLaRac = getLargest(giuCa);

            } else if (loaiBot == 4) {
                // BOT CẨN TRỌNG: Giống Bot Thông Minh nhưng cố chấp giữ cạ đến sau lượt 3
                if (soBaiDaDanh >= 3) {
                    danhLaRac = getLargest(bestRem);
                } else {
                    if (!racHoanToan.empty()) danhLaRac = getLargest(racHoanToan);
                    else danhLaRac = getLargest(giuCa);
                }
            }
        }

//        cout << "   >>> [" << getTen() << "] Danh la: [" << danhLaRac.getTenLaBai() << "]\n";

        for (auto it = BaiTrenTay.begin(); it != BaiTrenTay.end(); ++it) {
            if (*it == danhLaRac) { BaiTrenTay.erase(it); break; }
        }
        return danhLaRac;
    }

    void Add_BaiNamTrenSan(Pocket p) { BaiNamTrenSan.push_back(p); }

    void Erase_BaiNamTrenSan(Pocket p) {
        for (auto it = BaiNamTrenSan.begin(); it != BaiNamTrenSan.end(); ++it) {
            if (*it == p) { BaiNamTrenSan.erase(it); break; }
        }
    }

    void HaBai() {
        vector<vector<Pocket>> bestPhoms;
        vector<Pocket> bestRem;
        int minScore = 1e9;
        checkPhom(BaiTrenTay, {}, bestPhoms, bestRem, minScore);

        PhomDaHa = bestPhoms;
        for(auto& pAn : PhomTuViecAn) PhomDaHa.push_back(pAn);
        BaiTrenTay = bestRem;

        if(!PhomDaHa.empty()){
//            cout << "   >>> [" << getTen() << "] VUA HA PHOM!\n";
        }

        for (auto phom : PhomDaHa) {
            PhomChungToanBan.push_back(phom);
        }
    }

    void GuiBai() {
        bool coTheGuiTiep = true;
        while (coTheGuiTiep && !BaiTrenTay.empty()) {
            coTheGuiTiep = false;
            for (auto it = BaiTrenTay.begin(); it != BaiTrenTay.end(); ) {
                bool daGuiLaNay = false;
                for (auto& phomChung : PhomChungToanBan) {
                    vector<Pocket> testPhom = phomChung;
                    testPhom.push_back(*it);
                    if (isPhom(testPhom)) {
                        phomChung.push_back(*it);
//                        cout << "   >>> [" << getTen() << "] DA GUI LA BAI [" << it->getTenLaBai() << "] VAO PHOM NGUOI KHAC!\n";
                        it = BaiTrenTay.erase(it);
                        daGuiLaNay = true;
                        coTheGuiTiep = true;
                        break;
                    }
                }
                if (!daGuiLaNay) ++it;
            }
        }
    }

    int tinhDiem() {
        int score = 0;
        for (Pocket p : BaiTrenTay) score += p.getSo();
        return score;
    }

    bool U() {
        vector<vector<Pocket>> bestPhoms; vector<Pocket> bestRem;
        int minScore = 1e9;
        checkPhom(BaiTrenTay, {}, bestPhoms, bestRem, minScore);
        if (bestRem.empty()) return true;
        if (BaiTrenTay.size() == 10 && bestRem.size() == 1) return true;
        return false;
    }
};

ofstream thuong("BotThuong.csv",ios::app);
ofstream thongminh("BotThongMinh.csv",ios::app);
ofstream lieu("BotLieu.csv",ios::app);
ofstream cantrong("BotCanTrong.csv",ios::app);

vector<Pocket> BaiDaLo;
int main(int argc, char* argv[]) {
    // Nhận con số từ MoPhong và gán vào biến global_seed
    if (argc > 1) {
        // Dùng stoul (string to unsigned long) vì seed thường là số nguyên dương lớn
        global_seed = stoul(argv[1]);
    } else {
        // Trường hợp bạn bấm chạy trực tiếp Nguon.exe mà không qua MoPhong
        // Thì cho nó 1 con số mặc định để không bị lỗi
        global_seed = 12345;
    }
    bobai BoBai;
    vector<int> Score_ToTal(4, 0);
    int Cuoc[4] = {1, 1, 1, 1};

    // Khởi tạo 4 Bot tương ứng với 4 loại chiến thuật
    vector<Player> MangNguoiChoi(4);
    for(int i = 0; i < 4; i++){
        MangNguoiChoi[i].setId(i + 1);
        MangNguoiChoi[i].setLoaiBot(i + 1); // 1: Thường, 2: Thông minh, 3: Liều, 4: Cẩn trọng
    }

    MangNguoiChoi[0].KhoiTaoBoBai(10, BoBai);
    for (int i = 1; i < 4; i++) {
        MangNguoiChoi[i].KhoiTaoBoBai(9, BoBai);
    }
//
//    cout << "\n============= BAI KHOI TAO BAN DAU =============\n";
//    for(int i = 0; i < 4; i++){
////        cout << MangNguoiChoi[i].getTen() << "\n";
////        MangNguoiChoi[i].inBaiTrenTay();
//    }
//    cout << "================================================\n";
//
    Pocket NullPocket;
    int Luot = 0;
    Pocket LaBaiCoTheLay = NullPocket;
    bool DieuKienKetThuc = 0;

    while (DieuKienKetThuc == 0) {
        Player* player = &MangNguoiChoi[Luot];

//        cout << "\n--- Luot cua " << player->getTen() << " --- (Da danh: " << player->SoLuongBaiNamTrenSan() << " la)\n";
//        player->inBaiTrenTay();

        if (LaBaiCoTheLay != NullPocket) {
            if (player->SoLuongBaiNamTrenSan() != 4) {
                if (player->return_pick(LaBaiCoTheLay)) {
                    player->Add(LaBaiCoTheLay);

                    int j = Luot - 1;
                    if (j < 0) j += 4;
                    MangNguoiChoi[j].Erase_BaiNamTrenSan(LaBaiCoTheLay);

                    if (MangNguoiChoi[j].SoLuongBaiNamTrenSan() == 4) {
                        Cuoc[j] -= 4; Cuoc[Luot] += 4;
                    } else {
                        Cuoc[j] -= 1; Cuoc[Luot] += 1;
                    }
                    LaBaiCoTheLay = NullPocket;
                } else {
                    Pocket laBoc = BoBai.BocBai();
                    player->Add(laBoc);
//                    cout << "   >>> [" << player->getTen() << "] Vua boc tu noc la: [" << laBoc.getTenLaBai() << "]\n";
                    LaBaiCoTheLay = NullPocket;
                }

                if (player->U()) {
//                    cout << "\n!!! [" << player->getTen() << "] DA U !!!\n";
                    Score_ToTal[Luot] = -100000;
                    break;
                }
            }
        }

        if (LaBaiCoTheLay == NullPocket) {
            LaBaiCoTheLay = player->return_discard();
            BaiDaLo.push_back(LaBaiCoTheLay);
            player->Add_BaiNamTrenSan(LaBaiCoTheLay);
        }

        if (player->U()) {
//            cout << "\n!!! [" << player->getTen() << "] DA U !!!\n";
            Score_ToTal[Luot] = -100000;
            break;
        }

        int j = Luot - 1;
        if (j < 0) j += 4;
        if (MangNguoiChoi[j].SoLuongBaiNamTrenSan() == 4) {
            MangNguoiChoi[j].HaBai();
            MangNguoiChoi[j].GuiBai();
            Score_ToTal[j] = MangNguoiChoi[j].tinhDiem();
        }

        Luot++;
        if (Luot >= 4) Luot -= 4;

        DieuKienKetThuc = 1;
        for (int i = 0; i < 4; i++) {
            if (MangNguoiChoi[i].SoLuongBaiNamTrenSan() < 4) {
                DieuKienKetThuc = 0;
            }
        }
    }

//    cout << "\n=============== KET QUA VAN BAI ===============\n";
    if (Score_ToTal[Luot] == -100000) {
        for (int i = 0; i < 4; i++) {
            if (i == Luot) Cuoc[i] += 45; // Mình ù ăn của cả 3 nhà
            else Cuoc[i] -= 15;           // Mỗi nhà thua 15
        }
    } else {
        vector<int> XepHang = Score_ToTal;
        sort(XepHang.begin(), XepHang.end());

        // In ra điểm rác của 4 bot
//        for (int i = 0; i < 4; i++) {
//            cout << MangNguoiChoi[i].getTen() << " - Diem rac: " << Score_ToTal[i] << "\n";
//            if (Score_ToTal[i] == XepHang[0]) Cuoc[i] += 6;
//            else if (Score_ToTal[i] == XepHang[1]) Cuoc[i] -= 1;
//            else if (Score_ToTal[i] == XepHang[2]) Cuoc[i] -= 2;
//            else if (Score_ToTal[i] == XepHang[3]) Cuoc[i] -= 3;
//        }
    }

//    cout << "\n=============== TONG KET CUOC ===============\n";

    thuong<<Cuoc[0]-1<<endl;
    thongminh<<Cuoc[1]-1<<endl;
    lieu<<Cuoc[2]-1<<endl;
    cantrong<<Cuoc[3]-1<<endl;


    thuong.close();
    thongminh.close();
    lieu.close();
    cantrong.close();

    return 0;
}
