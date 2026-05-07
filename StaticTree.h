#pragma once

#include <vector>
#include <algorithm>
#include <set>
#include <ostream>

struct StaticNode {
    int key;
    int left;
    int right;

    StaticNode(int k = 0) {
        key = k;
        left = -1;
        right = -1;
    }
};

class StaticTree {
private:
    std::vector<StaticNode> normal;
    std::vector<StaticNode> tree;
    std::vector<int> h;

    int normalRoot;
    int root;

    int buildBST(const std::vector<int>& a, int l, int r) {
        if (l > r) return -1;

        int mid = l + (r - l) / 2;
        int id = (int)normal.size();

        normal.push_back(StaticNode(a[mid]));

        normal[id].left = buildBST(a, l, mid - 1);
        normal[id].right = buildBST(a, mid + 1, r);

        return id;
    }

    int calcHeight(int u) {
        if (u == -1) return 0;

        int leftHeight = calcHeight(normal[u].left);
        int rightHeight = calcHeight(normal[u].right);

        h[u] = 1 + std::max(leftHeight, rightHeight);
        return h[u];
    }

    void getRootsAtDepth(int u, int depth, std::vector<int>& roots) const {
        if (u == -1) return;

        if (depth == 0) {
            roots.push_back(u);
            return;
        }

        getRootsAtDepth(normal[u].left, depth - 1, roots);
        getRootsAtDepth(normal[u].right, depth - 1, roots);
    }

    void makeVEBOrder(int u, int heightLimit, std::vector<int>& order) const {
        if (u == -1 || heightLimit <= 0) return;

        heightLimit = std::min(heightLimit, h[u]);

        if (heightLimit == 1) {
            order.push_back(u);
            return;
        }

        int topHeight = (heightLimit + 1) / 2;
        int bottomHeight = heightLimit - topHeight;

        makeVEBOrder(u, topHeight, order);

        std::vector<int> bottomRoots;
        getRootsAtDepth(u, topHeight, bottomRoots);

        for (int r : bottomRoots) {
            makeVEBOrder(r, bottomHeight, order);
        }
    }

    void buildVEBLayout() {
        if (normalRoot == -1) return;

        std::vector<int> order;
        makeVEBOrder(normalRoot, h[normalRoot], order);

        std::vector<int> pos(normal.size(), -1);

        for (int i = 0; i < (int)order.size(); i++) {
            pos[order[i]] = i;
        }

        tree.resize(order.size());

        for (int i = 0; i < (int)order.size(); i++) {
            int old = order[i];

            tree[i] = StaticNode(normal[old].key);

            if (normal[old].left != -1) {
                tree[i].left = pos[normal[old].left];
            }

            if (normal[old].right != -1) {
                tree[i].right = pos[normal[old].right];
            }
        }

        root = pos[normalRoot];
    }

public:
    StaticTree(std::vector<int> a) {
        normalRoot = -1;
        root = -1;

        std::sort(a.begin(), a.end());
        a.erase(std::unique(a.begin(), a.end()), a.end());

        if (a.empty()) return;

        normalRoot = buildBST(a, 0, (int)a.size() - 1);

        h.assign(normal.size(), 0);
        calcHeight(normalRoot);

        buildVEBLayout();

        std::vector<StaticNode>().swap(normal);
        std::vector<int>().swap(h);
    }

    bool find(int x) const {
        int u = root;

        while (u != -1) {
            if (x == tree[u].key) return true;

            if (x < tree[u].key)
                u = tree[u].left;
            else
                u = tree[u].right;
        }

        return false;
    }

    bool predecessor(int x, int& ans) const {
        int u = root;
        bool found = false;

        while (u != -1) {
            if (tree[u].key < x) {
                ans = tree[u].key;
                found = true;
                u = tree[u].right;
            } else {
                u = tree[u].left;
            }
        }

        return found;
    }

    bool successor(int x, int& ans) const {
        int u = root;
        bool found = false;

        while (u != -1) {
            if (tree[u].key > x) {
                ans = tree[u].key;
                found = true;
                u = tree[u].left;
            } else {
                u = tree[u].right;
            }
        }

        return found;
    }

    void printMemoryLayout(std::ostream& out) const {
        for (int i = 0; i < (int)tree.size(); i++) {
            out << tree[i].key << " ";
        }
        out << "\n";
    }

    int countBlocksInSearch(int x, int B) const {
        int u = root;
        std::set<int> blocks;

        while (u != -1) {
            blocks.insert(u / B);

            if (x == tree[u].key) break;

            if (x < tree[u].key)
                u = tree[u].left;
            else
                u = tree[u].right;
        }

        return (int)blocks.size();
    }

    int size() const {
        return (int)tree.size();
    }
};