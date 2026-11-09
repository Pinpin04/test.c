#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define main quick_sort_pivot_main
#include "quick_sort_pivot.c"
#undef main

static int failures = 0;

static void check(int cond, const char *name)
{
    if (cond) {
        printf("[PASS] %s\n", name);
    } else {
        printf("[FAIL] %s\n", name);
        failures++;
    }
}

static int cmp_int(const void *x, const void *y)
{
    int a = *(const int *)x;
    int b = *(const int *)y;
    return (a > b) - (a < b);
}

static void test_case(const char *case_name, const int *origin, int n)
{
    int *ref = (int *)malloc(sizeof(int) * n);
    int *a = (int *)malloc(sizeof(int) * n);
    int methods[3] = {PIVOT_FIRST, PIVOT_RANDOM, PIVOT_MEDIAN};
    const char *mnames[3] = {"First", "Random", "Median-3"};
    int m;
    char msg[256];

    memcpy(ref, origin, sizeof(int) * n);
    qsort(ref, n, sizeof(int), cmp_int);

    for (m = 0; m < 3; m++) {
        memcpy(a, origin, sizeof(int) * n);
        quick_recursive(a, 0, n - 1, methods[m]);
        sprintf(msg, "%s / recursive / %s", case_name, mnames[m]);
        check(memcmp(a, ref, sizeof(int) * n) == 0, msg);

        memcpy(a, origin, sizeof(int) * n);
        quick_iterative(a, n, methods[m]);
        sprintf(msg, "%s / iterative / %s", case_name, mnames[m]);
        check(memcmp(a, ref, sizeof(int) * n) == 0, msg);
    }

    free(ref);
    free(a);
}

int main(void)
{
    int one[] = {42};
    int two[] = {2, 1};
    int dup[] = {5, 5, 5, 5, 5, 5};
    static int sorted[1000];
    static int rev[1000];
    static int rnd[5000];
    int i;

    srand(12345);

    for (i = 0; i < 1000; i++) {
        sorted[i] = i;
        rev[i] = 1000 - i;
    }
    for (i = 0; i < 5000; i++) {
        rnd[i] = rand() % 1000;
    }

    test_case("one element", one, 1);
    test_case("two elements", two, 2);
    test_case("all duplicates", dup, 6);
    test_case("sorted 1000", sorted, 1000);
    test_case("reverse 1000", rev, 1000);
    test_case("random 5000 with duplicates", rnd, 5000);

    {
        int trio[3] = {10, 50, 30};
        int p;
        cmp_count = 0;
        p = choose_pivot_index(trio, 0, 2, PIVOT_MEDIAN);
        check(trio[p] == 30, "median-of-three selects 30 from {10, 50, 30}");
        check(cmp_count == 3, "median-of-three uses 3 comparisons on a 3-element range");
    }

    printf("\n%s (%d failure(s))\n",
           failures == 0 ? "ALL TESTS PASSED" : "SOME TESTS FAILED", failures);
    return failures == 0 ? 0 : 1;
}
