#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <string>
#include <ctime>
#include <cstdlib>
#include <algorithm> // for std::sort
#include "inc/Block.h"
#include "inc/Contour.h"
#include "inc/Floorplanner.h"
#include "inc/SimulatedAnnealing.h"

using namespace std;

int extract_number(const std::string& name){
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

// Read block information from input file
// Format: block_name (width height col_mult row_mult) ...
vector<Block> readBlocks(const string& filename) {
    vector<Block> blocks;
    ifstream infile(filename);
    string line;
    
    // Process each line in the input file
    while (getline(infile, line)) {
        if (line.empty()) continue;
        
        istringstream iss(line);
        Block b;
        iss >> b.name; // Read block name
        
        string token;
        // Read all size options for this block
        while (iss >> token) {
            // Remove opening parenthesis
            if (token.front() == '(') token = token.substr(1);
            
            // Parse width, height, column multiplier, row multiplier
            double w = stod(token);
            iss >> token; 
            double h = stod(token);
            iss >> token; 
            int c = stoi(token);
            iss >> token; 
            // Remove closing parenthesis
            if (token.back() == ')') token.pop_back();
            int r = stoi(token);
            
            // Add this option to the block
            b.options.push_back({w, h, c, r});
        }
        
        // Validate that block has at least one size option
        if (b.options.empty()) {
            cerr << "Error: Invalid size option for device " << b.name << endl;
            exit(1);
        }
        
        blocks.push_back(b);
    }
    
    return blocks;
}

// Write floorplanning results to output file
// Format: area, chip_width chip_height, INL, block_name x y (width height col_mult row_mult)
void writeOutput(const std::string& filename, const Floorplanner& fp) {
    std::ofstream out(filename);
    out << std::fixed << std::setprecision(4);

    // Write total area
    out << fp.area() << "\n";

    // Write chip dimensions
    out << std::setprecision(2);
    double xmax = 0, ymax = 0;
    for (const auto& b : fp.blocks) {
        xmax = std::max(xmax, b.x + b.width());
        ymax = std::max(ymax, b.y + b.height());
    }
    out << xmax << " " << ymax << "\n";

    // Write INL value
    out << fp.INL() << "\n";

    // Copy and sort blocks by numeric suffix of name
    std::vector<Block> sorted_blocks = fp.blocks;
    std::sort(sorted_blocks.begin(), sorted_blocks.end(),
        [](const Block& a, const Block& b) {
            return extract_number(a.name) < extract_number(b.name);
        });

    // Write block placements
    for (const auto& b : sorted_blocks) {
        const auto& o = b.options[b.selected_option];
        out << b.name << " " << b.x << " " << b.y
            << " (" << o.width << " " << o.height << " "
            << o.col_mult << " " << o.row_mult << ")\n";
    }
}

// Main function - entry point of the program
int main(int argc, char* argv[]) {
    // Check command line arguments
    if (argc != 3) {
        cerr << "Usage: ./placer input.block output.out\n";
        return 1;
    }
    
    // Initialize random seed for reproducible results
    srand(time(nullptr));

    // Read input blocks from file
    vector<Block> blocks = readBlocks(argv[1]);
    
    // Create initial floorplanner with input blocks
    Floorplanner initial(blocks);
    initial.buildInitialBTree();  // Build initial B*-Tree structure
    initial.place();              // Place blocks using contour packing

    // Run simulated annealing optimization
    SimulatedAnnealing sa(initial);
    sa.run();

    // Get the best solution and write to output file
    Floorplanner best = sa.getBest();
    writeOutput(argv[2], best);
    
    return 0;
}