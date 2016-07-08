//
//  main.cpp
//  DltInc_poc
//
//  Created by Gurpreet Singh on 6/17/16.
//  Copyright © 2016 Gurpreet Singh. All rights reserved.
//

#include "dltInc.h"


#define REPLICA_FACTOR 3
#define TOTAL_TOKENS 256
#define NUM_NODES_TO_ADD 2
#define NUM_NODES_TO_STARTWITH 3
#define WEIGHT_GAP 1
#define BOOT_TIME_WEIGHT 2
#define ADD_NODE_WITH_WEIGHT BOOT_TIME_WEIGHT
#define WEIGHT_FACTOR 0

DltInc::nwKv createTempKv(int numNodes, int idSeed, int startingWeight, int weightGap) {
    DltInc::nwKv nodeWeightMap;
    
    for (int i = 0; i < numNodes; i++) {
        nodeWeightMap[idSeed++] = DltInc::wInfo(startingWeight + weightGap*(i+1), 0);
    }
    return nodeWeightMap;
}


void testRun(int numberOfRuns, int numberOfNodesToAdd, int weight) {
    for (int i = REPLICA_FACTOR; i <= numberOfRuns; i++) {
        DltInc dlt(TOTAL_TOKENS, 3);

        auto kv = createTempKv(i, 1, BOOT_TIME_WEIGHT, 0);
        dlt.init(kv);
        
        dlt.addNodesInBulk(numberOfNodesToAdd, weight, WEIGHT_FACTOR);
    }
}

int main(int argc, const char * argv[]) {

    testRun(NUM_NODES_TO_STARTWITH, NUM_NODES_TO_ADD, ADD_NODE_WITH_WEIGHT);

    return 0;
}