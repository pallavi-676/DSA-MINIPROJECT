#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <fstream>
#include <queue>
#include <stack>

using namespace std;

enum class AccountType { Saving=1, Current, Business };

string accountTypeToString(AccountType t){ if(t==AccountType::Saving) return "Saving"; if(t==AccountType::Current) return "Current"; return "Business"; }

struct Account{ int accountNumber; string holderName; double balance; int pin; AccountType type; };

const string JSON_FILE = "accounts.json";


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

class MediumBank {
    vector<Account> accounts;
    int nextAccountNumber = 1001;
    stack<pair<int,double>> undoStack; // acc, amount (withdraw)
    queue<pair<int,string>> serviceQueue;

    void load() {
        accounts.clear();
        ifstream f(JSON_FILE);
        if(!f) return;
        string s, content;
        while (getline(f,s)) content+=s;
        f.close();
        size_t pos=0;
        while(true){ size_t l=content.find('{',pos); if(l==string::npos) break; size_t r=content.find('}',l); if(r==string::npos) break; string obj=content.substr(l,r-l+1); Account a; a.accountNumber=(int)extractNumberValue(obj,"accountNumber"); a.holderName=extractStringValue(obj,"holderName"); a.balance=extractNumberValue(obj,"balance"); a.pin=(int)extractNumberValue(obj,"pin"); string tt=extractStringValue(obj,"type"); if(tt=="Saving") a.type=AccountType::Saving; else if(tt=="Current") a.type=AccountType::Current; else a.type=AccountType::Business; accounts.push_back(a); if(a.accountNumber>=nextAccountNumber) nextAccountNumber=a.accountNumber+1; pos=r+1; }
    }
    void save(){ ofstream f(JSON_FILE, ios::trunc); if(!f) return; f<<"[\n"; for(size_t i=0;i<accounts.size();++i){ auto &a=accounts[i]; f<<"  {"; f<<"\"accountNumber\":"<<a.accountNumber<<","; f<<"\"holderName\":\""<<a.holderName<<"\","; f<<"\"balance\":"<<fixed<<setprecision(2)<<a.balance<<","; f<<"\"pin\":"<<a.pin<<","; f<<"\"type\":\""<<accountTypeToString(a.type)<<"\""; f<<"}"; if(i+1<accounts.size())f<<","; f<<"\n"; } f<<"]\n"; f.close(); }

public:
    MediumBank(){ load(); }
    void createAccount(){ Account a{}; a.accountNumber=nextAccountNumber++; cout<<"Holder Name: "; cin.ignore(numeric_limits<streamsize>::max(),'\n'); getline(cin,a.holderName); cout<<"Initial Balance: "; cin>>a.balance; cout<<"Set PIN: "; cin>>a.pin; int c; cout<<"Type (1 Save,2 Curr,3 Bus): "; cin>>c; a.type=(c==1?AccountType::Saving:(c==2?AccountType::Current:AccountType::Business)); accounts.push_back(a); save(); cout<<"Created "<<a.accountNumber<<"\n"; }

    // insertion sort by id (medium level)
    void insertionSortById(){ for(int i=1;i<(int)accounts.size();++i){ Account key=accounts[i]; int j=i-1; while(j>=0 && accounts[j].accountNumber>key.accountNumber){ accounts[j+1]=accounts[j]; --j;} accounts[j+1]=key; } }

    int linearSearchById(int accNo){ for(int i=0;i<(int)accounts.size();++i) if(accounts[i].accountNumber==accNo) return i; return -1; }

    void list(){ for(auto &a:accounts) cout<<a.accountNumber<<" "<<a.holderName<<" "<<a.balance<<"\n"; }
    void withdraw(){ int acc; double amt; cout<<"Account#: "; cin>>acc; cout<<"Amt: "; cin>>amt; int idx=linearSearchById(acc); if(idx==-1) { cout<<"Not found\n"; return; } if(accounts[idx].balance<amt){ cout<<"Insufficient\n"; return; } accounts[idx].balance-=amt; undoStack.push({acc,amt}); save(); cout<<"Withdrew\n"; }
    void undo(){ if(undoStack.empty()){ cout<<"Nothing to undo\n"; return; } auto p=undoStack.top(); undoStack.pop(); int idx=linearSearchById(p.first); if(idx==-1){ cout<<"Account missing\n"; return; } accounts[idx].balance+=p.second; save(); cout<<"Undo complete\n"; }
    void enqueueService(){ int acc; string svc; cout<<"Account#: "; cin>>acc; cout<<"Service: "; cin>>svc; serviceQueue.push({acc,svc}); cout<<"Enqueued\n"; }
    void serve(){ if(serviceQueue.empty()){ cout<<"No one\n"; return; } auto p=serviceQueue.front(); serviceQueue.pop(); cout<<"Serving "<<p.first<<" ("<<p.second<<")\n"; }
};

void showMenu(){ cout<<"\nMedium BANK MENU\n1.Create 2.List 3.Withdraw 4.Undo 5.Enqueue 6.Serve 0.Exit\nChoice:"; }

int main(){ MediumBank b; int ch; do{ showMenu(); cin>>ch; switch(ch){ case 1: b.createAccount(); break; case 2: b.list(); break; case 3: b.withdraw(); break; case 4: b.undo(); break; case 5: b.enqueueService(); break; case 6: b.serve(); break; case 0: break; default: cout<<"Bad\n"; } } while(ch!=0); return 0; }
