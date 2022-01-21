#ifndef GENERATION_H
#define GENERATION_H

#include "rcpsp.h"

using namespace std;

class generationClass
{
private:
    vector<double> priorityVector;
    vector<int> solution;
    vector<int> activityList;
    int timeFinished;
public:
    generationClass(){priorityVector.clear(); solution.clear(); activityList.clear(); timeFinished=0;}
    generationClass(vector<double> in_priorityVector, vector<int> in_solution,vector<int> in_activityList, int in_timeFinished){
        priorityVector=in_priorityVector;
        solution=in_solution;
        timeFinished=in_timeFinished;
        activityList=in_activityList;
    }

    double getPriorityVector(int pos){return priorityVector[pos];}
    int getSolution(int pos){return solution[pos];}
    int getTimeFinished(){return timeFinished;}
    int getActivityList(int i){return activityList[i];}

    int sizeOfSolution(){return solution.size();}

    void setPriorityVector(int position, double value){priorityVector[position]=value;}
    void setSolution(int position, int value){solution[position]=value;}
    void setTimeFinished(int value){timeFinished=value;}
    void setActivityList(int position, int value){activityList[position]=value;}

    void pushPriorityVector(double value){priorityVector.push_back(value);}
    void pushSolution(int value){solution.push_back(value);}
    void pushActivityList(int value){activityList.push_back(value);}

    void clearPriorityVector(){priorityVector.clear();}
    void clearSolution(){solution.clear();}
    void clearActivityList(){activityList.clear();}

    void reverseActivityList(){
        //reverse(activityList.begin(),activityList.end()+1);
        vector<int> tempSolution;
        tempSolution.clear();
        tempSolution.push_back(inputs-1);
        for(int i=0;i<inputs-1;i++) tempSolution.push_back(activityList[i]);

        activityList.clear(); 
        for(int i=inputs-2;i>=0;i--) activityList.push_back(tempSolution[i]);
    }
    int sizeActivityList(){ return activityList.size();}

    vector<int> localBestSolution;
    int localBestTime;

    void localInit(){
        localBestSolution.clear();
        localBestTime=1000000;
    }
}*gen;

#endif