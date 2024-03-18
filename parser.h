#pragma once

#include <stdio.h>
#include <string.h>

#define TYPE float
#include "array.h"
#undef TYPE

#define TYPE char
#include "array.h"
#undef TYPE

typedef struct record_t {
    float_array_t values;
    char_array_t classname;
} record_t;

#define TYPE record_t
#include "array.h"
#undef TYPE

typedef struct trainset_t {
    record_t_array_t records;
} trainset_t;

void trainset_dealloc(trainset_t *trainset)
{
    ARRAY_ITER(trainset->records, r) {
        record_t record = array_get_record_t(&trainset->records, r);
        array_free_float(&record.values);
        array_free_char(&record.classname);
    }
    array_free_record_t(&trainset->records);
}

float_array_t parse_input_line(char *str)
{
    float_array_t result;
    array_init_float(&result);

    char_array_t string_float;
    array_init_char(&string_float);

    char c = 0;
    do {
        c = *str++;
        switch(c) {
            case '\0':
            case ',': {
                array_add_char(&string_float, '\0');

                // Converting string float to float
                float item = atof(string_float.data);
                
                array_add_float(&result, item);
                array_free_char(&string_float);
                break;
            }
            default: {
                array_add_char(&string_float, c);
                break;
            }
        }
        
    } while(c != '\0');
    return result;
}

trainset_t parse_trainset_file(const char *filepath)
{
    FILE *file = fopen(filepath, "rb");

    trainset_t trainset = {0};
    array_init_record_t(&trainset.records);

    record_t record = {0};

    float_array_t float_array;
    array_init_float(&float_array);

    char_array_t char_array = {0};
    array_init_char(&char_array);

    char c = 0;
    do {
        c = fgetc(file);
        switch(c) {
            case ',': {
                // Adding null terminator to char array to make sure it doesn't go into infinite loop
                array_add_char(&char_array, '\0');

                // Converting string float to float
                float item = atof(char_array.data);
                
                array_add_float(&float_array, item);
                array_free_char(&char_array);
                break;
            }
            // Fallthrough for edge case
            case EOF: {
                if(!char_array.size) return trainset;
            }
            case '\r': {
                size_t cpos = ftell(file);
                if(fgetc(file) != '\n') {
                    fseek(file, cpos, SEEK_SET);
                    break;
                }
            }
            case '\n': {
                // Adding null terminator to convert it to normal array
                array_add_char(&char_array, '\0');

                record.values = array_detach_float(&float_array);
                record.classname = array_detach_char(&char_array);

                array_add_record_t(&trainset.records, record);
                break;
            }
            default: {
                array_add_char(&char_array, c);
                break;
            }
        }
    } while(c != EOF);

    return trainset;
}