#ifndef FLOORPLANNER_H
#define FLOORPLANNER_H

#include "Block.h"
#include "Contour.h"
#include <vector>
#include <utility>

// Main floorplanning class that manages B*-Tree and block placement
class Floorplanner {
public:
    std::vector<Block> blocks;      // List of blocks to be placed
    BTreeNode* root;                // Root of the B*-Tree
    Contour hcontour;               // Horizontal contour for placement
    Contour vcontour;               // Vertical contour for placement

    // Default constructor
    Floorplanner(const std::vector<Block>& bks);

    // Copy constructor for deep copying
    Floorplanner(const Floorplanner& o);

    // Assignment operator
    Floorplanner& operator=(const Floorplanner& o);

    // Destructor to clean up memory
    ~Floorplanner();

    // Build initial B*-Tree structure from blocks
    void buildInitialBTree();

    // Place blocks according to B*-Tree structure using contour packing
    void place();

    // Calculate total area of the floorplan (bounding box area)
    double area() const;

    // Calculate Integral Non-Linearity (INL) metric
    double INL() const;

    // Calculate centroid of the floorplan
    std::pair<double, double> centroid() const;

    // Calculate penalty for overlapping blocks
    double overlapPenalty() const;

    // Perform perturbation for simulated annealing (swap, variant change, move)
    void perturb();

    // Check if target node is descendant of root node
    bool isDescendant(BTreeNode* root, BTreeNode* target);

    // Print B*-Tree structure for debugging
    void printTree(BTreeNode* node, const std::string& prefix = "", bool isRight = true);

    // Find the position where the numeric suffix starts
    int extract_number(const std::string& name) const;

private:
    // Helper function to clone entire tree structure
    BTreeNode* cloneTree(BTreeNode* node);

    // Helper function to recursively delete tree
    void deleteTree(BTreeNode* node);

    // Helper function to swap block indices in B*-Tree nodes
    void swapBlockIndex(BTreeNode* node, int a, int b);

    // Collect all nodes in the tree into a vector
    void collectNodes(BTreeNode* node, std::vector<BTreeNode*>& nodes);

    // Remove a specific node from the tree
    bool removeNode(BTreeNode*& node, BTreeNode* target);

    // Insert a subtree at the rightmost position of given root
    void insertSubtree(BTreeNode* root, BTreeNode* subtree);
};

#endif // FLOORPLANNER_H