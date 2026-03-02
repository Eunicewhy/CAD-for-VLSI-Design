#ifndef CONTOUR_H
#define CONTOUR_H

#include <vector>

// Contour class for maintaining skyline/contour information during packing
class Contour {
private:
    // Structure to represent a segment of the contour
    struct Segment {
        double start_x, end_x, height;      // Start x, end x, and height of segment
        Segment(double s, double e, double h) : start_x(s), end_x(e), height(h) {}
    };
    
    std::vector<Segment> segments;          // List of contour segments

public:
    // Constructor initializes with a base segment covering infinite width at height 0
    Contour();

    // Get maximum height in the range [x_start, x_start + width]
    double getMaxHeight(double x_start, double width);

    // Update contour by placing a rectangle at (x_start, width) with new_height
    void update(double x_start, double width, double new_height);

    // Get maximum x coordinate where height > 0
    double maxX() const;

    // Get maximum height across all segments
    double maxY() const;
};

#endif // CONTOUR_H