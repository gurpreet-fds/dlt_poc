//
//  dltInc_wrapper.h
//  DltInc_poc
//
//  Created by Gurpreet Singh on 7/8/16.
//  Copyright © 2016 Gurpreet Singh. All rights reserved.
//

#ifndef dltInc_wrapper_h
#define dltInc_wrapper_h

#include "dltInc.h"
#include <memory>

// Wrapper class for the DLT-inc algorithm class. Any add/remove node
// operation should be done through this class.

/**
 * There is a strong correlation between the total number of tokens and
 * total number of nodes a cluster should have.
 *
 * Taking the standard FDS configuration as benchmark where there are
 * 10 disks in a node, for parallelism each disk should hold atleast
 * 1 primary token, so for any given token distribution, there
 * should be atleast 10 primary tokens assigned to a SM.
 * If the number of primary tokens assigned to a SM in cluster falls
 * below 10, that is an indication that we need more granular tokens/
 * more number of tokens for a given deployment.
 *
 * The optimum configurations:
 * Num of Tokens ---> maximum number of SMs a cluster should have
 *  256                 ~24 nodes
 *  512                 ~48 nodes
 * and so on.
 */
 
class DltInc_wrapper {
public:

    const static int REPLICA_FACTOR = 3;
    const int TOTAL_TOKENS = 256;
    
    // should be same as replica factor
    const int NUM_NODES_TO_STARTWITH = 3;

    // in case of heterogenous nodes.
    const int WEIGHT_GAP = 1;

    // weights of nodes which makes up the initial cluster.
    const int BOOT_TIME_WEIGHT = 2;

    // weight variation factor between nodes to be added.
    const int WEIGHT_FACTOR = 0;

    // Add number of nodes to the cluster
    void addNode(int numberOfNodesToAdd, int weight);
    
    // Remove node(s) from the cluster.
    // input arg vector holds node id of nodes to be removed.
    void removeNode(vector<int>& nodesToRemove);

private:

    // Creates a temporary key-value map for given number of nodes
    DltInc::nwKv createTempKv(int numNodes, int idSeed, int startingWeight, int weightGap);

    // holds the final dlt after all node additions/removals.
    std::unique_ptr<DltInc> finalDlt;
    
};

#endif /* dltInc_wrapper_h */
