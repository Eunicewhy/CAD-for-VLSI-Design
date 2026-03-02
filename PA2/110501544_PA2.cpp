#include <iostream>
#include <fstream>
#include <cstring>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <algorithm>
#include <limits.h>
#include <queue>
#include <time.h>
using namespace std;

struct Cell {
    int gain = 0;
    int region = -1;    // 0: left region, 1: right region
    bool locked = false;
    unordered_set<string> nets;
};

struct Net {
    unordered_set<int> cells;
    int count_left = 0;     // count of cells in the left region
    int count_right = 0;    // count of cells in the right region
};

bool cmp_value(const pair<int, int>& a, const pair<int, int>& b) {
    return a.second > b.second;   // sort in descending order of gain
}

// Global Variables
unordered_map<string, Net> nets;
unordered_map<int, Cell> cells;
int balance_limit = 0;      // constraint: |A - B| <= total_cells/5
int count_left = 0, count_right = 0;  // count of cells in the left and right region

// Functions
int Calculate_Gain(int cell_id);
void Input_Parse(const string& filename, int &cell_count);
void Initial_Partition_Random();
void Initial_Partition_Greedy();
bool can_move(int cell_id);
int FM_Algorithm(int cell_count);
void Run_FM(int cell_count, const string &filename, chrono::time_point<chrono::high_resolution_clock> START);
int Compute_Cut_Size();
void Output_Result(const string &filename);


// Main Function
int main(int argc, char *argv[]){
    const auto start = chrono::high_resolution_clock::now();
    if(argc < 3){
        cout << "Usage: " << argv[0] << " input.txt output.txt" << endl;
        return 1;
    }
    int method = 0;
    if(argc == 4) method = stoi(argv[3]);
    int cell_count = 0;
    // Read file
    Input_Parse(argv[1], cell_count);

    // Initial Solution
    if(method == 1) Initial_Partition_Random();
    else Initial_Partition_Greedy();
    cout << "Initial cut size: " << Compute_Cut_Size() << endl << endl;

    // Initialize the gain of each cell and output the initial result (calculate once first)
    for(auto &it : cells){
        it.second.gain = Calculate_Gain(it.first);
    }
    Output_Result(argv[2]);

    // Call Run_FM for multiple passes
    Run_FM(cell_count, argv[2], start);

    // Output final result
    Output_Result(argv[2]);

    cout << "end iteration" << endl;
    auto finish = chrono::high_resolution_clock::now();
    cout << "runtime: " << (float)chrono::duration_cast<chrono::microseconds>(finish-start).count() / 1000000  << " S" << endl;
    return 0;
}


// 1. Parse the input file
void Input_Parse(const string& filename, int &cell_count){
    ifstream inputfile(filename, ios::in);
    if(!inputfile)
    {
        cerr << "File could not be opened" << endl;
        exit(0);
    }
    string line;
    while(inputfile >> line){
        if(line == "NET"){
            inputfile >> line;
            string net_name = line;
            getline(inputfile, line);
            char *token = strtok(&line[0], "c{ }");
            // Save the information of each cell
            while (token != NULL){
                int id = stoi(token);
                if(cells[id].region == -1){
                    cells[id].region = 0; // default region
                    cell_count++;
                }
                cells[id].nets.insert(net_name);
                nets[net_name].cells.insert(id);
                token = strtok(NULL, "c{ }");
            }
        }
    }
    balance_limit = cell_count / 5;     // constraint: |A - B| <= total_cells/5
    if(balance_limit < 1) balance_limit = 1; // at least 1
    inputfile.close();
}

// 2. Initial Solution
// Method 1: randomly assign region
void Initial_Partition_Random(){
    // As initial statistics for the count of cells in each region
    count_left = 0;
    count_right = 0;
    srand(time(NULL));

    for(auto &c: cells){
        // Randomly assign the cell's region (0 or 1)
        c.second.region = rand() % 2;
        int cid = c.first;
        // Update nets and the counts in both regions
        if(cells[cid].region == 0){
            count_left++;
            for(auto &n: cells[cid].nets)
                nets[n].count_left++;
        }
        else{
            count_right++;
            for(auto &n: cells[cid].nets)
                nets[n].count_right++;
        }
    }
}

// Method 2: Greedy Partitioning Based on Connectivity
// Sort by the number of connections of cells, and sequentially choose partitions that minimize the cut impact
void Initial_Partition_Greedy() {
    // First mark all cells' regions as -1 (unassigned region)
    for (auto &p : cells)
        p.second.region = -1;
    
    // Sort by the degree of each cell, higher degree is prioritized
    vector<pair<int, int>> cellDeg; // pair<cell_id, degree>
    for (auto &p : cells) {
        cellDeg.push_back({p.first, (int)p.second.nets.size()});
    }
    sort(cellDeg.begin(), cellDeg.end(), cmp_value);
    
    // As initial statistics for the count of cells in each region
    count_left = 0; 
    count_right = 0;
    
    // For each cell, simulate the cost of placing it in the left or right region
    for (auto &entry : cellDeg) {
        int cid = entry.first;
        int cost_left = 0, cost_right = 0;
        // Check each net connected to the cell
        for (auto &netName : cells[cid].nets) {
            // If there are already cells assigned to the net, increase the count for that region
            int left = 0, right = 0;
            for (int neigh : nets[netName].cells) {
                if (cells[neigh].region == 0) left++;
                else if (cells[neigh].region == 1) right++;
            }
            // If no cells are assigned to the net, placing it anywhere has no cost
            if(left == 0 && right == 0) continue;
            // If placed in the left region:
            // If there are already cells in the right region, placing it in the left will increase the cost by 1
            if(right > 0)
                cost_left++;
            // Similarly for placing in the right region:
            if(left > 0)
                cost_right++;
        }
        int chosen_region = -1;
        // If the count difference in cells between regions has reached the balance limit, choose the lesser populated side
        if(abs((count_left + 1) - count_right) > balance_limit) {
            chosen_region = 1;
        } 
        else if (abs(count_left - (count_right + 1)) > balance_limit) {
            chosen_region = 0;
        } 
        // If the cell count difference is not significant, choose the region with lower cost
        else {
            chosen_region = (cost_left <= cost_right) ? 0 : 1;
        }
        cells[cid].region = chosen_region;

        // Update the count_left and count_right in nets
        if(chosen_region == 0){
            count_left++;
            for(auto &n: cells[cid].nets)
                nets[n].count_left++;
        }
        else{
            count_right++;
            for(auto &n: cells[cid].nets)
                nets[n].count_right++;
        }
    }
}

// 3. Calculate the gain value of a cell (gain = # nets cut -> uncut - # nets uncut -> cut)
int Calculate_Gain(int cell_id) {
    int gain = 0;
    // Get the current region of the cell
    int from_region = cells[cell_id].region;
    // Traverse all nets connected to the cell
    for (const auto &n : cells[cell_id].nets) {
        // Current count of cells in the net's regions
        int original_left = nets[n].count_left;
        int original_right = nets[n].count_right;
        // Simulate the count of cells in the net's regions after moving
        int tmp_left = original_left;
        int tmp_right = original_right;

        // Subtract the cell from its original region and add it to the opposite region
        if (from_region == 0) {
            tmp_left--;
            tmp_right++;
        }
        else {
            tmp_right--;
            tmp_left++;
        }

        // Check if the net is a cut net before and after the move
        bool before_cut = (original_left > 0 && original_right > 0);
        bool after_cut  = (tmp_left > 0 && tmp_right > 0);
        if (before_cut && !after_cut)
            gain++;   // From cut to uncut, gain +1
        else if (!before_cut && after_cut)
            gain--;   // From uncut to cut, gain -1
    }
    return gain;
}

// Check if the balance constraint allows the cell to move (constraint: |A - B| <= total_cells/5)  
bool can_move(int cell_id) {
    int from = cells[cell_id].region;
    // Expected counts of cells in the regions after the move
    int new_left = (from == 0) ? (count_left - 1) : (count_left + 1);
    int new_right = (from == 1) ? (count_right - 1) : (count_right + 1);
    // Check balance
    return abs(new_left - new_right) <= balance_limit;
}

// 4. Single FM Algorithm pass: find the highest gain nodes from both regions and choose a node to move based on rules  
// Algorithm Description:
// (1) Identify the nodes with the highest gain value from both region A and B, resulting in two candidate nodes.
// (2) Eliminate any candidate nodes that would cause an imbalance if moved. If both candidate nodes are eliminated, the algorithm ends.
// (3) Among the two candidate nodes, select the one with the highest gain value to move to the opposite region. If both candidate nodes have the same gain value, choose the one that results in a better balance. If they are still the same, prefer the node from region A.
// (4) Calculate the cumulative gain of the move sequence and find the best cumulative gain point, recording the partition status at that time. If there is no cumulative gain, record the partition status before executing FM.
int FM_Algorithm(int cell_count) {
    // First unlock all cells and recalculate the initial gain
    for (auto &kv : cells) {
        kv.second.locked = false;
        kv.second.gain = Calculate_Gain(kv.first);
    }
    // Move record to keep track of each cell_id and its original gain
    struct MoveRecord {
        int cell_id;
        int gain;
        int from;  // original region
        int to;    // moved region
    };
    vector<MoveRecord> moveSeq;
    moveSeq.reserve(cell_count);
    
    int num_locked = 0;
    // Repeatedly select candidate moves until all cells are locked or no valid moves are available
    while (num_locked < cell_count) {
        // Find the highest gain candidate nodes from A and B
        int candidate_A = -1, candidate_B = -1;
        int best_gain_A = INT_MIN, best_gain_B = INT_MIN;
        for (const auto &c : cells) {
            int id = c.first;
            if (!cells[id].locked && cells[id].region == 0 && can_move(id)) {
                if (cells[id].gain > best_gain_A) {
                    best_gain_A = cells[id].gain;
                    candidate_A = id;
                }
            }
            if (!cells[id].locked && cells[id].region == 1 && can_move(id)) {
                if (cells[id].gain > best_gain_B) {
                    best_gain_B = cells[id].gain;
                    candidate_B = id;
                }
            }
        }
        
        // If there are no candidates on both sides, terminate this FM pass
        if (candidate_A == -1 && candidate_B == -1)
            break;
        // If only one side has a candidate, choose that one
        int chosen = -1;
        if (candidate_A == -1)
            chosen = candidate_B;
        else if (candidate_B == -1)
            chosen = candidate_A;
        else {
            // If both sides have candidates, compare their gains
            if (best_gain_A > best_gain_B)
                chosen = candidate_A;
            else if (best_gain_B > best_gain_A)
                chosen = candidate_B;
            else {
                // If the gains are equal, calculate the balance after moving and 
                // choose the candidate that minimizes the difference in counts between the two regions
                int new_left_A = count_left - 1;
                int new_right_A = count_right + 1;
                int new_left_B = count_left + 1;
                int new_right_B = count_right - 1;

                int diff_A = abs(new_left_A - new_right_A);
                int diff_B = abs(new_left_B - new_right_B);
                if (diff_A <= diff_B)
                    chosen = candidate_A;   // If diff_A = diff_B, choose candidate_A
                else
                    chosen = candidate_B;
            }
        }
        // Execute the chosen move
        int from = cells[chosen].region;
        int to = 1 - from;
        // Record the gain and partition change of this move
        moveSeq.push_back({ chosen, cells[chosen].gain, from, to });
        // Lock the cell
        cells[chosen].locked = true;
        // Update global region cell counts
        if (from == 0) {
            count_left--;
            count_right++;
        } 
        else {
            count_left++;
            count_right--;
        }
        // Update the count of cells in the corresponding regions for the nets the cell belongs to
        for (const auto &netName : cells[chosen].nets) {
            if (from == 0) {
                nets[netName].count_left--;
                nets[netName].count_right++;
            } 
            else {
                nets[netName].count_right--;
                nets[netName].count_left++;
            }
        }
        // Actually update the cell's region to the opposite
        cells[chosen].region = to;
        num_locked++;
        // Update the gain values of neighboring cells sharing the same nets
        for (const auto &netName : cells[chosen].nets) {
            for (const auto &cid : nets[netName].cells) {
                if (!cells[cid].locked) {
                    cells[cid].gain = Calculate_Gain(cid);
                }
            }
        }
    } 
    
    // Calculate the cumulative gain of the move sequence and find the best cumulative gain point (maximum partial sum)
    int curSum = 0;
    int bestSum = INT_MIN;
    int bestIndex = -1;
    for (int i = 0; i < moveSeq.size(); i++) {
        curSum += moveSeq[i].gain;
        if (curSum > bestSum) {
            bestSum = curSum;
            bestIndex = i;
        }
    }
    // If the best cumulative gain is less than or equal to 0, rollback all moves
    if (bestSum <= 0) {
        for (int i = moveSeq.size() - 1; i >= 0; i--) {
            int cid = moveSeq[i].cell_id;
            // The original move direction was from -> to, this rollback will be in reverse
            int to = moveSeq[i].from;
            cells[cid].region = to;
            if (to == 0) { 
                count_left++; 
                count_right--; 
            }
            else { 
                count_left--; 
                count_right++; 
            }
            for (const auto &netName : cells[cid].nets) {
                if (to == 0) {
                    nets[netName].count_left++;
                    nets[netName].count_right--;
                } 
                else {
                    nets[netName].count_left--;
                    nets[netName].count_right++;
                }
            }
        }
        return 0;
    } 
    else {
        // Rollback all moves from bestIndex + 1 to the end
        for (int i = moveSeq.size() - 1; i > bestIndex; i--) {
            int cid = moveSeq[i].cell_id;
            int to = moveSeq[i].from;
            cells[cid].region = to;
            if (to == 0) { 
                count_left++; 
                count_right--; 
            }
            else { 
                count_left--; 
                count_right++; 
            }
            for (const auto &netName : cells[cid].nets) {
                if (to == 0) {
                    nets[netName].count_left++;
                    nets[netName].count_right--;
                } 
                else {
                    nets[netName].count_left--;
                    nets[netName].count_right++;
                }
            }
        }
        return bestSum;
    }
}

// 5. Repeatedly call FM_Algorithm until no positive gain is achieved
void Run_FM(int cell_count, const string &filename, chrono::time_point<chrono::high_resolution_clock> START) {
    int iteration = 0;
    while (true) {
        int pass_gain = FM_Algorithm(cell_count);
        iteration++;
        cout << "iteration: "<< iteration << endl;
        int curr_cut_size = Compute_Cut_Size();
        cout << "Current cut size: " << curr_cut_size << endl;
        if (pass_gain <= 0)
            break;
        auto TIME = chrono::high_resolution_clock::now();
        // If execution time exceeds 8 minutes, output results after each iteration
        if((float)(chrono::duration_cast<chrono::milliseconds>(TIME - START).count()/1000/60) >= 8){
            Output_Result(filename);
        }
        if((float)(chrono::duration_cast<chrono::milliseconds>(TIME - START).count()/1000/60) >= 10){
            break;
        }
    }
}

// Calculate the current cut size
int Compute_Cut_Size(){
    int cut = 0;
    for(const auto &kv : nets){
        const Net &nref = kv.second;
        if(nref.count_left > 0 && nref.count_right > 0){
            cut++;
        }
    }
    return cut;
}

// 6. Output results to the output file
void Output_Result(const string &filename) {
    ofstream outputfile(filename);
    if(!outputfile){
        cerr << "cannot open output file: " << filename << endl;
        exit(1);
    }
    int cut_size = Compute_Cut_Size();
    outputfile << "cut_size " << cut_size << "\nA\n";
    for(const auto &p : cells) {
        if(p.second.region == 0)
            outputfile << "c" << p.first << "\n";
    }
    outputfile << "B\n";
    for(const auto &p : cells) {
        if(p.second.region == 1)
            outputfile << "c" << p.first << "\n";
    }
    outputfile.close();
}
