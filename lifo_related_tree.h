#include <iostream>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <queue>
using namespace std;

// 前向声明
struct BinaryTree;

// 多叉树节点定义
struct MultiNode {
    string order;
    double storage;
    double demand;
    MultiNode* parent;
    vector<MultiNode*> children;

    MultiNode(string order, double storage = 0.0, double demand = 0.0, MultiNode* parent = nullptr)
        : order(order), storage(storage), demand(demand), parent(parent) {}
};

// 多叉树定义
struct MultiTree {
    MultiNode* root = nullptr;
    unordered_map<string, MultiNode*> nodeMap; // order到节点的映射

    // 添加节点到多叉树
    void addNode(string order, double storage, double demand, string parentOrder = "") {
        MultiNode* node = new MultiNode(order, storage, demand);
        nodeMap[order] = node;
        
        if (parentOrder.empty()) {
            root = node;
        } else {
            MultiNode* parent = nodeMap[parentOrder];
            node->parent = parent;
            parent->children.push_back(node);
        }
    }

    // 将多叉树转换为二叉树
    BinaryTree toBinaryTree();
};

// 二叉树节点定义（已在原始代码中定义，此处保留）
struct TreeNode {
public:
    std::string order;
    double storage; // 这个位置可以容纳的容量
    double demand;
    TreeNode* parent;
    TreeNode* left;
    TreeNode* right;
    std::string leftparent;  // 存储原多叉树父节点标识

    TreeNode(std::string order, TreeNode* parent = nullptr)
        : order(order), parent(parent), left(nullptr), right(nullptr), storage(0), demand(0) {
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
        leftparent = "";
    }

    void setstorage(double n){
        storage = n;
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
};

// 二叉树定义（原始代码基础上增强）
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
    BinaryTree(TreeNode* r) : root(r) {}

    ~BinaryTree() {
        // 内存释放逻辑（实际使用时需要实现）
    }

    void initRoot(std::string order, double c) {
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
            if(node == root) continue;
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

    bool insert(TreeNode* parent, bool insertLeft, std::string order, double s) {
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

    // 二叉树转多叉树
    MultiTree toMultiTree() {
        MultiTree mt;
        if (!root) return mt;

        unordered_map<TreeNode*, MultiNode*> nodeMap;
        queue<TreeNode*> q;
        q.push(root);

        // 处理根节点
        mt.root = new MultiNode(root->order, root->storage, root->demand);
        mt.nodeMap[root->order] = mt.root;
        nodeMap[root] = mt.root;

        while (!q.empty()) {
            TreeNode* curr = q.front();
            q.pop();
            
            // 获取对应的多叉树节点
            MultiNode* mnode = nodeMap[curr];
            
            // 处理左孩子（第一个子节点）
            TreeNode* child = curr->left;
            while (child) {
                // 创建新的多叉树节点
                MultiNode* newChild = new MultiNode(
                    child->order, 
                    child->storage, 
                    child->demand,
                    mnode
                );
                
                // 建立关系
                mt.nodeMap[child->order] = newChild;
                nodeMap[child] = newChild;
                mnode->children.push_back(newChild);
                
                // 入队处理
                q.push(child);
                
                // 移动到右兄弟节点
                child = child->right;
            }
        }
        
        return mt;
    }
};

// 多叉树转二叉树实现
BinaryTree MultiTree::toBinaryTree() {
    BinaryTree bt;
    if (!root) return bt;

    unordered_map<MultiNode*, TreeNode*> nodeMap;
    queue<MultiNode*> q;
    q.push(root);
    
    // 处理根节点
    TreeNode* binRoot = new TreeNode(root->order);
    binRoot->setstorage(root->storage);
    binRoot->setdemand(root->demand);
    binRoot->setleftparent("");
    bt.initRoot(root->order, root->storage);
    nodeMap[root] = bt.getRoot();

    while (!q.empty()) {
        MultiNode* curr = q.front();
        q.pop();
        
        // 获取对应的二叉树节点
        TreeNode* bnode = nodeMap[curr];
        
        if (!curr->children.empty()) {
            // 处理第一个孩子
            MultiNode* firstChild = curr->children[0];
            TreeNode* binChild = new TreeNode(firstChild->order, bnode);
            binChild->setstorage(firstChild->storage);
            binChild->setdemand(firstChild->demand);
            binChild->setleftparent(curr->order);
            
            bnode->left = binChild;
            nodeMap[firstChild] = binChild;
            q.push(firstChild);
            
            // 处理右兄弟（剩余孩子节点）
            TreeNode* prev = binChild;
            for (int i = 1; i < curr->children.size(); ++i) {
                MultiNode* sibling = curr->children[i];
                TreeNode* binSibling = new TreeNode(sibling->order, prev);
                binSibling->setstorage(sibling->storage);
                binSibling->setdemand(sibling->demand);
                binSibling->setleftparent(curr->order);
                
                prev->right = binSibling;
                nodeMap[sibling] = binSibling;
                q.push(sibling);
                prev = binSibling;
            }
        }
    }
    
    // 构建availableNodes
    bt.getAvailableNodes().clear();
    queue<TreeNode*> nodes;
    nodes.push(bt.getRoot());
    while (!nodes.empty()) {
        TreeNode* node = nodes.front();
        nodes.pop();
        if (node->left || node->right) {
            if (!node->left || !node->right) {
                bt.getAvailableNodes().insert(node);
            }
            if (node->left) nodes.push(node->left);
            if (node->right) nodes.push(node->right);
        } else {
            bt.getAvailableNodes().insert(node);
        }
    }
    
    return bt;
}