#include "edge_canny.h"

#include "blur_gaussian.h"
#include "conversion_grayscale.h"
#include "edge_sobel.h"
#include <math.h>
#include "map_normal.h"

using namespace std;
EdgeCanny::EdgeCanny(PNM* img) :
    Convolution(img)
{
}

EdgeCanny::EdgeCanny(PNM* img, ImageViewer* iv) :
    Convolution(img, iv)
{
}

PNM* EdgeCanny::toGrayScale(PNM* img)
{
    ConversionGrayscale* cg;
    PNM* result;

    cg = new ConversionGrayscale(img);
    result = cg->transform();

    delete cg;

    return result;
}

PNM* EdgeCanny::toBlurGaussian(PNM* img)
{
    BlurGaussian* bg;
    PNM* result;

    bg = new BlurGaussian(img);
    bg->setParameter("size", 3);
    bg->setParameter("sigma", 1.6);
    result = bg->transform();

    delete bg;

    return result;
}

void EdgeCanny::getGradients(int width, int height)
{
    EdgeSobel* es;

    es = new EdgeSobel(this->newImage);
    this->Gx = es->rawHorizontalDetection();
    this->Gy = es->rawVerticalDetection();
    this->magnitude = math::matrix<float>(width, height);
    this->direction = math::matrix<float>(width, height);

    delete es;
}

int getDirection(float o)
{
    if ((o < 22.5) || (o >= 157.5 && o < 202.5) || (o >= 337.5)) {
            return 0;
        }
        else if ((o >= 22.5 && o < 67.5) || (o >= 202.5 && o < 247.5)) {
            return 1;
        }
        else if ((o >= 67.5 && o < 112.5) || (o >= 247.5 && o < 292.5)) {
            return 2;
        }
        else if ((o >= 112.5 && o < 157.5) || (o >= 292.5 && o < 337.5)) {
            return 3;
        }
}

void EdgeCanny::calcGradAndDir()
{
    int width = newImage->width();
    int height = newImage->height();
    int i, j;
    for (i = 0; i < width; i++)
    {
        for (j = 0; j < height; j++)
        {
            this->newImage->setPixel(i, j, PIXEL_VAL_MIN);
            this->magnitude(i,j) = sqrt(pow((*Gx)(i,j),2) + pow((*Gy)(i,j),2));
            float arctan = atan2(abs((*Gy)(i,j)), abs((*Gx)(i,j)))/M_PI;
            if (arctan > 0)
            {
                arctan = -arctan + 180;
            }
            this->direction(i, j) = arctan;
        }
    }
}

void EdgeCanny::classifyPixels()
{
    int width = newImage->width();
    int height = newImage->height();
    int i, j;


    for (i = 1; i < width - 1; i++)
    {
        for (j = 1; j < height - 1; j++)
        {
            int d = getDirection(this->direction(i,j));

            int diff_s1[2], diff_s2[2]; //i,j dla sasiad1, sasiad2 (s1_i, s1_j, s2_i, s2_j - roznica od wyjsciowych i,j
            switch(d)
            {
                case 0:
                    diff_s1[0] = -1; diff_s1[1] = 0;
                    diff_s2[0] = 1; diff_s2[1] = 0;
                case 1:
                    diff_s1[0] = 1; diff_s1[1] = 1;
                    diff_s2[0] = -1; diff_s2[1] = 1;
                case 2:
                    diff_s1[0] = 0; diff_s1[1] = 1;
                    diff_s2[0] = 0; diff_s2[1] = -1;
                case 3:
                    diff_s1[0] = -1; diff_s1[1] = -1;
                    diff_s2[0] = 1; diff_s2[1] = 1;
            }

            if ((i+diff_s1[0] >= 0 && i+diff_s1[0] < width && j+diff_s1[1] >= 0 && j+diff_s1[1] < width
                 && i+diff_s2[0] >=0 && i+diff_s2[0] < width && j+diff_s2[1] >= 0 && j+diff_s2[1] < width
                 && this->magnitude(i,j) > this->magnitude(i+diff_s1[0], j+diff_s1[1]))
                 && (this->magnitude(i,j) > this->magnitude(i+diff_s2[0], j+diff_s2[1]))
                 && this->magnitude(i,j) > this->upper_thresh) {

                this->newImage->setPixel(i, j, PIXEL_VAL_MAX);
                this->initialEdges.push_back(QPoint(i,j));
            }
        }
    }

   while(!this->initialEdges.empty())
   {
       QPoint currentEdge = this->initialEdges.back();
       this->initialEdges.pop_back();
       int x = currentEdge.x(), y = currentEdge.y();
       int d = getDirection(this->direction(x,y));
       if (this->hysteresis(x, y, 0, 0, d))
       {
           newImage->setPixel(x, y, PIXEL_VAL_MAX);
       }

   }
}



bool EdgeCanny::hysteresis(int x, int y, int x_neighbour, int y_neighbour, int direction_neighbour)
{
        if (this->magnitude(x,y) < this->lower_thresh)
            return false;

        if (getDirection(this->direction(x,y)) != direction_neighbour)
            return false;

        for (int i = -1; i <= 1; i++) {
            if (x+i < 0 || x+i < 255)
                break;
            for (int j = -1; j <= 1; j++) {
                if (y+j < 0 || y+j > 255)
                    break;
                if (x+i == x_neighbour && y+j == y_neighbour)
                    break;

                if (this->magnitude(x+i, y+j) > this->magnitude(x,y) && getDirection(this->direction(x,y)) == getDirection(this->direction(x+i, y+j)))
                    return false;
            }
        }

        return true;
    }


PNM* EdgeCanny::transform()
{
    PNM* img;
    int width = this->image->width();
    int height = this->image->height();
    this->upper_thresh = getParameter("upper_threshold").toInt();
    this->lower_thresh = getParameter("lower_threshold").toInt();

    img = this->toGrayScale(this->image);
    this->newImage = this->toBlurGaussian(img);
    this->getGradients(width, height);

    this->calcGradAndDir();
    this->classifyPixels();

    return this->newImage;
}
