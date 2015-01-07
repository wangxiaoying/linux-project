#ifndef CCONSTANTS_INCLUDED
#define CCONSTANTS_INCLUDED

#define MAX_WORD_LENGTH 50 //Floccinaucinihilipilification is the longest english word except scientific terms
#define AVERAGE_WORD_LENGTH 5 // according to statistics

#define HASH_MULTIPLIER 43001173 // should be a prime
#define NUMBER_OF_HASH_MUTEX 293 // should be a prime

#define NODES_MALLOC_DELTA 10000
#define RECORDS_MALLOC_DELTA 10000

#define DYNVECTOR_SINGLE_THREADED_VERSION // if defined, no mutexs will be used i.e. the class not multi-thread safe
#define DYNVECTOR_MEMSET_WITH_SSE

#define TOLERANCE_DISTANCE_CATENATION 3
#define TOLERANCE_DISTANCE_EXCERPT 4
#define TOLERANCE_RELEVANT_DISTANCE 30
#define TOLERANCE_VERY_RELEVANT_DISTANCE 10

#define BOOST_DIRECTORY 50
#define BOOST_APP 500

#endif // CCONSTANTS_INCLUDED
