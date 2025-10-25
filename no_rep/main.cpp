#include <bits/stdc++.h>
using namespace std;

vector<int> dp = {0, 2, 3};

/*
0 - 0
1 - 2
2 - 3
3 - 5
4 - 8
*/

int count(int n){
    if(n < 0) return 0;
    if(dp.size() > n) return dp[n];

    dp.push_back(count(n-1) + count(n-2));

    return dp[n];
}

void func(int n, int k, string &up){
    if(n <= 0) return;
    // cout << "n: " << n << " k: " << k << " up: " << up << endl;


    if(k == 0 || count(n-1) > k || k-count(n-1) < 0){
        // cout << "taking 0" << endl;
        up = up + "0";
        func(n-1, k, up);
    }
    else {

        // cout << "taking 1" << endl;
        if(n == 1)
            up = up + "1";
        else{
            up = up + "10";
            func(n-2, k-count(n-1), up);
        }
    }
}

string find_k(int k, int n){
    if(k > count(n)) return "-1";
    string ans;
    func(n, k, ans);
    return ans;
}

int main(){
    int n = 4;
    // cin >> n;
    cout << "for " << n << " digit number:" << endl;
    cout << count(n) << endl;
    for(int i = 0; i < count(n); i++)
        cout << i << ". " << find_k(i, n) << endl;
    return 0;
}


/*
0. 0000
1. 0001
2. 0010
3. 0100
4. 0101
5. 1000
6. 1001
7. 1010 
*/