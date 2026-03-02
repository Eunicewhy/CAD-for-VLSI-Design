#include "../inc/Contour.h"
#include <algorithm>
#include <cmath>

// Initialize contour with a single segment covering infinite width at height 0
Contour::Contour() { 
    segments.emplace_back(0, 1e9, 0); 
}

// Find the maximum height in the specified range [x_start, x_start + width]
double Contour::getMaxHeight(double x_start, double width) {
    double x_end = x_start + width;
    double max_h = 0;
    
    // Check all segments that overlap with the query range
    for (auto& seg : segments) {
        // Skip segments that don't overlap with query range
        if (seg.end_x <= x_start || seg.start_x >= x_end) continue;
        max_h = std::max(max_h, seg.height);
    }
    return max_h;
}

// Update contour by placing a rectangle and adjusting affected segments
void Contour::update(double x_start, double width, double new_height) {
    double x_end = x_start + width;
    
    std::vector<Segment> new_segments;
    
    // Process existing segments: split or keep based on overlap with new rectangle
    for (auto& seg : segments) {
        // If segment is completely outside the update range, keep it unchanged
        if(seg.end_x <= x_start || seg.start_x >= x_end) {
            new_segments.push_back(seg);
        } 
        else{
            // Segment overlaps with update range, may need to split
            // Keep left portion if it exists
            if(seg.start_x < x_start)
                new_segments.emplace_back(seg.start_x, x_start, seg.height);
            // Keep right portion if it exists
            if(seg.end_x > x_end)
                new_segments.emplace_back(x_end, seg.end_x, seg.height);
        }
    }
    
    // Insert new segment with updated height
    new_segments.emplace_back(x_start, x_end, new_height);
    
    // Sort segments by start position
    std::sort(new_segments.begin(), new_segments.end(),
         [](const Segment& a, const Segment& b) { return a.start_x < b.start_x; });

    // Merge adjacent segments with same height
    std::vector<Segment> merged;
    for (auto& seg : new_segments) {
        // Check if current segment can be merged with the last merged segment
        if (!merged.empty() && std::fabs(merged.back().height - seg.height) < 1e-8 &&
            std::fabs(merged.back().end_x - seg.start_x) < 1e-8) {
            // Merge by extending the end of the last segment
            merged.back().end_x = seg.end_x;
        } else {
            // Cannot merge, add as new segment
            merged.push_back(seg);
        }
    }
    segments = merged;
}

// Find the maximum x coordinate where height > 0
double Contour::maxX() const {
    double mx = 0;
    for (auto& seg : segments) {
        if (seg.height > 0) mx = std::max(mx, seg.end_x);
    }
    return mx;
}

// Find the maximum height across all segments
double Contour::maxY() const {
    double my = 0;
    for (auto& seg : segments) {
        my = std::max(my, seg.height);
    }
    return my;
}