#include <SDL_render.h>
#include <iostream>
#include <limits>
#include <fstream>
#include <string>
#include "GUIRenderer.h"
#include <cmath>

using namespace std;

#ifndef DATAMANAGER_H
#define DATAMANAGER_H

class DataManager {

private:
    const char *fileName = "../ang20180814t224053_rfl_v2r2/ang20180814t224053_corr_v2r2_img";
    // info from header dataset file; limit the size of the data to half of the lines
    int samples = 637, lines = 4207, bands = 425;
    // Each band is acquired in a specific spectre shade measurable in nanometer.
    // It is also known the spectrum of colors is approximately:
    // "visible-violet": {'lower': 375, 'upper': 450, 'color': 'violet'},
    // "visible-blue": {'lower': 450, 'upper': 485, 'color': 'blue'},
    // "visible-cyan": {'lower': 485, 'upper': 500, 'color': 'cyan'},
    // "visible-green": {'lower': 500, 'upper': 565, 'color': 'green'},
    // "visible-yellow": {'lower': 565, 'upper': 590, 'color': 'yellow'},
    // "visible-orange": {'lower': 590, 'upper': 625, 'color': 'orange'},
    // "visible-red": {'lower': 625, 'upper': 740, 'color': 'red'},
    // "near-infrared": {'lower': 740, 'upper': 1100, 'color': 'gray'},
    // "shortwave-infrared": {'lower': 1100, 'upper': 2500, 'color': 'white'}

    // to reconstruct an RGB image we take the bands acquired in the average of the range
    // R: 667.5nm, G: 540nm, B: 470nm
    // reading the dataset metadata (which describe the match between bands index and nm value) we take the following bands index
    int R = 58-1, G = 33-1, B = 19-1;
    // normalization [0-255] factors for corresponding bands
    float rescaleFactorR = 1, rescaleFactorG = 1, rescaleFactorB = 1;
    SDL_Color* colorsArray = nullptr;
    SDL_Surface* imageSurface = nullptr;
    SDL_Surface* clustersSurface = nullptr;

public:
    DataManager(int dataQt){
        // compute the quantity of data you want to use
        lines = lines/pow(2,4-dataQt);
    }
    int getSamples() const { return samples; }
    int getLines() const { return lines; }
    int getBands() const { return bands; }
    SDL_Surface* getImage() const { return imageSurface; }
    SDL_Surface* getClustersImage() const { return clustersSurface; }

    float *loadData();
    void showData(GUIRenderer *gui, float *data);
    void showClustersOverlay(GUIRenderer *gui, int *pixelsMap, int numClusters);

};

#endif // DATAMANAGER_H