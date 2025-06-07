#include "voting.h"

#include <stdbool.h>
#include <stdio.h>
/*--------------------------*/
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <limits.h>
/*--------------------------*/

// Enable in Makefile
#ifdef DEBUG_PRINTS_ENABLED
#define DebugPrint(...) printf(__VA_ARGS__);
#else
#define DebugPrint(...)
#endif

#define PRIMES_SZ 1024
#define DISTRICTS_SZ 56
#define PARTIES_SZ 5


struct District {
    int did;
    int seats;
    int blanks;
    int invalids;
    int partyVotes[PARTIES_SZ];
};

struct Station {
    int sid;
    int did;
    int registered;
    Voter* voters;
    Station* next;
};
struct Voter {
    int vid;
    bool voted;
    Voter* parent;
    Voter* lc;
    Voter* rc;
};

struct Party {
    int pid;
    int electedCount;
    Candidate* candidates;
};
struct Candidate {
    int cid;
    int did;
    int votes;
    bool isElected;
    Candidate* lc;
    Candidate* rc;
};

struct ElectedCandidate {
    int cid;
    int did;
    int pid;
    ElectedCandidate* next;
};

District Districts[DISTRICTS_SZ];
Station** StationsHT;
Party Parties[PARTIES_SZ];
ElectedCandidate* Parliament;

const int DefaultDid = -1;
const int BlankDid = -1;
const int InvalidDid = -2;

const int Primes[PRIMES_SZ] = {
    0, 1, 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541, 547, 557, 563, 569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619, 631, 641, 643, 647, 653, 659, 661, 673, 677, 683, 691, 701, 709, 719, 727, 733, 739, 743, 751, 757, 761, 769, 773, 787, 797, 809, 811, 821, 823, 827, 829, 839, 853, 857, 859, 863, 877, 881, 883, 887, 907, 911, 919, 929, 937, 941, 947, 953, 967, 971, 977, 983, 991, 997, 1009, 1013, 1019, 1021, 1031, 1033, 1039, 1049, 1051, 1061, 1063, 1069, 1087, 1091, 1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151, 1153, 1163, 1171, 1181, 1187, 1193, 1201, 1213, 1217, 1223, 1229, 1231, 1237, 1249, 1259, 1277, 1279, 1283, 1289, 1291, 1297, 1301, 1303, 1307, 1319, 1321, 1327, 1361, 1367, 1373, 1381, 1399, 1409, 1423, 1427, 1429, 1433, 1439, 1447, 1451, 1453, 1459, 1471, 1481, 1483, 1487, 1489, 1493, 1499, 1511, 1523, 1531, 1543, 1549, 1553, 1559, 1567, 1571, 1579, 1583, 1597, 1601, 1607, 1609, 1613, 1619, 1621, 1627, 1637, 1657, 1663, 1667, 1669, 1693, 1697, 1699, 1709, 1721, 1723, 1733, 1741, 1747, 1753, 1759, 1777, 1783, 1787, 1789, 1801, 1811, 1823, 1831, 1847, 1861, 1867, 1871, 1873, 1877, 1879, 1889, 1901, 1907, 1913, 1931, 1933, 1949, 1951, 1973, 1979, 1987, 1993, 1997, 1999, 2003, 2011, 2017, 2027, 2029, 2039, 2053, 2063, 2069, 2081, 2083, 2087, 2089, 2099, 2111, 2113, 2129, 2131, 2137, 2141, 2143, 2153, 2161, 2179, 2203, 2207, 2213, 2221, 2237, 2239, 2243, 2251, 2267, 2269, 2273, 2281, 2287, 2293, 2297, 2309, 2311, 2333, 2339, 2341, 2347, 2351, 2357, 2371, 2377, 2381, 2383, 2389, 2393, 2399, 2411, 2417, 2423, 2437, 2441, 2447, 2459, 2467, 2473, 2477, 2503, 2521, 2531, 2539, 2543, 2549, 2551, 2557, 2579, 2591, 2593, 2609, 2617, 2621, 2633, 2647, 2657, 2659, 2663, 2671, 2677, 2683, 2687, 2689, 2693, 2699, 2707, 2711, 2713, 2719, 2729, 2731, 2741, 2749, 2753, 2767, 2777, 2789, 2791, 2797, 2801, 2803, 2819, 2833, 2837, 2843, 2851, 2857, 2861, 2879, 2887, 2897, 2903, 2909, 2917, 2927, 2939, 2953, 2957, 2963, 2969, 2971, 2999, 3001, 3011, 3019, 3023, 3037, 3041, 3049, 3061, 3067, 3079, 3083, 3089, 3109, 3119, 3121, 3137, 3163, 3167, 3169, 3181, 3187, 3191, 3203, 3209, 3217, 3221, 3229, 3251, 3253, 3257, 3259, 3271, 3299, 3301, 3307, 3313, 3319, 3323, 3329, 3331, 3343, 3347, 3359, 3361, 3371, 3373, 3389, 3391, 3407, 3413, 3433, 3449, 3457, 3461, 3463, 3467, 3469, 3491, 3499, 3511, 3517, 3527, 3529, 3533, 3539, 3541, 3547, 3557, 3559, 3571, 3581, 3583, 3593, 3607, 3613, 3617, 3623, 3631, 3637, 3643, 3659, 3671, 3673, 3677, 3691, 3697, 3701, 3709, 3719, 3727, 3733, 3739, 3761, 3767, 3769, 3779, 3793, 3797, 3803, 3821, 3823, 3833, 3847, 3851, 3853, 3863, 3877, 3881, 3889, 3907, 3911, 3917, 3919, 3923, 3929, 3931, 3943, 3947, 3967, 3989, 4001, 4003, 4007, 4013, 4019, 4021, 4027, 4049, 4051, 4057, 4073, 4079, 4091, 4093, 4099, 4111, 4127, 4129, 4133, 4139, 4153, 4157, 4159, 4177, 4201, 4211, 4217, 4219, 4229, 4231, 4241, 4243, 4253, 4259, 4261, 4271, 4273, 4283, 4289, 4297, 4327, 4337, 4339, 4349, 4357, 4363, 4373, 4391, 4397, 4409, 4421, 4423, 4441, 4447, 4451, 4457, 4463, 4481, 4483, 4493, 4507, 4513, 4517, 4519, 4523, 4547, 4549, 4561, 4567, 4583, 4591, 4597, 4603, 4621, 4637, 4639, 4643, 4649, 4651, 4657, 4663, 4673, 4679, 4691, 4703, 4721, 4723, 4729, 4733, 4751, 4759, 4783, 4787, 4789, 4793, 4799, 4801, 4813, 4817, 4831, 4861, 4871, 4877, 4889, 4903, 4909, 4919, 4931, 4933, 4937, 4943, 4951, 4957, 4967, 4969, 4973, 4987, 4993, 4999, 5003, 5009, 5011, 5021, 5023, 5039, 5051, 5059, 5077, 5081, 5087, 5099, 5101, 5107, 5113, 5119, 5147, 5153, 5167, 5171, 5179, 5189, 5197, 5209, 5227, 5231, 5233, 5237, 5261, 5273, 5279, 5281, 5297, 5303, 5309, 5323, 5333, 5347, 5351, 5381, 5387, 5393, 5399, 5407, 5413, 5417, 5419, 5431, 5437, 5441, 5443, 5449, 5471, 5477, 5479, 5483, 5501, 5503, 5507, 5519, 5521, 5527, 5531, 5557, 5563, 5569, 5573, 5581, 5591, 5623, 5639, 5641, 5647, 5651, 5653, 5657, 5659, 5669, 5683, 5689, 5693, 5701, 5711, 5717, 5737, 5741, 5743, 5749, 5779, 5783, 5791, 5801, 5807, 5813, 5821, 5827, 5839, 5843, 5849, 5851, 5857, 5861, 5867, 5869, 5879, 5881, 5897, 5903, 5923, 5927, 5939, 5953, 5981, 5987, 6007, 6011, 6029, 6037, 6043, 6047, 6053, 6067, 6073, 6079, 6089, 6091, 6101, 6113, 6121, 6131, 6133, 6143, 6151, 6163, 6173, 6197, 6199, 6203, 6211, 6217, 6221, 6229, 6247, 6257, 6263, 6269, 6271, 6277, 6287, 6299, 6301, 6311, 6317, 6323, 6329, 6337, 6343, 6353, 6359, 6361, 6367, 6373, 6379, 6389, 6397, 6421, 6427, 6449, 6451, 6469, 6473, 6481, 6491, 6521, 6529, 6547, 6551, 6553, 6563, 6569, 6571, 6577, 6581, 6599, 6607, 6619, 6637, 6653, 6659, 6661, 6673, 6679, 6689, 6691, 6701, 6703, 6709, 6719, 6733, 6737, 6761, 6763, 6779, 6781, 6791, 6793, 6803, 6823, 6827, 6829, 6833, 6841, 6857, 6863, 6869, 6871, 6883, 6899, 6907, 6911, 6917, 6947, 6949, 6959, 6961, 6967, 6971, 6977, 6983, 6991, 6997, 7001, 7013, 7019, 7027, 7039, 7043, 7057, 7069, 7079, 7103, 7109, 7121, 7127, 7129, 7151, 7159, 7177, 7187, 7193, 7207, 7211, 7213, 7219, 7229, 7237, 7243, 7247, 7253, 7283, 7297, 7307, 7309, 7321, 7331, 7333, 7349, 7351, 7369, 7393, 7411, 7417, 7433, 7451, 7457, 7459, 7477, 7481, 7487, 7489, 7499, 7507, 7517, 7523, 7529, 7537, 7541, 7547, 7549, 7559, 7561, 7573, 7577, 7583, 7589, 7591, 7603, 7607, 7621, 7639, 7643, 7649, 7669, 7673, 7681, 7687, 7691, 7699, 7703, 7717, 7723, 7727, 7741, 7753, 7757, 7759, 7789, 7793, 7817, 7823, 7829, 7841, 7853, 7867, 7873, 7877, 7879, 7883, 7901, 7907, 7919, 7927, 7933, 7937, 7949, 7951, 7963, 7993, 8009, 8011, 8017, 8039, 8053, 8059, 8069, 8081, 8087, 8089, 8093, 8101, 8111, 8117, 8123
};

int MaxStationsCount;
int MaxSid;

int count;

/*----------------------------------------------------------*/
/* global variable holding the size of 'stations' hashtable */
int hashtable_size;
/* variables for hash function. they obtein their final value below */
int a, b, p;

/*---- my hash function ------------------------------------*/
int hash(int sid){
	return (((a*sid + b) % p) % hashtable_size);
}

/*--- function that initialises hash variables -------------*/
void initialise_hash_vars(void){
	srand(time(0)); /* different random nums generated each time */
	a = (rand() % (MaxSid - 0 + 1)) + 0; /* a = random num in [0, MaxSid] */
	b = (rand() % (MaxSid - 1 + 1)) + 1; /* b = random num in [1, MaxSid] (b must not be 0) */
	int i;
	for(i=0; i<PRIMES_SZ; i++){     /* devision with prime numbers helps making a more uniform spreading */
		if(Primes[i] > MaxSid){     /* a different prime number is chosen each time the program runs */
			p = Primes[i];
			break;
		}
	}
}
/*---------------------------------------------------------*/


void EventAnnounceElections(int parsedMaxStationsCount, int parsedMaxSid) 
{
    DebugPrint("A %d %d\n", parsedMaxStationsCount, parsedMaxSid);
    
    /* initialises global variables MaxStationCount and MaxSid */
    MaxStationsCount = parsedMaxStationsCount;
    MaxSid = parsedMaxSid;

    /* initialises Districts' array */
    for(int i=0; i<DISTRICTS_SZ; i++)
    {
        Districts[i].did = -1;
        Districts[i].seats = -1;
        Districts[i].blanks = 0;
        Districts[i].invalids = 0;
        for(int j=0; j<PARTIES_SZ; j++)
        {
            Districts[i].partyVotes[j] = 0;
        }
    }

    /* hashtable's size is defined, according to max number of stations */
    hashtable_size = MaxStationsCount/5; /* Number 5 is chosen because I would like each hashtable chain to contain 5 voting stations,
                                          if hashing works perfectly well and as a result, voting stations are sorted in the most uniform 
                                          way possible ((MaxStationsCount/hashtable_size)==5  ==>  (MaxStationsCount/5)==hashtable_size). */
    /* memory declaration for stations' hashtable */
    StationsHT = (Station **)malloc(hashtable_size * sizeof(Station *));
    if(!StationsHT)
    {
        /* malloc failed */
        fprintf(stderr, "ERROR: Malloc <(StationsHT)> <(event A)> failed\nEvent A terminated with exit code -1\n\n");
        exit(-1);
    }

    /* stations' hashtable is initialised with null */
    for(int i=0; i<hashtable_size; i++)
    {
        StationsHT[i] = NULL;
    }

    /* hash function's variables (a, b and p) are initialised */
    initialise_hash_vars();

    /* initialises Parties' array */
    for(int i=0; i<PARTIES_SZ; i++)
    {
        Parties[i].pid = i;
        Parties[i].electedCount = 0;
        Parties[i].candidates = NULL;
    }

    /* initialises Parliament */
    Parliament = NULL;

    /* prints event message */
    printf("\nA <%d> <%d>\n\nDONE\n\n", MaxStationsCount, MaxSid);
    
}


void EventCreateDistrict(int did, int seats) 
{
    int position, i;

    DebugPrint("D %d %d\n", did, seats);

    /* position of the first empty slot into Districts array is returned */
    position = returnPosition();

    if(position >= 0)
    {
        /* no error occured and position holds an accurate value */
        Districts[position].did = did;
        Districts[position].seats = seats;
    }
    else
    {
        /* Districts array is full */
        fprintf(stderr, "ERROR: Districts array is full\nEventCreateDistrict() terminated\n\n");
        return;
    }

    /* prints event message */
    printf("\nD <%d> <%d>\n\n  Districts\n  ", did, seats);

    i=0;
    while((i<DISTRICTS_SZ) && (Districts[i].did != -1))
    {
        if((i+1) < DISTRICTS_SZ)
        {
            if(Districts[i+1].did != -1)
            {
                /* not the last district to be printed */
                printf("<%d>, ", Districts[i].did);
            }
            else
            {
                /* last district to be printed */
                printf("<%d>", Districts[i].did);
            }
        }
        else
        {
            /* last district to be printed */
            printf("<%d>", Districts[i].did);
        }

        i++;
    }

    printf("\n\nDONE\n\n");

}


/* 
 * int returnPosition()
 * 
 * This is a function that returns the position of the first empty slot 
 */
int returnPosition()
{
    int range, position, start;
    
    range = DISTRICTS_SZ;
    start = 0;
    
    /*--------------------- finds the first gap into the array, as we look at it from the left to the right -----------------*/
    while(!(range <= 0))
    {
        position = start + (range/2);
        if(Districts[position].did == -1)
        {
            if((position == 0) || (Districts[position-1].did != -1))
            {
                /* first empty place into the array found */
                return position;
            }

            /* ignores right half */
            range = range/2;
        }
        else
        {
            /* ignores left half */
            start = position + 1;
            range = range - (range/2) - 1;    /* and now range covers previous right half */
        }
    }

    /* error --> no empty slot found into Districts array */
    return -1;

}


void EventCreateStation(int sid, int did) {
    DebugPrint("S %d %d\n", sid, did);
    
    Station *ptr_new, *ptr1, *ptr2;

    ptr_new = (Station *)malloc(sizeof(Station));
    if(!ptr_new)
    {
        fprintf(stderr, "ERROR: Malloc failed\nEventCreateStation() terminated\nProgram terminated with exit code -1\n\n");
        exit(-1);
    }

    ptr_new->did = did;
    ptr_new->sid = sid;
    ptr_new->registered = 0;
    ptr_new->voters = NULL;


    ptr1 = StationsHT[hash(sid)];
    ptr2 = NULL;
    while(ptr1)
    {
        if(ptr1->sid == sid)
        {
            break;
        }

        ptr2 = ptr1;
        ptr1 = ptr1->next;
    }

    if(ptr1)
    {
        fprintf(stderr, "ERROR: Station with sid = <%d> already exists into 'Stations' hashtable\nEventCreateStation() terminated\nProgram terminated with exit code -1\n\n", sid);
        free(ptr_new);
        exit(-1);
    }
    
    if(!ptr2)
    {
        /* empty chain --> new station will be added in the beginning of the chain */
        StationsHT[hash(sid)] = ptr_new;
        ptr_new->next = NULL;
    }
    else
    {
        /* not empty chain */
        ptr1 = StationsHT[hash(sid)];
        ptr2 = NULL;
        while(ptr1 && ((ptr1->sid) < (ptr_new->sid)))
        {
            ptr2 = ptr1;
            ptr1 = ptr1->next;
        }

        if(!ptr2)
        {
            /* new station will be added in the beginning of the chain */
            StationsHT[hash(sid)] = ptr_new;
            ptr_new->next = ptr1;
        }
        else
        {
            ptr2->next = ptr_new;
            ptr_new->next = ptr1;
        }
    }
    
    /* prints event message */
    printf("\nS <%d> <%d>\n\nStations[%d]\n", sid, did, hash(sid));

    ptr1 = StationsHT[hash(sid)];
    while(ptr1)
    {
        if(ptr1->next)
        {
            printf("<%d>, ", ptr1->sid);
        }
        else
        {
            printf("<%d>", ptr1->sid);
        }

        ptr1 = ptr1->next;
    }

    printf("\n\nDONE\n\n");

}


/*================= Event R (register voter) =========================================================*/
void EventRegisterVoter(int vid, int sid) {
    DebugPrint("R %d %d\n", vid, sid);
    
    Station *ptr;
    int num_of_voters;

    ptr = StationsHT[hash(sid)];
    while(ptr && (ptr->sid != sid))
    {
        ptr = ptr->next;
    }

    if(!ptr)
    {
        fprintf(stderr, "ERROR: Station with sid = <%d> not found\nEventRegisterVoter() terminated\nProgram terminated with exit code -1\n\n", sid);
        exit(-1);
    }

    ptr->voters = voterInsert(ptr->voters, vid, ptr);

    /* prints event message */
    printf("R <%d> <%d>\n\n  Voters[<%d>]\n  ", vid, ptr->sid, ptr->sid);

    count = 0;
    InOrderVotersCount(ptr->voters);
    num_of_voters = count;

    count = 0;
    InOrderVotersPrint(ptr->voters, num_of_voters);
    printf("\n\nDONE\n\n");
    
}


Voter *voterInsert(Voter *vtrs, int vid, Station *ptr)
{
    Voter *new_vtr, *vtr1;
    int level, number_of_nodes, num_of_leafs, counter, i, k;

    /* memory allocation for the new voter */
    new_vtr = (Voter *)malloc(sizeof(Voter));
    if(!new_vtr)
    {
        fprintf(stderr, "ERROR: Malloc failed\nEventRegisterVoter() terminated\nProgram terminated with exit code -1\n\n");
        exit(-1);
    }

    /* new voter's fields are initialised */
    new_vtr->voted = false;
    new_vtr->parent = NULL;
    new_vtr->lc = NULL;
    new_vtr->rc = NULL;
    new_vtr->vid = vid;

    if(!vtrs)
    {
        /* voters' tree is empty */
        /* voters' tree of height 0 is created and returned */
        (ptr->registered)++;
        return new_vtr;
    }
    else
    {
        /* voters' tree is not empty */
        if(vtrs->lc)
        {
            if(vtrs->rc)
            {
                /* tree has at least 3 voters already */
                number_of_nodes = ptr->registered; /* number of tree nodes (existing voters)*/
                level = (int)(log(number_of_nodes) / log(2)); /* finds the tree level */
                
                /* finds num of nodes from level no. '0' up to level no. '(level-1)' */
                i=0;
                k=0;
                while(i<level)
                {
                    k += (int)pow(2, i);
                    i++;
                }

                /* finds num of nodes of level no. 'level' */
                num_of_leafs = number_of_nodes - k;
            
                if(num_of_leafs == (int)pow(2, level))
                {
                    /* tree is a perfect tree, so new node is added as its first left child of level no. '(level+1)' */
                    vtr1 = vtrs;
                    while(vtr1->lc)
                    {
                        vtr1 = vtr1->lc;
                    }
                    /* new voter is added as the left-most node of level no. '(level+1)' */
                    (ptr->registered)++;
                    vtr1->lc = new_vtr;
                    new_vtr->parent = vtr1;
                    /* modified voters' tree is returned */
                    return vtrs;
                }
                else
                {
                    /* tree consists of a perfect tree of height (level-1), into which at least one leafs have been added */
                    vtr1 = vtrs;
                    counter = 0;
                    /* finds position where new node will be placed */
                    while(counter<(level-1))
                    {
                        if(num_of_leafs < ((int)pow(2, level) / (int)pow(2, counter+1)))
                        {
                            /* checks left half of the tree */
                            vtr1 = vtr1->lc;
                        }
                        else
                        {
                            /* checks right half of the tree */
                            vtr1 = vtr1->rc;
                        }

                        counter++;
                    }
                    if(vtr1->lc)
                    {
                        /* new node will be the right child */
                        (ptr->registered)++;
                        vtr1->rc = new_vtr;
                        new_vtr->parent = vtr1;
                    }
                    else
                    {
                        /* new node will be the left child */
                        (ptr->registered)++;
                        vtr1->lc = new_vtr;
                        new_vtr->parent = vtr1;
                    }
			
		    return vtrs;
                }
            }
            else
            {
                /* voters' height is 1 */
                /* new voter is added as root's right child */
                (ptr->registered)++;
                vtrs->rc = new_vtr;
                new_vtr->parent = vtrs;
                /* modified voters' tree is returned */
                return vtrs;
            }
        }
        else
        {
            /* voters' tree is of height 0 */
            /* new voter is added as root's left child */
            (ptr->registered)++;
            vtrs->lc = new_vtr;
            new_vtr->parent = vtrs;
            /* modified voters' tree is returned */
            return vtrs;
        }
    }
}


void InOrderVotersPrint(Voter *vtrs, int num_of_nodes)
{
    if(!vtrs)
    {
        return;
    }

    InOrderVotersPrint(vtrs->lc, num_of_nodes);

    if(count < (num_of_nodes-1))
    {
        printf("<%d>, ", vtrs->vid);
    }
    else
    {
        printf("<%d>", vtrs->vid);
    }
    count++;

    InOrderVotersPrint(vtrs->rc, num_of_nodes);

}

void InOrderVotersCount(Voter *vtrs)
{
    if(!vtrs)
    {
        return;
    }

    InOrderVotersCount(vtrs->lc);
    count++;
    InOrderVotersCount(vtrs->rc);
}

/*========================================================================================================================================================*/


/*================= Event C (register candidate) =========================================================================================================*/
void EventRegisterCandidate(int cid, int pid, int did) {
    DebugPrint("C %d %d %d\n", cid, pid, did);

    Candidate *cptr, *cptr1, *new_cnd;
    int num_of_candidates;

    /* checks for invalid pid */
    if(pid>=PARTIES_SZ || pid<0)
    {
        fprintf(stderr, "ERROR: Invalid pid!\nEventRegisterCandidate() terminated\n\n");
        return;
    }

    /* memory allocation for the new candidate node */
    new_cnd = (Candidate *)malloc(sizeof(Candidate));
    if(!new_cnd)
    {
        /* malloc failed */
        fprintf(stderr, "ERROR: Malloc failed\nEventRegisterCandidate() terminated\nProgram terminated with exit code -1\n\n");
        exit(-1);
    }

    /* initialises new candidate */
    new_cnd->cid = cid;
    new_cnd->did = did;
    new_cnd->isElected = false;
    new_cnd->lc = NULL;
    new_cnd->rc = NULL;
    new_cnd->votes = 0;

    cptr = Parties[pid].candidates;
    cptr1 = NULL;
    
    if(!cptr)
    {
        /* candidate tree is empty, so new candidate will be the first candidate to be added */
        Parties[pid].candidates = new_cnd;
    }
    else
    {
        /* candidate tree is not empty */
        /* traverses the (binary search) candidate tree and finds the right position */
        while(cptr)
        {
            if((new_cnd->cid) < (cptr->cid))
            {
                /* goes to the left subtree */
                cptr1 = cptr;
                cptr = cptr->lc;
            }
            else if((new_cnd->cid) > (cptr->cid))
            {
                /* goes to the right subtree */
                cptr1 = cptr;
                cptr = cptr->rc;
            }
            else
            {
                /* error: candidate with ID=cid found */
                fprintf(stderr, "ERROR: candidate with cid=<%d> has already been registered\nEventRegisterCandidate() terminated\n\n", cid);
                free(new_cnd);
                return;
            }
        }
        /* checks whether new candidate will be the left or the right child of its parent candidate */
        if((new_cnd->cid) > (cptr1->cid))
        {
            /* right child */
            cptr1->rc = new_cnd;
        }
        else
        {
            /* left child */
            cptr1->lc = new_cnd;
        }

    }

    /* prints event message */
    printf("C <%d> <%d> <%d>\n\n  Candidates[<%d>]", cid, pid, did, pid);

    count = 0;
    InOrderCandidatesCount(Parties[pid].candidates);
    num_of_candidates = count;

    count = 0;
    InOrderCandidatesPrint(Parties[pid].candidates, num_of_candidates);
    printf("\n\nDONE\n\n");

}


void InOrderCandidatesCount(Candidate *cptr)
{
    if(!cptr)
    {
        return;
    }

    InOrderCandidatesCount(cptr->lc);
    count++;
    InOrderCandidatesCount(cptr->rc);
}


void InOrderCandidatesPrint(Candidate *cptr, int num_of_nodes)
{
    if(!cptr)
    {
        return;
    }

    InOrderCandidatesPrint(cptr->lc, num_of_nodes);

    if(count < (num_of_nodes-1))
    {
        printf("\n  <%d> <%d>,", cptr->cid, cptr->did);
    }
    else
    {
        printf("\n  <%d> <%d>", cptr->cid, cptr->did);
    }
    count++;

    InOrderCandidatesPrint(cptr->rc, num_of_nodes);

}

/*============================================================================================================================*/

/*============================================== event EventVote (V) =========================================================*/
void EventVote(int vid, int sid, int cid, int pid) 
{
    DebugPrint("V %d %d %d %d\n", vid, sid, cid, pid);
    
    Candidate *cptr;
    Station *sptr;
    Voter *vptr;

    sptr = StationsHT[hash(sid)];
    while(sptr && (sptr->sid != sid))
    {
        sptr = sptr->next;
    }

    if(!sptr)
    {
        /* station with ID=sid not found into 'stations' hashtable */
        fprintf(stderr, "ERROR: Station with sid=<%d> was not found\nEventVote() terminated\nProgram terminated with exit code -1\n\n", sid);
        exit(-1);
    }

    vptr = sptr->voters;
    preOrderVotersTraversal_EventV(vptr, sptr, vid, cid, pid);  

    /* prints event message */
    if((cid == -1) || (cid == -2))
    {
    	printf("V <%d> <%d> <%d> <%d>\n\n  Districts[<%d>]\n  blanks <%d>\n  invalids <%d>\n\n  partyVotes", vid, sid, cid, pid, sptr->did, Districts[sptr->did].blanks, Districts[sptr->did].invalids);

    	for(int i=0; i<PARTIES_SZ; i++)
    	{
        	if(i == 4)
        	{
            		/* the last one */
            		printf("\n  <%d> <%d>", i, Districts[sptr->did].partyVotes[i]);
        	}
        	else
        	{
            		/* not the last one */
            		printf("\n  <%d> <%d>,", i, Districts[sptr->did].partyVotes[i]);
        	}
    	}
    }
    else
    {
    	/* searches for candidate with ID=cid */
        cptr = Parties[pid].candidates;
        while(cptr)
        {
        	if((cptr->cid) < cid)
                {
                	/* goes to the right subtree */
                    	cptr = cptr->rc;
                }
                else if((cptr->cid) > cid)
                {
                    	/* goes to the left subtree */
                    	cptr = cptr->lc;
                }
                else
                {
                    	/* candidate with ID=cid found */
                   	break;
		}
        }

	printf("V <%d> <%d> <%d> <%d>\n\n  Districts[<%d>]\n  blanks <%d>\n  invalids <%d>\n\n  partyVotes", vid, sid, cid, pid, cptr->did, Districts[cptr->did].blanks, Districts[cptr->did].invalids);    
    	
	for(int i=0; i<PARTIES_SZ; i++)
	{
		if(i == 4)
		{
			/* the last one */
			printf("\n  <%d> <%d>", i, Districts[cptr->did].partyVotes[i]);
		}
		else
		{
			/* not the last one */
			printf("\n  <%d> <%d>,", i, Districts[cptr->did].partyVotes[i]);
		}
	}
    }

    printf("\n\nDONE\n\n");
}

bool preOrderVotersTraversal_EventV(Voter *vptr, Station *sptr, int vid, int cid, int pid)
{
    bool flag;
    Candidate *cptr;

    if(!vptr)
    {
        /* vptr == NULL */
        return false;
    }

    if(vptr->vid == vid)
    {
        /* voter was found */
        /* voter votes */
        vptr->voted = true;
        if(cid == -1)
        {
            /* voter voted blank */
            Districts[sptr->did].blanks++;
        }
        else if(cid == -2)
        {
            /* voter's vote is invalid */
            Districts[sptr->did].invalids++;
        }
        else
        {
            /* searches for candidate with ID=cid */
            cptr = Parties[pid].candidates;
            while(cptr)
            {
                if((cptr->cid) < cid)
                {
                    /* goes to the right subtree */
                    cptr = cptr->rc;
                }
                else if((cptr->cid) > cid)
                {
                    /* goes to the left subtree */
                    cptr = cptr->lc;
                }
                else
                {
                    /* candidate with ID=cid found */
                    (cptr->votes)++;
                    (Districts[cptr->did].partyVotes[pid])++;
                    return true; /* because voter was found */
		}
            }

            fprintf(stderr, "ERROR: candidate with cid=<%d> not found\npreOrderVotersTraversal_EventV() terminated\nEventVote() terminated\nProgram terminated with exit code -1\n\n", cid);
            exit(-1);
        }
    }
    else
    {
        flag = preOrderVotersTraversal_EventV(vptr->lc, sptr, vid, cid, pid);
        if(!flag)
        {
            /* voter was not found into left sub tree , so check the right one */
            flag = preOrderVotersTraversal_EventV(vptr->rc, sptr, vid, cid, pid);
        }

        return flag;
    }
}

/*========================================================================================================================================*/

/*===================================== EventCountVotes (M) ==============================================================================*/
void EventCountVotes(int did) {
    DebugPrint("M %d\n", did);
    
    int i, valid_votes, partyElected[5], total_elected;
    double electional_metre;
    
    valid_votes = 0;
    for(i=0; i<PARTIES_SZ; i++)
    {
        valid_votes += Districts[did].partyVotes[i];
    }

    if(!Districts[did].seats)
    {
        /* seats == 0 */
        electional_metre = (double)0;
    }
    else
    {
        electional_metre = (double)valid_votes / Districts[did].seats;
    }

    for(i=0; i<PARTIES_SZ; i++)
    {
        if(electional_metre)
        {
            partyElected[i] = (int)(Districts[did].partyVotes[i] / electional_metre);
            
            /* counts party's candidates in district with ID=did */
            count = 0;
            InOrderCandidatesCount_M(Parties[i].candidates, did);
            
            if(count < partyElected[i])
            {
                /* there are fewer actual candidates (that got at least one vote) 
		 * than the number of elected candidates */
                /* so partyElected[i] is corrected */
                partyElected[i] = count;
            }
            
            Parties[i].electedCount += partyElected[i]; 
            Districts[did].seats -= partyElected[i];
        }
        else
        {
            partyElected[i] = 0;
        }
    }

    /* starts printing event message */
    printf("M <%d>\n\n  seats", did); 

    for(i=0; i<PARTIES_SZ; i++)
    {
        /* for each party algorithm for the election of candidates into district with ID=did is applied */
        ElectPartyCandidatesInDistrict(i, did, partyElected[i]);
    }   

    /* counts all elected (int district with ID=did) candidates from all parties */
    for(i=0; i<PARTIES_SZ; i++)
    {
    	total_elected += partyElected[i];
    }    

    count = 0;
    for(i=0; i<PARTIES_SZ; i++)
    {
    	PreOrderCandidatesPrint_M(Parties[i].candidates, total_elected, i, did);
    }

    /* ends printing event message */
    printf("\n\nDONE\n\n");

}

void PreOrderCandidatesPrint_M(Candidate *cptr, int total_elected, int pid, int did)
{
	if(!cptr)
        {
        	/* cptr == NULL */
		return;
	}

	if((count < total_elected) && (cptr->did == did) && (cptr->isElected == true))
	{
		if(count+1 == total_elected)
		{
			/* last candidate to be printed */
			printf("\n  <%d> <%d> <%d>", cptr->cid, pid, cptr->votes);
		}
		else
		{
			/* not last candidate to be printed */
			printf("\n  <%d> <%d> <%d>,", cptr->cid, pid, cptr->votes);
		}

		count++;
	}

	PreOrderCandidatesPrint_M(cptr->lc, total_elected, pid, did);
	PreOrderCandidatesPrint_M(cptr->rc, total_elected, pid, did);
}

void InOrderCandidatesCount_M(Candidate *cptr, int did)
{
    if(!cptr)
    {
        /* cptr == NULL */
        return;
    }

    InOrderCandidatesCount_M(cptr->lc, did);
    if((cptr->did == did) && (cptr->votes))
    {
        count++;
    }
    InOrderCandidatesCount_M(cptr->rc, did);
    
}

void ElectPartyCandidatesInDistrict(int pid, int did, int elected_num_from_pid)
{
    /* pid is the party's ID */
    /* did is the district's ID */
    /* elected_num_from_pid is the number of candidates from party with ID=pid 
       to be elected into district with ID=did */

    Candidate **elected;
    Candidate *cptr;
    int i;

    if(elected_num_from_pid)
    {
    	/* memory allocation for 'elected' min-heap array */
    	elected = (Candidate **)malloc(elected_num_from_pid * sizeof(Candidate *));
    	if(!elected)
    	{
        	/* malloc failed */
        	fprintf(stderr, "ERROR: Malloc failed\nElectPartyCandidatesInDistrict() terminated\nProgram terminated with exit code -1\n\n");
        	exit(-1);
    	}
    }
    else
    {
	elected = NULL;
	return;
    }

    /* 'elected' array is initialised with NULL */
    for(i=0; i<elected_num_from_pid; i++)
    {
        elected[i] = NULL;
    }

    /* candidates' tree for party with ID=pid is traversed and 'elected' array is initialised with the first candidates with ID_did=did found */
    cptr = Parties[pid].candidates;
    count = 0;
    PreOrderCandidatesElectedInit(cptr, elected, elected_num_from_pid, did);

    cptr = Parties[pid].candidates;
    /* candidates' tree is traversed and 'elected' array takes its final form */
    PreOrderCandidatesElected_Final(cptr, elected, elected_num_from_pid, did);

    /* now , 'elected' array contains pointers to the elected (in district with ID=did) candidates from party with ID=pid */
    /* those candidates' field 'isElected' is assigned 'true' value */
    for(i=0; i<elected_num_from_pid; i++)
    {
	(elected[i])->isElected = true;
    }

}

/* function for pre-order traversal of candidates' tree for party with ID=pid, that is used to initialise 
   'elected' min-heap array for party with ID=pid */
void PreOrderCandidatesElectedInit(Candidate *cptr, Candidate **elected, int elected_num_from_pid, int did)
{
    if(!cptr)
    {
        /* cptr == NULL */
        return;
    }

    if((count < elected_num_from_pid) && (cptr->did == did) && (cptr->votes))
    {
       /* pointer cptr is inserted into 'elected' array */ 
       MinHeapInsert(cptr, elected);
    }

    PreOrderCandidatesElectedInit(cptr->lc, elected, elected_num_from_pid, did);
    PreOrderCandidatesElectedInit(cptr->rc, elected, elected_num_from_pid, did);
}


/* function for pre-order traversal of candidates' tree for party pid with
 * ID=pid, that is used to edit 'elected' min-heap array for party with ID=pid 
 * and make it reach each final stage, containing pointers to the candidates
 * that will actually be elected */
void PreOrderCandidatesElected_Final(Candidate *cptr, Candidate **elected, int elected_num_from_pid, int did)
{
	int i, found;

	found = 0;

	if(!cptr)
	{
		/* cptr == NULL */
		return;
	}

	if(elected)
        {
		for(i=0; i<elected_num_from_pid; i++)
		{
			if(cptr->cid == (elected[i])->cid)
			{
				found = 1;
			}
		}

		if((cptr->did == did) && ((cptr->votes) > ((elected[0])->votes)) && (!found))
		{
			MinHeapDeleteMin(elected);
			/* pointer cptr is inserted into 'elected' array */
			MinHeapInsert(cptr, elected);
		}
	}

	PreOrderCandidatesElected_Final(cptr->lc, elected, elected_num_from_pid, did);
	PreOrderCandidatesElected_Final(cptr->rc, elected, elected_num_from_pid, did);
}

/* min-heap delete of the root node */
void MinHeapDeleteMin(Candidate **elected)
{

	/* root node becomes the right-most last level node*/
	elected[0] = elected[count-1];
	elected[count-1] = NULL;	/* last element is deleted */
	count--; /* 'count' keeps holding heapsize, that was decremented by one right now */
	/* heapifies down the 'elected' array, starting from "root" node (its first element) */
	heapifyDown(0, elected);
}

/* min-heap insert */
void MinHeapInsert(Candidate *cptr, Candidate **elected)
{
    count++; /* count holds the heap-size */
    elected[count-1] = cptr;
    heapifyUp(count-1, elected);
}

void heapifyDown(int index, Candidate **elected)
{
	/* if index == (count-1), return (nothing to heapify down) */
	if(count > (index+1))
	{
		if(hasLeftChild(index) && hasRightChild(index))
		{
			/* parent node has both left and right child */
			if((((elected[getLeftChildIndex(index)])->votes) < ((elected[getParentIndex(index)])->votes))
				&& (((elected[getRightChildIndex(index)])->votes) < ((elected[getParentIndex(index)])->votes)))
			{
				/* both children have less votes than parent */
				if(((elected[getLeftChildIndex(index)])->votes) <= ((elected[getRightChildIndex(index)])->votes))	
				{
					/* left child has less (or equal) votes than the right child */
					/* swaps parent with left child */
					swap(index, getLeftChildIndex(index), elected);
					/* heapifies down */
					heapifyDown(getLeftChildIndex(index), elected);
				}
				else
				{
					/* right child has less votes than the left child */
					/* swaps parent with right child */
					swap(index, getRightChildIndex(index), elected);
					/* heapifies down */
					heapifyDown(getRightChildIndex(index), elected);
				}		
			}
			else if(((elected[getLeftChildIndex(index)])->votes) < ((elected[getParentIndex(index)])->votes))
			{
				/* left child only has less votes than parent */
				/* swaps parent with left child */
				swap(index, getLeftChildIndex(index), elected);
				/* heapifies down */
				heapifyDown(getLeftChildIndex(index), elected);
			}
			else if(((elected[getRightChildIndex(index)])->votes) < ((elected[getParentIndex(index)])->votes))
			{
				/* right child only has less votes than parent */
				/* swaps parent with right child */
				swap(index, getRightChildIndex(index), elected);
				/* heapifies down */
				heapifyDown(getRightChildIndex(index), elected);
			}
			/* else, no child has less votes than the parent , so there is nothing to heapify */
		}
		else if(hasLeftChild(index))
		{
			/* parent node has only left child */
			if (((elected[getLeftChildIndex(index)])->votes) < ((elected[getParentIndex(index)])->votes))
			{
				/* left child has less votes than parent */
				/* swaps parent with left child */
				swap(index, getLeftChildIndex(index), elected);
				/* heapifies down */
				heapifyDown(getLeftChildIndex(index), elected);
			}
		}
		else if(hasRightChild(index))
		{
			/* parent node has only right child */
			if (((elected[getRightChildIndex(index)])->votes) < ((elected[getParentIndex(index)])->votes))
			{
				/* right child has less votes than parent */
				/* swaps parent with right child */
				swap(index, getRightChildIndex(index), elected);
				/* heapifies down */
				heapifyDown(getRightChildIndex(index), elected);
			}
		}
		/* else, parent node has no children, so there is nothing to heapify */
	}
}

/* min-heap heapifyUp */
void heapifyUp(int index, Candidate **elected)
{
    int parent_index; 

    /* if index == 0 , return (nothing to heapify up) */
    if(index)
    {
        parent_index = getParentIndex(index); /* parent candidate's index is returned */
        if(((elected[parent_index])->votes) > ((elected[index])->votes))
        {
            /* parent candidate has collected more votes than index candidate */
            swap(parent_index, index, elected); /* there is need for a swap between parent and child */
            heapifyUp(parent_index, elected);
        }

    }

}

/* returns index (in 'elected' array) of parent node for the node represented by 'index' parameter */
int getParentIndex(int index)
{
    return (index - 1)/2;
}

/* returns index (in 'elected' array) for the left-child node for the node represented by 'index' parameter */
int getLeftChildIndex(int index)
{
	return (2*index + 1);
}

/* returns index (in 'elected' array) for the right-child node for the node represented by 'index' parameter */
int getRightChildIndex(int index)
{
	return (2*index + 2);
}

/* returns true if node 'index' has right child (in 'elected' array) 
   returns false otherwise */
bool hasRightChild(int index)
{
	return (getRightChildIndex(index) < count);
}

/* returns true if node 'index' has left child (in 'elected' array) 
   returns false otherwise */
bool hasLeftChild(int index)
{
	return (getLeftChildIndex(index) < count);
}

/* swaps parent with child */
void swap(int parent_index, int index, Candidate **elected)
{
    Candidate *temp;

    temp = elected[parent_index];
    elected[parent_index] = elected[index];
    elected[index] = temp;

}

/*==============================================================================================================================================================================*/

/*============= event N ========================================================================================================================================================*/

void EventFormParliament(void) {
    DebugPrint("N\n");
    
    int i;
    Candidate* elected[5];
    ElectedCandidate *elcptr;

    for(i=0; i<PARTIES_SZ; i++)
    {
        elected[i] = makeArrayElectedFromParty(i);
    }

    Parliament = makeParliament(elected);

    /* prints event message */
    printf("N\n\n  members");
    elcptr = Parliament;
    while(elcptr)
    {
        if(elcptr->next)
        {
            /* not last candidate to be printed */
            printf("\n  <%d> <%d> <%d>,", elcptr->cid, elcptr->pid, elcptr->did);
        }
        else
        {
            /* last candidate to be printed */
            printf("\n  <%d> <%d> <%d>", elcptr->cid, elcptr->pid, elcptr->did);
        }
    
        elcptr = elcptr->next;
    }
    printf("\n\nDONE\n\n");
    
}

ElectedCandidate *makeParliament(Candidate **elected)
{
    int i, j, k, l, m, max_cid, mpid, n;
    ElectedCandidate *L;

    i=0;
    j=0;
    k=0;
    l=0;
    m=0;
    L = NULL;
    while((i < Parties[0].electedCount) || (j < Parties[1].electedCount) || (k < Parties[2].electedCount)
                || (l < Parties[3].electedCount) || (m < Parties[4].electedCount))
    {
        max_cid = INT_MIN;
        
        for(n=0; n<PARTIES_SZ; n++)
        {
            if(n == 0)
            {
                if((i < Parties[0].electedCount) && ((elected[n][i]).cid > max_cid))
                {
                    max_cid = (elected[n][i]).cid;
                    mpid = n;
                }
            }
            else if(n == 1)
            {
                if((j < Parties[1].electedCount) && ((elected[n][j]).cid > max_cid))
                {
                    max_cid = (elected[n][j]).cid;
                    mpid = n;
                }
            }
            else if(n == 2)
            {
                if((k < Parties[2].electedCount) && ((elected[n][k]).cid > max_cid))
                {
                    max_cid = (elected[n][k]).cid;
                    mpid = n;
                }
            }
            else if(n == 3)
            {
                if((l < Parties[3].electedCount) && ((elected[n][l]).cid > max_cid))
                {
                    max_cid = (elected[n][l]).cid;
                    mpid = n;
                }
            }
            else
            {
                if((m < Parties[4].electedCount) && ((elected[n][m]).cid > max_cid))
                {
                    max_cid = (elected[n][m]).cid;
                    mpid = n;
                }
            }
        }

        if(mpid == 0)
        {
            L = ParliamentListInsert(L, &elected[mpid][i], mpid);
            i++;
        }
        else if(mpid == 1)
        {
            L = ParliamentListInsert(L, &elected[mpid][j], mpid);
            j++;
        }
        else if(mpid == 2)
        {
            L = ParliamentListInsert(L, &elected[mpid][k], mpid);
            k++;
        }
        else if(mpid == 3)
        {
            L = ParliamentListInsert(L, &elected[mpid][l], mpid);
            l++;
        }
        else
        {
            L = ParliamentListInsert(L, &elected[mpid][m], mpid);
            m++;
        }
    }

    return L;
}

ElectedCandidate *ParliamentListInsert(ElectedCandidate *L, Candidate *cptr, int pid)
{
    ElectedCandidate *elcptr, *ptr;

    elcptr = (ElectedCandidate *)malloc(sizeof(ElectedCandidate));
    if(!elcptr)
    {
        fprintf(stderr, "ERROR: Malloc failed\nParliamentListInsert() terminated\nProgram terminated with exit code -1\n\n");
        exit(-1);
    }

    elcptr->cid = cptr->cid;
    elcptr->did = cptr->did;
    elcptr->next = NULL;
    elcptr->pid = pid;

    /* inserts candidate pointed to by cptr into list L */
    if(!L)
    {
        /* first candidate into the list */
        L = elcptr;
    }
    else
    {
        /* last candidate into the list */
        ptr = L;
        while(ptr->next)
        {
            ptr = ptr->next;
        }
    
        ptr->next = elcptr;
    }

    return L;
}


Candidate *makeArrayElectedFromParty(int pid)
{
    Candidate *L;

    L = (Candidate *)malloc(Parties[pid].electedCount * sizeof(Candidate));
    if(!L)
    {
        fprintf(stderr, "ERROR: Malloc failed\nmakeListElectedParty() terminated\nProgram terminated with exit code -1\n\n");
        exit(-1);
    }

    count = Parties[pid].electedCount-1;
    InOrderCandidatesTraversal_N(Parties[pid].candidates, L, Parties[pid].electedCount);

    return L;
}


void InOrderCandidatesTraversal_N(Candidate *cptr, Candidate *L, int size_L)
{
    if(!cptr)
    {
        /* cptr == NULL*/
        return;
    }

    InOrderCandidatesTraversal_N(cptr->lc, L, size_L);

    if(count >= 0)
    {
        /* L array is not full yet */
        if(cptr->isElected)
        {
            /* cptr points to an elected candidate */
            L[count] = *cptr;    /* candidate is added into array L */
            (L[count]).lc = NULL;
            (L[count]).rc = NULL;
            count--;
        }
    }

    InOrderCandidatesTraversal_N(cptr->rc, L, size_L);
}

/*================================================================================================================================================================================*/

/*========================= event I ==============================================================================================================================================*/

void EventPrintDistrict(int did) {
    DebugPrint("I %d\n", did);
    
    int i, j, seats;
    ElectedCandidate *elcptr;

    /* traverses districts' array to find district with ID=did */
    i=0;
    while((i<DISTRICTS_SZ) && (Districts[i].did != did))
    {
	i++;
    }

    if(i == DISTRICTS_SZ)
    {
	/* district with ID=did not found */
	fprintf(stderr, "ERROR: district with ID=did not found\nEventPrintDistrict() terminated\n\n");
    	return;
    }

    /* counts total seats district with ID=did elects */
    seats = 0;
    elcptr = Parliament;
    while(elcptr)
    {
	if(elcptr->did == did)
 	{
		seats++;	
	}

	elcptr = elcptr->next;
    }

    /* prints event message */
    printf("I <%d>\n\n  seats <%d>\n  blanks <%d>\n  invalids <%d>\n\n  partyVotes", did, seats, Districts[i].blanks, Districts[i].invalids);

    j=0;
    while(j<PARTIES_SZ)
    {
	if(j+1 == PARTIES_SZ)
	{
		/* last party */
		printf("\n  <%d> <%d>", j, Districts[i].partyVotes[j]);
	}
	else
	{
		/* not last party */
		printf("\n  <%d> <%d>,", j, Districts[i].partyVotes[j]);
	}

	j++;
    }

    printf("\n\nDONE\n\n");

}
/*=======================================================================================================================================================*/

/*======================== event J ======================================================================================================================*/

void EventPrintStation(int sid) {
    DebugPrint("J %d\n", sid);

    Station *sptr;

    /* traverses appropriate (hash function) station chain to find station with ID=sid */
    sptr = StationsHT[hash(sid)];
    while(sptr && (sptr->sid != sid))
    {
	sptr = sptr->next;
    }

    if(!sptr)
    {
	/* station with ID=sid not found */
	fprintf(stderr, "ERROR: station with ID=sid not found\nEventPrintStation() terminated\n\n");
	return;
    }

    /* prints event message */
    printf("J <%d>\n\n  registered %d\n\n  voters", sid, sptr->registered);
    count = 0;
    InOrderVotersTraversalPrint_J(sptr->voters, sptr);
    printf("\n\nDONE\n\n");
}

void InOrderVotersTraversalPrint_J(Voter *vptr, Station *sptr)
{
	if(!vptr)
	{
		/* vptr == NULL */
		return;
	}

	InOrderVotersTraversalPrint_J(vptr->lc, sptr);
	
	if((count+1) < (sptr->registered))
	{
		/* not last voter to be printed */
		printf("\n  <%d> <%s>,", vptr->vid, hasVoted(vptr->voted));
	}
	else
	{
		/* last voter to be printed */
		printf("\n  <%d> <%s>", vptr->vid, hasVoted(vptr->voted));
	}
	count++;

	InOrderVotersTraversalPrint_J(vptr->rc, sptr);
}

char *hasVoted(bool voted)
{
	if(voted)
	{
		return "yes";
	}
	else
	{
		return "no";
	}
}
/*==========================================================================================================================================================*/

/*============== event K ===================================================================================================================================*/

void EventPrintParty(int pid) {
	DebugPrint("K %d\n", pid);

	if((pid >= PARTIES_SZ) || (pid < 0))
	{
		fprintf(stderr, "ERROR: invalid pid\nEventPrintParty() terminated\n\n");
		return;
	}
	
	printf("K <%d>\n\n  elected", pid);
	count = 0;
	InOrderCandidatesTraversalPrint_K(Parties[pid].candidates, pid);
	printf("\n\nDONE\n\n");

}

void InOrderCandidatesTraversalPrint_K(Candidate *cptr, int pid)
{
	if(!cptr)
	{
		/* cptr == NULL */
		return;
	}

	InOrderCandidatesTraversalPrint_K(cptr->lc, pid);
	
	if(count < (Parties[pid].electedCount))
	{
		if(cptr->isElected)
		{
			if(count+1 < (Parties[pid].electedCount))
			{
				/* not last candidate to be printed */
				printf("\n  <%d> <%d>,", cptr->cid, cptr->votes);
			}
			else
			{
				/* last candidate to be printed */
				printf("\n  <%d> <%d>", cptr->cid, cptr->votes);
			}
		}
	}

	InOrderCandidatesTraversalPrint_K(cptr->rc, pid);
}

/*==========================================================================================================================================================*/

/*========== event L =======================================================================================================================================*/

void EventPrintParliament(void) {
	DebugPrint("L\n");
    	
	ElectedCandidate *elcptr;	

	printf("L\n\n  members");
	
	elcptr = Parliament;
	while(elcptr)
	{
		if(elcptr->next)
		{
			/* not the last elected candidate to be printed */
			printf("\n  <%d> <%d> <%d>,", elcptr->cid, elcptr->pid, elcptr->did);
		}		
		else
		{
			/* last elected candidate to be printed */
			printf("\n  <%d> <%d> <%d>", elcptr->cid, elcptr->pid, elcptr->did);
		}

		elcptr = elcptr->next;
	}

	printf("\n\nDONE\n\n");
}

/*==========================================================================================================================================================*/

void EventBonusUnregisterVoter(int vid, int sid) {
    DebugPrint("BU %d %d\n", vid, sid);
    // TODO
}



void EventBonusFreeMemory(void) {
    DebugPrint("BF\n");
    
    ElectedCandidate *elcptr;

         
}


