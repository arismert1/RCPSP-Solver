#include "rcpsp.h"
#include "generation.h"
#include "best.h"
using namespace std;

string* changeFilename(int numOfFiles){
    string *fileVector;
    string tmp;
    fileVector=new string [numOfFiles];
    int a; int b;

    a=(int)filename[10]-'0';
    b=(int)filename[8]-'0';

    for(int loop=0;loop<numOfFiles;loop++){
        fileVector[loop]=filename;
        if(a<10){
            a++;
            filename[8]=b+'0';
            filename[10]=a+'0';
        }
        if(a==11){
            filename=tmp;
            a=1;
            b++;
            filename[8]=b+'0';
            filename[10]=a+'0';
        }
        if(a==10){
            tmp=filename;
            //filename="x1/X1_10.RCP";
            //filename="j60/J601_10.RCP";
            filename="j60/J6010_10.RCP";
            //filename="x1/X60_10.RCP";
            filename[8]=b+'0';
            a=11;
        }
    }
    return fileVector;
}

void createExcel(){
    outfile<<filename<<",";
    for(int i=0;i<inputs-1;i++){
        outfile<<best.getBestSolution(i)<<",";
    }
    outfile<<endl;

    outfile<<"BestTime:,";
    for(int i=0;i<inputs-1;i++){
        outfile<<best.getBestStart(i)<<",";
    }
    outfile<<endl;

    outfile<<best.getBestTime()<<",";
    for(int i=0;i<inputs-1;i++){
        outfile<<best.getBestFinish(i)<<",";
    }
    outfile<<endl<<endl;
}

double fRand(double fMin, double fMax){ //RANDOM DOUBLE NUMBERS
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    srand((time_t)ts.tv_nsec);
    double f = (double)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

void createPredecessorModel(){
    for(int i=0; i<inputs;i++){
        for(int j=0;j<rcpsp[i].getNumOfSuccessors();j++){
            rcpsp[rcpsp[i].getSuccessorID(j)].pushPredecessorID(rcpsp[i].getActivity());
        }
    }
}

void createPriorityVector(){
    vector<double> randArray;
    int randInd=0;
    for(int i=0;i<numOfActivities*populationSize;i++)
        randArray.push_back(fRand(0,1));

    for(int g=0;g<populationSize;g++){
        gen[g].clearPriorityVector();
    }

    for(int g=0;g<populationSize;g++){
        for(int i=1;i<=numOfActivities;i++) { //VGALE EKSW TO START KAI TO FINISH
            gen[g].pushPriorityVector(randArray[randInd]/rcpsp[i].getPheromone());
            randInd++;
        }
    }
}

void createActivityVector(){ //ACTIVITY VECTOR BASED ON PRIORITY VECTOR
    for(int g=0;g<populationSize;g++){
        gen[g].clearSolution();
    }

    for(int g=0;g<populationSize;g++){
        for(int i=0;i<numOfActivities;i++){
            gen[g].pushSolution(0);
        }
    }

    double tempMin=10.0;
    double lastMin=0.0;
    int rankValue;

    for(int g=0;g<populationSize;g++){
        lastMin=0.0;
        for(int rankInd=0;rankInd<numOfActivities;rankInd++){
            tempMin=10.0;
            for(int prioInd=0;prioInd<numOfActivities; prioInd++){
                if((gen[g].getPriorityVector(prioInd)<tempMin) && (gen[g].getPriorityVector(prioInd)>lastMin)){
                    tempMin=gen[g].getPriorityVector(prioInd);
                    rankValue=prioInd;
                }
            }
            gen[g].setSolution(rankValue,rankInd+1);
            lastMin=tempMin;
        }
    }
}

void updatePriorityVector(){ //PRIORITY VECTOR BASED ON ACTIVITY LIST
    for(int g=0;g<populationSize;g++)
        gen[g].clearPriorityVector();

    double priority;

    for(int g=0;g<populationSize;g++)
        gen[g].pushPriorityVector(0.0);

    for(int g=0;g<populationSize;g++){
        for(int i=1;i<=numOfActivities;i++){
            priority=(gen[g].getSolution(i-1)*1.0)/(numOfActivities*1.0);
            gen[g].setPriorityVector(i-1,priority);
        }
    }

    double *temp;
    temp=new double [numOfActivities];

    for(int i=0;i<numOfActivities;i++)
        temp[i]=fRand(0.950001,0.999999);

    for(int g=0;g<populationSize;g++){
        for(int i=0;i<numOfActivities;i++){
            //tmp=fRand(0.94999,0.99999);
            gen[g].setPriorityVector(i,gen[g].getPriorityVector(i)*temp[i]);
        }
    }
}

bool allPredecessorsChecked(int act){
    int exitInd=0;
    bool ok;

    while(exitInd<rcpsp[act].getNumOfPredecessors()){
        if(rcpsp[rcpsp[act].getPredecessorID(exitInd)].isChecked()){
            ok=true;
            exitInd++;
        }
        else {
            ok=false;
            break;
        }
    }
    return ok;   
}

bool allSuccessorsChecked(int act){
    int exitInd=0;
    bool ok;

    while(exitInd<rcpsp[act].getNumOfSuccessors()){
        if(rcpsp[rcpsp[act].getSuccessorID(exitInd)].isChecked()){
            ok=true;
            exitInd++;
        }
        else {
            ok=false;
            break;
        }
    }
    return ok;   
}

int latestPredecessorFinish(int act){ 
    int max=0;

    for(int i=0;i<rcpsp[act].getNumOfPredecessors();i++){
        if(rcpsp[rcpsp[act].getPredecessorID(i)].getFinish()>max){
            max=rcpsp[rcpsp[act].getPredecessorID(i)].getFinish();
        }
    }
    return max;
}

int earliestSuccessorStart(int act){ 
    int min=1000;

    for(int i=0;i<rcpsp[act].getNumOfSuccessors();i++){
        if(rcpsp[rcpsp[act].getSuccessorID(i)].getStart()<min){
            min=rcpsp[rcpsp[act].getSuccessorID(i)].getStart();
        }
    }
    return min;
}

int backwardActIntroduction(int act, vector<vector<int>> funcTimeUnits){ //TIME ACT GOES IN BACKWARD SCHEDULE
    int ess=earliestSuccessorStart(act);
    int dur=rcpsp[act].getDuration();
    
    int i=ess-dur;
    while(i<ess){
        for(int j=0;j<numOfResources;j++){
            if(funcTimeUnits[i][j]+rcpsp[act].getDemand(j)>resourceCap[j]){
                ess--;
                i=ess-dur-1;
                break;
            }
        }
        i++;
    }
    return ess;
}

void backwardSchedule(int g, vector<int> currentSolution){
    for(int i=0;i<inputs;i++)
        rcpsp[i].setFinish(maxTimeUnits);

    vector<vector<int>> timeUnits; //pinakas me 8eseis oses oi xronikes monades kai value oso to total demand
    vector<int> dummyVector;

    for(int a=0;a<numOfResources;a++) dummyVector.push_back(0);
    for(int a=0;a<maxTimeUnits;a++) timeUnits.push_back(dummyVector);

    gen[g].setTimeFinished(maxTimeUnits);
    gen[g].clearActivityList();

    for(int a=0;a<maxTimeUnits;a++){
        for(int k=0;k<numOfResources; k++){
            timeUnits[a][k]=0;
        }
    }

    rcpsp[inputs-1].setaChecked(true);
    for(int k=0;k<inputs-1;k++){
    rcpsp[k].setaChecked(false);
    }

    int actInd=0;
    int exitInd=0;
    while(exitInd<inputs-1){ //MEXRI NA GINEI CHECKED TO FINISH (START)
        if((!rcpsp[currentSolution[actInd]].isChecked()) && (allSuccessorsChecked(currentSolution[actInd]))){
            rcpsp[currentSolution[actInd]].setFinish(backwardActIntroduction(currentSolution[actInd],timeUnits));//ANANEWSE TO START
            int lowerBound=rcpsp[currentSolution[actInd]].getStart();

            for(int update=lowerBound;update<rcpsp[currentSolution[actInd]].getFinish();update++){
                for(int resource=0;resource<numOfResources;resource++){ //update timeUnits
                    timeUnits[update][resource]+=rcpsp[currentSolution[actInd]].getDemand(resource);
                    if(update<gen[g].getTimeFinished()) gen[g].setTimeFinished(update); //TELIKO TIME FINISHED
                }
            }
            rcpsp[currentSolution[actInd]].setaChecked(true);
            gen[g].pushActivityList(currentSolution[actInd]);
            actInd=0;
            exitInd++;
        }
        else actInd++;
    }
    gen[g].setTimeFinished(maxTimeUnits-gen[g].getTimeFinished());

    gen[g].reverseActivityList();

    double increase;
    double step=1.0;
    for(int i=0;i<numOfActivities;i++){
        increase=(1.0-(step/(numOfActivities*1.0)))*fRand(1.0001,1.1001)/(numOfActivities*10.0);
        rcpsp[gen[g].getActivityList(i)].setPheromone(rcpsp[gen[g].getActivityList(i)].getPheromone()+increase);
        step+=1.0;
    }

    if(gen[g].getTimeFinished()<gen[g].localBestTime){
        gen[g].localBestTime=maxTimeUnits-gen[g].getTimeFinished();
        gen[g].localBestSolution.clear();
        for(int pos=0;pos<inputs-1;pos++)
            gen[g].localBestSolution.push_back(gen[g].getActivityList(pos));
    }

    int difference=rcpsp[0].getStart();

    if(gen[g].getTimeFinished()<=best.getBestTime()/geneticTolerance){
        best.found=true;
    }

    if((gen[g].getTimeFinished()<best.getBestTime()) || best.bestKey){
        best.clearBestSolution();
        best.clearBestStart();
        best.clearBestFinish();
        for(int i=0;i<inputs-1; i++){
            best.pushBestSolution(currentSolution[i]);
            best.pushBestStart(rcpsp[currentSolution[i]].getStart()-difference);
            best.pushBestFinish(rcpsp[currentSolution[i]].getFinish()-difference);
        }
        best.setBestTime(rcpsp[inputs-1].getFinish()-rcpsp[0].getFinish());
        best.bestKey=false;
    }
}

int forwardActIntroduction(int act, vector<vector<int>> funcTimeUnits){ //TIME ACT GOES IN FORWARD SCHEDULE

    int lpf=latestPredecessorFinish(act);
    int dur=rcpsp[act].getDuration();
    
    int i=lpf;
    while(i<lpf+dur){
        for(int j=0;j<numOfResources;j++){
            if(funcTimeUnits[i][j]+rcpsp[act].getDemand(j)>resourceCap[j]){
                lpf++;
                i=lpf-1;
                break;
            }
        }
        i++;
    }
    return lpf;
}

void forwardSchedule(int g, vector<int> currentSolution){
    vector<vector<int>> timeUnits; //pinakas me 8eseis oses oi xronikes monades kai value oso to total demand
    vector<int> dummyVector;

    for(int a=0;a<numOfResources;a++) dummyVector.push_back(0);
    for(int a=0;a<maxTimeUnits;a++) timeUnits.push_back(dummyVector);

    rcpsp[0].setStart(0);

    vector<int> currentActivityList;
    gen[g].setTimeFinished(0);

    for(int a=0;a<maxTimeUnits;a++){
        for(int k=0;k<numOfResources; k++){
            timeUnits[a][k]=0;
        }
    }


    rcpsp[0].setaChecked(true);
    for(int k=1;k<=inputs-1;k++){ //8ESE OLA TA ACTIVITIES NOT CHECKED
    rcpsp[k].setaChecked(false);
    }

    int actInd=0;
    int exitInd=0;
    while(exitInd<inputs-1){ //MEXRI NA GINEI CHECKED TO FINISH
        if((!rcpsp[currentSolution[actInd]].isChecked()) && (allPredecessorsChecked(currentSolution[actInd]))){

            rcpsp[currentSolution[actInd]].setStart(forwardActIntroduction(currentSolution[actInd],timeUnits));//ANANEWSE TO START
            
            int upperBound=rcpsp[currentSolution[actInd]].getFinish();

            for(int update=rcpsp[currentSolution[actInd]].getStart();update<upperBound;update++){
                for(int resource=0;resource<numOfResources;resource++){ //update timeUnits
                    timeUnits[update][resource]+=rcpsp[currentSolution[actInd]].getDemand(resource);
                    if(update>gen[g].getTimeFinished()) gen[g].setTimeFinished(update); //TELIKO TIME FINISHED
                }
            }

            rcpsp[currentSolution[actInd]].setaChecked(true);
            currentActivityList.push_back(currentSolution[actInd]);
            actInd=0;
            exitInd++;
        }
        else actInd++;
    }
    gen[g].setTimeFinished(gen[g].getTimeFinished()+1);

    double increase;
    double step=1.0;
    for(int i=0;i<numOfActivities;i++){
        increase=(1.0-(step/(numOfActivities*1.0)))*fRand(1.0001,1.1001)/(numOfActivities*10.0);
        rcpsp[currentActivityList[i]].setPheromone(rcpsp[currentActivityList[i]].getPheromone()+increase);
        step+=1.0;
    }

    if(gen[g].getTimeFinished()<gen[g].localBestTime){
        gen[g].localBestTime=gen[g].getTimeFinished();
        gen[g].localBestSolution.clear();
        for(int pos=0;pos<inputs-1;pos++)
            gen[g].localBestSolution.push_back(currentActivityList[pos]);
    }

    if(gen[g].getTimeFinished()<=best.getBestTime()*geneticTolerance){
        best.found=true;
    }

    if((gen[g].getTimeFinished()<best.getBestTime()) || best.bestKey){
        best.setBestTime(gen[g].getTimeFinished());
        best.clearBestSolution();
        best.clearBestStart();
        best.clearBestFinish();
        for(int i=0;i<inputs-1; i++){
            best.pushBestSolution(currentSolution[i]);
            best.pushBestStart(rcpsp[currentSolution[i]].getStart());
            best.pushBestFinish(rcpsp[currentSolution[i]].getFinish());
        }
        best.bestKey=false;
    }
}

void singlePointCrossover(){
    vector<int> pairs;
    pairs.clear();
    for(int i=0;i<populationSize;i++) {pairs.push_back(i);}

    int splitPoint;

    std::random_device rd;
    std::mt19937 generator(rd()); 
    normal_distribution<double> distribution((double)numOfActivities/2,2.5);

    random_shuffle(pairs.begin(), pairs.end());

    do{
    splitPoint=abs((int)distribution(generator));
    }while ((splitPoint<2) || (splitPoint>inputs-2));

    int ind=0;
    vector<double> tempVec;
    tempVec.clear();

    for(int g=0;g<populationSize;g+=2){
        for(int tmp=splitPoint;tmp<numOfActivities;tmp++){
            tempVec.push_back(gen[pairs[g]].getPriorityVector(tmp));
            gen[pairs[g]].setPriorityVector(tmp, gen[pairs[g+1]].getPriorityVector(tmp));
            gen[pairs[g+1]].setPriorityVector(tmp, tempVec[ind]);
            ind++;
        }  
    }  
}

void twoPointCrossover(){
    vector<int> pairs;
    pairs.clear();

    int splitPoint1;
    int splitPoint2;

    int ind=0;
    vector<double> tempVec;
    tempVec.clear();

    for(int i=0;i<populationSize;i++) {pairs.push_back(i);}
    random_shuffle(pairs.begin(), pairs.end());

    std::random_device rd;
    std::mt19937 generator(rd()); 
    normal_distribution<double> distribution((double)numOfActivities/2.5,3.0);

    do{
     splitPoint1=abs((int)distribution(generator));
     //splitPoint1=abs(rand()%inputs/3)+(inputs/3);
     splitPoint2=(abs(rand()%numOfActivities-splitPoint1))+splitPoint1+3;
    }while ((splitPoint1<3) && (splitPoint1>inputs-3) && (splitPoint2>numOfActivities));

    for(int g=0;g<populationSize;g+=2){
        for(int tmp=splitPoint1;tmp<splitPoint2;tmp++){
            tempVec.push_back(gen[pairs[g]].getPriorityVector(tmp));
            gen[pairs[g]].setPriorityVector(tmp, gen[pairs[g+1]].getPriorityVector(tmp));
            gen[pairs[g+1]].setPriorityVector(tmp, tempVec[ind]);
            ind++;
        }
    }
}

void softMutation(){
    int mutatedChildValue=10000;
    int mutatedChild;
    for(int g=0;g<populationSize;g++){
        if(gen[g].getTimeFinished()<mutatedChildValue){
            mutatedChildValue=gen[g].getTimeFinished();
            mutatedChild=g;
        }
    }

    //int i=(int) fRand(0,numOfActivities);
    //gen[mutatedChild].setPriorityVector(i,gen[mutatedChild].getPriorityVector(i)*fRand(0.2001,0.7999));

    for(int i=0;i<numOfActivities;i++){
        gen[mutatedChild].setPriorityVector(i,gen[mutatedChild].getPriorityVector(i)*fRand(0.2001,0.7999));
    }
}

void hardMutation(){
    int reduction=8;
    int mutatedChild[populationSize/reduction];
    vector<int> forbidden;
    forbidden.clear();
    forbidden.push_back(10000);

    for(int i=0;i<populationSize/reduction;i++){
        mutatedChild[i]=rand()%populationSize/reduction;
        for(long unsigned int j=0;j<forbidden.size();j++){
            if(forbidden[j]==mutatedChild[i]){
                i--;
                break;
            }
        }
        forbidden.push_back(mutatedChild[i]);
    }

    for(int j=0;j<populationSize/reduction;j++){
        for(int i=0;i<numOfActivities;i++){
            gen[mutatedChild[j]].setPriorityVector(i,gen[mutatedChild[j]].getPriorityVector(i)*fRand(0.2001,0.7999));
        }
    }
}

void localSearch(){
    vector<int> currentSolution;

    for(int g=0;g<populationSize;g++){
        epanalipseis++;
        gen[g].localInit();

        //-----------------------------------------------------------------------------
        //-----------------------------------------------------------------------------
        currentSolution.clear();
        currentSolution.push_back(0);
        for(int i=0;i<numOfActivities;i++) currentSolution.push_back(gen[g].getSolution(i));
        currentSolution.push_back(inputs-1);

        forwardSchedule(g,currentSolution); //RUNS FORWARD ACTIVITY VECTOR
        //-----------------------------------------------------------------------------
        //-----------------------------------------------------------------------------

        currentSolution[0]=inputs-1;
        currentSolution[inputs-1]=0;

        backwardSchedule(g, currentSolution); //RUNS BACKWARD ACTIVITY VECTOR
        //-----------------------------------------------------------------------------
        //-----------------------------------------------------------------------------

        vector<int> tempSolution;
        tempSolution.clear();
        tempSolution.push_back(inputs-1);
        for(int i=0;i<inputs-1;i++) tempSolution.push_back(gen[g].getActivityList(i));

        currentSolution.clear();  
        for(int i=inputs-1;i>=0;i--) currentSolution.push_back(tempSolution[i]);

        forwardSchedule(g,currentSolution); //RUNS BACKWARD'S ACTIVITY LIST
    }
}

void localPheromoneUpdate(){
    for(int g=0;g<populationSize;g++){
        double increase;
        double step=1.0;
        for(int i=0;i<numOfActivities;i++){
            increase=(1.0-(step/(numOfActivities*1.0)))/(populationSize*fRand(1.5,2.5));
            rcpsp[gen[g].localBestSolution[i]].setPheromone(rcpsp[gen[g].localBestSolution[i]].getPheromone()+increase);
            step+=1.0;
        }
    }

    if(evaporationInd==evaporationTableSize) { //edw 8a ginei allagi stin seira otan ftasei sto telos
        evaporationInd=1;
        for(int i=0;i<inputs;i++) rcpsp[i].setEvaporationTable(0,rcpsp[i].getPheromone());
        for(int i=0;i<inputs;i++)
            for(int j=1;j<evaporationTableSize;j++)
                rcpsp[i].setEvaporationTable(j,1.0);
    }

    for(int i=0;i<inputs;i++){
        rcpsp[i].setEvaporationTable(evaporationInd,rcpsp[i].getPheromone());
    }
    evaporationInd++;

    for(int i=0;i<inputs;i++){
        double sum=0.0;
        for(int j=0;j<evaporationTableSize;j++){
            sum+=rcpsp[i].getEvaporationTable(j);
        }
        rcpsp[i].setPheromone(sum/(evaporationTableSize*1.0));
    }
}

void globalPheromoneUpdate(){
    double increase;
    double step=1.0;
    for(int i=0;i<numOfActivities;i++){
        increase=(1.0-(step/(numOfActivities*1.0)))/(fRand(0.4001,0.5999));
        rcpsp[best.getBestSolution(i)].setPheromone(rcpsp[best.getBestSolution(i)].getPheromone()+increase);
        step+=1.0;
    }

    if(evaporationInd==evaporationTableSize) { //edw 8a ginei allagi stin seira otan ftasei sto telos
        evaporationInd=1;
        for(int i=0;i<inputs;i++) rcpsp[i].setEvaporationTable(0,rcpsp[i].getPheromone());
        for(int i=0;i<inputs;i++)
            for(int j=1;j<evaporationTableSize;j++)
                rcpsp[i].setEvaporationTable(j,1.0);
    }

    for(int i=0;i<inputs;i++){
        rcpsp[i].setEvaporationTable(evaporationInd,rcpsp[i].getPheromone());
    }
    evaporationInd++;

    for(int i=0;i<inputs;i++){
        double sum=0.0;
        for(int j=0;j<evaporationTableSize;j++){
            sum+=rcpsp[i].getEvaporationTable(j);
        }
        rcpsp[i].setPheromone(sum/(evaporationTableSize*1.0));
    }
}

void applyEvaporation(){
    double meiwsi=1-evaporationRate;
    for(int i=0;i<inputs;i++){
        for(int j=0;j<evaporationTableSize;j++){
            if(rcpsp[i].getEvaporationTable(j)!=1.0){
                rcpsp[i].setEvaporationTable(j, rcpsp[i].getEvaporationTable(j)*meiwsi);
            }
        }
        meiwsi=meiwsi*0.95;
    }
}

void updatePopulation(){
    for(int g=0;g<populationSize;g++){
        gen[g].clearSolution(); //ACTIVITY VECTOR -> ACTIVITY LIST
        for(int i=0;i<numOfActivities;i++)
            gen[g].pushSolution(gen[g].localBestSolution[i]);
    }
}

void shrinkPOP(){
    generationClass *genTemp;

    populationSize=initialPOP*0.7;
    if(populationSize%2!=0)populationSize=populationSize+1;

    genTemp=new generationClass [initialPOP];

    for(int g=0;g<initialPOP;g++){
        genTemp[g].clearActivityList();
        genTemp[g].clearPriorityVector();
        genTemp[g].clearSolution();
        for(int i=0;i<numOfActivities;i++){
            genTemp[g].pushSolution(gen[g].getSolution(i));
            genTemp[g].pushActivityList(gen[g].getActivityList(i));
            genTemp[g].pushPriorityVector(gen[g].getPriorityVector(i));
        }
        genTemp[g].setTimeFinished(gen[g].getTimeFinished());
    }

    gen=new generationClass [populationSize];

    for(int g=0;g<populationSize;g++){
        gen[g].clearActivityList();
        gen[g].clearPriorityVector();
        gen[g].clearSolution();
        gen[g].localInit();
    }

    for(int g=0;g<populationSize;g++){
        for(int i=0;i<numOfActivities;i++){
            gen[g].pushSolution(genTemp[g].getSolution(i));
            gen[g].pushActivityList(genTemp[g].getActivityList(i));
            gen[g].pushPriorityVector(genTemp[g].getPriorityVector(i));
        }
        gen[g].setTimeFinished(genTemp[g].getTimeFinished());
    }

    for(int g=populationSize;g<initialPOP;g++){
        for(int i=0;i<populationSize;i++){
            if(genTemp[g].getTimeFinished()<gen[i].getTimeFinished()){
                gen[i].clearActivityList();
                gen[i].clearPriorityVector();
                gen[i].clearSolution();
                gen[i].setTimeFinished(genTemp[g].getTimeFinished());
                for(int j=0;j<numOfActivities;j++){
                    gen[i].pushSolution(genTemp[g].getSolution(j));
                    gen[i].pushActivityList(genTemp[g].getActivityList(j));
                    gen[i].pushPriorityVector(genTemp[g].getPriorityVector(j));
                }
                break;
            }
        }
    }
}
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
void printBestSolution(){
    /*cout<<"Best Solution Ever:"<<endl;
    for(int i=0;i<inputs-1;i++){
        cout<<best.getBestSolution(i)<<" "<<best.getBestStart(i)<<" "<<best.getBestFinish(i)<<endl;
    }*/
    cout<<"Best Time: "<<best.getBestTime()<<endl;
}

void printPriorityVector(){
    cout<<"Vector of Priority: "<<endl;
    for(int g=0;g<populationSize;g++){
        for(int i=0;i<numOfActivities;i++) {
            cout<<gen[g].getPriorityVector(i)<<" ";
        }
        cout<<endl;
    }
}

void printActivityVector(){
    cout<<"Activities: "<<endl;

    for(int g=0;g<populationSize;g++){
        for(int i=0;i<numOfActivities;i++){
            cout<<gen[g].getSolution(i)<<" ";
        }
        cout<<endl;
    }
}

void printPredecessorModel(){
    cout<<"PREDECESSOR MODEL"<<endl;
    for(int i=0;i<inputs;i++){
        cout<<rcpsp[i].getActivity()<<" "<<rcpsp[i].getDuration()<<" ";
        for(int j=0;j<numOfResources;j++)cout<<rcpsp[i].getDemand(j)<<" ";
        for(int j=0;j<rcpsp[i].getNumOfPredecessors();j++) cout<<rcpsp[i].getPredecessorID(j)<<" ";
        cout<<endl;
    }
}

void printSuccessorModel(){
    cout<<"SUCCESSOR MODEL"<<endl;
    for(int i=0;i<inputs;i++){
        cout<<rcpsp[i].getActivity()<<" "<<rcpsp[i].getDuration()<<" ";
        for(int j=0;j<numOfResources;j++) cout<<rcpsp[i].getDemand(j)<<" ";
        cout<<rcpsp[i].getNumOfSuccessors()<<" ";
        for(int j=0;j<rcpsp[i].getNumOfSuccessors();j++) cout<<rcpsp[i].getSuccessorID(j)<<" ";
        cout<<endl;
    }
}

//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------

void readData(){

    rcpspClass();
    generationClass();
    bestClass();

    best.init();
    epanalipseis=0;

    gen=new generationClass [populationSize];

    int in_activity=0; int in_duration; int *in_demand; int in_numOfSuccessors; int *in_successorID;vector<int> in_predecessorID; int in_start=0; double in_pheromone; double *in_evaporationTable; bool in_aChecked=false;

    //DUMMY VECTORS
    vector<int> vec_activity;
    vector<int> vec_duration;
    vector<vector<int>> vec_demand;
    vector<int> vec_demand_2;
    vector<int> vec_numOfSuc;
    vector<vector<int>> vec_sucID;
    vector<int> vec_sucID_2;

    vec_activity.clear();
    vec_duration.clear();
    vec_demand.clear();
    vec_demand_2.clear();
    vec_numOfSuc.clear();
    vec_sucID.clear();
    vec_sucID_2.clear();

    in_pheromone=1.0;
    in_predecessorID.clear();     

    fstream in_file;
    string ch;

    in_file.open(filename);
    if(!in_file.is_open()) {
        cout<<"File "<<filename<<" not found!"<<endl;
        exit(1);
    }

    int line=0;

    while(!in_file.eof()){
        if(line==0){
            in_file>>inputs>>numOfResources;
            rcpsp=new rcpspClass [inputs];
            resourceCap=new int [numOfResources];
            numOfActivities=inputs-2;
        }

        if(line==1){
            for(int j=0;j<numOfResources;j++)
                in_file>>resourceCap[j];
        }

        if(line>1){
                vec_demand_2.clear();
                vec_sucID_2.clear();

                in_file>>in_duration;
                vec_duration.push_back(in_duration);

                in_demand=new int[numOfResources];
                for(int i=0;i<numOfResources;i++){
                    in_file>>in_demand[i];
                    vec_demand_2.push_back(in_demand[i]);
                }
                vec_demand.push_back(vec_demand_2);
                
                in_file>>in_numOfSuccessors;
                vec_numOfSuc.push_back(in_numOfSuccessors);

                in_successorID=new int [in_numOfSuccessors];
                for(int i=0;i<in_numOfSuccessors;i++){
                    in_file>>in_successorID[i];
                    in_successorID[i]--;
                    vec_sucID_2.push_back(in_successorID[i]);
                }
                vec_sucID.push_back(vec_sucID_2);

                vec_activity.push_back(in_activity);
                in_activity++;

                line++;
                getline(in_file,ch);
        }
        line++;
    }
    in_file.close();
    
    for(int i=0;i<inputs;i++){
        in_evaporationTable=new double [evaporationTableSize];
        for(int j=0;j<evaporationTableSize;j++) in_evaporationTable[j]=1.0;
        in_demand=new int [numOfResources];
        in_activity=vec_activity[i];
        in_duration=vec_duration[i];
        for(int j=0;j<numOfResources;j++) in_demand[j]=vec_demand[i][j];
        in_numOfSuccessors=vec_numOfSuc[i];
        in_successorID=new int [in_numOfSuccessors];
        for(int j=0;j<in_numOfSuccessors;j++) in_successorID[j]=vec_sucID[i][j];

        rcpsp[i]=rcpspClass(in_activity,in_duration,in_demand,in_numOfSuccessors, in_successorID, in_predecessorID, in_start,in_pheromone,in_evaporationTable,in_aChecked);
    }

    for(int i=0;i<inputs;i++)
        maxTimeUnits+=rcpsp[i].getDuration();
}

void printPheromone(){
    cout<<"Pheromone: ";
    for(int i=1;i<=numOfActivities;i++){
        cout<<rcpsp[i].getPheromone()<<" ";
    }
    cout<<endl;
}

int main()
{
    clock_t start= clock();

    string *fileVec;
    fileVec=changeFilename(numOfFiles);

    vector<int> hrs;
    hrs.clear();
    int tmp;

    fstream optimal_file;
    string ch;

    //optimal_file.open("j30/j30hrs.sm");
    optimal_file.open("j60/j60hrs.sm");
    //optimal_file.open("x1/j120hrs.sm");

    if(!optimal_file.is_open()) {
        cout<<"File not Found!"<<endl;
        exit(1);
    }

    while(!optimal_file.eof()){
        optimal_file>>tmp;
        hrs.push_back(tmp);
    }
    int hrsInd=90;

    for(int fileLoop=0;fileLoop<numOfFiles;fileLoop++){

        clock_t clockLoop= clock();

        filename=fileVec[fileLoop];

        readData();

        createPredecessorModel();
        
        //printSuccessorModel();
        
        //printPredecessorModel();

        for(int aloops=0;aloops<antLoops;aloops++){

            populationSize=initialPOP;

            gen=new generationClass [populationSize];

            createPriorityVector();

            createActivityVector();

            //printPriorityVector();

            //printActivityVector();

            localSearch();

            updatePopulation();

            updatePriorityVector();

            localPheromoneUpdate();

            applyEvaporation();

            //printPheromone();

            //printPriorityVector();

            //printActivityVector();

            shrinkPOP();

            int gLoop=0;
            int limit=0;
            bool mutationMode=0;

            while(gLoop<geneticLoops){
                best.found=false;
                if(limit<generationLimit){

                    //singlePointCrossover();
                    twoPointCrossover();

                    if(mutationMode==0)softMutation();
                    if(mutationMode==1){hardMutation(); mutationMode=0;}

                    createActivityVector();

                    //printPriorityVector();

                    //printActivityVector();

                    localSearch();

                    updatePopulation();

                    updatePriorityVector();

                    localPheromoneUpdate();

                    applyEvaporation();

                    if(best.found==true) limit=0;
                    else limit++;
                }
                else{
                    mutationMode=1;
                    limit=0;
                    gLoop++;
                }
                if(best.getBestTime()==hrs[hrsInd]){break;}
                if(epanalipseis>=5000){break;}
            }
            globalPheromoneUpdate();

            applyEvaporation();

            //printPheromone();

            if(best.getBestTime()==hrs[hrsInd]){break;}
            if(epanalipseis>=5000){break;}
        }

        printBestSolution();

        createExcel();

        clock_t end= clock();
        double time_spent = (double)(end- clockLoop)/ CLOCKS_PER_SEC;

        cout<<"Execution Time: "<<time_spent<<" sec\nFile: "<<filename<<"\nEpanalipseis: "<<epanalipseis<<endl;

        hrsInd++;
    }
    clock_t end= clock();
    double time_spent = (double)(end- start)/ CLOCKS_PER_SEC;
    cout<<"\nTotal Time: "<<time_spent<<" sec"<<endl;

    return 0;
}