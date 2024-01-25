#include <sys/types.h>
#include <netdb.h>
#include <netinet/if_ether.h>
#include <time.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/time.h>
#include <net/ethernet.h>
#include <signal.h>
#include <ctype.h>

#include "helloList.h"
#include "tierList.h"
#include "genEnvironment.h"
#include "errorControl.h"
#include "printPacketDetails.h"
#include "stringMessages.h"
#include "fwdAlgorithmHelper.h"
#include "baseConversion.h"
#include "endNetworkUtils.h"


extern int ctrlSend(char eth[], char pay[]);
extern int ctrlLabelSend(int, char eth[], char pay[]);
extern int dataSend(char etherPort[], unsigned char ipPacket[], char destTier[],
	char srcTier[], int ipPacketSize);
extern int endNetworkSend(char[], uint8_t*, int);
extern int dataFwd(char etherPort[20], unsigned char MPLRPacket[],
	int MPLRPacketSize);
extern int dataDecapsulation(char etherPort[20],
	unsigned char MPLRDecapsPacket[], int MPLRDecapsSize);
extern int dataDecapsulationUDP(char etherPort[20], unsigned char MPLRDecapsPacket[],
	int MPLRDecapsSize);
extern int packetForwardAlgorithm(char currentTier[], char desTier[]);
extern int isFWDFieldsSet();

extern int isEnvSet();
extern int isTierSet();
extern int setTierInfo(char tierValue[]);
extern char* getTierInfo();
extern int setControlIF();

extern void printIPPacketDetails(unsigned char ipPacket[], int nIPSize);
extern void printMPLRPacketDetails(unsigned char mplrPacket[], int nSize);
extern int CheckAllDestinationLabels(char* dest);
extern void printNeighbourTable();
extern struct addr_tuple* failedLL_head;
//extern void checkForLinkFailure();

void checkEntriesToAdvertise();
void checkEntriesToUpdate();
void checkForLinkFailures(struct addr_tuple*, int);
bool isInterfaceActive(struct in_addr, int);
void getMyTierAddresses();
void timestamp();
//cur_tm - added to reduce the hello timer from sec to millisec  - 4/22/2022- Samruddhi
struct timeval cur_tm;
void add_LL(char label[10]);

char* interfaceList[10];
int interfaceListSize;
int actionflag = 1;      //To dodge messages when an interface comes up after being down
char ip_add[20];
char specificLine[100];
char payLoadTier[100];
char tierDest[12] = "0.0";
time_t start_time;

time_t process_start_time;
time_t process_end_time;
double process_timeDiff;
struct timeval process_before, process_after;
struct addr_list* headaddr;
struct addr_tuple* tablehead;

char* tierAddr[20]; //this character array store the tier address
char* ipAddress[16]; //this character array store the ip address of ip node
int cidrs[100] = { 0 }; //subnet mask eg: 24
char* portName[10];
struct in_addr ips[100];
char ip_add[20];
// Structure to keep track of failed EndIPs
struct addr_tuple* failedEndIPs_head = NULL;
struct addr_tuple* failedAddr_head = NULL;
long long int MPLRCtrlSendCount = 0;
long long int MPLRDataSendCount = 0;
long long int MPLRDataFwdCount = 0;
long long int MPLRCtrlSendRound = 0;
long long int ipReceivedCount = 0;
long long int MPLRCtrlReceivedCount = 0;
long long int MPLRDataReceivedCount = 0;
long long int MPLRMsgVReceivedCount = 0;
long long int MPLROtherReceivedCount = 0;
long long int MPLRDataReceivedCountForMe = 0;
long long int MPLRDecapsulated = 0;
long long int errorCount = 0;

int totalIPError = 0;
int finalARGCValue = -100;
int endNode = -1;
FILE* fptr; //global variable. TO enable logs to be written to files 
int myTierValue = -1;
int tierAddrCount = 0;
int enableLogScreen = 1; //flag to set whether logs need to be shown in screen
int enableLogFiles = 0; //flag to set whether logs need to be shown in file - NS - set this to 0 on 3-28-20
bool recvdLabel = false;
int programStableFlag = 0;
struct labels {
	char label[100];
	struct labels* next;
};

// The labels allocated to each child node.
struct labels* allocatedLabels = NULL;

// To keep the track of the children to give new names.
int myChildCount = 0;

int freqCount(char str[], char search);

void signal_callbackZ_handler(int signum);
char* macAddrtoString(unsigned char* addr, char* strMAC, size_t size);

int trimAndDelNewLine();
char* strrmc(char* str, char ch);

int packetStats();
void printInputStats();

int setInterfaces();
int freeInterfaces();
int generateChildLabel(char* myEtherPort, int childTier, struct labels** labelList);
void addLabelsList(struct labels* labelList, char label[]);
void joinChildTierParentUIDInterface(char childLabel[], char myTierAddress[], char myEtherPort[]);
void printMyLabels();

void msgType1(char[2048], unsigned char*, char[5], struct sockaddr_ll);
void msgType2(char[2048], unsigned char*, char[5], struct sockaddr_ll, int*);
void msgType5(char[2048], unsigned char*, char[5], struct sockaddr_ll);
void msgType7(char[2048], unsigned char*, char[5], struct sockaddr_ll);
void msgType8(char[2048], unsigned char*, char[5], struct sockaddr_ll);
void msgType9(char[2048], unsigned char*, char[5], struct sockaddr_ll);
void msgType10(char[2048], unsigned char*, char[5], struct sockaddr_ll);
void msgType11(char[2048], unsigned char*, char[5], struct sockaddr_ll);
void msgType12(char[2048], unsigned char*, char[5], struct sockaddr_ll);
void msgType13(char[2048], unsigned char*, char[5], struct sockaddr_ll, int*);
void msgType14(char[2048], unsigned char*, char[5], struct sockaddr_ll);
void msgType15(char[2048], unsigned char*, char[5], struct sockaddr_ll);
void msgType16(char[2048], unsigned char*, char[5], struct sockaddr_ll, int*);

extern int myTotalTierAddress;

double time_diff(struct timeval before, struct timeval after)
{
	double x_ms, y_ms, diff;

	x_ms = (double)before.tv_sec * 1000000 + (double)before.tv_usec;
	y_ms = (double)after.tv_sec * 1000000 + (double)after.tv_usec;

	diff = (double)y_ms - (double)x_ms;

	return diff;
}

void clean_stdin(void)
{
	int c;
	do {
		c = getchar();
	} while (c != '\n' && c != EOF);
}
//boolean updateEndDestinationTierAddrHC(char tempIP[]) - changed this to send my label to requestIPResponse. 02/07/2022 -- Samruddhi 

boolean updateEndDestinationTierAddrHC(char tempIP[], char Mytier[]) {
	deleteList();
	boolean updateStatus = false;

	char* tempTier = updateEndTierAddr(tempIP, Mytier);

	if (tempTier != NULL) {

		strcpy(tierDest, tempTier);
		updateStatus = true;
	}

	/*
	*  // old logic
	*
	* if (strcmp(tempIP, "10.1.3.2") == 0) {
	memset(tierDest, '\0', 12);
	strcpy(tierDest, "1.1");
	updateStatus = true;
	} else {
	if (strcmp(tempIP, "10.1.2.3") == 0) {
	memset(tierDest, '\0', 12);
	strcpy(tierDest, "1.3");
	updateStatus = true;
	}
	}*/

	//printf("TEST: End Destination Tier Address : %s\n", tierDest);
	return updateStatus;
}
/**
* _get_MACTest(int,char[])
*
* method to send and receive MPLR, IP messages in one thread with timeout mechanism
*
* @param conditionVal (int) - condition for execution
* @param specialParam (char[]) - optinal parameters : separated by #
*
* @return status (int) - method return value
*/
int _get_MACTest(struct addr_tuple* myAddr) {

	if (enableLogScreen)
		printf("\n MNLR started  ... \n");
	if (enableLogFiles)
		fprintf(fptr, "\n MNLR started  ... \n");

	time_t time0 = time(0); //store the start time in time0, so that later we can compare
	time_t time1; //declare a new timer. time1 will be used later to compare with time0 to compare timings 

	//Below 4 lines are added to reduce the hello timer from sec to millisec  - 4/22/2022- Samruddhi
	double time00;
	double time01;
	gettimeofday(&cur_tm, NULL);
	time00 = ((double)cur_tm.tv_sec * 1000000 + (double)cur_tm.tv_usec) / 1000000;


	int sock, n; //declare 2 integer variables for sock
	int sockIP, nIP; //declare 2 integer variables for socketIp and nIO
	int sockUDP, nUDP;

	char buffer[2048];
	char bufferIP[2048];

	unsigned char* ethhead = NULL;
	unsigned char tempIP[1500];
	struct ether_addr ether;

	char recvOnEtherPort[5];
	char recvOnEtherPortIP[5];
	char MACString[18];

	struct sockaddr_ll src_addr;
	struct sockaddr_ll src_addrIP;

	recvOnEtherPort[5] = '\0';

	// Creating the MNLR CONTROL SOCKET HERE
	if ((sock = socket(AF_PACKET, SOCK_RAW, htons(0x8850))) < 0) {
		errorCount++;
		perror("ERROR: MNLR Socket ");
		printf("\nERROR: MNLR Socket ");

	}

	// Creating the MNLR IP SOCKET HERE
	if ((sockIP = socket(AF_PACKET, SOCK_RAW, htons(0x0800))) < 0) {
		errorCount++;
		perror("ERROR: IP Socket ");
		printf("\nERROR: IP Socket ");

	}

	//Create a UDP socket
	if ((sockUDP = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("ERROR: UDP Socket ");
		printf("\n ERROR: UDP Socket ");
	}

	char ctrlPayLoadA[200];
	memset(ctrlPayLoadA, '\0', 200);
	uint8_t numOfAddr = (uint8_t)getCountOfTierAddr();
	memcpy(ctrlPayLoadA, &numOfAddr, 1); // NS - memory of numAddr stored in ctrlPayLoadA
	int cpLength = 1;

	int q = 0;

	//size of the tier address and tier Address is copied to cntrlPayLoadA in the below for loop
	for (; q < numOfAddr; q++) {
		char tempAddrA[20];
		memset(tempAddrA, '\0', 20);
		strcpy(tempAddrA, getTierAddr(q)); //copy the qth tier address to tempAddrA
		freeGetTierAddr(); //free the tier address field

		uint8_t localTierSizeA = strlen(tempAddrA);

		memcpy(ctrlPayLoadA + cpLength, &localTierSizeA, 1);
		cpLength = cpLength + 1;
		memcpy(ctrlPayLoadA + cpLength, tempAddrA, localTierSizeA);
		cpLength = cpLength + localTierSizeA;
		printf("Control PayLoad %d --- %s", q, ctrlPayLoadA);
		// NS- must print this and check // ctrlPayLoadA can store only 200 char- could have been variable 
		// SM - Check the SOH and BEL issues in the log 
	}

	// Send should initiate before receive
	setInterfaces(); //interfaceList is being set by this function

	int loopCounter1 = 1;

	//this for loop sends the control message as well as keeps track of the no of control messages sent
	for (; loopCounter1 < interfaceListSize; loopCounter1++) {
		ctrlSend(interfaceList[loopCounter1], ctrlPayLoadA); //ctrlSend method is used to send Ethernet frame(list of all tier addresses)
		MPLRCtrlSendCount++; //keeps track of the no of control messages sent
	}
	MPLRCtrlSendRound++;

	time0 = time(0);

	//Below 2 lines are added to reduce the hello timer from sec to millisec - 4/22/2022- Samruddhi
	gettimeofday(&cur_tm, NULL);
	time00 = ((double)cur_tm.tv_sec * 1000000 + (double)cur_tm.tv_usec) / 1000000;

	printMyLabels();
	print_entries_LL();
	printf("\n Processing messages now...\n");

	char labelAssignmentPayLoad[200];

	setInterfaces();

	int cplength = 0;

	// Clearing the payload
	memset(labelAssignmentPayLoad, '\0', 200);

	// Setting the tierValue
	uint8_t tierValue = (uint8_t)myTierValue;
	memcpy(labelAssignmentPayLoad + cplength, &tierValue, 1);

	if (myTierValue != 1) {
		printf("\n Sending NULL join request to all its interfaces, "
			"interfaceListSize = %d payloadSize=%d", interfaceListSize, (int)strlen(labelAssignmentPayLoad));

		// Send NULL Join Message (Message Type, Tier Value) to all other nodes
		for (int i = 1; i < interfaceListSize; i++) {
			ctrlLabelSend(MESSAGE_TYPE_JOIN, interfaceList[i], labelAssignmentPayLoad);
		}
	}
	freeInterfaces();
	interfaceListSize = 0;
	process_start_time = time(0);



	// Repeats the steps from now on
	while (1) {

		process_end_time = time(0);

		double timeDiff = difftime(time1, time0);

		int flag = 0;
		int flagIP = 0;
		time1 = time(0);

		//Below 2 lines are added to reduce the hello timer from sec to millisec - 4/22/2022- Samruddhi
		gettimeofday(&cur_tm, NULL);
		time01 = ((double)cur_tm.tv_sec * 1000000 + (double)cur_tm.tv_usec) / 1000000;

		char ctrlPayLoadB[200];

		// KCT 01-19-2023
		// Adds back any entries for any endnodes that have become active again after failure
		// Also checks if we have any new end node link failure and removes entries from the tier 1
		// table for failed endnodes 
		if (!endNode) {
			// Adds entries to tier 1 for failed endnodes that became active and stores failed nodes into failedEndIPs_head
			checkForLinkFailures(myAddr, 0);

			// if we have new failed IPS then Advertise.
			if (failedEndIPs_head != NULL) {
				printf("I am here failed IP\n");

				setInterfaces(); //interfaceList is being set by this function

				int loopCounter2 = 1;
				int port_number = 0;

				uint8_t* mplrPayload = allocate_ustrmem(IP_MAXPACKET);
				int mplrPayloadLen = 0;

				// builds payload that is used for advertising TierAdd<->Ipaddress entries. - KCT 01-19-2023
				mplrPayloadLen = buildPayloadRemoveAdvts(mplrPayload, failedEndIPs_head, 2);
				if (mplrPayloadLen) {
					for (; loopCounter2 < interfaceListSize; loopCounter2++) {
						// MPLR TYPE 5.

						// KCT - 01-19-2023
						// matchPort return the corressponding port tag for each interface by comparing value of interfaceList
						// 1 is for same level neighbours
						// 2 is for child interface
						// 3 is for parent interface
						port_number = matchPort(interfaceList[loopCounter2]);
						if (port_number == 3) {
							// KCT 01-19-2023
							// sends endNW type message to parent interface to add TierAdd<->Ipaddress  
							// entries of the recovered end node to tier 1 table
							endNetworkSend(interfaceList[loopCounter2], mplrPayload, mplrPayloadLen);
							printf("sending V failed IP\n");
						}
					}
				}
				free(mplrPayload);
				freeInterfaces();
				interfaceListSize = 0;
			}
		} // end if (!endNode)

		checkEntriesToAdvertise();
		//checkEntriesToUpdate();

				/*
			Creating a hello message structure
			[Count_of_tier_addresses |Length_of_tier_address_1|Tier_Address_1| Length_of_tier_address_2|Tier_Address_2|..... ]
			numOfAddrB = Count_of_tier_addresses
			localTierSizeB = Length_of_a_tier_address
			tempAddrB = Tier_Address

			Pending - No need to send all the addresses evry time, need to work on that - 07/02/2021
		*/

		//Send hello packets after every 0.25 seconds
		if (time01 - time00 > 0.25) {  //Hello timer (from 1.2 to 0.25) and dead timer(from 1.3 to 0.35) reduced 4/20/2022 - Samruddhi

			memset(ctrlPayLoadB, '\0', 200);
			uint8_t numOfAddrB = getCountOfTierAddr();
			memcpy(ctrlPayLoadB, &numOfAddrB, 1);
			cpLength = 1;

			int qq = 0;
			for (; qq < getCountOfTierAddr(); qq++) {

				char tempAddrB[20];
				memset(tempAddrB, '\0', 20);
				strcpy(tempAddrB, getTierAddr(qq));

				freeGetTierAddr();

				uint8_t localTierSizeB = strlen(tempAddrB);

				memcpy(ctrlPayLoadB + cpLength, &localTierSizeB, 1);
				cpLength = cpLength + 1;
				memcpy(ctrlPayLoadB + cpLength, tempAddrB, localTierSizeB);
				cpLength = cpLength + localTierSizeB;

			}

			// Send on multiple interface in a loop
			setInterfaces(); //interfaceList is being set by this function

			int loopCounter2 = 1;

			for (; loopCounter2 < interfaceListSize; loopCounter2++) {
				ctrlSend(interfaceList[loopCounter2], ctrlPayLoadB); //send control messages
				MPLRCtrlSendCount++;
			}

			int ck = 0;
			time_t current_time = time(0);
			double delTimeDiff = difftime(current_time, start_time);

			if (delTimeDiff > 75) {
				ck = delete();
				programStableFlag = 1;
			}

			if (ck == 1) {
				printf("\nNeighbour Table after removing the addresses of the unreachable node");
				timestamp();
				printNeighbourTable();

				printf("\nMy Label Table after removing unreachable node");
				timestamp();
				printMyLabels();

				if (failedLL_head != NULL) {
					actionflag = 0;
					int loopCounter2 = 1;// dont send on eth0 hence changed it to 1
					uint8_t* mplrPayload = allocate_ustrmem(IP_MAXPACKET);
					int mplrPayloadLen = 0;
					int port_number = 0;
					mplrPayloadLen = buildPayloadRemoveAdvts(mplrPayload, failedLL_head, 4);
					if (mplrPayloadLen) {
						for (; loopCounter2 < interfaceListSize; loopCounter2++) {
							// MPLR TYPE 5.
							port_number = matchPort(interfaceList[loopCounter2]);
							if (port_number == 3) {
								endNetworkSend(interfaceList[loopCounter2], mplrPayload, mplrPayloadLen);
							}
							if (myTierValue == 1 && port_number == 1) {
								endNetworkSend(interfaceList[loopCounter2], mplrPayload, mplrPayloadLen);
							}
						}
					}
					free(mplrPayload);
					interfaceListSize = 0;
					delete_failed_LL_Addr(failedLL_head);
				}

				if (myTierValue == 1 || myTierValue == 3) {
					printf("\nUpdated Label List");
					timestamp();
					print_entries_LL();
				}
			}
			time0 = time(0); //reset time0
			gettimeofday(&cur_tm, NULL);
			time00 = ((double)cur_tm.tv_sec * 1000000 + (double)cur_tm.tv_usec) / 1000000;
			freeInterfaces();
			interfaceListSize = 0;
			MPLRCtrlSendRound++;
		}

		socklen_t addr_len = sizeof src_addr;
		socklen_t addr_lenIP = sizeof src_addrIP;

		// Recieve messages from the IP socket created.
		nIP = recvfrom(sockIP, bufferIP, 2048, MSG_DONTWAIT,
			(struct sockaddr*)&src_addrIP, &addr_lenIP); //inbuilt function to recieve information from socket. If there is any message in socket, we recieve a avalue greater than -1 and if there is no message, -1 will be returned.
		//NS - nIP has the packet size in bytes 

		if (nIP == -1) {
			flagIP = 1; //if no message is there in socket, nIP is -1 and flagIP is set to 1
		}

		// if some messages are available in the IP socket.
		if (flagIP == 0) { //if message is recived from socket flagIP remains 0 and we enter the loop

			unsigned int tcIP = src_addrIP.sll_ifindex;

			if_indextoname(tcIP, recvOnEtherPortIP); //if_indextoname() function returns the name of the network interface corresponding to the interface index ifindex

			// Fix for GENI , Ignoring messages from control interface
			char* ctrlInterface = "eth0";

			if (strcmp(recvOnEtherPortIP, ctrlInterface) == 0) { //eth0 is the control interface. We do not need process any messages on eth0
				continue;
			}

			if (ctrlIFName != NULL) {

				if ((strncmp(recvOnEtherPortIP, ctrlIFName, strlen(ctrlIFName))
					!= 0)) {

					ipReceivedCount++;
					if (enableLogScreen) {
						printf("\n*******************************************************************************");
						printf("\nTEST: IP Packet Received \n");
						printf("\n");
					}

					unsigned char* ipHeadWithPayload;
					int ipPacketSize = nIP - 14; // NS- remove MAC header
					ipHeadWithPayload = (unsigned char*)malloc(ipPacketSize);
					memset(ipHeadWithPayload, '\0', ipPacketSize);
					memcpy(ipHeadWithPayload, &bufferIP[14], ipPacketSize);
					//NS ipPakcet  now stored in ipHeadWithPayload

					setInterfaces(); //interfaceList is being set by this function

					unsigned char ipDestTemp[20];
					memset(ipDestTemp, '\0', 20);
					sprintf(ipDestTemp, "%u.%u.%u.%u", ipHeadWithPayload[16],
						ipHeadWithPayload[17], ipHeadWithPayload[18],
						ipHeadWithPayload[19]);  // NS destination IP address is stored in ipDestTemp
					if (enableLogScreen)
						printf("IP Destination : %s  \n", ipDestTemp);

					unsigned char ipSourceTemp[20];
					memset(ipSourceTemp, '\0', 20);
					sprintf(ipSourceTemp, "%u.%u.%u.%u", ipHeadWithPayload[12],
						ipHeadWithPayload[13], ipHeadWithPayload[14],
						ipHeadWithPayload[15]); // NS source IP address is stored in ipSourceTemp
					if (enableLogScreen)
						printf("IP Source  : %s  \n", ipSourceTemp);

					if (enableLogScreen)
						printf("Calling Forwarding Algorithm - DataSend\n");

					//Add Request-Response for getting destination tier address linked to the IP- Samruddhi -- 1/26/2022

					int packetFwdStatus = -1; // NS this variable is set here 

					if (isTierSet() == 0) {
						if (enableLogScreen)
							printf("%s: isTierSet() == 0", __FUNCTION__);

						boolean checkDestStatus =
							updateEndDestinationTierAddrHC(ipDestTemp, headTL->tier); // NS find tier address of destination router connected to dest IP network

						if (checkDestStatus == false) {
							errorCount++;
							if (enableLogScreen)
								printf("ERROR: End destination tier address not available 590 \n");

						}
						///NS - whay call packet forwarding algorithm if End destination tier address not available
						else {
							printf("\n Calling packetForwardAlgorithm linenumber =%d", __LINE__);
							setTierInfo(headTL->tier);
							packetFwdStatus = packetForwardAlgorithm(tierAddress,
								tierDest); //forward the packets using packetforwarding algorithm
						}
					}
					else {
						if (enableLogScreen)
							printf("ERROR: Tier info was not set \n");
						printf("\n Calling packetForwardAlgorithm linenumber =%d", __LINE__);
						setTierInfo(headTL->tier);
						packetFwdStatus = packetForwardAlgorithm(tierAddress,
							tierDest);//forward the packets using packetforwarding algorithm
					}
					if (enableLogScreen)
						printf("%s: packetFwdStatus = %d \n", __FUNCTION__, packetFwdStatus);

					if (packetFwdStatus == 0) { //if we are able to move forward, get inside the loop
						if (enableLogScreen)
							printf("%s: packetFwdStatus == 0", __FUNCTION__);

						if ((strlen(fwdTierAddr) == strlen(tierAddress))
							&& (strcmp(fwdTierAddr, tierAddress) == 0)) {
							if (enableLogScreen)
								printf("TEST: Received IP packet -(it's for me only, no forwarding needed) \n");

						}
						else {
							if (enableLogScreen)
								printf("TEST: Recieved IP packet is not for me \n");

							if (isFWDFieldsSet() == 0) {
								if (enableLogScreen) {
									printf("TEST: Forwarding IP Packet as MNLR Data Packet (Encapsulation) \n");
									printf("TEST: Sending on interface: %s \n", fwdInterface);
								}
								dataSend(fwdInterface, ipHeadWithPayload,
									tierDest, tierAddress, ipPacketSize); //send the data

								MPLRDataSendCount++;
							}
						}
					}

					freeInterfaces();
					interfaceListSize = 0;

				}
			}
			else {
				ipReceivedCount++;

				if (enableLogScreen) {
					printf("TEST: IP Packet Received \n");
					printf("\n");
				}

				unsigned char* ipHeadWithPayload;
				int ipPacketSize = nIP - 14;
				ipHeadWithPayload = (unsigned char*)malloc(ipPacketSize);
				memset(ipHeadWithPayload, '\0', ipPacketSize);
				memcpy(ipHeadWithPayload, &bufferIP[14], ipPacketSize);

				printf("\n");

				setInterfaces(); //interfaceList is being set by this function

				unsigned char ipDestTemp[20];
				sprintf(ipDestTemp, "%u.%u.%u.%u", ipHeadWithPayload[16],
					ipHeadWithPayload[17], ipHeadWithPayload[18],
					ipHeadWithPayload[19]);
				printf("[2]\nIP Destination : %s  \n", ipDestTemp);

				unsigned char ipSourceTemp[20];
				memset(ipSourceTemp, '\0', 20);
				sprintf(ipSourceTemp, "%u.%u.%u.%u", ipHeadWithPayload[12],
					ipHeadWithPayload[13], ipHeadWithPayload[14],
					ipHeadWithPayload[15]);
				if (enableLogFiles)
					printf("IP Source  : %s  \n", ipSourceTemp);

				printf("Calling Forwarding Algorithm - DataSend\n");

				// TESTING FWD ALGORITHM 2 - Case: IP Packet Received, Control Interface not set

				int packetFwdStatus = -1;

				if (isTierSet() == 0) {
					boolean checkDestStatus = updateEndDestinationTierAddrHC(ipDestTemp, headTL->tier);

					if (checkDestStatus == false) {
						errorCount++;
						printf("\n End destination tier address not available in the node \n");

					}
					else {
						printf("\n Calling packetForwardAlgorithm linenumber =%d", __LINE__);

						setTierInfo(headTL->tier);

						packetFwdStatus = packetForwardAlgorithm(tierAddress,
							tierDest);
						printf("**interface: %s \n", fwdInterface);
					}
				}
				else {
					printf("ERROR: Tier info was not set\n");

					printf("\n Calling packetForwardAlgorithm linenumber =%d", __LINE__);
					setTierInfo(headTL->tier);
					packetFwdStatus = packetForwardAlgorithm(tierAddress,
						tierDest);
				}

				if (packetFwdStatus == 0) {
					if ((strlen(fwdTierAddr) == strlen(tierAddress))
						&& (strcmp(fwdTierAddr, tierAddress) == 0)) {
						printf("TEST: Received IP packet -(it's for me only, no forwarding needed) \n");
					}
					else {
						if (isFWDFieldsSet() == 0) {
							printf("TEST: Forwarding IP Packet as MNLR Data Packet (Encapsulation) \n");
							printf("Sending on interface: %s \n", fwdInterface);

							dataSend(fwdInterface, ipHeadWithPayload, tierDest,
								tierAddress, ipPacketSize);

							MPLRDataSendCount++;
						}
					}
				}
				freeInterfaces();
				interfaceListSize = 0;
			}
		}

		// Receive messages from the control socket.
		n = recvfrom(sock, buffer, 2048, MSG_DONTWAIT,
			(struct sockaddr*)&src_addr, &addr_len); //recvfrom - receive a message from a socket

		if (n == -1) {
			flag = 1;
		}

		// if some message is present in the control socket.
		if (flag == 0) {

			unsigned int tc = src_addr.sll_ifindex;

			if_indextoname(tc, recvOnEtherPort); //mappings between network interface names and indexes

			ethhead = (unsigned char*)buffer;

			if (ethhead != NULL) {
				MPLROtherReceivedCount++;

				uint8_t checkMSGType = (ethhead[14]);

				/*
				Checking for different type of MNLR messages

				0x01 = Hello Message

				0x02 = Encapsulated IP Message

				0x05 = IP to Tier Address Mappping message
					Actions under message 0x05:
						action == MESSAGE_TYPE_ENDNW_INTER_IP
						action == MESSAGE_TYPE_ENDNW_ADD
						action == MESSAGE_TYPE_ENDNW_UPDATE
						action == MESSAGE_TYPE_ENDNW_REMOVE
						action == MESSAGE_TYPE_ENDNW_REMOVE_ADDR
						action == MESSAGE_TYPE_ENDNW_REMOVE
						action == MESSAGE_TYPE_ENDNW_REMOVE_ADDR

				Below ones are the newly added ones for auto naming.
				0x07 = MESSAGE_TYPE_JOIN
				0x08 = MESSAGE_TYPE_LABELS_AVAILABLE
				0x09 = MESSAGE_TYPE_LABELS_ACCEPTED
				0x10 = MESSAGE_TYPE_MY_LABELS_ADD
				0X11 = MESSAGE_TYPE_MY_LABELS_DELETE
				0x12 = REQUEST-IP-RESOLVE (from T3 to T1 node)
				0x13 = RESPONSE-IP-RESOLVE (from T1 to T3 node)
				0X14 = MESSAGE_TYPE_LABELS_LOST
				0x15 = MESSAGE_TYPE_PUBLISH_IP_ADD : sent by T1 node
				0x16 = MESSAGE_TYPE_PUBLISH_IP_DELETE : sent by T1 node
				*********************************************
				*/

				printf("\n%s : checkMSGType=%d\n", __FUNCTION__, checkMSGType);

				if (strcmp(recvOnEtherPort, "eth0") != 0) {
					switch (checkMSGType) {

					case MESSAGE_TYPE_CTRL:
						msgType1(buffer, ethhead, recvOnEtherPort, src_addr);
						break;

					case MESSAGE_TYPE_DATA:
						msgType2(buffer, ethhead, recvOnEtherPort, src_addr, &n);
						break;

					case MESSAGE_TYPE_ENDNW:
						msgType5(buffer, ethhead, recvOnEtherPort, src_addr);
						break;

					case MESSAGE_TYPE_JOIN:
						msgType7(buffer, ethhead, recvOnEtherPort, src_addr);
						break;

					case MESSAGE_TYPE_LABELS_AVAILABLE:
						msgType8(buffer, ethhead, recvOnEtherPort, src_addr);
						break;

					case MESSAGE_TYPE_LABELS_ACCEPTED:
						msgType9(buffer, ethhead, recvOnEtherPort, src_addr);
						break;

					case MESSAGE_TYPE_MY_LABELS_ADD:
						msgType10(buffer, ethhead, recvOnEtherPort, src_addr);
						break;

					case MESSAGE_TYPE_MY_LABELS_DELETE:
						msgType11(buffer, ethhead, recvOnEtherPort, src_addr);
						break;

					case MESSAGE_TYPE_REQUEST_IP_RESOLVE:
						msgType12(buffer, ethhead, recvOnEtherPort, src_addr);
						break;

					case MESSAGE_TYPE_RESPONSE_IP_RESOLVE:
						msgType13(buffer, ethhead, recvOnEtherPort, src_addr, &n);
						break;

					case MESSAGE_TYPE_LABELS_LOST:
						msgType14(buffer, ethhead, recvOnEtherPort, src_addr);
						break;

					case MESSAGE_TYPE_PUBLISH_IP_DELETE:
						msgType15(buffer, ethhead, recvOnEtherPort, src_addr);
						break;

					case MESSAGE_TYPE_PUBLISH_IP_ADD:
						msgType16(buffer, ethhead, recvOnEtherPort, src_addr, &n);
						break;

					}
				}
			}
		}
	}
	return 0;
}

/**
* setInterfaces() //interfaceList is being set by this function
*
* method to set active interfaces in a interfaceList
*
* @return status (int) - method return value
*/
int setInterfaces() {

	if (isEnvSet() != 0)
		setControlIF();

	struct ifaddrs* ifaddr, * ifa;
	int family;

	int countInterface = 0;
	char temp[20];

	if (getifaddrs(&ifaddr) == -1) { /*The getifaddrs() function creates a linked list of structures
	describing the network interfaces of the local system, and stores the
	address of the first item of the list in *ifap.*/
		errorCount++;
		perror("ERROR: getifaddrs");
		exit(EXIT_FAILURE);
	}

	int init = 0;

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL)
			continue;

		family = ifa->ifa_addr->sa_family;

		if (family == AF_PACKET) {

			strcpy(temp, ifa->ifa_name);

			// testing whether interface is up or not
			if ((ifa->ifa_flags & IFF_UP) == 0) {
				//if interface is down
			}

			else {

				if ((ifa->ifa_flags & IFF_LOOPBACK) == 0) {

					if (ctrlIFName != NULL) {

						if (strlen(temp) == strlen(ctrlIFName)) {

							if ((strncmp(temp, ctrlIFName, strlen(ctrlIFName))
								!= 0)) {

								countInterface = countInterface + 1;
								interfaceList[init] = malloc(1 + strlen(temp));
								strcpy(interfaceList[init], temp);
								init = init + 1;

							}

						}
						else {

							countInterface = countInterface + 1;
							interfaceList[init] = malloc(1 + strlen(temp));
							strcpy(interfaceList[init], temp);
							init = init + 1;

						}

					}
					else {

						countInterface = countInterface + 1;
						interfaceList[init] = malloc(1 + strlen(temp));
						strcpy(interfaceList[init], temp);
						init = init + 1;

					}
				}
			}

		}
	}

	interfaceListSize = countInterface;

	freeifaddrs(ifaddr);
	return 0;
}

/**
* freeInterfaces()
*
* method to free interfaceList
*
* @return status (int) - method return value
*/
int freeInterfaces() {
	int loopCounter1 = 0;
	for (; loopCounter1 < interfaceListSize; loopCounter1++) {
		free(interfaceList[loopCounter1]);
	}
	interfaceListSize = 0;
	return 0;
}

/**
* convertStringToInteger()
*
* Method which converts the passed string into integer.
*
* @return status (int) - method returns integer value of the string.
*/

int convertStringToInteger(char* num)
{
	int result = 0, i = 0;
	int len = strlen(num);
	for (i = 0; i < len; i++) {
		result = result * 10 + (num[i] - '0');
	}
	//printf("%d", result);
	return result;
}


/*
*  EntryPoint - main() - Each node starts execution here
*/
int main(int argc, char** argv) {

	/*
	The 2 arguments are argc, which is the no of arguments, and argv, which is a pointer of pointers to each of the arguments

	E.g., ./run -T 2.1.2 -N 0 10.10.5.2 24 eth1. 7 components are present in this argument, hence argc is 7

	The arguments are:
	1) -T - stands for tier label
	2)2.1.2 - tier label address
	3)-N - stands for Node
	4)0 - 0 specifies edge node and 1 specifies non edge node
	5)10.10.5.2 - ip address of the ip node
	6)24 - subnet mask
	7)eth1 - interface through which the ip node is connected to edge node.
	*/

	//As the argument comes as a pointer of pointers. we need to split and save them to following variables:
	//char *tierAddr[20]; //this character array store the tier address 
	//char *ipAddress[16]; //this character array store the ip address of ip node
	//int cidrs[100] = { 0 }; //subnet mask eg: 24
	//char *portName[10]; //interface no or port no eg : \eth0

	int endNWCount = 0;
	start_time = time(0);

	//Code to write to log file
	gettimeofday(&process_before, NULL);
	// RVP For Logging purpose
	char* filename = "./testLogs.txt"; //The filename to which logs is being saved.
	fptr = fopen(filename, "w");
	if (enableLogFiles) fprintf(fptr, " Writing logs to file ... \n");
	fflush(fptr);

	if (argc > 1) { //only if the number of arguments is greater than 1, we will go forward. Otherwise exit code execution

		argc--; //decrement the counter
		argv++; //increment the memory location. first value in argv is "./run". Now we incremented to "-T" 

		char* opt = argv[0]; //declare a pointer and assign the first pointer in "argv" to "opt". {*argv = argv[0]}. "-T"  assigned to opt

		// Checking for -T and storing Tier in myTierValue
		if (strcmp(opt, "-T") == 0) {
			argc--;
			argv++;	//argv is now on Tier Value. E.g., -T 2. argv is on '2'.
			myTierValue = convertStringToInteger(argv[0]);
			argc--;
			argv++; //argv now points to the next argument
		}
		else {
			printf(" Error : Invalid Argument , Tier Value is not passed ");
			exit(0);
		}
		timestamp();
		printf("\nMy Tier Value = %d\n", myTierValue);

		// Checking only if the current node belongs to Tier 1 
		// All other nodes will be named automatically based on the Tier 1 node.

		if (myTierValue == 1)
		{
			opt = argv[0];
			// Checking for -L
			if (strcmp(opt, "-L") == 0) {
				argc--;
				argv++;

				//initA is not used except for the value 0. I think there is no need of variable - Team KCT 01/02/2023
				//int initA = 0;

				char* next = argv[0];

				// setTierInfo() can be removed as it does the same work as insertTierAddr(), but it currently sets values 
				// that are used in multiple locations. All of them need to be altered and then remove setTierInfo
				// - Team KCT 01/02/2023

				//function to set the tier address of the node. we are passing the tier label address(eg 1.1) as the argument.
				setTierInfo(next);

				//pass it to list of tier addresses of the node. Here it is core router so only a single Tier Addr is inserted.
				insertTierAddr(next);
				tierAddr[0] = malloc(1 + strlen(next));
				strcpy(tierAddr[0], next);
				//initA++;
				argc--;
				argv++;
				printf("tierAddr of Tier 1 Node = %s\n", tierAddr[0]);
			}
			else {
				printf(" Error : Invalid Argument , Label not assigned to Tier 1 Router ");
				exit(0);
			}
		}

		// Adding the interface and the edge node IP to the local lists.
		if (argc > 0) {
			opt = argv[0];
			// Checking whether the node is an edge node or an intermediate node.
			if (strcmp(opt, "-N") == 0) {
				argc--;
				argv++;

				//atoi converts string to integer. Here we checks what comes after the -N (0 or 1 
				//comes after -N. 0 represent edge not and 1 represents not an edge node)
				int edgeNode = atoi(argv[0]);
				int initB = 0;
				argc--;
				argv++;
				if (edgeNode == 0) { //if 0 comes aftrer -N, we are having an edge node and we enter this block
					// Only for Edge Node
					endNode = 0; //endNode is updated to 0, if we are at edge node
					int iterationNCount = 0;

					printf("Getting IP../\n\n");
					do {
						char* nextN = argv[0]; //here the ipaddress is stored in the pointer

						if (nextN[0] == '-') {
							if (iterationNCount == 0) {
								totalIPError++;
							}
							break;
						}

						// pass it to other - IP
						ipAddress[initB] = malloc(1 + strlen(nextN)); //allocate memory for the ipaddress[initB]
						strcpy(ipAddress[initB], nextN); //copy the ipaddress of the ip node to ipaddress[initB]
						argc--;
						argv++;

						char* nextN2 = argv[0]; //here the subnet mask is saved to the pointer 

						if (nextN2[0] == '-') {
							totalIPError++;
							break;
						}

						cidrs[initB] = atoi(nextN2); //convert the subnet mask to integer and store in cidrs[initB]
						argc--;
						argv++;

						char* nextN3 = argv[0]; //interface is saved in the pointer

						if (nextN3[0] == '-') {
							totalIPError++;
							break;
						}

						// pass it to other - Port name
						portName[initB] = malloc(1 + strlen(nextN3)); //allocate memory for portName[initB]
						strcpy(portName[initB], nextN3); //copy the interface to portName[initB]
						initB++;
						endNWCount++;
						argc--;
						argv++;
						iterationNCount++;

						printf("IP = %s\nSubnet Mask = %s\nPort Name = %s\n\n\n", ipAddress[initB], cidrs[initB], portName[initB]);

					} while (argc > 0);
				}
				else {
					//  skip till '-' encountered
					while (argc > 0) {
						char* skipNext = argv[0];
						if (skipNext[0] == '-') {
							break;
						}
						argc--;
						argv++;
					}
				}
			}
			else {
				printf(" Error : Invalid Argument , value for -N not provided ");
				exit(0);
			}
		}

		if (argc > 0) {
			printf("ERROR: Too many parameters \n");
			exit(0);
		}
	}
	else { //if argc <=0 then error
		printf("ERROR: Not enough parameters \n");
		exit(0);
	}
	finalARGCValue = argc;

	// Converting IPV4 or IPV6 addresses from text to binary format.
	// Doing it for all the edge IP addresses.

	//only tier 3 will be connected to ip node - Team KCT 12-22-2022
	printf("Converting IP to binary...\n\n");
	if (myTierValue == 3) {
		int z = 0;
		for (z = 0; z < endNWCount; z++) {
			inet_pton(AF_INET, ipAddress[z], &ips[z]); //inet_pton convert ipaddress from text to binary. ips[z] will have the binary form of the ip address stored as text in ipaddress[z]
			ips[z].s_addr = ntohl(ips[z].s_addr); //The ntohl() function converts the unsigned integer netlong from network byte order to host byte order.
			ips[z].s_addr = ((ips[z].s_addr >> (32 - cidrs[z])) << (32 - cidrs[z])); //remove the last element from ipaddress. eg 10.10.5.2 will change to 10.10.5.0. we are finding the network address from the host address.
			ips[z].s_addr = htonl(ips[z].s_addr); //The htonl() function converts the unsigned integer hostlong from host byte order to network byte order.
		}
		sprintf(ip_add, "%s", inet_ntoa(ips[0]));
	}

	if (myTierValue != 1) {
		setInterfaces(); //gets list of interfaces except control interface to send null join message through - KCT 12-22-2022
		getMyTierAddresses(tierAddr); //send null join message across all interfaces to get labels from parent - KCT 12-22-2022
	}

	struct addr_tuple* myAddr;

	//Storing Tier Address, IP Address, Subnet Mask and Port in myAddr
	if (endNode == 0) {

		int index1, index2;
		int numTemp = 0;

		// if there are multiple IPS then do number of end IPS * tier addresses entries here.
		for (index1 = 0; index1 < myTotalTierAddress; index1++) {
			for (index2 = 0; index2 < endNWCount; index2++) {
				struct addr_tuple* myAddr = (struct addr_tuple*)malloc(sizeof(struct addr_tuple));
				strcpy(myAddr->tier_addr, tierAddr[index1]);  // myaddr is a pointer that points to the first element in the array i.e. 

				myAddr->ip_addr.s_addr = (unsigned long int) malloc(sizeof(unsigned long int));
				memset(&myAddr->ip_addr.s_addr, '\0', sizeof(unsigned long int));
				myAddr->ip_addr.s_addr = ips[index2].s_addr;  // NS is this correct? - memcopy?

				char* temp11;

				myAddr->cidr = (uint8_t)cidrs[index2];

				strcpy(myAddr->etherPortName, portName[index2]);

				// Add code for port number
				myAddr->if_index = -1;
				myAddr->isNew = true;
				myAddr->next = NULL;

				//comment add_entry_LL as mapping needs to be saved only on tier1 -- Samruddhi 1/24/2022
				add_entry_LL(myAddr);
			}
		}
		printf("----------------------------------------\n");
		printf("End of main()... Starting _get_MACTest()\n");
		printf("----------------------------------------");
		// Activating protocol to be ready
		_get_MACTest(myAddr);
	}
	else {
		printf("----------------------------------------\n");
		printf("End of main()... Starting _get_MACTest()\n");
		printf("----------------------------------------");
		// Activating protocol to be ready
		_get_MACTest(NULL);
	}

	return 0;
}

/**
* freqCount(char[],char[])
*
* method to find count of particular character
*
* @param str (char[]) - string
* @param search (char[]) - character to find occurrence
*
* @return count (int) - occurrence count of a character
*/
int freqCount(char str[], char search) {

	int i, count = 0;
	for (i = 0; str[i] != '\0'; ++i) {
		if (str[i] == search)
			count++;
	}

	return count;
}

// Function just to print the details based on the signal.

void signal_callbackZ_handler(int signum) {

	//printf("\n");
	//printf("TEST: Caught signal Z - %d \n", signum);
	//printf("\n");

	// Cleanup and close up stuff here

	// calling another function
	displayTierAddr();
	displayNeighbor();
	printInputStats();
	packetStats();

	// Terminate program
	exit(signum);
}

char* macAddrtoString(unsigned char* addr, char* strMAC, size_t size) {
	if (addr == NULL || strMAC == NULL || size < 18)
		return NULL;

	snprintf(strMAC, size, "%02x:%02x:%02x:%02x:%02x:%02x", addr[0], addr[1],
		addr[2], addr[3], addr[4], addr[5]);

	return strMAC;
}

int trimAndDelNewLine() {

	size_t len = strlen(specificLine);
	if (len > 0) {
		if (isspace(specificLine[len - 1]) || specificLine[len - 1] == '\n') {
			//printf("TEST: Inside trim: if of if:%c\n", specificLine[len - 1]);
			len = len - 1;
			specificLine[len] = '\0';
		}

	}
	return 0;
}

//removes the specified character 'ch' from 'str'
char* strrmc(char* str, char ch) {
	char* from, * to;
	from = to = str;
	while (*from) {
		if (*from == ch)
			++from;
		else
			*to++ = *from++;
	}
	*to = '\0';
	return str;
}
/*function to update the end tier address*/

void printInputStats() {

	//printf("\n");
	//printf("\n");
	//printf("TEST: Printing Input Parameters related stats \n");
	//printf("TEST: Final value of ARGC : %d \n", finalARGCValue);
	//printf("TEST:     Total I/P Error : %d \n", totalIPError);
	//printf("\n");
}

int packetStats() {

	//printf(" \n");
	//printf("TEST:                    Current Node Tier Address : %s\n",
	//		tierAddress);
	//printf("TEST:                     Rounds of MPLR Ctrl sent : %lld\n",
	//		MPLR   Round);
	//printf("TEST:                       MPLR Ctrl Packets sent : %lld\n",
	//		MPLRCtrlSendCount);
	//printf("TEST:                       MPLR Data Packets sent : %lld\n",
	//		MPLRDataSendCount);
	//printf("TEST:                  MPLR Data Packets forwarded : %lld\n",
	//		MPLRDataFwdCount);
	//printf("TEST:                          IP Packets received : %lld\n",
	//		ipReceivedCount);
	//printf("TEST:                   MPLR Ctrl Packets received : %lld\n",
	//		MPLRCtrlReceivedCount);
	//printf("TEST:                  MPLR MSG V Packets received : %lld\n",
	//		MPLRMsgVReceivedCount);
	//printf("TEST:                   MPLR Data Packets received : %lld\n",
	//		MPLRDataReceivedCount);
	//printf("TEST:    MPLR Data Packets received (Egress Node ) : %lld\n",
	//		MPLRDataReceivedCountForMe);
	//printf("TEST:                     MPLR Packet Decapsulated : %lld\n",
	//		MPLRDecapsulated);
	//printf("TEST:      MPLR Packet received (MSG Type not 1/2) : %lld\n",
	//		MPLROtherReceivedCount);
	//printf("TEST:                    Error Count (Since start) : %lld\n",
	//		errorCount);
	//printf("TEST:                       Control interface name : %s\n",
	//		ctrlIFName);
	return 0;
}

void checkEntriesToAdvertise() {
	setInterfaces(); //interfaceList is being set by this function
	int ifs = 1;
	int port_number = 0;
	// Whenever there is an update we have to advertise it to others.
	for (; ifs < interfaceListSize; ifs++) {
		// If new port we have to advertise our tierAdd<->IPAdd table.
		uint8_t* mplrPayload = allocate_ustrmem(IP_MAXPACKET);
		int mplrPayloadLen = 0;
		//NS - buildPayload defined in endNW.c  
		mplrPayloadLen = buildPayload(mplrPayload, ONLY_NEW_ENTRIES, (int)if_nametoindex(interfaceList[ifs]));
		if (mplrPayloadLen) {
			//printf("\nin checkEntriesToAdvertise");
			port_number = matchPort(interfaceList[ifs]);
			//printf("-----------%d",port_number);
			if (port_number == 3) {
				endNetworkSend(interfaceList[ifs], mplrPayload, mplrPayloadLen);
			}
		}
		free(mplrPayload);

	}
	freeInterfaces();
	interfaceListSize = 0;

	// Mark new entries as old now.
	clearEntryState();
}

/*
checkEntriesToSync()
Input : No Input
Output : No return value

This function SYNCs the IP to label map in tier 1 nodes
*/

void checkEntriesToSync() {
	//printf("\nI am Syncing..\n");
	setInterfaces(); //interfaceList is being set by this function
	int ifs = 1;
	int port_number = 0;
	// Whenever there is an update we have to advertise it to others.
	for (; ifs < interfaceListSize; ifs++) {
		// If new port we have to advertise our tierAdd<->IPAdd table.
		uint8_t* mplrPayload = allocate_ustrmem(IP_MAXPACKET);
		int mplrPayloadLen = 0;
		//NS - buildPayload defined in endNW.c  
		mplrPayloadLen = buildPayload(mplrPayload, ONLY_NEW_ENTRIES, (int)if_nametoindex(interfaceList[ifs]));
		if (mplrPayloadLen) {

			port_number = matchPort(interfaceList[ifs]);
			//printf("-----------%d",port_number);
			if (port_number == 1) {
				//printf("\nin checkEntriesToSync");
				endNetworkSend(interfaceList[ifs], mplrPayload, mplrPayloadLen);
			}
		}
		free(mplrPayload);

	}
	freeInterfaces();
	interfaceListSize = 0;

	// Mark new entries as old now.
	clearEntryState();
}

/*

*/


void checkEntriesToUpdate() {
	struct addr_tuple* current = failedLL_head;

	while (current != NULL) {
		struct addr_tuple* freeptr;
		if (compare(current->tier_addr) == 1) {


			struct addr_tuple* temp = (struct addr_tuple*)calloc(1, sizeof(struct addr_tuple));
			strcpy(temp->tier_addr, current->tier_addr);

			temp->ip_addr.s_addr = (unsigned long int) malloc(sizeof(unsigned long int));
			memset(&temp->ip_addr.s_addr, '\0', sizeof(temp->ip_addr.s_addr));
			//printf("temp->etherPortName = %s, current->etherPortName = %s", temp->etherPortName, current->etherPortName);
			strcpy(temp->etherPortName, current->etherPortName);

			temp->if_index = -1;
			temp->isNew = true;

			memcpy(&temp->ip_addr, &current->ip_addr, sizeof(struct in_addr));
			temp->cidr = current->cidr;
			//print_entries_LL();
			//add_entry_LL(temp);
			delete_failed_LL_Addr(temp->tier_addr);
			printf("\nUpdated Label List");
			//print_entries_LL();

			insertTierAddr(temp->tier_addr);
			printf("\nUpdated MyLabel List");
			//printMyLabels();
		}
		current = current->next;
	}
}

//Send NULL Join Request //called in helloList.h
void sendNullJoin(int to_port[20]) {
	char labelAssignmentPayLoad[200];
	setInterfaces();
	int cplength = 0;
	// Clearing the payload

	memset(labelAssignmentPayLoad, '\0', 200);

	// Setting the tierValue

	uint8_t tierValue = (uint8_t)myTierValue;

	memcpy(labelAssignmentPayLoad + cplength, &tierValue, 1);

	printf("\n Sending NULL join request to recovered interface, "
		"interfaceListSize = %d payloadSize=%d", interfaceListSize, (int)strlen(labelAssignmentPayLoad));
	// Send NULL Join Message (Message Type, Tier Value) to all other nodes

	//for (int i =1;i < interfaceListSize; i++) {
		//printf("Interface List : %s",interfaceList[i]);
	ctrlLabelSend(MESSAGE_TYPE_JOIN, to_port, labelAssignmentPayLoad);
	//}

	freeInterfaces();
	interfaceListSize = 0;
}


/*
This function is used to populate the failed IP's to the variable "failedEndIPs_head". addr_tuple is being populated by the active interfaces.
*/
void checkForLinkFailures(struct addr_tuple* myAddr, int numTierAddr) {
	// Failed End Links IPS

	struct addr_tuple* current = failedEndIPs_head;
	struct addr_tuple* previous = NULL;

	// Check if any failed End IPS became active.
	while (current != NULL) {

		if (isInterfaceActive(current->ip_addr, current->cidr)) {

			// Add into the table as they are active.
			struct addr_tuple* a = (struct addr_tuple*)calloc(1, sizeof(struct addr_tuple));
			strcpy(a->tier_addr, current->tier_addr);
			// insert info about index from which the packet has been received from.
			a->if_index = -1;
			a->isNew = true;
			memcpy(&a->ip_addr, &current->ip_addr, sizeof(struct in_addr));
			a->cidr = current->cidr;
			add_entry_LL(a);
			print_entries_LL();

			struct addr_tuple* freeptr;
			if (current == failedEndIPs_head) {
				failedEndIPs_head = current->next;
				previous = NULL;
				freeptr = current;
				current = NULL;
			}
			else {
				previous->next = current->next;
				freeptr = current;
				current = current->next;
			}
			free(freeptr);
			continue;
		}
		previous = current;
		current = current->next;
	}
	int  i = 0;
	for (; i < numTierAddr; i++) {
		// and check to see if address is already added to failed IP list or not.
		struct addr_tuple* ptr = failedEndIPs_head;
		bool isInFailedList = false;

		while (ptr != NULL) {
			if ((myAddr[i].ip_addr.s_addr == ptr->ip_addr.s_addr) && (strcmp(myAddr[i].tier_addr, ptr->tier_addr) == 0)) {
				isInFailedList = true;
				break;
			}
			ptr = ptr->next;
		}
		//printf("* %u *",myAddr[i].cidr);
		// if interface is not active, add to failed IP list.
		if ((!isInFailedList) && (!isInterfaceActive(myAddr[i].ip_addr, myAddr[i].cidr))) {
			struct addr_tuple* temp = (struct addr_tuple*)calloc(1, sizeof(struct addr_tuple));
			memcpy(temp, &myAddr[i], sizeof(struct addr_tuple));
			temp->isNew = true;
			temp->next = NULL;
			delete_entry_LL_IP(myAddr[i].ip_addr);
			if (failedEndIPs_head == NULL) {
				failedEndIPs_head = temp;
				//printf("Will be removing this %s\n", inet_ntoa(failedEndIPs_head->ip_addr));
			}
			else {
				temp->next = failedEndIPs_head;
				failedEndIPs_head = temp;
			}
		}
	}

}

void delete_failedEndIPs(uint8_t* tier_addr) {

	if (failedEndIPs_head == NULL) {
		printf("failedEndIPs_head is NULL");
		//return false;
	}
	else {
		struct addr_tuple* current = failedEndIPs_head;
		struct addr_tuple* prev = failedEndIPs_head;
		while (current != NULL) {

			if (strcmp(tier_addr, current->tier_addr) == 0) {

				if (failedEndIPs_head == current) {
					failedEndIPs_head = failedEndIPs_head->next;
					free(current);
					current = failedEndIPs_head;
					continue;
				}
				else {
					prev->next = current->next;
					free(current);
					current = prev;
				}
			}
			prev = current;
			current = current->next;
		}
	}
}


/*
This funtion is used to find all interfaces in a node and check whether interfaces are active
*/
bool isInterfaceActive(struct in_addr ip, int cidr) {
	// find all interfaces on the node.
	struct ifaddrs* ifaddr, * ifa;

	if (getifaddrs(&ifaddr)) { /* The getifaddrs() function creates a linked list of structures describing the network interfaces of the local system, and stores the address of the first item of the list in *ifap. */
		perror("Error: getifaddrs() Failed\n");
		if (enableLogScreen)
			printf("Is Interface Active\n");
		if (enableLogFiles)
			fprintf(fptr, "Is Interface Active\n");

		exit(0);
	}

	// loop through the list
	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL) {
			continue;
		}
		int family;
		family = ifa->ifa_addr->sa_family;

		// populate interface names, if interface is UP and if ethernet interface doesn't belong to control interface and Loopback interface.
		if (family == AF_INET && (strncmp(ifa->ifa_name, "lo", 2) != 0) && (ifa->ifa_flags & IFF_UP) != 0) {

			struct sockaddr_in* ipaddr = ((struct sockaddr_in*)ifa->ifa_addr);
			struct sockaddr_in* subnmask = ((struct sockaddr_in*)ifa->ifa_netmask);

			if (ip.s_addr == (ipaddr->sin_addr.s_addr & subnmask->sin_addr.s_addr)) {
				freeifaddrs(ifaddr);
				return true;
			}

		}
	}
	freeifaddrs(ifaddr);
	return false;
}


/**
* getMyTierAddresses()
*
* Method to obtain label for each node.
* The label is received from nodes in the higher level by sending the JOIN requests.
* The nodes at higher level will send a list of available labels which can be used by this node.
* It will accept all the labels and sends the LABELS_ACCEPTED message back to the node from which
* it recieved the labels.
*
* @return status (void) - method returns none.
*/

void getMyTierAddresses(char* tierAddr[])
{
	printf("\n Entering %s", __FUNCTION__);

	// Creating a socket here for auto addressing and related variables.
	int sock;
	struct sockaddr_ll src_addr;
	socklen_t addr_len = sizeof src_addr;
	char buffer[2048];
	unsigned char* ethhead = NULL;
	int n;
	char recvOnEtherPort[5];


	// Creating the MNLR CONTROL SOCKET HERE
	if ((sock = socket(AF_PACKET, SOCK_RAW, htons(0x8850))) < 0) {
		printf("\nERROR: MNLR Socket ");
	}
	printf("\n Created a socket for auto address label! ");

	// Checking whether label is set or not
	// If not set, will wait here till the label is set

	while (!recvdLabel)
	{
		int i = 0;
		// Form the NULL join message here
		char labelAssignmentPayLoad[200];
		int cplength = 0;
		// Clearing the payload
		memset(labelAssignmentPayLoad, '\0', 200);

		// Setting the tierValue
		uint8_t tierValue = (uint8_t)myTierValue;
		memcpy(labelAssignmentPayLoad + cplength, &tierValue, 1);

		// Wait for 2 seconds
		sleep(1);

		printf("\n Sending NULL join request to all its interfaces, "
			"interfaceListSize = %d  payloadSize=%d", interfaceListSize, (int)strlen(labelAssignmentPayLoad));
		// Send NULL Join Message (Message Type, Tier Value) to all other nodes
		for (i = 1; i < interfaceListSize; i++) {
			ctrlLabelSend(MESSAGE_TYPE_JOIN, interfaceList[i], labelAssignmentPayLoad);
		}
		timestamp();

		printf("\n Waiting for MESSAGE_TYPE_LABELS_AVAILABLE message");
		// wait for addresses for some time
		n = recvfrom(sock, buffer, 2048, MSG_DONTWAIT,
			(struct sockaddr*)&src_addr, &addr_len);
		if (n == -1) {
			printf("\n Timeout happened, NO MESSAGE_TYPE_LABELS_AVAILABLE\n");
		}
		else {

			unsigned int tc = src_addr.sll_ifindex;
			if_indextoname(tc, recvOnEtherPort);
			//printf("\n Control message recvd from %s \n",recvOnEtherPort); // uncomment it later
			ethhead = (unsigned char*)buffer;


			if (ethhead == NULL) {
				printf("\n AutoLabel recieved message is empty \n");
			}
			else {


				uint8_t checkMSGType = (ethhead[14]);
				//printf("\n checkMSGType=%d \n",checkMSGType); //uncomment it later



				//   checkMSGType = (ethhead[15]);
				//   printf("\n checkMSGType=%d \n",checkMSGType);

				if (checkMSGType == MESSAGE_TYPE_LABELS_AVAILABLE) {
					printf("\n Received MESSAGE_TYPE_LABELS_AVAILABLE_2 \n");

					int numberLabels = ethhead[15];
					printf("\n Number of Labels available = %d", numberLabels);
					int i = 0;
					struct labels* acceptedList;

					//to_parent port tag 
					int match_Port = matchPort(recvOnEtherPort);
					if (match_Port == 0) {
						tagport(recvOnEtherPort, 3);
					}


					// Accepting each label here
					int messagePointer = 16;
					for (; i < numberLabels; i++) {

						int labelLength = ethhead[messagePointer];
						printf("\n Label Length = %d\n", labelLength);

						messagePointer++;

						char label[10];
						memcpy(label, ethhead + messagePointer, labelLength);
						printf("\n Label  = %s  Label length= %d\n", label, (int)strlen(label));

						messagePointer = messagePointer + labelLength;

						if (!recvdLabel) {
							recvdLabel = true;
							//set the label here
							/* For Default tier address */
							setTierInfo(label);
						}

						// pass it to tier address list
						insertTierAddr(label);

						tierAddr[i] = malloc(1 + strlen(label));
						strcpy(tierAddr[i], label);
						// tierAddrCount++;

						printf("\nAdding the label to the list\n");

						//creating a list of accepted labels
						if (!acceptedList) {
							printf("\nAdding the first label to the list\n");
							acceptedList = (struct labels*)malloc(sizeof(struct labels));
							strncpy(acceptedList->label, label, strlen(label));
							acceptedList->next = NULL;
							notify_myLabels_Update(1, label);
						}
						else {
							printf("\nAdding all other labels to the list\n %s", label);
							struct labels* newLabel = (struct labels*)malloc(sizeof(struct labels));
							strncpy(newLabel->label, label, strlen(label));
							newLabel->next = acceptedList;
							acceptedList = newLabel;
							notify_myLabels_Update(1, label);
						}
						checkEntriesToAdvertise();

					}

					// Generate Labels Accepted Message
					cplength = 0;

					// Clearing the payload
					memset(labelAssignmentPayLoad, '\0', 200);

					// Set tbe labels here .. TO be done
					uint8_t numLabels = (uint8_t)numberLabels;
					memcpy(labelAssignmentPayLoad + cplength, &numLabels, 1);
					cplength++;

					struct labels* temp = acceptedList;


					// Copy all other labels to the labels accepted message
					while (!temp) {

						char label[10];
						memcpy(label, temp->label, strlen(temp->label));

						// Set tbe labellength here
						uint8_t labelLength = strlen(label);
						memcpy(labelAssignmentPayLoad + cplength, &numLabels, 1);
						cplength++;

						// Set tbe label here
						memcpy(labelAssignmentPayLoad + cplength, label, labelLength);
						cplength = cplength + labelLength;
					}



					// Sending labels accepted message
					ctrlLabelSend(MESSAGE_TYPE_LABELS_ACCEPTED, recvOnEtherPort, labelAssignmentPayLoad);
					// int loopCounter1 = 1;
					// printf("\ninterfaceListSize - %d ",interfaceListSize);
					// for (; loopCounter1 < interfaceListSize; loopCounter1++) {
					// 	ctrlLabelSend(MESSAGE_TYPE_MY_LABELS_ADD, interfaceList[loopCounter1], labelAssignmentPayLoad); //ctrlSend method is used to send Ethernet frame
					// 	printf("\nSending MESSAGE_TYPE_MY_LABELS_ADD message to - %s at ",interfaceList[loopCounter1]);
					// }
					printMyLabels();
				}

				// Add timeout for wait
				// Get the addresses
				// Add it to the local table
				// Send the LABELS accepted update to the node from which it got the message
				// Break the loop,when you get labels from at-least two different nodes */

			}
		}

	}

	shutdown(sock, 2);
	printf("\n Exiting %s", __FUNCTION__);
}

/**
* addLabelsList()
*
* Add the passed label to the passed list
*
* @return status (void) - method returns none.
*/

void addLabelsList(struct labels* labelList, char label[]) {

	printf("\nEnter %s, Label to be added : %s\n", __FUNCTION__, label);
	struct labels* newlabel = (struct labels*)malloc(sizeof(struct labels));
	memcpy(newlabel->label, label, strlen(label));
	newlabel->next = NULL;
	printf("\n Created the label %p \n", labelList);

	if (!labelList) {
		printf("\n labellist is null now \n");
		labelList = newlabel;
	}
	else {
		struct labels* temp = labelList;
		while (temp->next != NULL) {
			temp = temp->next;
		}
		temp->next = newlabel;
	}
	printf("\nExit %s", __FUNCTION__);
}


void notify_lostmychild(char port[20]) {
	if (myTierValue == 2) {
		printf("\nI am at tier 2 and lost my child\n");
		struct labels* lostlabelList;
		int numblostLabels = generateChildLabel(port, myTierValue + 1, &lostlabelList);
		printf("\n%d Labels Lost\n", numblostLabels);

		// Generate the payload for sending this message.

		// Form the delete message here
		char labelDeletePayLoad[200];
		int cplength = 0;
		// Clearing the payload
		memset(labelDeletePayLoad, '\0', 200);

		printf("\n %s : Creating the payload to send the lost labels %s\n", __FUNCTION__, lostlabelList->label);
		// printf("\n %s : Setting the number of labels in the payload \n",__FUNCTION__);
			// Setting the number of labels being send
		uint8_t numberOfLabels = (uint8_t)numblostLabels; // Need to modify
		memcpy(labelDeletePayLoad + cplength, &numberOfLabels, 1);
		printf("\n %s : labelDeletePayLoad(string) = %x", __FUNCTION__, *labelDeletePayLoad);// NS to fix 
		cplength++;

		struct labels* temp = lostlabelList;
		while (temp != NULL) {
			printf("\n label list is not null\n");
			printf("\n current label in the message = %s\n", temp->label);
			// printf("\n %s : Setting the label length in the payload \n", __FUNCTION__);
				// Setting the label length being send
			uint8_t labelLength = (uint8_t)strlen(temp->label); // Need to modify
			memcpy(labelDeletePayLoad + cplength, &labelLength, 1);
			cplength++;

			//printf("\n %s : Setting the label in the payload \n", __FUNCTION__);
			// Setting the labels being send
			// Need to modify
			memcpy(labelDeletePayLoad + cplength, temp->label, labelLength);
			cplength = cplength + labelLength;
			temp = temp->next;
			printf("\n %s : GETTING THE NEXT LABEL \n", __FUNCTION__);

		}
		/*
		printf("\n MESSAGE BEING SENT = %s",labelDeletePayLoad);
		printf("\n MESSAGE LENGTH = %d \n",(int)strlen(labelDeletePayLoad));
		/*int i;
		for(i= 0 ; i < 200; i++){ //NS to fix  200?
			printf("%c",labelDeletePayLoad[i]);  //NS to fix
		}*/ // nS commented on 4-13-20 

		//  printf("\n MESSAGE BEING SEND = %s",labelDeletePayLoad);
		setInterfaces();
		int port_number = 0;
		printf("\n Sending MESSAGE_TYPE_LABELS_LOST  to all the parent interfaces, "
			"interfaceListSize = %d payloadSize=%d \n", interfaceListSize,
			(int)strlen(labelDeletePayLoad));
		int ifs = 1;
		for (; ifs < interfaceListSize; ifs++) {
			port_number = matchPort(interfaceList[ifs]);
			if (port_number == 3) {
				// Send MESSAGE_TYPE_LABELS_AVAILABLE  (Message Type, Tier Value) to all other nodes
				ctrlLabelSend(MESSAGE_TYPE_LABELS_LOST, interfaceList[ifs], labelDeletePayLoad);
			}
		}

		freeInterfaces();
		interfaceListSize = 0;
		//else { 
		// 	printf("\nPort is not stable, label will be generated once port is stable");
		// }	

	}
}

/**
* generateChildLabel()
*
* Generate the child labels with respect to all available labels for the current node.
* Joins the Tier Value of the child , UID of each parent label and the interface to which the parent is connected.
*
* @return status (int) - method return the number of labels created.
*/

int generateChildLabel(char* myEtherPort, int childTier, struct labels** labelList) {

	printf("\n\n Inside %s", __FUNCTION__);
	printf("\n Child port = %s", myEtherPort);

	//samruddhi
	//portTag struct update and lisnked list
	//tag = 2 : to_child port
	int match_Port = matchPort(myEtherPort);
	if (match_Port == 0) {

		tagport(myEtherPort, 2);
	}
	// Get each tier address and create the labels here

	char* myTierAddress = getTierInfo();
	int countNewLabels = 0;
	//printf("\n My TierAddress = %s", myTierAddress);

	// Creating the label for the child here
	char childLabel[10];
	memset(childLabel, '\0', 10);
	sprintf(childLabel, "%d", childTier);

	struct nodeTL* temp = headTL;
	*labelList = NULL;

	printf("\n Going through each of my tier addresses , primary address = %s\n", temp->tier);


	while (temp) {

		countNewLabels++;
		printf("\n Current TierAddress = %s \n", temp->tier);
		memset(childLabel, '\0', 10);
		sprintf(childLabel, "%d", childTier);
		// Creating the child Label here
		joinChildTierParentUIDInterface(childLabel, temp->tier, myEtherPort);

		struct labels* newLabel = (struct labels*)malloc(sizeof(struct labels));

		printf("\n childLabel = %s  strlen(childLabel)=%d", childLabel, (int)strlen(childLabel));
		memcpy(newLabel->label, childLabel, strlen(childLabel) + 1);
		printf("\n newLabel->label = %s  strlen(newLabel->label)=%d", childLabel, (int)strlen(newLabel->label));

		newLabel->next = *labelList;
		*labelList = newLabel;
		printf("\n Created the new child label = %s\n", newLabel->label);
		temp = temp->next;
	}

	printf("\n Exit: %s , Number of labels generated = %d \n", __FUNCTION__, countNewLabels);
	//printf("\n Exit: %s , Number of labels generated = %d  labelList= %s\n",__FUNCTION__,countNewLabels,(*labelList)->label); NS commented on 4-14.20
	return countNewLabels;

}


/**
* notify_myLabels_Update()
*
* method to let other node know that I have a label added or deleted.
*
* @return status (void) - method return none
*/



void notify_myLabels_Update(int update_action, char inTier[20]) {
	printf("\ninTier : %s", inTier);
	char ctrlPayLoadC[200];
	memset(ctrlPayLoadC, '\0', 200);
	uint8_t numOfAddr = 1;//**1 samruddhi check
	memcpy(ctrlPayLoadC, &numOfAddr, 1); // NS - memory of numAddr stored in ctrlPayLoadC
	int cpLength = 1;


	char tempAddrA[20];
	memset(tempAddrA, '\0', 20);
	strcpy(tempAddrA, inTier); //copy the t th tier address to tempAddrA

	uint8_t localTierSizeA = strlen(tempAddrA);
	//printf("TEST:(helloM_List) ctrl packet - localTierSizeA: %u  \n",
		//	localTierSizeA);
	memcpy(ctrlPayLoadC + cpLength, &localTierSizeA, 1); // copying the tier address size
	cpLength = cpLength + 1;
	memcpy(ctrlPayLoadC + cpLength, tempAddrA, localTierSizeA); // copying the tier address
	cpLength = cpLength + localTierSizeA;
	// printf("PayLoad --- %s",ctrlPayLoadC); 
   // NS- must print this and check // ctrlPayLoadC can store only 200 char- could have been variable 
   // SM - Check the SOH and BEL issues in the log 
   //printf("\nctrlPayLoadC : %s", ctrlPayLoadC);

// Send should initiate before receive
	setInterfaces(); //interfaceList is being set by this function

	if (update_action == 1) {
		int loopCounter1 = 1;
		//this for loop sends the control message as well as keeps track of the no of control messages sent
		for (; loopCounter1 < interfaceListSize; loopCounter1++) {
			//if(findStable(interfaceList[loopCounter1])==0){
			ctrlLabelSend(MESSAGE_TYPE_MY_LABELS_ADD, interfaceList[loopCounter1], ctrlPayLoadC); //ctrlSend method is used to send Ethernet frame
			MPLRCtrlSendCount++; //keeps track of the no of control messages sent
			printf("\nSending MESSAGE_TYPE_MY_LABELS_ADD message to - %s at ", interfaceList[loopCounter1]);
			//}

		}
	}
	else {
		int loopCounter1 = 1;

		//this for loop sends the control message as well as keeps track of the no of control messages sent
		for (; loopCounter1 < interfaceListSize; loopCounter1++) {
			//if(findStable(interfaceList[loopCounter1])==0){
			ctrlLabelSend(MESSAGE_TYPE_MY_LABELS_DELETE, interfaceList[loopCounter1], ctrlPayLoadC); //ctrlSend method is used to send Ethernet frame
			MPLRCtrlSendCount++; //keeps track of the no of control messages sent
			printf("\nSending MESSAGE_TYPE_MY_LABELS_DELETE message to - %s at ", interfaceList[loopCounter1]);
			//}
		}
	}

	freeInterfaces();
	interfaceListSize = 0;

}

/**
* requestIPresolve()
*
* method to request tier1 node for IP resolve.
*
* @return status (void) - method return none
*/



void requestIPresolve(char destinationInterfaceIPAddr[], char Mytier[]) {

	printf("\ndestinationInterfaceIPAddr : %s", destinationInterfaceIPAddr);
	char ctrlPayLoadC[200];
	memset(ctrlPayLoadC, '\0', 200);
	uint8_t numOfAddr = 1;//**1 samruddhi check
	memcpy(ctrlPayLoadC, &numOfAddr, 1); // NS - memory of numAddr stored in ctrlPayLoadC
	int cpLength = 1;

	//For MyTier -- 02/07/2022
	char tempMyAddr[20];
	memset(tempMyAddr, '\0', 20);
	strcpy(tempMyAddr, Mytier); //copy mytier to temp

	uint8_t localTierSize = strlen(tempMyAddr);

	memcpy(ctrlPayLoadC + cpLength, &localTierSize, 1); // copying the tier address size
	cpLength = cpLength + 1;
	memcpy(ctrlPayLoadC + cpLength, tempMyAddr, localTierSize); // copying the tier address
	cpLength = cpLength + localTierSize;
	//-----------------------------------------------------------------------------------------

	char tempAddrA[20];
	memset(tempAddrA, '\0', 20);
	strcpy(tempAddrA, destinationInterfaceIPAddr); //copy the t th tier address to tempAddrA

	uint8_t localTierSizeA = strlen(tempAddrA);
	//printf("TEST:(helloM_List) ctrl packet - localTierSizeA: %u  \n",
		//	localTierSizeA);
	memcpy(ctrlPayLoadC + cpLength, &localTierSizeA, 1); // copying the tier address size
	cpLength = cpLength + 1;
	memcpy(ctrlPayLoadC + cpLength, tempAddrA, localTierSizeA); // copying the tier address
	cpLength = cpLength + localTierSizeA;


	//  printf("PayLoad --- %s",ctrlPayLoadC); 
	// NS- must print this and check // ctrlPayLoadC can store only 200 char- could have been variable 
	// SM - Check the SOH and BEL issues in the log 
	//printf("\nctrlPayLoadC : %s", ctrlPayLoadC);

	// Send should initiate before receive
	setInterfaces(); //interfaceList is being set by this function


	//this for loop sends the control message as well as keeps track of the no of control messages sent
	char parentTierAdd[20];
	memset(parentTierAdd, '\0', 20);
	findParntLongst(tierAddress, parentTierAdd);
	struct nodeHL* fNode = headHL;
	char eth_addr[20];
	if (fNode == NULL) {
		if (enableLogScreen) {
			printf("\nERROR: Neighbor List is empty \n");
		}
	}
	// traverse the list
	// testing
	while (fNode != NULL) {
		//while (fNode->next != NULL) {
		if ((strlen(fNode->tier) == strlen(parentTierAdd))
			&& ((strncmp(fNode->tier, parentTierAdd, strlen(parentTierAdd)) == 0))) {
			strcpy(eth_addr, fNode->port);
			break;
		}
		else {
			fNode = fNode->next;
		}

	}


	ctrlLabelSend(MESSAGE_TYPE_REQUEST_IP_RESOLVE, eth_addr, ctrlPayLoadC); //ctrlSend method is used to send Ethernet frame
	MPLRCtrlSendCount++; //keeps track of the no of control messages sent
	printf("\nSending MESSAGE_TYPE_REQUEST_IP_RESOLVE message to - %s ", eth_addr);





	// struct in_addr ip;
	// if (inet_pton(AF_INET, destinationInterfaceIPAddr, &ip) == -1) { //inet_pton - convert IPv4 and IPv6 addresses from text to binary form
	// 	if(enableLogScreen)
	// 		printf("Error: inet_pton() returned error");
	// 	if(enableLogFiles)
	// 		fprintf(fptr,"Error: inet_pton() returned error");
	// }

	// uint8_t *mplrPayload = allocate_ustrmem(IP_MAXPACKET);
	// int mplrPayloadLen = 0;
	// // buildPayload defined in endNW.c  
	// mplrPayloadLen = buildIPResolvePacket(mplrPayload, ip);

	// setInterfaces(); //interfaceList is being set by this function
	// int ifs = 1;
	// int port_number = 0;


	// if (mplrPayloadLen) {

	// 	for(; ifs < interfaceListSize; ifs++){
	// 		port_number= matchPort(interfaceList[ifs]);
	// 		//printf("-----------%d",port_number);
	// 		if(port_number==3){

	// 			printf("\nForwarding resolve request to : %s", interfaceList[ifs]);
	// 			endNetworkSend(interfaceList[ifs], mplrPayload, mplrPayloadLen);
	// 		}
	// 	}
	// free(mplrPayload);

	// }
	freeInterfaces();
	interfaceListSize = 0;

}

/**
* responseIPresolve()
*
* method to response tier3 node as a result of request IP resolve.
	params : resolved label, resolved ip, return to which label.
*
* @return status (void) - method return none
*/


void responseIPresolve(char tieraddr[], struct in_addr ip_addr, char DestTierLabel[], uint8_t entries, int isnew) {

	uint8_t* mplrPayload = allocate_ustrmem(IP_MAXPACKET);
	int mplrPayloadLen = 0;
	// buildPayload defined in endNW.c  
	// printf("\nSending buildIPResolvePacket : %s", tieraddr);
	int tierValue = 0;
	if (myTierValue == 1) {
		tierValue = 1;
	}// can be optimized -- to do Samruddhi 4/15/2022
	if (isnew == 1) {
		mplrPayloadLen = buildIPResolvePacket(mplrPayload, DestTierLabel, tierValue);
	}
	// }else{
	// 	mplrPayloadLen = buildIPResolveFwdPacket(mplrPayload,DestTierLabel,tierValue,entries);
	// }
	//printf("\nipAddr: %s", inet_ntoa(ip_addr));
	setInterfaces(); //interfaceList is being set by this function
	int ifs = 1;
	int port_number = 0;

	//printf("buildIPResolvePacket out::\n");		
	if (mplrPayloadLen) {
		//printf("I have mplrPayloadLen %s\n",mplrPayload);

		char parentofDest[20];
		memset(parentofDest, '\0', 20);
		char eth_addr1[20];
		//printf("\nI am here  %s\n", DestTierLabel);		
		findParentLongest(DestTierLabel, parentofDest);
		//printf("\nParent : %s\n", parentofDest);
		struct nodeHL* fNode1 = headHL;


		if (fNode1 == NULL) {
			if (enableLogScreen) {
				printf("\nERROR: Neighbor List is empty \n");
			}
		}
		// traverse the list
		// testing
		while (fNode1 != NULL) {
			//while (fNode->next != NULL) {
			if ((strlen(fNode1->tier) == strlen(parentofDest)) && ((strncmp(fNode1->tier, parentofDest, strlen(parentofDest)) == 0))) {
				strcpy(eth_addr1, fNode1->port);
				printf("\nForwarding resolve response to : %s on %s", fNode1->tier, eth_addr1);
				endNetworkSend(eth_addr1, mplrPayload, mplrPayloadLen);
				return;
			}

			fNode1 = fNode1->next;

		}




		// for(; ifs < interfaceListSize; ifs++){
		// 	port_number= matchPort(interfaceList[ifs]);
		// 	//printf("-----------%d",port_number);
		// 	if(port_number==2){
		// 		printf("\nForwarding resolve response to : %s", interfaceList[ifs]);
		// 		endNetworkSend(interfaceList[ifs], mplrPayload, mplrPayloadLen);
		// 	}
		// }

		free(mplrPayload);

	}
	freeInterfaces();
	interfaceListSize = 0;
	return;
}

//checkIfSubstr(large,small)
void deleteIPLabel(char failedEth[]) {

	if (myTierValue == 1) {
		//Samruddhi - need to generalize this. newaddress[3] is defined to save tier 1's grandchild sublabel.
		//if eth3 from 1.1 is down I need to delete IP addresses with substr as 1.3
		struct addr_tuple* current = tablehead;
		char* myTierAddress = getTierInfo();

		char newaddress[10] = ".";
		//strcpy(newaddress,myTierAddress);
		strcpy(newaddress, "2.");
		strcat(newaddress, myTierAddress + 2);
		strcat(newaddress, ".");
		strcat(newaddress, failedEth + 3);
		printf("\n newaddress = %s\n", newaddress);
		while (current != NULL) {
			if (checkIfGrandParent(current->tier_addr, newaddress)) {
				printf("Found grand child %s\n", current->tier_addr);
				modify_LL(current->tier_addr);
				delete_entry_LL_Addr(current->tier_addr);
			}
			current = current->next;
		}

		if (failedLL_head != NULL) {
			//printf("\nin if (failedLL_head != NULL) 3600\n");
			setInterfaces(); //interfaceList is being set by this function


			int loopCounter2 = 1;// dont send on eth0 hence changed it to 1
			uint8_t* mplrPayload = allocate_ustrmem(IP_MAXPACKET);
			int mplrPayloadLen = 0;
			int port_number = 0;
			mplrPayloadLen = buildPayloadRemoveAdvts(mplrPayload, failedLL_head, 4);
			//printf("\nPayload :  : %s",mplrPayload); 
			if (mplrPayloadLen) {
				for (; loopCounter2 < interfaceListSize; loopCounter2++) {
					// MPLR TYPE 5.
					port_number = matchPort(interfaceList[loopCounter2]);
					if (port_number == 1) {
						//printf("I am sending remove to %s\n",interfaceList[loopCounter2]);
						endNetworkSend(interfaceList[loopCounter2], mplrPayload, mplrPayloadLen);
					}
				}
			}
			free(mplrPayload);
			//print_entries_LL();
			freeInterfaces();
			interfaceListSize = 0;
		}
	}
}

/*
publishIPLabelMap()
Input : Label that needs to be published, and action (int)
	action : 1 - add
	action : 2 - delete
Output : No return value

This function creates the payload as per the labels provided and sends to other child nodes
*/

void publishIPLabelMap(char label[], int action) {
	//action : 1 - add
	//action : 2 - delete
	// if(myTierValue==1){
	if (action == 1) {
		printf("\npublishIPLabelMap : label : %s", label);
		char ctrlPayLoadC[200];
		memset(ctrlPayLoadC, '\0', 200);
		uint8_t numOfAddr = 1;//**1 samruddhi check
		memcpy(ctrlPayLoadC, &numOfAddr, 1); // NS - memory of numAddr stored in ctrlPayLoadC
		int cpLength = 1;


		char tempAddrA[20];
		memset(tempAddrA, '\0', 20);
		strcpy(tempAddrA, label); //copy the t th tier address to tempAddrA

		uint8_t localTierSizeA = strlen(tempAddrA);
		//printf("TEST:(helloM_List) ctrl packet - localTierSizeA: %u  \n",
			//	localTierSizeA);
		memcpy(ctrlPayLoadC + cpLength, &localTierSizeA, 1); // copying the tier address size
		cpLength = cpLength + 1;
		memcpy(ctrlPayLoadC + cpLength, tempAddrA, localTierSizeA); // copying the tier address
		cpLength = cpLength + localTierSizeA;
		// printf("PayLoad --- %s",ctrlPayLoadC); 
		// NS- must print this and check // ctrlPayLoadC can store only 200 char- could have been variable 
		// SM - Check the SOH and BEL issues in the log 
		//printf("\nctrlPayLoadC : %s", ctrlPayLoadC);

		// Send should initiate before receive
		setInterfaces(); //interfaceList is being set by this function


		int loopCounter1 = 1;

		//this for loop sends the control message as well as keeps track of the no of control messages sent
		for (; loopCounter1 < interfaceListSize; loopCounter1++) {
			int match_Port = matchPort(interfaceList[loopCounter1]);
			if (match_Port == 2) {
				ctrlLabelSend(MESSAGE_TYPE_PUBLISH_IP_DELETE, interfaceList[loopCounter1], ctrlPayLoadC); //ctrlSend method is used to send Ethernet frame
				MPLRCtrlSendCount++; //keeps track of the no of control messages sent
				printf("\nSending MESSAGE_TYPE_PUBLISH_IP_DELETE message to - %s at ", interfaceList[loopCounter1]);
			}

		}
		interfaceListSize = 0;
	}
	if (action == 2) {
		uint8_t* mplrPayload = allocate_ustrmem(IP_MAXPACKET);
		int mplrPayloadLen = 0;
		setInterfaces();
		mplrPayloadLen = buildIPPublishPacket(mplrPayload, label);
		//printf("mplrPayloadLen: %d\n",mplrPayloadLen);
		if (mplrPayloadLen) {

			int loopCounter2 = 1;

			for (; loopCounter2 < interfaceListSize; loopCounter2++) {
				// MPLR TYPE 5.

				int port_number = matchPort(interfaceList[loopCounter2]);

				if (port_number == 2) {

					printf("\nSending MESSAGE_TYPE_PUBLISH_IP_ADD message to - %s at ", interfaceList[loopCounter2]);
					endNetworkSend(interfaceList[loopCounter2], mplrPayload, mplrPayloadLen);
				}
			}
		}
		interfaceListSize = 0;
		free(mplrPayload);
	}
	//}

}

void findParentofDestination(char DestTierLabel[], char eth_addr1[], char* parentDest) {
	char* parentofDest;
	parentofDest = getParent(DestTierLabel, '.');
	//not coming here 02/11/2022
	printf("\nGot Parent : %s", parentofDest);
	struct nodeHL* fNode1 = headHL;


	if (fNode1 == NULL) {
		if (enableLogScreen) {
			printf("\nERROR: Neighbor List is empty \n");
		}
	}
	// traverse the list
	// testing
	while (fNode1 != NULL) {
		//while (fNode->next != NULL) {
		if ((strlen(fNode1->tier) == strlen(parentofDest)) && ((strncmp(fNode1->tier, parentofDest, strlen(parentofDest)) == 0))) {
			strcpy(eth_addr1, fNode1->port);
			strcpy(parentDest, parentofDest);
			return;
			break;
		}
		else {
			fNode1 = fNode1->next;
		}

	}

	return;
}


/**
* joinChildTierParentUIDInterface()
*
* method to join the Tier Value of the child , UID of the parent and the interface to which the parent is connected.
*
* @return status (void) - method return none
*/

void joinChildTierParentUIDInterface(char childLabel[], char myTierAddress[], char myEtherPort[]) {
	printf("\n My TierAddress = %s", myTierAddress);
	//Getting the UID of the label of hte current address.
	int i = 0;
	while (myTierAddress[i] != '.') {
		i++;
	}
	int curLengthChildLabel = strlen(childLabel);
	strcpy(childLabel + curLengthChildLabel, myTierAddress + i);
	printf("\n After appending the uid of the parent, childLabel = %s ", childLabel);

	char temp[10] = ".";
	strcpy(temp + 1, myEtherPort + 3);
	printf("\n temp = %s", temp);

	curLengthChildLabel = strlen(childLabel);
	strcpy(childLabel + curLengthChildLabel, temp);

	printf("\n After appending the port ID, ChildLabel = %s", childLabel);
}


/**
* printMyLabels()
*
* method to print my labels.
*
* @return status (void) - method return none
*/

void printMyLabels() {
	struct nodeTL* temp = headTL;
	printf("\n My labels are: ");
	while (temp->next != NULL) {
		printf(" %s ,", temp->tier);
		temp = temp->next;
	}
	printf("%s ", temp->tier);
	printf("\n");
	return;
}

/**
* Temporary
* printAddresstoResponse() for printing addr_list
* method to print my labels.
*
* @return status (void) - method return none
*/

void printAddresstoResponse() {
	struct addr_list* temp = headaddr;
	if (temp == NULL) {
		printf("\n My Response labels are: No labels in response\n");
	}
	else {
		printf("\n My Response labels are: \n");
		while (temp->next != NULL) {
			printf(" %s ,", temp->label);
			temp = temp->next;
		}
		printf("%s \n", temp->label);
	}
	return;
}



void add_LL(char label[10]) {
	int endNWCount = 1;
	int myTotalTierAddress = 1;
	if (endNode == 0) { //endNode is updated to 0, if we are at edge node, ie if edgeNode is 0

		int index1, index2;
		int numTemp = 0;

		for (index1 = 0; index1 < myTotalTierAddress; index1++) {
			for (index2 = 0; index2 < endNWCount; index2++) {
				struct addr_tuple* myAddr1 = (struct addr_tuple*)malloc(sizeof(struct addr_tuple));

				strcpy(myAddr1->tier_addr, label);  // myaddr is a pointer that points to the first elment in the array i.e. 

				myAddr1->ip_addr.s_addr = (unsigned long int) malloc(sizeof(unsigned long int));
				memset(&myAddr1->ip_addr.s_addr, '\0', sizeof(myAddr1->ip_addr.s_addr));
				myAddr1->ip_addr.s_addr = ips[index2].s_addr;  // NS is this correct? - memcopy?
				char* temp11;

				myAddr1->cidr = (uint8_t)cidrs[index2];

				strcpy(myAddr1->etherPortName, portName[index2]);

				myAddr1->if_index = -1;
				myAddr1->isNew = true;
				myAddr1->next == NULL;
				add_entry_LL(myAddr1);

				printf("%d", numTemp);

			}
		}
	}
	return;
}

//===========================================
//MESSAGE TYPES
//===========================================

void msgType1(char buffer[2048], unsigned char* ethhead, char recvOnEtherPort[5], struct sockaddr_ll src_addr) {

	MPLRCtrlReceivedCount++;
	MPLROtherReceivedCount--;

	int tierAddrTotal = (ethhead[15]);

	int lengthIndex = 16;
	int z = 0;

	for (; z < tierAddrTotal; z++) {

		int lengthOfTierAddrTemp = 0;
		lengthOfTierAddrTemp = (ethhead[lengthIndex]);

		unsigned char tierAddrTemp[lengthOfTierAddrTemp];
		memset(tierAddrTemp, '\0', lengthOfTierAddrTemp + 1);

		lengthIndex = lengthIndex + 1;
		memcpy(tierAddrTemp, &ethhead[lengthIndex], lengthOfTierAddrTemp);

		lengthIndex = lengthIndex + lengthOfTierAddrTemp;

		//stablePort - 1 is stable 
		int stablePort = 0;
		stablePort = staging(recvOnEtherPort);

		if (stablePort == 0 && programStableFlag == 1) {
			printf("1Sending NULL join on: %s", recvOnEtherPort);
			sendNullJoin(recvOnEtherPort);
		}// 12/1/2021 - send Null Join if an iterface is down and came up

		if (stablePort == 1) {
			int isNewPort = 0;
			//1 : if new node is added or appended to the linked list 
			//0 : if a node is just updated 

			isNewPort = insert(tierAddrTemp, recvOnEtherPort);

			if (isNewPort == 1 && programStableFlag == 1) {
				printf("Sending NULL join on: %s", recvOnEtherPort);
				sendNullJoin(recvOnEtherPort);
			}// 12/1/2021 - send Null Join if an iterface is down and came up
		}
	}
}

void msgType2(char buffer[2048], unsigned char* ethhead, char recvOnEtherPort[5], struct sockaddr_ll src_addr, int *n) {
	printf("\n");
	MPLRDataReceivedCount++;
	MPLROtherReceivedCount--;

	// printing destination tier length
	unsigned char desLenTemp[2];
	sprintf(desLenTemp, "%u", ethhead[15]);
	uint8_t desLen = atoi(desLenTemp);
	printf("  Destination Tier Length  : %d \n", desLen);

	// printing destination tier
	char destLocal[20];
	memset(destLocal, '\0', 20);
	memcpy(destLocal, &ethhead[16], desLen);
	printf("  Destination Tier (Local) : %s \n", destLocal);

	char finalDes[20];
	memset(finalDes, '\0', 20);
	memcpy(finalDes, &destLocal, desLen);
	printf("  Destination Tier (Final) : %s \n", finalDes);

	// printing source tier length
	int tempLoc = 16 + desLen;
	unsigned char srcLenTemp[2];
	sprintf(srcLenTemp, "%u", ethhead[tempLoc]);
	uint8_t srcLen = atoi(srcLenTemp);
	printf("        Source Tier Length : %d \n", srcLen);

	// printing source tier
	tempLoc = tempLoc + 1;
	char srcLocal[20];
	memset(srcLocal, '\0', 20);
	memcpy(srcLocal, &ethhead[tempLoc], srcLen);
	printf("      Source Tier (Local) : %s \n", srcLocal);

	char finalSrc[20];
	memset(finalSrc, '\0', 20);
	memcpy(finalSrc, &srcLocal, srcLen);
	printf("      Source Tier (Final) : %s \n", finalSrc);

	tempLoc = tempLoc + srcLen;

	// TESTING FWD ALGORITHM 3 - Case: MPLR Data Packet Received

	int packetFwdStatus = -1;

	if (isTierSet() == 0) {
		printf("\nCalling packetForwardAlgorithm - Destination label = %s", finalDes);
		setTierInfo(headTL->tier);
		packetFwdStatus = packetForwardAlgorithm(tierAddress,
			finalDes);
	}
	else {
		errorCount++;
	}

	if (packetFwdStatus == 0) {

		//check whether the packet reached the destination by checking all the labels of the node
		int isDestination = CheckAllDestinationLabels(fwdTierAddr);

		if (isDestination == 0) {
			int decapsPacketSize = *n - tempLoc;

			unsigned char decapsPacket[decapsPacketSize];
			memset(decapsPacket, '\0', decapsPacketSize);
			memcpy(decapsPacket, &buffer[tempLoc],
				decapsPacketSize);

			unsigned char tempIPTemp[20];
			sprintf(tempIPTemp, "%u.%u.%u.%u", decapsPacket[16],
				decapsPacket[17], decapsPacket[18],
				decapsPacket[19]);

			struct in_addr* nwIPTemp = getNetworkIP(tempIPTemp);
			char* portNameTemp = findPortName(nwIPTemp);

			// check for null
			// if not null, call dataDecapsulation
			if (portNameTemp != NULL) {
				//printf("Decaps at port %s \n", portNameTemp);

				struct iphdr* iphUDP = (struct iphdr*)decapsPacket;
				printf("\n%s : the protocol is =%d\n", __FUNCTION__, iphUDP->protocol);

				switch (iphUDP->protocol) //Check the Protocol and do accordingly...
				{
				case 1:  //ICMP Protocol
					dataDecapsulation(portNameTemp, decapsPacket, decapsPacketSize);
					break;

				case 2:  //IGMP Protocol
					dataDecapsulation(portNameTemp, decapsPacket, decapsPacketSize);
					break;

				case 6:  //TCP Protocol
					dataDecapsulation(portNameTemp, decapsPacket, decapsPacketSize);
					break;

				case 17: //UDP Protocol
					dataDecapsulationUDP(portNameTemp, decapsPacket, decapsPacketSize);
					break;

				default: //Some Other Protocol like ARP etc.
					dataDecapsulation(portNameTemp, decapsPacket, decapsPacketSize);
					break;
				}

				MPLRDecapsulated++;
			}
			else {
				errorCount++;
			}
		}
		else {
			if (isFWDFieldsSet() == 0) {

				int MPLRPacketSize = *n - 14;
				unsigned char MPLRPacketFwd[MPLRPacketSize];
				memset(MPLRPacketFwd, '\0', MPLRPacketSize);
				memcpy(MPLRPacketFwd, &buffer[14],
					MPLRPacketSize);

				dataFwd(fwdInterface, MPLRPacketFwd,
					MPLRPacketSize);
				MPLRDataFwdCount++;
			}
		}
	}
	else {
		errorCount++;
	}
}

void msgType5(char buffer[2048], unsigned char* ethhead, char recvOnEtherPort[5], struct sockaddr_ll src_addr) {
	printf("\n Message 5 on : %s", recvOnEtherPort);

	MPLRMsgVReceivedCount++;
	MPLROtherReceivedCount--;
	timestamp();

	// Add the data entries in the payload if not already present.
	uint8_t totalEntries = ethhead[15]; //skip

	// Action to be performed
	uint8_t action = ethhead[16]; //skip

	int index = 17;
	// check my tier - if I am not tier 1, skip steps unitl 1242

	if (myTierValue == 1) {
		printf("if(myTierValue==1)");

		//This is to make sure the action 1 with entire Label list from failed interface is dodged 
		//when the interface becames active after being down
		if (actionflag == 0 && (action == 1 && totalEntries > 4)) {
			return;
		}
		printf("\n the action is %u :\n", action);

		int hasDeletions = 0;
		printf("totalEntries = %d", totalEntries);
		while (totalEntries > 0) {
			uint8_t tierLen = ethhead[index];
			uint8_t tierAddr[tierLen + 1];

			memset(tierAddr, '\0', tierLen + 1);
			index++; // pass the length byte

			memcpy(tierAddr, &ethhead[index], tierLen);

			index += tierLen;
			tierAddr[tierLen] = '\0';

			printf("\n\nTierLen :%u TierAddr: %s\n", tierLen, tierAddr);

			uint8_t ipLen = ethhead[index];
			struct in_addr ipAddr;

			index++; // pass the length byte

			memcpy(&ipAddr, &ethhead[index],
				sizeof(struct in_addr));

			//Only entries from other nodes are to be appended, else discard 
			if (strcmp(ip_add, inet_ntoa(ipAddr)) == 0) {
				totalEntries--;
				return;
			}
			index = index + sizeof(struct in_addr);

			uint8_t cidr = ethhead[index];

			index++; // pass the length of cidr

			if (action == MESSAGE_TYPE_ENDNW_ADD) {


				printf("\nipLen :%u ipAddr: %s cidr: %u", ipLen, inet_ntoa(ipAddr), cidr); //cidr %u to %d

				struct addr_tuple* a = (struct addr_tuple*)malloc(sizeof(struct addr_tuple));
				memset(a, '\0', sizeof(struct addr_tuple));
				strcpy(a->tier_addr, tierAddr);

				char tempaddr[20];
				memset(tempaddr, '\0', 20);
				strcpy(tempaddr, tierAddr);


				if (find_entry_LL(&ipAddr, tierAddr) == NULL) {
					// insert info about index from which the packet has been received from.
					a->if_index = src_addr.sll_ifindex;
					a->isNew = true;
					memcpy(&a->ip_addr, &ipAddr,
						sizeof(struct in_addr));
					a->cidr = cidr;
					add_entry_LL(a);
					//implement publish_add - -  to do Samruddhi 04/15/2022
					print_entries_LL();
					if (programStableFlag == 1)
						publishIPLabelMap(tempaddr, 2);
				}
			}
			else if (action == MESSAGE_TYPE_ENDNW_UPDATE) {
				printf("\naction == MESSAGE_TYPE_ENDNW_UPDATE");

				// Still not implemented, can be done by recording interface index maybe

			}
			else if (action == MESSAGE_TYPE_ENDNW_REMOVE) {
				printf("\naction == MESSAGE_TYPE_ENDNW_REMOVE");

				if (delete_entry_LL_IP(ipAddr)) {
					hasDeletions++;
				}

			}
			else if (action == MESSAGE_TYPE_ENDNW_REMOVE_ADDR) {

				printf("\naction == MESSAGE_TYPE_ENDNW_REMOVE_ADDR ");
				if (delete_entry_LL_Addr(tierAddr)) {
					print_entries_LL();
					hasDeletions++;
				}
			}
			totalEntries--;
		} // end of while

		// Send this frame out of interfaces other than where it came from
		if (hasDeletions && (action == MESSAGE_TYPE_ENDNW_REMOVE || action == MESSAGE_TYPE_ENDNW_REMOVE_ADDR)) {

			uint8_t* mplrPayload = allocate_ustrmem(IP_MAXPACKET);
			int mplrPayloadLen = 0;
			int z = 0;

			for (z = 14; z < index; z++, mplrPayloadLen++) {
				mplrPayload[mplrPayloadLen] = ethhead[z];
			}

			setInterfaces(); //interfaceList is being set by this function

			if (mplrPayloadLen) {
				int ifs2 = 1;
				for (; ifs2 < interfaceListSize; ifs2++) {
					// dont send update, if it is from the same interface.
					if (strcmp(recvOnEtherPort, interfaceList[ifs2]) != 0) {

						endNetworkSend(interfaceList[ifs2], mplrPayload, mplrPayloadLen);

					}
				}
			}
			free(mplrPayload);
			freeInterfaces();
			interfaceListSize = 0;
			print_entries_LL();
		}
		//Sync tier1 node IP mapping tables -- Samruddhi -- 1/26/2022
		checkEntriesToSync();
	}
	else {
		char parentTierAdd[20];
		memset(parentTierAdd, '\0', 20);

		findParntLongst(tierAddress, parentTierAdd);
		setByTierOnly(parentTierAdd, true);

		printf("\nPassing  ip mapping payload to %s via %s", parentTierAdd, fwdInterface);

		while (totalEntries > 0) {
			uint8_t tierLen = ethhead[index];
			uint8_t tierAddr[tierLen + 1];

			memset(tierAddr, '\0', tierLen + 1);
			index++; // pass the length byte

			memcpy(tierAddr, &ethhead[index], tierLen);

			index += tierLen;
			tierAddr[tierLen] = '\0';

			uint8_t ipLen = ethhead[index];
			struct in_addr ipAddr;

			index++; // pass the length byte

			memcpy(&ipAddr, &ethhead[index],
				sizeof(struct in_addr));

			index = index + sizeof(struct in_addr);

			uint8_t cidr = ethhead[index];

			char parentTierAdd[20];
			memset(parentTierAdd, '\0', 20);

			findParntLongst(tierAddress, parentTierAdd);
			setByTierOnly(parentTierAdd, true);
			printf("\nReceived ip address = %s and its cdir %d and need to send it my parent %s via %s", inet_ntoa(ipAddr), cidr, parentTierAdd, fwdInterface);
			index++; // pass the length of cidr

			totalEntries--;
		}
		uint8_t* mplrPayload = allocate_ustrmem(IP_MAXPACKET);
		int mplrPayloadLen = 0;
		int z = 0;
		for (z = 14; z < index; z++, mplrPayloadLen++) {
			mplrPayload[mplrPayloadLen] = ethhead[z];
		}

		endNetworkSend(fwdInterface,
			mplrPayload, mplrPayloadLen);
	}
}

void msgType7(char buffer[2048], unsigned char* ethhead, char recvOnEtherPort[5], struct sockaddr_ll src_addr) {
	printf("\nReceived MESSAGE_TYPE_JOIN from : %s", recvOnEtherPort);
	MPLRMsgVReceivedCount++;
	MPLROtherReceivedCount--;
	timestamp();
	//sleep(1);
	// check for the tierValue.
	// if the tierValue is < my TierValue , ignore the message
	// else
	// generate the label based on the interface it recived the message from
	// Send the label to the node
	// Should get a label accepted message in return

	int stablePort = 0;
	stablePort = staging(recvOnEtherPort);

	// NS - we have to count joins etc for the three message from neighbor count 
	// NS if(stablePort==1) check if count=3 and print 

	uint8_t tierValueRequestedNode = ethhead[15];
	printf("\nMy Tier Value = %d ", myTierValue);
	printf("\nTier Value of the Requested Node = %d ", tierValueRequestedNode);
	if (myTierValue < tierValueRequestedNode && strcmp(recvOnEtherPort, "eth0") != 0) {
		printf("\nMESSAGE_TYPE_JOIN request received from a node at lower tier ");
		printf("\nGenerating the label for the node");
		printf("\nInterface from which the request recvd = %s", recvOnEtherPort);

		struct labels* labelList;

		int numbNewLabels = generateChildLabel(recvOnEtherPort, tierValueRequestedNode, &labelList);
		printf("\n%s : Labels are generated\n", __FUNCTION__);
		//Send this labelList to recvOnEtherPort

		// Generate the payload for sending this message.

		// Form the NULL join message here
		char labelAssignmentPayLoad[200];
		int cplength = 0;

		// Clearing the payload
		memset(labelAssignmentPayLoad, '\0', 200);

		printf("\n %s : Creating the payload to send the new labels %s\n", __FUNCTION__, labelList->label);

		// Setting the number of labels being send
		uint8_t numberOfLabels = (uint8_t)numbNewLabels; // Need to modify
		memcpy(labelAssignmentPayLoad + cplength, &numberOfLabels, 1);
		printf("\n %s : labelAssignmentPayLoad(string) = %x", __FUNCTION__, *labelAssignmentPayLoad);// NS to fix 
		cplength++;

		struct labels* temp = labelList;
		while (temp != NULL) {
			printf("\n label list is not null\n");
			printf("\n current label in the message = %s\n", temp->label);

			// Setting the label length being send
			uint8_t labelLength = (uint8_t)strlen(temp->label); // Need to modify
			memcpy(labelAssignmentPayLoad + cplength, &labelLength, 1);
			cplength++;

			// Setting the labels being send
			// Need to modify
			memcpy(labelAssignmentPayLoad + cplength, temp->label, labelLength);
			cplength = cplength + labelLength;
			temp = temp->next;
			printf("\n %s : GETTING THE NEXT LABEL \n", __FUNCTION__);

		}
		printf("\n Sending MESSAGE_TYPE_LABELS_AVAILABLE  to all the interface, "
			"interfaceListSize = %d payloadSize=%d \n", interfaceListSize,
			(int)strlen(labelAssignmentPayLoad));
		// Send MESSAGE_TYPE_LABELS_AVAILABLE  (Message Type, Tier Value) to all other nodes
		ctrlLabelSend(MESSAGE_TYPE_LABELS_AVAILABLE, recvOnEtherPort, labelAssignmentPayLoad);
	}
}

void msgType8(char buffer[2048], unsigned char* ethhead, char recvOnEtherPort[5], struct sockaddr_ll src_addr) {
	printf("\n Received MESSAGE_TYPE_LABELS_AVAILABLE \n");
	MPLRMsgVReceivedCount++;
	MPLROtherReceivedCount--;
	timestamp();
	setInterfaces();

	int stablePort = 0;
	stablePort = staging(recvOnEtherPort);

	//to_parent port tag 
	int match_Port = matchPort(recvOnEtherPort);
	if (match_Port == 0) {
		tagport(recvOnEtherPort, 3);
	}

	int numberLabels = ethhead[15];
	printf("\n Number of Labels available = %d\n", numberLabels);
	int i = 0;
	struct labels* acceptedList;

	// Accepting each label here
	int messagePointer = 16;
	for (; i < numberLabels; i++) {

		int labelLength = ethhead[messagePointer];
		printf("\n Label Length = %d\n", labelLength);

		messagePointer++;

		char label[20];
		memset(label, '\0', 10);
		memcpy(label, ethhead + messagePointer, labelLength);
		printf("\n Label  =%s Label length= %d\n", label, (int)strlen(label));

		messagePointer = messagePointer + labelLength;

		if (!recvdLabel) {
			recvdLabel = true;
			// set the label here
			// For Default tier address 
			setTierInfo(label);
		}

		// pass it to tier address list
		int check = insertTierAddr(label);
		if (check == 1) {
			add_LL(label);

			printf("checking my Tier Addresses");
			printf("\nAdding the label to the list %s", label);

			//creating a list of accepted labels
			if (!acceptedList) {
				printf("\nAdding the first label to the list\n");
				acceptedList = (struct labels*)malloc(sizeof(struct labels));
				strncpy(acceptedList->label, label, strlen(label));
				acceptedList->next = NULL;
				notify_myLabels_Update(1, label);
			}
			else {
				printf("\nAdding all other labels to the list\n");
				struct labels* newLabel = (struct labels*)malloc(sizeof(struct labels));
				strncpy(newLabel->label, label, strlen(label));
				newLabel->next = acceptedList;
				acceptedList = newLabel;
				printf("insertion success");
				print_entries_LL();
				notify_myLabels_Update(1, label);

				// 12/7/2021 : Send Msg V just after ip to label mapping is updated

				// If new port we have to advertise our tierAdd<->IPAdd table.
				uint8_t* mplrPayload = allocate_ustrmem(
					IP_MAXPACKET);
				int mplrPayloadLen = 0;

				mplrPayloadLen = buildPayload(mplrPayload,
					COMPLETE_TABLE, 0);
				//NS - buildPayload defined in endNW.c 

				print_entries_LL();
				if (mplrPayloadLen) {
					printf("endNetworkSend after insert");
					endNetworkSend(recvOnEtherPort, mplrPayload,
						mplrPayloadLen); //method to send MSG TYPE V 
				}
				free(mplrPayload);
			}
		}
		checkEntriesToAdvertise();
		print_entries_LL();
	}
	struct labels* temp = acceptedList;
	if (temp != NULL) {
		// Generate Labels Accepted Message
		int cplength = 0;
		char labelAssignmentPayLoad[200];

		// Clearing the payload
		memset(labelAssignmentPayLoad, '\0', 200);

		// Set tbe labels here .. TO be done
		uint8_t numLabels = (uint8_t)numberLabels;
		memcpy(labelAssignmentPayLoad + cplength, &numLabels, 1);
		cplength++;

		// Copy all other labels to the labels accepted message
		while (!temp) {

			char label[10];
			memcpy(label, temp->label, strlen(temp->label));

			// Set tbe labellength here
			uint8_t labelLength = strlen(label);
			memcpy(labelAssignmentPayLoad + cplength, &numLabels, 1);
			cplength++;

			// Set tbe label here
			memcpy(labelAssignmentPayLoad + cplength, label, labelLength);
			cplength = cplength + labelLength;
		}

		// Sending labels accepted message

		ctrlLabelSend(MESSAGE_TYPE_LABELS_ACCEPTED, recvOnEtherPort, labelAssignmentPayLoad);
		printMyLabels();
		printf("\n Calling printMyLabels linenumber =%d", __LINE__);
	}
}

void msgType9(char buffer[2048], unsigned char* ethhead, char recvOnEtherPort[5], struct sockaddr_ll src_addr) {
	int stablePort = 0;
	stablePort = staging(recvOnEtherPort);

	printf("\n Received MESSAGE_TYPE_LABELS_ACCEPTED \n");
	MPLRMsgVReceivedCount++;
	MPLROtherReceivedCount--;
	timestamp();
}

void msgType10(char buffer[2048], unsigned char* ethhead, char recvOnEtherPort[5], struct sockaddr_ll src_addr) {
	printf("\n Received MESSAGE_TYPE_MY_LABELS_ADD on : %s \n", recvOnEtherPort);
	MPLRMsgVReceivedCount++;
	MPLROtherReceivedCount--;
	timestamp();
	int tierAddrTotal = (ethhead[15]);

	printf("  No. of Tier Addr : %d\n", tierAddrTotal);

	int lengthIndex = 16;
	int z = 0;

	for (; z < tierAddrTotal; z++) {

		int lengthOfTierAddrTemp = 0;
		lengthOfTierAddrTemp = (ethhead[lengthIndex]);
		printf("      Tier Address  Length : %d \n", lengthOfTierAddrTemp);
		printf("      Number of Tier Address : %d, value of z : %d \n", tierAddrTotal, z);
		unsigned char tierAddrTemp[lengthOfTierAddrTemp];

		memset(tierAddrTemp, '\0', lengthOfTierAddrTemp + 1);

		lengthIndex = lengthIndex + 1;
		memcpy(tierAddrTemp, &ethhead[lengthIndex], lengthOfTierAddrTemp);

		lengthIndex = lengthIndex + lengthOfTierAddrTemp;

		//Staging - samruddhi

		//stablePort 1 is stable 
		int stablePort = 0;
		stablePort = staging(recvOnEtherPort);

		if (stablePort == 1) {
			int isNewPort = 0;
			//1 : if new node is added or appended to the linked list 
			//0 : if a node is just updated 
			isNewPort = insert(tierAddrTemp, recvOnEtherPort);
		}
	}
	int match_Port = matchPort(recvOnEtherPort);
	if (match_Port == 3) {
		sendNullJoin(recvOnEtherPort);
	}
}

void msgType11(char buffer[2048], unsigned char* ethhead, char recvOnEtherPort[5], struct sockaddr_ll src_addr) {
	printf("\n Received MESSAGE_TYPE_MY_LABELS_DELETE on : %s \n", recvOnEtherPort);
	MPLRMsgVReceivedCount++;
	MPLROtherReceivedCount--;
	timestamp();

	int tierAddrTotal = (ethhead[15]);
	printf("  No. of Tier Addr : %d\n", tierAddrTotal);

	int lengthIndex = 16;
	int z = 0;

	int lengthOfTierAddrTemp = 0;
	lengthOfTierAddrTemp = (ethhead[lengthIndex]);
	printf("      Tier Address  Length : %d \n", lengthOfTierAddrTemp);
	printf("      Number of Tier Address : %d, value of z : %d \n", tierAddrTotal, z);

	unsigned char tierAddrTemp[lengthOfTierAddrTemp];
	memset(tierAddrTemp, '\0', lengthOfTierAddrTemp + 1);
	lengthIndex = lengthIndex + 1;

	memcpy(tierAddrTemp, &ethhead[lengthIndex], lengthOfTierAddrTemp);
	printf("      Tier Address : %s \n", tierAddrTemp);
	lengthIndex = lengthIndex + lengthOfTierAddrTemp;

	struct nodeHL* temp2 = headHL;
	struct nodeHL* prev2 = headHL;
	int match_Port = matchPort(recvOnEtherPort);
	while (temp2 != NULL) {
		if (strlen(temp2->tier) == strlen(tierAddrTemp)) {

			if (strncmp(temp2->tier, tierAddrTemp, strlen(tierAddrTemp)) == 0) {
				if (temp2 == headHL) {
					printf("TEST: Head node removed value was %s\n", temp2->tier);
					headHL = temp2->next;
				}
				else {
					prev2->next = temp2->next;
					printf("TEST: other node removed value was %s\n", temp2->tier);
				}
				if (match_Port == 3) {
					printf("My parent's label lost so deleting from my labels\n");
					deleteMyLabelsRelated(temp2->tier);
				}
			}
			else {
				prev2 = temp2;
			}

		}
		else {
			prev2 = temp2;
		}
		temp2 = temp2->next;
	}
	printNeighbourTable();
	printMyLabels();
	delete_failed_LL_Addr(failedLL_head);
	print_entries_LL();
}

void msgType12(char buffer[2048], unsigned char* ethhead, char recvOnEtherPort[5], struct sockaddr_ll src_addr) {
	uint8_t totalEntries = 0;
	printf("\n TEST: MNLR Message 12 received : %s \n", recvOnEtherPort);

	MPLRMsgVReceivedCount++;
	MPLROtherReceivedCount--;
	timestamp();

	int lengthIndex = 16;
	// check my tier - if I am not tier 1, skip steps unitl 1242

	int lengthOfMytier = 0;
	lengthOfMytier = (ethhead[lengthIndex]);

	unsigned char MytierAddrTemp[lengthOfMytier];
	memset(MytierAddrTemp, '\0', lengthOfMytier + 1);

	lengthIndex = lengthIndex + 1;
	memcpy(MytierAddrTemp, &ethhead[lengthIndex], lengthOfMytier);
	lengthIndex = lengthIndex + lengthOfMytier;

	int lengthOfTierAddrTemp = 0;
	lengthOfTierAddrTemp = (ethhead[lengthIndex]);

	unsigned char tierAddrTemp[lengthOfTierAddrTemp];
	memset(tierAddrTemp, '\0', lengthOfTierAddrTemp + 1);

	lengthIndex = lengthIndex + 1;
	memcpy(tierAddrTemp, &ethhead[lengthIndex], lengthOfTierAddrTemp);
	lengthIndex = lengthIndex + lengthOfTierAddrTemp;

	printf("\n Message 12 from : %s", MytierAddrTemp);
	if (myTierValue == 1) {
		printf("\nMessage at tier 1 : Getting the labels for source and destination\n");
		printf("\nResolving ip for destination label :  %s", MytierAddrTemp);
		deleteList(); //delete resolving list

		char* tempTier = updateEndTierAddr(tierAddrTemp, MytierAddrTemp);
		char destAddr[20];
		memcpy(destAddr, tempTier, 20);

		uint8_t cidr;
		struct in_addr ip;
		inet_pton(AF_INET, tierAddrTemp, &ip);

		//sending response to sender
		responseIPresolve(destAddr, ip, MytierAddrTemp, totalEntries, 1);

		deleteList();
		printAddresstoResponse();

		printf("\nForwarding source labels to destination ");

		char mylabelIP[20];
		findIPforLabel(MytierAddrTemp, mylabelIP);

		//calling updateEndTierAddr double --- needs to change!! Samruddhi
		char* tempTier1 = updateEndTierAddr(mylabelIP, tempTier);

		uint8_t cidr1;
		struct in_addr ip1;
		inet_pton(AF_INET, mylabelIP, &ip1);
		responseIPresolve(MytierAddrTemp, ip1, destAddr, totalEntries, 1);
	}
	else {
		printf("\nelse : sent destination ip to my parent to resolve and will return to %s", MytierAddrTemp);
		requestIPresolve(tierAddrTemp, MytierAddrTemp);
	}
}

void msgType13(char buffer[2048], unsigned char* ethhead, char recvOnEtherPort[5], struct sockaddr_ll src_addr, int *n) {
	printf("\n TEST: MNLR Message 13 received : %s \n", recvOnEtherPort);

	MPLRMsgVReceivedCount++;
	MPLROtherReceivedCount--;
	timestamp();

	// Add the data entries in the payload if not already present.
	uint8_t totalEntries = ethhead[15]; //skip
	int tierAddrTotal = (ethhead[15]);

	printf("  No. of Tier Addr : %d\n", tierAddrTotal);
	// Action to be performed

	int lengthIndex = 16;
	// check my tier - if I am not tier 1, skip steps unitl 1242

	//Destination reading
	int lengthOfMytier = 0;
	lengthOfMytier = (ethhead[lengthIndex]);

	unsigned char MytierAddrTemp[lengthOfMytier];
	memset(MytierAddrTemp, '\0', lengthOfMytier + 1);

	lengthIndex = lengthIndex + 1;
	memcpy(MytierAddrTemp, &ethhead[lengthIndex], lengthOfMytier);
	lengthIndex = lengthIndex + lengthOfMytier;

	printf("\n Message 13 for : %s", MytierAddrTemp);

	int chkDestLbl = 1;//1=ERROR, 0=Success

	//check whether the packet reached the destination by checking all the labels of the node
	chkDestLbl = CheckAllDestinationLabels(MytierAddrTemp);
	printf("\n Message 13 chkDestLbl : %d", chkDestLbl);
	if (chkDestLbl == 0)
	{
		printf("\n Message 13 (response) received at destination\n");
		//Adding functionalities after packet is recieved  2/18/2022

		printf("\nAdding to my ip to label map:");
		while (totalEntries > 0) {
			//ResponseLabel reading

			int lengthOfTierAddrTemp = 0;
			lengthOfTierAddrTemp = (ethhead[lengthIndex]);

			unsigned char tierAddrTemp[lengthOfTierAddrTemp];
			memset(tierAddrTemp, '\0', lengthOfTierAddrTemp + 1);

			lengthIndex = lengthIndex + 1;
			memcpy(tierAddrTemp, &ethhead[lengthIndex], lengthOfTierAddrTemp);
			lengthIndex = lengthIndex + lengthOfTierAddrTemp;

			printf("\nGot in response : %s", tierAddrTemp);

			//ResponseIP read
			uint8_t ipLen = ethhead[lengthIndex];
			struct in_addr ipAddr;

			lengthIndex++; // pass the length byte

			memcpy(&ipAddr, &ethhead[lengthIndex],
				sizeof(struct in_addr));

			lengthIndex = lengthIndex + sizeof(struct in_addr);

			uint8_t cidr = ethhead[lengthIndex];

			lengthIndex++;

			//Only entries from other nodes are to be appended, else discard 
			if (strcmp(ip_add, inet_ntoa(ipAddr)) == 0) {
				totalEntries--;
				return;
			}
			printf("\nipLen :%u ipAddr: %s cidr: %u", ipLen, inet_ntoa(ipAddr), cidr); //cidr %u to %d

			struct addr_tuple* a = (struct addr_tuple*)malloc(sizeof(struct addr_tuple));
			memset(a, '\0', sizeof(struct addr_tuple));
			strcpy(a->tier_addr, tierAddrTemp);

			if (find_entry_LL(&ipAddr, tierAddrTemp) == NULL) {
				// insert info about index from which the packet has been received from.
				a->if_index = src_addr.sll_ifindex;
				a->isNew = true;
				memcpy(&a->ip_addr, &ipAddr,
					sizeof(struct in_addr));
				a->cidr = cidr;
				add_entry_LL(a);
				print_entries_LL();
				totalEntries--;
			}
			else {
				totalEntries--;
			}
		}
	}
	else {
		printf("\n Message 13 (response) not for me! Send it to my child");
		//structure of message 13 : Destination|Label|IP
		//structure of responseIPresolve : resolved label, resolved ip, return to which label.
		uint8_t ipLen = ethhead[lengthIndex];
		struct in_addr ipAddr;

		lengthIndex++; // pass the length byte

		memcpy(&ipAddr, &ethhead[lengthIndex],
			sizeof(struct in_addr));

		lengthIndex = lengthIndex + sizeof(struct in_addr);

		uint8_t cidr = ethhead[lengthIndex];

		lengthIndex++; // pass the length of cidr

		//Copying ethhead[14] till end in a buffer
		unsigned char* headWithPayload;
		int packetSize = *n - 14; // NS- remove MAC header
		headWithPayload = (unsigned char*)malloc(packetSize);
		memset(headWithPayload, '\0', packetSize);
		memcpy(headWithPayload, &buffer[14], packetSize);


		//forwarding the load as it is with replacing the head
		setInterfaces();
		char parentofDest[20];
		memset(parentofDest, '\0', 20);
		char eth_addr1[20];

		findParentLongest(MytierAddrTemp, parentofDest);
		printf("\nParent : %s\n", parentofDest);
		struct nodeHL* fNode1 = headHL;


		if (fNode1 == NULL) {
			if (enableLogScreen) {
				printf("\nERROR: Neighbor List is empty \n");
			}
		}
		// traverse the list
		// testing
		while (fNode1 != NULL) {
			//while (fNode->next != NULL) {
			if ((strlen(fNode1->tier) == strlen(parentofDest)) && ((strncmp(fNode1->tier, parentofDest, strlen(parentofDest)) == 0))) {
				strcpy(eth_addr1, fNode1->port);
				printf("\nForwarding resolve response to : %s on %s", fNode1->tier, eth_addr1);
				endNetworkSend(eth_addr1, headWithPayload, packetSize);
				break; //return ()()
			}

			fNode1 = fNode1->next;

		}
		free(headWithPayload);
		freeInterfaces();
		interfaceListSize = 0;
	}
}

void msgType14(char buffer[2048], unsigned char* ethhead, char recvOnEtherPort[5], struct sockaddr_ll src_addr) {
	printf("\n Received MESSAGE_TYPE_LABELS_LOST on : %s \n", recvOnEtherPort);
	MPLRMsgVReceivedCount++;
	MPLROtherReceivedCount--;
	timestamp();

	//Data entries in the payload 
	uint8_t totalEntries = ethhead[15];
	int index = 16;
	int hasDeletions = 0;
	printf("totalEntries = %d", totalEntries);
	while (totalEntries > 0) {
		uint8_t tierLen = ethhead[index];
		uint8_t tierAddr[tierLen + 1];

		memset(tierAddr, '\0', tierLen + 1);
		index++;        // pass the length byte

		memcpy(tierAddr, &ethhead[index], tierLen);

		index += tierLen;
		tierAddr[tierLen] = '\0';

		printf("\n\nTierLen :%u TierAddr: %s\n", tierLen, tierAddr);

		modify_LL(tierAddr);
		hasDeletions++;

		totalEntries--;
	}

	//Samruddhi 4/6/2022 : Edit to work for sync -to do
	if (failedLL_head != NULL) {
		setInterfaces(); //interfaceList is being set by this function

		int loopCounter2 = 1;// dont send on eth0 hence changed it to 1
		uint8_t* mplrPayload = allocate_ustrmem(IP_MAXPACKET);
		int mplrPayloadLen = 0;
		int port_number = 0;
		mplrPayloadLen = buildPayloadRemoveAdvts(mplrPayload, failedLL_head, 4);
		if (mplrPayloadLen) {
			for (; loopCounter2 < interfaceListSize; loopCounter2++) {
				// MPLR TYPE 5.
				port_number = matchPort(interfaceList[loopCounter2]);
				if (port_number == 1) {
					endNetworkSend(interfaceList[loopCounter2], mplrPayload, mplrPayloadLen);
				}
			}
		}
		free(mplrPayload);
		print_entries_LL();
		freeInterfaces();
		interfaceListSize = 0;
	}
}

void msgType15(char buffer[2048], unsigned char* ethhead, char recvOnEtherPort[5], struct sockaddr_ll src_addr) {
	printf("\n Received MESSAGE_TYPE_PUBLISH_IP_DELETE on : %s \n", recvOnEtherPort);
	MPLRMsgVReceivedCount++;
	MPLROtherReceivedCount--;
	timestamp();

	int lengthIndex = 16;

	int lengthOfTierAddrTemp = 0;
	lengthOfTierAddrTemp = (ethhead[lengthIndex]);

	unsigned char tierAddrTemp[lengthOfTierAddrTemp];
	memset(tierAddrTemp, '\0', lengthOfTierAddrTemp + 1);

	lengthIndex = lengthIndex + 1;

	memcpy(tierAddrTemp, &ethhead[lengthIndex], lengthOfTierAddrTemp);
	printf("      Tier Address : %s \n", tierAddrTemp);

	lengthIndex = lengthIndex + lengthOfTierAddrTemp;

	if (myTierValue == 3) {
		if (delete_entry_LL_Addr(tierAddrTemp)) {
			print_entries_LL();
		}
	}
	else {
		printf("In else: MESSAGE_TYPE_PUBLISH_IP_DELETE\n");
		publishIPLabelMap(tierAddrTemp, 1);
	}
}

void msgType16(char buffer[2048], unsigned char* ethhead, char recvOnEtherPort[5], struct sockaddr_ll src_addr, int *n) {
	printf("\n Received MESSAGE_TYPE_PUBLISH_IP_ADD on : %s \n", recvOnEtherPort);
	MPLRMsgVReceivedCount++;
	MPLROtherReceivedCount--;
	timestamp();

	if (myTierValue == 3) {
		printf("\nMESSAGE_TYPE_PUBLISH_IP_ADD received at destination\n");
		int lengthIndex = 16;

		//Publish Label reading

		int lengthOfTierAddrTemp = 0;
		lengthOfTierAddrTemp = (ethhead[lengthIndex]);

		unsigned char tierAddrTemp[lengthOfTierAddrTemp];
		memset(tierAddrTemp, '\0', lengthOfTierAddrTemp + 1);

		lengthIndex = lengthIndex + 1;
		memcpy(tierAddrTemp, &ethhead[lengthIndex], lengthOfTierAddrTemp);

		lengthIndex = lengthIndex + lengthOfTierAddrTemp;
		printf("\nGot in response : %s", tierAddrTemp);

		//Publish IP read
		uint8_t ipLen = ethhead[lengthIndex];
		struct in_addr ipAddr;

		lengthIndex++; // pass the length byte

		memcpy(&ipAddr, &ethhead[lengthIndex],
			sizeof(struct in_addr));

		lengthIndex = lengthIndex + sizeof(struct in_addr);

		uint8_t cidr = ethhead[lengthIndex];

		lengthIndex++;

		//Only entries from other nodes are to be appended, else discard 
		if (strcmp(ip_add, inet_ntoa(ipAddr)) == 0 || searchIPinMyMap(inet_ntoa(ipAddr)) == 0) {
			return;
		}
		printf("\nipLen :%u ipAddr: %s cidr: %u", ipLen, inet_ntoa(ipAddr), cidr); //cidr %u to %d

		struct addr_tuple* a = (struct addr_tuple*)malloc(sizeof(struct addr_tuple));
		memset(a, '\0', sizeof(struct addr_tuple));
		strcpy(a->tier_addr, tierAddrTemp);

		if (find_entry_LL(&ipAddr, tierAddrTemp) == NULL) {
			// insert info about index from which the packet has been received from.
			a->if_index = src_addr.sll_ifindex;
			a->isNew = true;
			memcpy(&a->ip_addr, &ipAddr,
				sizeof(struct in_addr));
			a->cidr = cidr;
			add_entry_LL(a);
			print_entries_LL();
		}

	}
	else {
		printf("\nMESSAGE_TYPE_PUBLISH_IP_ADD sent to my children to update\n");
		//Copying ethhead[14] till end in a buffer
		unsigned char* headWithPayload;
		int packetSize = *n - 14; // NS- remove MAC header
		headWithPayload = (unsigned char*)malloc(packetSize);
		memset(headWithPayload, '\0', packetSize);
		memcpy(headWithPayload, &buffer[14], packetSize);

		setInterfaces();
		int loopCounter2 = 1;

		for (; loopCounter2 < interfaceListSize; loopCounter2++) {
			// MPLR TYPE 5.
			int port_number = matchPort(interfaceList[loopCounter2]);
			if (port_number == 2) {

				printf("\nSending MESSAGE_TYPE_PUBLISH_IP_ADD message to - %s at ", interfaceList[loopCounter2]);
				endNetworkSend(interfaceList[loopCounter2], headWithPayload, packetSize);
			}
		}
		free(headWithPayload);
		freeInterfaces();
		interfaceListSize = 0;
	}
}