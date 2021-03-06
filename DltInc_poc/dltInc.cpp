//
//  dltInc.cpp
//  DltInc_poc
//
//  Created by Gurpreet Singh on 6/17/16.
//  Copyright © 2016 Gurpreet Singh. All rights reserved.
//

#include "dltInc.h"

DltInc::DltInc(int t, int r): tokens(t), replicas(r), dltv(0) {
    table = new int*[r];
    for (int i = 0; i < r; i++) {
        table[i] = new int[t];
    }
    for (int i = 0; i < r; i++) {
        for (int j = 0; j < t; j++) {
            table[i][j] = 0;
        }
    }
}

DltInc::~DltInc() {
    for (int i = 0; i < replicas; i++) {
        delete[] table[i];
    }
    delete[] table;
}

int DltInc::init(nwKv& nodeWeightMap) {

    //store kv locally
    nwMap = nodeWeightMap;

    int tw = 0;
    // calculate total weight of cluster.
    for (auto& nw: nwMap) {
        tw += nw.second.first;
    }

    totalClusterWeight = tw;

    // fill up relative weight of nodes.
    for (auto& nw: nwMap) {
        auto tempRelWeight = (float)nw.second.first/tw;
        nw.second.second = tempRelWeight;
    }
    
    // node id - weight map is filled. Calculate
    // optimal positions for the table.
    auto maxSeed = 0;
    l12associations.clear();
    l123associations.clear();
    fillDlt(table, maxSeed);
    fillTableFroml12Associations(1);
    fillTableFroml123Associations(2);
    cout << "Base cluster configuration and token allocation\n";
    printTokensOwnedAllLevels();
    return ok;
}

void DltInc::addSingleNode(int nodeId, int absWeight) {
    addNode(pair<int, float>(nodeId, absWeight));
}

void DltInc::addNodesInBulk(int numOfNodes, int weight, int wfactor) {
    for (int i = 0; i < numOfNodes; i++) {
        auto nodeToAddId = nwMap.size()+1;
        addNode(pair<int, float>(nodeToAddId, weight + wfactor*i));
        cout << "Cluster config after add node: " << nodeToAddId << endl;
        printTokensOwnedAllLevels();
    }
}

int DltInc::getTotalTokensOwned(int nodeId) {
    int totalTokens = 0;
    for (int i = 0; i < replicas; i++) {
        for (int j = 0; j < tokens; j++) {
            if (table[i][j] == nodeId) {
                totalTokens++;
            }
        }
    }
    return totalTokens;
}

int DltInc::totalTokensOnLevel(int nodeId, int row) {
    int total = 0;
    for (int i = 0; i < tokens; i++) {
        if (table[row][i] == nodeId) {
            total += 1;
        }
    }
    return total;
}

void DltInc::printTokensOwned() {
    for (auto& node : nwMap) {
        cout << node.first << " -> " << getTotalTokensOwned(node.first) << endl;
    }
    cout << endl;
}
        
int** DltInc::createTempTable(int r, int t) {
    auto temptable = new int*[r];
    for (int i = 0; i < r; i++) {
        temptable[i] = new int[t];
    }
    for (int i = 0; i < r; i++) {
        for (int j = 0; j < t; j++) {
            temptable[i][j] = 0;
        }
    }
    return temptable;
}

void DltInc::destroyTempTable(int** temptable, int replicas) {
    for (int i = 0; i < replicas; i++) {
        delete[] temptable[i];
    }
    delete[] temptable;
}


void DltInc::fillDlt(int **dlt, int seed) {
    assignPrimaries(dlt, seed);
    assignSecondaries(dlt, seed, 1);
    assignTertiaries(dlt, seed, 2);
}

int DltInc::assignSecondaries(int **table, int seed, int replicaNum) {

    for (nwMapIter l1iter = nwMap.begin(); l1iter != nwMap.end(); ++l1iter) {
        int nodeId = l1iter->first;
        int tokensToAssign = totalTokensOnLevel(nodeId, 0);
        auto curTokenToAssignIter = l12associations[nodeId].begin();
        for (nwMapIter l2iter = nwMap.begin(); l2iter != nwMap.end(); ++l2iter) {
            if (l1iter == l2iter) {
                continue;
            }
            auto newWeight = totalClusterWeight - l1iter->second.first;
            float newRelWeight = (float)l2iter->second.first/newWeight;
            int l12tokensToAssign = round(newRelWeight * tokensToAssign);
            // fix .5 case
            while (l12tokensToAssign && curTokenToAssignIter != l12associations[nodeId].end()) {
                curTokenToAssignIter->second = l2iter->first;
                l123associations[pair<int, int>(nodeId, l2iter->first)][curTokenToAssignIter->first] = -1;
                l12tokensToAssign--;
                curTokenToAssignIter++;
            }
        }
    }
    
    set<int> toUseNodes;
    for (auto& node : nwMap) {
        toUseNodes.insert(node.first);
    }
    
    for (auto& entry: l12associations) {
        for (auto iter = entry.second.begin(); iter != entry.second.end(); ++iter) {
            if (iter->second == -1) {
                set<int> s;
                s.insert(entry.first);
                iter->second = findSuitableNode(s, toUseNodes);
                l123associations[pair<int, int>(entry.first, iter->second)][iter->first] = -1;
            }
        }
    }
    
    return ok;
}



int DltInc::assignTertiaries(int **table, int seed, int replicaNum) {
    
    for (nwMapIter iterp = nwMap.begin(); iterp != nwMap.end(); ++iterp) {
        for (nwMapIter iters = nwMap.begin(); iters != nwMap.end(); ++iters) {
            
            auto curl12pair = pair<int, int>(iterp->first, iters->first);
            auto curIter = l123associations[curl12pair].begin();
            int tokensToAssign = l123associations[curl12pair].size();
            for (nwMapIter iter2 = nwMap.begin(); iter2 != nwMap.end(); ++iter2) {
                if (iterp == iter2 || iters == iter2) {
                    continue;
                }
                
                auto newWeight = totalClusterWeight - (iterp->second.first + iters->second.first);
                float newRelWeight = (float)iter2->second.first/newWeight;
                int l123tokens = round(newRelWeight * tokensToAssign);
               
                // fix .5 case
                while (l123tokens && curIter != l123associations[curl12pair].end()) {
                    curIter->second = iter2->first;
                    l123tokens--;
                    curIter++;
                }
            }
        }
    }
    
    set<int> toUseNodes;
    for (auto& node : nwMap) {
        toUseNodes.insert(node.first);
    }
    
    for (auto& entry: l123associations) {
        for (auto iter = entry.second.begin(); iter != entry.second.end(); ++iter) {
            if (iter->second == -1) {
                set<int> s;
                s.insert(entry.first.first);
                s.insert(entry.first.second);
                iter->second = findSuitableNode(s, toUseNodes);
            }
        }
    }
    
    return ok;
}

int DltInc::findSuitableNode(set<int>& nodes, set<int>& nodesToUse) {
    
    int chosenNode = -1;

    auto node = nodes.begin();
    while (node != nodes.end()) {
        if (nodesToUse.empty()) {
            break;
        } else if (nodesToUse.find(*node) != nodesToUse.end()) {
            nodesToUse.erase(*node);
            node++;
        } else {
            nodes.erase(*node);
            node = nodes.begin();
        }
    }
    
    if (!nodesToUse.empty()) {
        chosenNode = *nodesToUse.begin();
        nodesToUse.erase(nodesToUse.begin());
    } else {
        for (auto& node : nwMap) {
            nodesToUse.insert(node.first);
        }
        for (auto node = nodes.begin(); node != nodes.end(); ++node) {
            nodesToUse.erase(*node);
        }
        chosenNode = *nodesToUse.begin();
    }
    
    // put back the erased nodes into nodesToUse
    nodesToUse.insert(nodes.begin(), nodes.end());
    return chosenNode;
}

void DltInc::fillTableFroml12Associations(int row) {
    
    for (auto iter = l12associations.begin(); iter != l12associations.end(); ++iter) {
        for (auto iter2 = iter->second.begin(); iter2 != iter->second.end(); ++iter2) {
            table[row][iter2->first]  = iter2->second;
        }
    }
}

void DltInc::fillTableFroml123Associations(int row) {
    
    for (auto iter = l123associations.begin(); iter != l123associations.end(); ++iter) {
        for (auto iter2 = iter->second.begin(); iter2 != iter->second.end(); ++iter2) {
            table[row][iter2->first]  = iter2->second;
        }
    }
}

int DltInc::assignPrimaries(int **table, int seed) {
    
    int totalTokensAssigned = 0;
    for (auto &node: nwMap) {
        int tokensToAssign = node.second.second * tokens;
        totalTokensAssigned += tokensToAssign;
        
        while (tokensToAssign-- > 0) {
            int currentToken = seed++ % tokens;
            table[0][currentToken] = node.first;
            l12associations[node.first][currentToken] = -1;
        }
    }

    nwMapIter iter = nwMap.begin();
    while (totalTokensAssigned++ < tokens) {
        int currentToken = (seed++ % tokens);
        table[0][currentToken] = iter->first;
        l12associations[iter->first][currentToken] = -1;
        iter++;
        if (iter == nwMap.end()) { iter = nwMap.begin(); }
    }
    
    return ok;
}

void DltInc::printNodeWeightMap() {
    
    cout << "id - aw - rw" << endl;
    for (nwMapIter iter = nwMap.begin();
         iter != nwMap.end(); iter++) {
        cout << iter->first << " " << iter->second.first << " " << iter->second.second << endl;
    }
    cout << endl;
}

void DltInc::printTable() {
    
    cout << "Dlt version: " << dltv << " rf: " << replicas << " tokens: " << tokens << endl;
    for (int i = 0; i < tokens; i++) {
        cout << i << " ";
        for (int j = 0; j < replicas; j++) {
            cout << table[j][i] << " ";
        }
        cout << endl;
    }
    cout << "====================\n";
}

void DltInc::printl12associations() {
    
    cout << endl << "N " << "T " << "l12 ";
    for (auto iter = l12associations.begin(); iter != l12associations.end(); ++iter) {
        for (auto iter2 = iter->second.begin(); iter2 != iter->second.end(); ++iter2) {
            cout << endl << iter->first << " " << iter2->first << " " << iter2->second;
        }
    }
    cout << endl;
}

void DltInc::printl123associations() {
    
    cout << endl << "N " << " T " << "l123 ";
    for (auto iter = l123associations.begin(); iter != l123associations.end(); ++iter) {
        for (auto iter2 = iter->second.begin(); iter2 != iter->second.end(); ++iter2) {
            cout << endl << "(" << iter->first.first << "," << iter->first.second << ") -> "
                 << iter2->first << " " << iter2->second;
        }
    }
    cout << endl;
}

bool lessPair(pair<int, float>& lhs, pair<int, float>& rhs) {
    
    if (lhs.second > rhs.second) {
        return true;
    } else {
        return false;
    }
}

int DltInc::addNode(pair<int, float> nodeAndWeight) {
    
    auto newClusterWeight = nodeAndWeight.second + totalClusterWeight;
    tokensToTransferVec.clear();

    for (auto& node: nwMap) {
        auto newRelWeight = float(node.second.first)/newClusterWeight;
        // update the new rel weight
        auto changedRelWeight = ((float)getTotalTokensOwned(node.first))/(tokens * replicas) - newRelWeight;
        tokensToTransferVec.push_back(std::pair<int, float>(node.first, changedRelWeight * tokens));
    }
    
    sort(tokensToTransferVec.begin(), tokensToTransferVec.end(), lessPair);
    
        
    int optimum = round(nodeAndWeight.second/newClusterWeight * replicas * tokens);
    bool noMoreToAssign = false;
    int nextAssignmentFrom = 0;

    for (auto& giverNode : tokensToTransferVec) {
        auto tokensToGiveOnThisLevel = round(giverNode.second);
        nextAssignmentFrom = reassignTokens(nextAssignmentFrom, giverNode.first, nodeAndWeight.first,
                                            tokensToGiveOnThisLevel, optimum);
    }
    
    //update all data structures to reflect new node.
    totalClusterWeight = newClusterWeight;
    
    // update relative weights of existing nodes in the cluster.
    for (auto& existingNode : nwMap) {
        existingNode.second.second = float(existingNode.second.first)/totalClusterWeight;
    }
    
    // update info of new node added to the cluster.
    nwMap[nodeAndWeight.first] = wInfo(nodeAndWeight.second, nodeAndWeight.second/totalClusterWeight);
    
    // increase the dlt version.
    ++dltv;
    
    return ok;
}

bool DltInc::isNotPartOfTokenDltColumn(int column, int nodeId) {
    
    for (int row = 0; row < replicas; ++row) {
        if (table[row][column] == nodeId) {
            return false;
        }
    }
    return true;
}

int DltInc::reassignTokens(int nextAssignmentStartsHere, int giverId, int takerId, int tokensToGivePerLevel, int optimum) {
    
    int curTokenNumber = nextAssignmentStartsHere;
    int startingPoint = curTokenNumber;
    for (int level = 0; level < replicas; ++level) {
        auto numOfTokens = round(tokensToGivePerLevel);
        curTokenNumber = startingPoint;
        startingPoint = ((curTokenNumber - 1) % tokens);
        if (startingPoint < 0) {
            startingPoint = tokens - 1;
        }
        do {
            if (numOfTokens <= 0 || getTotalTokensOwned(takerId) >= optimum) {
                startingPoint = curTokenNumber;
                break;
            }
            if (table[level][curTokenNumber] == giverId && isNotPartOfTokenDltColumn(curTokenNumber, takerId)) {
                table[level][curTokenNumber] = takerId;
                --numOfTokens;
            }
            ++curTokenNumber;
            curTokenNumber %= tokens;
        } while ((curTokenNumber != startingPoint));
    }
    
    return startingPoint;

}

int DltInc::printTokensToGive() {
    
    for (auto& node: tokensToTransferVec) {
        cout << endl << "node id: " << node.first << " ttg: " << node.second;
    }
    cout << endl;
    return ok;
}

bool comparator(pair<int, int> iterFirst, pair<int, int> iterSecond) {
    
    if (iterFirst.second < iterSecond.second) {
        return true;
    } else {
        return false;
    }
}

void DltInc::printTokensOwnedAllLevels() {
    
    map<int, int> tokendb;
    map<int, int> tokendbp;
    map<int, int> tokendbs;
    map<int, int> tokendbt;
    
    for (int i = 0; i < replicas; i++) {
        for (int j = 0; j < tokens; j++) {
            if (table[i][j] != 0) {
                tokendb[table[i][j]]++;
                if (i == 0) tokendbp[table[i][j]]++;
                else if (i == 1) tokendbs[table[i][j]]++;
                else if (i == 2) tokendbt[table[i][j]]++;
            }
        }
    }
    
    auto minmax = minmax_element(tokendb.begin(), tokendb.end(), comparator);
    cout << "Nodes in cluster: " << nwMap.size() << " --> ";
    for (auto& eachNode: nwMap) {
        cout << eachNode.first << " ";
    }
 
    auto it = tokendb.begin();
    auto itp = tokendbp.begin();
    auto its = tokendbs.begin();
    auto itt = tokendbt.begin();

    cout << endl;
    auto nwMapIter1 = nwMap.begin();
    cout << setw(4) << "Node" << setw(5) << "L1" << setw(5) << "L2" << setw(5) << "L3" << setw(8) << "Total" << endl;
    while (nwMapIter1 != nwMap.end()) {
        cout  << setw(3) << it->first << ":" << setw(5) << itp->second
              << setw(5) << its->second << setw(5) << itt->second << setw(8)
              << it->second << setw(15) << "(optimum: "
              << (nwMapIter1->second.second * replicas * tokens) << ")" << endl;
        if (it != tokendb.end()) ++it;
        if (itp != tokendbp.end()) ++itp;
        if (its != tokendbs.end()) ++its;
        if (itt != tokendbt.end()) ++itt;
        nwMapIter1++;
    }
    
    cout << endl;
}
