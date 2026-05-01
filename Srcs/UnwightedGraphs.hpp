#ifndef UNWIGHTEDGRAPHS_HPP
#define UNWIGHTEDGRAPHS_HPP
#include <bits/stdc++.h>
using namespace std;
template <class T>
class BFS {
private:
    vector<vector<int>> adj;
    vector<bool> visited;
    vector<int> parent;
    vector<int> dist;
    bool directed = false;

public:
    BFS(int n) {
        adj.resize(n);
        parent.assign(n, -1);
        dist.assign(n, -1);
        visited.assign(n, false);
    }

    void ToggleDirection() {
        directed = !directed;
    }

    void AddEdge(int x, int y) {
        adj[x].push_back(y);
        if (!directed) adj[y].push_back(x);
    }

    void traverse(int x) {
        fill(visited.begin(), visited.end(), false);
        fill(dist.begin(), dist.end(), -1);
        fill(parent.begin(), parent.end(), -1);

        queue<int> q;
        q.push(x);
        visited[x] = true;
        dist[x] = 0;

        while (!q.empty()) {
            int u = q.front();
            q.pop();

            for (auto v : adj[u]) {
                if (!visited[v]) {
                    visited[v] = true;
                    dist[v] = dist[u] + 1;
                    parent[v] = u;
                    q.push(v);
                }
            }
        }
    }
};
template <class T>
class DFS {
private:
    vector<vector<int>> adj;
    vector<bool> visited;
    vector<int> parent;
    bool directed = false;

public:
    DFS(int n) {
        adj.resize(n);
        visited.assign(n, false);
        parent.assign(n, -1);
    }

    void ToggleDirection() {
        directed = !directed;
    }

    void AddEdge(int x, int y) {
        adj[x].push_back(y);
        if (!directed) adj[y].push_back(x);
    }

    void dfs(int u) {
        visited[u] = true;

        for (auto v : adj[u]) {
            if (!visited[v]) {
                parent[v] = u;
                dfs(v);
            }
        }
    }

    void traverse(int x) {
        fill(visited.begin(), visited.end(), false);
        fill(parent.begin(), parent.end(), -1);
        dfs(x);
    }
};
#endif
