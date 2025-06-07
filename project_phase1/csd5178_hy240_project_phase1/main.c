/*
 * ============================================
 * file: main.c
 * @Author John Malliotakis (jmal@csd.uoc.gr)
 * @Version 24/10/2023
 *
 * @e-mail hy240@csd.uoc.gr
 *
 * @brief Main function
 *        for CS240 Project Phase 1,
 *        Winter Semester 2024-2025
 * @see   Compile using supplied Makefile by running: make
 * ============================================
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "elections.h"

/* Maximum input line size */
#define MAX_LINE 1024

/* 
 * Uncomment the following line to
 * enable debugging prints
 * or comment to disable it
 */
/* #define DEBUG */
#ifdef DEBUG
//#define DPRINT(...) fprintf(stderr, __VA_ARGS__);
#else
#define DPRINT(...)
#endif /* DEBUG */

/*
 * Globals:
 * you may add some here for certain events
 * (such as D and P)
 */
struct district Districts[56];
struct party Parties[5];
struct parliament Parliament;

void destroy_structures(void)
{
	/*
	 * TODO: For a bonus
	 * empty all structures
	 * and free all memory associated
	 * with list nodes here
	 */
}

int main(int argc, char *argv[])
{
	FILE *event_file;
	char line_buffer[MAX_LINE];

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <input_file>\n",
				argv[0]);
		exit(EXIT_FAILURE);
	}

	event_file = fopen(argv[1], "r");
	if (!event_file) {
		perror("fopen error for event file open");
		exit(EXIT_FAILURE);
	}

	while (fgets(line_buffer, MAX_LINE, event_file)) {
		char *trimmed_line;
		char event;
		int did, sid, vid, cid, pid, seats;
		int ret = 0;
		/*
		 * First trim any whitespace
		 * leading the line.
		 */
		trimmed_line = line_buffer;
		while (trimmed_line && isspace(*trimmed_line))
			trimmed_line++;
		if (!trimmed_line)
			continue;
		/* 
		 * Find the event,
		 * or comment starting with #
		 */
		if (sscanf(trimmed_line, "%c", &event) != 1) {
			fprintf(stderr, "Could not parse event type out of input line:\n\t%s",
					trimmed_line);
			fclose(event_file);
			exit(EXIT_FAILURE);
		}

		switch (event) {
			/* Comment, ignore this line */
			case '#':
				break;
			case 'A':
				announce_elections();
				break;
			case 'D':
				if (sscanf(trimmed_line, "D %d %d", &did, &seats) != 2) {
					fprintf(stderr, "Event D parsing error\n");
					ret = 1;
					break;
				}
				ret = create_district(did, seats);
				break;
			case 'S':
				if (sscanf(trimmed_line, "S %d %d", &sid, &did) != 2) {
					fprintf(stderr, "Event S parsing error\n");
					ret = 1;
					break;
				}
				ret = create_station(sid, did);
				break;
			case 'P':
				if (sscanf(trimmed_line, "P %d", &pid) != 1) {
					fprintf(stderr, "Event P parsing error\n");
					ret = 1;
					break;
				}
				create_party(pid);
				break;
			case 'C':
				if (sscanf(trimmed_line, "C %d %d %d", &cid, &did, &pid) != 3) {
					fprintf(stderr, "Event C parsing error\n");
					ret = 1;
					break;
				}
				ret = register_candidate(cid, did, pid);
				break;
			case 'R':
				if (sscanf(trimmed_line, "R %d %d %d", &vid, &did, &sid) != 3) {
					fprintf(stderr, "Event R parsing error\n");
					ret = 1;
					break;
				}
				ret = register_voter(vid, did, sid);
				break;
			case 'U':
				if (sscanf(trimmed_line, "U %d", &vid) != 1) {
					fprintf(stderr, "Event U parsing error\n");
					ret = 1;
					break;
				}
				ret = unregister_voter(vid);
				break;
			case 'E':
				delete_empty_stations();
				break;
			case 'V':
				if (sscanf(trimmed_line, "V %d %d %d", &vid, &sid, &cid) != 3) {
					fprintf(stderr, "Event V parsing error\n");
					ret = 1;
					break;
				}
				ret = vote(vid, sid, cid);
				break;
			case 'M':
				if (sscanf(trimmed_line, "M %d", &did) != 1) {
					fprintf(stderr, "Event M parsing error\n");
					ret = 1;
					break;
				}
				count_votes(did);
				break;
			case 'G':
				form_government();
				break;
			case 'N':
				form_parliament();
				break;
			case 'I':
				if (sscanf(trimmed_line, "I %d", &did) != 1) {
					fprintf(stderr, "Event I parsing error\n");
					ret = 1;
					break;
				}
				print_district(did);
				break;
			case 'J':
				if (sscanf(trimmed_line, "J %d %d", &sid, &did) != 2) {
					fprintf(stderr, "Event J parsing error\n");
					ret = 1;
					break;
				}
				print_station(sid, did);
				break;
			case 'K':
				if (sscanf(trimmed_line, "K %d", &pid) != 1) {
					fprintf(stderr, "Event I parsing error\n");
					ret = 1;
					break;
				}
				print_party(pid);
				break;
			case 'L':
				print_parliament();
				break;
			default:
				fprintf(stderr, "WARNING: Unrecognized event %c. Continuing...\n",
						event);
				ret = 1;
				break;
		}
		fprintf(stderr, "Event %c %s.\n", event,
				ret ? "failed" : "succeeded");
	}
	fclose(event_file);
	destroy_structures();
	return 0;
}



/*
 * Announce Elections - Event A
 * Initialize global structures,
 * i.e., districts array, party array,
 * parliament list.
 * 
 * If field is of type int, it gets gets initialised with int value -1. 
 * If field is of type pointer to smth, it gets initialised with null.
 * 
 */
void announce_elections(void)
{
	int i;

	/* initialises Districts array */
	for(i=0; i<56; i++)
	{
		Districts[i].allotted = -1;
		Districts[i].blanks = -1;
		Districts[i].candidates = NULL;
		Districts[i].did = -1;
		Districts[i].seats = -1;
		Districts[i].stations = NULL;
		Districts[i].voids = -1;
	}

	/* initialises Parties array */
	for(i=0; i<5; i++)
	{
		Parties[i].elected = NULL;
		Parties[i].nelected = -1;
		Parties[i].pid = -1;
	}

	/* initialises struct Parliament */
	Parliament.members = NULL;

	/* prints event message */
	printf("\nA\n\nDONE\n\n");

}


/* 
 * Create District - Event D
 * Create and initialize a new
 * district with ID=did and
 * total seats=seats. Insert it
 * to global districts array in O(1)
 * time. Returns 0 on success, 1 on
 * failure.
 */
int create_district(int did, int seats)
{
	int i;

	/* finds first empty position */
	i = 0;
	while(i<56 && (Districts[i].did != -1))
	{
		i++;
	}

	if(i==56)
	{
		/* Districts array is full */
		/* returns 1 */
		fprintf(stderr, "ERROR: Districts array is full!\nInsertion of district with id <%d> failed! Function terminated with exit code 1\n\n", did);
		return 1;
	}

	/* initialises district */
	Districts[i].did = did;
	Districts[i].seats = seats;
	Districts[i].allotted = 0;

	/* event descriptor does not specify the need for the below initialisations, but they are just convenient for me */
	Districts[i].blanks = 0;
	Districts[i].voids = 0;
	/*-----------------------*/


	/* prints event message */
	printf("\nD <%d> <%d>\n\n  Districts = ", did, seats);
	i = 0;
	while(Districts[i].did != -1)
	{
		if(Districts[i+1].did != -1)
		{
			/* not last one */
			printf("<%d>, ", Districts[i].did);
		}
		else
		{
			/* last one */
			printf("<%d>", Districts[i].did);
		}

		i++;
	}
	printf("\n\nDONE\n\n");

	return 0;
}


/*
 * Create Station - Event S
 * Create and initialize a new voting
 * station with ID=sid, and add it to
 * the station list of the district
 * with ID=did. Returns 0 on success, 1 on
 * failure. 'registered' field gets initialised 
 * with value 0, while 'voters' list is empty 
 * (only contains the guard node).
 * 
 */
int create_station(int sid, int did)
{
	struct station *n_station, *ptr;
	int i;

	/* declares memory for new station */
	n_station = (struct station *)malloc(sizeof(struct station));
	if(!n_station)
	{
		/* mem. declaration failed */
		fprintf(stderr, "ERROR: Malloc (n_station) failed!\nFunction terminated with exit code 1\n\n");
		return 1;
	}

	/* new station's fields' initialisation */
	n_station->registered = 0;
	n_station->next = NULL;
	n_station->sid = sid;
	/* declares memory for voter sentinel node */
	n_station->voters = (struct voter *)malloc(sizeof(struct voter));
	if(!(n_station->voters))
	{
		/* mem. declaration failed */
		fprintf(stderr, "ERROR: Malloc (n_station->voters) failed!\nFunction terminated with exit code 1\n\n");
		return 1;
	}

	/* voter sentinel node's fields' initialisation */
	n_station->voters->next = NULL;
	n_station->voters->vid = -1;
	n_station->voters->voted = -1;

	/* vsentinel pointer points to the same mem. address as voters pointer */
	n_station->vsentinel = n_station->voters;

	/* searches for district with did = <did> */
	if(did == -1)
	{
		/* invalid did given */
		fprintf(stderr, "ERROR: invalid did (%d) given as parameter!\nFunction returned 1\n\n", did);
		return 1;
	}

	i = 0;
	while(i<56 && (Districts[i].did != did))
	{
		i++;
	}
	if(i == 56)
	{
		fprintf(stderr, "ERROR: district with did = <%d> not found!\nFunction returned 1\n\n", did);
		return 1;
	}

	if(!(Districts[i].stations))
	{
		/* new station is the first station to be added for district <did> */
		Districts[i].stations = n_station;
	}
	else
	{
		/* new station is added in the end of simply linked list 'stations' of district <did> */
		ptr = Districts[i].stations;
		while(ptr->next)
		{
			ptr = ptr->next;
		}
		ptr->next = n_station;
	}

	/* prints event message */
	printf("\nS <%d> <%d>\n\n  Stations = ", sid, did);
	ptr = Districts[i].stations;
	while(ptr)
	{
		if(ptr->next)
		{
			/* not last one */
			printf("<%d>, ", ptr->sid);
		}
		else
		{
			/* last one */
			printf("<%d>", ptr->sid);
		}

		ptr = ptr->next;
	}
	printf("\n\nDONE\n\n");

	return 0;
}


/*
 * Create Party - Event P
 * Create and initialize a new party with
 * ID=pid. Add it to the global party array.
 */
void create_party(int pid)
{
	int i;

	/* finds first empty position into Parties array */
	i = 0;
	while(i<5 && (Parties[i].pid != -1))
	{
		i++;
	}

	if(i == 5)
	{
		/* Parties array is full */
		fprintf(stderr, "ERROR: No space left for a new party. Parties array is full.\nFunction returned 1\n\n");
		return;
	}

	/* initialises party's fields */
	Parties[i].pid = pid;
	Parties[i].nelected = 0;

	/* prints event message */
	printf("\nP <%d>\n\n  Parties = ", pid);
	i = 0;
	while(i<5 && (Parties[i].pid != -1))
	{
		if(i+1 < 5)
		{
			if(Parties[i+1].pid != -1)
			{
				/* not the last one */
				printf("<%d>, ", Parties[i].pid);
			}
			else
			{
				/* the last one */
				printf("<%d>", Parties[i].pid);
			}
		}
		else
		{
			/* the last one */
			printf("<%d>", Parties[i].pid);
		}

		i++;
	} 
	printf("\n\nDONE\n\n");

}


/*
 * Register Candidate - Event C
 * Create and initialize the candidate with ID=cid
 * and party ID=pid, in district with ID=pid. 
 * cid may not be any of the reserved values 0 or 1,
 * and pid must be a valid party ID. Add the candidate 
 * to the candidate list of district did. Returns 0 on 
 * success, 1 on failure
 */
int register_candidate(int cid, int did, int pid)
{
	struct candidate *n_candidate, *ptr;
	int i;

	/* memory declaration for new candidate */
	n_candidate = (struct candidate *)malloc(sizeof(struct candidate));
	if(!n_candidate)
	{
		/* malloc failed */
		fprintf(stderr, "ERROR: Malloc (n_candidate) failed!\nFunction terminated with exit code 1\n\n");
		return 1;
	}
	
	/* initialises new candidate's fields */
	n_candidate->cid = cid;
	n_candidate->pid = pid;
	n_candidate->votes = 0;
	n_candidate->prev = NULL;
	n_candidate->next = NULL;
	
	/* finds district with did = <did> */
	i = 0;
	while(i<56 && (Districts[i].did != did))
	{
		i++;
	}

	if(i == 56)
	{
		/* district with ID=did not found */
		fprintf(stderr, "ERROR: district with did = <%d> not found!\nFunction returned 1\n\n", did);
		return 1;
	}

	/* adds new candidate to the double linked candidate list of district with did = <did> */
	ptr = Districts[i].candidates;
	if(!ptr)
	{
		/* new candidate is the first candidate for district with did = <did> */
		Districts[i].candidates = n_candidate;
		return 0;
	}

	/* adds new candidate to the end of the double linked candidate list */
	while(ptr->next)
	{
		if(ptr->cid == cid)
		{
			fprintf(stderr, "ERROR: candidate with cid = <%d> has already been registered\nFunction returned 1\n\n", cid);
			return 1;
		}

		ptr = ptr->next;
	}
	
	if(ptr->cid == cid)
	{
		fprintf(stderr, "ERROR: candidate with cid = <%d> has already been registered\nFunction returned 1\n\n", cid);
		return 1;
	}

	ptr->next = n_candidate;
	n_candidate->prev = ptr;


	/* prints event message */
	printf("\nC <%d> <%d> <%d>\n\n  Candidates = ", cid, did, pid);
	ptr = Districts[i].candidates;
	while(ptr)
	{
		if(ptr->next)
		{
			/* not the last one */
			printf("<%d>, ", ptr->cid);
		}
		else
		{
			/* the last one */
			printf("<%d>", ptr->cid);
		}

		ptr = ptr->next;
	}
	printf("\n\nDONE\n\n");

	return 0;
}


/*
 * Register voter - Event R
 * Creates and initializes a new voter with ID=vid, if not
 * already existing for station with ID=sid, that belongs
 * to the district with ID-did. If such a district / station
 * does not exist, function terminates and returns 1. In 
 * any other case, it adds the voter in the beginning of the 
 * voter list of station sid, which is stored in the station 
 * list of district did. Also, it increments by one the total 
 * number of registered voters belonging to the station with 
 * ID=sid. Function returns 0 on success and 1 on failure.
 * 
 */
int register_voter(int vid, int did, int sid)
{
	int i, flag;
	struct station *ptr;
	struct voter *p, *p2;

	if(vid == -1)
	{
		fprintf(stderr, "ERROR: Invalid vid (<%d>)\nFunction returned 1\n\n", vid);
		return 1;
	}
	
	/* searches for the district with did = <did> */
	i = 0;
	while (i<56 && (Districts[i].did != did))
	{
		i++;
	}

	if(i == 56)
	{
		/* district with did = <did> not found */
		fprintf(stderr, "ERROR: district with did = <%d> not found!\nFunction returned 1\n\n", did);
		return 1;
	}

	/* district with did = <did> found */
	/* searches for the station with sid = <sid> */
	ptr = Districts[i].stations;
	while(ptr && (ptr->sid != sid))
	{
		ptr = ptr->next;
	}

	if(!ptr)
	{
		/* station with sid = <sid> not found */
		fprintf(stderr, "ERROR: station with sid = <%d> not found!\nFunction returned 1\n\n", sid);
		return 1;
	}
	else
	{
		/* station with sid = <sid> found */
		/* checks if voter with vid = <vid> already exists */
		p = ptr->voters;
		flag = 0;
		while(p)
		{
			if(p->vid == vid)
			{
				/* finds already existing voter with vid = <vid> */
				flag = 1;
				break;
			}

			p = p->next;
		}

		if(flag)
		{
			/* already existing voter with vid = <vid> found */
			fprintf(stderr, "ERROR: voter with vid = <%d> already exists!\nFunction returned 1\n\n", vid);
			return 1;
		}

		/* already existing voter with vid = <vid> not found */
		/* creates new voter */
		p = (struct voter *)malloc(sizeof(struct voter));
		if(!p)
		{
			/* memory declaration for the new voter failed */
			fprintf(stderr, "ERROR: Malloc (p --> new_voter) failed!\nFunction returned 1\n\n");
			return 1;
		}

		/* initialises new voter's fields */
		p->vid = vid;
		p->voted = 0;
		p->next = NULL;

		/* adds new voter in the end of that station's voters' list, before voter-sentinel */
		if(ptr->voters == ptr->vsentinel)
		{
			/* new voter is the first voter to be added into voters' list of that station */
			p->next = ptr->voters;
			ptr->voters = p;
		}
		else
		{
			/* finds last voter (voter before vsentinel) */
			p2 = ptr->voters;
			while(p2->next->vid != -1)
			{
				p2 = p2->next;
			}
			/* new voter's next field points to vsentinel */
			p->next = ptr->vsentinel;
			/* last voter's next field points to new voter */
			p2->next = p;
		}

		/* increments that station's number of registered voters by one */
		ptr->registered += 1;

		/* prints event message */
		printf("\nR <%d> <%d> <%d>\n\n  Voters = ", vid, did, sid);
		p = ptr->voters;
		while(p && (p->vid != -1))
		{
			if(p->next->vid != -1)
			{
				/* not the last one */
				printf("<%d>, ", p->vid);
			}
			else
			{
				/* the last one */
				printf("<%d>", p->vid);
			}

			p = p->next;
		}
		printf("\n\nDONE\n\n");

		return 0;
	}
}



/*
 * Unregister voter - Event U
 * Remove voter with ID=vid from the appropriate
 * voter list. Returns 0 on success, 1 on failure.
 */
int unregister_voter(int vid)
{
	int i, found;
	struct station *ptr;
	struct voter *p1, *p2;

	/* searches for voter with vid = <vid> */
	i = 0;
	found = 0;	/* 0 for not finding voter, 1 for finding him */
	/* for each district */
	while(i<56 && (Districts[i].did != -1))
	{
		/* for each of its stations */
		ptr = Districts[i].stations;
		while(ptr)
		{
			/* searches for voter with vid = <vid> */
			p1 = ptr->voters;
			p2 = NULL;
			while(p1)
			{
				if(p1->vid == vid)
				{
					/* voter with vid = <vid> found */
					if(!p2)
					{
						/* voter with vid = <vid> is the first into voters' list */
						ptr->voters = p1->next;
						free(p1);
						p1 = NULL;
					}
					else
					{
						/* voter with vid = <vid> is not the first into voters' list */
						p2->next = p1->next;
						free(p1);
						p1 = NULL;
						p2 = NULL;
					}

					/* number of registered voters for station with sid = <ptr->sid> decrements by one */
					ptr->registered -= 1;

					/* prints event message */
					printf("\nU <%d>\n\n  <%d> <%d>\n\n  Voters = ", vid, Districts[i].did, ptr->sid);
					p1 = ptr->voters;
					while(p1 && (p1->vid != -1))
					{
						if(p1->next->vid != -1)
						{
							printf("<%d>, ", p1->vid);
						}
						else
						{
							printf("<%d>", p1->vid);
						}

						p1 = p1->next;
					}
					printf("\n\nDONE\n\n");

					found = 1;
					/* skips 3rd while loop */
					break;
				}

				p2 = p1;
				p1 = p1->next;
			}

			if(found)
			{
				/* skips 2nd while loop */
				break;
			}

			ptr = ptr->next;
		}

		if(found)
		{
			/* skips 1st while loop */
			break;
		}

		i++;
	}

	if(found)
	{
		/* voter was found and deleted */
		return 0;
	}
	else
	{
		/* voter was not found */
		fprintf(stderr, "ERROR: voter with vid = <%d> not found!\nFunction returned 1\n\n", vid);
		return 1;
	}

}


/*
 * Delete empty stations - Event E
 * Iterate all voting stations across all districts.
 * Remove from the corresponding station lists any station
 * with 0 registered voters. O(n) time complexity, where
 * n = total voting stations across all districts.
 */
void delete_empty_stations(void)
{
	int i;
	struct station *ptr1, *ptr2;

	/* prints event message along the way */
	printf("\nE\n");

	/* traverses districts */
	i = 0;
	while(i<56 && (Districts[i].did != -1))
	{
		ptr1 = Districts[i].stations;
		ptr2 = NULL;
		/* traverses stations */
		while(ptr1)
		{
			if(!(ptr1->registered))
			{
				/* empty station found */
				
				/* deletes voter-sentinel */
				free(ptr1->voters);
				ptr1->voters = NULL;
				ptr1->vsentinel = NULL;

				if(!ptr2)
				{
					/* station to be deleted is the first station for district i */
					Districts[i].stations = ptr1->next;
					printf("\n <%d> <%d>", ptr1->sid, Districts[i].did);
					free(ptr1);
					ptr1 = Districts[i].stations;
				}
				else
				{
					/* station to be deleted is not the first station for district i */
					ptr2->next = ptr1->next;
					printf("\n <%d> <%d>", ptr1->sid, Districts[i].did);
					free(ptr1);
					ptr1 = ptr2->next;
				}
			}
			else
			{
				ptr2 = ptr1;
				ptr1 = ptr1->next;
			}
	
		}

		i++;
	}
	printf("\n\nDONE\n\n");
}


/*
 * Vote - Event V.
 * Voter vid votes at station sid for candidate cid. It is an error
 * if any input argument is invalid (no voter vid, or no station sid,
 * or cid does not match any registered candidate (and is also not 0 or 1)).
 * Otherwise, add one vote to candidate cid, and swap places with previous
 * candidate in list if candidate cid now has more votes than the previous.
 * Returns 0 on success, 1 on failure.
 */
int vote(int vid, int sid, int cid)
{
	int i, station_found;
	struct station *ptr;
	struct voter *p;
	struct candidate *cptr, *cptr2;

	if(vid == -1)
	{
		fprintf(stderr, "ERROR: Invalid vid (-1)!\nFunction returned 1\n\n");
		return 1;
	}

	station_found = 0;
	i = 0;
	while(i<56 && (Districts[i].did != -1))
	{
		ptr = Districts[i].stations;
		while(ptr && (ptr->sid != sid))
		{
			ptr = ptr->next;
		}

		if(ptr)
		{
			/* station with ID=sid found */
			station_found = 1;
			break;
		}

		i++;
	}

	if(!station_found)
	{
		/* station with ID=sid not found finally */
		fprintf(stderr, "ERROR: station with sid = <%d> not found!\nFunction returned 1\n\n", sid);
		return 1;
	}
	


	p = ptr->voters;
	while((p->vid != -1) && (p->vid != vid))
	{
		p = p->next;
	}

	if(!(p->vid != -1))
	{
		/* voter with ID=vid not found into station with ID=sid finally */
		fprintf(stderr, "ERROR: voter with vid = <%d> not found!\nFunction returned 1\n\n", vid);
		return 1;
	}
	
	/* voter with ID=vid found */
	
	p->voted = 1;	/* voter votes */
	
	if(cid == 0)
	{
		Districts[i].blanks ++;
	}
	else if(cid == 1)
	{
		Districts[i].voids ++;
	}
	else
	{
		cptr = Districts[i].candidates;
		while(cptr && (cptr->cid != cid))
		{
			cptr = cptr->next;
		}

		if(!cptr)
		{
			/* candidate with ID=cid not found finally */
			fprintf(stderr, "ERROR: candidate with cid = <%d> not found!\nFunction returned 1\n\n", cid);
			return 1;
		}

		/* candidate with ID=cid found finally */

		cptr->votes ++; /* his votes increment by one */
		
		cptr2 = cptr->prev;
		while(cptr2 && ((cptr->votes) > (cptr2->votes)))
		{
			if((cptr2->prev) && (cptr->next))
			{
				/* cptr2 is not the first and cptr is not the last 
				   candidate into candidates' list */
				cptr2->prev->next = cptr;
				cptr->prev = cptr2->prev;
				cptr2->prev = cptr;
				cptr2->next = cptr->next;
				cptr->next = cptr2;
				cptr2->next->prev = cptr2;

				cptr2 = cptr->prev;
			}
			else if(cptr->next)
			{
				/* cptr2 is the first and cptr is not the last
				   candidate into candidates' list */
				Districts[i].candidates = cptr;
				cptr->prev = NULL;
				cptr->next->prev = cptr2;
				cptr2->prev = cptr;
				cptr2->prev = cptr;
				cptr2->next = cptr->next;
				cptr->next = cptr2;

				cptr2 = NULL;
			}
			else if(cptr2->prev)
			{
				/* cptr2 is not the first and cptr is the last
				   candidate into candidates' list */
				cptr2->prev->next = cptr;
				cptr->next = cptr2;
				cptr2->next = NULL;
				cptr->prev = cptr2->prev;
				cptr2->prev = cptr;

				cptr2 = cptr->prev;
			}
			else
			{
				/* cptr2 is the first and cptr is the last 
				   candidate into candidates' list */
				Districts[i].candidates = cptr;
				cptr->prev = NULL;
				cptr->next = cptr2;
				cptr2->next = NULL;
				cptr2->prev = cptr;

				cptr2 = NULL;
			}
		}
	}

	/* prints event message */
	printf("\nV <%d> <%d> <%d>\n\n  District = <%d>\n  Candidate votes = ", vid, sid, cid, Districts[i].did);
	cptr = Districts[i].candidates;
	while(cptr)
	{
		if(cptr->next)
		{
			/* not the last candidate */
			printf("(<%d>, <%d>), ", cptr->cid, cptr->votes);
		}
		else
		{
			/* the last candidate */
			printf("(<%d>, <%d>)", cptr->cid, cptr->votes);
		}

		cptr = cptr->next;
	}
	printf("\n  Blanks = <%d>\n  Voids = <%d>", Districts[i].blanks, Districts[i].voids);
	printf("\n\nDONE\n\n");
	

	return 0;
}



/*
 * Count Votes - Event M
 * Count votes for district with ID=did.
 * Compute the total seats per party, say x, based on total candidate votes.
 * For each party, the first x candidates are removed from the district candidate
 * list, and added to the party's elected candidates list, and the
 * district allotted field is increased by x. 
 */
void count_votes(int did)
{
	int i, j, valid_votes, totalVotes, pvotes[5][2] = {0};
	long double electional_metre;
	struct candidate *cptr, *cptr2, *cptr3, *cptr4;
	struct station *ptr;
	struct voter *p;

	i = 0;
	while(i<56 && (Districts[i].did != did))
	{
		i++;
	}

	if(i == 56)
	{
		/* district with ID=did not found */
		fprintf(stderr, "ERROR: district with did = <%d> not found!\nFunction terminated\n\n", did);
		return;
	}

	/* district with ID=did found */
	
	cptr = Districts[i].candidates;
	while(cptr)
	{
		pvotes[cptr->pid][0] += cptr->votes;

		cptr = cptr->next;
	}
/*
	valid_votes = 0;
	for(j=0; j<5; j++)
	{
		valid_votes += pvotes[j][0];
	}
*/
	totalVotes = 0;
	ptr = Districts[i].stations;
	while(ptr)
	{
		p = ptr->voters;
		while(p->next)
		{
			if(p->voted)
			{
				totalVotes += 1;
			}

			p = p->next;
		}

		ptr = ptr->next;
	}

	valid_votes = totalVotes - ((Districts[i].voids) + (Districts[i].blanks));
	
	electional_metre = (long double)(valid_votes) / Districts[i].seats;

	for(j=0; j<5; j++)
	{
		if(electional_metre)
		{
			pvotes[j][1] = (int)(pvotes[j][0] / electional_metre);
		}
		else
		{
			/* corner case (electional_metre == 0): no seat is given to any party */
			pvotes[j][1] = 0;
		}
	}

	/* starts printing event message */
	printf("\nM <%d>\n\n  Seats =\n", did);

	cptr = Districts[i].candidates;
	while(cptr && (pvotes[0][1] || pvotes[1][1] || pvotes[2][1] || pvotes[3][1] || pvotes[4][1]))
	{
		if(!(cptr->elected) && pvotes[cptr->pid][1] && (cptr->votes))
		{
			cptr->elected = 1;
			pvotes[cptr->pid][1] --;
			
			Districts[i].allotted ++;

			Parties[cptr->pid].nelected ++;

			cptr2 = Parties[cptr->pid].elected;
			cptr3 = NULL;
			while(cptr2 && (cptr2->votes < cptr->votes))
			{
				cptr3 = cptr2;
				cptr2 = cptr2->next;
			}

			
			cptr4 = (struct candidate *)malloc(sizeof(struct candidate));
			if(!cptr4)
			{
				fprintf(stderr, "ERROR: Malloc (cptr4) failed!\nFunction terminated!\n\n");
				return;
			}

			*(cptr4) = *(cptr);
			cptr4->next = NULL;
			cptr4->prev = NULL;

			if(!cptr3)
			{
				/* newly elected candidate will be the first into the list */
				Parties[cptr->pid].elected = cptr4;
				cptr4->next = cptr2;
			}
			else
			{
				if(!cptr2)
				{
					/* newly elected candidate will be the last one into the list */
					cptr3->next = cptr4;
				}
				else
				{
					cptr3->next = cptr4;
					cptr4->next = cptr2;
				}
			}
			
			printf("\n    <%d> <%d> <%d>", cptr->cid, cptr->pid, cptr->votes);
		}

		cptr = cptr->next;
	}
	
	printf("\n\nDONE\n\n");
}


/*
 * Form Government - Event G
 * Distribute remaining district seats.
 * Start with the first party (by number of elected candidates).
 * For each district, distribute any remaining seat (their total number
 * is seats - allotted) to any remaining candidates of this party, similar
 * to the count votes event. If there are still seats to be distributed and
 * no more candidates of the first party, distribute the remaining seats based
 * on vote count by iterating the candidate list.
 */
void form_government(void)
{
	int i, j, flag, fpid, max, not_alloted;
	struct candidate *cptr, *cptr2, *cptr3, *cptr4;

	/* finds pid of the first party */
	flag = 0;
	max = -1; /* max is set to -1 because 0 votes will be the least valid value for num. of votes*/
	for(i=0; i<5; i++)
	{
		if(Parties[i].nelected > max)
		{
			max = Parties[i].nelected;
			fpid = Parties[i].pid;
			j = i; /* j holds the array position of the first party */
			flag = 1;
		}
	}

	if(!flag)
	{
		/* no first party, because no party has been created */
		fprintf(stderr, "ERROR: no party has been created!\nFunction terminated\n\n");
		return;
	}

	/* starts printing event message */
	printf("\nG\n\n  Seats =\n");
	
	/* for each district */
	i = 0;
	while(i<56 && (Districts[i].did != -1))
	{
		/* finds not-alloted seats */
		not_alloted = Districts[i].seats - Districts[i].allotted;

		/* for each candidate */
		cptr = Districts[i].candidates;
		/* (if not-allotted seats exist) */
		while(cptr && not_alloted)
		{
			/* if candidate is not elected and belongs to the first party */
			if(!(cptr->elected) && (cptr->pid == fpid))
			{
				/* candidate is elected */
				printf("\n    <%d> <%d> <%d>", Districts[i].did, cptr->cid, cptr->votes);

				not_alloted --;		/* not-allotted seats decrement by one */
				cptr->elected = 1;

				Parties[j].nelected ++;

				Districts[i].allotted ++;
				
				/* candidate is added to elected candidates' list for the first party */
				cptr2 = Parties[j].elected;
				cptr3 = NULL;
				while(cptr2 && (cptr2->votes < cptr->votes))
				{
					cptr3 = cptr2;
					cptr2 = cptr2->next;
				}

				/* memory declaration for the newly elected candidate */
				cptr4 = (struct candidate *)malloc(sizeof(struct candidate));
				if(!cptr4)
				{
					/* malloc failed */
					fprintf(stderr, "ERROR: Malloc (cptr4) failed!\nFunction terminated!\n\n");
					return;
				}

				/* copy of candidate's data */
				*(cptr4) = *(cptr);
				cptr4->next = NULL;
				cptr4->prev = NULL;

				if(!cptr3)
				{
					/* newly elected candidate will be the first into the list */
					Parties[cptr->pid].elected = cptr4;
					cptr4->next = cptr2;
				}
				else
				{
					if(!cptr2)
					{
						/* newly elected candidate will be the last one into the list */
						cptr3->next = cptr4;
					}
					else
					{
						cptr3->next = cptr4;
						cptr4->next = cptr2;
					}
				}
			}

			cptr = cptr->next;
		}

		/* if there are still not-allotted seats for that district */
		if(not_alloted)
		{
			/* for each candidate */
			cptr = Districts[i].candidates;
			while(cptr && not_alloted)
			{
				/* if candidate has not been elected yet */
				if(!(cptr->elected))
				{
					/* candidate is elected */
					printf("\n    <%d> <%d> <%d>", Districts[i].did, cptr->cid, cptr->votes);

					not_alloted --;		/* not-allotted seats decrement by one */
					cptr->elected = 1;

					Parties[cptr->pid].nelected ++;

					Districts[i].allotted ++;

					/* newly elected candidate is added into his party's elected candidates' list */
					cptr2 = Parties[cptr->pid].elected;
					cptr3 = NULL;
					while(cptr2 && (cptr2->votes < cptr->votes))
					{
						cptr3 = cptr2;
						cptr2 = cptr2->next;
					}

					/* memory declaration for newly elected candidate for party's elected candidates' list */
					cptr4 = (struct candidate *)malloc(sizeof(struct candidate));
					if(!cptr4)
					{
						/* malloc failed */
						fprintf(stderr, "ERROR: Malloc (cptr4) failed!\nFunction terminated!\n\n");
						return;
					}

					/* copy of data */
					*(cptr4) = *(cptr);
					cptr4->next = NULL;
					cptr4->prev = NULL;

					if(!cptr3)
					{
						/* newly elected candidate will be the first into the list */
						Parties[cptr->pid].elected = cptr4;
						cptr4->next = cptr2;
					}
					else
					{
						if(!cptr2)
						{
							/* newly elected candidate will be the last one into the list */
							cptr3->next = cptr4;
						}
						else
						{
							cptr3->next = cptr4;
							cptr4->next = cptr2;
						}
					}
				}

				cptr = cptr->next;
			}
		}

		i++;
	}

	/* print message ends */
	printf("\n\nDONE\n\n"); 
}



/*
 * Form Parliament - Event N
 * Form parliament candidate list from party elected
 * candidate lists. Merge the elected candidate list
 * for all parties into one sorted (by decreasing vote
 * count) list. O(n) time complexity, where n = total number
 * of elected candidates.
 */
void form_parliament(void)
{
	int i;
	struct candidate *cptr, *cptr2, *cptr3, *cptr4;

	/* for each party */
	i = 0;
	while(i<5)
	{
		/* checks if party is unregistered or if party has collected 0 votes */
		if((Parties[i].pid == -1) || (!(Parties[i].nelected)))
		{
			/* if so, it continuous for the next party */
			i++;
			continue;
		}

		//printf("ok1\n");

		/* traverses parliament candidate-list */
		cptr = Parliament.members;
		cptr2 = NULL;
		cptr3 = NULL;
		while(cptr)
		{
			cptr3 = cptr2;
			cptr2 = cptr;
			cptr = cptr->next;
		}

		//printf("ok2\n");

		if(!cptr2)
		{
			/* parliament candidate-list is empty */
			/* party's candidates are copied into parliament candidate-list */

			/* for each candidate of the i-th party */
			cptr = Parties[i].elected;
			while(cptr)
			{
				if(!(cptr->elected))
				{
					/* skips candidate if he has not been elected */
					cptr = cptr->next;
					continue;
				}

				/* a new candidate-node is made (malloc) */
				cptr2 = (struct candidate *)malloc(sizeof(struct candidate));
				if(!cptr2)
				{
					/* malloc failed */
					fprintf(stderr, "ERROR: Malloc (cptr2) failed!\nFunction terminated\n\n");
					return;
				}

				/* candidate's data are copied into new candidate-node */
				*(cptr2) = *(cptr);
				cptr2->next = NULL;
				cptr2->prev = NULL;
				
				if(!(Parliament.members))
				{
					/* new candidate-node is placed in the beginning of the parliament candidate-list */
					Parliament.members = cptr2;
				}
				else
				{
					/* new candidate-node is placed in the end of the parliament candidate-list */
					cptr3 = Parliament.members;
					while(cptr3->next)
					{
						cptr3 = cptr3->next;
					}
					cptr3->next = cptr2;
				}

				cptr = cptr->next;
			}
			//printf("OK\n");
		}
		else
		{
			//printf("ok3\n");
			/* parliament candidate-list is not empty */
			/* for each candidate of the i-th party */
			cptr = Parties[i].elected;
			while(cptr)
			{
				if(!(cptr->elected))
				{
					/* skips candidate if he has not been elected */
					cptr = cptr->next;
					continue;
				}

				/* a new candidate-node is made (malloc) */
				cptr3 = (struct candidate *)malloc(sizeof(struct candidate));
				if(!cptr3)
				{
					/* malloc failed */
					fprintf(stderr, "ERROR: Malloc (cptr3) failed!\nFunction terminated\n\n");
					return;
				}

				/* candidate's data are copied into new candidate-node */
				*(cptr3) = *(cptr);
				cptr3->next = NULL;
				cptr3->prev = NULL;

				/* parliament candidate-list is traversed to find the position for the new candidate-node */
				cptr2 = Parliament.members;
				cptr4 = NULL;
				while(cptr2 && (cptr2->votes < cptr3->votes))
				{
					cptr4 = cptr2;
					cptr2 = cptr2->next;
				}

				if(!cptr4)
				{
					/* new candidate-node will be placed in the beginning of the parliament candidate-list */
					Parliament.members = cptr3;
					cptr3->next = cptr2;
				}
				else
				{
					if(!cptr2)
					{
						/* new candidate-node will be placed in the end of the parliament candidate-list */
						cptr4->next = cptr3;
					}
					else
					{
						/* new candidate-node will be placed somewhere else into the parliament candidate-list */
						cptr4->next = cptr3;
						cptr3->next = cptr2;
					}
				}

				cptr = cptr->next;
			}
		}

		i++;
	}

	/* prints event message */
	printf("\nN\n\n  Members =\n");
	cptr = Parliament.members;
	while(cptr)
	{
		printf("\n    <%d> <%d> <%d>", cptr->cid, cptr->pid, cptr->votes);

		cptr = cptr->next;
	}
	printf("\n\nDONE\n\n");
	
}



/*
 * Print District - Event I
 * Print district information
 * for district with ID=did.
 */
void print_district(int did)
{
	int i;
	struct candidate *cptr;
	struct station *ptr;

	/* searches for district with ID=did */
	i = 0;
	while(i<56 && (Districts[i].did != did))
	{
		i++;
	}

	if(i == 56)
	{
		/* district with ID=did not found */
		fprintf(stderr, "ERROR: District with did = <%d> not found\nFunction terminated\n\n", did);
		return;
	}

	/* prints event message */
	printf("I <%d>\n\n  Seats = <%d>\n  Blanks = <%d>\n  Voids = <%d>\n  Candidates =", did, Districts[i].seats, Districts[i].blanks, Districts[i].voids);

	cptr = Districts[i].candidates;
	while(cptr)
	{
		printf("\n    <%d> <%d> <%d>", cptr->cid, cptr->pid, cptr->votes);

		cptr = cptr->next;
	}

	printf("\n  Stations = ");
	ptr = Districts[i].stations;
	while(ptr)
	{
		if(ptr->next)
		{
			/* not the last station */
			printf("<%d>, ", ptr->sid);
		}
		else
		{
			/* the last station */
			printf("<%d>", ptr->sid);
		}

		ptr = ptr->next;
	}
	printf("\n\nDONE\n\n");

}


/*
 * Print Station - Event J
 * Print voting station information
 * for station ID=sid, within district
 * with ID=did
 */
void print_station(int sid, int did)
{
	int i;
	struct station *ptr;
	struct voter *p;

	/* searches for district with ID=did */
	i = 0;
	while(i<56 && (Districts[i].did != did))
	{
		i++;
	}

	if(i == 56)
	{
		/* district with ID=did not found */
		fprintf(stderr, "ERROR: District with did = <%d> not found\nFunction terminated\n\n", did);
		return;
	}

	/* searches for station with ID=sid into district with ID=did */
	ptr = Districts[i].stations;
	while(ptr && (ptr->sid != sid))
	{
		ptr = ptr->next;
	}

	if(!ptr)
	{
		/* station with ID=sid not found */
		fprintf(stderr, "ERROR: Station with sid = <%d> not found into district with did = <%d>\nFunction terminated\n\n", sid, did);
		return;
	}

	/* prints event message */
	printf("J <%d>\n\n  Registered = <%d>\n  Voters =", ptr->sid, ptr->registered);

	p = ptr->voters;
	while(p->vid != -1)
	{
		printf("\n    <%d> ", p->vid);

		if(p->voted)
		{
			/* voter has voted */
			printf("<yes>");
		}
		else
		{
			/* voter has not voted */
			printf("<no>");
		}

		p = p->next;
	}
	printf("\n\nDONE\n\n");
}


/*
 * Print party - Event K
 * Print party information for party with ID=pid
 */
void print_party(int pid)
{
	int i;
	struct candidate *cptr;

	/* searches for party with ID=pid */
	i = 0;
	while(i<5 && (Parties[i].pid != pid))
	{
		i++;
	}

	if(i == 5)
	{
		/* party with ID=pid not found */
		fprintf(stderr, "ERROR: Party with pid = <%d> not found\nFunction terminated\n\n", pid);
		return;
	}

	/* prints event message */
	printf("K <%d>\n\n  Elected =", pid);
	cptr = Parties[i].elected;
	while(cptr)
	{
		printf("\n    <%d> <%d>", cptr->cid, cptr->votes);

		cptr = cptr->next;
	}
	printf("\n\nDONE\n\n");

}


/*
 * Print parliament - Event L
 * Print parliament elected candidate list.
 */ 
void print_parliament(void)
{
	struct candidate *cptr;

	/* prints event message */
	printf("L\n\n  Members =");

	cptr = Parliament.members;
	while(cptr)
	{
		printf("\n    <%d> <%d> <%d>", cptr->cid, cptr->pid, cptr->votes);

		cptr = cptr->next;
	}
	printf("\n\nDONE\n\n");
	
}



