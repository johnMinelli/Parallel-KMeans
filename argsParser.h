#include <iostream>
#include <sstream>

#ifndef ARGPARSER_H
#define ARGPARSER_H

using namespace std;

struct ArgsParser {
    ArgsParser() : numClusters(10), maxIterations(10), dataUsage(1), searchParallelThreads(1), kmeansParallelThreads(0), searchClusters(false), displayClusters(false), writeOutputLog(false), writeTimeLog(false) {};

    int parse(int argc, char **argv);

    int numClusters, maxIterations, dataUsage, searchParallelThreads, kmeansParallelThreads;
    bool searchClusters;
    bool displayClusters;
    bool writeOutputLog;
    bool writeTimeLog;
};

void printHelp(char *arg0);

#endif // ARGPARSER_H