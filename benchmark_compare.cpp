#include <algorithm>
#include <chrono>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>
using namespace std;

struct Account {
    int accountNumber;
    string holderName;
    double balance;
    int pin;
    string type;
};

static string extractStringValue(const string &obj, const string &key) {
    string pat = '"' + key + '"';
    size_t p = obj.find(pat);
    if (p == string::npos) return "";
    p = obj.find(':', p);
    if (p == string::npos) return "";
    p++;
    while (p < obj.size() && isspace((unsigned char)obj[p])) p++;
    if (p < obj.size() && obj[p] == '"') {
        size_t q = obj.find('"', p+1);
        if (q == string::npos) return "";
        return obj.substr(p+1, q-(p+1));
    }
    return "";
}

static double extractNumberValue(const string &obj, const string &key) {
    string pat = '"' + key + '"';
    size_t p = obj.find(pat);
    if (p == string::npos) return 0.0;
    p = obj.find(':', p);
    if (p == string::npos) return 0.0;
    p++;
    while (p < obj.size() && isspace((unsigned char)obj[p])) p++;
    size_t q = p;
    while (q < obj.size() && (isdigit((unsigned char)obj[q]) || obj[q]=='-' || obj[q]=='.')) q++;
    if (q<=p) return 0.0;
    return stod(obj.substr(p,q-p));
}

vector<Account> loadAccounts(const string &filename) {
    vector<Account> accounts;
    ifstream f(filename);
    if (!f) {
        cerr << "Unable to open " << filename << "\n";
        return accounts;
    }
    string s, content;
    while (getline(f, s)) content += s;
    f.close();
    size_t pos = 0;
    while (true) {
        size_t l = content.find('{', pos);
        if (l == string::npos) break;
        size_t r = content.find('}', l);
        if (r == string::npos) break;
        string obj = content.substr(l, r-l+1);
        Account a;
        a.accountNumber = static_cast<int>(extractNumberValue(obj, "accountNumber"));
        a.holderName = extractStringValue(obj, "holderName");
        a.balance = extractNumberValue(obj, "balance");
        a.pin = static_cast<int>(extractNumberValue(obj, "pin"));
        a.type = extractStringValue(obj, "type");
        accounts.push_back(move(a));
        pos = r+1;
    }
    return accounts;
}

// insertion sort by accountNumber
void insertionSortById(vector<Account> &arr) {
    for (size_t i = 1; i < arr.size(); ++i) {
        Account key = arr[i];
        int j = (int)i - 1;
        while (j >= 0 && arr[j].accountNumber > key.accountNumber) {
            arr[j+1] = arr[j];
            --j;
        }
        arr[j+1] = key;
    }
}

int linearSearchById(const vector<Account> &arr, int id) {
    for (size_t i = 0; i < arr.size(); ++i) if (arr[i].accountNumber == id) return (int)i;
    return -1;
}

int binarySearchById(const vector<Account> &arr, int id) {
    int lo = 0, hi = (int)arr.size() - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (arr[mid].accountNumber == id) return mid;
        if (arr[mid].accountNumber < id) lo = mid + 1;
        else hi = mid - 1;
    }
    return -1;
}

int main() {
    string filename = "accounts.json";
    cout << "Loading accounts from " << filename << "...\n";
    auto accounts = loadAccounts(filename);
    size_t N = accounts.size();
    if (N == 0) { cerr << "No accounts loaded. Ensure accounts.json exists.\n"; return 1; }
    cout << "Loaded " << N << " accounts.\n";

    // approximate memory footprint
    size_t vec_capacity = accounts.capacity();
    size_t mem_accounts = vec_capacity * sizeof(Account);
    size_t mem_strings = 0;
    for (const auto &a: accounts) mem_strings += a.holderName.capacity();
    cout << "Approx memory (accounts array): " << mem_accounts << " bytes\n";
    cout << "Approx memory (holderName strings capacity): " << mem_strings << " bytes\n";
    cout << "Approx total (rough): " << (mem_accounts + mem_strings) << " bytes\n\n";

    // Prepare test keys for searching
    const int M = 10000; // number of searches
    vector<int> queries; queries.reserve(M);
    std::mt19937 rng(123456);
    uniform_int_distribution<int> dist(0, (int)N-1);
    for (int i = 0; i < M; ++i) {
        queries.push_back(accounts[dist(rng)].accountNumber);
    }

    // FAST: std::sort + binary search
    auto arr_fast = accounts; // copy
    auto t1 = chrono::high_resolution_clock::now();
    sort(arr_fast.begin(), arr_fast.end(), [](const Account &a, const Account &b){ return a.accountNumber < b.accountNumber; });
    auto t2 = chrono::high_resolution_clock::now();
    double sort_fast_ms = chrono::duration<double, milli>(t2-t1).count();

    auto t3 = chrono::high_resolution_clock::now();
    int found = 0;
    for (int q : queries) {
        int idx = binarySearchById(arr_fast, q);
        if (idx >= 0) ++found;
    }
    auto t4 = chrono::high_resolution_clock::now();
    double search_fast_ms = chrono::duration<double, milli>(t4-t3).count();

    // MEDIUM: insertion sort + linear search
    auto arr_med = accounts; // copy
    auto t5 = chrono::high_resolution_clock::now();
    insertionSortById(arr_med);
    auto t6 = chrono::high_resolution_clock::now();
    double sort_med_ms = chrono::duration<double, milli>(t6-t5).count();

    auto t7 = chrono::high_resolution_clock::now();
    int found2 = 0;
    for (int q : queries) {
        int idx = linearSearchById(arr_med, q);
        if (idx >= 0) ++found2;
    }
    auto t8 = chrono::high_resolution_clock::now();
    double search_med_ms = chrono::duration<double, milli>(t8-t7).count();

    cout << fixed << setprecision(3);
    cout << "FAST (std::sort) sort time: " << sort_fast_ms << " ms\n";
    cout << "FAST (binary search) total for " << M << " queries: " << search_fast_ms << " ms\n";
    cout << "MEDIUM (insertion sort) sort time: " << sort_med_ms << " ms\n";
    cout << "MEDIUM (linear search) total for " << M << " queries: " << search_med_ms << " ms\n\n";

    cout << "Search found counts: fast=" << found << ", med=" << found2 << " (should match)\n\n";

    // Theoretical complexities
    cout << "Theoretical complexities summary:\n";
    cout << "  Sorting: std::sort => O(N log N) average, insertion sort => O(N^2) worst/average.\n";
    cout << "  Searching: binary search => O(log N), linear search => O(N).\n";
    cout << "How measured:\n";
    cout << "  - We measured actual wall-clock times for sorting and for performing M searches.\n";
    cout << "  - Sort times scale roughly as expected: insertion sort becomes much slower than O(N log N) sorter as N grows.\n";
    cout << "  - For searching, binary search time per query scales like log N; linear search scales like N, so total for M queries: M*logN vs M*N.\n\n";

    cout << "How to interpret results (example):\n";
    cout << "  - If sort_fast_ms << sort_med_ms, it confirms O(N log N) vs O(N^2).\n";
    cout << "  - If search_fast_ms << search_med_ms, it confirms binary vs linear cost for M queries.\n";

    cout << "Notes on space complexity:\n";
    cout << "  - We approximated memory used by account array as capacity * sizeof(Account) + sum(string capacities).\n";
    cout << "  - std::sort uses O(log N) extra stack space; insertion sort is in-place O(1) extra.\n\n";
    cout << "Recommendation: For large N prefer std::sort + binary search.\n";

    return 0;
}
