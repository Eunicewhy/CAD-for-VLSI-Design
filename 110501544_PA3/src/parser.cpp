#include "../inc/parser.h"
#include <iostream>
#include <fstream>
#include <cstring>

using namespace std;

// extract numeric values from .lib index/value line
// for example: capacitance, cell rise, cell fall, rise transition, fall transition
vector<double> Parser::Store_Value(string line){
    vector<double> value;
    line.erase(0, line.find_first_of("\""));
    char *token = strtok(&line[0], "\" (),;\\");
    while(token != NULL){
        value.emplace_back(stof(token));
        // cout << token << " ";
        token = strtok(NULL, "\" (),;\\");
    }
    // cout << endl;
    return value;
}

// Parse Netlist File
void Parser::Netlist_Parse(const string& filename){
    ifstream inputfile(filename, ios::in);
    if(!inputfile)
    {
        cerr<<"File could not be opened"<<endl;
        exit(0);
    }
    
    // Read file
    string line, type;
    int pin_num = 0;
    while(getline(inputfile, line)){
        // store module name
        if(line.find("//") != string::npos){
            continue;       // Skip comment lines
        }
        else if(line.find("endmodule") != string::npos){
            break;          // End of module
        }
        // Parse module name
        else if(line.find("module ") != string::npos){
            char *token = strtok(&line[0], "\t (.),;");
            token = strtok(NULL, "\t (.),;");
            module_name = token;
        }
        // Parse input/output/wire declarations
        else if(line.find("input") != string::npos || line.find("output") != string::npos || line.find("wire") != string::npos){
            char *token = strtok(&line[0], "\t (.),;");
            type = token;       // Store type (input/output/wire)
            token = strtok(NULL, "\t (.),;");
            while (token != NULL){
                net_map[token].type = type;     // Assign net type
                if(type == "output") output_nets.emplace_back(token);    // Save primary outputs
                token = strtok(NULL, "\t (.),;");
            }
        }
        // Parse logic gate instances and their connections
        else if(line.find("NOR2X1") != string::npos || line.find("INVX1") != string::npos || line.find("NANDX1") != string::npos) {
            char *token = strtok(&line[0], "\t (.),;");
            type = token;           // Gate type (NOR2X1/INVX1/NANDX1)
            token = strtok(NULL, "\t (.),;");
            string gate_name = token;       // ex. U1
            gate_map[gate_name].type = type;    // Store gate type
            token = strtok(NULL, "\t (.),;");   
            while (token != NULL){
                string in_out = token;          // Pin name (A1, A2, I, ZN)
                token = strtok(NULL, "\t (.),;");
                if(in_out == "A1" || in_out == "A2" || in_out == "I"){
                    gate_map[gate_name].inputs.emplace_back(token);     // Add input net
                    net_map[token].gate_in.emplace_back(make_pair(gate_name, in_out));      // Update net fanin info
                }
                else if (in_out == "ZN"){
                    gate_map[gate_name].output = token;     // Save output net
                    net_map[token].gate_out = gate_name;    // Connect net to gate output
                }
                token = strtok(NULL, "\t (.),;");
            }
        }
    }
}

// Parse Library File
void Parser::Lib_Parse(const string& filename){
    ifstream inputfile(filename, ios::in);
    if(!inputfile)
    {
        cerr<<"File could not be opened"<<endl;
        exit(0);
    }

    string line;
    CellLib curr_cell;
    bool in_cell = false;
    while(getline(inputfile, line)){
        // store total output net capacitance
        if (line.find("index_1") != string::npos) {
            // cout << "total_output_net_capacitance: ";
            output_cap = Store_Value(line);         // Extract output load index values
        }
        // store input transition time
        else if (line.find("index_2") != string::npos) {
            // cout << "input_transition_time: ";
            input_trans = Store_Value(line);        // Extract input transition index values
        }
        // store cell information
        else if(line.find("cell (") != string::npos){
            in_cell = true;             // Start of cell information
            curr_cell = CellLib();      // Reset current cell
            curr_cell.cell_name = line.substr(line.find("(")+1, line.find(")") - line.find("(") - 1);
            // cout << "Cell: " << curr_cell.cell_name << endl; 
        }

        if(in_cell){
            // store cell capacitance
            if(line.find("capacitance") != string::npos){
                string value = line.substr(line.find(":")+2, line.find(";") - line.find(":") - 2);
                curr_cell.capacitance.emplace_back(stof(value));    // Load pin capacitance
                // cout << "capacitance: " << value << endl;
            }
            // store cell_rise
            else if(line.find("cell_rise") != string::npos){
                // cout << "cell_rise: " << endl;
                for(int i=0; i < input_trans.size(); i++){
                    getline(inputfile, line);
                    curr_cell.cell_rise.emplace_back(Store_Value(line));    // Add cell rise
                }
            }
            // store cell_fall
            else if(line.find("cell_fall") != string::npos){
                // cout << "cell_fall: " << endl;
                for(int i=0; i < input_trans.size(); i++){
                    getline(inputfile, line);
                    curr_cell.cell_fall.emplace_back(Store_Value(line));    // Add cell rise
                }
            }
            // store rise_transition
            else if(line.find("rise_transition") != string::npos){
                // cout << "rise_transition: " << endl;
                for(int i=0; i < input_trans.size(); i++){
                    getline(inputfile, line);
                    curr_cell.rise_transition.emplace_back(Store_Value(line));  // Add rise transition
                }
            }
            // store fall_transition
            else if(line.find("fall_transition") != string::npos){
                // cout << "fall_transition: " << endl;
                for(int i=0; i < input_trans.size(); i++){
                    getline(inputfile, line);
                    curr_cell.fall_transition.emplace_back(Store_Value(line));  // Add fall transition
                }
            }
            // end of current cell
            else if(line == "}"){
                cell_lib.emplace_back(curr_cell);  // Store current cell information
                in_cell = false;    // End of current cell
                // cout << "End of cell " << curr_cell.cell_name << endl << endl;
            }
        }

    }
}