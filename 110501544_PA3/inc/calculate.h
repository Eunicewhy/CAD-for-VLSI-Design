#ifndef CALCULATE_HPP
#define CALCULATE_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include "parser.h"

using namespace std;

// Define enum to index cell types in cell_lib
enum CellType {
    NOR2X1 = 0,
    INVX1,
    NANDX1,
    NUM_CELL_TYPES // count of cell types
};

class Calculate: public Parser {
private:
    double x[2], y[2], F[2][2], xx, yy;     // Variables of Bilinear interpolate
    int index_1, index_2;                   // Indices in output_cap and input_trans arrays
    
    void Get_F(vector<vector<double>> lut_type);    // Extract 2x2 submatrix from LUT
    double bilinear_interpolate();                  // Bilinear interpolate
// Public data for STA to access
public:
    LOAD Calculate_Load(string ZN);    // Compute output load seen by net
    DELAY Calculate_Delay_Trans(string gate_type, double load, double delay);   // Compute delay and transition time
    CellType Get_Cell_Type(const string& name);           // Map gate name to CellType enum
};

#endif 
