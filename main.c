#include <stdio.h>
#include "stdlib.h"
#include <sys/stat.h>
#include <assert.h>
#include <stdbool.h>
#include "sys/time.h"
#include "lmdb.h"
#include "string.h"



void write(MDB_txn* txn, MDB_dbi dbi, int key, char* value, int value_size){
    assert(txn != NULL);

    int result;

    MDB_val mdb_key, data;

    mdb_key.mv_data = (void *) &key;
    mdb_key.mv_size = sizeof(key);

    data.mv_data = (void *) &value;
    data.mv_size = sizeof(char) * value_size;

    result = mdb_put(txn, dbi, &mdb_key, &data, 0);
    if (result != 0)
        printf("%s\n", mdb_strerror(result));
    assert(result == 0);
}

int main(int argc, char* argv[]) {
    if (argc != 5) {// ?
        printf("Only two input arguments. The path to set up the database, the number of writes in the loop, and then the number of bytes to write each iteration.");
        return 1;
    }

    const char* dbpath = argv[1];
    const int num_loops = atoi(argv[2]);
    const int numb_bytes_to_write = atoi(argv[3]);

    bool remove_db_after;
    if (strncasecmp(argv[4], "yes", 4) == 0 || strncasecmp(argv[4], "y", 2) == 0) {
        remove_db_after = true;
    } else if (strncasecmp(argv[4], "no", 4) == 0 || strncasecmp(argv[4], "n", 2) == 0) {
        remove_db_after = false;
    } else {
        printf("Last argument must be a yes or no to deleting the database files afterwards");
        return 1;
    }


    struct stat st;
    bool dir_exists = (stat(dbpath, &st) == 0);

    if (!dir_exists && (mkdir(dbpath, S_IRWXU) != 0)) {
        printf("Failed to create env dir %s",
                        dbpath);
        return 1;
    }

    MDB_env* env = NULL;
    MDB_txn* txn = NULL;
    MDB_dbi dbi = 0;
    int result;

    if ((result = mdb_env_create(&env)) != 0) {
        printf("Could not create lmdb environment. %s",
                        mdb_strerror(result));
        return 1;
    }

    if ((result = mdb_env_set_mapsize(env, 5000000)) != 0) {
        printf("Could not set lmdb map size. %s", mdb_strerror(result));
        return 1;
    }
    if ((result = mdb_env_open(env, dbpath,
                               0,
                               S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)) != 0) {
        printf("Could not open lmdb environment at %s. %s",
                        dbpath, mdb_strerror(result));
        return 1;
    }
    if ((result = mdb_txn_begin(env, NULL, 0, &txn)) != 0) {
        printf("Could not start txn on lmdb environment at %s. %s",
                        dbpath, mdb_strerror(result));
        return 1;
    }
    if ((result = mdb_open(txn, NULL, MDB_INTEGERKEY, &dbi)) != 0) {
        printf("Could not open db on lmdb environment at %s. %s",
                        dbpath, mdb_strerror(result));
        return 1;
    }



    if ((result = mdb_txn_commit(txn)) != 0) {
        printf("Could not commit txn on lmdb environment at %s. %s",
                        dbpath, mdb_strerror(result));
        return 1;
    }


    char* dummy_value = malloc(sizeof(char) * numb_bytes_to_write);
    for (int i = 0; i < numb_bytes_to_write - 1; i++){
        dummy_value[i] = 'a';
    }
    dummy_value[numb_bytes_to_write - 1] = '\n';
    const int dummy_key = 0;

    struct timespec total_start_time;
    struct timespec total_end_time;

    timespec_get(&total_start_time, TIME_UTC);
    for (int i = 0; i < num_loops; i++) {

        if ((result = mdb_txn_begin(env, NULL, 0, &txn)) != 0) {
            printf("Could not start txn on lmdb environment at %s. %s",
                   dbpath, mdb_strerror(result));
            return 1;
        }

        write(txn, dbi, dummy_key, dummy_value, numb_bytes_to_write);

        if ((result = mdb_txn_commit(txn)) != 0) {
            printf("Could not commit txn on lmdb environment at %s. %s",
                   dbpath, mdb_strerror(result));
            return 1;
        }
    }

    timespec_get(&total_end_time, TIME_UTC);


    double start_ns = total_start_time.tv_nsec + (total_start_time.tv_sec * 1000000000);
    double end_ns = total_end_time.tv_nsec + (total_end_time.tv_sec * 1000000000);


    double resulting_ns = end_ns - start_ns;

    printf("Average Latency: %fms\n", (resulting_ns / num_loops) * 0.000001);
    printf("Throughput: %f writes/second\n", (num_loops / resulting_ns) * 1000000000);


    if (remove_db_after) {
            char rm_command[8 + (sizeof(char) * strlen(dbpath))];
            sprintf(rm_command, "rm -r %s", dbpath);
            system(rm_command);
    }


    return 0;
}