/**
 * Reads file names from stdin and distributes
 * them evenly to map workers.
 * @param m         The number of map workers.
 * @param in_pipes  An array of mapper->master pipes.
 * @param out_pipes An array of master->mapper pipes.
 */
void process_files(char *path, int m, int *in_pipes, int *out_pipes){
    int current_worker = 0;

    char file_name[MAX_FILENAME];
    int path_length = strlen(path);

    // Just print for now
    while(scanf("%s", file_name) != EOF){
        // Send to worker

        write(
            out_pipes[current_worker],
            path,
            path_length
        );

        if(path[path_length - 1] != '/'){
            write(out_pipes[current_worker], "/", 1);
        }

        write(
            out_pipes[current_worker],
            file_name,
            strlen(file_name)
        );
        write(out_pipes[current_worker], "\0", 1);

        // Distribute uniformly
        current_worker++;
        if(current_worker + 1 > m){
            current_worker = 0;
        }
    }
    
    Pair read_pair;
    
    fd_set in_pipes_set;
    FD_ZERO(&in_pipes_set);

    int closed_pipes = 0;
    int max_pipe = 0;

    for(int i = 0; i < m; i++){
        close(out_pipes[i]);

        // listen to in pipes
        FD_SET(in_pipes[i], &in_pipes_set);
        if(max_pipe < in_pipes[i]){
            max_pipe = in_pipes[i];
        }
    }

    while(closed_pipes < m){
        select(max_pipe + 1, &in_pipes_set, NULL, NULL, NULL);
        // Process ready pipes
        for(int i = 0; i < m; i++){
            if(FD_ISSET(in_pipes[i], &in_pipes_set)){
                // Read from pipe
                
                int read_result = read(in_pipes[i], &read_pair, sizeof(Pair));
                if(read_result == 0){
                    closed_pipes++;
                }else if(read_result < 0){
                    printf("Master: Error reading map, %d", errno);
                }else{
                    // Process <key, value> (i.e. send to reduce worker).
                    printf("Processing <%s, %s>\n", read_pair.key, read_pair.value);
                    // printf("Processing ayy");
                }
            }
        }
    }
}

/**
 * error handling function
 */


/**
 * Creates m map workers ready for use.
 * @param  m [description]
 * @return   [description]
 */
void create_map_workers(char *path, int m){
    int in_pipes[m];
    int out_pipes[m];

    // Fork indicator
    int f = -1;

    // Fork n times
    // Establish pipes with each child
    // Return open pipes
    for(int i = 0; i < m; i++){
        // Create the mapper->master pipe
        int mapper_master_pipe[2];
        if(pipe(mapper_master_pipe)){
            fprintf(stderr, "Map Worker: mapper_paster pipe failed.\n");
            exit(20);
        }

        // Create the master->mapper pipe
        int master_mapper_pipe[2];
        if(pipe(master_mapper_pipe)){
            fprintf(stderr, "Map Worker: master_mapper pipe failed.\n");
            exit(21);
        }

        // Fork into map worker
        f = fork();

        if(f == 0){
            // Child (map worker)
            // Route STDOUT to pipe mapper->master
            close(mapper_master_pipe[0]);
            dup2(mapper_master_pipe[1], STDOUT_FILENO);

            // Route STDIN from pipe master->mapper
            close(master_mapper_pipe[1]);
            dup2(master_mapper_pipe[0], STDIN_FILENO);

            // We don't want to spawn children from child
            break;
        }else if(f > 0){
            // Master
            // Store mapper->master pipe
            close(mapper_master_pipe[1]);
            in_pipes[i] = mapper_master_pipe[0];

            // Store master->mapper pipe
            close(master_mapper_pipe[0]);
            out_pipes[i] = master_mapper_pipe[1];
        }else{
            // ERROR
            fprintf(stderr, "Map Worker: fork failed.\n");
            exit(22);
        }
    }

    if(f == 0){
        // Break from child
        // do map stuff
        map_digest_files();
    }else{
        // Finished creating map workers
        // Read stdin for filenames, send to map workers.
        process_files(path, m, in_pipes, out_pipes);
    }
}

/**
 * Creates a walker worker and routes its
 * stdout to this process' stdin.
 */
int create_master(char *path, int m){
    // Create the walker->master pipe
    int walker_pipe[2];
    if(pipe(walker_pipe)){
        fprintf(stderr, "Walker Worker: pipe failed.\n");
        exit(10);
    }

    // Fork into walker worker
    int f = fork();
    if(f == 0){
        // Child (walker worker)
        close(walker_pipe[0]);
        dup2(walker_pipe[1], STDOUT_FILENO);
        walk_directory(path);
    }else if(f > 0){
        // Master, route child stdout to stdin
        close(walker_pipe[1]);
        dup2(walker_pipe[0], STDIN_FILENO);

        // Start the mapping process.
        create_map_workers(path, m);
    }else{
        // ERROR
        fprintf(stderr, "Walker Worker: fork failed.\n");
        exit(11);
    }

    return 0;
}