#include <bits/stdc++.h>
using namespace std;

void solve() {
    int n, m, q;
    cin >> n >> m >> q;

    map<int, vector<int>> adj;
    for (int i = 0; i < m; i++) {
        int u, v;
        cin >> u >> v;
        adj[u].push_back(v);
    }

    vector<pair<int,int>> queries(q);
    for (int i = 0; i < q; i++) {
        int x, u;
        cin >> x >> u;
        queries[i] = {x, u};
    }

    set<int> red;

    for(auto i : queries){
        auto [a, b] = i;
        if(a == 1){
            red.insert(b);
            continue;
        }

        queue<pair<int, int>> q;
        q.push({b, 0});
        while(q.size()){
            auto [node, person] = q.front();
            if(adj[node].empty() && ){
                cout << "YES\n";
                break;
            }

        }
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(0);

    int t;
    cin >> t;
    while (t--) solve();
}