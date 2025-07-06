#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

#define dist 25

#define fuel 10

void printarr(int arr[])
{
    for (int i = 0; i < dist; i++)
    {
        cout << arr[i] << " ";
    }
    cout << endl;
}

int func(vector<int> loc, vector<int> pr, int n)
{
    int arr[dist] = {0};
    if(loc[0] != 0) return -1;
    for (int i = 0; i < fuel; i++)
    {
        arr[i] = pr[0];
    }

    for (int i = 1; i < n; i++)
    {
        int cur = loc[i]; // current location

        if (arr[cur] == 0)
        {
            return -1;
        }

        for(int j = 0; j < 10 && cur + j < dist; j++){
            if(arr[cur+j] > pr[i] || arr[cur+j] == 0){
                arr[cur+j] = pr[i];
            }
        }

        // if (arr[cur] < pr[i])
        // {
        //     int tmp = cur;
        //     int rem = 10;
        //     while (arr[tmp] > 0)
        //     {
        //         tmp++;
        //         rem--;
        //     }
        //     for (int j = 0; j < rem; j++)
        //     {
        //         arr[tmp + j] = pr[i];
        //     }
        // }
        // else
        // {
        //     for (int j = 0; j < fuel; j++)
        //     {
        //         arr[cur + j] = pr[i];
        //     }
        // }
        printarr(arr);
    }

    int final = 0;
    for(int i = 0; i < dist; i++) final += arr[i];
    return arr[dist-1] > 0 ? final : -1;
}

int main()
{
    int n = 0;
    cout << "Destination: " << dist << "\nnumber of petrol pumps: \n";
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

    int k = func(loc, p, n);
    cout << endl << "ANS: " << k << endl;

    return 1;
}