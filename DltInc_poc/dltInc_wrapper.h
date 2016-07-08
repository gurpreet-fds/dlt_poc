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

class DltInc_wrapper {
public:
    const static int REPLICA_FACTOR = 3;
    const int TOTAL_TOKENS = 256;
    const int NUM_NODES_TO_STARTWITH = 3;
    const int WEIGHT_GAP = 1;
    const int BOOT_TIME_WEIGHT = 2;
    const int WEIGHT_FACTOR = 0;

    DltInc::nwKv createTempKv(int numNodes, int idSeed, int startingWeight, int weightGap);
    void addNode(int numberOfNodesToAdd, int weight);
    void removeNode(vector<int>& nodesToRemove);

private:
    std::unique_ptr<DltInc> finalDlt;
    
};

#endif /* dltInc_wrapper_h */
