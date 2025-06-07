/*
 * ============================================
 * file: voting.h
 * @Author Myron Tsatsarakis (myrontsa@csd.uoc.gr)
 * @Version 26/11/2024
 *
 * @e-mail hy240@csd.uoc.gr
 *
 * @brief Structure and function declarations
 *        for CS240 Project Phase 2,
 *        Winter Semester 2024-2025
 * ============================================
 */

#ifndef VOTING_H
#define VOTING_H

#include <stdbool.h>

typedef struct District District;
typedef struct Station Station;
typedef struct Voter Voter;
typedef struct Party Party;
typedef struct Candidate Candidate;
typedef struct ElectedCandidate ElectedCandidate;


void EventAnnounceElections(int maxStationsCount, int maxSid);
void EventCreateDistrict(int did, int seats);
void EventCreateStation(int sid, int did);
void EventRegisterVoter(int vid, int sid);
void EventRegisterCandidate(int cid, int pid, int did);
void EventVote(int vid, int sid, int cid, int pid);
void EventCountVotes(int did);
void EventFormParliament(void);
void EventPrintDistrict(int did);
void EventPrintStation(int sid);
void EventPrintParty(int pid);
void EventPrintParliament(void);
void EventBonusUnregisterVoter(int vid, int sid);
void EventBonusFreeMemory(void);

/*----------------my functions used into other functions ------------*/
int hash(int sid);
void initialise_hash_vars(void);
int returnPosition();
Voter *voterInsert(Voter *vtrs, int vid, Station *ptr);
void InOrderVotersPrint(Voter *vtrs, int num_of_nodes);
void InOrderVotersCount(Voter *vtrs);
void InOrderCandidatesPrint(Candidate *cptr, int num_of_nodes);
void InOrderCandidatesCount(Candidate *cptr);
bool preOrderVotersTraversal_EventV(Voter *vptr, Station *sptr, int vid, int cid, int pid);

/* event M */
void InOrderCandidatesCount_M(Candidate *cptr, int did);
void ElectPartyCandidatesInDistrict(int pid, int did, int elected_num_from_pid);
void PreOrderCandidatesElectedInit(Candidate *cptr, Candidate **elected, int elected_num_from_pid, int did);
void PreOrderCandidatesElected_Final(Candidate *cptr, Candidate **elected, int elected_num_from_pid, int did);
void MinHeapDeleteMin(Candidate **elected);
void MinHeapInsert(Candidate *cptr, Candidate **elected);
void heapifyUp(int index, Candidate **elected);
void heapifyDown(int index, Candidate **elected);
int getParentIndex(int index);
int getLeftChildIndex(int index);
int getRightChildIndex(int index);
bool hasRightChild(int index);
bool hasLeftChild(int index);
void swap(int parent_index, int index, Candidate **elected);
void PreOrderCandidatesPrint_M(Candidate *cptr, int total_elected, int pid, int did);

/* event N */
ElectedCandidate *makeParliament(Candidate **elected);
ElectedCandidate *ParliamentListInsert(ElectedCandidate *L, Candidate *cptr, int pid);
Candidate *makeArrayElectedFromParty(int pid);
void InOrderCandidatesTraversal_N(Candidate *cptr, Candidate *L, int size_L);

/* event J */
void InOrderVotersTraversalPrint_J(Voter *vptr, Station *sptr);
char *hasVoted(bool voted);

/* event K */
void InOrderCandidatesTraversalPrint_K(Candidate *cptr, int pid);
/*-------------------------------------------------------------------*/

#endif
