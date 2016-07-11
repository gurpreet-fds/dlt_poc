//
//  dltInc.h
//  DltInc_poc
//
//  Created by Gurpreet Singh on 6/17/16.
//  Copyright © 2016 Gurpreet Singh. All rights reserved.
//

#ifndef dltInc_h
#define dltInc_h

#include <stdio.h>
#include <utility>
#include <map>
#include <set>
#include <iostream>
#include <math.h>
#include <vector>
#include <iomanip>


using namespace std;
#define ok 0

class DltInc {
public:
    typedef int id;
    // node id - abs weight
    typedef std::pair<int, int> info;
    //abs weight - rel weight
    typedef std::pair<int, float> wInfo;
    typedef std::map<id, wInfo> nwKv;
    typedef nwKv::iterator nwMapIter;
    typedef nwKv::reverse_iterator nwMapRIter;
    
    
    explicit DltInc(int tokens, int replicas);
    ~DltInc();
    
    // init the DLT which inital set of nodes passed as arg in nwMap
    int init(nwKv& nwMap);
    
    // assign L1,L2,L3 assignments of tokens. This is used during
    // first time fill-up of DLT.
    int assignPrimaries(int **table, int seed);
    int assignSecondaries(int **table, int seed, int replicaNum);
    int assignTertiaries(int **table, int seed, int replicaNum);
    
    // set of helper functions
    int** createTempTable(int r, int t);
    void destroyTempTable(int** table, int replicas);

    // set of print helper functions
    void printTable();
    void printNodeWeightMap();
    void printTokensOwned();
    void printl12associations();
    void printl123associations();
    int printTokensToGive();
    void printTokensOwnedAllLevels();
    
    
    void fillDlt(int ** table, int seed);
    void fillTableFroml12Associations(int row);
    void fillTableFroml123Associations(int row);

    // set of getter functions to get tokens owned by node nodeId
    int getTotalTokensOwned(int nodeId);
    int totalTokensOnLevel(int nodeId, int row);

    nwKv getKvMap() { return nwMap; }
    
    // Find nodes to assign for secondary/tertiary ownership allocations.
    int findSuitableNode(set<int>& nodes, set<int>& nodesToUse);

    // Reassigns tokens for a giverId node to takerId node
    int reassignTokens(int nextAssignmentStartsHere, int giverId, int takerId, int numOfTokens, int optimum);

    // Add more than 1 nodes to the cluster.
    void addNodesInBulk(int numOfNodes, int weight, int wfactor=0);

    // Add single node to the cluster.
    void addSingleNode(int nodeId, int absWeight);
    
private:
    int addNode(pair<int, float> nodeWeight);
    bool isNotPartOfTokenDltColumn(int column, int nodeId);

    // DLT table [replicas][#of tokens]
    int** table;

    // DLT version
    int dltv;

    // Total number of tokens assigned to the cluster.
    int tokens;

    // Number of replicas
    int replicas;

    // Map holding the nodeId -> weight mapping.
    nwKv nwMap;

    // Total absolute cluster weight.
    float totalClusterWeight;
    
    
    
    // Helper data structures, which are used while calculating
    // base DLT and add node/remove node cases.
    
    // L1 id , <Token, L2 id>
    map<int, map<int, int>> l12associations;
    // <L1 id, L2 id>, <Token, L3 id>
    map<pair<int, int>, map<int, int>> l123associations;
    
    //<node id, number of tokens to give>
    vector<pair<int, float>> tokensToTransferVec;

    map<int, int> tokensGivenMap;
};


#endif /* dltInc_h */
