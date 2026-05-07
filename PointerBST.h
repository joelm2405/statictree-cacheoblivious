#pragma once

#include <vector>
#include <algorithm>

struct BSTNode {
    int key;
    BSTNode* left;
    BSTNode* right;

    BSTNode(int k) {
        key = k;
        left = nullptr;
        right = nullptr;
    }
};

class PointerBST {
private:
    BSTNode* root;

    BSTNode* buildBalanced(const std::vector<int>& a, int l, int r) {
        if (l > r) return nullptr;

        int mid = l + (r - l) / 2;
        BSTNode* node = new BSTNode(a[mid]);

        node->left = buildBalanced(a, l, mid - 1);
        node->right = buildBalanced(a, mid + 1, r);

        return node;
    }

    void destroy(BSTNode* node) {
        if (node == nullptr) return;

        destroy(node->left);
        destroy(node->right);

        delete node;
    }

public:
    PointerBST(std::vector<int> a) {
        std::sort(a.begin(), a.end());
        a.erase(std::unique(a.begin(), a.end()), a.end());

        root = buildBalanced(a, 0, (int)a.size() - 1);
    }

    PointerBST(const PointerBST&) = delete;
    PointerBST& operator=(const PointerBST&) = delete;

    bool find(int x) const {
        BSTNode* current = root;

        while (current != nullptr) {
            if (x == current->key) return true;

            if (x < current->key)
                current = current->left;
            else
                current = current->right;
        }

        return false;
    }

    ~PointerBST() {
        destroy(root);
    }
};