`timescale 1ns/1ps
module A_ccOOIYT (N2,N3,N6,N7,N8,N29,N17,N24,N27,N33);

input N2,N3,N6,N7,N8,N29;
output N17,N24,N27,N33;
wire N10,N11,N16,N19,N23;

nand NAND2_1 (N10, N8, N3);
not NOT1_1 (N27, N23);
nand NAND2_2 (N11, N3, N6);
or OR3_1 (N23, N6, N3, N11);
nand NAND2_3 (N19, N11, N7);
xor XOR2_1 (N17, N10, N16);
and AND3_1 (N24, N8, N19, N16);
nor NOR2_1 (N16, N2, N11);
and AND4_1 (N33, N8, N7, N19, N29);

endmodule