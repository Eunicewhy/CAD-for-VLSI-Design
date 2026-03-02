#ifndef BLOCK_H
#define BLOCK_H

#include <vector>
#include <string>
#include <cctype>

// Structure to define different size options for a block
struct BlockOption {
    double width, height;       // Dimensions of the block option
    int col_mult, row_mult;     // Column and row multipliers for layout
};

// Structure representing a block in the floorplan
struct Block {
    std::string name;                           // Name/identifier of the block
    std::vector<BlockOption> options;           // Available size options for this block
    int selected_option = 0;                    // Currently selected option index
    double x = 0, y = 0;                       // Position coordinates in the floorplan

    // Get width of currently selected option
    double width() const { return options[selected_option].width; }
    
    // Get height of currently selected option
    double height() const { return options[selected_option].height; }
};

// Node structure for B*-Tree representation
struct BTreeNode {
    int block_idx;              // Index of the block this node represents
    BTreeNode* left;            // Right child (represents blocks to the right)
    BTreeNode* right;           // Upper child (represents blocks above)
    
    // Constructor to initialize node with block index
    BTreeNode(int idx) : block_idx(idx), left(nullptr), right(nullptr) {}
};

#endif // BLOCK_H