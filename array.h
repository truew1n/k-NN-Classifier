#ifndef ARRAY_INCLUDE_ONCE
    #define ARRAY_INCLUDE_ONCE

    #include <stdio.h>
    #include <stdint.h>
    #include <stdlib.h>
    #include <string.h>

    #ifndef TYPE
        #define TYPE int
    #endif

    #define TOKENPASTE(x, y)                    x ## y

    #define ARRAY_NAME(T)                       TOKENPASTE(T, _array_t)

    #define ARRAY_INIT(T)                       TOKENPASTE(array_init_, T)
    #define ARRAY_INIT_SIZE(T)                  TOKENPASTE(array_init_size_, T)
    #define ARRAY_FREE(T)                       TOKENPASTE(array_free_, T)
    #define ARRAY_DETACH(T)                     TOKENPASTE(array_detach_, T)
    #define ARRAY_ADD(T)                        TOKENPASTE(array_add_, T)
    #define ARRAY_GET(T)                        TOKENPASTE(array_get_, T)
    #define ARRAY_PRINT(T)                      TOKENPASTE(array_print_, T)
    #define ARRAY_PRINTLN(T)                    TOKENPASTE(array_println_, T)

    #define ARRAY_SORT(T)                       TOKENPASTE(array_sort_, T)
    #define ARRAY_SWAP(T)                       TOKENPASTE(array_swap_, T)
    #define ARRAY_HOARE_PARTITION(T)            TOKENPASTE(array_hoare_partition_, T)
    #define ARRAY_QUICK_SORT(T)                 TOKENPASTE(array_quick_sort_, T)

    #define ARRAY_COMPARATOR(T)                 TOKENPASTE(T, _comparator_t)
    #define ARRAY_PRINTER(T)                    TOKENPASTE(T, _printer_t)
    #define ARRAY_EXCEPTION(T)                  TOKENPASTE(T, _exception_t)


    #define EMPTY_EXCEPTION                     -1
    #define EXCEPTION_NO_EXCEPTION              0
    #define EXCEPTION_INDEX_OUT_OF_BOUNDS       1
    #define EXCPETION_NULL_COMPARATOR           2

    #define ARRAY_ITER(array, iname)            for(int32_t iname = 0; iname < array.size; ++iname)
    #define ARRAY_ITER_R(array, iname)          for(int32_t iname = 0; iname < array->size; ++iname)

#endif

typedef struct ARRAY_COMPARATOR(TYPE) {
    int32_t (*compare)(TYPE, TYPE);
    int8_t leg_values[3];
} ARRAY_COMPARATOR(TYPE);

typedef struct ARRAY_PRINTER(TYPE) {
    void (*print)(TYPE);
    void (*println)(TYPE);
} ARRAY_PRINTER(TYPE);

typedef struct ARRAY_EXCEPTION(TYPE) {
    void (*raise)(int64_t);
    int64_t exception_code;
} ARRAY_EXCEPTION(TYPE);

typedef struct ARRAY_NAME(TYPE) {
    int32_t size;
    TYPE *data;
    ARRAY_COMPARATOR(TYPE) comparator;
    ARRAY_PRINTER(TYPE) printer;
    ARRAY_EXCEPTION(TYPE) exception;
} ARRAY_NAME(TYPE);

void ARRAY_INIT (TYPE) (ARRAY_NAME(TYPE) *array)
{
    array->size = 0;
    array->data = NULL;
    array->comparator.compare = NULL;
    array->comparator.leg_values[0] = -1;
    array->comparator.leg_values[1] = 0;
    array->comparator.leg_values[2] = 1;
    array->printer.print = NULL;
    array->printer.println = NULL;
    array->exception.raise = NULL;
    array->exception.exception_code = 0;
}

void ARRAY_INIT_SIZE (TYPE) (ARRAY_NAME(TYPE) *array, int32_t size)
{
    ARRAY_INIT (TYPE) (array);
    array->size = size;
    array->data = (TYPE *) calloc(size, sizeof(TYPE));
}

void ARRAY_FREE (TYPE) (ARRAY_NAME(TYPE) *array)
{
    if(array->data) {
        free(array->data);
    }
    
    ARRAY_INIT(TYPE)(array);
}

ARRAY_NAME(TYPE) ARRAY_DETACH (TYPE) (ARRAY_NAME(TYPE) *array)
{
    ARRAY_NAME(TYPE) result = *array;
    ARRAY_INIT(TYPE)(array);
    return result;
}

void ARRAY_ADD (TYPE) (ARRAY_NAME(TYPE) *array, TYPE item)
{
    array->data = (TYPE *) realloc(array->data, sizeof(TYPE) * (array->size + 1));
    array->data[array->size] = item;
    array->size++;
}

TYPE ARRAY_GET (TYPE) (ARRAY_NAME(TYPE) *array, int32_t index)
{
    if(index < 0 || index >= array->size) {
        if(array->exception.raise) {
            array->exception.exception_code = EXCEPTION_INDEX_OUT_OF_BOUNDS;
            array->exception.raise(array->exception.exception_code);
        }
        return (TYPE){0};
    }
    return array->data[index];
}

void ARRAY_PRINT (TYPE) (ARRAY_NAME(TYPE) *array)
{
    ARRAY_ITER_R(array, i) {
        array->printer.print(ARRAY_GET(TYPE)(array, i));
    }
}

void ARRAY_PRINTLN (TYPE) (ARRAY_NAME(TYPE) *array)
{
    ARRAY_ITER_R(array, i) {
        array->printer.println(ARRAY_GET(TYPE)(array, i));
    }
}

void ARRAY_SWAP (TYPE) (ARRAY_NAME(TYPE) *array, int32_t i, int32_t j)
{
    TYPE buffer = array->data[i];
    array->data[i] = array->data[j];
    array->data[j] = buffer;
}

int32_t ARRAY_HOARE_PARTITION (TYPE) (ARRAY_NAME(TYPE) *array, int32_t low, int32_t high) {
    TYPE pivot = ARRAY_GET(TYPE)(array, low);
    int32_t i = low - 1;
    int32_t j = high + 1;

    while (1) {
        do {
            i++;
        } while (array->comparator.compare(ARRAY_GET(TYPE)(array, i), pivot) == -1);

        do {
            j--;
        } while (array->comparator.compare(ARRAY_GET(TYPE)(array, j), pivot) == 1);

        if (i >= j)
            return j;

        ARRAY_SWAP(TYPE)(array, i, j);
    }
}

void ARRAY_QUICK_SORT (TYPE) (ARRAY_NAME(TYPE) *array, int32_t low, int32_t high)
{
    if (low < high) {
        int32_t pi = ARRAY_HOARE_PARTITION(TYPE)(array, low, high);

        ARRAY_QUICK_SORT(TYPE)(array, low, pi);
        ARRAY_QUICK_SORT(TYPE)(array, pi + 1, high);
    }
}

void ARRAY_SORT (TYPE) (ARRAY_NAME(TYPE) *array)
{
    if(array->comparator.compare) {
        ARRAY_QUICK_SORT(TYPE)(array, 0, array->size - 1);
    } else {
        if(array->exception.raise) {
            array->exception.exception_code = EXCPETION_NULL_COMPARATOR;
            array->exception.raise(array->exception.exception_code);
        }
    }
}