#include <bits/stdc++.h>
using namespace std;

struct Node {
    int key;
    int left;
    int right;

    Node(int k = 0) {
        key = k;
        left = -1;
        right = -1;
    }
};

class StaticTree {
private:
    vector<Node> normal;   // BST normal
    vector<Node> tree;     // BST guardado en orden vEB
    vector<int> h;

    int normalRoot;
    int root;

    int buildBST(vector<int>& a, int l, int r) {
        if (l > r) return -1;

        int mid = (l + r) / 2;
        int id = normal.size();

        normal.push_back(Node(a[mid]));

        normal[id].left = buildBST(a, l, mid - 1);
        normal[id].right = buildBST(a, mid + 1, r);

        return id;
    }

    int calcHeight(int u) {
        if (u == -1) return 0;

        int hl = calcHeight(normal[u].left);
        int hr = calcHeight(normal[u].right);

        h[u] = 1 + max(hl, hr);
        return h[u];
    }

    void getRootsAtDepth(int u, int depth, vector<int>& roots) {
        if (u == -1) return;

        if (depth == 0) {
            roots.push_back(u);
            return;
        }

        getRootsAtDepth(normal[u].left, depth - 1, roots);
        getRootsAtDepth(normal[u].right, depth - 1, roots);
    }

    void makeVEBOrder(int u, int heightLimit, vector<int>& order) {
        if (u == -1 || heightLimit <= 0) return;

        heightLimit = min(heightLimit, h[u]);

        if (heightLimit == 1) {
            order.push_back(u);
            return;
        }

        int topHeight = (heightLimit + 1) / 2;
        int bottomHeight = heightLimit - topHeight;

        // Primero se guarda la parte superior
        makeVEBOrder(u, topHeight, order);

        // Luego se buscan las raíces de las partes inferiores
        vector<int> bottomRoots;
        getRootsAtDepth(u, topHeight, bottomRoots);

        // Finalmente se guardan esas partes inferiores
        for (int r : bottomRoots) {
            makeVEBOrder(r, bottomHeight, order);
        }
    }

    void buildVEBLayout() {
        vector<int> order;
        makeVEBOrder(normalRoot, h[normalRoot], order);

        vector<int> pos(normal.size(), -1);

        for (int i = 0; i < (int)order.size(); i++) {
            pos[order[i]] = i;
        }

        tree.resize(order.size());

        for (int i = 0; i < (int)order.size(); i++) {
            int old = order[i];

            tree[i] = Node(normal[old].key);

            if (normal[old].left != -1)
                tree[i].left = pos[normal[old].left];

            if (normal[old].right != -1)
                tree[i].right = pos[normal[old].right];
        }

        root = pos[normalRoot];
    }

public:
    StaticTree(vector<int> a) {
        normalRoot = -1;
        root = -1;

        sort(a.begin(), a.end());
        a.erase(unique(a.begin(), a.end()), a.end());

        if (a.empty()) return;

        normalRoot = buildBST(a, 0, (int)a.size() - 1);

        h.assign(normal.size(), 0);
        calcHeight(normalRoot);

        buildVEBLayout();

        vector<Node>().swap(normal);
        vector<int>().swap(h);
    }

    bool find(int x) {
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

    bool predecessor(int x, int& ans) {
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

    bool successor(int x, int& ans) {
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

    void printMemoryLayout() {
        for (int i = 0; i < (int)tree.size(); i++) {
            cout << tree[i].key << " ";
        }
        cout << "\n";
    }

    int countBlocksInSearch(int x, int B) {
        int u = root;
        set<int> blocks;

        while (u != -1) {
            blocks.insert(u / B);

            if (x == tree[u].key) break;

            if (x < tree[u].key)
                u = tree[u].left;
            else
                u = tree[u].right;
        }

        return blocks.size();
    }
};

int main() {
    vector<int> keys = {
        1, 2, 3, 4, 5, 6, 7, 8,
        9, 10, 11, 12, 13, 14, 15
    };

    StaticTree st(keys);

    cout << "Orden en memoria vEB:\n";
    st.printMemoryLayout();

    int x = 11;

    if (st.find(x))
        cout << x << " esta en el arbol\n";
    else
        cout << x << " no esta en el arbol\n";

    int ans;

    if (st.predecessor(x, ans))
        cout << "Predecesor: " << ans << "\n";
    else
        cout << "No tiene predecesor\n";

    if (st.successor(x, ans))
        cout << "Sucesor: " << ans << "\n";
    else
        cout << "No tiene sucesor\n";

    cout << "Bloques tocados con B = 4: ";
    cout << st.countBlocksInSearch(x, 4) << "\n";

    return 0;
}