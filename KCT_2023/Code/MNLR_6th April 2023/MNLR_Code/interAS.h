#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include "tierList.h"
#include "endNetworkUtils.h"
#include "sendAndFwd.h"

extern char* tierAddress;
extern uint8_t  cache_BR_address[];
extern void findParntLongst(char* myTierAdd,char* parentTierAdd);
extern boolean setByTierOnly(char inTier[20], boolean setFWDFields);
extern int endNetworkSend(char[], uint8_t *, int);
extern int buildPayloadQuery(uint8_t *, char[], int);
extern int setTierInfo(char tierValue[]);
extern int packetForwardAlgorithm(char currentTier[], char desTier[]);
extern int isFWDFieldsSet();
extern int dataSend(char etherPort[], unsigned char ipPacket[], char destTier[],
char srcTier[], int ipPacketSize);
extern int isTierSet();
extern int old_interfaceListSize;
extern boolean updateEndDestinationTierAddrHC(char tempIP[],char Mytier[]);
extern int freeInterfaces();
extern int buildPayloadInterAS(uint8_t *);
extern int setInterfaces();
extern int interAS_neighbor;
extern struct timeval cur_tm;
extern double interface_time;
extern uint8_t tierLen;
extern int cache_nIP;
extern int interAS_flag;
extern char cache_bufferIP[];
extern char interAS_portName[];
extern int interfaceListSize;
extern char *interfaceList[10];
extern long long int MPLRCtrlSendCount;
extern struct addr_tuple *interAS_tablehead;

/*
	BRfwdIP() is executed at BRs to send a InterAS_IP message to the other BR - KCT 06/04/2023
*/
void BRfwdIP (unsigned char *ethhead, int n, char buffer[]){
	printf("\nSending IPs to other BR... \n ");
	printf("\nEthhead value :%u",ethhead[15]);

	unsigned char desLenTemp[2];
	sprintf(desLenTemp, "%u", ethhead[15]);
	uint8_t desLen = atoi(desLenTemp);

	// printing destination tier
	char destLocal[20];
	memset(destLocal, '\0', 20);
	memcpy(destLocal, &ethhead[16], desLen);

	char finalDes[20];
	memset(finalDes, '\0', 20);
	memcpy(finalDes, &destLocal, desLen);

	// printing source tier length
	int tempLoc = 16 + desLen;
	unsigned char srcLenTemp[2];
	sprintf(srcLenTemp, "%u", ethhead[tempLoc]);

	uint8_t srcLen = atoi(srcLenTemp);

	// printing source tier
	tempLoc = tempLoc + 1;
	char srcLocal[20];
	memset(srcLocal, '\0', 20);
	memcpy(srcLocal, &ethhead[tempLoc], srcLen);

	char finalSrc[20];
	memset(finalSrc, '\0', 20);
	memcpy(finalSrc, &srcLocal, srcLen);

	tempLoc = tempLoc + srcLen;

	int decapsPacketSize = n - tempLoc;
						
	// decapsulating the EIBP header from the packet
	unsigned char decapsPacket[decapsPacketSize];
	memset(decapsPacket, '\0', decapsPacketSize);
	memcpy(decapsPacket, &buffer[tempLoc], decapsPacketSize);
						
	uint8_t *mplrPayload = allocate_ustrmem (IP_MAXPACKET);

	int mplrPayloadLen = 0;

	// building a payload of type INTERAS_IP to send to the other BR
	mplrPayloadLen = buildPayloadInterIp(mplrPayload,decapsPacket,decapsPacketSize,MESSAGE_TYPE_INTERAS_IP);

	printf("\nSending the payload after decapsulating the header from the ip packet\n");
							
	if (mplrPayloadLen) {
        endNetworkSend(interAS_portName, mplrPayload, mplrPayloadLen);
    }
	free(mplrPayload);
}

/*
	MESSAGE_TYPE_INTERAS_IP is received from other BR. This is the IP packet to be sent to an ipnode in the current AS
*/

void message_3(unsigned char *ethhead, char recvOnEtherPort[], char queryPort[], int n, char buffer[],char tierDest[],int nIP,char bufferIP[]){
	
	uint8_t action = ethhead[15];

	if (action == MESSAGE_TYPE_INTERAS_IP){ // Received IP from BR of other AS

		printf("\nReceived an IP from different AS\n");

		unsigned char *ipHeadWithPayload;
		int ipPacketSize = n - 16;
		ipHeadWithPayload = (unsigned char*) malloc(ipPacketSize);

		memset(ipHeadWithPayload, '\0', ipPacketSize);
		memcpy(ipHeadWithPayload, &buffer[16], ipPacketSize);
						
		unsigned char ipDestTemp[20];
		sprintf(ipDestTemp, "%u.%u.%u.%u", ipHeadWithPayload[16], ipHeadWithPayload[17],ipHeadWithPayload[18], ipHeadWithPayload[19]);

		unsigned char desLenTemp[2];
		sprintf(desLenTemp, "%u", ethhead[17]);
		uint8_t desLen = atoi(desLenTemp);

		// printing destination tier
		char destLocal[20];
		memset(destLocal, '\0', 20);
		memcpy(destLocal, &ethhead[18], desLen);

		char finalDes[20];
		memset(finalDes, '\0', 20);
		memcpy(finalDes, &destLocal, desLen);

		unsigned char destIP[20];
		sprintf(destIP, "%u.%u.%u.%u", ethhead[47], ethhead[48], ethhead[49], ethhead[50]);
								
		// printing source tier length
		int tempLoc = 18 + desLen;
		unsigned char srcLenTemp[2];
		sprintf(srcLenTemp, "%u", ethhead[tempLoc]);

		uint8_t srcLen = atoi(srcLenTemp);
					
		// printing source tier
		tempLoc = tempLoc + 1;
		char srcLocal[20];
		memset(srcLocal, '\0', 20);
		memcpy(srcLocal, &ethhead[tempLoc], srcLen);

		char finalSrc[20];
		memset(finalSrc, '\0', 20);
		memcpy(finalSrc, &srcLocal, srcLen);

		cache_nIP = n;
		memset(cache_bufferIP, '\0', n);
							
		memcpy(cache_bufferIP,buffer,n);

		unsigned char *ipHeadPayload;
		int ipPacketSiz = cache_nIP - 14;
		ipHeadPayload = (unsigned char*) malloc(ipPacketSiz);
		memset(ipHeadPayload, '\0', ipPacketSiz);
		memcpy(ipHeadPayload, &cache_bufferIP[14], ipPacketSiz);
						
		unsigned char ipDestTem[20];
		sprintf(ipDestTem, "%u.%u.%u.%u", ipHeadPayload[18],ipHeadPayload[19],ipHeadPayload[20],ipHeadPayload[21]);
						
		int packetFwdStatus = -1;

		if (isTierSet() == 0) {

			boolean checkDestStatus = updateEndDestinationTierAddrHC(ipDestTemp,headTL->tier);

			if (checkDestStatus == false) {
				errorCount++;
					if(strlen(cache_BR_address)!=0){
						printf("cached BR address %s",cache_BR_address);
						strcpy(tierDest, cache_BR_address);

						setTierInfo(headTL->tier);
						packetFwdStatus = packetForwardAlgorithm(tierAddress,
						tierDest);
					}
					else{ // not cached the BR address 
						cache_nIP = nIP;
						memset(cache_bufferIP, '\0', nIP);
							
						memcpy(cache_bufferIP,bufferIP,nIP);
						char parentTierAdd[20];
						memset(parentTierAdd,'\0',20);
						printf("\n End destination tier address not available - to send a query\n");
						findParntLongst(tierAddress,parentTierAdd);
						setByTierOnly(parentTierAdd, true);
						printf("\nSending a query to Tier-1 node for BR address");
							
						uint8_t *mplrPayload = allocate_ustrmem (IP_MAXPACKET);

						int mplrPayloadLen = 0;

						mplrPayloadLen = buildPayloadQuery(mplrPayload,ipDestTemp,MESSAGE_TYPE_QUERY_REQUEST);
							
						if (mplrPayloadLen) {
								endNetworkSend(fwdInterface, mplrPayload, mplrPayloadLen);
						}
						free(mplrPayload);
					}
			}
			else{
				setTierInfo(headTL->tier);
				packetFwdStatus = packetForwardAlgorithm(tierAddress, tierDest);
			}
		} 
		else {
			printf("ERROR: Tier info was not set\n");

			setTierInfo(headTL->tier);
			packetFwdStatus = packetForwardAlgorithm(tierAddress, tierDest);
		}

		if (packetFwdStatus == 0) {
				
			if ((strlen(fwdTierAddr) == strlen(tierAddress)) && (strcmp(fwdTierAddr, tierAddress) == 0)) {
					printf("TEST: Received IP packet -(it's for me only, no forwarding needed) \n");
			} else {
				if (isFWDFieldsSet() == 0) {
					printf("TEST: Forwarding IP Packet as MNLR Data Packet (Encapsulation) \n");
					printf("Sending on interface: %s \n", fwdInterface);
					dataSend(fwdInterface, ipHeadWithPayload, tierDest, tierAddress, ipPacketSize);
				}
			}
		}

		freeInterfaces();
		interfaceListSize = 0;
	}
}

/*
*	Receives message from BRand forwards to T1 if it is T2
*	If it is at T1 then it adds the IP of other AS to table
* 
*	- KCT 06/04/2023
*/
int add_interAS_IP_label_map_T1_T2 (uint8_t totalEntries, unsigned char *ethhead, int* index, struct sockaddr_ll src_addr, int myTierValue){

	int IPentries = 0;
	printf("in function NT total entries = %d", totalEntries );

	if(myTierValue==1){ 
								
		while(totalEntries > 0) {
				
			uint8_t tierLen = ethhead[*index];
			uint8_t tierAddr[tierLen + 1];

			memset(tierAddr, '\0', tierLen + 1);
			++*index; // pass the length byte

			memcpy(tierAddr, &ethhead[*index], tierLen);

			*index += tierLen;
			tierAddr[tierLen] = '\0';

			uint8_t ipLen = ethhead[*index];
			struct in_addr ipAddr;
			++*index; // pass the length byte

			memcpy(&ipAddr, &ethhead[*index], sizeof(struct in_addr));
			*index = *index + sizeof(struct in_addr);

			uint8_t cidr = ethhead[*index];							
			++*index; // pass the length of cidr

			struct addr_tuple *temp = (struct addr_tuple*) malloc(sizeof(struct addr_tuple));
			memset(temp, '\0', sizeof(struct addr_tuple));
			strcpy(temp->tier_addr, tierAddr);

			interAS_neighbor = 1; 

			if (find_entry_LL(&ipAddr, tierAddr, interAS_neighbor ) == 0) {  //To check 
			// insert info about index from which the packet has been received from.
				printf("\nAdding interAS ip address: %s cdir: %d to the interAS mapping table",inet_ntoa(ipAddr),cidr);
				temp->if_index = src_addr.sll_ifindex;
				temp->isNew = true;
				memcpy(&temp->ip_addr, &ipAddr, sizeof(struct in_addr));
				temp->cidr = cidr;
				add_entry_LL(temp,interAS_neighbor);
				IPentries++;
				print_interAS_entries_LL();
			}
			totalEntries--;
		}
	}
	else{  // tier 2 will just forward 
		char parentTierAdd[20];
		memset(parentTierAdd,'\0',20);
								
		findParntLongst(tierAddress,parentTierAdd);
		setByTierOnly(parentTierAdd, true);

		printf("\nPassing the interAS ip mapping payload to %s via %s",parentTierAdd,fwdInterface);
							
		while(totalEntries > 0) {
			uint8_t tierLen = ethhead[*index];
			uint8_t tierAddr[tierLen + 1];

			memset(tierAddr, '\0', tierLen + 1);
			++*index; // pass the length byte

			memcpy(tierAddr, &ethhead[*index], tierLen);

			*index += tierLen;
			tierAddr[tierLen] = '\0';

			uint8_t ipLen = ethhead[*index];
			struct in_addr ipAddr;

			++*index; // pass the length byte

			memcpy(&ipAddr, &ethhead[*index], sizeof(struct in_addr));

			*index = *index + sizeof(struct in_addr);

			uint8_t cidr = ethhead[*index];

			printf("\nReceived interAS ip address = %s and its cdir %d and need to send it my parent %s via %s",inet_ntoa(ipAddr),cidr,parentTierAdd,fwdInterface);
			++*index; // pass the length of cidr
								
			totalEntries--;
		} 
		uint8_t *mplrPayload = allocate_ustrmem(IP_MAXPACKET);
		int mplrPayloadLen = 0;
		int z = 0;
		for (z = 14; z < *index; z++, mplrPayloadLen++) {
			mplrPayload[mplrPayloadLen] = ethhead[z];
		}

		endNetworkSend(fwdInterface, mplrPayload, mplrPayloadLen);
	}
	return IPentries;
}

/*
	Received IP messages from Tier 1. This populates the ip-label table at the BR

	- KCT 06/04/2023
*/
int populate_InterAS_IP_to_label_table(uint8_t totalEntries, unsigned char *ethhead, int* index, struct sockaddr_ll src_addr, int myTierValue, int interAS_neighbor){
	
	uint8_t action = ethhead[16]; 
	int IPentries = 0;
	while (totalEntries > 0) {
		uint8_t tierLen = ethhead[*index];
		uint8_t tierAddr[tierLen + 1];

		memset(tierAddr, '\0', tierLen + 1);
		++*index; // pass the length byte
						
		memcpy(tierAddr, &ethhead[*index], tierLen);

		*index += tierLen;
		tierAddr[tierLen] = '\0';

		printf("TierLen :%u TierAddr: %s\n", tierLen, tierAddr);

		uint8_t ipLen = ethhead[*index];
		struct in_addr ipAddr;

		++*index; // pass the length byte
						
		memcpy(&ipAddr, &ethhead[*index], sizeof(struct in_addr));

		*index = *index + sizeof(struct in_addr);

		uint8_t cidr = ethhead[*index];

		++*index; // pass the length of cidr

		if (action == MESSAGE_TYPE_ENDNW_ADD) {

			printf("\nipLen :%u ipAddr: %s cidr: %u\n", ipLen, inet_ntoa(ipAddr), cidr); //cidr %u to %d

			struct addr_tuple *a = (struct addr_tuple*) malloc(sizeof(struct addr_tuple));
			memset(a, '\0', sizeof(struct addr_tuple));

			printf("\n interAS flag in populate interAS_IP = %d,  interAS_neighbor = %d \n", interAS_flag, interAS_neighbor );
			if(interAS_flag==1){
				strcpy(a->tier_addr, headTL->tier) ; // stores my labels mapped to the IP address
			}
			else{
				strcpy(a->tier_addr, tierAddr);  // else sends the label to IP map as is.
			}

			if (find_entry_LL(&ipAddr, tierAddr, interAS_neighbor) == 0) {
				// insert info about index from which the packet has been received from.
				a->if_index = src_addr.sll_ifindex;
				a->isNew = true;

				memcpy(&a->ip_addr, &ipAddr, sizeof(struct in_addr));
				a->cidr = cidr;

				add_entry_LL(a,interAS_flag);

				if (interAS_flag==1){
					print_interAS_entries_LL();
				}
			}
		} else if (action == MESSAGE_TYPE_ENDNW_UPDATE) {

			// Still not implemented, can be done by recording interface index maybe

		} else if (action == MESSAGE_TYPE_ENDNW_REMOVE) {	
			if (delete_entry_LL_IP(ipAddr)) {
				IPentries++;
			}
		} else if(action == MESSAGE_TYPE_ENDNW_REMOVE_ADDR){
			
			if (delete_entry_LL_Addr(tierAddr)) {
					print_entries_LL();
					IPentries++;
			}		
		}  
		totalEntries--;
	} // end of while

	return IPentries;
}

/*
	BR forwards IPs of other AS to T1. Fowarding in T2 is done by add_interAS_IP_label_map_T1_T2()

	- KCT 06/04/2023
*/
int fwdBRaddr (){

	printf("\n Inside fwBRaddr Need to send the ip address to Tier 1 node"); 

	int ifs=0; 
	setInterfaces(); 

	for (ifs = 0; ifs < interfaceListSize; ifs++) {
		if(strcmp(interfaceList[ifs],interAS_portName)!=0 && strcmp(interfaceList[ifs],"eth0")!=0){
			printf("\nSending not on interAS_portname %s ", interAS_portName );
			uint8_t *mplrPayload = allocate_ustrmem (IP_MAXPACKET);

			int mplrPayloadLen = 0;
			mplrPayloadLen = buildPayloadInterAS(mplrPayload);

			if (mplrPayloadLen) {
                    endNetworkSend(interfaceList[ifs], mplrPayload, mplrPayloadLen);
					printf("Sending interAS ip address via non-interAS Port %s with the playload len %d",interfaceList[ifs],mplrPayloadLen);
            }
			free(mplrPayload);
		}
	}
	freeInterfaces();
}

/*
*	Corresponds to Message type 17
	BR sends request to T1 to get IPs of its own AS. T2 forwards request to T1. T1 responds with response message(Type 18)

	- KCT 06/04/2023
*/
void requestIPresolveBR(char DestIPAddr[], char Mytier[]){
	printf("\nSending Message 17 requestIPresolveBR\n");

	char ctrlPayLoadC[400];
	memset(ctrlPayLoadC, '\0', 400);

	uint8_t numOfAddr = 1;
	memcpy(ctrlPayLoadC, &numOfAddr, 1);

	int cpLength = 1;

	//For MyTier 
	char tempMyAddr[20];
	memset(tempMyAddr, '\0', 20);
	strcpy(tempMyAddr, Mytier); //copy mytier to temp
	
	uint8_t localTierSize = strlen(tempMyAddr);
			
	memcpy(ctrlPayLoadC + cpLength, &localTierSize, 1); // copying the tier address size
	cpLength = cpLength + 1;

	memcpy(ctrlPayLoadC + cpLength, tempMyAddr, localTierSize); // copying the tier address
	cpLength = cpLength + localTierSize; 
		
	char tempAddrA[20];
	memset(tempAddrA, '\0', 20);
	strcpy(tempAddrA, DestIPAddr); //copy the the tier address to tempAddrA
			
	uint8_t localTierSizeA = strlen(tempAddrA);
			
	memcpy(ctrlPayLoadC + cpLength, &localTierSizeA, 1); // copying the tier address size
	cpLength = cpLength + 1;

	memcpy(ctrlPayLoadC + cpLength, tempAddrA, localTierSizeA); // copying the tier address
	cpLength = cpLength + localTierSizeA; 

	printf("\nBuilt the PayLoad for BR\n");

	setInterfaces(); //interfaceList is being set by this function
	
	char parentTierAdd[20];
	memset(parentTierAdd,'\0',20);
	/*
	if I am tier 2, then I will call 
	findParntLongst(tierAddress,parentTierAdd); with ter 3 addr
	and pass the address that you get to the next findParntLongst(tierAddress,parentTierAdd); 
	*/

	//If block not needed? - KCT 30th March 2023
	if (myTierValue == 2){
		findParntLongst(Mytier,parentTierAdd);
		printf("in findParntLongst at a tier 2 node value of parentTierAdd = %s ", parentTierAdd);
		char tierAddress[20];
		memset(tierAddress, '\0', 20);	
		strcpy (tierAddress, parentTierAdd);
	}

	findParntLongst(tierAddress,parentTierAdd); // tierAddress is global 
	printf("in findParntLongst at a tier 2 node value of parentTierAdd FIXED = %s ", parentTierAdd);

	struct nodeHL *fNode = headHL; 
	char eth_addr[20];

	if (fNode == NULL) {
		if(enableLogScreen){
			printf("\nERROR: Neighbor List is empty \n");
		}
	}
	
	//Looping through neighbour table to find parent interface
	while (fNode != NULL) {
		if ((strlen(fNode->tier) == strlen(parentTierAdd)) && ((strncmp(fNode->tier, parentTierAdd, strlen(parentTierAdd)) == 0))) {
			strcpy(eth_addr,fNode->port);
			break;
		} else {
			fNode = fNode->next;
		}
	}
	        
	ctrlLabelSend(MESSAGE_TYPE_REQUEST_IP_RESOLVE_BR, eth_addr, ctrlPayLoadC); //ctrlSend method is used to send Ethernet frame
	MPLRCtrlSendCount++; //keeps track of the no of control messages sent
	printf("\nSending MESSAGE_TYPE_REQUEST_IP_RESOLVE_BR message to - %s ",eth_addr);
				
	freeInterfaces();
	interfaceListSize = 0;
}

/*
*	Corresponds to Message type 18
	T1 responds to request message(Type 17) and returns IP-Label Map to BR

	- KCT 06/04/2023
*/
void responseIPresolveBR(struct in_addr ip_addr, char DestTierLabel[],uint8_t entries){

	printf("\nCreating buildpayloadBR : %s", DestTierLabel);

	uint8_t *mplrPayload = allocate_ustrmem(IP_MAXPACKET);
	int mplrPayloadLen = 0;

	printf("\nChecking ip address passed  : %s", ip_addr);

	int tierValue = 0;
	if (myTierValue == 1) { // my tiervalue is intialized when the code is startd in a node. 
		tierValue =1;
	}

	printf("\nIn progress buildpayloadBR  : %s", DestTierLabel);
	mplrPayloadLen = buildpayloadBR(mplrPayload, DestTierLabel, tierValue);

	setInterfaces(); //interfaceList is being set by this function
	int ifs = 1;
	int port_number = 0;
		
	if (mplrPayloadLen) {
		char parentofDest[20];
		memset(parentofDest,'\0',20);
		char eth_addr1[20];
		printf("\nI am here for BR %s\n", DestTierLabel);
		findParentLongest(DestTierLabel,parentofDest);
		printf("\nParent for BR: %s\n", parentofDest);
		struct nodeHL *fNode1 = headHL;  // Neighbour list
		
		if (fNode1 == NULL) {
			if(enableLogScreen){
				printf("\nERROR: Neighbor List is empty BR\n");
			}
		}

		//Looping through interfaces to find child interface to send response
		while (fNode1 != NULL) {
			if ((strlen(fNode1->tier) == strlen(parentofDest)) && ((strncmp(fNode1->tier, parentofDest, strlen(parentofDest)) == 0))) {
				strcpy(eth_addr1,fNode1->port);
				printf("\nForwarding resolve response to BR: %s on %s",fNode1->tier, eth_addr1);
				endNetworkSend(eth_addr1, mplrPayload, mplrPayloadLen);
				return;
			} 					
			fNode1 = fNode1->next;
		}
		free(mplrPayload);			
	}		

	freeInterfaces();
	interfaceListSize = 0;

	return;
}