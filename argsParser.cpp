#include "argsParser.h"

int ArgsParser::parse(int argc, char **argv) {
    string x;
    char **args = argv + 1;
    try {
        while (*args) {
            x = *args;
            if (x == "-o") {
                // Write results to the specified file
                writeOutputLog = true;
            } else if (x == "-t") {
                // Write the execution time to a log
                writeTimeLog = true;
            } else if (x == "-k") {
                // Specify number of clusters to compute
                istringstream(*++args) >> numClusters;
            } else if (x == "-ksearch") {
                // Specify behaviour of algorithm execution as a search process over k
                searchClusters = true;
                istringstream(*++args) >> searchParallelThreads;
            } else if (x == "-d") {
                // Specify number portion of data to use
                istringstream(*++args) >> dataUsage;
                if(dataUsage < 0 || dataUsage > 4){
                    cout << "ERROR: Unexpected command line value: dataUsage parameter not in range [0-4]" << endl;
                    throw std::invalid_argument("");
                }
            } else if (x == "-i") {
                // Specify max number of iterations to perform
                istringstream(*++args) >> maxIterations;
            } else if (x == "-s") {
                // Specify the visualization of the results
                displayClusters = true;
            } else if (x == "-h") {
                printHelp(argv[0]);
                return 1;
            } else {
                cout << "ERROR: Unexpected command line argument: " << x << endl;
                printHelp(argv[0]);
                return 1;
            }
            ++args;
        }
    } catch (...) {
        cout << "Exception caught in ArgParser::parseArgs." << endl;
        printHelp(argv[0]);
        return 1;
    }
    return 0;
}

void printHelp(char *arg0) {
    cout << endl << "Performs single-pass clustering for the specified image file." << endl
              << "USAGE:" << endl
              << "    " << arg0 << " "
              << "[-k numClusters] [-ksearch] [-i maxIterations] [-d dataUsage] [-s] [-o] [-v] [-t]"
              << endl
              << "ARGUMENTS:" << endl
              << "    -k\tNumber of clusters to compute (default=10)." << endl
              << "    -ksearch\tExecute the algorithm over the number of clusters in range [2, k]." << endl
              << "    -i\tMax number of iterations to perform (default=10)." << endl
              << "    -d\tData usage fraction 0=min, 4=max (default=1)." << endl
              << "    -s\tVisualize the results of algorithm execution." << endl
              << "    -o\tWrite resulting clusters to log file." << endl
              << "    -t\tWrite execution time to log file." << endl
              << endl << flush;
}
