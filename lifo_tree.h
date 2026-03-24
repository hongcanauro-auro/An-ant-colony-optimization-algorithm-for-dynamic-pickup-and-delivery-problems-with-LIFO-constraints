#include <iostream>
#include <vector>
#include <unordered_set>
#include <string>
using namespace std;

struct TreeNode {
public:
    std::string order;
    double storage; // 这个位置可以容纳的容量
    double demand;
    double below_load;
    TreeNode* parent;
    TreeNode* left;
    TreeNode* right;
    std::string leftparent;  // 存储第一个左祖先的父节点数据
    TreeNode*hasleft;
    TreeNode(std::string order, TreeNode* parent = nullptr)
        : order(order), parent(parent), left(nullptr), right(nullptr) {
        updateAncestorInfo();
    }

    bool isLeftChild() {
        return parent && parent->left == this;
    }

    bool isRightChild() {
        return parent && parent->right == this;
    }

    void updateAncestorInfo() {
        TreeNode* current = this;
        while (current->parent) {
            if (current->isLeftChild()) {
                leftparent = current->parent->order;
                return;
            }
            current = current->parent;
        }
        leftparent = "fuck";
    }

    void setstorage(double n){
        storage = n;
    }
    
    void setbelow_load(double n){
        below_load = n;
    }

    void setleftparent(string l){
        leftparent = l;
    }

    void setdemand(double d){
        demand = d;
    }
    
    double getstorage(){
        return storage;
    }

    double getbelow_load(){
        return below_load;
    }
};

class BinaryTree {
private:
    TreeNode* root;
    std::unordered_set<TreeNode*> availableNodes;

    void preOrderHelper(TreeNode* node, std::vector<std::string>& result) {
        if (!node) return;
        result.push_back(node->order);
        preOrderHelper(node->left, result);
        preOrderHelper(node->right, result);
    }

    void postOrderHelper(TreeNode* node, std::vector<std::string>& result) {
        if (!node) return;
        postOrderHelper(node->left, result);
        postOrderHelper(node->right, result);
        result.push_back(node->order);
    }

public:
    
    BinaryTree() : root(nullptr) {}

    ~BinaryTree() {
        // 实际使用时需要补充内存释放逻辑
    }

    void initRoot(std::string order,double c) {
        if (!root) {
            root = new TreeNode(order);
            root->setstorage(c);
            root->setdemand(0);
            availableNodes.insert(root);
        }
    }

    TreeNode* getRoot() {
        return root;
    }

    std::unordered_set<TreeNode*>& getAvailableNodes() {
        return availableNodes;
    }

    std::vector<TreeNode*> getNodesWithAvailableLeft() {
        std::vector<TreeNode*> result;
        for (auto node : availableNodes) {
            if(node == root)continue;
            if (!node->left) result.push_back(node);
        }
        return result;
    }

    std::vector<TreeNode*> getNodesWithAvailableRight() {
        std::vector<TreeNode*> result;
        for (auto node : availableNodes) {
            if (!node->right) result.push_back(node);
        }
        return result;
    }

    bool insert(TreeNode* parent, bool insertLeft, std::string order,double s) {
        if (!parent || (insertLeft && parent->left) || (!insertLeft && parent->right)) {
            return false;
        }

        TreeNode* newNode = new TreeNode(order, parent);
        newNode->setdemand(s);
        if (insertLeft) {
            newNode->setleftparent(parent->order);
            newNode->setstorage(parent->getstorage() - s);
            parent->left = newNode;
        } else {
            newNode->setleftparent(parent->leftparent);
            newNode->setstorage(parent->getstorage() + parent->demand - s);
            parent->right = newNode;
        }

        availableNodes.insert(newNode);

        // 更新父节点的可用状态
        bool parentFull = parent->left && parent->right;
        if (parentFull) {
            availableNodes.erase(parent);
        }

        return true;
    }

    std::vector<std::string> preOrderTraversal() {
        std::vector<std::string> result;
        preOrderHelper(root, result);
        return result;
    }

    std::vector<std::string> postOrderTraversal() {
        std::vector<std::string> result;
        postOrderHelper(root, result);
        return result;
    }
};
