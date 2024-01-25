/*
 * helloList.h
 *
 *  Created on: Mar 29, 2015
 *  Updated on: Apr 02, 2015
 *      Author: tsp3859
 */

#ifndef HELLOLIST_H_
#define HELLOLIST_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

#include "boolean.h"
#include "fwdAlgorithmHelper.h"
#include "tierList.h"
#include "endNetworkUtils.h"

extern FILE *fptr;
extern int enableLogScreen;
extern int enableLogFiles;

void update(char inPort[20]);
int find(char inTier[20], char inPort[20]);
boolean containsTierAddress(char testStr[20]);
boolean setByTierPartial(char inTier[20], boolean setFWDFields);
boolean setByTierOnly(char inTier[20], boolean setFWDFields);
boolean setByTierManually(char inTier[20], boolean setFWDFields);
void timestamp();
struct timeval current_time;

extern void getUID(char* curUID,char* currentTier);

extern void modify_LL(char *addr);

struct nodeHL {
	char tier[20];          // tier value
	char port[20];    		//eth port on which neighbor
	struct nodeHL *next;      // next node
}*headHL;

struct stagingNode {         
	char port[20]; 	    // received new msg on interface
	int countHello;		//count of new msg
	double lastUpdate;      // last updated time
	struct stagingNode *next;      // next node
}*stagingHead;

struct stablePort{
	char port[20];			//interface which is stable
	double lastUpdate;      // last updated time
	struct stablePort *next;
}*headStable;


/**s
 * append()
 *
 * method to add node after previous node, called by insert()
 *
 * @param inTier (char[]) - tier value
 * @param inPort (char[]) - interface value
 *
 */

void append(char inTier[20], char inPort[20]) {

	struct nodeHL *temp, *right;
	temp = (struct nodeHL *) malloc(sizeof(struct nodeHL));

	strcpy(temp->tier, inTier);
	strcpy(temp->port, inPort);
	// gettimeofday(&current_time , NULL);
	// temp->lastUpdate = ((double)current_time.tv_sec*1000000 + (double)current_time.tv_usec)/1000000;
	
	right = (struct nodeHL *) headHL;
	while (right->next != NULL)
		right = right->next;
	right->next = temp;
	right = temp;
	
	right->next = NULL;

	//portTag struct update and lisnked list
	//port_tag : a tag (int) which can identify the port 
		//tag = 1 : Neighbor port
		//tag = 2 : to_child from parent port
		//tag = 3 : to_parent from child port

	int match_Port = matchPort(inPort);
	if(match_Port==0){
		tagport(inPort,1);
	}
	printf("\nTEST: Node appended successfully %s\n", temp->tier);

}


boolean checkIfSubstring(char* add1 , char* add2){

 	int posAdd1 = 0;
 	int posAdd2 = 0;
 	int val1 = 0;
 	int val2 = 0;
 	
 	if(enableLogScreen)
 		printf("\n %s Compare : label1 = %s and label2 = %s \n",__FUNCTION__,add1,add2);   

	while(add1[posAdd1++] != '.');
 	while(add2[posAdd2++] != '.');

 	while( (add1[posAdd1] != '\0') && (add2[posAdd2] != '\0'))
 	{
		// printf("\n posAdd1 = %d  \n",posAdd1);  
 		// printf("\n add1[posAdd1] =%c",add1[posAdd1]);
		while( (add1[posAdd1] != '.') && (add1[posAdd1] != '\0'))
 		{

 			// printf("\n add1[posAdd1] = %c  posAdd1=%d\n",add1[posAdd1],posAdd1);   
			val1 = (val1 * 10 )+  add1[posAdd1] - '0' ;
			posAdd1++;
 		}
	
	 	// printf("\n posAdd2 = %d \n",posAdd2);  
 		// printf("\n add2[posAdd2] =%c",add2[posAdd2]);
 		while( (add2[posAdd2] != '.') && (add2[posAdd2] != '\0'))
 		{
 			// printf("\n add2[posAdd2] = %c  posAdd2=%d\n",add2[posAdd2],posAdd2);   
 			val2 = (val2 * 10 )+  add2[posAdd2] - '0' ;
			posAdd2++;
 		}
 		//printf("\nval1 = %d val2 = %d\n",val1,val2);
 		// printf("\n  add1[posAdd1] =%c add2[posAdd2] =%c",add1[posAdd1],add2[posAdd2]);
 		if(val1 != val2)
 		{
 			if(enableLogScreen)
 				printf("\n%s :No substring match found in between (%s) and (%s)\n",__FUNCTION__, add1, add2);

 			return false;
 		}

		if(add1[posAdd1] == '\0' || add2[posAdd2] == '\0')
			break;
		posAdd1++;
		posAdd2++;
 		val1 = val2 = 0;
 	} 
 	if(enableLogScreen)
 		printf("\n%s :Exit- Matched substring: (%s) and (%s)\n",__FUNCTION__, add1, add2);

 	return true;
 }

/*Similar to checkIfSubstring but without printf statements, used to update the Lable list when an interface goes down or up
checkIfSubstring is used in forward algorithm, the print statements are useful for forward algorithm analysis*/
 boolean checkIfSubstr(char* add1 , char* add2){

 	int posAdd1 = 0;
 	int posAdd2 = 0;
 	int val1 = 0;
 	int val2 = 0;
 	
 	// printf("\n Add1 = %d  \n",strlen(add1));
	// printf("\n Add2 = %d  \n",strlen(add2)); 
	// if(strlen(add1)>strlen(add2))
	// {
	// 	return false;
	// }  

	while(add1[posAdd1++] != '.');
 	while(add2[posAdd2++] != '.');

 	while( (add1[posAdd1] != '\0') && (add2[posAdd2] != '\0'))
 	{
		// printf("\n posAdd1 = %d  \n",posAdd1);
		// printf("\n posAdd2 = %d  \n",posAdd2);  
 		// printf("\n add1[posAdd1] =%c",add1[posAdd1]);
		//  printf("\n add2[posAdd2] =%c",add2[posAdd2]);
		while( (add1[posAdd1] != '.') && (add1[posAdd1] != '\0'))
 		{

 			// printf("\n add1[posAdd1] = %c  posAdd1=%d\n",add1[posAdd1],posAdd1);   
			val1 = (val1 * 10 )+  add1[posAdd1] - '0' ;
			posAdd1++;
 		}
	
	 	// printf("\n posAdd2 = %d \n",posAdd2);  
 		// printf("\n add2[posAdd2] =%c",add2[posAdd2]);
 		while( (add2[posAdd2] != '.') && (add2[posAdd2] != '\0'))
 		{
 			// printf("\n add2[posAdd2] = %c  posAdd2=%d\n",add2[posAdd2],posAdd2);   
 			val2 = (val2 * 10 )+  add2[posAdd2] - '0' ;
			posAdd2++;
 		}
 		//printf("\nval1 = %d val2 = %d\n",val1,val2);
 		// printf("\n  add1[posAdd1] =%c add2[posAdd2] =%c",add1[posAdd1],add2[posAdd2]);
 		if(val1 != val2)
 		{
 			

 			return false;
 		}

		if(add1[posAdd1] == '\0' || add2[posAdd2] == '\0')
			break;
		posAdd1++;
		posAdd2++;
 		val1 = val2 = 0;
 	} 
 	

 	return true;
 }


  boolean checkIfGrandParent(char* add1 , char* add2){

 	int posAdd1 = 0;
 	int posAdd2 = 0;
 	int val1 = 0;
 	int val2 = 0;
 	
 	// printf("\n Add1 = %d  \n",strlen(add1));
	// printf("\n Add2 = %d  \n",strlen(add2)); 
	// if(strlen(add1)>strlen(add2))
	// {
	// 	return false;
	// }  
	int count=0;
	while(add1[posAdd1++] != '.' && count<2){
		count++;
	}
 	while(add2[posAdd2++] != '.');

 	while( (add1[posAdd1] != '\0') && (add2[posAdd2] != '\0'))
 	{
		// printf("\n posAdd1 = %d  \n",posAdd1);
		// printf("\n posAdd2 = %d  \n",posAdd2);  
 		// printf("\n add1[posAdd1] =%c",add1[posAdd1]);
		//  printf("\n add2[posAdd2] =%c",add2[posAdd2]);
		while( (add1[posAdd1] != '.') && (add1[posAdd1] != '\0'))
 		{

 			// printf("\n add1[posAdd1] = %c  posAdd1=%d\n",add1[posAdd1],posAdd1);   
			val1 = (val1 * 10 )+  add1[posAdd1] - '0' ;
			posAdd1++;
 		}
	
	 	// printf("\n posAdd2 = %d \n",posAdd2);  
 		// printf("\n add2[posAdd2] =%c",add2[posAdd2]);
 		while( (add2[posAdd2] != '.') && (add2[posAdd2] != '\0'))
 		{
 			// printf("\n add2[posAdd2] = %c  posAdd2=%d\n",add2[posAdd2],posAdd2);   
 			val2 = (val2 * 10 )+  add2[posAdd2] - '0' ;
			posAdd2++;
 		}
 		//printf("\nval1 = %d val2 = %d\n",val1,val2);
 		// printf("\n  add1[posAdd1] =%c add2[posAdd2] =%c",add1[posAdd1],add2[posAdd2]);
 		if(val1 != val2)
 		{
 			

 			return false;
 		}

		if(add1[posAdd1] == '\0' || add2[posAdd2] == '\0')
			break;
		posAdd1++;
		posAdd2++;
 		val1 = val2 = 0;
 	} 
 	

 	return true;
 }



 void deleteList()
{
   /* deref headaddr to get the real head */
   struct addr_list* prev = headaddr;

  while (headaddr)
    {
        headaddr = headaddr->next;
        printf("Deleting from response temporary list %s\n", prev->label);
        free(prev);
        prev = headaddr;
    }
	//printf("\nDeleted.. \n");
	return;
}

/**
 * add()
 *
 * method to add node after previous node, called by insert()
 *
 * @param inTier (char[]) - tier value
 * @param inPort (char[]) - interface value
 *
 */

void add(char inTier[20], char inPort[20]) {

	struct nodeHL *temp;
		//samruddhi
	struct portTag *temp2;
	temp = (struct nodeHL *) malloc(sizeof(struct nodeHL));
	strcpy(temp->tier, inTier);
	strcpy(temp->port, inPort);
	// gettimeofday(&current_time , NULL);
	// temp->lastUpdate = ((double)current_time.tv_sec*1000000 + (double)current_time.tv_usec)/1000000;

	if (headHL == NULL) {
		headHL = temp;
		headHL->next = NULL;
	} else {
		temp->next = headHL;
		headHL = temp;
	}


	int match_Port = matchPort(inPort);
	if(match_Port==0){
		tagport(inPort,1);
	}
	printf("\nTEST: Node added successfully %s\n", temp->tier);
}




/**
 * printNeighbourTable()
 *
 * print the neighbour table
 *
 * @return void
 extern void printNeighbourTable() {
 */

 
void printNeighbourTable() {

	struct nodeHL *fNode = headHL;
	char* temp;
	if (fNode == NULL) {
		if(enableLogScreen)
			printf("ERROR: Neighbor List is empty (Isolated Node)\n");
		return;
	}
	// traverse the list
	// testing
	if(enableLogScreen)
		printf("\n*************** Neighbor Table *************\n");
	while (fNode->next != NULL) {
		temp  = fNode->tier;		
		if(enableLogScreen)
			printf(" ------- %s -----\n",temp);
		fNode = fNode->next;
	}
	printf(" ------- %s -----\n",fNode->tier);
	return;
}
 
 int staging(char inPort[20]){
 	//Staging part - Samruddhi

	int checkstableport = findStable(inPort);
	
	int stable=0;
	if(checkstableport == 1){
		// Label is not in NT 
		//printf("Port is not stable %s",inPort);
		int checkStaging = findStaging(inPort);

		if(checkStaging==1){
			//Label is not in staging
			//printf("FindStagingNegative");
			
		
			struct stagingNode*temp2;
			temp2 = malloc(sizeof(struct stagingNode));

			strcpy(temp2->port, inPort);
			gettimeofday(&current_time , NULL);
			temp2->lastUpdate = ((double)current_time.tv_sec*1000000 + (double)current_time.tv_usec)/1000000;
			temp2->countHello=1;
			printf("TEST: Message Counter %d\n", temp2->countHello);
			// if(enableLogScreen)
			//printf("\nCount : %d",temp2->countHello);
			// if(enableLogScreen)
			//printf("\nPort : %s",temp2->port);

			if (stagingHead == NULL) {
				stagingHead = temp2;
				stagingHead->next = NULL;
			} 	else {
				temp2->next = stagingHead;
				stagingHead = temp2;
			}
		} else{
			//Label is in stagging
			//printf("\nIn updateStaging %s",inPort);
			stable = updateStaging(inPort);
			
		}

	}
	else {
			//printf("Port is stable %s \n",inPort);
			stable=1;
		}


	return stable;	
}

 /**
 * insert()
 *
 * method to add node into a list (duplicate entry-safe)
 *
 * @param inTier (char[]) - tier value
 * @param inPort (char[]) - interface value
 *
 * @return isEntryNew
 */

int insert(char inTier[20], char inPort[20]) {

	struct nodeHL *temp;
	temp = headHL;

	int isEntryNew = 0;

	if (temp == NULL) {

		add(inTier, inPort);
		isEntryNew = 1;
		printNeighbourTable();
		printMyLabels();
		

	} else {

		int checkNode = find(inTier, inPort);
		//printf("checkNode %d\n",checkNode);

		if (checkNode == 1) { // The entry is not present in the table, so append it to the table.
			append(inTier, inPort);
			isEntryNew = 1;
			printNeighbourTable();
			printMyLabels();
		} 
		else {
			//printf("going to update\n");
			update(inPort);
			
		}

	}

	return isEntryNew;
}

/**
 * find()
 *
 * method to check whether a node is present or not in list
 *
 * @param inTier (char[]) - tier value
 * @param inPort (char[]) - interface value
 *
 * @return returnVal (int) - 0 for present otherwise 1
 */
int find(char inTier[20], char inPort[20]) {

	int returnVal = 1;

	struct nodeHL *fNode = headHL;

	// traverse the list
	// testing
	while (fNode != NULL) {
		//while (fNode->next != NULL) {

		// Target Node
		// Length Check
		// Value check

		if (strlen(fNode->tier) == strlen(inTier)) {
			if (strncmp(fNode->tier, inTier, strlen(inTier)) == 0) {

				if (strlen(fNode->port) == strlen(inPort)) {

					//}
					if (strncmp(fNode->port, inPort, strlen(inPort)) == 0) {
						//printf("We have : %s, %s\n",fNode->port,inPort);
						returnVal = 0;
						break;
					}
				}

			}

		}

		fNode = fNode->next;
	}
	//printf("returnVal %d\n",returnVal);
	return returnVal;
}


int findStaging(char inPort[20]) {
	

	int returnVal = 1;

		struct stagingNode *fNode = stagingHead;
	
	// traverse the list
	// testing
	while (fNode != NULL) {


				if (strlen(fNode->port) == strlen(inPort)) {

					if (strncmp(fNode->port, inPort, strlen(inPort)) == 0) {
						//printf("FindStagingPositive");
						returnVal = 0;
						break;
					}
				}


		fNode = fNode->next;
	}
	//printf("\nreturn val : %d",returnVal);
	return returnVal;
}

int findStable(char inPort[20]) {
	

	int returnVal = 1;

		struct stablePort *fNode = headStable;
	
	// traverse the list
	// testing
	while (fNode != NULL) {


				if (strlen(fNode->port) == strlen(inPort)) {

					if (strncmp(fNode->port, inPort, strlen(inPort)) == 0) {
						//printf("FindStagingPositive");
						returnVal = 0;
						break;
					}
				}


		fNode = fNode->next;
	}
	//printf("\nreturn val : %d",returnVal);
	return returnVal;
}


void deleteStaging(char inPort[20]) {

		struct stagingNode *fNode = stagingHead;
		struct stagingNode *prev1 = stagingHead;
	// traverse the list
	// testing
	while (fNode != NULL) {

				if (strlen(fNode->port) == strlen(inPort)) {

					if (strncmp(fNode->port, inPort, strlen(inPort)) == 0) {
						//printf("FindStagingPositive");
						//pendingsam
						if (fNode == stagingHead) {
							printf("\nTEST: Head node removed from stagging value was %s\n", fNode->port);
							stagingHead = fNode->next;
							//free(temp);
						} else {
							prev1->next = fNode->next;
							printf("\nTEST: other node removed from stagging value was %s\n", fNode->port);
							//free(temp);
						}
					}
				}
				 else{
				prev1 = fNode;
		} 

		fNode = fNode->next;
	}

	// printf("\nStaging List :");
	// while (fNode != NULL) {
	// 	printf("\n %s",fNode->port);
	// 	fNode = fNode->next;
	// }


}

void deleteStable(char inPort[20]) {

		struct stablePort *fNode = headStable;
		struct stablePort *prev1= headStable;
	// traverse the list
	// testing
	while (fNode != NULL) {

				if (strlen(fNode->port) == strlen(inPort)) {

					if (strncmp(fNode->port, inPort, strlen(inPort)) == 0) {
						//printf("FindStagingPositive");
						//pendingsam
						if (fNode == headStable) {
							printf("TEST: Head stable node removed value was %s\n", fNode->port);
							headStable = fNode->next;
							//free(temp);
						} else {
							prev1->next = fNode->next;
							printf("TEST: other stable node removed value was %s\n", fNode->port);
							//free(temp);
						}
					}
				}
				 else{
				prev1 = fNode;
		} 


		fNode = fNode->next;
	}

}

/**
 * update()
 *
 * method to update the timer information of a node
 *
 * @param inTier (char[]) - tier value
 * @param inPort (char[]) - interface value
 */
void update(char inPort[20]) {
	//printf("Here to update time\n");
	// to be updated
	struct stablePort *uNode = headStable;

	// traverse the list
	// testing
	while (uNode != NULL) {
		//printf("Here to update time2\n");
		//while (uNode->next != NULL) {

		// Target Node
		// Length Check
		// Value check
		// Update timer

		// // if (strlen(uNode->tier) == strlen(inTier)) {
		// // 	if (strncmp(uNode->tier, inTier, strlen(inTier)) == 0) {

		// // 		if (strlen(uNode->port) == strlen(inPort)) {

		// 			//}
		// 			if (strncmp(uNode->tier, inTier, strlen(inTier)) == 0) {
				if (strlen(uNode->port) == strlen(inPort)) {
					//printf("Here to update time3\n");
					if (strncmp(uNode->port, inPort, strlen(inPort)) == 0) {

						gettimeofday(&current_time , NULL);
						uNode->lastUpdate = ((double)current_time.tv_sec*1000000 + (double)current_time.tv_usec)/1000000;
						
						//printf("TEST: lastUpdate updated %s\n", uNode->port);
						//break;
					}
				}
			uNode = uNode->next;
			}

		}



int updateStaging(char inPort[20]) {


	struct stagingNode *fNode = stagingHead;

	// printf("\nStaging List :");
	// while (fNode != NULL) {
	// 	printf("\n %s",fNode->port);
	// 	fNode = fNode->next;
	// }

	struct stagingNode *uNode = stagingHead;

		int stable = 0;
	// traverse the list
	// testing


	while (uNode != NULL) {

				if (strlen(uNode->port) == strlen(inPort)) {

					//}
					if (strncmp(uNode->port, inPort, strlen(inPort)) == 0) {
						gettimeofday(&current_time , NULL);
						uNode->lastUpdate = ((double)current_time.tv_sec*1000000 + (double)current_time.tv_usec)/1000000;
						uNode->countHello = uNode->countHello+1;
						printf("TEST: Message Counter %d\n", uNode->countHello);
						//break;
						
						if(uNode->countHello>=3){
							// if(enableLogScreen)
							// 	printf("\nhello : %d",uNode->countHello);
							// if(enableLogScreen)
						 	 printf("\n MessageCount >= 3 : %s",inPort);
							stable=1;
							//adding to stablePort
							struct stablePort *temp2;
							temp2 = (struct stablePort *) malloc(sizeof(struct stablePort));
							gettimeofday(&current_time , NULL);
							temp2->lastUpdate = ((double)current_time.tv_sec*1000000 + (double)current_time.tv_usec)/1000000;
							strcpy(temp2->port, inPort);

							if (headStable == NULL) {
								//printf("\n Inif ");
								headStable = temp2;
								headStable->next = NULL;
							} 	else {
								//printf("\n Inelse ");
								temp2->next = headStable;
								headStable = temp2;
							}
							// printf("\n Stable Port List : ");
							// while(temp2 != NULL){
							// 	printf("\n %s",temp2->port);
							// 	temp2 = temp2->next;
							// }
							//Removing from staging
							deleteStaging(inPort);

							
						}
					}
				}
						

		// 	}

		// }

		uNode = uNode->next;

	}
	

return stable;

}

int compare(char *addr){
	struct nodeHL *temp = headHL;
	int ret = 0;
	while(temp){
		//printf("temp->tier = %s ,addr = %s",temp->tier,addr);
		if(checkIfSubstr(temp->tier,addr)){
			return 1;
		}
		temp = temp ->next;
	}

	return ret;
}

int deleteMyLabelsRelated(char tier[20]){
	printf("In delete related \n");
	struct nodeTL *temp = headTL;
	int ret = 0;
	while(temp){
		if(checkIfSubstr(temp->tier,tier)){
			modify_LL(temp->tier);
			deleteTierAddr(temp->tier);
			return 1;
		}
		temp = temp ->next;
	}
	return ret;
}

/**
 * delete()
 *
 * method to delete node from neighbor based on timeout mechanism
 *
 * @return status (int) - method return value
 */

int delete() {

	//printf("\ndelete;");
	//struct nodeHL *deletedTierAddr = NULL;
	struct stablePort *temp = headStable;
	struct stablePort *prev = headStable;
	//struct nodeTL *fNode = headTL; // headTL - head of the My label table
	struct stagingNode *temp1 = stagingHead;
	struct stagingNode *prev1 = stagingHead;
	  // headHL is the head of the neighbor label table
	int ret = 0;

	int t = 0;

	while (temp != NULL) {

		gettimeofday(&current_time , NULL);
		double cur_time = ((double)current_time.tv_sec*1000000 + (double)current_time.tv_usec)/1000000;
		double delTimeDiff = cur_time - temp->lastUpdate;
		//printf("TEST: delTimeDiff: %f, port: %s \n", delTimeDiff, temp->port);
		
		
		
		// If last updated local time is more than desired time
		if (delTimeDiff >= 0.40) { //dead timer(from 1.3 to 0.40) reduced 4/20/2022 - Samruddhi
			//printf("TEST: Inside Time diff delete block (>30)\n");
			int match_Port = matchPort(temp->port);
			if(t==0){
				printf("\n%s",temp->port);
				
				if(match_Port==2){
					//if the eth wich failed is to_child then generate label and send to core router - Samruddhi - 03/28/2022
					notify_lostmychild(temp->port);
				}
				printf("\n*****************Interface went down at %ld *****************",time(0));
				timestamp();
				
				t = 1;
			}
				struct nodeHL *temp2 = headHL;//neighbour
				struct nodeHL *prev2 = headHL;

			while(temp2 != NULL){
				if (strlen(temp2->port) == strlen(temp->port)) {

					//}
					if (strncmp(temp2->port, temp->port, strlen(temp->port)) == 0) {


						if (temp2 == headHL) {
							printf("\nTEST862: Head node removed value was %s\n", temp2->tier);
								headHL = temp2->next;

						} else {
							prev2->next = temp2->next;
							printf("\nTEST867: other node removed value was %s\n", temp2->tier);
						}
						printf("\nmatch_Port deleted: : %d",temp2->tier);
						if(match_Port==3){
							//delete all mylabels related to temp2->tier
							deleteMyLabelsRelated(temp2->tier);
						}
						if(match_Port==2){
						
							deleteIPLabel(temp->port);
						}
						ret= 1;
						//}
						

					}else{
						prev2=temp2;
					}

				}
				temp2 = temp2->next;
			}
			deleteStable(temp->port);
			//deleteTag(temp->port);
		} else{
			prev = temp;
			} 
		temp = temp->next;
	}
		//staging time check 
	while (temp1 != NULL) {
		gettimeofday(&current_time , NULL);
		double cur_time = ((double)current_time.tv_sec*1000000 + (double)current_time.tv_usec)/1000000;
		double delTimeDiff = cur_time - temp1->lastUpdate;
		//printf("TEST: delTimeDiff: %f, label: %s, port: %s \n", delTimeDiff, temp->tier, temp->port);

		// If last updated local time is more than desired time
		if (delTimeDiff >= 0.60) {
				//if node to be removed is head
				if (temp1 == stagingHead) {
					//printf("TEST: Head node removed value was %s\n", temp->tier);
					stagingHead = temp1->next;
					//free(temp);
					} else {
						prev1->next = temp1->next;
						//printf("TEST: other node removed value was %s\n", temp->tier);
						//free(temp);
					}
								//}
			} else{
				prev1 = temp1;
		} 
		temp1 = temp1->next;
		// 	// if node to be removed is head
		// 	if (temp == headHL) {
		// 		//printf("TEST: Head node removed value was %s\n", temp->tier);
		// 		headHL = temp->next;

		// 		//free(temp);
		// 		//return 1;
		// 	} else {
		// 		prev->next = temp->next;
		// 		//printf("TEST: other node removed value was %s\n", temp->tier);
		// 		//free(temp);
		// 		//return 1;
		// 	}
		// 	ret = 1;
		// }
		// else{
		// 	prev = temp;
		// }
		// temp = temp->next;

	}
	
	return ret;
}


int tagport(char inPort[20],int tagnum){

	struct portTag*temp2;
	temp2 = (struct portTag *) malloc(sizeof(struct portTag));
	strcpy(temp2->port, inPort);
	temp2->tag= tagnum;

	if(enableLogScreen)
		printf("\nPort : %s",temp2->port);
	if(enableLogScreen)
		printf("\nPort_tag : %d",temp2->tag);

	//linked list of portTag
	if (port_tag == NULL) {
		port_tag = temp2;
		port_tag->next = NULL;
	} else {
		temp2->next = port_tag;
		port_tag = temp2;
	}

}

int matchPort(char inPort[20]) {

	int returnVal = 0;

	struct portTag *temp = port_tag;

	// traverse the list
	// testing
	while (temp != NULL) {
		//while (temp->next != NULL) {

		// Target Node
		// Length Check
		// Value check
	
				if (strlen(temp->port) == strlen(inPort)) {

					//}
					if (strncmp(temp->port, inPort, strlen(inPort)) == 0) {

						returnVal = temp->tag;
						break;

					}

				}
		temp = temp->next;
	}

	return returnVal;
}


/**
 * deleteTag()
 *
 * method to delete node Tag from portTag when node is deleted
 *
 * @return void
 */

void deleteTag(char inPort[20]) {

		struct portTag *fNode = port_tag;
		struct portTag *prev1= port_tag;
	// traverse the list
	// testing
	while (fNode != NULL) {

				if (strlen(fNode->port) == strlen(inPort)) {

					if (strncmp(fNode->port, inPort, strlen(inPort)) == 0) {
						//printf("FindStagingPositive");
						//pendingsam
						if (fNode == headStable) {
							printf("TEST: Head nodetag removed value was for %s\n", fNode->port);
							headStable = fNode->next;
							//free(temp);
						} else {
							prev1->next = fNode->next;
							printf("TEST: other nodetag removed value was for %s\n", fNode->port);
							//free(temp);
						}
					}
				}
				 else{
				prev1 = fNode;
		} 


		fNode = fNode->next;
	}

}

/*Compares each label in the Label list(LL) with the neighbor table, if the substring matches, returns 0, else return 1
return 0 - There is a substring match and the label shouldn't be removed from the LL
return 1 - No substring match was found, indicating the label's interface is down, hence the label should be removed from the LL*/
int compare_NT(char *lab){
	struct nodeHL *temp = headHL;
	while(temp!=NULL){
		if(checkIfSubstr(temp->tier,lab)){
			printf("sub %s : %s\n",temp->tier,lab);
			return 0;
		}
		temp = temp->next;
	}
	return 1;
}

//Identifies and deletes from the label list when an interface to a node is down
int findFailed_LL() {
	struct nodeTL *temp = headTL; //headTL : MyLabels 
	struct nodeTL *prev;
	
	while (temp != NULL) {
		
		if(compare_NT(temp->tier)){
			
			modify_LL(temp->tier);
			deleteTierAddr(temp->tier);
			
			// if(deleted==1){
			// 	printf("Notification delte\n");
				
			// }
		}	
		temp = temp->next;
	}
	
	return 1;
}


/**
 * displayNeighbor()
 *
 * method to print neighbour list entries
 *
 * @param inTier (char[]) - tier value
 *
 */
//void displayNeighbor(struct node *r) {
void displayNeighbor() {

	struct nodeHL *r;
	r = headHL;
	if (r == NULL) {
		//printf("No Neighbors\n");
		return;
	}
	//printf("My Neighbors\n");
	int i = 1;
	while (r != NULL) {
		//printf("Tier Address %d - Length %zd : %s : Port: %s\n", i,
				//strlen(r->tier), r->tier, r->port);
		i = i + 1;
		r = r->next;
	}
	//printf("\n");
}

/**
 * count()
 *
 * method to count number of neighbours
 *
 * @return localCount (int) - count of neighbour nodes
 */
int count() {
	struct nodeHL *n;
	int localCount = 0;
	n = headHL;
	while (n != NULL) {
		n = n->next;
		localCount++;
	}
	return localCount;
}

/**
 * contains(char[])
 *
 * whether there is a tier address in neighbor table or not
 *
 * @return true or false
 */
boolean containsTierAddress(char testStr[20]) {

	boolean check = false;

	struct nodeHL *fNode = headHL;

	if (fNode == NULL) {

		if(enableLogScreen){
			printf("\nERROR: Neighbor List is empty (Isolated Node)\n");
			printf("\nTEST: Before return check %d \n", check);
		}
		if(enableLogFiles){
			fprintf(fptr,"\nERROR: Neighbor List is empty (Isolated Node)\n");
			fprintf(fptr,"\nTEST: Before return check %d \n", check);
		}
		return check;
	}

	// traverse the list
	// testing
	while (fNode != NULL) {
		//while (fNode->next != NULL) {

		if ((strlen(fNode->tier) == strlen(testStr))
				&& ((strncmp(fNode->tier, testStr, strlen(testStr)) == 0))) {
			check = true;
			break;

		} else {
			fNode = fNode->next;
		}

	}
	//printf("TEST: Before return check %d \n", check);
	return check;
}

/**
 * setByTierPartial(char[],boolean)
 *
 * If there is a tier address partial match, it will set forwarding fields
 * Should only be used in an uplink scenario
 *
 * @param inTier       - tier address (partial) to be probed
 * @param setFWDFields - if true will set forwarding fields
 *
 * @return true or false
 */
boolean setByTierPartial(char inTier[20], boolean setFWDFields) {

	//printf("Inside setByTierPartial - helloList.h \n");
	boolean returnVal = false;

	struct nodeHL *fNode = headHL;

	if (fNode == NULL) {

		if(enableLogScreen)
			printf("\nERROR: Failed to set FWD Tier Address (Isolated Node)\n");
		if(enableLogFiles)
			fprintf(fptr,"\nERROR: Failed to set FWD Tier Address (Isolated Node)\n");
		return returnVal;
	}

	// traverse the list
	// testing
	while (fNode != NULL) {
		//while (fNode->next != NULL) {

		// Target Node
		// Length Check
		// Value check

		//	if (strlen(fNode->tier) == strlen(inTier)) {
		if (strncmp(fNode->tier, inTier, strlen(inTier)) == 0) {

			if (setFWDFields == true) {

				fwdTierAddr = (char *) malloc(20);
				memset(fwdTierAddr, '\0', 20);
				strcpy(fwdTierAddr, fNode->tier);

				// set fwd tier port
				fwdInterface = (char *) malloc(20);
				memset(fwdInterface, '\0', 20);
				strcpy(fwdInterface, fNode->port);

				returnVal = true;

			}

			returnVal = true;
			break;

		}

		//	}

		fNode = fNode->next;
	}

	return returnVal;
}

/**
 * setByTierOnly(char[],boolean)
 *
 * If there is a tier address complete match, it will set forwarding fields
 * smart method detects error before forwarding packet
 *
 * @param inTier       - tier address to be probed
 * @param setFWDFields - if true will set forwarding fields
 *
 * @return true or false
 */
boolean setByTierOnly(char inTier[20], boolean setFWDFields) {

	//printf("Inside setByTierOnly - helloList.h \n");
	boolean returnVal = false;

	struct nodeHL *fNode = headHL;

	if (fNode == NULL) {

		if(enableLogScreen)
			printf("ERROR: Failed to set FWD Tier Address (Isolated Node)\n");
		return returnVal;
	}

	// traverse the list
	// testing
	while (fNode != NULL) {
		//while (fNode->next != NULL) {

		// Target Node
		// Length Check
		// Value check

		if ((strlen(fNode->tier) == strlen(inTier))
				&& ((strncmp(fNode->tier, inTier, strlen(inTier)) == 0))) {

			if (setFWDFields == true) {
// NS what is happening with fwdTierAddr and fwdInterface
				fwdTierAddr = (char *) malloc(20);
				memset(fwdTierAddr, '\0', 20);
				strcpy(fwdTierAddr, fNode->tier);

				// set fwd tier port
				fwdInterface = (char *) malloc(20);
				memset(fwdInterface, '\0', 20);
				strcpy(fwdInterface, fNode->port);

				returnVal = true;

			}

			returnVal = true;
			break;

			//	}

		}

		fNode = fNode->next;
	}

	return returnVal;
}

// TODO
/**
 * setByTierManually(char[],boolean)
 *
 * a Manual method to set fwd fields
 * does not perform pre-check
 * AVOID IT
 *
 * Currently Used by:
 *  1. Forwarding Algorithm - Case A
 * @param inTier       - tier address to be set directly
 * @param setFWDFields - if true will set forwarding fields
 *
 * @return true or false
 */
boolean setByTierManually(char inTier[20], boolean setFWDFields) {

	//printf("Inside setByTierManually - helloList.h \n");
	boolean returnVal = false;

	if (setFWDFields == true) {

		// set fwd tier address
		fwdTierAddr = (char *) malloc(20);
		memset(fwdTierAddr, '\0', 20);
		strcpy(fwdTierAddr, inTier);

		// set fwd tier port
		fwdInterface = (char *) malloc(20);
		memset(fwdInterface, '\0', 20);
		strcpy(fwdInterface, "xxx");

		returnVal = true;

	}

	return returnVal;
}





/**
 * findParntLongst(char[],char[])
 *
 * return the longest matching parent adress in the table 
 *
 * @return void

 */
 
 
 
 void findParntLongst(char* myTierAdd,char* parentTierAdd) 
 {
	struct nodeHL *fNode = headHL;  //NEIGHBOR TABLE
	char* temp;
	if (fNode == NULL) {
		if(enableLogScreen)
			printf("\nERROR: Neighbor List is empty (Isolated Node)\n");

	}

	while (fNode != NULL) {
		temp  = fNode->tier;		
		struct nodeTL *fNode1 = headTL;
		while(fNode1 !=NULL) {
			char *temp1 = fNode1->tier;

		if(strlen(temp1) > strlen(temp)){
           if(enableLogScreen)
			printf("findParntLongst Inside first if temp =%s  and temp1 = %s", temp, temp1);
			//struct nodeTL *temp1 = headTL; 

			//int tempLen = findMatchedTeirAddrLength(myTierAdd,temp);
			if(checkIfSubstring(temp1,temp)){
				//longestMtchLength = tempLen;
				if(enableLogScreen)
			     printf("findParntLongst Inside second if temp = %s and temp1 = %s", temp, temp1);
				strcpy(parentTierAdd, temp);
				return;
			}
		}
		fNode1 = fNode1->next;
 		}
		fNode = fNode->next;
	}
	return;
 }

void findParentLongest(char *myTierAdd,char* parentTierAdd) 
 {
	 printf("\nIn findParentLongest\n");
	struct nodeHL *fNode = headHL;  //NEIGHBOR TABLE
	char *temp;
	if (fNode == NULL) {
		if(enableLogScreen)
			printf("\nERROR: Neighbor List is empty (Isolated Node)\n");

	}

	char temp1[20];
	//printf("\nI am here  %s\n", myTierAdd);
	strcpy(temp1,myTierAdd);
	printf("\nFinding parent for %s\n", temp1);
	// struct nodeTL *fNode1 = headTL;
	// printf("my tier %s", fNode1->tier);
//&&(strlen(temp)>strlen(fNode1->tier))
	while (fNode != NULL) {
		temp  = fNode->tier;
		if (strlen(fNode->tier) == strlen(temp1)) {
			if (strncmp(fNode->tier, temp1, strlen(temp1)) == 0) {
				strcpy(parentTierAdd, temp);
				return;
			}
		}
		fNode = fNode->next;
	}
	struct nodeHL *fNode1 = headHL;
	while (fNode1 != NULL) {
		temp  = fNode1->tier; // neighbor		

		if(strlen(temp1) > strlen(temp)){
        	if(enableLogScreen)
				printf("findParentLongest Inside first if temp =%s  and temp1 = %s\n", temp, temp1);
			if(checkIfSubstring(temp1,temp)){
							//longestMtchLength = tempLen;
				if(enableLogScreen)
			     	printf("findParentLongest Inside second if temp = %s and temp1 = %s\n", temp, temp1);
				strcpy(parentTierAdd, temp);
				return;
			}		
		}
		fNode1 = fNode1->next;
	}
	
	return;
 }



/**
 * findChildLongst(char[],char[])
 *
 * return the longest matching child adress in the table 
 *
 * @return void

 */

 //Modified by Supriya, on September 6, 2017.

 void findChildLongst(char* desTierAdd,char* childTierAdd, char* myLabel)
 {
	struct nodeHL *fNode = headHL;
	char* temp;
	if (fNode == NULL) {
		if(enableLogScreen)
			printf("\nERROR: Neighbor List is empty (Isolated Node)\n");

		return;
	}
	
	//initializing the longest matching length to 0
	//int longestMtchLength = 0;
	if(enableLogScreen)
			printf("\n%s: Finding the appropriate child \n",__FUNCTION__);

	while (fNode != NULL) {
		temp  = fNode->tier;		
		// if(enableLogScreen)
		// 	printf("\nfindChildLongst : Current Neighbour = %s \n",temp);
		// if(enableLogFiles)
		// 	fprintf(fptr,"\nfindChildLongst : Current Neighbour = %s \n",temp);
		if(strlen(temp) >= strlen(myLabel)){// new line added by Supriya
			if(strlen(temp) <= strlen(desTierAdd)){

				//int tempLen = findUIDtoDestinationMatch(desTierAdd,temp);
				if(checkIfSubstring(desTierAdd,temp)){
					//ongestMtchLength = tempLen;
					strcpy(childTierAdd, temp);
					// if(enableLogScreen)
					// 	printf("\nfindChildLongst : Result = %s \n",childTierAdd);
					// if(enableLogFiles)
					// 	fprintf(fptr,"\nfindChildLongst : Result = %s \n",childTierAdd);
					return;
				}
			}
		}
		fNode = fNode->next;
	}

	// if(enableLogScreen)
	// 	printf("\n findChildLongst : Result = %s \n",childTierAdd);
	// if(enableLogFiles)
	// 	fprintf(fptr,"\n findChildLongst : Result = %s \n",childTierAdd);
	return;
 }

/**
 * examineNeighbourTable(char[])
 *
 * return the longest matching adress in the table with the destination address
 *
 *type = 1: case 3: check substring only if len_neighbor label < len_destination label
 *type = 2: case 4: check substring only if len_neighbor label <= len_my label and len_neighor label > len_dest label
 *type = 3: case 5: check substring only if len_neighbor label >= len_my label and len_neighor label < len_dest label
 */

int examineNeighbourTable1(char* desTierAdd, char* longstMatchingNgbr,char* myLabel, int type) 
 {
	struct nodeHL *fNode = headHL;  // pointer to neighbor table 
	char* temp;
	if (fNode == NULL) {
		if(enableLogScreen)
			printf("\nERROR: Neighbor List is empty (Isolated Node)\n");
		return 1;
	}

	while (fNode != NULL) {
			temp  = fNode->tier;
			if(strlen(temp) < strlen(desTierAdd)){
				strcpy(longstMatchingNgbr,temp);
				return 0;	
			}
			fNode = fNode->next;
	}

 }

 //modified by Supriya on August 28,2017

 int examineNeighbourTable(char* desTierAdd, char* longstMatchingNgbr) 
 {
 	int retVal = 1; //ERROR / FAILURE
	struct nodeHL *fNode = headHL;  // pointer to neighbor table 
	char* temp;
	if (fNode == NULL) {
		if(enableLogScreen)
			printf("\nERROR: Neighbor List is empty (Isolated Node)\n");

		return retVal;
	}
	
	//initializing the longest matching length to 0
	int longestMtchLength = 0; //changed to 0 from 1 on august 25, 2017
	//int tempLen = 0; 

		while (fNode != NULL) {
			temp  = fNode->tier;	
			if(enableLogScreen){
				//printf("\n%s temp->%s desTierAdd-->%s",__FUNCTION__,temp,desTierAdd);
				printf("\n%s: Check if my neighbor: %s is a substring of destination label(uid): %s",__FUNCTION__,temp,desTierAdd);	
			}

			if(strlen(temp) < strlen(desTierAdd)){
				if(checkIfSubstring(desTierAdd,temp)){
					strcpy(longstMatchingNgbr, temp);
					return 0;
				}
			}

			fNode = fNode->next;
		}
	
	// else {
	// 	if (type == 2){
	// 		while (fNode != NULL) {
	// 			temp  = fNode->tier;	
	// 			if(enableLogScreen){
	// 				//printf("\n%s temp->%s desTierAdd-->%s",__FUNCTION__,temp,desTierAdd);
	// 				printf("\n%s: Check if destination label(uid): %s is a substring of my neighbor: %s",__FUNCTION__,desTierAdd,temp);	
	// 			}

	// 			if(strlen(temp) <= strlen(myLabel)){
	// 				if(strlen(temp) > strlen(desTierAdd)){
	// 					if(checkIfSubstring(desTierAdd,temp)){
	// 						strcpy(longstMatchingNgbr, temp);
	// 						return 0;
	// 					}
	// 				}
	// 			}
	// 			// tempLen = findMatchedTeirAddrLength(desTierAdd,temp);
	// 			// if(enableLogScreen)
	// 			// 	printf("\n %s Matched Length = %d",__FUNCTION__,tempLen);	
	// 			// if(enableLogFiles)
	// 			// 	fprintf(fptr,"\n %s Matched Length = %d",__FUNCTION__,tempLen);	
	// 			// if(tempLen > longestMtchLength){
	// 			// 	longestMtchLength = tempLen;
	// 			// 	strcpy(longstMatchingNgbr, temp);
	// 			// 	retVal = 0;
	// 			// }
	// 			fNode = fNode->next;
	// 		}
	// 	}
	// 	else{
	// 		while (fNode != NULL) {
	// 			temp  = fNode->tier;	
	// 			if(enableLogScreen){
	// 				//printf("\n%s temp->%s desTierAdd-->%s",__FUNCTION__,temp,desTierAdd);
	// 				printf("\n%s: Check if my neighbor: %s is a substring of destination label(uid): %s",__FUNCTION__,desTierAdd, temp);	
	// 			}


	// 			if(strlen(temp) >= strlen(myLabel)){
	// 				if(strlen(temp) < strlen(desTierAdd)){
	// 					if(checkIfSubstring(desTierAdd,temp)){
	// 						strcpy(longstMatchingNgbr, temp);
	// 						return 0;
	// 					}
	// 				}
	// 			}	
				
	// 			fNode = fNode->next;
	// 		}

	// 	}
	// }

	return retVal;
 }


/**
 * findMatchedTeirAddrLength(char[],char[])
 *
 * find the matched length of two tier values
 *
 * @return length (int)
 */



 int findMatchedTeirAddrLength(char* add1 , char* add2){

 	int matchedLength = 0;
 	int posAdd1 = 0;
 	int posAdd2 = 0;
 	int val1 = 0;
 	int val2 = 0;
 	
 	if(enableLogScreen)
 		printf("\n %s Enter : label1 = %s label2 = %s \n",__FUNCTION__,add1,add2);   
 	if(enableLogFiles)
		fprintf(fptr,"\n %s Enter : label1 = %s label2 = %s \n",__FUNCTION__,add1,add2); 
	// skip the tier value of both the addresses


	while(add1[posAdd1++] != '.');
 	while(add2[posAdd2++] != '.');
		
 	// printf("\n posAdd1 = %d posAdd2 = %d \n",posAdd1,posAdd2);  

 	while( (add1[posAdd1] != '\0') && (add2[posAdd2] != '\0'))
 	{
		
 		// printf("\n posAdd1 = %d  \n",posAdd1);  
 		// printf("\n add1[posAdd1] =%c",add1[posAdd1]);
		while( (add1[posAdd1] != '.') && (add1[posAdd1] != '\0'))
 		{

 			// printf("\n add1[posAdd1] = %c  posAdd1=%d\n",add1[posAdd1],posAdd1);   
			val1 = (val1 * 10 )+  add1[posAdd1] - '0' ;
			posAdd1++;
 		}
		posAdd1++;

		
	 	// printf("\n posAdd2 = %d \n",posAdd2);  
 		// printf("\n add2[posAdd2] =%c",add2[posAdd2]);
 		while( (add2[posAdd2] != '.') && (add2[posAdd2] != '\0'))
 		{
 			// printf("\n add2[posAdd2] = %c  posAdd2=%d\n",add2[posAdd2],posAdd2);   
 			val2 = (val2 * 10 )+  add2[posAdd2] - '0' ;
			posAdd2++;
 		}
		posAdd2++;

 		// printf("\nval1 = %d val2 = %d\n",val1,val2);

 		// printf("\n  add1[posAdd1] =%c add2[posAdd2] =%c",add1[posAdd1],add2[posAdd2]);

 		if(val1 == val2)
 		{
 			matchedLength++;
 		}
 		else
 		{
 			break;
 		}
		
		if(add1[posAdd1] == '\0' || add2[posAdd2] == '\0')
			break;

 		val1 = val2 = 0;

 	} 
 	if(enableLogScreen)
 		printf("\n %s :Exit- Matched Length = %d",__FUNCTION__,matchedLength);
 	if(enableLogFiles)
		fprintf(fptr,"\n %s :Exit- Matched Length = %d",__FUNCTION__,matchedLength);
 	return matchedLength;
 }

/**
 * findUIDmatchfromNeighborTable(char[])
 *
 * return the neighbor table entry whose UID matches with the substring of the destination address
 *

 * @return int

 */
int findUIDmatchfromNeighborTable(char* desTierAdd,char* longstMatchingNgbr)
{
	int retVal = 1; //ERROR / FAILURE
	struct nodeHL *fNode = headHL;
	char* temp;
	if (fNode == NULL) {
		if(enableLogScreen)
			printf("\n ERROR: Neighbor List is empty (Isolated Node)\n");
		if(enableLogFiles)
			fprintf(fptr,"\n ERROR: Neighbor List is empty (Isolated Node)\n");
		return retVal;
	}

	//initializing the longest matching length to 0
	int longestMtchLength = 0;
	//int tempLen = 0;

	while (fNode != NULL) {
		temp  = fNode->tier;
		// if(enableLogScreen){
		// 	printf("\n%s temp->%s desTierAdd-->%s",__FUNCTION__,temp,desTierAdd);
		// 	printf("\n %s Checking the match for %s in desTierAdd=%s",__FUNCTION__,temp,desTierAdd);
		// }

		//tempLen = findUIDtoDestinationMatch(desTierAdd,temp);
		//if(enableLogScreen)
		//	printf("\n %s Matched Length = %d",__FUNCTION__,tempLen);

		if(checkIfSubstring(desTierAdd,temp)){
			//longestMtchLength = tempLen;
			strcpy(longstMatchingNgbr, temp);
			return 0;// success
		}
		fNode = fNode->next;
	}

	return retVal;
}


/**
 * findUIDtoDestinationMatch(char[],char[])
 *
 * find whether myUID is a substring of destinationUID (Here add1 is destination).
 *
 * @return length (int)
 */


//Not used anymore August 25, 2017 instead using checkIfSubstring()
int findUIDtoDestinationMatch(char* destAddr , char* neighborAddr){

	int matchedLength = 0;
	matchedLength  = findMatchedTeirAddrLength(destAddr, neighborAddr);
	printf("\n%s : Checking the neighbour address (%s) match with destination address(%s),  MatchedLength = %d",__FUNCTION__,neighborAddr, destAddr,matchedLength);
	return matchedLength;

}



//Code by JOE commented on August 25, 2017
// int findUIDtoDestinationMatch(char* add1 , char* add2){

// 	char destUID[20];
// 	char neighbourUID[20];

// 	// replaced myUID with neighbour - Aug 24
// 	getUID(neighbourUID,add2);
// 	getUID(destUID,add1);

//     if (strstr(destUID, neighbourUID) == NULL) {
//         printf("Substring not matched : %s not in %s", neighbourUID, destUID);
//         return 0;
//     }
//     else {
//         printf("Substring matched : %s", strstr(destUID, neighbourUID));
//         // Get the length
//         return sizeof(neighbourUID)-1;
//     }

// }

/**
 * CheckAllDestinationLabels(char[])
 *
 * check all the destination labels and find whether if any label matches with the destination label.
 *
 * @return length (int)
 */



int CheckAllDestinationLabels(char* dest){

	int retVal = 1; //ERROR / FAILURE
	struct nodeTL *fNode = headTL;
	char* temp;


	while (fNode != NULL) {
		temp  = fNode->tier;
		//printf("\n CheckAllDestinationLabels : The element is : [%s]",temp);
		printf("\n%s: Comparing destination label : [%s] with my label [%s]\n",__FUNCTION__,dest,temp);
		if ((strlen(temp) == strlen(dest))
			&& ((strncmp(temp, dest, strlen(dest)) == 0))){
			return 0;
		}
		fNode = fNode->next;
	}
	return 1;
}

//Function Added by supriya on August 31, 2017. This function checks if the destination label is a subsrting of 
//any of my labels(Which means that the destination is either my parent or grandparent). 
//IF yes then returns true and generates the address of the parent of the current label to whom the 
//the packet will be sent. 

boolean isDestSubstringOfMyLabels(char* destLabel,char* myMatchedLabel) 
 {
	struct nodeTL *fNode = headTL;
	char* temp;
	if (fNode == NULL) {
		if(enableLogScreen)
			printf("\nERROR: My label List is empty.\n");
		if(enableLogFiles)
			fprintf(fptr,"\nERROR: My label List is empty.\n");
		return false;
	}
	
	while (fNode != NULL) {
		temp  = fNode->tier;
		if(strlen(destLabel) < strlen(temp)){

			if(checkIfSubstring(destLabel,temp)){ // NS defiend up 

				strcpy(myMatchedLabel, temp);
				return true;
			}
		}
		fNode = fNode->next;
	}
	return false;
 }

//Function added by supriya 
 //This function checks if any of my label is a subsrting of the destination label.
 boolean isMyLabelSubstringOfDest(char* destLabel,char* myMatchedLabel) 
 {
	struct nodeTL *fNode = headTL;
	char* temp;
	if (fNode == NULL) {
		if(enableLogScreen)
			printf("\nERROR: My label List is empty.\n");
		return false;
	}
	
	while (fNode != NULL) {
		temp  = fNode->tier;
		if(strlen(destLabel) > strlen(temp)){

			if(checkIfSubstring(destLabel,temp)){

				strcpy(myMatchedLabel, temp);
				return true;
			}
		}
		fNode = fNode->next;
	}
	return false;
 }

/**
 * getParent()
 *
 * method to get the Parent from the current Tier address and store it in curParent.
 *
 * @return returns the parents tier value. returns 0 if the node is tier 1 node
 */

int getParentName(char* curParent,char* currentTier){

	int i = 0;
	int count = 0;
	int count1 = 0;
	int tierValue = 0;
	char* tierVal;

	//get the count of the '.'
	while(currentTier[i] != '\0'){
		if(currentTier[i] == '.'){
			count++;
		}
		i++;
	}

	i=0;

	//skip the first tier value
	while(currentTier[i] != '.'){
		tierVal[i] = currentTier[i];
		i++;
	}
	tierVal[i] = '\0';
	i = i+1;

	int k = 0;

	//store the parent address in 'curParent'
	while(currentTier[i] != '\0'){
		if(currentTier[i] == '.'){
			count1++;
		}
		if(count1 == count){
			break;
		} else{
			curParent[k] = currentTier[i];
			i++;
			k++;
		}
	}
	curParent[k] = '\0';

	tierValue = atoi(tierVal);
	printf("\nThe parent tier value is : %d\n", tierValue-1);
	strcat(tierVal, ".");
	strcat(tierVal, curParent);
	printf("\nThe parent is here : %s\n", tierVal);
	strcpy(curParent,tierVal);
	return tierValue-1;


}

void removeLabels(){

	int retVal = 1; //ERROR / FAILURE
	struct nodeTL *fNodeML = headTL;
	char* tempML;

	struct nodeHL *fNodeNT = headHL;
	char* tempNT;

	//for deletion
	struct nodeTL *temp, *prev;
	temp = headTL;

	int del = 0;
	int nodeDel = 0;

	while (fNodeML != NULL) {
		tempML  = fNodeML->tier;

		char curParent[20];
		memset(curParent,'\0',20);
		int prnt = getParentName(curParent, tempML);
		printf("\nThe parent of [%s] is [%s]\n",tempML,curParent);
		if(prnt>0) { //returns 0 if the present node is tier 1 node
			while (fNodeNT != NULL) {
				tempNT = fNodeNT->tier;
				if((strlen(tempNT) == strlen(curParent))
				   && ((strncmp(tempNT, curParent, strlen(curParent)) == 0))){
					del = 1;
					break;
				}
				fNodeNT = fNodeNT->next;
			}
			if(del ==0){
				printf("\nThe parent of [%s] is [%s]\n",tempML,curParent);
				if (fNodeML == headTL) {
					headTL = fNodeML->next;
				} else {
					prev->next = fNodeML->next;
					nodeDel = 1;
				}
			}
		}
		if(nodeDel == 0){
			prev = fNodeML;
		}
		fNodeML = fNodeML->next;
		del = 0;
		nodeDel = 0;
	}

}

int getTimeStamp(char *buf, uint len, struct timespec *ts)
{
  int ret;
  struct tm t;

  tzset();

  if(localtime_r(&(ts->tv_sec), &t) == NULL)
  {
    return 1;
  }

  ret = strftime(buf, len, "%F %T", &t);
  if(ret == 0)
  {
    return 2;
  }

  len -= ret - 1;

  ret = snprintf(&buf[strlen(buf)], len, ".%09ld", ts->tv_nsec);
  if(ret >= len)
  {
    return 3;
  }

  return 0;
}

void timestamp()
{
  clockid_t clk_id = CLOCK_REALTIME;
  const uint TIME_FORMAT_LENGTH = strlen("2000-12-31 12:59:59.123456789") + 1;
  char timeStampFormat[TIME_FORMAT_LENGTH];
  struct timespec ts;
  clock_gettime(clk_id, &ts);

  if(getTimeStamp(timeStampFormat, sizeof(timeStampFormat), &ts) != 0)
  {
    printf("getTimeStamp failed!\n");
  }

  else
  {

    printf("\nTimestamp - %s", timeStampFormat);
   
  }

  return;
}




#endif

