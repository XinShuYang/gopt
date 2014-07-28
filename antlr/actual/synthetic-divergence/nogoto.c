#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<time.h>
#include<sys/ipc.h>
#include<sys/shm.h>

#include "param.h"
#include "fpp.h"

struct cache_bkt		/* 64 bytes */
{
	int slot_arr[SLOTS_PER_BKT];
};
struct cache_bkt *cache;

#define ABS(a) (a > 0 ? a : -1 * a)

// Each packet contains a random integer. The memory address accessed
// by the packet is determined by an expensive hash of the integer.
int *pkts;

int sum = 0;

// batch_index must be declared outside process_pkts_in_batch
int batch_index = 0;

// Process BATCH_SIZE pkts starting from lo
int process_pkts_in_batch(int *pkt_lo)
{
	// Like a foreach loop
	foreach(batch_index, BATCH_SIZE) {
		
		int i;
		int jumper = pkt_lo[batch_index];
			
		for(i = 0; i < DEPTH; i++) {
			FPP_EXPENSIVE(&cache[jumper]);
			int *arr = cache[jumper].slot_arr;
			int j, best_j = 0;

			int max_diff = ABS(arr[0] - jumper) % 8;

			for(j = 1; j < SLOTS_PER_BKT; j ++) {
				if(ABS(arr[j] - jumper) % 8 > max_diff) {
					max_diff = ABS(arr[j] - jumper) % 8;
					best_j = j;
				}
			}
			
			jumper = arr[best_j];
			if(jumper % 16 == 0) {		// GCC will optimize this
				break;
			}
		}

		sum += jumper;
	}
}

int main(int argc, char **argv)
{
	int i, j;

	// Allocate a large memory area
	fprintf(stderr, "Size of cache = %lu\n", NUM_BS * sizeof(struct cache_bkt));

	int sid = shmget(CACHE_SID, NUM_BS * sizeof(struct cache_bkt), 
		IPC_CREAT | 0666 | SHM_HUGETLB);
	if(sid < 0) {
		fprintf(stderr, "Could not create cache\n");
		exit(-1);
	}
	cache = shmat(sid, 0, 0);

	// Fill in the cache with index into itself
	for(i = 0; i < NUM_BS; i ++) {
		for(j = 0; j < SLOTS_PER_BKT; j++) {
			cache[i].slot_arr[j] = rand() & NUM_BS_;
		}
	}

	// Allocate the packets
	pkts = (int *) malloc(NUM_PKTS * sizeof(int));
	for(i = 0; i < NUM_PKTS; i++) {
		pkts[i] = rand() & NUM_BS_;
	}

	fprintf(stderr, "Finished creating cache and packets\n");

	long long start, end;
	start = get_cycles();

	for(i = 0; i < NUM_PKTS; i += BATCH_SIZE) {
		process_pkts_in_batch(&pkts[i]);
	}
	
	end = get_cycles();

	// xia-router2 frequency = 2.7 Ghz
	long long ns = ((long long) (end - start) / 2.7);

	printf("Total time = %f s, sum = %d\n", ns / 1000000000.0, sum);
	printf("Average time per batch = %lld ns\n", ns / (NUM_PKTS / BATCH_SIZE));
	printf("Average time per packet = %lld ns \n", ns / NUM_PKTS);

}
