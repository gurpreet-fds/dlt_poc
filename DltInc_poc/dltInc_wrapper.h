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
