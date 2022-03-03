#include <iostream>
#include <algorithm>
#include "argsParser.h"
#include "dataManager.h"
#include "kmeans.h"
#ifdef SDL_VERSION
#include "GUIRenderer.h"
#define USE_SDL true
#endif
#if defined(_OPENMP)
#include <omp.h>
#define USE_OMP true
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
// script for cluster execution

int main(int argc, char *argv[]) {
#ifdef USE_OMP
    printf("OpenMP enabled - Parallel execution with %d threads\n", omp_get_max_threads()); fflush(stdout);
#endif

    // Cmd arguments parsing
    ArgsParser parser;
    if(parser.parse(argc, argv))
        return 1;

    // Features pre setup
    if(parser.writeTimeLog) std::ofstream f("time.log");  // new empty file
    if(parser.writeOutputLog) std::ofstream f("output.log");  // new empty file
#ifndef USE_SDL
    if(parser.displayClusters){
        parser.displayClusters = false;
        printf("SDL library not found - Show features disabled\n"); fflush(stdout);
    }
#endif
    if(parser.displayClusters and (parser.searchParallelThreads!=1)){
        parser.displayClusters = false;
        printf("Multithreads ksearch doesn't support visualization - Show features disabled\n"); fflush(stdout);
    }
    bool displayClusters = parser.displayClusters;

    // Load data
#ifdef USE_OMP
    double initial_start_time = omp_get_wtime(), start_time = omp_get_wtime();
    double time_per_cluster_iter = 0;
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

    // Variables to keep track of best k value search, initialized at first position
    int minK = parser.searchClusters? 2:parser.numClusters;
    int bestK = minK;
    long bestKVar = std::numeric_limits<long>::max();

    // Make K-search
#ifdef USE_OMP
    omp_set_nested(true);  // let every thread of search (the executions on different k values) start their pool
    #pragma omp parallel for default(shared) num_threads(parser.searchParallelThreads)
#endif
    for (int k=minK; k < parser.numClusters+1; k++) {

    #ifdef USE_OMP
        if(parser.searchClusters)  // set the size of the thread pool available for KMeans algorithm execution
            omp_set_num_threads(parser.kmeansParallelThreads);
    #endif

        int numClusters = k;
        int i = 0;

    #ifdef USE_SDL  // Initialize SDL
        GUIRenderer* gui;
        if(displayClusters) {
            gui = new GUIRenderer(SCREEN_WIDTH, SCREEN_HEIGHT);
            displayClusters = gui->guiInitialized;
        }
    #endif

    #ifdef USE_SDL  // Show image
        if(displayClusters)
            dataMgr.showData(gui, data);
    #endif

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

        printf("(k=%d) Starting initialization..\n", numClusters);
        for (i = 0; i < numClusters; i++) {
            // HWC save format
            copyCentroidAddress(data + ((i * numCols / numClusters) * numRows + (i * numRows / numClusters)) * numBands, centroids[i], numBands);
        }

        printf("(k=%d) Starting iterate:\n", numClusters);

    #ifdef USE_OMP
        start_time = omp_get_wtime();
    #endif

        // First iteration
        assignObjects(data, pixelsMap, numPixels, centroids, numClusters, numBands);
        printf("(k=%d) Iteration 1... Initial cluster map calculated.\n", numClusters); fflush(stdout);
        kMeansIterations = 1;

        // Update cluster map until max number of iterations has been reached or
        // fewer than a threshold number of pixels are reassigned between iterations.

        for (; kMeansIterations < parser.maxIterations; kMeansIterations++) {
    #ifdef USE_SDL
            if(displayClusters)
                dataMgr.showClustersOverlay(gui, pixelsMap, numClusters);
    #endif
            // New iteration
            computeCentroids(data, pixelsMap, numPixels, centroids, numClusters, numBands, clustersSize);
            numChanged = assignObjects(data, pixelsMap, numPixels, centroids, numClusters, numBands);
            cout << "(k=" << numClusters << ") Iteration " << kMeansIterations + 1 << "... " << numChanged << " pixels reassigned." << endl << flush;
        }
        /*-------------------------------------------------------------------------------------------*/

        // Write output results
    #ifdef USE_OMP
        double end_time = omp_get_wtime();
        printf("(k=%d) Number of iterations: %d, total time: %f seconds, time per iteration: %f seconds\n",
               numClusters, kMeansIterations, (end_time - start_time), (end_time - start_time)/kMeansIterations);
        fflush(stdout);
        time_per_cluster_iter += ((end_time - start_time)/(kMeansIterations*numClusters));
        if(parser.writeTimeLog) {
            std::cout << "(k=" << numClusters << ") Writing execution time to file \"time.log\"." << std::endl;
            #pragma omp critical
            {
                std::ofstream fout; fout.open("time.log", ios::app);
                fout << "(k=" << numClusters << ") Number of iterations: " << kMeansIterations << ", total time: " << (end_time - start_time)
                     << " seconds, time per iteration: " <<(end_time - start_time)/kMeansIterations<< " seconds\n" << std::endl;
                fout.close();
            }
        }
    #else
        printf("End of iterations\n"); fflush(stdout);
    #endif
        float inVariance = computeClusterVariance(data, pixelsMap, numPixels, centroids, numBands);
        printf("(k=%d) Within-class variance: %f\n\n", numClusters, inVariance); fflush(stdout);

    #ifdef USE_OMP
    #pragma omp critical
    #endif
        if(inVariance < bestKVar) {
            bestKVar = inVariance;
            bestK = numClusters;
        }

        if(parser.writeOutputLog) {
            std::cout << "(k=" << numClusters << ") Writing output file \"output.log\"." << std::endl;
    #ifdef USE_OMP
    #pragma omp critical
    {
    #endif
            std::ofstream fout; fout.open("output.log", ios::app);
            fout << "k=" << numClusters << std::endl << "rows=" << numRows << std::endl << "columns=" << numCols << std::endl;
            for (i = 0; i < numRows * numCols; i++)
                fout << pixelsMap[i] << std::endl;
            fout.close();
    #ifdef USE_OMP
    }
    #endif
        }

    #ifdef USE_SDL  // Setup events loop
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
    #endif

    }  // End search

    // Log output
    if(parser.searchClusters) {
        std::ofstream fout;
        printf("Best k value found is %d with within-class variance %d\n", bestK, bestKVar);
        if(parser.writeTimeLog) {
            fout.open("time.log", ios::app);
            fout << "Best k value found is " << bestK << " with within-class variance " << bestKVar << std::endl;
        }
#ifdef USE_OMP
        float totTime = (omp_get_wtime() - initial_start_time);
        float kiterTime = time_per_cluster_iter/(parser.numClusters+1 - minK);
        printf("Search executed in %f seconds, time per cluster iteration: %f\n", totTime, kiterTime);
        if(parser.writeTimeLog)
            fout << "Search executed in " << totTime << " seconds, time per cluster iteration: " << kiterTime << std::endl;
#endif
        if(parser.writeTimeLog) fout.close();
        fflush(stdout);
    }

    return 0;
}