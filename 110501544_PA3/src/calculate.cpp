#include "../inc/calculate.h"
#include <iostream>
#include <fstream>
#include <cstring>

using namespace std;

// Extract 2x2 values from LUT for interpolation
void Calculate::Get_F(vector<vector<double>> lut_type){
    F[0][0] = lut_type[index_2][index_1];           // Top-left
    F[1][0] = lut_type[index_2][index_1 + 1];       // Top-right
    F[0][1] = lut_type[index_2 + 1][index_1];       // Bottom-left
    F[1][1] = lut_type[index_2 + 1][index_1 + 1];   // Bottom-right
    // cout << "F[0][0]: " << F[0][0] << " F[1][0]: " << F[1][0] << " F[0][1]: " << F[0][1] << " F[1][1]: " << F[1][1] << endl;
}

// Apply bilinear interpolation over a 2x2 table F[][] with (xx, yy)
double Calculate::bilinear_interpolate(){
    // Calculate x interpolation along y0 and y1
    double R0 = F[0][0] + (F[1][0] - F[0][0]) * ((xx - x[0]) / (x[1] - x[0]));
    double R1 = F[0][1] + (F[1][1] - F[0][1]) * ((xx - x[0]) / (x[1] - x[0]));
    
    // Calculate y interpolation along R0 and R1
    return R0 + (R1 - R0) * ((yy - y[0]) / (y[1] - y[0]));
}

// Compute output load for a net ZN
LOAD Calculate::Calculate_Load(string ZN){
    LOAD value;
    value.gate_name = net_map[ZN].gate_out;     // Identify the gate that drives this net
    value.load = 0.0;
    if(net_map[ZN].type == "output") value.load = OUTPUT_LOAD;      // If net is primary output, use fixed load
    else{
        for(const auto& gate_in : net_map[ZN].gate_in){
            int pidx = (gate_in.second == "A1" || gate_in.second == "I") ? 0 : 1;       // Select pin index
            string type = gate_map[gate_in.first].type;         // Gate type connected to this net
            CellType cidx = Get_Cell_Type(type);                // Lookup cell type
            value.load += cell_lib[cidx].capacitance[pidx];     // Accumulate pin capacitance
        }
    }
    // cout << "gate: " << value.gate_name << " load: " << value.load << endl;
    return value;
}

// Compute delay and transition time of a gate based on load and input transition
DELAY Calculate::Calculate_Delay_Trans(string gate_name, double load, double delay){    
    DELAY value;
    value.is_rise = 1;
    value.gate_name = gate_name;
    // Find bounding indices for interpolation
    auto v1 = lower_bound(output_cap.begin(), output_cap.end(), load);
    auto v2 = lower_bound(input_trans.begin(), input_trans.end(), delay);
    // Ensure to get two valid interpolation points
    x[0] = (load > *(v1 - 1)) ? *(v1 - 1) : *v1;
    x[1] = (load > *(v1 - 1)) ? *v1 : *(v1 + 1);
    y[0] = (delay > *(v2 - 1)) ? *(v2 - 1) : *v2;
    y[1] = (delay > *(v2 - 1)) ? *v2 : *(v2 + 1);
    // Store the actual interpolation point
    xx = load;
    yy = delay;
    // Get indices for accessing LUTs
    index_1 = (load > *(v1 - 1)) ? v1 - output_cap.begin() - 1 : v1 - output_cap.begin();
    index_2 = (delay > *(v2 - 1)) ? v2 - input_trans.begin() - 1 : v2 - input_trans.begin();
    // Lookup cell type
    CellType cidx = Get_Cell_Type(gate_map[gate_name].type);
    
    // Interpolate cell_rise
    Get_F(cell_lib[cidx].cell_rise);
    double delay_value1 = bilinear_interpolate();
    
    // Interpolate cell_fall
    Get_F(cell_lib[cidx].cell_fall);
    double delay_value2 = bilinear_interpolate();
    
    // Use larger of rise/fall delay
    if(delay_value1 < delay_value2) {
        delay_value1 = delay_value2;
        value.is_rise = 0;
    }
    value.delay = delay_value1;
    
    // Interpolate corresponding transition
    if(value.is_rise == 1)
        Get_F(cell_lib[cidx].rise_transition);
    else 
        Get_F(cell_lib[cidx].fall_transition);
    value.trans = bilinear_interpolate();
    
    // cout << value.gate_name << " " << value.is_rise << " " << value.delay << " " << value.trans << endl;
    return value;
}

// Map gate name string to enum index
CellType Calculate::Get_Cell_Type(const string& name) {
    if (name == "NOR2X1") return NOR2X1;
    else if (name == "INVX1") return INVX1;
    else if (name == "NANDX1") return NANDX1;
    else throw invalid_argument("Unknown cell name: " + name);
}