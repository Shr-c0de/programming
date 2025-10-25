#include <bits/stdc++.h>
using namespace std;

#define int long long

void help()
{
    int a, b;
    cin >> a >> b;

    if (a % 2 == 1 && b % 2 == 1)
    {
        cout << a * b + 1 << endl;
    }
    else if (a % 2 == 0 && b % 2 == 1)
    {
        cout << -1 << endl;
    }
    else if (a % 2 == 0 && b % 2 == 0)
    {
        cout << (a * b) / 2 + 2 << endl;
    }
    else
    {
        if (b % 4 == 0)
            cout << (a * b) / 2 + 2 << endl;
        else
            cout << -1 << endl;
    }
}

signed main()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int t;
    cin >> t;
    while (t--)
        help();
}
