#include "endNetworkUtils.h"

extern FILE *fptr;
extern int enableLogScreen;
extern int enableLogFiles;

struct addr_tuple *failedLL_head=NULL;
struct addr_list *headaddr=NULL;
struct addr_tuple *tablehead=NULL;

//Tuheena 28apr2022 declarations InterAS starts
struct addr_tuple *interAS_tablehead = NULL; //to store interAS labels
extern int myTierValue;   
extern int interAS_flag;     
//Tuheena declaration ends

struct iprequestFlag{
	char ip[20];
	int flag;
	struct iprequestFlag *next;
}*head_iRF;

void clearEntryState() {
	struct addr_tuple *current;
	for (current = tablehead; current != NULL; current = current->next) {
		current->isNew = false;
	}
}

char * allocate_strmem(int len) {
	void *tmp;

	if (len <= 0) {
		fprintf(stderr,
				"ERROR: Cannot allocate memory because len = %i in allocate_strmem().\n",
				len);
		exit(EXIT_FAILURE);
	}

	tmp = (char *) malloc(len * sizeof(char));
	if (tmp != NULL) {
		memset(tmp, 0, len * sizeof(char));
		return (tmp);
	} else {
		fprintf(stderr,
				"ERROR: Cannot allocate memory for array allocate_strmem().\n");
		exit(EXIT_FAILURE);
	}
}

uint8_t * allocate_ustrmem(int len) {
	void *tmp;

	if (len <= 0) {
		fprintf(stderr,
				"ERROR: Cannot allocate memory because len = %i in allocate_ustrmem().\n",
				len);
		exit(EXIT_FAILURE);
	}

	tmp = (uint8_t *) malloc(len * sizeof(uint8_t));
	if (tmp != NULL) {
		memset(tmp, '\0', len * sizeof(uint8_t));
		return (tmp);
	} else {
		fprintf(stderr,
				"ERROR: Cannot allocate memory for array allocate_ustrmem().\n");
		exit(EXIT_FAILURE);
	}
}

int *
allocate_intmem(int len) {
	void *tmp;

	if (len <= 0) {
		fprintf(stderr,
				"ERROR: Cannot allocate memory because len = %i in allocate_intmem().\n",
				len);
		exit(EXIT_FAILURE);
	}

	tmp = (int *) malloc(len * sizeof(int));
	if (tmp != NULL) {
		memset(tmp, 0, len * sizeof(int));
		return (tmp);
	} else {
		fprintf(stderr,
				"ERROR: Cannot allocate memory for array allocate_intmem().\n");
		exit(EXIT_FAILURE);
	}
}

void add_entry_LL(struct addr_tuple *node, int interAS_neighbor) {
	printf("\nEntering add_entry_LL, %s \n", node->tier_addr );

	//Tuheena 28apr2022 InterAS starts
        if(interAS_neighbor==1){
        if (interAS_tablehead == NULL) {
                interAS_tablehead = node;
                interAS_tablehead->next = NULL;
				printf("\n interAStablehead is NULL interAS_neighbor %d ", interAS_neighbor);
        	} else {
				printf("\n interAStablehead is not NULL interAS_neighbor%d\n ", interAS_neighbor);
                struct addr_tuple *current = interAS_tablehead;
                while (current->next != NULL) {
                        current = current->next;
                } 
                current->next = node;
                node->next = NULL;
        	}
		
        }
	else{
	if (tablehead == NULL) {
		printf("\n tablehead is NULL inter AS neighbor = %d ", interAS_neighbor);
		tablehead = node;
		tablehead->next = NULL;
	} else {
		struct addr_tuple *current = tablehead;
		while (current->next != NULL) {
			current = current->next;
		}
		printf("\n tablehead is added inter AS neighbor =%d ", interAS_neighbor);
		current->next = node;
		node->next = NULL;
	}
	printf("\n Calling Check entries to advertise in add_entry_ll\n");
	}
}


struct addr_tuple* add_matched_entry(struct addr_tuple *node,
		struct addr_tuple *matched) {
	struct addr_tuple *ptr = matched;
	struct addr_tuple *temp = NULL;

	temp = (struct addr_tuple*) calloc(1, sizeof(struct addr_tuple));
	memcpy(temp, node, sizeof(struct addr_tuple));
	temp->next = NULL;

	if (ptr == NULL) {
		matched = temp;
	} else {
		while (ptr->next != NULL) {
			ptr = ptr->next;
		}
		ptr->next = temp;
	}
	return matched;
}

// delete entries in the table.
bool delete_entry_LL_IP(struct in_addr ip) {
        bool hasDeletions = false;

        if (tablehead == NULL) {
                return false;
        } else {
                struct addr_tuple *current = tablehead;
                struct addr_tuple *prev = NULL;
                while (current != NULL) {
					if (ip.s_addr == current->ip_addr.s_addr) {
						hasDeletions = true;
						if (tablehead == current) {
							tablehead = tablehead->next;
							free(current);
							current = tablehead;
							continue;
						} else {
							prev->next = current->next;
							free(current);
							current = prev;
						}
					}
					prev = current;
					current= current->next;
                }
        }
        print_entries_LL();
        return hasDeletions;
}

bool delete_entry_LL_Addr(uint8_t* tier_addr){
	bool hasDeletions = false;

        if (tablehead == NULL) {

                return false;
        } else {

                struct addr_tuple *current = tablehead;
                struct addr_tuple *prev = NULL;
                while (current != NULL) {
					
					if (strcmp(tier_addr,current->tier_addr)==0) {
					
						hasDeletions = true;
						//publish IPLabel map delete to my children - to do - samruddhi 4/11/2022
						printf("\nRemoving %s \n", current->tier_addr);
						publishIPLabelMap(current->tier_addr,1);
						if (tablehead == current) {
							tablehead = tablehead->next;
							free(current);
							current = tablehead;
							continue;
						} else {
							prev->next = current->next;
							free(current);
							current = prev;
						}
					}
					
					prev = current;
					current= current->next;
                }
        }

        return hasDeletions;
}

void delete_failed_LL_Addr(uint8_t* tier_addr){

        if (failedLL_head == NULL) {
				printf("failedLL_head is NULL");
                
        } else {
                struct addr_tuple *current = failedLL_head;
                struct addr_tuple *prev = failedLL_head;
                while (current != NULL) {
					
                        if (strcmp(tier_addr,current->tier_addr)==0) {
							
                                if (failedLL_head == current) {
                                        failedLL_head = failedLL_head->next;
                                        free(current);
                                        current = failedLL_head;
                                        continue;
                                } else {
                                        prev->next = current->next;
                                        free(current);
                                        current = prev;
                                }
                        }
                        prev = current;
                        current= current->next;
                }
        }
}
// match the longest prefix

void add_failed_entry_LL(struct addr_tuple *node) { 
	

	if (failedLL_head == NULL) {
		failedLL_head = node;
		failedLL_head->next = NULL;
	} else {
		struct addr_tuple *current = failedLL_head;
		while (current->next != NULL) {
			current = current->next;
		}
		current->next = node;
		node->next = NULL;
	}
}




/*
struct addr_tuple {
	int if_index;
	bool isNew;
	char tier_addr[MAX_TIER_ADDR_SIZE];
	struct in_addr ip_addr;
	uint8_t cidr;
	char etherPortName[10];
	struct addr_tuple *next;
};
this function takes ip addresses and tier addresses corresponding to an edge node
It tries to locate  an IP address mapped to Tieraddres in
struct tablehead 
//Tuheena 28apr2022 The inter_AS_flag is used to find interAS labels
*/
struct addr_tuple *find_entry_LL(struct in_addr *ip, char *tierAddr, int interAS_neighbor) {

        //Tuheena 28apr2022 InterAS starts
        if (interAS_neighbor ==1){ // if this is true
                if (interAS_tablehead == NULL) {
                return 0;
       		 } else {
                struct addr_tuple *current = interAS_tablehead;
                while (current != NULL) {
                        struct in_addr temp;
                        memcpy(&temp, ip, sizeof(struct in_addr));
                        temp.s_addr = ntohl(temp.s_addr);
                        temp.s_addr = ((temp.s_addr >> (32 - current->cidr))
                                        << (32 - current->cidr));
                        temp.s_addr = htonl(temp.s_addr);
                        if ((current->ip_addr.s_addr == temp.s_addr)) {
                                return 1;
                        }
						 current = current->next;
                }
                return 0;
        	}
        }
    else{
//Tuheena InterAS  ends

			if (tablehead == NULL) {
				printf("\n tablehead == NULL");
				return NULL;
				} else {
				printf("\n populating tablehead");
				struct addr_tuple *current = tablehead;
				while (current != NULL) {
					struct in_addr temp;
					memcpy(&temp, ip, sizeof(struct in_addr));

					temp.s_addr = ntohl(temp.s_addr);
					temp.s_addr = ((temp.s_addr >> (32 - current->cidr))
							<< (32 - current->cidr));
					temp.s_addr = htonl(temp.s_addr);

					if ((current->ip_addr.s_addr == temp.s_addr)
							&& (strncmp(tierAddr, current->tier_addr, strlen(tierAddr))
									== 0)) {
						return current;
					}
					current = current->next;
				}
				return NULL;
				}
		}
}

/*
print_entries_LL

Prints the ip to label map

*/

//Tuheena 28apr2022 InterAS 456-498 starts
//build payloads for interAS message exchanges
int buildPayloadInterAS(uint8_t *data){
        int payloadLen = 3;
        int entries = 0;
        struct addr_tuple *current;
        for (current = interAS_tablehead; current != NULL; current = current->next) {
                        if (current->isNew == true) {
                                
                                uint8_t tierLen = (uint8_t) strlen(current->tier_addr);
                                data[payloadLen] = tierLen;
                                payloadLen++;
                                memcpy(&data[payloadLen], current->tier_addr, tierLen);
                                payloadLen = payloadLen + tierLen;
                                uint8_t ipLen = (uint8_t) sizeof(struct in_addr);
                                data[payloadLen] = ipLen;
                                payloadLen++;
                                memcpy(&data[payloadLen], &current->ip_addr, ipLen);
                                payloadLen = payloadLen + ipLen;
                                data[payloadLen] = current->cidr;
                                payloadLen++;
                                entries++;
                        }
                }
                if (entries > 0) {
                data[0] = (uint8_t) MESSAGE_TYPE_ENDNW;
                data[1] = entries;
                data[2] = MESSAGE_TYPE_ENDNW_INTER_IP;
                
        } else {
                payloadLen = 0;
        }
        return payloadLen;
}
//Tuheena ends
//Tuheena 28apr2022 starts
//build payloads for interAS message exchanges
int buildPayloadQuery(uint8_t *data, char destIP[20], int msg){
        int payloadLen = 2;
        int entries = 0;
        uint8_t tierLen = (uint8_t) strlen(destIP);
        data[payloadLen] = tierLen;
        payloadLen++;
        
        memcpy(&data[payloadLen], destIP, tierLen);
        payloadLen = payloadLen + tierLen;
        data[0] = (uint8_t) MESSAGE_TYPE_QUERY;
        data[1] = msg;
        return payloadLen;
}
//build payloads for interAS message exchanges - this n=one sends a query
//Tuheena ends
//Tuheena 28apr2022 starts
int buildPayloadInterIp(uint8_t *data, char payload[200],int payloadLength, int msg){
        int payloadLen = 2;
        int entries = 0;

        memcpy(&data[payloadLen], payload, payloadLength);
        payloadLen = payloadLen + payloadLength;
        data[0] = (uint8_t) MESSAGE_TYPE_QUERY;
        data[1] = msg;
        return payloadLen;
}
//Tuheena ends
//Tuheena 28apr2022 starts
//get the BR address
char* getBrAddress(char destinationInterfaceIPAddr[]) {
        
        struct in_addr ip;
        if (inet_pton(AF_INET, destinationInterfaceIPAddr, &ip) == -1) { //inet_pton - convert IPv4 and IPv6 addresses from text to binary form
                if(enableLogScreen)
                        printf("Error: inet_pton() returned error");
                if(enableLogFiles)
                        fprintf(fptr,"Error: inet_pton() returned error");
        }
        struct addr_tuple *current = interAS_tablehead;
        while (current != NULL) {
                struct in_addr temp;
                memcpy(&temp, &ip, sizeof(struct in_addr));
                temp.s_addr = ntohl(temp.s_addr);
                temp.s_addr = (temp.s_addr >> (32 - current->cidr))
                                << (32 - current->cidr);
                temp.s_addr = htonl(temp.s_addr); //The htonl() function converts the unsigned integer hostlong from host byte order to network byte order.
                if (temp.s_addr == current->ip_addr.s_addr) {
                        return current->tier_addr;
                }
                current = current->next;
        }
        return NULL;
}
//Tuheena ends

void print_entries_LL() {
	struct addr_tuple *current= tablehead;

	if(current != NULL){
		if(enableLogScreen)
			printf("\nTier Address\t\tIP Address\n");
		if(enableLogFiles)
			fprintf(fptr,"Tier Address\t\tIP Address\n");
		for (current = tablehead; current != NULL; current = current->next) {
			if(enableLogScreen)
				printf("%s\t\t\t%s/%u\n", current->tier_addr, inet_ntoa(current->ip_addr), current->cidr);
			if(enableLogFiles)
				fprintf(fptr,"%s\t\t\t%s/%u\n", current->tier_addr, inet_ntoa(current->ip_addr), current->cidr);
	
		}
	}
}

	
//Tuheena 28apr2022 starts

void print_interAS_entries_LL() {
        struct addr_tuple *current;
        if(enableLogScreen){
                printf("\ninterAS ip Address");
                printf("\nBR-Tier Address\t\tIP Address\n");
        }
        if(enableLogFiles){
                fprintf(fptr,"\ninterAS ip Address");
                fprintf(fptr,"\nBR-Tier Address\t\tIP Address\n");
        }
        for (current = interAS_tablehead; current != NULL; current = current->next) {
                if(enableLogScreen)
                        printf("%s\t\t\t%s/%u\n",current->tier_addr, inet_ntoa(current->ip_addr), current->cidr);
                if(enableLogFiles)
                        fprintf(fptr,"%s\t\t\t%s/%u\n",current->tier_addr, inet_ntoa(current->ip_addr), current->cidr);
        }
}
//Tuheena ends

/*
	findIPforLabel

	This function takes the label as the input and finds the IP address tagged to the given label 

*/


void findIPforLabel(char label[],char ip[]){
	struct addr_tuple *current;
	for (current = tablehead; current != NULL; current = current->next) {
		if (strcmp(current->tier_addr,label)==0) {
			strcpy(ip,inet_ntoa(current->ip_addr));
			printf("\nipAddr: %s", inet_ntoa(current->ip_addr));
		}
	}
}

/*
	searchIPinMyMap

	This function takes the ip as the input.
	Returns 0 if the IP is not in my IP to label map
	Returns 1 if the IP is found in the IP to label map 
*/
int searchIPinMyMap(char ip[]){
	struct addr_tuple *current;
	char temp_ip[20],temp_ip2[20];
	strcpy(temp_ip,ip);
	for (current = tablehead; current != NULL; current = current->next) {
		strcpy(temp_ip2,inet_ntoa(current->ip_addr));

		if (strcmp(temp_ip,temp_ip2)==0) {
			return 1;
		}
	}
	return 0;
}



int buildIPPublishPacket(uint8_t *data, char addr[]){
	// int entries = 1; // 1 is kept by default since we will send only one address to resolve at a time.
	printf("\nIn buildIPPublishPacket \n");
		int entries=0;
		int payloadLen = 2;

		//struct addr_list *current;
		for (struct addr_tuple *current = tablehead; current != NULL; current = current->next) {

			if(strcmp(addr,current->tier_addr)==0){
				printf("Got a new label \n");
				
				//Adding publish label 
				uint8_t tierLen = (uint8_t) strlen(current->tier_addr);

				data[payloadLen] = tierLen; // tier len 
				payloadLen++;
				memcpy(&data[payloadLen], current->tier_addr, tierLen); 
				printf("\n in buildIPPublishPacket tier_addr : %s",current->tier_addr);//tier-addr
				payloadLen = payloadLen + tierLen; 

				//Adding publish IP
				uint8_t ipLen = (uint8_t) sizeof(struct in_addr);
				data[payloadLen] = ipLen;
				payloadLen++;
				memcpy(&data[payloadLen], &current->ip_addr, ipLen);
				payloadLen = payloadLen + ipLen;
				printf("\nipAddr: %s", inet_ntoa(current->ip_addr));

				//Adding cidr
				data[payloadLen] = current->cidr;
				payloadLen++;
				entries++;
			}
		}
		if (entries > 0) {
			data[0] = (uint8_t) MESSAGE_TYPE_PUBLISH_IP_ADD;
			data[1] = entries;
		} else {
			payloadLen = 0;
		}
	return payloadLen;

}


//for response - 2/4/2022
int buildIPResolvePacket(uint8_t *data, char destTier[],int tierValue){
	// int entries = 1; // 1 is kept by default since we will send only one address to resolve at a time.
	printf("\nIn buildIPResolvePacket \n");
	if(tierValue==1){
		int entries=0;
		int payloadLen = 2;
		
		//Adding Destination 
		uint8_t destTierLen = (uint8_t) strlen(destTier);
		
		data[payloadLen] = destTierLen; // tier len 
		payloadLen++;
		
		memcpy(&data[payloadLen], destTier, destTierLen); 
		printf("Dest_tier_addr added: %s\n",destTier);//tier-addr
		payloadLen = payloadLen + destTierLen; 

		//struct addr_list *current;
		for (struct addr_list *current = headaddr; current != NULL; current = current->next) {
			printf("Got my label in addr tuple \n");
		
			//Adding response label 
			uint8_t tierLen = (uint8_t) strlen(current->label);

			data[payloadLen] = tierLen; // tier len 
			payloadLen++;
			memcpy(&data[payloadLen], current->label, tierLen); 
			printf("\n in buildIPResolvePacket tier_addr : %s",current->label);//tier-addr
			payloadLen = payloadLen + tierLen; 

			//Adding response IP
			uint8_t ipLen = (uint8_t) sizeof(struct in_addr);
			data[payloadLen] = ipLen;
			payloadLen++;
			memcpy(&data[payloadLen], &current->ip_addr, ipLen);
			payloadLen = payloadLen + ipLen;
			printf("\nipAddr: %s", inet_ntoa(current->ip_addr));

			//Adding cidr
			data[payloadLen] = current->cidr;
			payloadLen++;
			entries++;
		}
		if (entries > 0) {
			data[0] = (uint8_t) MESSAGE_TYPE_RESPONSE_IP_RESOLVE;
			data[1] = entries;
		} else {
			payloadLen = 0;
		}
	return payloadLen;
		
	}

}


//Tuheena 18may 2022 for ResponseIPresolve starts

int buildpayloadBR(uint8_t *data, char destTier[], int tierValue ) {
	// replace variables with char destTier[],int tierValue
	// int buildpayloadBR(uint8_t *data, int msgLen, int checkIndex) - stroing a copy delete later 
	// shoudl be called only by tier 1
	printf("\nIn buildIPResolvePacket for BR \n");
	// we have to fix the check index - give the interface number on which 
	struct addr_tuple *current;
	//if(tierValue==1){
	int payloadLen = 2;
	int entries = 0;
	//Adding Destination 
	uint8_t destTierLen = (uint8_t) strlen(destTier);
	data[payloadLen] = destTierLen; // tier len 
		payloadLen++;

	memcpy(&data[payloadLen], destTier, destTierLen); 
	printf("Dest_tier_addr added: %s\n",destTier);//tier-addr
	payloadLen = payloadLen + destTierLen; 

		
		for (current = tablehead; current != NULL; current = current->next) {

				uint8_t tierLen = (uint8_t) strlen(current->tier_addr);

				data[payloadLen] = tierLen; // tier len 
				payloadLen++;

				memcpy(&data[payloadLen], current->tier_addr, tierLen); 
				printf("\n in buildpayloadBR tier_addr : %s",current->tier_addr);//tier-addr
				payloadLen = payloadLen + tierLen; 

				uint8_t ipLen = (uint8_t) sizeof(struct in_addr);
				data[payloadLen] = ipLen;
				payloadLen++;

				memcpy(&data[payloadLen], &current->ip_addr, ipLen); 
				payloadLen = payloadLen + ipLen;

				data[payloadLen] = current->cidr; //
				payloadLen++;
				entries++; //entries
		}


	if (entries > 0) {

		data[0] = (uint8_t) MESSAGE_TYPE_RESPONSE_IP_RESOLVE_BR;
		data[1] = entries;
	} else {
		payloadLen = 0;
	}
	return payloadLen;
	
}

//Tuheena 18may 2022 ends responseIpresolve

int buildPayload(uint8_t *data, int msgLen, int checkIndex) {
	struct addr_tuple *current;

	int payloadLen = 3;
	int entries = 0;
	//print_entries_LL();
	if (msgLen == 2) {
		
		for (current = tablehead; current != NULL; current = current->next) {

			if (current->isNew == true && checkIndex != current->if_index) {
				uint8_t tierLen = (uint8_t) strlen(current->tier_addr);
				
				data[payloadLen] = tierLen; // tier len 
				payloadLen++;
				memcpy(&data[payloadLen], current->tier_addr, tierLen); 
				printf("\n In buildPayload tier_addr : %s",current->tier_addr);//tier-addr
				payloadLen = payloadLen + tierLen; 
				uint8_t ipLen = (uint8_t) sizeof(struct in_addr);
				data[payloadLen] = ipLen;
				payloadLen++;
				
				memcpy(&data[payloadLen], &current->ip_addr, ipLen); 
				payloadLen = payloadLen + ipLen;
				
				data[payloadLen] = current->cidr; //
				payloadLen++;
				entries++; //entries
			}
		}
	} else {  // compplete_table = 1

		for (current = tablehead; current != NULL; current = current->next) {
			uint8_t tierLen = (uint8_t) strlen(current->tier_addr);
			data[payloadLen] = tierLen;
			payloadLen++;
			memcpy(&data[payloadLen], current->tier_addr, tierLen);
			payloadLen = payloadLen + tierLen;

			uint8_t ipLen = (uint8_t) sizeof(struct in_addr);
			data[payloadLen] = ipLen;
			payloadLen++;

			memcpy(&data[payloadLen], &current->ip_addr, ipLen);
			payloadLen = payloadLen + ipLen;

			data[payloadLen] = current->cidr;
			payloadLen++;
			entries++;

		}
	}

	if (entries > 0) {

		data[0] = (uint8_t) MESSAGE_TYPE_ENDNW;
		data[1] = entries;
		data[2] = MESSAGE_TYPE_ENDNW_ADD;
	} else {
		payloadLen = 0;
	}
	return payloadLen;
}


int buildPayloadRemoveAdvts(uint8_t *data, struct addr_tuple *failedIPS, int msg) {
        struct addr_tuple *current;
                                
        // To reserve byte for keeping track of the entries and message_type
        int payloadLen = 3;     
        int entries = 0;                
                                   
        for (current=failedIPS; (current != NULL) && (current->isNew == true) ; current = current->next) {

                uint8_t tierLen = (uint8_t)strlen(current->tier_addr);
                data[payloadLen] = tierLen;
                payloadLen++;           
                                                
                memcpy(&data[payloadLen], current->tier_addr, tierLen);
                payloadLen = payloadLen + tierLen;      
                                                
                uint8_t ipLen = (uint8_t)sizeof(struct in_addr);
                data[payloadLen] = ipLen;
                payloadLen++;           
                                        
                memcpy(&data[payloadLen], &current->ip_addr, ipLen );
                payloadLen = payloadLen + ipLen;
                        
                data[payloadLen] = current->cidr;

                current->isNew = false;
                payloadLen++;
                entries++;
				
			delete_failed_LL_Addr(current->tier_addr);
        }
        // fill number of entries added to payload, do this only if entries exist.
        if (entries > 0) {
                // Message type 5, used for advertising TierAdd<->Ipaddress entries.
                data[0] = (uint8_t) MESSAGE_TYPE_ENDNW;
                data[1] = entries;
                data[2] = msg;
		} else  {
                payloadLen = 0;
        }
        return payloadLen;
}
/* function to update the last 4 bytes*/


char* updateEndTierAddr(char destinationInterfaceIPAddr[],char Mytier[]) {
	
	printf("Destination ip address is : %s\n", destinationInterfaceIPAddr);
	printf("\n Requesting node tier address %s\n", Mytier);
	int found =0;
	struct in_addr ip;
	if (inet_pton(AF_INET, destinationInterfaceIPAddr, &ip) == -1) { //inet_pton - convert IPv4 and IPv6 addresses from text to binary form
		if(enableLogScreen)
			printf("Error: inet_pton() returned error");
		if(enableLogFiles)
			fprintf(fptr,"Error: inet_pton() returned error");
	}

	struct addr_tuple *current = tablehead; // my ip to label in T1

	while (current != NULL) {
		struct in_addr temp;
		memcpy(&temp, &ip, sizeof(struct in_addr));
		temp.s_addr = ntohl(temp.s_addr);//s_addr = /* address in network byte order */

		temp.s_addr = (temp.s_addr >> (32 - current->cidr))
				<< (32 - current->cidr);

		temp.s_addr = htonl(temp.s_addr); //The htonl() function converts the unsigned integer hostlong from host byte order to network byte order.
	
		if (temp.s_addr == current->ip_addr.s_addr) { // at tier 3 nodes they will not have the ip to label map for the destination address

			printf("Found tier :  %s \n", current->tier_addr);
			found = 1;
			//Called this function in msg12 to avoid calling it when I have the label --02/11/2022
			//responseIPresolve(current->tier_addr,current->ip_addr);
			//Creating a list of addresses
				struct addr_list*temp2;
				temp2 = (struct addr_list *) malloc(sizeof(struct addr_list));
				strcpy(temp2->label, current->tier_addr);
				memcpy(&temp2->ip_addr,&current->ip_addr, sizeof(struct in_addr));
				temp2->cidr= current->cidr;
			
				if(enableLogScreen)
				//linked list of portTag
				if (headaddr == NULL) {
					headaddr = temp2;
					headaddr->next = NULL;
				} else {
					temp2->next = headaddr;
					headaddr = temp2;
				}
		}

			current = current->next;
	}
	 	if (found ==0 && myTierValue == 1){ 
		// Tuheena 10June2022 For inter-AS pings whenever the Tier-1 node cannot find the labels in struct addr_tuple *current = tablehead; i.e. found==0 it will fall here.
		
			struct addr_tuple *current = interAS_tablehead; // my interas tables
				while (current != NULL) {
					printf("\n AM I here ??\n");
					struct in_addr temp;
					memcpy(&temp, &ip, sizeof(struct in_addr));
					temp.s_addr = ntohl(temp.s_addr);//s_addr  

					temp.s_addr = (temp.s_addr >> (32 - current->cidr))
							<< (32 - current->cidr);

					temp.s_addr = htonl(temp.s_addr); //The htonl() function converts the unsigned integer hostlong from host byte order to network byte order.
				
					if (temp.s_addr == current->ip_addr.s_addr) {

						printf("Found tier :  %s \n", current->tier_addr);
						found = 1;
						//Creating a list of addresses
							struct addr_list*temp2;
							temp2 = (struct addr_list *) malloc(sizeof(struct addr_list));
							strcpy(temp2->label, current->tier_addr);
							memcpy(&temp2->ip_addr,&current->ip_addr, sizeof(struct in_addr));
							temp2->cidr= current->cidr;
						
							if(enableLogScreen)
							//linked list of portTag
							if (headaddr == NULL) {
								headaddr = temp2;
								headaddr->next = NULL;
							} else {
								temp2->next = headaddr;
								headaddr = temp2;
							}
						
					}
					current = current->next;
				} // while ends here
		} 
		
	

		printf("\n out of the while loop in updateEndTierAddr, my tier value = %d\n", myTierValue ); 

		if(found==1){
			printAddresstoResponse();
			printf("\n");
			return headaddr->label;
		}

		if(found==0){  // used in Tier 3 nodes 

			printf("\n in Found equal 0 in updateEndTierAddr, my tier value = %d\n", myTierValue ); 

			printf("\nTier Not Found \n"); // not being resolved currently - NS may 12 2022
			
			//----------------------request flag find--------------------- happening at Tier 3 node not with ping but with the traffic generator 
			int requestsent = 0;
			struct iprequestFlag *current;
			for (current = head_iRF; current != NULL; current = current->next) {
				if (strcmp(current->ip,destinationInterfaceIPAddr)==0) {
					requestsent = 1;
					printf("Request will not be sent as its already in-progress");
				}
			}
		

			if(requestsent==0){ // request not yet responponded to - NS Check with Samruddhi 
				//add flag before sending request : 3/25/2022

				//----------------------request flag add start---------------------
				struct iprequestFlag*temp2;
				temp2 = (struct iprequestFlag *) malloc(sizeof(struct iprequestFlag));
				strcpy(temp2->ip, destinationInterfaceIPAddr);
				temp2->flag= 1;
					
				if(enableLogScreen)
					printf("\nip : %s\n",temp2->ip);
					//linked list of portTag
				if (head_iRF == NULL) {
					head_iRF = temp2;
					head_iRF->next = NULL;
				} else {
					temp2->next = head_iRF;
					head_iRF = temp2;
				}
				//----------------------request flag add end---------------------

				
				requestIPresolve(destinationInterfaceIPAddr,Mytier);
			}
			
		}
		return NULL;
}



void getListofResponseAddr(char destinationInterfaceIPAddr[]) {
	
	printf("\nDestination interface is : %s", destinationInterfaceIPAddr);
	int found =0;
	struct in_addr ip;
	if (inet_pton(AF_INET, destinationInterfaceIPAddr, &ip) == -1) { //inet_pton - convert IPv4 and IPv6 addresses from text to binary form
		if(enableLogScreen)
			printf("Error: inet_pton() returned error");
		if(enableLogFiles)
			fprintf(fptr,"Error: inet_pton() returned error");
	}

	struct addr_tuple *current = tablehead;
	while (current != NULL) {
		struct in_addr temp;
		memcpy(&temp, &ip, sizeof(struct in_addr));
		temp.s_addr = ntohl(temp.s_addr);//s_addr = /* address in network byte order */

		temp.s_addr = (temp.s_addr >> (32 - current->cidr))
				<< (32 - current->cidr);

		temp.s_addr = htonl(temp.s_addr); //The htonl() function converts the unsigned integer hostlong from host byte order to network byte order.
		if (temp.s_addr == current->ip_addr.s_addr) {

			printf("\nupdateEndTierAddr Found tier :  %s \n", current->tier_addr);
			found = 1;
			//Called this function in msg12 to avoid calling it when I have the label --02/11/2022
			//responseIPresolve(current->tier_addr,current->ip_addr);
			//Creating a list of addresses
				struct addr_list*temp2;
				temp2 = (struct addr_list *) malloc(sizeof(struct addr_list));
				strcpy(temp2->label, current->tier_addr);
				struct addr_list *t;
				if(enableLogScreen)
					printf("\nLabel : %s",temp2->label);

				//linked list of portTag
				if (headaddr == NULL) {
					headaddr = temp2;
					headaddr->next = NULL;
				} else {

					temp2->next = headaddr;
					headaddr = temp2;
		
				}
			
		}
		

		current = current->next;
	}
	if(found==1){
		printf("\nPrinting... \n");
		printAddresstoResponse();
		printf("\n");
	}

	if(found==0){
		printf("\nTier List Found \n");
	}
}

struct in_addr* getNetworkIP(char destinationInterfaceIPAddr[]) {

	struct in_addr ip;
	if (inet_pton(AF_INET, destinationInterfaceIPAddr, &ip) == -1) {
		if(enableLogScreen)
			printf("Error: inet_pton() returned error");
		if(enableLogFiles)
			fprintf(fptr,"Error: inet_pton() returned error");
	}

	struct addr_tuple *current = tablehead;
	while (current != NULL) {
		struct in_addr temp;
		memcpy(&temp, &ip, sizeof(struct in_addr));

		temp.s_addr = ntohl(temp.s_addr);

		temp.s_addr = (temp.s_addr >> (32 - current->cidr))
				<< (32 - current->cidr);

		temp.s_addr = htonl(temp.s_addr);
		if (temp.s_addr == current->ip_addr.s_addr) {

			return &current->ip_addr;
		}

		current = current->next;
	}

	return NULL;
}

char* findPortName(struct in_addr *nwIP) {

	struct addr_tuple *current = tablehead;
	while (nwIP != NULL && current != NULL) {
		if (current->ip_addr.s_addr == nwIP->s_addr) {
			
			return current->etherPortName;
		}

		current = current->next;
	}
	return NULL;
}
//Deletes the label from the Label List(LL) and adds the label to the failedLL_head
void modify_LL(char *addr){
	if (tablehead == NULL) {
                //return false;
        } else {
                struct addr_tuple *current = tablehead;
                //struct addr_tuple *prev = NULL;
                while (current != NULL) {
                        if(strcmp(current->tier_addr, addr)==0) {
								
							struct addr_tuple *a = (struct addr_tuple*) calloc (1, sizeof(struct addr_tuple));
							strcpy(a->tier_addr, current->tier_addr);
							// insert info about index from which the packet has been received from.
							a->if_index = -1;
							a->isNew = true;
							memcpy(&a->ip_addr, &current->ip_addr, sizeof(struct in_addr));
							a->cidr = current->cidr;
							strcpy(a->etherPortName, current->etherPortName);

							add_failed_entry_LL(a);
							delete_entry_LL_Addr(a->tier_addr);
                                
                        }
						
                        current= current->next;
                }
        }
}
