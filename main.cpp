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
// PHẦN 1: THUẬT TOÁN BACKTRACKING (có cắt tỉa nhánh cận)
// ============================================================================
namespace {
class TSPBacktracking {
private:
    int n;
    const vector<vector<int>>& cost;
    vector<long long> minOut;   // minOut[i]: cạnh ra nhỏ nhất của thành phố i
    vector<bool> visited;
    vector<int> path;           // lộ trình hiện tại
    vector<int> bestPath;       // lộ trình tốt nhất
    long long bestCost;

    void backtrack(int curr, int count, long long curCost, long long unvisitedMin) {
        // Đã thăm đủ n thành phố -> quay về 0
        if (count == n) {
            long long total = curCost + cost[curr][0];
            if (total < bestCost) {
                bestCost = total;
                bestPath = path;
                bestPath.push_back(0);
            }
            return;
        }

        // Cắt tỉa 2: cận dưới
        if (curCost + minOut[curr] + unvisitedMin >= bestCost) return;

        for (int next = 0; next < n; ++next) {
            if (visited[next]) continue;

            long long newCost = curCost + cost[curr][next];
            if (newCost >= bestCost) continue; // Cắt tỉa 1: vượt kỷ lục

            visited[next] = true;
            path.push_back(next);

            backtrack(next, count + 1, newCost, unvisitedMin - minOut[next]);

            path.pop_back();
            visited[next] = false;
        }
    }

public:
    TSPBacktracking(int n, const vector<vector<int>>& C)
        : n(n), cost(C), visited(n, false),
          bestCost(numeric_limits<long long>::max()) {
        minOut.assign(n, numeric_limits<long long>::max());
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                if (i != j) minOut[i] = min(minOut[i], (long long)cost[i][j]);
    }

    void solve() {
        visited[0] = true;
        path.push_back(0);

        long long unvisitedMin = 0;
        for (int i = 1; i < n; ++i) unvisitedMin += minOut[i];

        backtrack(0, 1, 0, unvisitedMin);
    }

    long long getBestCost() const { return bestCost; }
    const vector<int>& getBestPath() const { return bestPath; }
};
} // namespace

TSPResult solveBacktracking(int n, const vector<vector<int>>& C) {
    TSPResult res;
    res.strategy_name = "====BACKTRACKING====";
    res.min_cost = INF;
    res.is_cost_min_label = true;

TSPBacktracking solver(n, C);
    solver.solve();

    if (solver.getBestCost() < INF) {
        res.min_cost = (int)solver.getBestCost();
        res.path = solver.getBestPath();   // gồm n + 1 phần tử: 0 -> ... -> 0
    }
    return res;
}
// ============================================================================
// PHAN 2: THUAT TOAN BITMASK DP
// ============================================================================
TSPResult solveBitmaskDP(int n, const vector<vector<int>>& C) {
    TSPResult res;
    res.strategy_name = "====BITMASK DP====";
    res.min_cost = INF;
    res.is_cost_min_label = true;

    int totalMask = 1 << n;
    const long long INF_DP = numeric_limits<long long>::max() / 2;

    vector<vector<long long>> dp(
        totalMask, vector<long long>(n, INF_DP)
    );

    // Luu thanh pho truoc do de truy vet lo trinh
    vector<vector<int>> parent(
        totalMask, vector<int>(n, -1)
    );

    // Bat dau tai thanh pho 0
    dp[1][0] = 0;

    // Duyet cac trang thai
    for (int mask = 1; mask < totalMask; ++mask) {
        if (!(mask & 1)) continue;

        for (int u = 0; u < n; ++u) {
            if (!(mask & (1 << u))) continue;
            if (dp[mask][u] == INF_DP) continue;

            for (int v = 0; v < n; ++v) {
                if (mask & (1 << v)) continue;

                int newMask = mask | (1 << v);
                long long newCost = dp[mask][u] + C[u][v];

                if (newCost < dp[newMask][v]) {
                    dp[newMask][v] = newCost;
                    parent[newMask][v] = u;
                }
            }
        }
    }

    // Tim chi phi nho nhat khi quay ve thanh pho 0
    int fullMask = totalMask - 1;
    long long bestCost = INF_DP;
    int bestLast = -1;

    for (int u = 1; u < n; ++u) {
        long long totalCost = dp[fullMask][u] + C[u][0];

        if (totalCost < bestCost) {
            bestCost = totalCost;
            bestLast = u;
        }
    }

    // Truy vet lo trinh
    if (bestLast != -1 && bestCost < INF_DP) {
        res.min_cost = (int)bestCost;

        vector<int> reversePath;
        int mask = fullMask;
        int curr = bestLast;

        while (curr != 0) {
            reversePath.push_back(curr);

            int prev = parent[mask][curr];
            mask ^= (1 << curr);
            curr = prev;
        }

        res.path.push_back(0);

        for (int i = (int)reversePath.size() - 1; i >= 0; --i) {
            res.path.push_back(reversePath[i]);
        }

        res.path.push_back(0);
    }

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

// Kiem tra tinh hop le cua du lieu dau vao
bool validateInputData(int n, const vector<vector<int>>& C) {
    // Kiem tra so thanh pho
    if (n < 2 || n > 15) {
        cout << "So thanh pho n khong hop le (2 <= n <= 15)!\n";
        return false;
    }

    // Kiem tra kich thuoc ma tran
    if (C.size() != (size_t)n) {
        cout << "Kich thuoc ma tran khong hop le!\n";
        return false;
    }

    for (int i = 0; i < n; ++i) {
        if (C[i].size() != (size_t)n) {
            cout << "Ma tran khong du kich thuoc!\n";
            return false;
        }

        for (int j = 0; j < n; ++j) {
            // Duong cheo chinh phai bang 0
            if (i == j && C[i][j] != 0) {
                cout << "Duong cheo chinh phai bang 0!\n";
                return false;
            }

            // Chi phi giua hai thanh pho khac nhau phai duong
            if (i != j && C[i][j] <= 0) {
                cout << "Chi phi khong hop le!\n";
                return false;
            }
        }
    }

    return true;
}

// Doc du lieu tu file
bool readInputFromFile(const string& filename, int& n,
                       vector<vector<int>>& C) {
    ifstream fin(filename);

    if (!fin.is_open()) {
        return false;
    }

    // Doc so thanh pho
    if (!(fin >> n) || n < 2 || n > 15) {
        cout << "So thanh pho trong file khong hop le!\n";
        fin.close();
        return false;
    }

    C.assign(n, vector<int>(n, 0));

    // Doc ma tran chi phi
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (!(fin >> C[i][j])) {
                cout << "File input bi thieu hoac co du lieu khong hop le!\n";
                fin.close();
                return false;
            }
        }
    }

    // Kiem tra co du lieu thua sau ma tran hay khong
    string extra;

    if (fin >> extra) {
        cout << "File input co du lieu thua sau ma tran!\n";
        fin.close();
        return false;
    }

    fin.close();

    // Kiem tra tinh hop le cua ma tran
    return validateInputData(n, C);
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


// Kiem tra tinh hop le cua ket qua thuat toan
bool checkTSPResult(const TSPResult& res, int n,
                    const vector<vector<int>>& C) {

    // Kiem tra co tim duoc lo trinh hay khong
    if (res.min_cost == INF || res.path.size() != (size_t)n + 1) {
        return false;
    }

    // Lo trinh phai bat dau va ket thuc tai thanh pho 0
    if (res.path.front() != 0 || res.path.back() != 0) {
        return false;
    }

    vector<bool> visited(n, false);
    long long totalCost = 0;

    // Kiem tra cac thanh pho va tinh lai chi phi
    for (int i = 0; i < n; ++i) {
        int city = res.path[i];

        if (city < 0 || city >= n || visited[city]) {
            return false;
        }

        visited[city] = true;
        totalCost += C[res.path[i]][res.path[i + 1]];
    }

    // Dam bao da tham tat ca thanh pho
    for (int i = 0; i < n; ++i) {
        if (!visited[i]) {
            return false;
        }
    }

    // Chi phi tinh lai phai bang chi phi thuat toan tra ve
    return totalCost == res.min_cost;
}

int main(int argc, char* argv[]) {

    // Ham chay mot bo du lieu TSP
    auto runTest = [](const string& input_file,
                      const string& output_file) {

        int n = 0;
        vector<vector<int>> C;

        cout << "\n====================================\n";
        cout << "Dang chay: " << input_file << "\n";
        cout << "====================================\n";

        if (!readInputFromFile(input_file, n, C)) {
            cerr << "Khong the doc file: " << input_file << "\n";
            return;
        }

        ofstream fout(output_file);

        if (!fout.is_open()) {
            cerr << "Khong the tao file: " << output_file << "\n";
            return;
        }

        // Chay 3 thuat toan tren cung bo du lieu
        TSPResult resBacktracking =
            executeAndMeasure(solveBacktracking, n, C);
        outputResult(resBacktracking, fout);

        TSPResult resBitmaskDP =
            executeAndMeasure(solveBitmaskDP, n, C);
        outputResult(resBitmaskDP, fout);

        TSPResult resGreedy =
            executeAndMeasure(solveGreedy, n, C);
        outputResult(resGreedy, fout);
        // Kiem tra tinh dung dan cua 3 thuat toan
bool checkBT = checkTSPResult(resBacktracking, n, C);
bool checkDP = checkTSPResult(resBitmaskDP, n, C);
bool checkGreedy = checkTSPResult(resGreedy, n, C);

cout << "\n===== KIEM TRA KET QUA =====\n";

cout << "Backtracking: "
     << (checkBT ? "PASS" : "FAIL") << "\n";

cout << "Bitmask DP: "
     << (checkDP ? "PASS" : "FAIL") << "\n";

cout << "Greedy: "
     << (checkGreedy ? "PASS" : "FAIL") << "\n";

// So sanh chi phi toi uu
if (checkBT && checkDP) {
    if (resBacktracking.min_cost == resBitmaskDP.min_cost) {
        cout << "So sanh Backtracking va DP: PASS\n";
    } else {
        cout << "So sanh Backtracking va DP: FAIL\n";
    }
}
        fout.close();

        cout << "\nDa luu ket qua vao: " << output_file << "\n";
    };

    // Neu co tham so dong lenh thi chay mot bo test
    if (argc >= 2) {
        string input_file = argv[1];
        string output_file =
            (argc >= 3) ? argv[2] : "output.txt";

        runTest(input_file, output_file);
    }
    else {
        // Tu dong chay tat ca bo du lieu
        int testSizes[] = {4, 6, 8, 10, 12, 14, 15};

        for (int i = 0; i < 7; ++i) {
            string input_file =
                "input/input_" + to_string(testSizes[i]) + ".txt";

            string output_file =
                "output/output_" + to_string(testSizes[i]) + ".txt";

            runTest(input_file, output_file);
        }
    }

    return 0;
}
