#ifndef MASTER_H
#define MASTER_H

#define READ_END 0
#define WRITE_END 1

/*
 * This struct holds all array of pipes / fds interfacing with master.
 */
typedef struct pipe_set {
    int m;
    int r;
    int *from_mapper;
    int *to_mapper;
    int *to_reducer;
} PipeSet;

/*
 * Reads filenames located at dirname from stdin (sent by lister)
 * and distributes them evenly to mappers.
 */
void distribute_files(char *dirname);

/*
 * Read key value Pairs from mappers and
 * assigns by keys to reducer using hash function.
 */
void route_mapped_pairs();
/*
 * Creates m map workers ready for use.
 */
void create_map_workers();

/*
 * Creates m mappers and r reducers ready for use.
 */
void create_workers(char *dirname);


/**
 * Creates a master worker that spawns m map children
 * and r reduce children, initiating MapReduce.
 */
int create_master(char *dirname, int m, int r);

#endif

