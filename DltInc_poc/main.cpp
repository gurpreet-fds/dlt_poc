//
//  main.cpp
//  DltInc_poc
//
//  Created by Gurpreet Singh on 6/17/16.
//  Copyright © 2016 Gurpreet Singh. All rights reserved.
//

#include "dltInc_wrapper.h"

const int NUM_NODES_TO_ADD = 5;
const int ADD_NODE_WITH_WEIGHT = 2;

int main(int argc, const char * argv[]) {
    DltInc_wrapper dltWrapper;
    dltWrapper.addNode(NUM_NODES_TO_ADD, ADD_NODE_WITH_WEIGHT);

    cout << "\n************************************************\n";
    vector<int> nodesToRemove;
    int i = NUM_NODES_TO_ADD + DltInc_wrapper::REPLICA_FACTOR;
    for (; i > DltInc_wrapper::REPLICA_FACTOR; i--) {
        nodesToRemove.push_back(i);
    }
    dltWrapper.removeNode(nodesToRemove);
    return 0;
}