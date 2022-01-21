#ifndef BEST_H
#define BEST_H

#include "rcpsp.h"

class bestClass{
private:
    vector<int> bestSolution;
    vector<int> bestStart;
    vector<int> bestFinish;
    int bestTime;
public:
    bestClass(){
        bestSolution.clear();
        bestTime=10000;
        bestStart.clear();
        bestFinish.clear();
    }
    bestClass(vector<int> in_bestSolution, vector<int> in_bestStart, vector<int> in_bestFinish,  int in_bestTime){
        bestSolution=in_bestSolution;
        bestTime=in_bestTime;
        bestStart=in_bestStart;
        bestFinish=in_bestFinish;
    }
    int getBestSolution(int pos){return bestSolution[pos];}
    int getBestTime(){return bestTime;}
    int getBestStart(int pos){return bestStart[pos];}
    int getBestFinish(int pos){return bestFinish[pos];}

    void pushBestSolution(int value){bestSolution.push_back(value);}
    void pushBestStart(int value){bestStart.push_back(value);}
    void pushBestFinish(int value) {bestFinish.push_back(value);}

    void setBestTime(int value){bestTime=value;}
    void setBestStart(int pos,int value){bestStart[pos]=value;}
    void setBestFinish(int pos,int value){bestFinish[pos]=value;}

    void clearBestSolution(){bestSolution.clear();}
    void clearBestStart(){bestStart.clear();}
    void clearBestFinish(){bestFinish.clear();}

    void init(){
        clearBestSolution();
        clearBestStart();
        clearBestFinish();
        setBestTime(10000);
        bestKey=true;
        found=false;
    }

    bool bestKey;
    bool found;
}best;

#endif