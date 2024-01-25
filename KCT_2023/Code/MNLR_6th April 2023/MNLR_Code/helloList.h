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
extern char interAS_portName[];

boolean containsTierAddress(char testStr[20]);
boolean setByTierPartial(char inTier[20], boolean setFWDFields);
boolean setByTierOnly(char inTier[20], boolean setFWDFields);
boolean setByTierManually(char inTier[20], boolean setFWDFields);
void timestamp();
struct timeval current_time;

extern void getUID(char* curUID,char* currentTier);

extern void modify_LL(char *addr);

//Tuheena 28apr2022 variables InterAS
struct nodeHL *headHL = NULL;
extern int myTierValue;
struct nodeHL *interAS_head = NULL; //to store interAS neighbors
extern char unavailable_interface[5]; //to store unavailable interface when an interface goes down
//Tuheena InterAS ends

struct nodeHL {
	char tier[20];          // tier value
	char port[20];    		//eth port on which neighbor
	struct nodeHL *next;      // next node
} *headHL;         // linked list to neighbors

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
}*headStable;  //  updated state  of my ports


/**s
 * append()
 *
 * method to add node after previous node, called by insert()
 *
 * @param inTier (char[]) - tier value
 * @param inPort (char[]) - interface value
 *
 */

void append(char inTier[20], char inPort[20], int interAS) {

	struct nodeHL *temp, *right; // nodeHL stores my labels
	temp = (struct nodeHL *) malloc(sizeof(struct nodeHL));

	strcpy(temp->tier, inTier);
	strcpy(temp->port, inPort);
	
	//Tuheena 28apr2022 interAS
	if (interAS==1){
        right = (struct nodeHL *) interAS_head;
		printf("\n value of interAS %d\n",interAS); //Tuheena 2june2022: result is 1 

	}
	        
        else {  
		right = (struct nodeHL *) headHL;
		}

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
		//tag = 4 : interAS between the BR's


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

		while( (add1[posAdd1] != '.') && (add1[posAdd1] != '\0'))
 		{
  
			val1 = (val1 * 10 )+  add1[posAdd1] - '0' ;
			posAdd1++;
 		}
	
 		while( (add2[posAdd2] != '.') && (add2[posAdd2] != '\0'))
 		{
 
 			val2 = (val2 * 10 )+  add2[posAdd2] - '0' ;
			posAdd2++;
 		}

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
 	

	while(add1[posAdd1++] != '.');
 	while(add2[posAdd2++] != '.');

 	while( (add1[posAdd1] != '\0') && (add2[posAdd2] != '\0'))
 	{

		while( (add1[posAdd1] != '.') && (add1[posAdd1] != '\0'))
 		{

 			   
			val1 = (val1 * 10 )+  add1[posAdd1] - '0' ;
			posAdd1++;
 		}
	

 		while( (add2[posAdd2] != '.') && (add2[posAdd2] != '\0'))
 		{
 			 
 			val2 = (val2 * 10 )+  add2[posAdd2] - '0' ;
			posAdd2++;
 		}

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
 	
	int count=0;
	while(add1[posAdd1++] != '.' && count<2){
		count++;
	}
 	while(add2[posAdd2++] != '.');

 	while( (add1[posAdd1] != '\0') && (add2[posAdd2] != '\0'))
 	{

		while( (add1[posAdd1] != '.') && (add1[posAdd1] != '\0'))
 		{

 			 
			val1 = (val1 * 10 )+  add1[posAdd1] - '0' ;
			posAdd1++;
 		}
	
	 
 		while( (add2[posAdd2] != '.') && (add2[posAdd2] != '\0'))
 		{
 			 
 			val2 = (val2 * 10 )+  add2[posAdd2] - '0' ;
			posAdd2++;
 		}

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

void add(char inTier[20], char inPort[20], int interAS) { //Tuheena 28apr2022

	struct nodeHL *temp;
		//samruddhi
	struct portTag *temp2;
	temp = (struct nodeHL *) malloc(sizeof(struct nodeHL));
	strcpy(temp->tier, inTier);
	strcpy(temp->port, inPort);


	//Tuheena 28apr2022 interAS 160-169
    if (interAS==1){
            if (interAS_head == NULL) {
            	interAS_head = temp;
            	interAS_head->next = NULL;
        	} else {
                temp->next = interAS_head;
                interAS_head = temp;
        	}
        }
        else{
			//Tuheena 28apr2022 interAS 160-169 ends

			if (headHL == NULL) {
				headHL = temp;
				headHL->next = NULL;
			} else {
				temp->next = headHL;
				headHL = temp;
			}
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

//Tuheena 28apr2022 InterAS starts
void printInterASNeighbourTable() {
        struct nodeHL *fNode = interAS_head;
        char* temp;
        if (fNode == NULL) {
                if(enableLogScreen)
                        printf("ERROR: Neighbor List is empty (Isolated Node)\n");
                return;
        }
        // traverse the list
        // testing
        if(enableLogScreen)
                printf("\n*************** interAS Neighbor Table *************");
        while (fNode->next != NULL) {
                temp  = fNode->tier;                
                if(enableLogScreen)
                        printf("\n ------- %s -------",temp);
                fNode = fNode->next;
        }
        printf("\n ------- %s -------",fNode->tier);
        return;
}
//Tuheena InterAS  ends
 
 int staging(char inPort[20]){
 	//Staging part - Samruddhi

	int checkstableport = findStable(inPort);
	
	int stable=0;

	if(checkstableport == 1){ 

		int checkStaging = findStaging(inPort);

		if(checkStaging==1){

			struct stagingNode*temp2;
			temp2 = malloc(sizeof(struct stagingNode));

			strcpy(temp2->port, inPort);
			gettimeofday(&current_time , NULL);
			temp2->lastUpdate = ((double)current_time.tv_sec*1000000 + (double)current_time.tv_usec)/1000000;
			temp2->countHello=1;
			printf("TEST: Message Counter %d\n", temp2->countHello);

			if (stagingHead == NULL) {
				stagingHead = temp2;
				stagingHead->next = NULL;
			} 	else {
				temp2->next = stagingHead;
				stagingHead = temp2;
			}
		} else{
			//Label is in stagging
			printf("\nIn updateStaging %s",inPort);
			stable = updateStaging(inPort);
			
		}

	}
	else {

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

int insert(char inTier[20], char inPort[20], int interAS_neighbor) { 	
		int isEntryNew = 0;
		struct nodeHL *temp; //Tuheena 10 may added from sam code
		temp = headHL; //my neighbor table  Tuheena 10 may added from sam code
		//Tuheena 28apr2022 InterAS starts
    	if (interAS_neighbor){
                struct nodeHL *temp;
                temp = interAS_head; // what is in interAS_head
				
                if (temp == NULL) {
                        add(inTier, inPort, 1);
                        isEntryNew = 1;

                        printInterASNeighbourTable();
						printf("\n this is the best place to send BR IP request resolve \n");
						
                        char DestIPAddr[] = "0.0.0.0";
						char tempAddrA[20];
						memset(tempAddrA, '\0', 20);
						strcpy(tempAddrA, getTierAddr(0)); //copy the qth tier address to tempAddrA
					
									   
						requestIPresolveBR(DestIPAddr, tempAddrA );
						freeGetTierAddr(); //free the tier address field	

                } else {
                        int checkNode = find(inTier, inPort, 1);
                        if (checkNode == 1) { // The entry is not present in the table, so append it to the table.
								
                                append(inTier, inPort, 1);
                                isEntryNew = 1;
                                printInterASNeighbourTable();
                        } else { // the enrty is present, so update the table 
								
                                update(inTier, inPort, 1);
                        }
                }
			
        }
    	else{
		//Tuheena InterAS ends
		
		struct nodeHL *temp;
		temp = headHL;
		if (temp == NULL) {

		add(inTier, inPort, 0);
		isEntryNew = 1;
		printNeighbourTable();
		printMyLabels();
		
		}
		else {

			int checkNode = find(inTier, inPort, 0);

			if (checkNode == 1) { // The entry is not present in the table, so append it to the table.
				append(inTier, inPort, 0);
				isEntryNew = 1;
				printNeighbourTable();
				printMyLabels();
			} 
			else {
				update(inTier,inPort,0	);
			}

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
int find(char inTier[20], char inPort[20], int interAS) {

	int returnVal = 1;

//Tuheena 28apr2022 interAS starts
	struct nodeHL *fNode = NULL;
	if (interAS==1){ //interAS
		fNode = interAS_head;
	}
	else{
	 	fNode = headHL;
	}
//Tuheena interAS ends

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
						
						returnVal = 0;
						break;
					}
				}


		fNode = fNode->next;
	}
	
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
						//printf("\nFindStagingPositive %s", inPort);
						returnVal = 0;
						break;
					}
				}


		fNode = fNode->next;
	}
	
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

						if (fNode == stagingHead) {
							printf("\nTEST: Head node removed from stagging value was %s\n", fNode->port);
							stagingHead = fNode->next;
						} else {
							prev1->next = fNode->next;
							printf("\nTEST: other node removed from stagging value was %s\n", fNode->port);
						}
					}
				}
				 else{
				prev1 = fNode;
		} 

		fNode = fNode->next;
	}



}

void deleteStable(char inPort[20]) {

		struct stablePort *fNode = headStable;
		struct stablePort *prev1= headStable;
	// traverse the list
	// testing
	while (fNode != NULL) {

				if (strlen(fNode->port) == strlen(inPort)) {

					if (strncmp(fNode->port, inPort, strlen(inPort)) == 0) {
						if (fNode == headStable) {
							printf("TEST: Head stable node removed value was %s\n", fNode->port);
							headStable = fNode->next;
						} else {
							prev1->next = fNode->next;
							printf("TEST: other stable node removed value was %s\n", fNode->port);
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
void update(char inTier[20], char inPort[20], int interAS) {

	struct stablePort *uNode = headStable;
		// traverse the list
		// testing
		while (uNode != NULL) {

				if (strlen(uNode->port) == strlen(inPort)) {

					if (strncmp(uNode->port, inPort, strlen(inPort)) == 0) {

						gettimeofday(&current_time , NULL);
						uNode->lastUpdate = ((double)current_time.tv_sec*1000000 + (double)current_time.tv_usec)/1000000;

					}
				}

			uNode = uNode->next; // 
		}
}

//Tuheena 28apr2022 starts
//determine if the neighbor is a tier-1 node or not
int isTier1Neighbor(char port[20]){
        int ret = 0;
        
        struct nodeHL *fNode = headHL;
        while (fNode != NULL) {
                if (strlen(fNode->port) == strlen(port)) {
                        if (strncmp(fNode->port, port, strlen(port)) == 0 && strncmp(fNode->tier,"1",1) == 0) {
                                return 1;
                        }
                }
                fNode = fNode->next;
        }
        return ret;
}
//Tuheena ends


int updateStaging(char inPort[20]) {


	struct stagingNode *fNode = stagingHead;

	printf("\nStaging List : %s", inPort);


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
						
						if(uNode->countHello>=3){

						 	 printf("\n MessageCount >= 3 : %s",inPort);
							stable=1;
							//adding to stablePort
							struct stablePort *temp2;
							temp2 = (struct stablePort *) malloc(sizeof(struct stablePort));
							gettimeofday(&current_time , NULL);
							temp2->lastUpdate = ((double)current_time.tv_sec*1000000 + (double)current_time.tv_usec)/1000000;
							strcpy(temp2->port, inPort);

							if (headStable == NULL) {
								headStable = temp2;
								headStable->next = NULL;
							} 	else {
								temp2->next = headStable;
								headStable = temp2;
							}

							//Removing from staging
							deleteStaging(inPort);

							
						}
					}
				}
						
			uNode = uNode->next;

	}
	

	return stable;

}

int compare(char *addr){
	struct nodeHL *temp = headHL;
	int ret = 0;
	while(temp){
		
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
		
		// If last updated local time is more than desired time
		if (delTimeDiff >= 0.40) { //dead timer(from 1.3 to 0.40) reduced 4/20/2022 - Samruddhi
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

		// If last updated local time is more than desired time
		if (delTimeDiff >= 0.60) {
				//if node to be removed is head
				if (temp1 == stagingHead) {
				
					stagingHead = temp1->next;
					//free(temp);
					}else {
						prev1->next = temp1->next;
				
					}
								//}
		} else{
				prev1 = temp1;
			} 
		temp1 = temp1->next;

	}
	
	return ret;
}

/*
your are giving as input the port number such as eth1, eth2 ect
you are also giving it an intger that tells if it is to child/to parent etc
and it store in port_tag

*/
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
/*
Shenoy - give port number as input  and it will 
look up the port tag structure and return an
integer - telling you if it is child, parent etc. 

*/

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
	
		return;
	}

	int i = 1;
	while (r != NULL) {

		i = i + 1;
		r = r->next;
	}

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

		// Target Node
		// Length Check
		// Value check

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

					//Tuheena 28apr2022 885-892 starts
				if(strlen(unavailable_interface)!=0){
						if(strcmp(fNode->port,unavailable_interface)!=0){
								//printf("in in in in in in in %s",fNode->port);
								strcpy(parentTierAdd, temp);
								return;
						}
				}
				//Tuheena 28apr2022 885-892 ends
				else if(checkIfSubstring(temp1,temp)){
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

	strcpy(temp1,myTierAdd);
	printf("\nFinding parent for %s\n", temp1);

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

		if(strlen(temp) >= strlen(myLabel)){// new line added by Supriya
			if(strlen(temp) <= strlen(desTierAdd)){

				if(checkIfSubstring(desTierAdd,temp)){
					//ongestMtchLength = tempLen;
					strcpy(childTierAdd, temp);

					return;
				}
			}
		}
		fNode = fNode->next;
	}
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

 int examineNeighbourTable(char* desTierAdd, char* longstMatchingNgbr,char* myLabel, int type) 
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

	if(type == 1){ //TUheena 28apr2022
		while (fNode != NULL) {
			temp  = fNode->tier;	
			if(enableLogScreen){
				//printf("\n%s temp->%s desTierAdd-->%s",__FUNCTION__,temp,desTierAdd);
				printf("\n%s: Check if my neighbor: %s is a substring of destination label(uid): %s",__FUNCTION__,temp,desTierAdd);	
			}

			if(strlen(temp) < strlen(desTierAdd)){
				//Tuheena 28apr2022 1003-1012 starts
                if(strlen(unavailable_interface)!=0){
                    if(strcmp(fNode->port,unavailable_interface)!=0 && myTierValue > atoi(fNode->tier)){
                    //printf("in in in in in in in %s",fNode->port);
                        strcpy(longstMatchingNgbr, temp);
                        return 0;
                                
                    }
                }
                //Tuheena 28apr2022 1003-1012 ends
                             
				else if(checkIfSubstring(desTierAdd,temp)){
					strcpy(longstMatchingNgbr, temp);
					return 0;
				}
			}

			fNode = fNode->next;
		}
	}
	else {
		if (type == 2){
			while (fNode != NULL) {
				temp  = fNode->tier;	
				if(enableLogScreen){

					printf("\n%s: Check if destination label(uid): %s is a substring of my neighbor: %s",__FUNCTION__,desTierAdd,temp);	
				}

				if(strlen(temp) <= strlen(myLabel)){
					if(strlen(temp) > strlen(desTierAdd)){
						if(checkIfSubstring(desTierAdd,temp)){
							strcpy(longstMatchingNgbr, temp);
							return 0;
						}
					}
				}

				fNode = fNode->next;
			}
		}
		else{
			while (fNode != NULL) {
				temp  = fNode->tier;	
				if(enableLogScreen){
					printf("\n%s: Check if my neighbor: %s is a substring of destination label(uid): %s",__FUNCTION__,desTierAdd, temp);	
				}


				if(strlen(temp) >= strlen(myLabel)){
					if(strlen(temp) < strlen(desTierAdd)){
						if(checkIfSubstring(desTierAdd,temp)){
							strcpy(longstMatchingNgbr, temp);
							return 0;
						}
					}
				}	
				
				fNode = fNode->next;
			}

		}
	}
//Tuheena 28apr2022 uncommented from if type==2
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
		  

 	while( (add1[posAdd1] != '\0') && (add2[posAdd2] != '\0'))
 	{
		
		while( (add1[posAdd1] != '.') && (add1[posAdd1] != '\0'))
 		{
  
			val1 = (val1 * 10 )+  add1[posAdd1] - '0' ;
			posAdd1++;
 		}
		posAdd1++;

 		while( (add2[posAdd2] != '.') && (add2[posAdd2] != '\0'))
 		{
  
 			val2 = (val2 * 10 )+  add2[posAdd2] - '0' ;
			posAdd2++;
 		}
		posAdd2++;

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


	while (fNode != NULL) {
		temp  = fNode->tier;


		if(checkIfSubstring(desTierAdd,temp)){

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

/// tryong to fix the use of grandparent. On hold 
boolean checkIfSubstringBR(char* add1 , char* add2){

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

		while( (add1[posAdd1] != '.') && (add1[posAdd1] != '\0'))
 		{
 
			val1 = (val1 * 10 )+  add1[posAdd1] - '0' ;
			posAdd1++;
 		}
	
 		while( (add2[posAdd2] != '.') && (add2[posAdd2] != '\0'))
 		{

 			val2 = (val2 * 10 )+  add2[posAdd2] - '0' ;
			posAdd2++;
 		}

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

