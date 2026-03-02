#include "../inc/Floorplanner.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <queue>
#include <cstdlib>
#include <string>


// Constructor: initialize with given blocks
Floorplanner::Floorplanner(const std::vector<Block>& bks) : blocks(bks) {
    root = nullptr;
}

// Copy constructor: perform deep copy of blocks and tree structure
Floorplanner::Floorplanner(const Floorplanner& o) : blocks(o.blocks) {
    root = cloneTree(o.root);
}

// Assignment operator: handle self-assignment and perform deep copy
Floorplanner& Floorplanner::operator=(const Floorplanner& o) {
    if (this == &o) return *this;
    deleteTree(root);
    blocks = o.blocks;
    root = cloneTree(o.root);
    return *this;
}

// Destructor: clean up tree memory
Floorplanner::~Floorplanner() {
    deleteTree(root);
}

// Build initial B*-Tree structure as a complete binary tree
void Floorplanner::buildInitialBTree() {
    if (blocks.empty()) {
        root = nullptr;
        return;
    }

    // Create nodes for all blocks
    std::vector<BTreeNode*> nodes(blocks.size(), nullptr);
    for (int i = 0; i < blocks.size(); ++i) {
        nodes[i] = new BTreeNode(i);
    }

    // Build complete binary tree structure
    for (int i = 0; i < blocks.size(); ++i) {
        int left_idx = 2 * i + 1;   // Left child index
        int right_idx = 2 * i + 2;  // Right child index
        
        // Assign left child if exists
        if (left_idx < blocks.size()) {
            nodes[i]->left = nodes[left_idx];
        }
        // Assign right child if exists
        if (right_idx < blocks.size()) {
            nodes[i]->right = nodes[right_idx];
        }
    }

    root = nodes[0];
}

// Place blocks using contour packing based on B*-Tree structure
void Floorplanner::place() {
    hcontour = Contour();  // Initialize horizontal contour
    vcontour = Contour();  // Initialize vertical contour
    if (!root) return;

    // Structure for BFS queue containing node and base coordinates
    struct QueueNode {
        BTreeNode* node;
        double base_x, base_y;
    };

    // Start BFS from root node
    std::queue<QueueNode> q;
    q.push({root, 0, 0});
    
    // Place root block at origin and update contours
    Block& r = blocks[root->block_idx];
    hcontour.update(0, r.width(), r.height());
    vcontour.update(0, r.height(), r.width());
    r.x = 0; r.y = 0;

    // Process all nodes in BFS order
    while (!q.empty()) {
        QueueNode curr = q.front();
        q.pop();

        BTreeNode* node = curr.node;
        Block& b = blocks[node->block_idx];
        double base_x = curr.base_x;
        double base_y = curr.base_y;

        BTreeNode* left = node->left;   // Right child (horizontal relationship)
        BTreeNode* right = node->right; // Upper child (vertical relationship)

        // Process right child (blocks above current block)
        if (right) {
            Block& up = blocks[right->block_idx];

            // Find suitable position for upper block
            double y = hcontour.getMaxHeight(base_x, up.width());
            y = std::max(y, base_y + b.height());  // Must be above current block
            double x = vcontour.getMaxHeight(y, up.height());

            up.x = x;
            up.y = y;
            
            // Update both contours with new block placement
            hcontour.update(x, up.width(),  y + up.height());
            vcontour.update(y, up.height(), x + up.width());

            q.push({right, x, y});
        }

        // Process left child (blocks to the right of current block)
        if (left) {
            Block& rt = blocks[left->block_idx];

            // Find suitable position for right block
            double x = vcontour.getMaxHeight(base_y, rt.height());
            x = std::max(x, base_x + b.width());  // Must be to the right of current block
            double y = hcontour.getMaxHeight(x, rt.width());

            rt.x = x;
            rt.y = y;

            // Update both contours with new block placement
            hcontour.update(x, rt.width(),  y + rt.height());
            vcontour.update(y, rt.height(), x + rt.width());
            
            q.push({left, x, y});
        }
    }
}

// Calculate total area of the floorplan (bounding box)
double Floorplanner::area() const {
    double xmax = 0, ymax = 0;
    for (auto& b : blocks) {
        xmax = std::max(xmax, b.x + b.width());
        ymax = std::max(ymax, b.y + b.height());
    }
    return xmax * ymax;
}

// Calculate Integral Non-Linearity (INL) metric
double Floorplanner::INL() const {
    auto cent = centroid();
    std::vector<std::pair<int, double>> dists;

    // Pair: (index in blocks, squared distance to centroid)
    for (int i = 0; i < blocks.size(); ++i) {
        const auto& b = blocks[i];
        double cx = b.x + b.width() / 2.0;
        double cy = b.y + b.height() / 2.0;
        double dx = cx - cent.first;
        double dy = cy - cent.second;
        double dist2 = dx * dx + dy * dy;
        dists.emplace_back(i, dist2);
    }

    // Sort by number at the end of the block name
    std::sort(dists.begin(), dists.end(),
        [this](const std::pair<int, double>& a, const std::pair<int, double>& b) {
            int ida = extract_number(blocks[a.first].name);
            int idb = extract_number(blocks[b.first].name);
            return ida < idb;
        });

    // Cumulative sum of squared distances
    std::vector<double> S;
    double acc = 0;
    for (auto& v : dists) {
        acc += v.second;
        S.push_back(acc);
    }

    // Linear regression: fit line to (i, S[i])
    int n = (int)S.size();
    double sx = 0, sy = 0, sx2 = 0, sxy = 0;
    for (int i = 0; i < n; ++i) {
        sx += i;
        sy += S[i];
        sx2 += i * i;
        sxy += i * S[i];
    }

    double a = (n * sxy - sx * sy) / (n * sx2 - sx * sx);
    double b = (sy - a * sx) / n;

    // Max deviation from regression line
    double max_err = 0;
    for (int i = 0; i < n; ++i) {
        double val = std::fabs(S[i] - (a * i + b));
        if (val > max_err) max_err = val;
    }

    return max_err;
}



// Calculate centroid (center point) of the floorplan
std::pair<double, double> Floorplanner::centroid() const {
    double xmin = 1e9, xmax = -1e9, ymin = 1e9, ymax = -1e9;
    
    // Find bounding box coordinates
    for (auto& b : blocks) {
        xmin = std::min(xmin, b.x);
        xmax = std::max(xmax, b.x + b.width());
        ymin = std::min(ymin, b.y);
        ymax = std::max(ymax, b.y + b.height());
    }
    return {(xmin + xmax) / 2, (ymin + ymax) / 2};
}

// Calculate penalty for overlapping blocks
double Floorplanner::overlapPenalty() const {
    double penalty = 0;
    
    // Check all pairs of blocks for overlap
    for (int i = 0; i < blocks.size(); ++i) {
        const auto& a = blocks[i];
        for (int j = i + 1; j < (int)blocks.size(); ++j) {
            const auto& b = blocks[j];
            
            // Check if blocks overlap
            if (a.x < b.x + b.width() && a.x + a.width() > b.x &&
                a.y < b.y + b.height() && a.y + a.height() > b.y) {
                // Calculate overlap area
                double x_overlap = std::min(a.x + a.width(), b.x + b.width()) - std::max(a.x, b.x);
                double y_overlap = std::min(a.y + a.height(), b.y + b.height()) - std::max(a.y, b.y);
                penalty += x_overlap * y_overlap;
            }
        }
    }
    return penalty;
}

// Perform random perturbation for simulated annealing
void Floorplanner::perturb() {
    int choice = rand() % 3;

    // Option 1: Swap two blocks in B*-Tree (swap block indices, not tree structure)
    if (choice == 0) {
        if (blocks.size() < 2) return;
        int a = rand() % blocks.size();
        int b = rand() % blocks.size();
        while (b == a) b = rand() % blocks.size();
        swapBlockIndex(root, a, b);
    }
    // Option 2: Change variant/option of a random block
    else if(choice == 1){
        int i = rand() % blocks.size();
        Block& blk = blocks[i];
        blk.selected_option = (blk.selected_option + 1) % blk.options.size();
    }
    // Option 3: Move a node to different position in B*-Tree
    else {
        std::vector<BTreeNode*> nodes;
        collectNodes(root, nodes);
        if (nodes.size() < 2) return;
    
        // Randomly select a node to move (cannot be root)
        int remove_idx = rand() % nodes.size();
        while (nodes[remove_idx] == root) remove_idx = rand() % nodes.size();
        BTreeNode* target_node = nodes[remove_idx];
    
        // Try to remove the node from B*-Tree
        if (!removeNode(root, target_node)) return;
    
        // Insert at different position (left or right child)
        nodes.clear();
        collectNodes(root, nodes);
        if (nodes.empty()) return;
        int insert_idx = rand() % nodes.size();
        BTreeNode* insert_node = nodes[insert_idx];
    
        bool insert_left = rand() % 2;
    
        if (insert_left) {
            // Insert as leftmost child
            while (insert_node->left) {
                insert_node = insert_node->left;
            }
            insert_node->left = target_node;
            target_node->left = nullptr;
            target_node->right = nullptr;
        } else {
            // Insert as rightmost child
            while (insert_node->right) {
                insert_node = insert_node->right;
            }
            insert_node->right = target_node;
            target_node->left = nullptr;
            target_node->right = nullptr;
        }
    }
}

// Check if target node is a descendant of root node
bool Floorplanner::isDescendant(BTreeNode* root, BTreeNode* target) {
    if (!root) return false;
    if (root == target) return true;
    return isDescendant(root->left, target) || isDescendant(root->right, target);
}

// Print B*-Tree structure for debugging purposes
void Floorplanner::printTree(BTreeNode* node, const std::string& prefix, bool isRight) {
    if (!node) return;

    Block& b = blocks[node->block_idx];

    // Print right child (vertical relationship)
    if (node->right) {
        printTree(node->right, prefix + (isRight ? "│   " : "    "), false);
    }

    // Print current node
    std::cout << prefix;
    std::cout << (isRight ? "└── " : "┌── ");
    std::cout << b.name << " (opt " << b.selected_option << ")" << std::endl;

    // Print left child (horizontal relationship)
    if (node->left) {
        printTree(node->left, prefix + (isRight ? "    " : "│   "), true);
    }
}

// Deep copy entire tree structure
BTreeNode* Floorplanner::cloneTree(BTreeNode* node) {
    if (!node) return nullptr;
    BTreeNode* c = new BTreeNode(node->block_idx);
    c->left  = cloneTree(node->left);
    c->right = cloneTree(node->right);
    return c;
}

// Recursively delete tree and free memory
void Floorplanner::deleteTree(BTreeNode* node) {
    if (!node) return;
    deleteTree(node->left);
    deleteTree(node->right);
    delete node;
}

// Swap block indices in all nodes of the tree
void Floorplanner::swapBlockIndex(BTreeNode* node, int a, int b) {
    if (!node) return;
    if (node->block_idx == a) node->block_idx = b;
    else if (node->block_idx == b) node->block_idx = a;
    swapBlockIndex(node->left, a, b);
    swapBlockIndex(node->right, a, b);
}

// Collect all nodes in the tree into a vector using DFS
void Floorplanner::collectNodes(BTreeNode* node, std::vector<BTreeNode*>& nodes) {
    if (!node) return;
    nodes.push_back(node);
    collectNodes(node->left, nodes);
    collectNodes(node->right, nodes);
}

// Remove a specific node from the tree and reconnect children
bool Floorplanner::removeNode(BTreeNode*& node, BTreeNode* target) {
    if (!node) return false;

    // If current node is the target, remove it and handle children
    if (node == target) {
        if (target->left) {
            // Replace with left child and attach right subtree
            BTreeNode* right_subtree = target->right;
            node = target->left;
            insertSubtree(node, right_subtree);
        } else {
            // Replace with right child
            node = target->right;
        }
        return true;
    }

    // Recursively search in left and right subtrees
    if (removeNode(node->left, target)) return true;
    if (removeNode(node->right, target)) return true;
    return false;
}

// Insert subtree at the rightmost position of the given root
void Floorplanner::insertSubtree(BTreeNode* root, BTreeNode* subtree) {
    if (!subtree) return;
    BTreeNode* curr = root;
    // Find rightmost node
    while (curr->right) curr = curr->right;
    curr->right = subtree;
}

// Find the position where the numeric suffix starts
int Floorplanner::extract_number(const std::string& name) const{
    int i = name.size() - 1;
    while (i >= 0 && std::isdigit(name[i])) {
        --i;
    }
    ++i; // move to first digit
    if (i < name.size()) {
        return std::stoi(name.substr(i));
    }
    return 0; // Default to 0 if no digits found (shouldn't happen)
}