#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <fstream>
#include <valarray>


// Calculate the Euclidean distance between v1 and v2.  Each is assumed to be
// a pointer to an array of 3 doubles.
template<typename S, typename T>
double distance(const S *a, const T *b, unsigned int numVals) {
    double sumSq = 0.0;
    for (unsigned int i = 0; i < numVals; i++)
        sumSq += (a[i] - b[i]) * (a[i] - b[i]);
    return sqrt(sumSq);
}

template<typename S, typename T>
void arrayAdd(const S *src, T *dest, unsigned int numVals) {
    for (unsigned int i = 0; i < numVals; i++)
        dest[i] += src[i];
}

static int* HSVtoRGB(float H, float S, float V) {
    if (H > 360 || H < 0 || S > 100 || S < 0 || V > 100 || V < 0) {
        cout << "The givem HSV values are not in valid range" << endl;
        return nullptr;
    }
    float s = S / 100;
    float v = V / 100;
    float C = s * v;
    float X = C * (1 - abs(fmod(H / 60.0, 2) - 1));
    float m = v - C;
    float r, g, b;
    if (H >= 0 && H < 60) {
        r = C, g = X, b = 0;
    } else if (H >= 60 && H < 120) {
        r = X, g = C, b = 0;
    } else if (H >= 120 && H < 180) {
        r = 0, g = C, b = X;
    } else if (H >= 180 && H < 240) {
        r = 0, g = X, b = C;
    } else if (H >= 240 && H < 300) {
        r = X, g = 0, b = C;
    } else {
        r = C, g = 0, b = X;
    }
    int R = (r + m) * 255;
    int G = (g + m) * 255;
    int B = (b + m) * 255;
    int* value = new int[3]{R, G, B};;
    return value;
}

template<typename S, typename T>
void copyCentroidAddress(const S *src, T *dest, unsigned long numVals) {
    for (unsigned long i = 0; i < numVals; i++)
        dest[i] = src[i];
    return;
}

#endif // COMMON_H
