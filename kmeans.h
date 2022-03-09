#ifndef KMEANS_H
#define KMEANS_H

#include <stdlib.h>
#include "common.h"

#if defined(_OPENMP)
#define USE_OMP true
#endif

using namespace std;

// Computes the cluster centers by averaging all objects assigned to each cluster.
// ARGUMENTS:
//   data		An array with numObjects * dataDepth elements
//   objMapping		numObjects long array whose elements will associate each
//			object with a specific cluster.
//   numObjects		Number of objects  (each of which has dataDepth elements).
//   centroids	    A numClusters long array of pointers to dataDepth-length
//			arrays in which the cluster centers will be returned.
//   numClusters	Length of centroids.  Also, max value in objMapping is
//			numClusters - 1.
//   dataDepth		Depth of each object and also vectors contained in centroids.
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
        #pragma omp barrier
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

// Calculates clusterMap, which associates each object with one of the elements of centroids.
// ARGUMENTS:
//   data		An array with numObjects * dataDepth elements
//   objMapping		numObjects long array whose elements will associate each
//			object with a specific cluster.
//   numObjects		Number of objects  (each of which has dataDepth elements).
//   centroids		A numClusters long array of pointers to dataDepth-length
//			arrays that define the cluster locations
//   numClusters	Length of centroids.  Also, max value in objMapping is
//			numClusters - 1.
//   dataDepth		Depth of each object and also vectors contained in centroids.
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
            numChanged += 1;
        objMapping[i] = nearestCluster;
    }
    return numChanged;
}

// Calculates the total within-cluster variance for as a sum of all clusters. The distance from pixel
// to centroid is normalized by the number of bands.
// ARGUMENTS:
//   data		An array with numObjects * dataDepth elements
//   objMapping		numObjects long array whose elements will associate each
//			object with a specific cluster.
//   numObjects		Number of objects  (each of which has dataDepth elements).
//   centroids		A numClusters long array of pointers to dataDepth-length
//			arrays that define the cluster locations
//   dataDepth		Depth of each object and also vectors contained in centers.
template<typename T>
double computeClusterVariance(const T *data, int *objMapping, long numObjects, float **centroids, int dataDepth) {
    long i;
    double clustersVariance = 0;
    const T *pixel;

#ifdef USE_OMP
#pragma omp parallel for default(shared) private(pixel) reduction(+:clustersVariance)
#endif
    for (i = 0; i < (int) numObjects; i++) {
        pixel = data + (i * dataDepth);
        clustersVariance += distance(pixel, centroids[objMapping[i]], dataDepth)/dataDepth;
    }
    return clustersVariance;
}

#endif // KMEANS_H

