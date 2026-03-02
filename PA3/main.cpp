#include <iostream>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <chrono>
#include "inc/calculate.h"
using namespace std;

// Comparator: sort gate load
bool cmp_load(const LOAD& a, const LOAD& b) {
    if(a.load == b.load) return stoi(a.gate_name.substr(1)) < stoi(b.gate_name.substr(1));   // sort by gate name if load is equal
    return a.load > b.load;   // sort in descending order of gain
}

// Comparator: sort gate delay
bool cmp_delay(const DELAY& a, const DELAY& b) {
    if(a.delay == b.delay) return stoi(a.gate_name.substr(1)) < stoi(b.gate_name.substr(1));   // sort by gate name if load is equal
    return a.delay > b.delay;   // sort in descending order of gain
}

class STA: public Calculate{
private:
    vector<LOAD> gate_load;     // Stores gate load results
    vector<DELAY> gate_delay;   // Stores gate delay results

    PATH longest, shortest;     // Store longest and shortest paths

    void Output_Load();         // Write gate load to file
    void Output_Delay();        // Write gate delay to file
    void Output_Path();         // Write critical paths to file
    void Recursive(string ZN);  // Recursive traversal from output back to inputs

public:
    STA(char* argv[]);          // Constructor to initialize the STA object and parse the files
    void print_gate_and_net();  // Debug print (not used)
    void run();                 // Run STA process
};

int main(int argc, char* argv[]) {
    auto start = chrono::high_resolution_clock::now();  // Start timer
    if (argc < 3) {
        cout << "Usage: " << argv[0] << " netlist.txt lib.txt" << endl;
        return 1;
    }

    STA sta(argv);
    // sta.print_gate_and_net();
    sta.run();
    auto finish = chrono::high_resolution_clock::now(); // Stop timer
    auto duration = chrono::duration_cast<chrono::microseconds>(finish-start);
    cout << endl << "Execution Time: " << duration.count()  << " microseconds" << endl;
    return 0;
}

// Constructor: parse input files
STA::STA(char* argv[]){
    Netlist_Parse(argv[1]);
    Lib_Parse(argv[2]); 
}

// Output gate load values to file
void STA::Output_Load(){
    // name the output file
    string filename = "Load_110501544_";
    filename += module_name + ".txt";
    // open the output file
    ofstream outfile(filename);
    if(!outfile){
        cerr << "cannot open output file: " << filename << endl;
        exit(1);
    }
    // sort the gate load by load value and gate name
    sort(gate_load.begin(), gate_load.end(), cmp_load);
    // output the gate laod to the file
    for(const auto& gate : gate_load){
        outfile << gate.gate_name << " " << fixed << setprecision(6) << gate.load << endl;
    }
    outfile.close();
}

// Output gate delay and transition time to file
void STA::Output_Delay(){
    // name the output file
    string filename = "Delay_110501544_";
    filename += module_name + ".txt";
    // open the output file
    ofstream outfile(filename);
    if(!outfile){
        cerr << "cannot open output file: " << filename << endl;
        exit(1);
    }
    // sort the gate load by load value and gate name
    sort(gate_delay.begin(), gate_delay.end(), cmp_delay);
    // output the gate load to the file
    for(const auto& gate : gate_delay){
        outfile << gate.gate_name << " " << gate.is_rise << " " << fixed << setprecision(6) << gate.delay << " " << gate.trans << endl;
    }
    outfile.close();
}

// Output longest and shortest timing paths to file
void STA::Output_Path(){
    string filename = "Path_110501544_";
    filename += module_name + ".txt";
    // open the output file
    ofstream outfile(filename);
    if(!outfile){
        cerr << "cannot open output file: " << filename << endl;
        exit(1);
    }

    // Longest path
    outfile << "Longest delay = " << fixed << setprecision(6) << longest.delay;
    outfile << ", the path is: ";
    for (size_t i = 0; i < longest.path.size(); ++i) {
        outfile << longest.path[i];
        if (i != longest.path.size() - 1)
            outfile << " -> ";
    }
    outfile << endl;

    // Shortest path
    outfile << "Shortest delay = " << fixed << setprecision(6) << shortest.delay; 
    outfile << ", the path is: ";
    for (size_t i = 0; i < shortest.path.size(); ++i) {
        outfile << shortest.path[i];
        if (i != shortest.path.size() - 1)
            outfile << " -> ";
    }
    outfile.close();
}

// Recursive function to compute delay and transition for net ZN
void STA::Recursive(string ZN){
    // cout << "ZN: " << ZN << " ";
    if(net_map[ZN].type == "input") {
        // cout << endl;
        net_map[ZN].worst_case.path.push_back(ZN);      // Base case: primary input
        return;
    }
    else if(net_map[ZN].delay != 0) 
    {
        // cout << endl;
        return;  // if the net is already visited, return
    }
    string gate = net_map[ZN].gate_out;
    double candidate_trans = 0.0f;
    string candidate_path_net;
    // Handle 2-input gates
    if(gate_map[gate].inputs.size() == 2){
        string A1 = gate_map[gate].inputs[0];
        string A2 = gate_map[gate].inputs[1];
        // cout << "A1: " << A1 ;
        // cout << " A2: " << A2 << endl;
        Recursive(A1);      // Process A1
        Recursive(A2);      // Process A2

        // Use the input with higher delay for transition time
        candidate_trans = (net_map[A1].delay >= net_map[A2].delay) ? net_map[A1].trans : net_map[A2].trans;
        
        // Use the input with the longer path as the path source
        candidate_path_net = (net_map[A1].worst_case.delay >= net_map[A2].worst_case.delay) ? A1 : A2;
    }
    // Handle single-input gates
    else if(gate_map[gate].inputs.size() == 1){
        string I = gate_map[gate].inputs[0];
        PATH path_I;
        // cout << "I: " << I << endl;
        Recursive(I);
        candidate_trans = net_map[I].trans;
        candidate_path_net = I;
    }
    // Calculate the output load
    gate_load.emplace_back(Calculate_Load(ZN));

    // Calculate delay and transition using bilinear interpolation
    gate_delay.emplace_back(Calculate_Delay_Trans(gate, gate_load.back().load, candidate_trans));
    net_map[ZN].trans = gate_delay.back().trans;
    net_map[ZN].delay = gate_delay.back().delay;

    // Update worst-case delay and path
    net_map[ZN].worst_case.delay = net_map[candidate_path_net].worst_case.delay + net_map[ZN].delay;
    // Add wire delay except for primary output
    if(net_map[ZN].type != "output") 
        net_map[ZN].worst_case.delay += WIRE_DELAY;  
    // Copy the path and append this net
    net_map[ZN].worst_case.path = net_map[candidate_path_net].worst_case.path;
    net_map[ZN].worst_case.path.push_back(ZN);
    return;
}

// Run STA for all output nets
void STA::run(){
    for(auto& output : output_nets){
        Recursive(output);  
        // Update longest delay
        if(net_map[output].worst_case.delay > longest.delay){
            longest.delay = net_map[output].worst_case.delay;
            longest.path = net_map[output].worst_case.path;
        }
        // Update shortest delay
        if(net_map[output].worst_case.delay < shortest.delay || shortest.delay == 0){
            shortest.delay = net_map[output].worst_case.delay;
            shortest.path = net_map[output].worst_case.path;
        }
    }
    // Write results to file
    Output_Load();
    Output_Delay();
    Output_Path();
}


void STA::print_gate_and_net(){
    // cout << "Module Name: " << module_name << endl;
    // cout << "Gates:" << endl;
    for(auto it = gate_map.begin(); it != gate_map.end(); it++){
        // cout << it->first << ": " << it->second.type << endl;
        // cout << "Inputs: ";
        for(auto input : it->second.inputs){
            // cout << input << " ";
        }
        // cout << endl;
        // cout << "Output: " << it->second.output << endl;
    }
    // cout << endl;
    // cout << "Nets:" << endl;
    for(auto it = net_map.begin(); it != net_map.end(); it++){
        // cout << it->first << ": " << it->second.type << endl;
        // cout << "Gate In: ";
        for(auto gate : it->second.gate_in){
            // cout << "("<< gate.first<< ", "<< gate.second<< ") ";
        }
        // cout << endl;
        // cout << "Gate Out: " << it->second.gate_out << endl;
    }
    // CellType idx = Get_Cell_Type("NOR2X1");
    // // cout << "cell name: " << cell_lib[idx].fall_transition[6][6] << endl;
}