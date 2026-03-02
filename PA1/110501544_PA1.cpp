#include <iostream>
#include <cstring>
#include <fstream>
#include <vector>
#include <map>
#include <stack>
#include <algorithm>
using namespace std;

#define GATE_SIZE 100

typedef struct GATE{
    string type;
    string name;
    vector<int> inputs;
    vector<int> outputs;
}Gate;

typedef struct NET{
    vector<int> input, output, wire;
}Net;

// 輸出檔案 (轉換成 verilog 的形式)
void output_verilog(char *argv[] , string module_name, map<string, Gate> gate, Net net){
    ofstream outputfile(argv[2], ios::out);
    if(!outputfile)
    {
        cerr<<"File could not be opened"<<endl;
        exit(0);
    }
    outputfile << "`timescale 1ns/1ps" << endl << "module " << module_name << " (";
    // 輸出 module 的 I/O
    for(const auto &i : net.input)
        outputfile << "N" << i << ",";
    for(const auto &o : net.output)
        outputfile << "N" << o << ( &o == &net.output.back() ? ");" : "," );
    // 輸出 input, output, wire 宣告
    outputfile << endl << endl << "input ";
    for(const auto &i : net.input)
        outputfile << "N" << i << ( &i == &net.input.back() ? ";" : "," );
    outputfile << endl << "output ";
    for(const auto &o : net.output)
        outputfile << "N" << o << ( &o == &net.output.back() ? ";" : "," );
    outputfile << endl << "wire ";
    for(const auto &w : net.wire)
        outputfile << "N" << w << ( &w == &net.wire.back() ? ";" : "," );
    
    outputfile << endl;
    // 輸出 gate
    map<string, int> gate_type;  // 紀錄 gate type 的數量
    for(const auto &g : gate){
        string str;
        for (int i=0; i<g.second.type.size(); i++)
            str += toupper(g.second.type[i]);
        str += to_string( g.second.inputs.size() + g.second.outputs.size() - 1 );
        outputfile << endl << g.second.type << " " << str << "_" << (gate_type[str]++) + 1 << " (" ;
        for(const auto &o : g.second.outputs)
            outputfile << "N" << o << ", ";
        for(const auto &i : g.second.inputs){
            outputfile << "N" << i << ( &i == &g.second.inputs.back() ? ");" : ", " );
        }
    }
    outputfile << endl << endl << "endmodule";
    outputfile.close();
}

// 將 pin 分類存放在 gate 中
void classify_gate(vector<int> &pin, string pinName, vector<int> wire, string in_or_out){
    int pinNum = stoi( pinName.substr(pinName.find(in_or_out) + 1, pinName.length()) );
    while ( pin.size() < pinNum )
        pin.push_back(0);
    pin[pinNum - 1] = wire.back();
}

// 讀檔並輸出檔案
void parse(char *argv[]){
    ifstream inputfile(argv[1], ios::in);
    if(!inputfile)
    {
        cerr<<"File could not be opened"<<endl;
        exit(0);
    }
    // 讀檔
    string line, name, module_name;
    map<string, Gate> gate; 
    Net net;
    int pin_num = 0;
    while(inputfile >> line){
        if(line == "*"){
            // 取得 module name
            getline(inputfile, line);
            module_name = line.substr(line.find("module ") + 7, line.length());
        }
        else if(line == "Inst"){
            string type;
            inputfile >> name;
            inputfile >> type;   
            gate[name].type = type; // save gate type
            gate[name].name = name;     // save gate name
        }
        else if(line == "NET"){
            inputfile >> name;
            // 把 "net1" 的 "net" 去掉，只留下後面的數字
            name.erase(0,3);
            // 先把 net 存放在 wire 中
            net.wire.push_back( stoi(name) );
            inputfile >> pin_num;
        }
        else if(line == "PIN"){
            int in_outpt = 0;   // 判斷是否為 input/output wire 還是 wire 決定是否要 wire.pop_back() (把 net 移出 wire)
            getline(inputfile, line);
            char *token = strtok(&line[0], ", ");
            while (token != NULL){
                name = token;
                if(name.find("inpt") != string::npos){  // 查找是否含有 "inpt" 字串
                    // 將 net 存放在 input 中
                    net.input.push_back( net.wire.back() );
                    in_outpt = 1;
                }
                else if(name.find("outpt") != string::npos){    // 查找是否含有 "outpt" 字串
                    // 將 net 存放在 output 中
                    net.output.push_back( net.wire.back() );
                    in_outpt = 1;
                }
                else{
                    // 將 pin 分類存放在 gate 中
                    string gateName = name.substr(0, name.find("/"));
                    string pinName = name.substr(name.find("/") + 1, name.length());
                    if(pinName.find("IN") != string::npos)
                        classify_gate(gate[gateName].inputs, pinName, net.wire, "N");
                    else if(pinName.find("OUT") != string::npos)
                        classify_gate(gate[gateName].outputs, pinName, net.wire, "T");
                }
                token = strtok(NULL, ", ");    
            }
            if(in_outpt == 1) net.wire.pop_back();  //若為 input/output wire 則把 ner 移出 wire
        }
    }
    // 排序 net 大小
    sort(net.input.begin(), net.input.end());
    sort(net.output.begin(), net.output.end());
    sort(net.wire.begin(), net.wire.end());
    // 輸出檔案
    output_verilog(argv, module_name, gate, net);
}


int main(int argc, char *argv[]){
    parse(argv);
    return 0;
}