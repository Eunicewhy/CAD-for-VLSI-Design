#ifndef SIMULATED_ANNEALING_H
#define SIMULATED_ANNEALING_H

#include "Floorplanner.h"
#include <chrono>

// Simulated Annealing optimization class for floorplanning
class SimulatedAnnealing {
public:
    // Constructor with optimization parameters
    SimulatedAnnealing(Floorplanner initial, int max_iter = 1000000, 
                      double init_temp = 100.0, double alpha = 0.7);

    // Run the simulated annealing optimization process
    void run();

    // Get the best solution found during optimization
    Floorplanner getBest();

private:
    Floorplanner best;          // Best solution found so far
    Floorplanner current;       // Current solution in optimization
    Floorplanner Initial;       // Initial solution for reference
    double best_cost;           // Cost of the best solution
    int ITER;                   // Maximum number of iterations
    double T;                   // Initial temperature
    double A;                   // Alpha parameter for cost function weighting
    double area_ref, inl_ref;   // Reference values for normalization
    double total_time;
    std::chrono::steady_clock::time_point deadline;  // Time limit for optimization
    std::chrono::steady_clock::time_point start_time;  // Time limit for optimization

    // Calculate cost function combining area, aspect ratio, and INL
    double cost(const Floorplanner& fp) const;

    // Determine whether to accept a worse solution based on temperature
    bool accept(double new_cost) const;
};

#endif // SIMULATED_ANNEALING_H