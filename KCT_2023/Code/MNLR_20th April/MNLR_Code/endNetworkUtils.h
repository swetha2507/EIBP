#ifndef ENDUTL_H
#define ENDUTL_H

#include <netinet/in.h>
#include <netinet/ip.h>
#include <errno.h>
#include <stdbool.h>

#include <limits.h>

#include "sendAndFwd.h"

#define IP_ADDR_LEN             16
#define IP4_HDRLEN              20
#define UDP_HDRLEN              8
#define COMPLETE_TABLE         	1
#define ONLY_NEW_ENTRIES 	2

struct addr_tuple {
	int if_index;  // interface index (char to int) 
	bool isNew; 
	char tier_addr[MAX_TIER_ADDR_SIZE]; //Labels
	struct in_addr ip_addr; //IP Addresses associated with that label 
	uint8_t cidr;
	char etherPortName[10]; 
	struct addr_tuple *next;
}*headTuple;


struct addr_list{
	char label[20]; //IP Address
	struct in_addr ip_addr; //IP Addresses associated with that label 
	uint8_t cidr;			//label whose IP matches for responseIP
	struct addr_list *next;
}*headaddr;

char *allocate_strmem(int);
uint8_t *allocate_ustrmem(int);
int *allocate_intmem(int);

void startAdvertising(char **, int, int);
void receiveAdvertisement(char **, int);
void clearEntryState();
int buildPayload(uint8_t *, int, int);

//void add_entry_LL(struct addr_tuple*); //Tuheena 28apr2022 
struct addr_tuple *find_entry_LL(struct in_addr *, char *tierAddr, int interAS_flag);
bool delete_entry_LL_IP(struct in_addr);
bool delete_entry_LL_Addr(uint8_t *);
void delete_failed_LL_Addr(uint8_t* );
void print_entries_LL();

struct addr_tuple* add_matched_entry(struct addr_tuple*, struct addr_tuple*);
int buildPayloadRemoveAdvts(uint8_t *, struct addr_tuple*, int);
struct addr_tuple* checkTableEntries(struct addr_tuple*);

char* updateEndTierAddr(char[],char[]);
char* findPortName(struct in_addr *);
struct in_addr* getNetworkIP(char[]);
void modify_LL(char *);
struct addr_tuple* getFailedAddr();

	
//Tuheena 28apr2022 InterAs variables starts
char* updateInterASEndTierAddr(char[]);
char* getBrAddress(char[]);
int buildPayloadInterAS(uint8_t *);
int buildPayloadQuery(uint8_t *, char[], int);
int buildPayloadInterIp(uint8_t *, char[], int, int );
void print_interAS_entries_LL();
void add_entry_LL(struct addr_tuple*, int);
//int buildPayloadBR(uint8_t *, int, int); //Tuheena 18 may 2022
// int find_entry_LL(struct in_addr *, char *tierAddr, int);
//Tuheena 28apr2022 InterAs variables ends

#endif
