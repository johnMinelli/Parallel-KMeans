#include <iostream>
#include <algorithm>
#include "SDL.h"
#include "argsParser.h"
#include "dataManager.h"
#include "kmeans.h"
#include "GUIRenderer.h"

#if defined(_OPENMP)
#include <omp.h>
#endif

using namespace std;

#define SCREEN_WIDTH    637
#define SCREEN_HEIGHT   1000

/****************************************************************************
 *
 * main.cpp
 *
 * Compile with:
 * gcc -fopenmp main.cpp -o main
 *
 * Run with:
 * OMP_NUM_THREADS=4 ./main
 *
 ****************************************************************************/

//TODO
// add parallel-k-search <nthreads> and k parameter becomes the maximum value of clusters to use [2-k]
// script for cluster execution
// output of values for evaluation from cluster and metric for inside class variance as a clustering result

int main(int argc, char *argv[]) {
#ifdef USE_OMP
    printf("OpenMP enabled - Parallel execution with %d threads\n", omp_get_max_threads()); fflush(stdout);
#endif

    // Cmd arguments parsing
    ArgsParser parser;
    if(parser.parse(argc, argv))
        return 1;
    // Variables to keep track of best k value search, initialized at first position
    int bestK = parser.searchClusters? 2:parser.numClusters;
    long bestKVar = std::numeric_limits<long>::max();
 #pragma omp for collapse(2)
        for (int k=bestK; k < parser.numClusters+1; k++) {
            for (int j=bestK; j < parser.numClusters+1; j++) {
                int s = omp_get_thread_num();
            printf(" %d cycle executed from %d thread out of %d\n", k, s, omp_get_num_threads());
            fflush(stdout);

            }
        }

#pragma omp barrier
    return(0);


    omp_set_nested(true);
    #pragma omp parallel for
    for (int k=bestK; k < parser.numClusters+1; k++) {
        int x = omp_get_thread_num();
        printf(" %d cycle executed from %d thread out of %d\n",k, x, omp_get_num_threads());
        fflush(stdout);

        omp_set_num_threads(2);
        int max = omp_get_num_threads();
        printf(" %d cycle and executed from %d thread out of %d\n",k, x, max);
        fflush(stdout);
        #pragma omp parallel for
        for (int b=0; b < 2; b++) {
            int y1 = omp_get_thread_num();
            int max1 = omp_get_num_threads();
            printf(" %d cycle and %d subcycle executed from %d thread and %d subsubthread out of %d\n",k, b, x, y1, max1);
            fflush(stdout);
        }
    }
#pragma omp barrier
    return(0);
    for (int k=bestK; k < parser.numClusters+1; k++) {

        int numClusters = k;
        bool displayClusters = parser.displayClusters;
        int i = 0;

        // Initialize SDL
        GUIRenderer* gui;
        if(displayClusters) {
            gui = new GUIRenderer(SCREEN_WIDTH, SCREEN_HEIGHT);
            displayClusters = gui->guiInitialized;
        }

        // Load data  TODO
    #ifdef USE_OMP
        double start_time = omp_get_wtime();
    #endif
        DataManager dataMgr = DataManager(parser.dataUsage);
        float *data = dataMgr.loadData();
        if(data == nullptr)
            return 1;
    #ifdef USE_OMP
        printf("Data read in: %f seconds\n", omp_get_wtime() - start_time); fflush(stdout);
    #else
        printf("Data read\n"; fflush(stdout);
    #endif

        // Show image
        if(displayClusters)
            dataMgr.showData(gui, data);

        /*------------------------------------------KMeans-------------------------------------------*/
        // Calculate initial cluster centers: use evenly spaced pixels along the image diagonal to initialize the clusters.

        int numRows = dataMgr.getLines(), numCols = dataMgr.getSamples(), numBands = dataMgr.getBands();
        int numPixels = numRows * numCols;
        // Store an array containing the coordinates of the centroids. Each centroid is an array long `numBands`
        float **centroids = new float*[numClusters];
        for (i = 0; i < numClusters; i++)
            centroids[i] =  new float[numBands];
        // Store an array containing the index of the cluster for each pixel
        int *pixelsMap = new int[numRows * numCols];
        // Store an array containing the number of pixels associated to each cluster
        long *clustersSize = new long[numClusters];
        long numChanged;
        int kMeansIterations;

        printf("Starting initialization..\n");
        for (i = 0; i < numClusters; i++) {
            // tboggs original use WHC
    //        copy(data + ((i * numRows / numClusters) * numCols + (i * numCols / numClusters)) * numBands, centers[i],
    //             numBands);
    //         we will use HWC
            copyCentroidAddress(data + ((i * numCols / numClusters) * numRows + (i * numRows / numClusters)) * numBands, centroids[i], numBands);
        }

        printf("Starting iterate:\n");

    #ifdef USE_OMP
        start_time = omp_get_wtime();
    #endif

        printf("Iteration 1... ");
        assignObjects(data, pixelsMap, numPixels, centroids, numClusters, numBands);
        printf("Initial cluster map calculated.\n"); fflush(stdout);
        kMeansIterations = 1;

        // Update cluster map until max number of iterations has been reached or
        // fewer than a threshold number of pixels are reassigned between iterations.

        for (; kMeansIterations < parser.maxIterations; kMeansIterations++) {
            if(displayClusters)
                dataMgr.showClustersOverlay(gui, pixelsMap, numClusters);
            cout << "Iteration " << kMeansIterations + 1 << "... " << flush;
            computeCentroids(data, pixelsMap, numPixels, centroids, numClusters, numBands, clustersSize);
            numChanged = assignObjects(data, pixelsMap, numPixels, centroids, numClusters, numBands);
            cout << numChanged << " pixels reassigned." << endl << flush;
        }
        /*-------------------------------------------------------------------------------------------*/

        // Write output results
    #ifdef USE_OMP
        double end_time = omp_get_wtime();
        printf("Number of iterations: %d, total time: %f seconds, time per iteration: %f seconds\n",
               kMeansIterations, (end_time - start_time), (end_time - start_time)/kMeansIterations);
        fflush(stdout);
        if (parser.writeTimeLog) {
            std::cout << "Writing execution time to file \"time.log\"." << std::endl;
            std::ofstream fout("time.log");
            fout << "Number of iterations: " << kMeansIterations << ", total time: " << (end_time - start_time)
                 << " seconds, time per iteration: " <<(end_time - start_time)/kMeansIterations<< " seconds\n" << std::endl;
            fout.close();
        }
    #else
        printf("End of iterations\n"); fflush(stdout);
    #endif
        long inVariance = computeClusterVariance(data, pixelsMap, numPixels, centroids, numBands);
        printf("Within-class variance: %d\n", inVariance); fflush(stdout);

        if(inVariance < bestKVar) {
            bestKVar = inVariance;
            bestK = numClusters;
        }

        if (parser.writeOutputLog) {
            std::cout << "Writing output file \"output.log\"." << std::endl;
            std::ofstream fout("output.log");
            fout << numRows << std::endl << numCols << std::endl;
            for (i = 0; i < numRows * numCols; i++)
                fout << pixelsMap[i] << std::endl;
            fout.close();
        }

        // Setup events loop
        if(displayClusters) {
            bool quit = parser.searchClusters? true : false;
            while (!quit) {
                SDL_Event e;
                SDL_WaitEvent(&e);
                if (e.type == SDL_QUIT)
                    quit = true;
                else if (e.type == SDL_MOUSEBUTTONDOWN) {
                    gui->renderSurface(dataMgr.getImage());
                } else if (e.type == SDL_MOUSEBUTTONUP) {
                    gui->renderSurface(dataMgr.getClustersImage());
                }
            }
            gui->quit();
        }

    }

    if(parser.searchClusters)
        printf("Best k value found is %d with within-class variance %d\n", bestK, bestKVar); fflush(stdout);

    return 0;
}