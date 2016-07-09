//
//  dltInc_wrapper.cpp
//  DltInc_poc
//
//  Created by Gurpreet Singh on 7/8/16.
//  Copyright © 2016 Gurpreet Singh. All rights reserved.
//

#include "dltInc_wrapper.h"



void DltInc_wrapper::addNode(int numberOfNodesToAdd, int weight) {
    DltInc* dlt = new DltInc(TOTAL_TOKENS, REPLICA_FACTOR);
    
    auto kv = createTempKv(REPLICA_FACTOR, 1, BOOT_TIME_WEIGHT, 0);
    dlt->init(kv);
    dlt->addNodesInBulk(numberOfNodesToAdd, weight, WEIGHT_FACTOR);
    finalDlt.reset(dlt);
}

void DltInc_wrapper::removeNode(vector<int>& nodesToRemoveVec) {
    
    cout << "Cluster config before remove node\n";
    finalDlt->printTokensOwnedAllLevels();
    
    auto tempKv = finalDlt->getKvMap();
    for (auto& nodeToRemove : nodesToRemoveVec) {
        tempKv.erase(nodeToRemove);
        for (auto& kvEntry : tempKv) {
            kvEntry.second.second = 0;
        }
    }
    DltInc::nwKv initialKv;
    while ((initialKv.size() < REPLICA_FACTOR) && (tempKv.size() > 0)) {
        initialKv.insert(*(tempKv.begin()));
        tempKv.erase(tempKv.begin());
    }
    
    DltInc* dlt = new DltInc(TOTAL_TOKENS, REPLICA_FACTOR);
    dlt->init(initialKv);
    
    for (auto& kvEntry : tempKv) {
        dlt->addSingleNode(kvEntry.first, kvEntry.second.first);
    }
    finalDlt.reset(dlt);

    cout << "Cluster config after remove node for node(s):";
    for (auto& nodeToRemove : nodesToRemoveVec) { cout << nodeToRemove << ","; }
    cout << endl;
    finalDlt->printTokensOwnedAllLevels();
}


DltInc::nwKv DltInc_wrapper::createTempKv(int numNodes, int idSeed, int startingWeight, int weightGap) {
    DltInc::nwKv nodeWeightMap;
    
    for (int i = 0; i < numNodes; i++) {
        nodeWeightMap[idSeed++] = DltInc::wInfo(startingWeight + weightGap*(i+1), 0);
    }
    return nodeWeightMap;
}