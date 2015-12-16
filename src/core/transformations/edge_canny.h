#ifndef EDGECANNY_H
#define EDGECANNY_H

#include "convolution.h"

class EdgeCanny : public Convolution
{
public:
    EdgeCanny(PNM*);
    EdgeCanny(PNM*, ImageViewer*);

    virtual PNM* transform();
private:
    PNM* toGrayScale(PNM*);
    PNM* toBlurGaussian(PNM*);
    void getGradients(PNM*);
    QList<QPoint> edgesList;
    math::matrix<float> magnitude;
    math::matrix<float> direction;
    void calcGradAndDir();
    void classifyPixels();
    void hysteresis();
    PNM* newImage;
    math::matrix<float> *v, *h;
};

#endif // EDGECANNY_H
