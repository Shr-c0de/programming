#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
using namespace std;

class Node
{
public:
    int data;
    vector<int> next;

    Node(int m)
    {
        data = m;
    }
    Node()
    {
        data = 0;
    }
};

int count_rem(vector<Node> &g, int node, int val)
{
    Node cur = g[node];
    if(cur.next.empty()){
        return cur.data;
    }

    int total = cur.data;

    for(auto i:cur.next){
        int k = count_rem(g, i, val);
        if(k == -1) return -1;

        if(total + k <= val){
            if(k == val) continue;
            total += k;
        }
        else return -1;
    }
    return total;
}

int main()
{
    int n;
    cin >> n;
    vector<Node> g(n);
    int maxval = 0;
    int total = 0;
    for (int i = 0; i < n; i++)
    {
        cin >> g[i].data;
        maxval = max(maxval, g[i].data);
        total += g[i].data;
    }
    for (int i = 0; i < n; i++)
    {
        int u, v;
        cin >> u >> v;
        g[u].next.push_back(v);
        g[v].next.push_back(u);
    }

    for (int i = maxval; i <= total; i++)
    {
        int k = count_rem(g, 0, i);
        if(k > 0) cout << k;
    }
}