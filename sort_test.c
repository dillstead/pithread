#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <malloc.h>
#include "thread.h"
#include "rand.h"

#define N      100000
#define CUTOFF 1000

struct args
{
    int *nums;
    int start;
    int end;
    int cutoff;
};

static void swap(int *x, int *y)
{
    int tmp = *x;
    
    *x = *y;
    *y = tmp;
}

static int partition(int *nums, int start, int end, int pos)
{
    int pivot = nums[pos];

    swap(&nums[pos], &nums[end]);
    end--;
    while (start <= end)
    {
        if (nums[start] < pivot)
        {
            start++;
        }
        else
        {
            swap(&nums[start], &nums[end]);
            end--;
        }
    }
    swap(&nums[start], &nums[pos]);
    return start;
}

static int quick(void *cl)
{
    struct args *args = cl;
    int end = args->end;
    int pos;

    if (args->start < args->end)
    {
        pos = partition(args->nums, args->start, end, end);
        args->end = pos - 1;
        if (pos - args->start > args->cutoff)
        {
            thread_new(quick, args, sizeof *args);
        }
        else
        {
            quick(args);
        }
        args->start = pos + 1;
        args->end = end;
        if (end - pos > args->cutoff)
        {
            thread_new(quick, args, sizeof *args);
        }
        else
        {
            quick(args);
        }
    }
    return 0;
}

static void sort(int *nums, int start, int end, int cutoff)
{
    struct args args;

    args.nums = nums;
    args.start = start;
    args.end = end;
    args.cutoff = cutoff;
    quick(&args);
    thread_join(NULL);
}

bool sort_test(void)
{
    int *nums;

    nums = malloc(N * sizeof *nums);
    if (!nums)
    {
        return 1;
    }
    for (int i = 0; i < N; i++)
    {
        nums[i] = rand(1, N);
    }
    sort(nums, 0, N - 1, CUTOFF);
    for (int i = 0; i < N - 1; i++)
    {
        if (nums[i] > nums[i + 1])
        {
            printf("sort_test: pos %d %d > %d\n",
                   i, nums[i], nums[i + 1]);
            return false;
        }
    }
    return true;
}
