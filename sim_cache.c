#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>


struct cache
{
	unsigned int c_size;
	unsigned int c_assoc;
	unsigned int c_blocksize;
	unsigned int c_replacePolicy;
	unsigned int c_writePolicy;
	unsigned int c_numOfSets;

	unsigned int* c_tagArray;
	unsigned int* dirty_bit;
	unsigned int* valid_in_bit;
	int* LRUCounter;
	int* count_set;

	unsigned int readCounter;
	unsigned int readMissCounter;
	unsigned int writeCounter;
	unsigned int writeMissCounter;
	unsigned int memoryAccessCounter;
	unsigned int noOfWritebacks;
	

	struct cache* nextLevel;

};


int toInteger(char* str);
int powerOfTwo(int  num);
void extractAddressParams(unsigned int addressInInt, struct cache* cache_ds, unsigned int* indexLocation, unsigned int* tagAddress);
int readFromAddress(struct cache* cache_ds, unsigned int indexLocation, unsigned int tagAddress);
int writeToAddress(struct cache* cache_ds, unsigned int indexLocation, unsigned int tagAddress);

void LRUForHit(struct cache* cache_ds, unsigned int indexLocation, unsigned int tagLocation);
void LRUForMiss(struct cache* cache_ds, unsigned int indexLocation, unsigned int* tagLocation);
void LeastFrequentForMiss(struct cache* cache_ds, unsigned int indexLocation, unsigned int* tagLocation);

int main(int argc, char **argv)
{

	struct cache* cache_ds = (struct cache*)malloc(sizeof(struct cache));
	
	FILE* trace_file;
	unsigned int countTraceEntries = 0, noOfTagEntries = 0, addressInInt = 0;
	unsigned int i=0, indexLocation = 0, tagAddress = 0, j=0;
	char readAddress[100];
	char *address, *isItReadOrWrite;
	double missRate = 0, accessTime = 0, missPenalty = 0, cacheHitTime = 0;

	if(argc<7)
	{
		printf("\nInsufficient Arguments Supplied. Returning..\n");
		return(0);
	}

	
	if( powerOfTwo( (cache_ds->c_blocksize = atoi(argv[1])) ) == 0 )
	{
		printf("\nBlock size is not a power of two. Returning..\n");
		return(0);
	}

	//check here for non-negative values

	cache_ds->c_size = atoi(argv[2]);
	cache_ds->c_assoc = atoi(argv[3]);
	cache_ds->c_replacePolicy = atoi(argv[4]);
	cache_ds->c_writePolicy = atoi(argv[5]);
	cache_ds->nextLevel = NULL;
	cache_ds->readCounter = 0;
	cache_ds->writeCounter = 0;
	cache_ds->readMissCounter = 0;
	cache_ds->writeMissCounter = 0;
	cache_ds->memoryAccessCounter = 0;
	cache_ds->noOfWritebacks = 0;
	

	cache_ds->c_numOfSets = (cache_ds->c_size/(cache_ds->c_blocksize*cache_ds->c_assoc));

	if( powerOfTwo( cache_ds->c_numOfSets ) == 0 )
	{
		printf("\nNumber of sets in Cache is not a power of two. Returning..\n");
		return(0);
	}
	
	
	//printf("\n Block size is : %d\n",cache_ds->c_blocksize);

	trace_file = fopen(argv[6], "r");

	noOfTagEntries = cache_ds->c_numOfSets*cache_ds->c_assoc;

	cache_ds->c_tagArray = (unsigned int*)malloc( (noOfTagEntries*sizeof(unsigned int)) );
	cache_ds->dirty_bit = (unsigned int*)malloc( (noOfTagEntries*sizeof(unsigned int)) );
	cache_ds->valid_in_bit = (unsigned int*)malloc( (noOfTagEntries*sizeof(unsigned int)) );
	cache_ds->LRUCounter = (int*)malloc( (noOfTagEntries*sizeof(int)) );
	cache_ds->count_set = (int*)malloc( ((cache_ds->c_numOfSets)*sizeof(int)) );


	memset( cache_ds->c_tagArray, 0, (sizeof(cache_ds->c_tagArray[0])*noOfTagEntries) );
	memset( cache_ds->dirty_bit, 0, (sizeof(cache_ds->dirty_bit[0])*noOfTagEntries) );
	memset( cache_ds->valid_in_bit, 0, (sizeof(cache_ds->valid_in_bit[0])*noOfTagEntries) );
	memset( cache_ds->LRUCounter, 0, (sizeof(cache_ds->LRUCounter[0])*noOfTagEntries) );
	memset( cache_ds->count_set, 0, (sizeof(cache_ds->LRUCounter[0])*(cache_ds->c_numOfSets)) );

	while( fgets(readAddress, 100, trace_file)!=NULL ) 
	{
	//	printf("\nAddress is : %s", readAddress);
		isItReadOrWrite = strtok(readAddress, " ");
		address = strtok(NULL, "\n");
		addressInInt = strtoll(address, NULL, 16); 


	//	printf("\n\n# %d : %s", (countTraceEntries+1), isItReadOrWrite);

		extractAddressParams(addressInInt, cache_ds, &indexLocation, &tagAddress);

	//	printf("\nL1 %s, %s: %x(tag %x, index %d)", isItReadOrWrite, address, addressInInt, tagAddress, indexLocation);
	//	printf("\nCurrent set %d: ", indexLocation);

		if( isItReadOrWrite[0] == 'r' || isItReadOrWrite[0] == 'R')
		{
			cache_ds->readCounter +=1;
			readFromAddress(cache_ds, indexLocation, tagAddress);
		}
		else if( isItReadOrWrite[0] == 'w' || isItReadOrWrite[0] == 'W')
		{
			cache_ds->writeCounter +=1;
			writeToAddress(cache_ds, indexLocation, tagAddress);
		}
		else
		{
			printf("\nTrace file doesn't specify R/W at line %d", (countTraceEntries+1));
			continue;
		}

	//	printf("\nChanged set %d: ", indexLocation);
		countTraceEntries++;
		
		

	}

	//printf("\n\n\n=============================\n\n Read counter: %d, r miss: %d, write counter: %d, w miss counter: %d, memory traffic counter: %d, writebacks counter : %d\n" , cache_ds->readCounter, cache_ds->readMissCounter, cache_ds->writeCounter, cache_ds->writeMissCounter, cache_ds->memoryAccessCounter, cache_ds->noOfWritebacks);	
	
	missRate = ( (double)( (int)cache_ds->readMissCounter + (int)cache_ds->writeMissCounter )/(double)( (int)cache_ds->readCounter + (int)cache_ds->writeCounter ) );

	missPenalty = (double)(20 + 0.5*( ((double)cache_ds->c_blocksize/16) ) );

	cache_ds->c_size = atoi(argv[2]);
	cache_ds->c_assoc = atoi(argv[3]);

	cacheHitTime = (double)( 0.25 + 2.5*( (double)cache_ds->c_size/(512*1024) ) + 0.025*( (double)cache_ds->c_blocksize/16 ) + 0.025*( (double)cache_ds->c_assoc ) );

	accessTime = (double)( (double)cacheHitTime + ( (double)missRate*(double)missPenalty ) );

	printf("  ===== Simulator configuration =====");
	printf("\n  L1_BLOCKSIZE:%22d", cache_ds->c_blocksize);
	printf("\n  L1_SIZE:%27d", cache_ds->c_size);
	printf("\n  L1_ASSOC:%26d", cache_ds->c_assoc);
	printf("\n  L1_REPLACEMENT_POLICY:%13d", cache_ds->c_replacePolicy);
	printf("\n  L1_WRITE_POLICY:%19d", cache_ds->c_writePolicy);
	printf("\n  trace_file:%24s", argv[6]);
	printf("\n  ===================================");
	printf("\n\n===== L1 contents =====\n");

	for( i=0; i<cache_ds->c_numOfSets; i++)
	{
		printf("set%4d:", i);
		for( j=0; j<cache_ds->c_assoc; j++)
		{
			printf("%8x ",cache_ds->c_tagArray[i + (j*cache_ds->c_numOfSets)]);
			if( cache_ds->c_writePolicy == 0)
			{
				if( cache_ds->dirty_bit[i + (j*cache_ds->c_numOfSets)] == 1)
					printf("D");
			}
		}
		printf("\n");
	}
	printf("\n");
	printf("  ====== Simulation results (raw) ======\n");
	printf("  a. number of L1 reads:%16d\n", cache_ds->readCounter);
	printf("  b. number of L1 read misses:%10d\n", cache_ds->readMissCounter);
	printf("  c. number of L1 writes:%15d\n", cache_ds->writeCounter);
	printf("  d. number of L1 write misses:%9d\n", cache_ds->writeMissCounter);
	printf("  e. L1 miss rate:%22.4f\n", missRate);
	printf("  f. number of writebacks from L1:%6d\n", cache_ds->noOfWritebacks);
	printf("  g. total memory traffic:%14d\n", cache_ds->memoryAccessCounter);
	printf("\n");

	printf("  ==== Simulation results (performance) ====\n");
	printf("  1. average access time:%15.4f ns\n", accessTime);  //========================================AAt calculate

	return(0);
}



int powerOfTwo(int num)
{
	if(num!=1)
	{
		while( (num%2 == 0) && num>1)
			num = num/2;

		if( num == 1)
			return(1);
	}
	
	return(0);
}



void extractAddressParams(unsigned int addressInInt, struct cache* cache_ds, unsigned int* indexLocation, unsigned int* tagAddress)
{
	int noOfBlockBits = 0, noOfIndexBits = 0, tempIndexNo = 0, i=0;
	
	noOfBlockBits = log2(cache_ds->c_blocksize);
	noOfIndexBits = log2(cache_ds->c_numOfSets);

	*indexLocation = addressInInt>>noOfBlockBits;

	for( i=0; i<noOfIndexBits; i++)
	{
		tempIndexNo = ( 1 | tempIndexNo<<1 );
	}

	*indexLocation = ( *indexLocation & tempIndexNo );
	*tagAddress = addressInInt>>(noOfBlockBits + noOfIndexBits);
}



int readFromAddress(struct cache* cache_ds, unsigned int indexLocation, unsigned int tagAddress)
{
	int i=0;
	unsigned int tagLocation = 0;

	for( i=0; i< (int)cache_ds->c_assoc; i++)
	{
		if( cache_ds->c_tagArray[indexLocation + (i*cache_ds->c_numOfSets)] == tagAddress )	//Checking Tag Entries
		{
		
			if(cache_ds->c_replacePolicy == 0 )	//LRU Policy
			{
				LRUForHit(cache_ds, indexLocation, ( indexLocation + (i*cache_ds->c_numOfSets) ) );
			}
			else if( cache_ds->c_replacePolicy == 1 )
			{
				cache_ds->LRUCounter[( indexLocation + (i*cache_ds->c_numOfSets) )] = ((int)cache_ds->LRUCounter[( indexLocation + (i*cache_ds->c_numOfSets) )]) + 1;
			}

			//printf("\nL1 HIT");
			return(0);
			
		}
	}


	if(cache_ds->c_replacePolicy == 0 )	//LRU Policy
	{
		//It's a Cache Miss
		//printf("\nL1 MISS");
		cache_ds->readMissCounter += 1;
		cache_ds->memoryAccessCounter += 1;		//increase the memory traffic counter
		LRUForMiss(cache_ds, indexLocation, &tagLocation);
		cache_ds->c_tagArray[tagLocation] = tagAddress;
	}
	else if( cache_ds->c_replacePolicy == 1 )
	{
		//It's a Cache Miss, LFU Policy
		//printf("\nL1 MISS");
		cache_ds->readMissCounter += 1;
		cache_ds->memoryAccessCounter += 1;
		//cache_ds->noOfWritebacks += 1;
		LeastFrequentForMiss(cache_ds, indexLocation, &tagLocation);
		cache_ds->c_tagArray[tagLocation] = tagAddress;
		cache_ds->LRUCounter[tagLocation] = (int)cache_ds->count_set[indexLocation] + 1;
	}

	if(cache_ds->c_writePolicy == 0)	//Write Back policy
	{
		if( cache_ds->dirty_bit[tagLocation] == 1 )
		{
			cache_ds->memoryAccessCounter += 1;
			cache_ds->noOfWritebacks += 1;
			cache_ds->dirty_bit[tagLocation] = 0;
		}
	}

	return(0);
}





int writeToAddress(struct cache* cache_ds, unsigned int indexLocation, unsigned int tagAddress)
{
	int i=0;
	unsigned int tagLocation = 0;

	for( i=0; i< (int)cache_ds->c_assoc; i++)
	{

		//first check tag if it is matched or not
		if( cache_ds->c_tagArray[indexLocation + (i*cache_ds->c_numOfSets)] == tagAddress )	//Checking Tag Entries
		{
			{
				//printf("\nL1 HIT");
				
				//0 for WBWA  --> Dirty bits
				//1 for WTNA  --> No use of dirty bits

				if( cache_ds->c_writePolicy == 0 )
				{
					cache_ds->dirty_bit[indexLocation + (i*cache_ds->c_numOfSets)] = 1;
				}
				else if( cache_ds->c_writePolicy == 1 )
				{
					cache_ds->memoryAccessCounter += 1;
				}

				if(cache_ds->c_replacePolicy == 0 )	//LRU Policy
				{
					LRUForHit(cache_ds, indexLocation, ( indexLocation + (i*cache_ds->c_numOfSets) ) );
				}
				else if( cache_ds->c_replacePolicy == 1 )	//LFU Policy
				{
					cache_ds->LRUCounter[( indexLocation + (i*cache_ds->c_numOfSets) )] = ((int)cache_ds->LRUCounter[( indexLocation + (i*cache_ds->c_numOfSets) )]) + 1;
				}

				return(0);
			}
		}
	}


	//Cache Miss
	for( i=0; i< (int)cache_ds->c_assoc; i++)
	{
		cache_ds->writeMissCounter += 1; 	// increase write miss counter
		cache_ds->memoryAccessCounter += 1;		//increase the memory traffic counter

		if( cache_ds->c_writePolicy == 0 )
		{
			//printf("\nL1 MISS, WRITE BACK");
			if(cache_ds->c_replacePolicy == 0 )	//LRU Policy
			{
				LRUForMiss(cache_ds, indexLocation, &tagLocation);
				cache_ds->LRUCounter[tagLocation] = 0;
			}
			else if( cache_ds->c_replacePolicy == 1 )	//LFU Policy
			{
				LeastFrequentForMiss(cache_ds, indexLocation, &tagLocation);
				cache_ds->LRUCounter[tagLocation] = (int)cache_ds->count_set[indexLocation] + 1;
			}

			if( (int)cache_ds->dirty_bit[tagLocation] == 1 )
			{
				cache_ds->memoryAccessCounter += 1;	
				cache_ds->noOfWritebacks += 1;
			}

			cache_ds->dirty_bit[tagLocation] = 1;
			cache_ds->c_tagArray[tagLocation] = tagAddress;
			
		}
		//else
		//{
	//		printf("\nL1 MISS, WRITE THROUGH");
	//	}
		return(0);
	}

	return(0);
}





void LRUForHit(struct cache* cache_ds, unsigned int indexLocation, unsigned int tagLocation)
{
	int i = 0;

	for( i=0; i< (int)cache_ds->c_assoc; i++)
	{
		if( (int)cache_ds->LRUCounter[indexLocation + (i*cache_ds->c_numOfSets)] < (int)cache_ds->LRUCounter[tagLocation] )	
		{
			cache_ds->LRUCounter[indexLocation + (i*cache_ds->c_numOfSets)] = ((int)cache_ds->LRUCounter[indexLocation + (i*cache_ds->c_numOfSets)]) + 1;
		}
	}
	
	cache_ds->LRUCounter[tagLocation] = 0;
}



void LRUForMiss(struct cache* cache_ds, unsigned int indexLocation, unsigned int* tagLocation)
{
	unsigned int i = 0;
	int max = -1;
	*tagLocation = 0;
	//printf("\nL1 UPDATE LRU");
	for( i=0; i<cache_ds->c_assoc; i++)
	{
		if( (int)cache_ds->LRUCounter[indexLocation + (i*cache_ds->c_numOfSets)] > (int)max )
		{
			max = cache_ds->LRUCounter[indexLocation + (i*cache_ds->c_numOfSets)];
			*tagLocation = ( indexLocation + (i*cache_ds->c_numOfSets) );
		}
	}


	for( i=0; i<cache_ds->c_assoc; i++)
	{
		cache_ds->LRUCounter[indexLocation + (i*cache_ds->c_numOfSets)] = ((int)cache_ds->LRUCounter[indexLocation + (i*cache_ds->c_numOfSets)]) + 1;
	}

	cache_ds->LRUCounter[*tagLocation] = 0;

}





void LeastFrequentForMiss(struct cache* cache_ds, unsigned int indexLocation, unsigned int* tagLocation)
{
	unsigned int i = 0;
	int min = 16777215;
	*tagLocation = 0;
	//printf("\nL1 UPDATE LFU");


	for( i=0; i<cache_ds->c_assoc; i++)
	{
		if( (int)cache_ds->LRUCounter[indexLocation + (i*cache_ds->c_numOfSets)] < (int)min )
		{
			min = cache_ds->LRUCounter[indexLocation + (i*cache_ds->c_numOfSets)];
			*tagLocation = ( indexLocation + (i*cache_ds->c_numOfSets) );
		}
	}

	//*tagLocation gives the location of block which is selected to be evicted
	cache_ds->count_set[indexLocation] = cache_ds->LRUCounter[*tagLocation];
}
