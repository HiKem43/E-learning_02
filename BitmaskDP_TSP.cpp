#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <climits>
#include <fstream>
using namespace std;

const long long INF = LLONG_MAX / 4;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // Doc du lieu tu input.txt
    ifstream fin("input.txt");
    if (!fin) {
        cerr << "Khong mo duoc file input.txt\n";
        return 1;
    }

    int n;
    fin >> n;

    if (n < 2 || n > 15) {
        cerr << "So thanh pho phai nam trong khoang 2 den 15.\n";
        return 1;
    }

    vector<vector<long long>> C(n, vector<long long>(n));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (!(fin >> C[i][j])) {
                cerr << "Du lieu ma tran khong hop le.\n";
                return 1;
            }
        }
    }
    fin.close();

    auto start = chrono::high_resolution_clock::now();

    const int FULL = (1 << n) - 1;

    // dp[mask][u]: chi phi nho nhat tu thanh pho 0,
    // tham cac thanh pho trong mask va ket thuc tai u.
    vector<vector<long long>> dp(1 << n, vector<long long>(n, INF));

    // parent[mask][u]: thanh pho dung truoc u trong trang thai toi uu.
    vector<vector<int>> parent(1 << n, vector<int>(n, -1));

    // Trang thai ban dau: chi co thanh pho 0 duoc tham.
    dp[1][0] = 0;

    for (int mask = 1; mask <= FULL; ++mask) {
        // Moi trang thai hop le phai co thanh pho 0.
        if ((mask & 1) == 0) continue;

        for (int u = 0; u < n; ++u) {
            if ((mask & (1 << u)) == 0 || dp[mask][u] == INF)
                continue;

            for (int v = 0; v < n; ++v) {
                // Chi di den thanh pho chua duoc tham.
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

    // Chon thanh pho cuoi cung sao cho chi phi quay ve 0 nho nhat.
    long long answer = INF;
    int last = -1;

    for (int u = 1; u < n; ++u) {
        if (dp[FULL][u] == INF) continue;

        long long totalCost = dp[FULL][u] + C[u][0];
        if (totalCost < answer) {
            answer = totalCost;
            last = u;
        }
    }

    auto finish = chrono::high_resolution_clock::now();
    long long elapsed =
        chrono::duration_cast<chrono::microseconds>(finish - start).count();

    if (last == -1) {
        cout << "Khong tim thay lo trinh hop le.\n";
        return 0;
    }

    // Truy vet lo trinh tu parent.
    vector<int> path;
    int mask = FULL;
    int u = last;

    while (u != -1) {
        path.push_back(u);
        int previous = parent[mask][u];
        mask ^= (1 << u);
        u = previous;
    }

    reverse(path.begin(), path.end());
    path.push_back(0);

    // Xuat ra man hinh va output.txt.
    ofstream fout("output.txt");
    auto printResult = [&](ostream& os) {
        os << "===== BITMASK DP =====\n";
        os << "Chi phi toi thieu: " << answer << '\n';
        os << "Lo trinh: ";
        for (size_t i = 0; i < path.size(); ++i) {
            if (i) os << " -> ";
            os << path[i];
        }
        os << "\nThoi gian: " << elapsed << " microseconds\n";
    };

    printResult(cout);
    if (fout) printResult(fout);

    return 0;
}
