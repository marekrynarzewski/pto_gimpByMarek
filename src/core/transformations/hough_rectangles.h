#ifndef HOUGH_RECTANGLES_H
#define HOUGH_RECTANGLES_H

#include "transformation.h"

class HoughRectangles : public Transformation
{
public:
    explicit HoughRectangles(PNM*);
    explicit HoughRectangles(PNM*, ImageViewer*);

    virtual PNM* transform();
private:
    PNM* detect_corners(PNM* img);
};

#endif // HOUGH_RECTANGLES_H
