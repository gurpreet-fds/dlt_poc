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
    int init(nwKv& nwMap);
    int addNode(info nodeInfo);
    void removeNode(int id);
    int assignTokens();
    int localOptima(int seed);
    float calculateOptimality(int** dlt, int seed);
    int assignPrimaries(int **table, int seed);
    int assignSecondaries(int **table, int seed, int replicaNum);
    int assignTertiaries(int **table, int seed, int replicaNum);
    
    int** createTempTable(int r, int t);
    void destroyTempTable(int** table, int replicas);
    void printTable();
    void printNodeWeightMap();
    void printTokensOwned();
    void printl12associations();
    void printl123associations();
    void fillDlt(int ** table, int seed);
    int getTotalTokensOwned(int nodeId);
    void fillTableFroml12Associations(int row);
    void fillTableFroml123Associations(int row);
    int totalTokensOnLevel(int nodeId, int row);
    void printTokensOwnedAllLevels();
    int findSuitableNode(set<int>& nodes, set<int>& nodesToUse);
    int addNode(pair<int, float> nodeWeight);
    int printTokensToGive();
    int reassignTokens(int level, int giverId, int takerId, int numOfTokens, int optimum);
    void addNodesInBulk(int numOfNodes, int weight, int wfactor=0);
    void addSingleNode(int nodeId, int absWeight);
    int reduceMarginToOptima();
    bool isNotPartOfTokenDltColumn(int column, int nodeId);
    nwKv getKvMap() { return nwMap; }
    
private:
    int** table;
    int dltv;
    int tokens;
    int replicas;
    nwKv nwMap;
    float totalClusterWeight;
    
    // L1 id , <Token, L2 id>
    map<int, map<int, int>> l12associations;
    
    // <L1 id, L2 id>, <Token, L3 id>
    map<pair<int, int>, map<int, int>> l123associations;
    
    //<node id, number of tokens to give>
    vector<pair<int, float>> tokensToTransferVec;
    map<int, int> tokensGivenMap;
};


#endif /* dltInc_h */
