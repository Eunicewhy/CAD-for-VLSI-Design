# PA4 cpp compiler
.PHONY: all run run1 run2 check1 check2 draw1 draw2 clean
all: 110501544_PA4.cpp src/Contour.cpp src/Floorplanner.cpp src/SimulatedAnealing.cpp
	g++ -std=c++11 110501544_PA4.cpp src/Contour.cpp src/Floorplanner.cpp src/SimulatedAnealing.cpp -o exe
run:
	./exe $(input) $(output)
run1:
	./exe case1.block case1.out
run2:
	./exe case2.block case2.out
check1:
	./PA4_checker case1.block case1.out
check2:
	./PA4_checker case2.block case2.out
draw1:
	python visualizer.py case1.out
draw2:
	python visualizer.py case2.out
clean:
	rm exe
	rm -rf *.txt



