#ifndef RECORDS_H
#define RECORDS_H

#define MAX_RECORDS 100
#define KEY_SIZE 64
#define VALUE_SIZE 256

typedef struct
{
    char key[KEY_SIZE];
    char value[VALUE_SIZE];
} Record;

int record_store_initialize(const char *directory);
int record_store_put(const char *key, const char *value);
int record_store_get(const char *key, char *value, int value_size);
int record_store_delete(const char *key);
int record_store_list(void);

#endif