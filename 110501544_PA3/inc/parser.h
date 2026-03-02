#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>
#include <unordered_map>
using namespace std;

#define OUTPUT_LOAD 0.03  // output load capacitance
#define WIRE_DELAY 0.005  // wire delay

// store gate output load info
struct LOAD{
    string gate_name;
    double load;
};

// store gate delay info
struct DELAY{
    string gate_name;
    bool is_rise;  // true: rise, false: fall
    double delay;
    double trans;
};

// represent a path for longest/shortest delay
struct PATH{
    double delay = 0.0f;
    vector<string> path;    // ordered net names along the path
};

// represent a logic gate
struct Gate {
    string type;                // Gate type (NOR2X1, INVX1, NAND2X1)
    vector<string> inputs;      // Input nets (A1, A2, I)
    string output;              // Output net (ZN)
};

// represent a net/wire
struct Net {
    string type;                // input, output, or wire
    vector<pair<string, string>> gate_in;   // List of gates that use this net as input <gate, pin>
    string gate_out = "";                   // Gate that drives this net
    double trans = 0.0f;                    // Output transition time
    double delay = 0.0f;                    // Output delay time

    PATH worst_case;        // Path object to track worst-case path
};

// Structure to store all data from .lib cell definition
struct CellLib {
    string cell_name;                       // Cell type name
    vector<double> capacitance;             // Pin capacitance list
    vector<vector<double>> cell_rise;
    vector<vector<double>> cell_fall;
    vector<vector<double>> rise_transition;
    vector<vector<double>> fall_transition;
};

class Parser {
private:
    vector<double> Store_Value(string line);  // Extract values from a LUT line

// Public data for STA to access
public:
    // netlist information
    string module_name;
    unordered_map<string, Gate> gate_map;   // Gate name to Gate mapping
    unordered_map<string, Net> net_map;     // Net name to Net mapping
    vector<string> output_nets;             // Primary output list

    // library information
    vector<double> output_cap;              // Index_1 (capacitance values)
    vector<double> input_trans;             // Index_2 (input transition values)
    vector<CellLib> cell_lib;               // All parsed cell types

    // Parse the netlist and library files
    void Netlist_Parse(const string& filename);     // Parse netlist file
    void Lib_Parse(const string& filename);         // Parse library file
};
#endif // PARSER_HPP