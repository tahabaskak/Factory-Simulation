#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

struct factoryStage {
    char deptName[255];
    int *deptNumber;
    int *timeDept;
    int *doneCarCounter;
    bool *isEmpty;                  /* if factoryStage is null, Processable */
    struct carList *currentProcCar; /* access momentary car */
    struct carList *doneCarList;    /* access processed car list */
    struct factoryStage *next;
};

struct carList {
    char carModel[255];
    char carUniqeCode[10];
    int *carStartTime;
    int *queneInsertTime;   /* this control operation time for car */
    int *startProc;         /* priority car */
    int *orgStartTime;      /* this control original start time access */
    struct carList *next;   /* for linked list */
};

struct carReport{
    char deptName[255];        /* this linked list for report operation */
    char carUniqeCode[10];
    int *queneInsertTime;
    int *timeDept;
    struct carReport *next;
};

struct departList{              /* this linked list uses report car for percentage and not completed stage  */
    char deptName[255];
    int *timeDept;
    struct departList *next;
};

void printFactory(struct factoryStage *startStage);
void reportCars(struct factoryStage *startStage,struct carList *startCar,int reportTime, char *reportType, char *carId,struct departList *startDepart);

int main(int argc, char* argv[])
{
    FILE *inputFile;
    char *inputFileName = argv[1];
    char line[256];
    char *command;
    char *deptName,*carModel,*carUniqeCode,*reportString,*reportCarID;
    int i,stageNumber,*timeDept,*carStartTime,*reportTime;
    struct factoryStage *temp,*startStage = NULL,*endStage = NULL,*restartFactoryList;
    struct departList *tempDepart,*startDepart = NULL,*endDepart = NULL;
    struct carList *tempCar, *startCar = NULL,*endCar = NULL, *restartCarList;


    inputFile = fopen(inputFileName, "r");
    if(!inputFile){
        printf("File not Found.");
        exit(1);
    }

    while(fgets(line,sizeof(line),inputFile)){
        command = strtok(line," ");
        if(strcmp(command,"AddDept")==0){
            stageNumber = atoi(strtok(NULL," \n"));
            deptName = strtok(NULL," \n");
            timeDept = atoi(strtok(NULL," \n"));
            tempDepart =  (struct departList *) malloc(sizeof(struct departList));
            tempDepart->timeDept = timeDept;
            strcpy(tempDepart->deptName,deptName);
            if(startDepart == NULL) {
                startDepart = tempDepart;
                endDepart   = tempDepart;
            }
            else {
                endDepart->next = tempDepart;
                endDepart = tempDepart;
            }

            for(i=0;i<stageNumber;i++){
                temp = (struct factoryStage *) malloc(sizeof(struct factoryStage));
                strcpy(temp->deptName,deptName);
                temp->deptNumber = (i+1);
                temp->timeDept = timeDept;
                temp->isEmpty = true;
                temp->currentProcCar = NULL;
                temp->doneCarList = NULL;
                temp->doneCarCounter =0;
                if(startStage == NULL) {
                    startStage = temp;
                    endStage   = temp;
                }
                else {
                    endStage->next = temp;
                    endStage = temp;
                }
            }
            printf("Department %s has been created.\n");
        }   /*End AddDept*/
        else if(strstr(command,"PrintFactory")){
            printFactory(startStage);
        }   /*End PrintFactory*/
        else if(strcmp(command,"Produce")==0){
            carStartTime = atoi(strtok(NULL," \n"));
            carModel = strtok(NULL," \n");
            carUniqeCode = strtok(NULL," \n");
            tempCar = (struct carList *) malloc(sizeof(struct carList));
            strcpy(tempCar->carModel,carModel);
            strcpy(tempCar->carUniqeCode,carUniqeCode);
            tempCar->startProc = 0;
            tempCar->carStartTime = carStartTime;
            tempCar->orgStartTime = carStartTime;
            tempCar->queneInsertTime =carStartTime;
            if(startCar == NULL) {
                startCar = tempCar;
                endCar   = tempCar;
            }
            else {
                endCar->next = tempCar;
                endCar = tempCar;
            }
        }   /*end Produce*/
        else if(strstr(command,"Report")){
            reportTime = atoi(strtok(NULL," "));
            reportString = strtok(NULL," ");
            if(strcmp(reportString,"Car")==0){
                reportCarID = strtok(NULL," \n");
            }
            reportCars(startStage,startCar,reportTime, reportString, reportCarID,startDepart);
            /* The following operation , every report command restarting first statement after report function each other list */
            restartCarList = startCar;
            while(restartCarList != NULL){
                restartCarList->carStartTime = restartCarList->orgStartTime;
                restartCarList->startProc = 0;
                restartCarList = restartCarList->next;
            }

            restartFactoryList = startStage;
            while(restartFactoryList != NULL){
                restartFactoryList->currentProcCar = NULL;
                restartFactoryList->doneCarCounter = 0;
                restartFactoryList->doneCarList = NULL;
                restartFactoryList->isEmpty = true;
                restartFactoryList = restartFactoryList->next;
            }

        }   /*End Report*/
    }/*End While for Line Reading*/
}

void reportCars(struct factoryStage *startStage,struct carList *startCar,int reportTime, char *reportType, char *carId,struct departList *startDepart){
    int i, tabCount = 0, FoundNextDepartment = 0, isAssigmentHave = 0,number=0;
    struct factoryStage *tempFactory = startStage;
    struct factoryStage *addDepartmentFactory = startStage, *findNextDepartment;
    struct carList *tempCar = startCar, *forDoneCar, *controlCar,*forDoneStart=NULL,*forDoneEnd=NULL, *loopCarListOne, *loopCarListTwo,
    *loopCarListThree;
    struct carReport *tempCarReport,*startCarReport = NULL,*endCarReport = NULL, *loopCarReport;
    struct departList * loopDepart;
    char *tempDepartName[255];
    int lengthString=0,lengthCarModel=0,lengthCarUniqCode=0,minusCounter=0, lengthDepartmentName=0;


    for(i=1 ; i<=reportTime ;i++){
        tempFactory = startStage;
        while(tempFactory != NULL){
            if(tempFactory->isEmpty == false){
                controlCar = tempFactory->currentProcCar;
                number = controlCar->carStartTime;
                /* This operation if carStartTime greater factoryStage timeDept*/
                if( (i - number ) >= tempFactory->timeDept){
                    if(tempFactory ->next == NULL){
                        /* create loopCarList and this list for doneCarList so this CarList use processed car for department */
                        loopCarListOne = tempFactory->doneCarList;
                        loopCarListTwo = (struct carList *) malloc(sizeof(struct carList));
                        strcpy(loopCarListTwo->carModel,controlCar->carModel);
                        strcpy(loopCarListTwo->carUniqeCode,controlCar->carUniqeCode);
                        loopCarListTwo->next = NULL;

                        if(loopCarListOne == NULL){
                            tempFactory->doneCarList = loopCarListTwo;
                        }
                        else{
                            loopCarListThree = tempFactory->doneCarList;
                            while(loopCarListThree->next != NULL){
                                loopCarListThree = loopCarListThree->next;
                            }
                            loopCarListThree->next = loopCarListTwo;
                        }
                        /* tempFactory stage again is empty and currentProcCar is null and isAssigmentHave is true*/
                        tempFactory->doneCarCounter++;
                        tempFactory->isEmpty = true;
                        tempFactory->currentProcCar = NULL;
                        isAssigmentHave = 1;
                        break;

                    }else{
                        findNextDepartment = tempFactory->next;
                        FoundNextDepartment = 0;
                        while(true){
                            if(findNextDepartment == NULL){
                                /* create loopCarList and this list for doneCarList so this CarList use processed car for department */
                                loopCarListOne = tempFactory->doneCarList;
                                loopCarListTwo = (struct carList *) malloc(sizeof(struct carList));
                                strcpy(loopCarListTwo->carModel,controlCar->carModel);
                                strcpy(loopCarListTwo->carUniqeCode,controlCar->carUniqeCode);
                                loopCarListTwo->next = NULL;

                                if(loopCarListOne == NULL){
                                    tempFactory->doneCarList = loopCarListTwo;
                                }
                                else{
                                    loopCarListThree = tempFactory->doneCarList;
                                    while(loopCarListThree->next != NULL){
                                        loopCarListThree = loopCarListThree->next;
                                    }
                                    loopCarListThree->next = loopCarListTwo;
                                }
                                /* tempFactory stage again is empty and currentProcCar is null and isAssigmentHave is true*/
                                tempFactory->doneCarCounter++;
                                tempFactory->isEmpty = true;
                                tempFactory->currentProcCar = NULL;
                                isAssigmentHave = 1;
                                break;
                            }
                            if(strcmp(tempFactory->deptName, findNextDepartment->deptName)!= 0){
                                if(FoundNextDepartment == 0){
                                    /* If next department not find , temp depart name equal next depart name*/
                                    strcpy(tempDepartName, findNextDepartment->deptName);
                                }

                                if(strcmp(tempDepartName, findNextDepartment->deptName) == 0){
                                    if(findNextDepartment->isEmpty == true){
                                        /* findNextDepartment is empty , so changing the following information , nextDepartment is not empty */
                                        findNextDepartment->isEmpty = false;
                                        controlCar->carStartTime = i;
                                        findNextDepartment->currentProcCar = controlCar;
                                        /* tempFactory stage again is empty and currentProcCar is null and isAssigmentHave is true*/
                                        tempFactory->doneCarCounter++;
                                        tempFactory->isEmpty = true;
                                        tempFactory->currentProcCar = NULL;
                                        isAssigmentHave = 1;
                                            /* Adding report List */
                                        tempCarReport = (struct carReport *) malloc(sizeof(struct carReport));
                                        strcpy(tempCarReport->deptName,findNextDepartment->deptName);
                                        strcpy(tempCarReport->carUniqeCode,controlCar->carUniqeCode);
                                        tempCarReport->queneInsertTime = i;
                                        tempCarReport->timeDept = findNextDepartment->timeDept;
                                        if(startCarReport == NULL) {
                                            startCarReport = tempCarReport;
                                            endCarReport   = tempCarReport;
                                        }
                                        else {
                                            endCarReport->next = tempCarReport;
                                            endCarReport = tempCarReport;
                                        }
                                        /* create loopCarList and this list for doneCarList so this CarList use processed car for department */
                                        loopCarListOne = tempFactory->doneCarList;
                                        loopCarListTwo = (struct carList *) malloc(sizeof(struct carList));
                                        strcpy(loopCarListTwo->carModel,controlCar->carModel);
                                        strcpy(loopCarListTwo->carUniqeCode,controlCar->carUniqeCode);
                                        loopCarListTwo->next = NULL;

                                        if(loopCarListOne == NULL){
                                            tempFactory->doneCarList = loopCarListTwo;
                                        }
                                        else{
                                            loopCarListThree = tempFactory->doneCarList;
                                            while(loopCarListThree->next != NULL){
                                                loopCarListThree = loopCarListThree->next;
                                            }
                                        loopCarListThree->next = loopCarListTwo;
                                        }
                                        break;
                                    }
                                }
                                else{
                                    break;
                                }
                                FoundNextDepartment++;
                            }
                            findNextDepartment = findNextDepartment->next;
                        }
                    }
                }
            }
            if(isAssigmentHave == 0){
                /* If isAssigmentHave is zero , next tempFactory  */
                tempFactory = tempFactory->next;
            }
            else{
                /* Else , tempFactory restarting first statement */
                isAssigmentHave = 0;
                tempFactory = startStage;
            }

        }

        tempCar = startCar;
        /*Add Car For Department*/
        while(tempCar != NULL){

            /*Find Cars that is operating*/
            if(tempCar->carStartTime <= i && tempCar->startProc==0){
                addDepartmentFactory = startStage;
                while(true){
                    /*Find start time*/
                    if(strcmp(startStage->deptName,addDepartmentFactory->deptName)==0){
                        /*finding department after take car in department and break operation*/
                        if(addDepartmentFactory->isEmpty == true){
                            /* if addDepartmentFactory is empty , so tempCar and tempCarReport changing information  */
                            tempCar->carStartTime = i;
                            addDepartmentFactory->currentProcCar = tempCar;
                            addDepartmentFactory->isEmpty = false;
                            tempCar->startProc = 1 + 1;
                            tempCarReport = (struct carReport *) malloc(sizeof(struct carReport));
                            strcpy(tempCarReport->deptName,addDepartmentFactory->deptName);
                            strcpy(tempCarReport->carUniqeCode,tempCar->carUniqeCode);
                            tempCarReport->queneInsertTime = i;
                            tempCarReport->timeDept = addDepartmentFactory->timeDept;
                            if(startCarReport == NULL) {
                                startCarReport = tempCarReport;
                                endCarReport   = tempCarReport;
                            }
                            else {
                                endCarReport->next = tempCarReport;
                                endCarReport = tempCarReport;
                            }

                            break;
                            }
                    }
                    else{
                        break;
                    }
                    addDepartmentFactory = addDepartmentFactory->next;
                }
            }
            tempCar = tempCar->next;
        }
    }

    if(strcmp(reportType,"Car")==0){
        bool isFounded = false;
        bool isFoundedDepart = false;
        bool isFoundedComletedDepartment = false;
        bool isNotComplete = false;
        int numberFindQueueTime=0;
        struct factoryStage *tempPrintFactory;
        forDoneCar = startCar;
        int totalDepartTime = 0;
        int passedDepartTime = 0;
        double percentage = 0;

        loopDepart = startDepart;
        while(loopDepart != NULL){
                /* This loop find timeDept for depart and adding totalDepartTime, so percentage finding with totalDepartTime*/
            numberFindQueueTime = loopDepart->timeDept;
            totalDepartTime = totalDepartTime + numberFindQueueTime;
            loopDepart = loopDepart->next;
        }

        printf("Command: Report Car %d %s\n",reportTime,carId);

        while(forDoneCar != NULL){
                /* All returning done car for report */
            if(strcmp(forDoneCar->carUniqeCode,carId)==0){
                lengthString = strlen("|Report for|");
                lengthCarModel = strlen(forDoneCar->carModel);
                lengthCarUniqCode = strlen(forDoneCar->carUniqeCode);
                for(minusCounter =0;minusCounter< (lengthCarModel+lengthCarUniqCode+lengthString +2);minusCounter++)
                    printf("-");
                printf("\n");

                printf("|Report for %s %s|\n", forDoneCar->carModel, forDoneCar->carUniqeCode);


                for(minusCounter =0;minusCounter< (lengthCarModel+lengthCarUniqCode+lengthString +2);minusCounter++)
                    printf("-");
                printf("\n");

                passedDepartTime = 0;
                isFounded = false;
                isFoundedDepart = false;
                loopCarReport = startCarReport;
                while(loopCarReport != NULL){
                    if(strcmp(loopCarReport->carUniqeCode, forDoneCar->carUniqeCode) == 0){
                        /* if car include done car list, so founded is true */
                        isFounded = true;
                        numberFindQueueTime = loopCarReport->queneInsertTime;
                        if((i -numberFindQueueTime) >= loopCarReport ->timeDept){
                            /* if car completed factory stage , writing factory stage name and timeDept */
                            printf("%s:%d, ", loopCarReport->deptName, loopCarReport->timeDept);
                            numberFindQueueTime= loopCarReport ->timeDept;
                            passedDepartTime = passedDepartTime + numberFindQueueTime;
                        }
                        else{
                            isNotComplete = true;
                            /* else car not completed factory stage, writing remaining time this stage and name */
                            printf("%s:%d, ", loopCarReport->deptName, (i -numberFindQueueTime));
                            passedDepartTime = passedDepartTime + i -numberFindQueueTime;
                            loopDepart = startDepart;
                            while(loopDepart != NULL){
                                if(isFoundedDepart == true){
                                    /* if car not completed all factory stage, writing not completed stage */
                                    printf("%s:0, ", loopDepart->deptName);
                                }
                                if(strcmp(loopDepart->deptName, loopCarReport->deptName) == 0){
                                    /* Finding factory stage for report car and isFoundedDepart is true */
                                    isFoundedDepart = true;
                                }
                                loopDepart = loopDepart->next;
                            }
                        }
                    }
                    loopCarReport = loopCarReport->next;
                }

                if(isFounded == false){
                    loopDepart = startDepart;
                    while(loopDepart != NULL){
                        /* If car don't enter other department, so other department not complete. */
                        printf("%s:0, ", loopDepart->deptName);
                        loopDepart = loopDepart->next;
                    }
                }


                percentage = (double)(100 * passedDepartTime) / (double)totalDepartTime;

                if(isFounded == true && isNotComplete == false && percentage < 100){

                loopDepart = startDepart;
                while(loopDepart != NULL){
                        loopCarReport = startCarReport;
                        while(loopCarReport != NULL){
                            if(strcmp(loopCarReport->carUniqeCode, forDoneCar->carUniqeCode) == 0){

                                if(strcmp(loopDepart->deptName, loopCarReport->deptName)== 0){
                                    isFoundedComletedDepartment = true;
                                    break;
                                }
                            }
                            loopCarReport = loopCarReport->next;
                        }
                        if(isFoundedComletedDepartment == false){
                                    printf("%s:0, ", loopDepart->deptName);
                        }
                        isFoundedComletedDepartment = false;
                    loopDepart = loopDepart->next;
                }
            }

                if(percentage >= 100){
                    /* if percentage is %100, so car operation completed. */
                    printf("| Start Time: %d | Complete:%.0f%% | Complete \n", forDoneCar->orgStartTime,percentage);
                }
                else{
                    /* Else , so car can't operation and writing percentage */
                    printf("| Start Time: %d | Complete:%.2f%% | Not complete \n", forDoneCar->orgStartTime,percentage);
                }


            }

            forDoneCar = forDoneCar->next;
        }
        printf("\n");
    }

    else if(strstr(reportType,"Cars")){
        bool isFounded = false;
        bool isFoundedDepart = false;
        bool isFoundedComletedDepartment = false;
        bool isNotComplete = false;
        int numberFindQueueTime=0;
        struct factoryStage *tempPrintFactory;
        forDoneCar = startCar;
        int totalDepartTime = 0;
        int passedDepartTime = 0;
        double percentage = 0;

        loopDepart = startDepart;
        while(loopDepart != NULL){
            /* This loop find timeDept for depart and adding totalDepartTime, so percentage finding with totalDepartTime*/
            numberFindQueueTime = loopDepart->timeDept;
            totalDepartTime = totalDepartTime + numberFindQueueTime;
            loopDepart = loopDepart->next;
        }

        printf("Command: Report Cars %d\n",reportTime);
        while(forDoneCar != NULL){
            /* All returning done car for report */
            lengthString = strlen("|Report for|");
            lengthCarModel = strlen(forDoneCar->carModel);
            lengthCarUniqCode = strlen(forDoneCar->carUniqeCode);
            for(minusCounter =0;minusCounter< (lengthCarModel+lengthCarUniqCode+lengthString +2);minusCounter++)
                printf("-");
            printf("\n");

            printf("|Report for %s %s|\n", forDoneCar->carModel, forDoneCar->carUniqeCode);

            for(minusCounter =0;minusCounter< (lengthCarModel+lengthCarUniqCode+lengthString +2);minusCounter++)
                printf("-");
            printf("\n");

            passedDepartTime = 0;
            isNotComplete = false;
            isFounded = false;
            isFoundedDepart = false;
            loopCarReport = startCarReport;
            while(loopCarReport != NULL){
                if(strcmp(loopCarReport->carUniqeCode, forDoneCar->carUniqeCode) == 0){
                    isFounded = true;
                    /* if car include done car list, so founded is true */
                    numberFindQueueTime = loopCarReport->queneInsertTime;
                    if((i -numberFindQueueTime) >= loopCarReport ->timeDept){
                        /* if car completed factory stage , writing factory stage name and timeDept */
                        printf("%s:%d, ", loopCarReport->deptName, loopCarReport->timeDept);
                        numberFindQueueTime= loopCarReport ->timeDept;
                        passedDepartTime = passedDepartTime + numberFindQueueTime;
                    }else{
                        isNotComplete = true;
                        /* else car not completed factory stage, writing remaining time this stage and name */
                        printf("%s:%d, ", loopCarReport->deptName, (i -numberFindQueueTime));
                        passedDepartTime = passedDepartTime + i -numberFindQueueTime;
                        loopDepart = startDepart;
                        while(loopDepart != NULL){
                            if(isFoundedDepart == true){
                                /* if car not completed all factory stage, writing not completed stage */
                                printf("%s:0, ", loopDepart->deptName);
                            }
                            if(strcmp(loopDepart->deptName, loopCarReport->deptName) == 0){
                                /* Finding factory stage for report car and isFoundedDepart is true */
                                isFoundedDepart = true;
                            }

                            loopDepart = loopDepart->next;
                        }
                    }
                }

                loopCarReport = loopCarReport->next;
            }


            percentage = (double)(100 * passedDepartTime) / (double)totalDepartTime;

            if(isFounded == false){
                loopDepart = startDepart;
                while(loopDepart != NULL){
                    /* If car don't enter other department, so other department not complete. */
                    printf("%s:0, ", loopDepart->deptName);
                    loopDepart = loopDepart->next;
                }
            }
            if(isFounded == true && isNotComplete == false && percentage < 100){

                loopDepart = startDepart;
                while(loopDepart != NULL){
                        loopCarReport = startCarReport;
                        while(loopCarReport != NULL){
                            if(strcmp(loopCarReport->carUniqeCode, forDoneCar->carUniqeCode) == 0){

                                if(strcmp(loopDepart->deptName, loopCarReport->deptName)== 0){
                                    isFoundedComletedDepartment = true;
                                    break;
                                }
                            }
                            loopCarReport = loopCarReport->next;
                        }
                        if(isFoundedComletedDepartment == false){
                                    printf("%s:0, ", loopDepart->deptName);
                        }
                        isFoundedComletedDepartment = false;
                    loopDepart = loopDepart->next;
                }
            }

            if(percentage >= 100){
                /* if percentage is %100, so car operation completed. */
                printf("| Start Time: %d | Complete:%.0f%% | Completed \n", forDoneCar->orgStartTime,percentage);
            }
            else{
                /* Else , so car can't operation and writing percentage */
                printf("| Start Time: %d | Complete:%.2f%% | Not complete \n", forDoneCar->orgStartTime,percentage);
            }

            forDoneCar = forDoneCar->next;
        }
        printf("\n");
    }

    else if(strcmp(reportType,"Department")){
        tempFactory = startStage;
        struct carList *reportDepartDoneCar;

        printf("Command: Report Departments %d \n",reportTime);
        while(tempFactory != NULL){
            /* Rotates all factory stage */
            lengthString = strlen("|Report for Department ""|");
            lengthDepartmentName = strlen(tempFactory->deptName);

            for(minusCounter =0;minusCounter< (lengthString + lengthDepartmentName +4);minusCounter++)
                printf("-");
            printf("\n");
            printf("|Report for Department \"%s %d\"|\n", tempFactory->deptName, tempFactory->deptNumber);
            for(minusCounter =0;minusCounter< (lengthString + lengthDepartmentName +4);minusCounter++)
                printf("-");
            printf("\n");

            if(tempFactory->isEmpty == false){
                /* if factory stage is not empty , processing car in this factory stage */
                controlCar = tempFactory->currentProcCar;
                printf("I am currently processing %s %s\n", controlCar->carModel, controlCar->carUniqeCode);
            }else{
                /* else this factory stage is empty*/
                printf("%s %d is now free.\n",tempFactory->deptName, tempFactory->deptNumber);
            }

            reportDepartDoneCar = tempFactory->doneCarList;

            tabCount = 1;   /*tabCount is processed sorting in this department*/
            if(reportDepartDoneCar != NULL){
                /* If department is not null , writing processed car in this department */
                printf("Processed Cars\n");
                 while(reportDepartDoneCar !=NULL){
                     /*  Rotates all reportDepartDoneCar in this department */
                    printf("%d. %s %s\n", tabCount, reportDepartDoneCar->carModel, reportDepartDoneCar->carUniqeCode);
                    tabCount++;
                    reportDepartDoneCar = reportDepartDoneCar->next;
                 }
            }
            tempFactory = tempFactory->next;
        }
        printf("\n");
    }

}

void printFactory(struct factoryStage *startStage){
    struct factoryStage *temp;
    int tabCount=0,i;
    temp = startStage;
    printf("-");
    while(temp !=NULL){
        /*  Rotates all factory stages */
        printf("%s %d ",temp->deptName,temp->deptNumber);
        if(temp->next != NULL && (strcmp(temp->deptName,temp->next->deptName)!=0)){
                /* That control is When a different factory department arrives */
            printf("\n");
            tabCount++;
            for(i=0;i<tabCount;i++)
                printf("\t");
            printf("-");
        }
        temp=temp->next;
    }
    printf("\n");
}
