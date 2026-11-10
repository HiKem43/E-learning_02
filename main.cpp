#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>
#include <string>
#include <limits>
#include <algorithm>

using namespace std;
using namespace std::chrono;

// Hằng số biểu diễn giá trị vô cùng
const int INF = 1e9;
struct TSPResult {
    string strategy_name;   // Tên chiến lược: BACKTRACKING, BITMASK DP, GREEDY
    int min_cost;           // Chi phí tối thiểu (hoặc chi phí tìm được)
    vector<int> path;       // Lộ trình đi qua các thành phố (0 -> ... -> 0)
    long long time_ms;      // Thời gian chạy thực nghiệm (ms)
    bool is_cost_min_label; // true: in "Chi phi toi thieu:", false: in "Chi phi:" (theo mẫu đề bài)
};

// ============================================================================
// PHẦN 1: THUẬT TOÁN BACKTRACKING
// ============================================================================
/*
 * - Input: Số thành phố n (2 <= n <= 15), ma trận chi phí C[n][n]
 * - Yêu cầu: Xuất phát từ thành phố 0, đi qua tất cả thành phố đúng 1 lần rồi quay về 0.
 * - Lưu chi phí tốt nhất vào res.min_cost và lộ trình vào res.path (gồm n + 1 phần tử, bắt đầu và kết thúc là 0).
 */
TSPResult solveBacktracking(int n, const vector<vector<int>>& C) {
    TSPResult res;
    res.strategy_name = "====BACKTRACKING====";
    res.min_cost = INF;
    res.is_cost_min_label = true;
    // TO DO:

    
    return res;
}

// ============================================================================
// PHẦN 2: THUẬT TOÁN BITMASK DP
// ============================================================================
/*
 * - Input: Số thành phố n (2 <= n <= 15), ma trận chi phí C[n][n]
 * - Yêu cầu: Sử dụng Quy hoạch động trạng thái Bitmask (dp[mask][u]) tìm hành trình ngắn nhất
 *   và truy vết (trace) lại lộ trình từ 0 -> ... -> 0.
 * - Lưu chi phí tối ưu vào res.min_cost và lộ trình vào res.path.
 */
TSPResult solveBitmaskDP(int n, const vector<vector<int>>& C) {
    TSPResult res;
    res.strategy_name = "====BITMASK DP====";
    res.min_cost = INF;
    res.is_cost_min_label = true;

    // TO DO:
    
    return res;
}

// ============================================================================
// PHẦN 3: THUẬT TOÁN GREEDY 
// ============================================================================
/*
 * - Input: Số thành phố n (2 <= n <= 15), ma trận chi phí C[n][n]
 * - Yêu cầu: Từ thành phố hiện tại (bắt đầu từ 0), luôn chọn thành phố chưa thăm có chi phí nhỏ nhất.
 *   Sau khi thăm đủ n thành phố thì quay về 0.
 * - Lưu tổng chi phí vào res.min_cost và lộ trình vào res.path.
 */
TSPResult solveGreedy(int n, const vector<vector<int>>& C) {
    TSPResult res;
    res.strategy_name = "==== GREEDY ====";
    res.min_cost = 0;
    res.is_cost_min_label = false; // Theo ví dụ đề bài, Greedy in "Chi phi: "

    vector<bool> visited(n, false);
    res.path.reserve(n + 1);

    int curr = 0; // Xuất phát cố định từ thành phố 0
    visited[curr] = true;
    res.path.push_back(curr);

    // Lần lượt tìm n - 1 thành phố tiếp theo theo chiến lược Láng giềng gần nhất
    for (int step = 1; step < n; ++step) {
        int next_city = -1;
        int min_edge = INF;

        for (int j = 0; j < n; ++j) {
            if (!visited[j] && C[curr][j] > 0 && C[curr][j] < min_edge) {
                min_edge = C[curr][j];
                next_city = j;
            }
        }

        // Nếu không tìm được đỉnh tiếp theo hợp lệ
        if (next_city == -1) {
            res.min_cost = INF;
            res.path.clear();
            return res;
        }

        visited[next_city] = true;
        res.path.push_back(next_city);
        res.min_cost += min_edge;
        curr = next_city;
    }

    // Quay trở về thành phố xuất phát (thành phố 0)
    if (C[curr][0] > 0) {
        res.min_cost += C[curr][0];
        res.path.push_back(0);
    } else {
        res.min_cost = INF;
        res.path.clear();
    }

    return res;
}

// ============================================================================
// PHẦN 4: NHẬP/XUẤT, ĐO THỜI GIAN & TÍCH HỢP)
// ============================================================================

// Kiểm tra tham số dòng lệnh có phải tên file hợp lệ không (loại bỏ '2>CON', '1>CON', '<CON' của GDB)
bool isValidFilename(const string& arg) {
    if (arg.empty()) return false;
    return arg.find('>') == string::npos && arg.find('<') == string::npos;
}

// Đọc dữ liệu từ file (hoặc luồng cin)
bool readInputFromFile(const string& filename, int& n, vector<vector<int>>& C) {
    ifstream fin(filename);
    if (!fin.is_open()) return false;

    if (!(fin >> n) || n < 2 || n > 15) {
        fin.close();
        return false;
    }

    C.assign(n, vector<int>(n, 0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            fin >> C[i][j];
        }
    }
    fin.close();
    return true;
}

// Hàm đo thời gian thực thi của từng thuật toán
template <typename Func>
TSPResult executeAndMeasure(Func solver, int n, const vector<vector<int>>& C) {
    auto start_time = high_resolution_clock::now();
    TSPResult res = solver(n, C);
    auto end_time = high_resolution_clock::now();
    res.time_ms = duration_cast<milliseconds>(end_time - start_time).count();
    return res;
}

// Xuất kết quả ra cả màn hình (console) và file .txt theo đúng định dạng đề bài
void outputResult(const TSPResult& res, ofstream& fout) {
    // 1. Xuất ra màn hình
    cout << res.strategy_name << "\n";
    if (res.is_cost_min_label) {
        cout << "Chi phi toi thieu: " << res.min_cost << "\n";
    } else {
        cout << "Chi phi: " << res.min_cost << "\n";
    }
    cout << "Lo trinh: ";
    for (size_t i = 0; i < res.path.size(); ++i) {
        cout << res.path[i] << (i + 1 < res.path.size() ? " " : "");
    }
    cout << "\n";
    cout << "Thoi gian: " << res.time_ms << " ms\n";

    // 2. Xuất ra file .txt
    if (fout.is_open()) {
        fout << res.strategy_name << "\n";
        if (res.is_cost_min_label) {
            fout << "Chi phi toi thieu: " << res.min_cost << "\n";
        } else {
            fout << "Chi phi: " << res.min_cost << "\n";
        }
        fout << "Lo trinh: ";
        for (size_t i = 0; i < res.path.size(); ++i) {
            fout << res.path[i] << (i + 1 < res.path.size() ? " " : "");
        }
        fout << "\n";
        fout << "Thoi gian: " << res.time_ms << " ms\n";
    }
}

int main(int argc, char* argv[]) {
    // Hỗ trợ truyền tên file input/output qua tham số dòng lệnh để Hạnh & Da dễ chạy nhiều bộ test
    string input_file = (argc >= 2 && isValidFilename(argv[1])) ? argv[1] : "input.txt";
    string output_file = (argc >= 3 && isValidFilename(argv[2])) ? argv[2] : "output.txt";

    int n = 0;
    vector<vector<int>> C;

    // Nếu không tìm thấy file input.txt, cho phép nhập trực tiếp từ bàn phím
    if (!readInputFromFile(input_file, n, C)) {
        cout << "[Thong bao] Khong tim thay file '" << input_file 
             << "'. Vui long nhap n va ma tran chi phi tu ban phim:\n";
        if (!(cin >> n) || n < 2 || n > 15) {
            cerr << "So thanh pho n khong hop le (2 <= n <= 15)!\n";
            return 1;
        }
        C.assign(n, vector<int>(n, 0));
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                cin >> C[i][j];
            }
        }
    }

    ofstream fout(output_file);
    if (!fout.is_open()) {
        cerr << "Khong the tao file xuat '" << output_file << "'!\n";
        return 1;
    }

    // Chạy độc lập 3 chiến lược trên cùng bộ dữ liệu và xuất kết quả
    TSPResult resBacktracking = executeAndMeasure(solveBacktracking, n, C);
    outputResult(resBacktracking, fout);

    TSPResult resBitmaskDP = executeAndMeasure(solveBitmaskDP, n, C);
    outputResult(resBitmaskDP, fout);

    TSPResult resGreedy = executeAndMeasure(solveGreedy, n, C);
    outputResult(resGreedy, fout);

    fout.close();
    return 0;
}
