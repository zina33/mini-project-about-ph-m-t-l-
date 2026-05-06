#include <bits/stdc++.h>
using namespace std;

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

        return strSo + " " + strChat; // Ví dụ: "A Bich", "10 Co"
    }
};

// Lớp đại diện cho bộ bài (Nọc)
class bobai {
private:
    int SoLuong = 52; // Tổng số lá bài ban đầu
    vector<Pocket> BoBai; // Mảng chứa các lá bài trong nọc

    // Hàm sinh số ngẫu nhiên trong khoảng [lo, hi] để bốc bài ngẫu nhiên
    int random(int lo, int hi) {
        static std::mt19937 rng(time(0));
        std::uniform_int_distribution<int> dist(lo, hi);
        return dist(rng);
    }
public:
    // Khởi tạo bộ bài chuẩn 52 lá
    bobai() {
        BoBai.push_back(Pocket(0, 0)); // Dummy pocket ở vị trí 0 (để dễ xử lý index)
        for (int i = 1; i <= 4; i++) { // Duyệt 4 chất
            for (int j = 1; j <= 13; j++) { // Duyệt 13 số
                BoBai.push_back(Pocket(j, i));
            }
        }
    }
    // Hàm bốc 1 lá bài từ nọc ra
    Pocket BocBai() {
        int ViTri = random(1, SoLuong); // Chọn ngẫu nhiên 1 lá còn lại
        Pocket ans = BoBai[ViTri];
        // Dồn mảng lại để xóa lá bài vừa bốc ra khỏi bộ bài
        for (int i = ViTri + 1; i <= SoLuong; i++)
            BoBai[i - 1] = BoBai[i];
        SoLuong = SoLuong - 1; // Giảm số lượng bài trong nọc
        return ans;
    }
};



// Lớp đại diện cho một người chơi (bot hoặc người thật)
class Player {
private:
    int id; // Số thứ tự người chơi (1 đến 4)
    bool laNguoiChoi = false; // Phân biệt người thật (thông minh) và bot thường
    vector<Pocket> BaiNamTrenSan; // Các lá bài rác người chơi đã đánh ra
    vector<Pocket> BaiTrenTay; // Các lá bài đang cầm trên tay
    vector<vector<Pocket>> PhomDaHa; // Các phỏm người chơi đã hạ
    vector<Pocket> CacLaBaiDaAn; // Những lá bài đã ăn của người khác
    vector<vector<Pocket>> PhomTuViecAn; // Các phỏm bắt buộc tạo ra do quá trình ăn bài

    // Kiểm tra xem 1 tập hợp bài có hợp lệ là 1 Phỏm không
    bool isPhom(vector<Pocket> nhom) {
        if (nhom.size() < 3) return false; // Phỏm phải có ít nhất 3 lá
        // Sắp xếp bài theo số, nếu trùng số thì sắp xếp theo chất
        sort(nhom.begin(), nhom.end(), [](Pocket& a, Pocket& b) {
            if (a.getSo() == b.getSo()) return a.getChat() < b.getChat();
            return a.getSo() < b.getSo();
        });

        // Kiểm tra trường hợp Phỏm sáp (cùng số, khác chất)
        bool sameSo = true;
        for (size_t i = 1; i < nhom.size(); ++i) {
            if (nhom[i].getSo() != nhom[0].getSo()) sameSo = false;
        }
        if (sameSo) return true;

        // Kiểm tra trường hợp Phỏm sảnh (cùng chất, số liên tiếp)
        bool isRun = true;
        for (size_t i = 1; i < nhom.size(); ++i) {
            if (nhom[i].getChat() != nhom[0].getChat() ||
                nhom[i].getSo() != nhom[i - 1].getSo() + 1) {
                isRun = false; break;
            }
        }
        return isRun;
    }

    // Kiểm tra xem 2 lá bài có tạo thành "Cạ" (có khả năng lên Phỏm) hay không
    bool isCa(Pocket a, Pocket b) {
        if (a.getSo() == b.getSo()) return true; // Cạ sáp (cùng số)
        // Cạ sảnh (cùng chất và cách nhau không quá 2 đơn vị số)
        if (a.getChat() == b.getChat() && abs(a.getSo() - b.getSo()) <= 2) return true;
        return false;
    }

    // Hàm đệ quy duyệt tất cả các tập con để tìm cách tách bài trên tay thành các Phỏm
    // nhằm tối thiểu hóa tổng điểm của các lá rác còn lại
    void checkPhom(vector<Pocket>& hand, vector<vector<Pocket>> currentPhoms,
                   vector<vector<Pocket>>& bestPhoms, vector<Pocket>& bestRem, int& minScore) {
        int currentScore = 0;
        // Tính tổng điểm của những lá không nằm trong phỏm (rác)
        for (Pocket p : hand) currentScore += p.getSo();

        // Cập nhật cấu hình có điểm rác thấp nhất
        if (currentScore < minScore) {
            minScore = currentScore;
            bestPhoms = currentPhoms; bestRem = hand; // bestRem là rác tốt nhất
        }

        int n = hand.size();
        // Duyệt qua tất cả các tập con bằng bitmask (2^n)
        for (int i = 1; i < (1 << n); ++i) {
            vector<Pocket> subset, remaining;
            for (int j = 0; j < n; ++j) {
                if ((i & (1 << j)) != 0) subset.push_back(hand[j]); // Lá được đưa vào nhóm thử nghiệm
                else remaining.push_back(hand[j]); // Lá còn lại
            }
            // Nếu tập con hợp lệ là Phỏm, tiếp tục đệ quy với số bài còn lại
            if (isPhom(subset)) {
                vector<vector<Pocket>> nextPhoms = currentPhoms;
                nextPhoms.push_back(subset);
                checkPhom(remaining, nextPhoms, bestPhoms, bestRem, minScore);
            }
        }
    }

    // Kiểm tra nhanh xem nếu nhận được lá bài p thì có phỏm để ăn không
    bool coTheAn(Pocket p) {
        if (p.getSo() == 0) return false;
        int n = BaiTrenTay.size();
        // Duyệt cặp 2 lá bất kỳ trên tay kết hợp với lá bài p xem có ra phỏm không
        for (int i = 0; i < n - 1; i++) {
            for (int j = i + 1; j < n; j++) {
                vector<Pocket> testnhom = {BaiTrenTay[i], BaiTrenTay[j], p};
                if (isPhom(testnhom)) return true;
            }
        }
        return false;
    }

public:
    void setId(int i) { id = i; }

    string getTen() {
        if (laNguoiChoi) return "Nguoi choi thu " + to_string(id) + " (thong minh)";
        return "Bot thu " + to_string(id);
    }

    // In toàn bộ bài trên tay ra màn hình
    void inBaiTrenTay() {
        for (Pocket p : BaiTrenTay) {
        }
    }

    // Rút n lá bài từ nọc khởi tạo lên tay
    void KhoiTaoBoBai(int n, bobai& BoBai) {
        for (int i = 0; i < n; i++) BaiTrenTay.push_back(BoBai.BocBai());
    }

    void LaNguoiChoi() { laNguoiChoi = true; }

    // Thêm 1 lá bài vào tay (khi bốc hoặc ăn)
    void Add(Pocket p) { BaiTrenTay.push_back(p); }

    int SoLuongBaiNamTrenSan() { return BaiNamTrenSan.size(); }

    // Xử lý logic Ăn bài
    bool return_pick(Pocket p) {
        if (p.getSo() == 0) return false;
        int n = BaiTrenTay.size();
        // Tìm kiếm các tổ hợp trên tay có thể ghép với lá bài đánh ra (p) để tạo Phỏm
        for (int i = 0; i < n - 1; i++) {
            for (int j = i + 1; j < n; j++) {
                vector<Pocket> testnhom = {BaiTrenTay[i], BaiTrenTay[j], p};
                if (isPhom(testnhom)) {
                    // Đưa bộ phỏm tạo được do ăn vào danh sách quản lý riêng
                    // Nhằm ngăn chặn bot vô tình đánh ra các lá bài cấu thành nên phỏm này
                    PhomTuViecAn.push_back(testnhom);
                    return true; // Quyết định ăn
                }
            }
        }
        return false; // Không thể ăn
    }

    // Xử lý logic Đánh rác
    Pocket return_discard() {
        if (BaiTrenTay.empty()) return Pocket();

        // 1. Tạo danh sách các lá bài CẤM đánh (thuộc về các phỏm đã được tạo do việc Ăn bài)
        vector<Pocket> danhSachCamDanh;
        for (auto& phom : PhomTuViecAn) {
            for (auto& la : phom) danhSachCamDanh.push_back(la);
        }

        // 2. Lọc ra những lá bài HỢP LỆ có thể đánh (không bị khóa bởi phỏm ăn)
        vector<Pocket> baiCoTheDanh;
        for (auto& laTay : BaiTrenTay) {
            bool biKhoa = false;
            for (auto& laCam : danhSachCamDanh) {
                if (laTay == laCam) { biKhoa = true; break; }
            }
            if (!biKhoa) baiCoTheDanh.push_back(laTay);
        }

        // Fallback: nếu xui xẻo tất cả bài đều bị khóa (hiếm), đành phải xét đánh tất cả
        if (baiCoTheDanh.empty()) baiCoTheDanh = BaiTrenTay;

        // 3. Sử dụng checkPhom để tìm xem đâu là rác tốt nhất (ít làm mất phỏm sẵn có)
        vector<vector<Pocket>> bestPhoms;
        vector<Pocket> bestRem;
        int minScore = 1e9;
        checkPhom(baiCoTheDanh, {}, bestPhoms, bestRem, minScore);

        Pocket danhLaRac;

        // TÁCH BIỆT CHIẾN THUẬT: Giữa bot thông minh (người chơi) và bot thường
        if (laNguoiChoi) {
            if (!bestRem.empty()) {
                // Nếu đã qua 2 lượt (có bài trên sân >= 2) -> ưu tiên đánh lá rác to nhất (đỡ điểm)
                if (BaiNamTrenSan.size() >= 2) {
                    danhLaRac = bestRem[0];
                    for (Pocket p : bestRem) {
                        if (p.getSo() > danhLaRac.getSo()) danhLaRac = p;
                    }
                } else {
                    // Nếu là lượt đầu/thứ hai: Cố gắng đánh rác hoàn toàn, GIỮ LẠI CẠ
                    vector<Pocket> racHoanToan;
                    vector<Pocket> giuCa;
                    for (size_t i = 0; i < bestRem.size(); i++) {
                        bool inCa = false;
                        for (size_t j = 0; j < bestRem.size(); j++) {
                            if (i == j) continue;
                            if (isCa(bestRem[i], bestRem[j])) { inCa = true; break; }
                        }
                        if (inCa) giuCa.push_back(bestRem[i]); // Chứa các lá bài đang là cạ
                        else racHoanToan.push_back(bestRem[i]); // Các lá bài chỏng chơ (rác thực sự)
                    }

                    // Nếu có rác hoàn toàn, chọn lá to nhất trong đó để đánh
                    if (!racHoanToan.empty()) {
                        danhLaRac = racHoanToan[0];
                        for (Pocket p : racHoanToan) {
                            if (p.getSo() > danhLaRac.getSo()) danhLaRac = p;
                        }
                    } else {
                        // Nếu bắt buộc phải xé cạ, xé cạ có lá bài to nhất
                        danhLaRac = giuCa[0];
                        for (Pocket p : giuCa) {
                            if (p.getSo() > danhLaRac.getSo()) danhLaRac = p;
                        }
                    }
                }
            } else {
                // Đã lên phỏm hết, đánh đại lá cuối cùng có thể đánh
                danhLaRac = baiCoTheDanh.back();
            }
        } else {
            // Chiến thuật của Bot thường: Luôn đánh lá bài to nhất trong đống rác (không màng đến cạ)
            if (!bestRem.empty()) {
                danhLaRac = bestRem[0];
                for (Pocket p : bestRem) {
                    if (p.getSo() > danhLaRac.getSo()) danhLaRac = p;
                }
            } else {
                danhLaRac = baiCoTheDanh.back();
            }
        }

        // Xóa lá bài vừa đánh ra khỏi tay
        for (auto it = BaiTrenTay.begin(); it != BaiTrenTay.end(); ++it) {
            if (*it == danhLaRac) { BaiTrenTay.erase(it); break; }
        }
        return danhLaRac;
    }

    // Thêm lá bài vào danh sách rác đã đánh
    void Add_BaiNamTrenSan(Pocket p) { BaiNamTrenSan.push_back(p); }

    // Xóa lá bài khỏi danh sách rác khi bị người khác ăn (Bị ăn chốt hoặc ăn thường)
    void Erase_BaiNamTrenSan(Pocket p) {
        for (auto it = BaiNamTrenSan.begin(); it != BaiNamTrenSan.end(); ++it) {
            if (*it == p) { BaiNamTrenSan.erase(it); break; }
        }
    }

    // Logic Hạ Phỏm ở cuối ván
    void HaBai(vector<vector<Pocket>>& PhomChungToanBan) {
        vector<vector<Pocket>> bestPhoms;
        vector<Pocket> bestRem;
        int minScore = 1e9;
        checkPhom(BaiTrenTay, {}, bestPhoms, bestRem, minScore); // Tìm bộ phỏm tốt nhất

        PhomDaHa = bestPhoms; // Cập nhật phỏm từ tay
        for(auto& pAn : PhomTuViecAn) PhomDaHa.push_back(pAn); // Cập nhật luôn phỏm từ việc ăn
        BaiTrenTay = bestRem; // Những lá còn lại trên tay

        if(!PhomDaHa.empty()){
        }

        // Đưa các phỏm vừa hạ lên khu vực chung của bàn để mọi người có thể gửi bài
        for (auto phom : PhomDaHa) {
            PhomChungToanBan.push_back(phom);
        }
    }

    // Logic Gửi Bài (ghép rác của mình vào phỏm người khác đã hạ)
    void GuiBai(vector<vector<Pocket>>& PhomChungToanBan) {
        bool coTheGuiTiep = true;
        // Lặp cho đến khi không còn gửi được lá nào nữa
        while (coTheGuiTiep && !BaiTrenTay.empty()) {
            coTheGuiTiep = false;
            for (auto it = BaiTrenTay.begin(); it != BaiTrenTay.end(); ) {
                bool daGuiLaNay = false;
                for (auto& phomChung : PhomChungToanBan) {
                    vector<Pocket> testPhom = phomChung;
                    testPhom.push_back(*it); // Thử ghép lá bài rác vào phỏm chung
                    if (isPhom(testPhom)) {  // Nếu vẫn giữ nguyên tính chất Phỏm
                        phomChung.push_back(*it); // Chấp nhận gửi
                        it = BaiTrenTay.erase(it); // Xóa khỏi tay
                        daGuiLaNay = true;
                        coTheGuiTiep = true; // Báo hiệu đã gửi thành công, sẽ check tiếp
                        break;
                    }
                }
                if (!daGuiLaNay) ++it; // Nếu không gửi được, xét tiếp lá khác
            }
        }
    }

    // Tính điểm của các lá rác còn lại (A=1, ..., K=13)
    int tinhDiem() {
        int score = 0;
        for (Pocket p : BaiTrenTay) score += p.getSo();
        return score;
    }

    // Kiểm tra điều kiện Ù (Hết rác, tất cả bài trên tay đều ghép thành phỏm)
    bool U() {
        vector<vector<Pocket>> bestPhoms; vector<Pocket> bestRem;
        int minScore = 1e9;
        checkPhom(BaiTrenTay, {}, bestPhoms, bestRem, minScore);
        if (bestRem.empty()) return true; // Ù bình thường (hoặc ù khan)
        // Nếu người đánh có 10 lá và chỉ dư đúng 1 lá rác -> cũng là Ù
        if (BaiTrenTay.size() == 10 && bestRem.size() == 1) return true;
        return false;
    }
};


int VongChoi(int chienthuatNguoiChoisesudung) {

    vector<Pocket> BaiDaLo; // Lưu vết các bài đã bị lật ra bàn (rác hoặc phỏm)
    // Biến toàn cục lưu trữ tất cả các Phỏm đã được hạ trên bàn (dùng để kiểm tra gửi bài)
    vector<vector<Pocket>> PhomChungToanBan;
    PhomChungToanBan.clear();
    bobai BoBai;
    vector<int> Score_ToTal(4, 0); // Mảng lưu điểm của 4 người
    int Cuoc[4] = {1, 1, 1, 1};    // Hệ số cược ban đầu (cơ bản)

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(1, 4);
    int ViTriNguoiChoi = dist(rng);

    vector<Player> MangNguoiChoi(4); // Tạo 4 người chơi

    if(chienthuatNguoiChoisesudung == 1) {
        MangNguoiChoi[ViTriNguoiChoi - 1].LaNguoiChoi(); // Gán 1 slot làm người thông minh
    } else {
        // Nếu không chọn chiến thuật thông minh, tất cả đều là bot thường
    }

    for(int i = 0; i < 4; i++){
        MangNguoiChoi[i].setId(i + 1);
    }
    // Luật chia bài: Người đi đầu tiên cầm 10 lá
    MangNguoiChoi[0].KhoiTaoBoBai(10, BoBai);
    // 3 người còn lại cầm 9 lá
    for (int i = 1; i < 4; i++) {
        MangNguoiChoi[i].KhoiTaoBoBai(9, BoBai);
    }

    for(int i = 0; i < 4; i++){
        MangNguoiChoi[i].inBaiTrenTay();
    }

    Pocket NullPocket;
    int Luot = 0; // Lượt đánh bắt đầu từ index 0
    Pocket LaBaiCoTheLay = NullPocket; // Trạng thái lá bài người trước đánh ra (để ăn)
    bool DieuKienKetThuc = 0; // Cờ kiểm tra kết thúc ván (1 là hết ván)

    // vòng lặp chính thức
    while (DieuKienKetThuc == 0) {
        Player* player = &MangNguoiChoi[Luot];

        player->inBaiTrenTay();

        // Xử lý ăn bài (Nếu có lá bài tỳ từ người chơi trước)
        if (LaBaiCoTheLay != NullPocket) {
            // Không được ăn nếu đã đánh 4 lá (tức là đến lượt hạ bài)
            if (player->SoLuongBaiNamTrenSan() != 4) {
                // Kiểm tra ăn bài
                if (player->return_pick(LaBaiCoTheLay)) {
                    player->Add(LaBaiCoTheLay); // Thêm lá vừa ăn vào tay

                    int j = Luot - 1; // Tìm index của người vừa bị ăn
                    if (j < 0) j += 4;
                    MangNguoiChoi[j].Erase_BaiNamTrenSan(LaBaiCoTheLay); // Xóa lá bài đó trên mâm của người bị ăn

                    // Luật đền tiền khi bị ăn
                    if (MangNguoiChoi[j].SoLuongBaiNamTrenSan() == 4) {
                        // Bị ăn CHỐT (lá thứ 4) -> Trừ nhiều cược
                        Cuoc[j] -= 4;
                        Cuoc[Luot] += 4;
                    } else {
                        // Bị ăn bình thường
                        Cuoc[j] -= 1;
                        Cuoc[Luot] += 1;
                    }

                    LaBaiCoTheLay = NullPocket; // Đã ăn xong, dọn trống bài tỳ
                } else {
                    // không ăn được thì phải bốc
                    // lưu lại lá bài bốc để in ra màn hình
                    Pocket laBoc = BoBai.BocBai();
                    player->Add(laBoc);
                    LaBaiCoTheLay = NullPocket;
                }

                // Kiểm tra Ù sau khi Bốc hoặc Ăn
                if (player->U()) {
                    Score_ToTal[Luot] = -100000; // Đặt điểm cực âm để xác định là người Ù
                    break; // Kết thúc ván ngay lập tức
                }
            }
        }

        // Xử lý Đánh bài
        if (LaBaiCoTheLay == NullPocket) {
            LaBaiCoTheLay = player->return_discard(); // Lấy lá rác đánh ra từ logic
            BaiDaLo.push_back(LaBaiCoTheLay); // Ghi log
            player->Add_BaiNamTrenSan(LaBaiCoTheLay); // Chuyển lá bài xuống sân rác của người này
        }

        // Kiểm tra Ù sau khi đánh (trường hợp Ù khan hoặc đánh ra vô tình Ù)
        if (player->U()) {
            Score_ToTal[Luot] = -100000;
            break;
        }

        // Kiểm tra logic Hạ Phỏm của NGƯỜI TRƯỚC ĐÓ
        // (Luật Phỏm: Khi đánh lá bài thứ 4 đi thì bắt đầu hạ bài)
        int j = Luot - 1;
        if (j < 0) j += 4;
        if (MangNguoiChoi[j].SoLuongBaiNamTrenSan() == 4) {
            MangNguoiChoi[j].HaBai(PhomChungToanBan); // Hạ các phỏm
            MangNguoiChoi[j].GuiBai(PhomChungToanBan); // Tìm xem có gửi bài được vào phỏm của người hạ trước không
            Score_ToTal[j] = MangNguoiChoi[j].tinhDiem(); // Tính điểm rác còn lại
        }

        Luot++;
        if (Luot >= 4) Luot -= 4;

        //Kiem tra dieu kien ket thuc vong choi
        DieuKienKetThuc = 1;
        for (int i = 0; i < 4; i++) {
            if (MangNguoiChoi[i].SoLuongBaiNamTrenSan() < 4) {
                DieuKienKetThuc = 0; // Chỉ cần 1 người chưa đánh đủ 4 lá thì vẫn lặp
            }
        }
    }

    // Xử lý chung chi tiền cược khi có người Ù
    if (Score_ToTal[Luot] == -100000) {
        if (Luot != ViTriNguoiChoi - 1) {
            Cuoc[ViTriNguoiChoi - 1] -= 15; // Phạt tiền ù (Người khác ù)
        } else {
            Cuoc[Luot] += 45; // Thưởng tiền ù (Mình ù)
        }
    } else {
        // Ván kết thúc do bốc hết nọc, xử lý xét hạng bằng cách so sánh điểm rác (ít điểm là thắng)
        vector<int> XepHang = Score_ToTal;
        sort(XepHang.begin(), XepHang.end()); // Sắp xếp điểm tăng dần (Nhất, Nhì, Ba, Bét)
        for (int i = 0; i < 4; i++) {
            if (Score_ToTal[i] == XepHang[0]) Cuoc[i] += 6;       // Nhất (ăn 6)
            else if (Score_ToTal[i] == XepHang[1]) Cuoc[i] -= 1;  // Nhì (thua 1)
            else if (Score_ToTal[i] == XepHang[2]) Cuoc[i] -= 2;  // Ba (thua 2)
            else if (Score_ToTal[i] == XepHang[3]) Cuoc[i] -= 3;  // Bét (thua 3)
        }
    }

    // In ra kết quả tiền cược của cá nhân Người chơi thực (User)
    if (Cuoc[ViTriNguoiChoi - 1] > 1) {
    } else if (Cuoc[ViTriNguoiChoi - 1] < 0) {
    } else {
    }

    return Cuoc[ViTriNguoiChoi - 1]; // Trả về kết quả cược của người chơi thực
}
ofstream fo("KetQuaMoPhong.txt");
int main(){
    int i;
    cout<<"Chien luoc nguoi choi se su dung (1,2): ";
    cin>>i;
    int n=100;
    cout<<"Nhap so lan mo phong: ";
    cin>>n;
    vector<int> results;
    long long sum = 0;
    while(n--){
        results.push_back(VongChoi(i));
        sum += results.back();
    }
    fo<< "TB cong: " << (double)sum / results.size() << endl;
    fo<< "Ket qua tung lan mo phong: " << endl;
    for(int result : results){
        fo << result << ' ';
    }
    fo.close();
}
