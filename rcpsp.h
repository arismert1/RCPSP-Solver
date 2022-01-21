#ifndef RCPSP_H
#define RCPSP_H

#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>
#include <algorithm>
#include <vector>
#include <time.h>
#include <random>

using namespace std;

class rcpspClass
{
private:
    int activity;
    int duration;
    int *demand;
    int numOfSuccessors;
    int *successorID;
    vector<int> predecessorID;
    int start;
    double pheromone;
    double *evaporationTable;
    bool aChecked; //true if activity IS checked
public:
    rcpspClass(){
        activity=0;
        duration=0; 
        demand=NULL; 
        numOfSuccessors=0; 
        successorID=NULL;
        predecessorID.clear();
        start=0;
        pheromone=1.0;
        evaporationTable=NULL;
        aChecked=false;
    }
    rcpspClass(int in_activity, int in_duration, int *in_demand, int in_numOfSuccessors, int *in_successorID, vector<int> in_predecessorID, int in_start, double in_pheromone,double *in_evaporationTable, bool in_aChecked){
        activity=in_activity; 
        duration=in_duration;
        demand=in_demand;
        numOfSuccessors=in_numOfSuccessors;
        successorID=in_successorID;
        start=in_start;
        pheromone=in_pheromone;
        evaporationTable=in_evaporationTable;
        aChecked=in_aChecked;
        predecessorID=in_predecessorID;
    }

    int getActivity(){return activity;}
    int getDuration(){return duration;}
    int getDemand(int pos){return demand[pos];}
    int getNumOfSuccessors(){return numOfSuccessors;}
    int getSuccessorID(int pos){return successorID[pos];}
    int getPredecessorID(int i){return predecessorID[i];}
    int getStart(){return start;}
    double getPheromone(){return pheromone;}
    double getEvaporationTable(int pos){return evaporationTable[pos];}
    bool isChecked(){return aChecked;}

    int getNumOfPredecessors(){return predecessorID.size();}
    int getFinish(){return start+duration;}

    void setActivity(int value){activity=value;}
    void setDuration(int value){duration=value;}
    void setDemand(int pos, int value){demand[pos]=value;}
    void setNumOfSuccessors(int value){numOfSuccessors=value;}
    void setSuccessorID(int pos, int value){successorID[pos]=value;}
    void pushPredecessorID(int value){predecessorID.push_back(value);}
    void setStart(int value){start=value;}
    void setPheromone(double value){pheromone=value;}
    void setEvaporationTable(int pos, double value){evaporationTable[pos]=value;}
    void setaChecked(bool value){aChecked=value;}

    void setFinish(int value){start=value-duration;}

}*rcpsp;

int inputs;
int numOfActivities;
int numOfResources;
int *resourceCap;
int maxTimeUnits=0;

//string filename="j30/J301_1.RCP";
//string filename="model.txt";
string filename="j60/J6010_1.RCP";
//string filename="x1/X1_1.RCP";
//string filename="x1/X60_1.RCP";

int populationSize=50;
int numOfFiles=100;
int antLoops=6;
int geneticLoops=4;
int generationLimit=6;

/*
int populationSize=4;
int numOfFiles=14;
int antLoops=1;
int geneticLoops=0;
int generationLimit=0;
*/
double geneticTolerance=1.0;

double evaporationRate=0.03;
int evaporationTableSize=100;
int evaporationInd=0;

int initialPOP=populationSize;
ofstream outfile("output_temp.txt");
int epanalipseis=0;

#endif