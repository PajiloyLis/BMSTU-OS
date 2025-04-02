#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define SIZE 10

int input(double *array, size_t *n);

double neg_mean(double *array, size_t n);

int main(void)
{
    int return_code = EXIT_SUCCESS;
    double array[SIZE];
    size_t n;
    if (input(array, &n) == EXIT_SUCCESS)
    {
        double ans = neg_mean(array, n);
        if (ans > 0)
        {
            printf("%d Error: there are no negative elements\n", getpid());
            return_code = EXIT_FAILURE;
        }
        else
        {
            printf("%d mean of negative elements: %.6lf\n", getpid(), ans);
        }
    }
    else
    {
        printf("%d Error: bad input\n", getpid());
        return_code = EXIT_FAILURE;
    }
    return return_code;
}

int input(double *array, size_t *n)
{
    int return_code = EXIT_SUCCESS;
    printf("%d enter size: ", getpid());
    if (scanf("%zu", n) != 1 || *n > SIZE)
    {
        return_code = EXIT_FAILURE;
    }
    else
    {
        for (size_t i = 0; i < *n && return_code == EXIT_SUCCESS; ++i)
        {
            printf("%d enter next elem: ", getpid());
            if (scanf("%lf", array + i) != 1)
            {
                return_code = EXIT_FAILURE;
            }
        }
    }
    return return_code;
}

double neg_mean(double *array, size_t n)
{
    double neg_sum = 0;
    int cnt = 0;
    for (size_t i = 0; i < n; ++i)
    {
        if (array[i] < 0)
        {
            neg_sum += array[i];
            cnt++;
        }
    }
    double answer;
    if (cnt != 0)
    {
        answer = neg_sum / cnt;
    }
    else
    {
        answer = 1.0;
    }
    return answer;
}
