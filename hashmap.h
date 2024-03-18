
#ifndef HASHMAP_OWN_PARAMS
#define HASHMAP_PRIME_NUMBER 69233
#define HASHMAP_BUCKET_COUNT 420
#endif

typedef struct pair_t {
    char *classname;
    int32_t length;
    int32_t count;
} pair_t;

typedef struct node_t {
    struct node_t *next;
    struct node_t *prev;
    pair_t value;
} node_t;

typedef struct bucket_t {
    node_t *head;
    node_t *tail;
    int32_t size;
} bucket_t;

#define TYPE bucket_t
#include "array.h"
#undef TYPE

typedef struct hashmap_t {
    bucket_t_array_t buckets;
} hashmap_t;

#include <string.h>
#include <stdbool.h>

int32_t hashmap_hash_scramble(int32_t key_part)
{
    key_part *= 0xCC9E2D51;
    key_part = (key_part << 15) | (key_part >> 17);
    key_part *= 0x1B873593;
    return key_part;
}

int32_t hashmap_hash(char *key, int32_t length)
{
    int32_t hash_value = HASHMAP_PRIME_NUMBER;
    int32_t key_part = 0;
    for (size_t i = length >> 2; i; --i) {
        memcpy(&key_part, key, sizeof(int32_t));
        key += sizeof(int32_t);
        hash_value ^= hashmap_hash_scramble(key_part);
        hash_value = (hash_value << 13) | (hash_value >> 19);
        hash_value = hash_value * 5 + 0xE6546B64;
    }
    key_part = 0;
    for (size_t i = length & 3; i; --i) {
        key_part <<= 8;
        key_part |= key[i - 1];
    }
    hash_value ^= hashmap_hash_scramble(key_part);
    hash_value ^= length;
    hash_value ^= hash_value >> 16;
    hash_value *= 0x85EBCA6B;
    hash_value ^= hash_value >> 13;
    hash_value *= 0xC2B2AE35;
    hash_value ^= hash_value >> 16;
    return hash_value;
}

void hashmap_add(hashmap_t *hashmap, char *key, int32_t length)
{
    int32_t index = hashmap_hash(key, length) % HASHMAP_BUCKET_COUNT;
    bucket_t *target = &hashmap->buckets.data[index];

    bool found = false;
    
    node_t *loopnode = target->head;
    while(loopnode) {
        if(!strcmp(loopnode->value.classname, key)) {
            found = true;
            loopnode->value.count++;
            loopnode = target->tail;
        }
        loopnode = loopnode->next;
    }

    if(!found) {
        pair_t pair = (pair_t){NULL, 0, 1};
        pair.classname = (char *) malloc(sizeof(char) * length);
        pair.length = length;
        strcpy(pair.classname, key);
        
        node_t *node = (node_t *) malloc(sizeof(node_t));

        if(!target->size) {
            node->prev = NULL;

            target->head = node;
            target->tail = node;
        } else {
            target->tail->next = node;
            node->prev = target->tail;

            target->tail = node;
        }
        node->next = NULL;
        node->value = pair;
        target->size++;
    }
}

pair_t hashmap_max(hashmap_t *hashmap)
{
    pair_t max = (pair_t){NULL, 0, -1};
    for(int i = 0; i < hashmap->buckets.size; ++i) {
        bucket_t bucket = hashmap->buckets.data[i];
        if(!bucket.size) continue;

        node_t *loopnode = bucket.head;
        while(loopnode) {
            if(loopnode->value.count > max.count) {
                max = loopnode->value;
            }
            loopnode = loopnode->next;
        }
    }
    return max;
}

pair_t hashmap_get(hashmap_t *hashmap, char *key, int32_t length)
{
    int32_t index = hashmap_hash(key, length) % HASHMAP_BUCKET_COUNT;
    bucket_t *target = &hashmap->buckets.data[index];
    if(!target->size) return (pair_t){NULL, 0, -1};

    node_t *node = target->head;
    while(node) {
        if(!strcmp(node->value.classname, key)) {
            return node->value;
        }
        node = node->next;
    }
    return (pair_t){NULL, 0, -1};
}

void hashmap_init(hashmap_t *hashmap)
{
    array_init_size_bucket_t(&hashmap->buckets, HASHMAP_BUCKET_COUNT);
}

void hashmap_free(hashmap_t *hashmap)
{
    ARRAY_ITER(hashmap->buckets, i) {
        bucket_t *bucket = &hashmap->buckets.data[i];
        
        if(bucket->size) {
            
            if(bucket->size > 1) {
                
                node_t *loopnode = bucket->head->next;
                while(loopnode) {
                    free(loopnode->prev->value.classname);
                    free(loopnode->prev);
                    loopnode = loopnode->next;
                } 
            }

            free(bucket->tail->value.classname);
            free(bucket->tail);
        }
    }
    
    array_free_bucket_t(&hashmap->buckets);
}