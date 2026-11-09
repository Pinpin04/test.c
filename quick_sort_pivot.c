#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PIVOT_FIRST  0
#define PIVOT_RANDOM 1
#define PIVOT_MEDIAN 2

static long long cmp_count = 0;

static void swap_int(int *a, int *b)
{
    int t = *a;
    *a = *b;
    *b = t;
}

static int choose_pivot_index(int *a, int lo, int hi, int method)
{
    int x, y, z;

    if (method == PIVOT_FIRST) {
        return lo;
    }
    if (method == PIVOT_RANDOM) {
        return lo + rand() % (hi - lo + 1);
    }

    x = lo;
    y = lo + (hi - lo) / 2;
    z = hi;

    cmp_count++;
    if (a[x] > a[y]) swap_int(&a[x], &a[y]);
    cmp_count++;
    if (a[y] > a[z]) swap_int(&a[y], &a[z]);
    cmp_count++;
    if (a[x] > a[y]) swap_int(&a[x], &a[y]);

    return y;
}

static int partition(int *a, int lo, int hi, int method)
{
    int p, i, j, pivot;

    p = choose_pivot_index(a, lo, hi, method);
    swap_int(&a[p], &a[hi]);
    pivot = a[hi];

    i = lo;
    for (j = lo; j < hi; j++) {
        cmp_count++;
        if (a[j] < pivot) {
            swap_int(&a[i], &a[j]);
            i++;
        }
    }
    swap_int(&a[i], &a[hi]);
    return i;
}

static void quick_recursive(int *a, int lo, int hi, int method)
{
    if (lo < hi) {
        int p = partition(a, lo, hi, method);
        quick_recursive(a, lo, p - 1, method);
        quick_recursive(a, p + 1, hi, method);
    }
}

static void quick_iterative(int *a, int n, int method)
{
    int *stack;
    int top = 0;

    if (n <= 1) return;

    stack = (int *)malloc(sizeof(int) * (4 * n + 8));
    if (stack == NULL) {
        printf("memory allocation failed\n");
        exit(1);
    }

    stack[top++] = 0;
    stack[top++] = n - 1;

    while (top > 0) {
        int hi = stack[--top];
        int lo = stack[--top];

        if (lo < hi) {
            int p = partition(a, lo, hi, method);
            stack[top++] = lo;
            stack[top++] = p - 1;
            stack[top++] = p + 1;
            stack[top++] = hi;
        }
    }

    free(stack);
}

static int is_sorted(const int *a, int n)
{
    int i;
    for (i = 1; i < n; i++) {
        if (a[i - 1] > a[i]) return 0;
    }
    return 1;
}

static void generate_data(int *a, int n)
{
    int i;
    for (i = 0; i < n; i++) {
        a[i] = rand() % 100000;
    }
}

static long long run_comparison(const int *origin, int n, int method, int recursive)
{
    int *copy = (int *)malloc(sizeof(int) * n);
    if (copy == NULL) {
        printf("memory allocation failed\n");
        exit(1);
    }

    memcpy(copy, origin, sizeof(int) * n);
    cmp_count = 0;

    if (recursive) {
        quick_recursive(copy, 0, n - 1, method);
    } else {
        quick_iterative(copy, n, method);
    }

    if (!is_sorted(copy, n)) {
        printf("ERROR: result is not sorted (n=%d, method=%d)\n", n, method);
        exit(1);
    }

    free(copy);
    return cmp_count;
}

static double measure_time_ms(const int *origin, int n, int recursive, int repeat)
{
    int *copy = (int *)malloc(sizeof(int) * n);
    double total = 0.0;
    int r;

    if (copy == NULL) {
        printf("memory allocation failed\n");
        exit(1);
    }

    for (r = 0; r < repeat; r++) {
        clock_t start, end;

        memcpy(copy, origin, sizeof(int) * n);

        start = clock();
        if (recursive) {
            quick_recursive(copy, 0, n - 1, PIVOT_MEDIAN);
        } else {
            quick_iterative(copy, n, PIVOT_MEDIAN);
        }
        end = clock();

        if (!is_sorted(copy, n)) {
            printf("ERROR: result is not sorted in timing test\n");
            exit(1);
        }

        total += (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
    }

    free(copy);
    return total / repeat;
}

int main(void)
{
    int sizes[] = {1000, 2000, 4000, 8000, 16000, 20000};
    int count = sizeof(sizes) / sizeof(sizes[0]);
    int i;

    srand((unsigned int)time(NULL));

    printf("[Pivot Selection Comparison]\n\n");

    for (i = 0; i < count; i++) {
        int n = sizes[i];
        int *origin = (int *)malloc(sizeof(int) * n);
        long long c_first, c_random, c_median;

        if (origin == NULL) {
            printf("memory allocation failed\n");
            return 1;
        }

        generate_data(origin, n);

        c_first  = run_comparison(origin, n, PIVOT_FIRST, 1);
        c_random = run_comparison(origin, n, PIVOT_RANDOM, 1);
        c_median = run_comparison(origin, n, PIVOT_MEDIAN, 1);

        printf("Data Size : %d\n\n", n);
        printf("Quick Sort (First)    : %lld comparisons\n", c_first);
        printf("Quick Sort (Random)   : %lld comparisons\n", c_random);
        printf("Quick Sort (Median-3) : %lld comparisons\n\n", c_median);

        free(origin);
    }

    printf("[Bonus: Recursive vs Iterative]\n\n");
    {
        int n = 10000;
        int repeat = 10;
        int *origin = (int *)malloc(sizeof(int) * n);
        double t_rec, t_iter;

        if (origin == NULL) {
            printf("memory allocation failed\n");
            return 1;
        }

        generate_data(origin, n);

        t_rec  = measure_time_ms(origin, n, 1, repeat);
        t_iter = measure_time_ms(origin, n, 0, repeat);

        printf("Data Size : %d\n", n);
        printf("Pivot     : Median-of-Three\n\n");
        printf("Recursive Quick Sort : %.3f ms\n", t_rec);
        printf("Iterative Quick Sort : %.3f ms\n", t_iter);

        free(origin);
    }

    return 0;
}
