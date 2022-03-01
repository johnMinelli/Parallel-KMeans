#ifndef KMEANS_H
#define KMEANS_H

#include <stdlib.h>
#include "common.h"


// Computes the cluster centers by averaging all pixels assigned to each cluster.
// ARGUMENTS:
//   data		    An array with numPixels * numBands elements
//   objMapping		numPixels long array whose elements associate each element
//			in image with a specific cluster.
//   numObjects		Number of pixels in image (each of which has numBands elements).
//   centroids	    A numClusters long array of pointers to numBands-length
//			arrays in which the cluster centers will be returned.
//   numClusters	Length of centers.  Also, max value in clusterMap is
//			numClusters - 1.
//   dataDepth		Length of pixels in image and vectors contained in centers.
//   clustersSize	An array to receive the number of pixels assigned to each cluster
#ifdef USE_OMP
template<typename T>
int computeCentroids(const T *data, const int *objMapping, long numObjects,
                   float **centroids, int numClusters,
                   int dataDepth, long *clustersSize) {
    long i;
    int j;

    #pragma omp parallel sections default(shared)
    {
        #pragma omp section
        std::fill(clustersSize, clustersSize + numClusters, 0);
        #pragma omp section
        for (i = 0; i < numClusters; i++) {
            std::fill((centroids[i]), (centroids[i]) + dataDepth, 0.f);
        }
    }
    #pragma omp parallel default(shared)
    {
        #pragma omp for
        for (i = 0; i < numObjects; i++) {
            arrayAdd(data + i * dataDepth, centroids[objMapping[i]], dataDepth);
            #pragma omp atomic
            clustersSize[objMapping[i]] += 1;
        }
        #pragma omp for collapse(2)
        for (i = 0; i < (int) numClusters; i++) {
            for (j = 0; j < dataDepth; j++)
                centroids[i][j] /= clustersSize[i];
        }
    }
    return 0;
}
#else
template<typename T>
int computeCentroids(const T *data, const int *objMapping, long numObjects,
                   float **centroids, int numClusters,
                   int dataDepth, long *clustersSize) {
    long i;
    int j;
    std::fill(clustersSize, clustersSize + numClusters, 0);
    for (i = 0; i < numClusters; i++) {
        std::fill((centroids[i]), (centroids[i]) + dataDepth, 0.f);
    }
    for (i = 0; i < numObjects; i++) {
        arrayAdd(data + i * dataDepth, centroids[objMapping[i]], dataDepth);
        clustersSize[objMapping[i]] += 1;
    }
    for (i = 0; i < (int) numClusters; i++) {
        for (j = 0; j < dataDepth; j++)
            centroids[i][j] /= clustersSize[i];
    } return 0;
}
#endif

// Calculates clusterMap, which associates each element of image with one of the
// elements of centers.
// ARGUMENTS:
//   data		An array with numPixels * numBands elements
//   objMapping		numPixels long array whose elements will associate each
//			element in image with a specific cluster.
//   numPixels		Number of pixels in image (each of which has numBands elements).
//   centers		A numClusters long array of pointers to numBands-length
//			arrays that define the cluster locations
//   numClusters	Length of centers.  Also, max value in clusterMap is
//			numClusters - 1.
//   dataDepth		Length of pixels in image and vectors contained in centers.
template<typename T>
long assignObjects(const T *data, int *objMapping, long numObjects, float **centroids,
                  int numClusters, int dataDepth) {
    long i, numChanged = 0;
    int j;
    double dist, minDist;
    int nearestCluster;
    const T *pixel;

#ifdef USE_OMP
#pragma omp parallel default(shared) private(pixel, nearestCluster, minDist, j, dist)
#pragma omp for reduction(+:numChanged)
#endif
    for (i = 0; i < (int) numObjects; i++) {
        pixel = data + i * dataDepth;

        // Determine the cluster nearest to this pixel
        nearestCluster = 0;
        minDist = distance(pixel, centroids[0], dataDepth);
        for (j = 1; j < numClusters; j++) {
            dist = distance(pixel, centroids[j], dataDepth);
            if (dist < minDist) {
                minDist = dist;
                nearestCluster = j;
            }
        }
        if (objMapping[i] != nearestCluster)
            ++numChanged;
        objMapping[i] = nearestCluster;
    }
    return numChanged;
}

template<typename T>
long computeClusterVariance(const T *data, int *objMapping, long numObjects, float **centroids, int dataDepth) {
    long i, clustersVariance = 0;
    const T *pixel;

#ifdef USE_OMP
#pragma omp parallel for default(shared) private(pixel) reduction(+:clustersVariance)
#endif
    for (i = 0; i < (int) numObjects; i++) {
        pixel = data + i * dataDepth;
        clustersVariance = distance(pixel, centroids[objMapping[i]], dataDepth);
    }
    return clustersVariance;
}

#endif // KMEANS_H

