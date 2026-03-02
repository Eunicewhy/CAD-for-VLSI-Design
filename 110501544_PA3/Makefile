# PA3 cpp compiler
.PHONY: all run main run1 run2 check1 check2 clean
all: main.cpp src/parser.cpp src/calculate.cpp
	g++ -std=c++11 main.cpp src/parser.cpp src/calculate.cpp -o exe
run:
	./exe $(input) test_lib.lib
run1:
	./exe c17.v test_lib.lib
run2:
	./exe example.v test_lib.lib
check1:
	./PA3_checker c17.v test_lib.lib  Load_110501544_c17.txt Delay_110501544_c17.txt Path_110501544_c17.txt
check2:
	./PA3_checker example.v test_lib.lib  Load_110501544_example.txt Delay_110501544_example.txt Path_110501544_example.txt
clean:
	rm exe

