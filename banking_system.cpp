#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <queue>
#include <stack>

using namespace std;

enum class AccountType {
    Saving = 1,
    Current,
    Business
};

string accountTypeToString(AccountType type) {
    switch (type) {
        case AccountType::Saving:
            return "Saving";
        case AccountType::Current:
            return "Current";
        case AccountType::Business:
            return "Business";
    }
    return "Unknown";
}

struct Account {
    int accountNumber;
    string holderName;
    double balance;
    int pin;
    AccountType type;
};

struct Loan {
    string borrowerName;
    string coBorrowerName;
    int tenureMonths;
    double annualInterestRate;
    double borrowingAmount;
    double emi;
    double remainingAmount;
    double amountPaid;
};

double calculateEMI(double principal, double annualRate, int tenureMonths) {
    double monthlyRate = annualRate / (12.0 * 100.0);
    if (tenureMonths <= 0) return 0.0;
    if (monthlyRate == 0.0) return principal / tenureMonths;

    double factor = pow(1 + monthlyRate, tenureMonths);
    return principal * monthlyRate * factor / (factor - 1);
}

class BankingSystem {
   private:
    vector<Account> accounts;
    vector<Loan> loans;
    int nextAccountNumber = 1001;
    // Undo stack for deposit/withdraw
    struct UndoEntry { int accountNumber; double amount; string op; };
    stack<UndoEntry> undoStack;

    // Simple FIFO service queue
    queue<pair<int,string>> serviceQueue;

    const string JSON_FILE = "accounts.json";

    int findAccountIndexByNumber(int accountNumber) const {
        for (int i = 0; i < static_cast<int>(accounts.size()); i++) {
            if (accounts[i].accountNumber == accountNumber) {
                return i;
            }
        }
        return -1;
    }

    // --- JSON persistence (simple, robust for files written by this program) ---
    static string extractStringValue(const string &obj, const string &key) {
        string pat = '"' + key + '"';
        size_t p = obj.find(pat);
        if (p == string::npos) return "";
        p = obj.find(':', p);
        if (p == string::npos) return "";
        // skip spaces
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

    void loadAccountsFromJson() {
        accounts.clear();
        ifstream f(JSON_FILE);
        if (!f) return; // no file yet
        string s; string content;
        while (getline(f,s)) content += s;
        f.close();
        // find objects { ... }
        size_t pos = 0;
        while (true) {
            size_t l = content.find('{', pos);
            if (l == string::npos) break;
            size_t r = content.find('}', l);
            if (r == string::npos) break;
            string obj = content.substr(l, r-l+1);
            Account a{};
            a.accountNumber = static_cast<int>(extractNumberValue(obj, "accountNumber"));
            a.holderName = extractStringValue(obj, "holderName");
            a.balance = extractNumberValue(obj, "balance");
            a.pin = static_cast<int>(extractNumberValue(obj, "pin"));
            string t = extractStringValue(obj, "type");
            if (t == "Saving") a.type = AccountType::Saving;
            else if (t == "Current") a.type = AccountType::Current;
            else a.type = AccountType::Business;
            accounts.push_back(a);
            if (a.accountNumber >= nextAccountNumber) nextAccountNumber = a.accountNumber + 1;
            pos = r+1;
        }
    }

    void saveAccountsToJson() const {
        ofstream f(JSON_FILE, ios::trunc);
        if (!f) return;
        f << "[\n";
        for (size_t i = 0; i < accounts.size(); ++i) {
            const auto &a = accounts[i];
            f << "  {";
            f << "\"accountNumber\":" << a.accountNumber << ",";
            f << "\"holderName\":\"" << a.holderName << "\",";
            f << "\"balance\":" << fixed << setprecision(2) << a.balance << ",";
            f << "\"pin\":" << a.pin << ",";
            f << "\"type\":\"" << accountTypeToString(a.type) << "\"";
            f << "}";
            if (i+1 < accounts.size()) f << ",";
            f << "\n";
        }
        f << "]\n";
        f.close();
    }

    int findLoanIndexByBorrower(const string& borrowerName) const {
        for (int i = 0; i < static_cast<int>(loans.size()); i++) {
            if (loans[i].borrowerName == borrowerName) {
                return i;
            }
        }
        return -1;
    }

   public:
    void createAccount() {
        Account account{};
        account.accountNumber = nextAccountNumber++;

        cout << "Holder Name: ";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        getline(cin, account.holderName);

        cout << "Initial Balance: ";
        cin >> account.balance;

        cout << "Set 4-digit PIN: ";
        cin >> account.pin;

        int typeChoice;
        cout << "Account Type (1.Saving 2.Current 3.Business): ";
        cin >> typeChoice;

        if (typeChoice == 1)
            account.type = AccountType::Saving;
        else if (typeChoice == 2)
            account.type = AccountType::Current;
        else
            account.type = AccountType::Business;

        accounts.push_back(account);
    // persist
    saveAccountsToJson();

        cout << "\nAccount Created Successfully\n";
        cout << "Account Number: " << account.accountNumber << "\n";
        cout << "Account Holder: " << account.holderName << "\n";
        cout << "Account Type  : " << accountTypeToString(account.type) << "\n";
        cout << fixed << setprecision(2);
        cout << "Balance       : " << account.balance << "\n";
    }

    // FAST sorting & searching: use std::sort and binary search
    void sortAccountsFastById() {
        sort(accounts.begin(), accounts.end(), [](const Account&a,const Account&b){return a.accountNumber<b.accountNumber;});
    }

    int binarySearchByAccountNumber(int accountNumber) {
        sortAccountsFastById();
        int lo = 0, hi = (int)accounts.size()-1;
        while (lo <= hi) {
            int mid = lo + (hi-lo)/2;
            if (accounts[mid].accountNumber == accountNumber) return mid;
            if (accounts[mid].accountNumber < accountNumber) lo = mid+1;
            else hi = mid-1;
        }
        return -1;
    }

    // Stack undo operations
    void pushUndo(const string &op, int accNo, double amount) {
        undoStack.push({accNo, amount, op});
    }
    void undoLast() {
        if (undoStack.empty()) { cout<<"Nothing to undo.\n"; return; }
        auto e = undoStack.top(); undoStack.pop();
        int idx = findAccountIndexByNumber(e.accountNumber);
        if (idx == -1) { cout<<"Account for undo not found.\n"; return; }
        if (e.op == "deposit") {
            accounts[idx].balance -= e.amount;
            cout<<"Undo deposit: -"<<e.amount<<" on "<<e.accountNumber<<"\n";
        } else if (e.op == "withdraw") {
            accounts[idx].balance += e.amount;
            cout<<"Undo withdraw: +"<<e.amount<<" on "<<e.accountNumber<<"\n";
        }
        saveAccountsToJson();
    }

    // Queue operations
    void enqueueService(int accNo, const string &svc) {
        serviceQueue.push({accNo, svc});
        cout<<"Enqueued service for "<<accNo<<" ("<<svc<<")\n";
    }
    void serveNext() {
        if (serviceQueue.empty()) { cout<<"No customers waiting.\n"; return; }
        auto t = serviceQueue.front(); serviceQueue.pop();
        cout<<"Serving Account "<<t.first<<" ("<<t.second<<")\n";
    }

    void checkBalance() const {
        int accountNumber;
        int enteredPin;

        cout << "Account Number: ";
        cin >> accountNumber;
        cout << "PIN: ";
        cin >> enteredPin;

        int idx = findAccountIndexByNumber(accountNumber);
        if (idx == -1) {
            cout << "Account not found.\n";
            return;
        }

        if (accounts[idx].pin != enteredPin) {
            cout << "Invalid PIN.\n";
            return;
        }

        cout << fixed << setprecision(2);
        cout << "Current Balance: " << accounts[idx].balance << "\n";
    }

    void withdrawMoney() {
        int accountNumber;
        int enteredPin;
        double amount;

        cout << "Account Number: ";
        cin >> accountNumber;
        cout << "PIN: ";
        cin >> enteredPin;
        cout << "Amount: ";
        cin >> amount;

        int idx = findAccountIndexByNumber(accountNumber);
        if (idx == -1) {
            cout << "Account not found.\n";
            return;
        }

        if (accounts[idx].pin != enteredPin) {
            cout << "Invalid PIN.\n";
            return;
        }

        if (amount <= 0) {
            cout << "Invalid amount.\n";
            return;
        }

        if (accounts[idx].balance < amount) {
            cout << "Insufficient balance.\n";
            return;
        }

        accounts[idx].balance = accounts[idx].balance - amount;
    pushUndo("withdraw", accounts[idx].accountNumber, amount);
    saveAccountsToJson();

        cout << fixed << setprecision(2);
        cout << "Withdraw Successful\n";
        cout << "Balance - Amount = New Balance\n";
        cout << accounts[idx].balance + amount << " - " << amount << " = " << accounts[idx].balance << "\n";
        cout << "New Balance: " << accounts[idx].balance << "\n";
    }

    void transferMoney() {
        int senderAccountNumber;
        int receiverAccountNumber;
        int securityCode;
        double amount;

        cout << "Sender Account Number: ";
        cin >> senderAccountNumber;
        cout << "Receiver Account Number: ";
        cin >> receiverAccountNumber;
        cout << "Amount: ";
        cin >> amount;
        cout << "Security Code (Sender PIN): ";
        cin >> securityCode;

        int senderIdx = findAccountIndexByNumber(senderAccountNumber);
        int receiverIdx = findAccountIndexByNumber(receiverAccountNumber);

        if (senderIdx == -1 || receiverIdx == -1) {
            cout << "Sender or Receiver account not found.\n";
            return;
        }

        if (accounts[senderIdx].pin != securityCode) {
            cout << "Invalid Security Code.\n";
            return;
        }

        if (amount <= 0) {
            cout << "Invalid amount.\n";
            return;
        }

        if (accounts[senderIdx].balance < amount) {
            cout << "Insufficient balance in sender account.\n";
            return;
        }

        accounts[senderIdx].balance -= amount;
        accounts[receiverIdx].balance += amount;
    pushUndo("withdraw", accounts[senderIdx].accountNumber, amount);
    // recording receiver deposit for potential undo not implemented for simplicity
    saveAccountsToJson();

        cout << fixed << setprecision(2);
        cout << "Transfer Successful\n";
        cout << "Sender: " << accounts[senderIdx].holderName << "\n";
        cout << "Receiver: " << accounts[receiverIdx].holderName << "\n";
        cout << "Amount: " << amount << "\n";
    }

    void createLoan() {
        int accountNumber;

        cout << "Borrower Account Number: ";
        cin >> accountNumber;

        int accountIdx = findAccountIndexByNumber(accountNumber);
        if (accountIdx == -1) {
            cout << "Account not found.\n";
            return;
        }

        Loan loan{};
        loan.borrowerName = accounts[accountIdx].holderName;

        cout << "Co-Borrower Name (if any, else enter NA): ";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        getline(cin, loan.coBorrowerName);

        cout << "Borrowing Amount: ";
        cin >> loan.borrowingAmount;
        cout << "Tenure (months): ";
        cin >> loan.tenureMonths;
        cout << "Interest (annual %): ";
        cin >> loan.annualInterestRate;

        if (loan.borrowingAmount <= 0 || loan.tenureMonths <= 0 || loan.annualInterestRate < 0) {
            cout << "Invalid loan values.\n";
            return;
        }

        double requiredBalance = loan.borrowingAmount / 10.0;
        if (accounts[accountIdx].balance < requiredBalance) {
            cout << fixed << setprecision(2);
            cout << "Loan Rejected\n";
            cout << "Required Balance (Borrowing Amount / 10): " << requiredBalance << "\n";
            cout << "Current Balance: " << accounts[accountIdx].balance << "\n";
            return;
        }

        loan.emi = calculateEMI(loan.borrowingAmount, loan.annualInterestRate, loan.tenureMonths);
        loan.remainingAmount = loan.emi * loan.tenureMonths;
        loan.amountPaid = 0.0;

        int existingLoanIdx = findLoanIndexByBorrower(loan.borrowerName);
        if (existingLoanIdx != -1) {
            loans[existingLoanIdx] = loan;
        } else {
            loans.push_back(loan);
        }

        cout << fixed << setprecision(2);
        cout << "Loan Approved\n";
        cout << "Borrower Name: " << loan.borrowerName << "\n";
        cout << "Co-Borrower : " << loan.coBorrowerName << "\n";
        cout << "Tenure      : " << loan.tenureMonths << " months\n";
        cout << "Interest    : " << loan.annualInterestRate << "%\n";
        cout << "Borrowing Amount : " << loan.borrowingAmount << "\n";
        cout << "EMI         : " << loan.emi << "\n";
        cout << "Remaining Amount: " << loan.remainingAmount << "\n";
        cout << "Amount Paid : " << loan.amountPaid << "\n";
    }

    void payEMI() {
        int accountNumber;
        double amount;

        cout << "Borrower Account Number: ";
        cin >> accountNumber;
        cout << "EMI Amount to Pay: ";
        cin >> amount;

        int accountIdx = findAccountIndexByNumber(accountNumber);
        if (accountIdx == -1) {
            cout << "Account not found.\n";
            return;
        }

        int loanIdx = findLoanIndexByBorrower(accounts[accountIdx].holderName);
        if (loanIdx == -1) {
            cout << "No loan found for this borrower.\n";
            return;
        }

        if (amount <= 0) {
            cout << "Invalid amount.\n";
            return;
        }

        if (accounts[accountIdx].balance < amount) {
            cout << "Insufficient balance in account.\n";
            return;
        }

        if (amount > loans[loanIdx].remainingAmount) {
            amount = loans[loanIdx].remainingAmount;
        }

        accounts[accountIdx].balance -= amount;
        loans[loanIdx].amountPaid += amount;
        loans[loanIdx].remainingAmount -= amount;

        cout << fixed << setprecision(2);
        cout << "EMI Payment Successful\n";
        cout << "Amount Paid     : " << loans[loanIdx].amountPaid << "\n";
        cout << "Remaining Amount: " << loans[loanIdx].remainingAmount << "\n";
    }

    void showLoan() const {
        int accountNumber;
        cout << "Borrower Account Number: ";
        cin >> accountNumber;

        int accountIdx = findAccountIndexByNumber(accountNumber);
        if (accountIdx == -1) {
            cout << "Account not found.\n";
            return;
        }

        int loanIdx = findLoanIndexByBorrower(accounts[accountIdx].holderName);
        if (loanIdx == -1) {
            cout << "No loan found for this borrower.\n";
            return;
        }

        const Loan& loan = loans[loanIdx];
        cout << fixed << setprecision(2);
        cout << "\nLoan Details\n";
        cout << "Borrower Name   : " << loan.borrowerName << "\n";
        cout << "Co-Borrower     : " << loan.coBorrowerName << "\n";
        cout << "Tenure          : " << loan.tenureMonths << " months\n";
        cout << "Interest        : " << loan.annualInterestRate << "%\n";
        cout << "Borrowing Amount: " << loan.borrowingAmount << "\n";
        cout << "EMI             : " << loan.emi << "\n";
        cout << "Remaining Amount: " << loan.remainingAmount << "\n";
        cout << "Amount Paid     : " << loan.amountPaid << "\n";
    }

    void listAccounts() const {
        if (accounts.empty()) {
            cout << "No accounts available.\n";
            return;
        }

        cout << fixed << setprecision(2);
        cout << "\nAccounts\n";
        for (const auto& account : accounts) {
            cout << "Account Number: " << account.accountNumber
                 << " | Holder: " << account.holderName
                 << " | Type: " << accountTypeToString(account.type)
                 << " | Balance: " << account.balance << "\n";
        }
    }
};

void showMenu() {
    cout << "\n===== BANKING SYSTEM =====\n";
    cout << "1. Create Account\n";
    cout << "2. Check Balance\n";
    cout << "3. Withdraw Money\n";
    cout << "4. Transfer Money\n";
    cout << "5. Create Loan\n";
    cout << "6. Pay EMI\n";
    cout << "7. Show Loan Details\n";
    cout << "8. List Accounts\n";
    cout << "0. Exit\n";
    cout << "Enter choice: ";
}

int main() {
    BankingSystem bank;
    int choice;

    do {
        showMenu();
        cin >> choice;

        switch (choice) {
            case 1:
                bank.createAccount();
                break;
            case 2:
                bank.checkBalance();
                break;
            case 3:
                bank.withdrawMoney();
                break;
            case 4:
                bank.transferMoney();
                break;
            case 5:
                bank.createLoan();
                break;
            case 6:
                bank.payEMI();
                break;
            case 7:
                bank.showLoan();
                break;
            case 8:
                bank.listAccounts();
                break;
            case 0:
                cout << "Exiting...\n";
                break;
            default:
                cout << "Invalid choice.\n";
        }
    } while (choice != 0);

    return 0;
}
