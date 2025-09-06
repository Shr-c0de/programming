#include <iostream>
#include <vector>
#include <algorithm>
#include <math.h>
#include <limits.h>

using namespace std;

class dsu
{
public:
    int n, m;
    int count;
    int ans;
    vector<vector<int>> g;

    dsu(int i, int j)
    {
        n = i, m = j;
        count = 0;
        ans = 0;
        g = vector<vector<int>>(n, vector<int>(m, INT_MAX));
    }

    void print()
    {
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < m; j++)
            {
                cout << (g[i][j] == INT_MAX ? 0 : g[i][j]) << "\t";
            }
            cout << endl;
        }
    }

    void rundfs(int x, int y, int val)
    {
        if (x < 0 || y < 0 || x >= n || y >= m || g[x][y] == INT_MAX)
        {
            return;
        }

        if (g[x][y] > val)
        {
            ans--;
            g[x][y] = val;
            replacedfs(x + 1, y, val);
            replacedfs(x - 1, y, val);
            replacedfs(x, y + 1, val);
            replacedfs(x, y - 1, val);
        }
        else if (g[x][y] < val)
        {
            ans--;
            replacedfs(x + 1, y, g[x][y]);
            replacedfs(x - 1, y, g[x][y]);
            replacedfs(x, y + 1, g[x][y]);
            replacedfs(x, y - 1, g[x][y]);
        }
    }
    void replacedfs(int x, int y, int val)
    {
        if (x < 0 || y < 0 || x >= n || y >= m || g[x][y] == INT_MAX)
        {
            return;
        }

        if (g[x][y] > val)
        {
            g[x][y] = val;
            replacedfs(x + 1, y, val);
            replacedfs(x - 1, y, val);
            replacedfs(x, y + 1, val);
            replacedfs(x, y - 1, val);
        }
    }

    void dsu_add(int x, int y)
    {

        int d = (x + 1 < n) ? g[x + 1][y] : INT_MAX;
        int u = (x - 1 >= 0) ? g[x - 1][y] : INT_MAX;
        int r = (y + 1 < m) ? g[x][y + 1] : INT_MAX;
        int l = (y - 1 >= 0) ? g[x][y - 1] : INT_MAX;

        int minval = min(d, min(u, min(r, l)));
        cout << " minval = " << minval << " ";
        
        if (minval == INT_MAX)
        {
            g[x][y] = ++count;
            ans++;
        }
        else
        {
            g[x][y] = minval;

            rundfs(x + 1, y, minval);
            rundfs(x - 1, y, minval);
            rundfs(x, y + 1, minval);
            rundfs(x, y - 1, minval);
        }

        cout << "ans = " << ans << endl;

        print();
    }
};

class Solution
{
public:
    vector<int> numOfIslands(int n, int m, vector<vector<int>> &op)
    {
        // code here
        dsu d(n, m);
        int x, y;
        int count = 0;
        vector<int> ans;

        for (auto i : op)
        {
            cout << count++ << endl;
            x = i[0], y = i[1];
            d.dsu_add(x, y);
            ans.push_back(d.ans);
        }

        return ans;
    }
};

int main()
{
    if (1)
    {
        int n = 2, m = 4;
        vector<vector<int>> inp = {{1, 3}, {0, 3}, {0, 1}, {1, 1}, {1, 0}, {1, 2}, {0, 3}, {1, 2}};

        Solution obj;
        vector<int> ans = obj.numOfIslands(n, m, inp);
        for (auto i : ans)
            cout << i << " ";
        cout << endl;
    }
    else
    {
        int n, m;
        cin >> n >> m;
        int k;
        cin >> k;
        vector<vector<int>> inp;
        int a, b;
        while (k--)
        {
            cin >> a >> b;
            inp.push_back({a, b});
        }
    }
}
