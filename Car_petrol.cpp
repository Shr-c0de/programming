#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

int func(vector<int> loc, vector<int> pr, int n)
{
}

int main()
{
    int n = 0;
    cout << "Destination: 25km\nnumber of petrol pumps: \n";
    cin >> n;
    vector<int> loc, p;
    if (n > 0)
    {
        loc = vector<int>(n, 0);
        p = vector<int>(n, 0);
        cout << "location and price: \n";
        for (int i = 0; i < n; i++)
        {
            cin >> loc[i] >> p[i];
        }
    }
    else
    {
        n = 6;
        loc = {0, 5, 7, 10, 15, 20};
        p = {2, 3, 1, 5, 6, 7};
    }

    func(loc, p, n);
}