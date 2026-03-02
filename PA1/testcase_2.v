`timescale 1ns/1ps
module c17 (N2,N3,N6,N7,N11,N22,N233);

input N2,N3,N6,N7,N11;
output N22,N233;
wire N11,N16,N19,N100;

nand NAND2_1 (N100, N11, N3);
nand NAND2_2 (N11, N3, N6);
nand NAND2_3 (N16, N2, N11);
nand NAND2_4 (N19, N11, N7);
nand NAND2_5 (N22, N100, N16);
nand NAND2_6 (N233, N16, N19);

endmodule
