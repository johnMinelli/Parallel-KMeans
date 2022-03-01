#include "dataManager.h"
#include "common.h"


template<typename T>
int readFile(const char *fileName, long long numVals, T *array, long long offset = 0L);

float *DataManager::loadData() {
    float *image = new float[samples*lines*bands], *processedImage = new float[samples*lines*bands];
    int ret = readFile<float>(fileName, samples*lines*bands, image);

    if (ret>0) {
        cout << "readFile returned " << ret << "." << endl;
        return nullptr;
    }

    int i = 0, j = 0, k = 0;
    int offset_HW = samples * lines;  // (BSQ format)
    int offset_CW = bands * samples;  // (BIL format)
    float maxR = std::numeric_limits<float>::min(), maxG = std::numeric_limits<float>::min(), maxB = std::numeric_limits<float>::min();

    // pre-process into HWC standard format and acquire max value in RGB bands for normalization
#ifdef USE_OMP
    #pragma omp parallel for collapse(2) default(shared) private(k) reduction(max:maxR, maxG, maxB)
#endif
        for (i = 0; i < lines; i++) {
            for (j = 0; j < samples; j++) {
                for (k = 0; k < bands; k++)
                    if (image[(i * offset_CW) + (k * samples) + j] == -9999)
                        processedImage[(i * offset_CW) + (j * bands) + k] = 0;
                    else processedImage[(i * offset_CW) + (j * bands) + k] = image[(i * offset_CW) + (k * samples) + j];

                // get max in red channel
                maxR = image[(i * offset_CW) + (R * samples) + j];
                // get max in green channel
                maxG = image[(i * offset_CW) + (G * samples) + j];
                // get max in blue channel
                maxB = image[(i * offset_CW) + (B * samples) + j];
            }
        }

    this->rescaleFactorR = (1/maxR * 255);
    this->rescaleFactorG = (1/maxG * 255);
    this->rescaleFactorB = (1/maxB * 255);

    return processedImage;
}

void DataManager::showData(GUIRenderer *gui, float *data) {
    Uint8 *rgb = new Uint8 [samples*lines*3];
    int i = 0, j = 0;
    int offset = samples * bands;

    for (i = 0; i < lines; i++) {
        for (j = 0; j < samples; j++) {
            rgb[(i*3*samples)+(j*3)+0] = (Uint8) (data[(i*offset) + (j * bands) + R] * rescaleFactorR);  //R
            rgb[(i*3*samples)+(j*3)+1] = (Uint8) (data[(i*offset) + (j * bands) + G] * rescaleFactorG);  //G
            rgb[(i*3*samples)+(j*3)+2] = (Uint8) (data[(i*offset) + (j * bands) + B] * rescaleFactorB);  //B
        }
    }
    imageSurface = SDL_CreateRGBSurfaceWithFormatFrom(rgb, samples, lines, 8, samples*3, SDL_PIXELFORMAT_RGB24);
    gui->renderSurface(imageSurface);
}

void DataManager::showClustersOverlay(GUIRenderer *gui, int* pixelsMap, int numClusters) {
    SDL_Surface* overlay = SDL_DuplicateSurface(imageSurface);
    SDL_PixelFormat *format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32);
    overlay = SDL_ConvertSurface(overlay, format, 0);
    if(colorsArray == nullptr){
        colorsArray  = (SDL_Color *)malloc(numClusters*sizeof(SDL_Color));
        int i;
        for (i = 0; i < numClusters; i++) {
            int* rgb = HSVtoRGB((i*360)/numClusters, 100, 100);
            colorsArray[i].r = rgb[0];
            colorsArray[i].g = rgb[1];
            colorsArray[i].b = rgb[2];
            colorsArray[i].a = 10;
        }
    }
    SDL_FreeFormat(format);

    gui->lockSurface(overlay);
    int i, sample, line;
    for (i = 0; i < samples * lines ; ++i) {
        line = i / samples;
        sample = i % samples;
        SDL_Color c = colorsArray[pixelsMap[i]];
        gui->setPixelSurface(overlay, sample, line, c);
    }
    gui->unlockSurface(overlay);
    gui->renderSurface(overlay);

    if(clustersSurface != nullptr)
        SDL_FreeSurface(clustersSurface);
    clustersSurface = overlay;
}

template<typename T>
int readFile(const char *fileName, long long numVals, T *array, long long offset) {
    ifstream fin;

    fin.open(fileName, std::ios_base::binary);
    if (!fin) {
        std::cout << "Unable to open " << fileName << " in readFile." << std::endl;
        return 1;
    }

    long long val = numVals * (long long)sizeof(T);
    long long file_offset = offset * (long long)sizeof(T);
    long long array_address = 0;
    fin.seekg(file_offset, std::ios_base::beg);
    if(val>LONG_MAX){
        while(val>0){
            fin.read((char *) array+array_address, LONG_MAX);
            val-=LONG_MAX;
            array_address+=LONG_MAX;
        }
    }else fin.read((char *) array+array_address, val);

    fin.close();
    return 0;
}
