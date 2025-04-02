#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define SPECIAL_EXIT 100
#define SIZE 10

int input(long long *array, size_t *n);

void selection_sort(long long *array, size_t n);

void output(long long *array, size_t n);

void swap(long long *a, long long *b);

int main(void)
{
    int exit_code = EXIT_SUCCESS;
    long long array[SIZE];
    size_t n = 0;
    if (input(array, &n) == SPECIAL_EXIT)
    {
        exit_code = SPECIAL_EXIT;
    }
    if (n == 0)
    {
        printf("%d Error: array is empty\n", getpid());
        exit_code = EXIT_FAILURE;
    }
    else
    {
        selection_sort(array, n);
        output(array, n);
    }
    return exit_code;
}

void output(long long *array, size_t n)
{
    printf("%d Sorted array: ", getpid());
    for (size_t i = 0; i < n; ++i)
    {
        printf("%lld ", *(array + i));
    }
    printf("\n");
}

void selection_sort(long long *array, size_t n)
{
    for (size_t i = 1; i < n; ++i)
    {
        for (size_t j = i; j > 0 && array[j] < array[j - 1]; --j)
        {
            swap(&array[j], &array[j-1]);
        }
    }
}

void swap(long long *a, long long *b)
{
    long long tmp = *a;
    *a = *b;
    *b = tmp;
}

int input(long long *array, size_t *n)
{
    int return_code = EXIT_SUCCESS;
    printf("%d Enter next_number: ", getpid());
    while (*n < SIZE && scanf("%lld", &array[*n]) == 1)
    {
        (*n)++;
        printf("%d Enter next_number: ", getpid());
    }
    long long tmp;
    if (*n >= SIZE && scanf("%lld", &tmp) == 1)
    {
        return_code = SPECIAL_EXIT;
    }
    return return_code;
}
