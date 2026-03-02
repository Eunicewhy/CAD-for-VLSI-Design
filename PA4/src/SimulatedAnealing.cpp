#include "../inc/SimulatedAnnealing.h"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <algorithm>

using namespace std;

// Constructor initializes SA parameters and reference values
SimulatedAnnealing::SimulatedAnnealing(Floorplanner initial, int max_iter, double init_temp, double alpha)
    : best(initial), current(initial), Initial(initial), ITER(max_iter), T(init_temp), A(alpha) {
    best_cost = cost(best);
    // Use initial solution's area and INL as normalization reference
    area_ref = initial.area();
    inl_ref = initial.INL();
    // Avoid division by zero
    if (inl_ref == 0) inl_ref = 1.0;

    // Set time deadline for optimization (590 seconds)
    total_time = 590.0;  // seconds
    start_time = chrono::steady_clock::now();
    deadline = start_time + chrono::seconds((int)total_time);
}

// Main optimization loop using simulated annealing
void SimulatedAnnealing::run() {
    cout << "initial area: " << area_ref << ", initial INL: " << inl_ref << endl;
    
    // Main iteration loop
    for (int iter = 0; iter < ITER; ++iter) {
        // Check time limit
        if (chrono::steady_clock::now() > deadline) {
            cout << "Time up at iter " << iter << "\n";
            break;
        }
        
        double Temp = T;
        // double factor = schedule(iter);
        bool uphill = true;     // Flag for uphill moves
        // float prob = 0.99;      // Cooling probability
        Floorplanner next = Initial; // Start from initial solution

        auto now = chrono::steady_clock::now();
        double elapsed = chrono::duration<double>(now - start_time).count();

        // Calculate cooling factor based on elapsed time
        double factor;
        if (elapsed < 0.4 * total_time) {
            factor = 0.98;      // Initial phase: fast cooling
        }
        else if (elapsed < 0.7 * total_time) {
            factor = 0.995;     // Middle phase: moderate cooling
        }
        else {
            factor = 0.999;     // Final phase: slow cooling
        }
        
        // Inner loop for temperature cooling
        do {
            // Generate new solution by perturbation
            next.perturb();
            next.place();

            double next_cost = cost(next);
            
            // Accept solution if better or based on probability
            if ((next_cost < best_cost || accept(next_cost))) {
                current = next;
                
                // Update best solution if improvement found and no overlap
                if (next_cost < best_cost && next.overlapPenalty() == 0) {
                    best = next;
                    best_cost = next_cost;
                    cout << "best cost: " << best_cost << endl;
                }
            }

            // Cool down temperature
            Temp *= factor;

            next = current;
        } while (Temp > 1e-3); // Continue until temperature is very low
    }
    
    // Print final tree structure
    best.printTree(best.root);
}

// Return the best solution found
Floorplanner SimulatedAnnealing::getBest() {
    Block& block = best.blocks[best.root->block_idx];
    cout << "best root: " << block.name << ", best cost: " << cost(best) << endl;
    cout << "best area: " << best.area() << ", best INL: " << best.INL() << endl;
    return best;
}

// Calculate cost function combining area and INL metrics
double SimulatedAnnealing::cost(const Floorplanner& fp) const {
    // 1. Calculate bounding box dimensions
    double xmax = 0, ymax = 0;
    for (auto& b : fp.blocks) {
        xmax = max(xmax, b.x + b.width());
        ymax = max(ymax, b.y + b.height());
    }
    double chipArea = xmax * ymax;

    // 2. Calculate Aspect Ratio (AR)
    double ar1 = xmax / ymax;
    double ar2 = ymax / xmax;
    double AR = max(ar1, ar2);

    // 3. Piecewise function F(AR) for aspect ratio penalty
    double fAR = 0;
    if (AR < 0.5)           fAR = 2 * (0.5 - AR);     // Penalty for too narrow
    else if (AR <= 2)       fAR = 0;                  // Acceptable range
    else /* AR > 2 */       fAR = AR - 2;             // Penalty for too wide

    // 4. Area-based cost with aspect ratio penalty
    double costAreaAR = chipArea * (1.0 + fAR);

    // 5. Calculate INL (Integral Non-Linearity)
    double inl = fp.INL();

    // 6. Final weighted cost combining area and INL
    return A * costAreaAR + (1 - A) * inl;
}

// Metropolis criterion for accepting worse solutions
bool SimulatedAnnealing::accept(double new_cost) const {
    double prob = exp((best_cost - new_cost) / T);
    return ((double)rand() / RAND_MAX) < prob;
}