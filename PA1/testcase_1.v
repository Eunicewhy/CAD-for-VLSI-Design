`timescale 1ns/1ps
module me1 (N1,N2,N3,N6,N10);

input N1,N2,N3;
output N6,N10;
wire N4,N5,N7,N8,N9;

xor XOR2_1 (N4, N1, N2);
xor XOR2_2 (N5, N4, N3);
and AND2_1 (N7, N4, N3);
and AND2_2 (N8, N1, N2);
or OR2_1 (N9, N7, N8);
buf BUF1_1 (N6, N5);
buf BUF1_2 (N10, N9);

endmodule
