#ifndef MASTER_H
#define MASTER_H

void process_files(char *path, int m, int *in_pipes, int *out_pipes,
        int r, int *reduce_pipes);


/**
 * Creates r reduce workers ready for use.
 * @param  m [description]
 * @param  r [description]
 * @return   [description]
 */
create_workers(char *path, int m, int r);

/**
 * Creates m map workers ready for use.
 * @param  m [description]
 * @return   [description]
 */
void create_map_workers(char *path, int m, int r, int *reduce_pipes);

/**
 * Creates a master worker that spawns m map children
 * and r reduce children, initiating a MapReduce operation.
 * @param  path the location of the folder containing the input data files.
 * @param  m    the number of map children.
 * @param  r    the number of reduce children.
 */
int create_master(char *path, int m, int r);

#endif