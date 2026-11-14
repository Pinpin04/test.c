#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_N 20000
#define TIME_RUNS 20
#define DATA_SEED 12345u
#define PIVOT_SEED 2463534242u

#define ALGO_HEAP 0
#define ALGO_MERGE 1
#define ALGO_QUICK_FIRST 2
#define ALGO_QUICK_RANDOM 3
#define ALGO_QUICK_MEDIAN3 4
#define ALGO_COUNT 5

#define FRAME_MERGE (2 * sizeof(int *) + 3 * sizeof(int))
#define FRAME_QUICK (sizeof(int *) + 3 * sizeof(int))
/* heap_sort: a, n, i, end  /  sift_down: a, root, size, child, tmp */
#define FRAME_HEAP_SORT (sizeof(int *) + 3 * sizeof(int))
#define FRAME_SIFT_DOWN (sizeof(int *) + 4 * sizeof(int))

typedef struct {
    long long comparisons;
    double time_ms;
    size_t memory_bytes;
    int ok;
} Result;

static long long cmp_count;
static int cur_depth;
static int max_depth;
static unsigned int pivot_state;

static unsigned int next_pivot_rand(void)
{
    pivot_state ^= pivot_state << 13;
    pivot_state ^= pivot_state >> 17;
    pivot_state ^= pivot_state << 5;
    return pivot_state;
}

static void swap_int(int *x, int *y)
{
    int t = *x;
    *x = *y;
    *y = t;
}


/* 최대 힙의 root 위치에서 아래로 내려가며 힙 속성을 복구 (반복문, 재귀 없음) */
static void sift_down(int a[], int root, int size)
{
    int child;
    int tmp;

    while ((child = 2 * root + 1) < size) {
        if (child + 1 < size) {
            cmp_count++;
            if (a[child] < a[child + 1]) {
                child++;
            }
        }
        cmp_count++;
        if (a[root] < a[child]) {
            tmp = a[root];
            a[root] = a[child];
            a[child] = tmp;
            root = child;
        } else {
            break;
        }
    }
}

/* 입력 배열 자체를 최대 힙으로 사용하는 제자리(in-place) 힙 정렬 */
static void heap_sort(int a[], int n)
{
    int i;
    int end;

    /* 1단계: 힙 구성 (마지막 내부 노드부터 루트까지 sift_down) */
    for (i = n / 2 - 1; i >= 0; i--) {
        sift_down(a, i, n);
    }
    /* 2단계: 루트(최댓값)와 마지막 원소 교환 후 힙 크기 감소, 힙 속성 복구 */
    for (end = n - 1; end > 0; end--) {
        swap_int(&a[0], &a[end]);
        sift_down(a, 0, end);
    }
}

static void merge_range(int a[], int tmp[], int lo, int mid, int hi)
{
    int i = lo;
    int j = mid + 1;
    int k = lo;

    while (i <= mid && j <= hi) {
        cmp_count++;
        if (a[i] <= a[j]) {
            tmp[k++] = a[i++];
        } else {
            tmp[k++] = a[j++];
        }
    }
    while (i <= mid) {
        tmp[k++] = a[i++];
    }
    while (j <= hi) {
        tmp[k++] = a[j++];
    }
    for (k = lo; k <= hi; k++) {
        a[k] = tmp[k];
    }
}

static void merge_sort_rec(int a[], int tmp[], int lo, int hi)
{
    int mid;

    if (lo >= hi) {
        return;
    }
    cur_depth++;
    if (cur_depth > max_depth) {
        max_depth = cur_depth;
    }
    mid = lo + (hi - lo) / 2;
    merge_sort_rec(a, tmp, lo, mid);
    merge_sort_rec(a, tmp, mid + 1, hi);
    merge_range(a, tmp, lo, mid, hi);
    cur_depth--;
}

static int median3_index(const int a[], int lo, int hi)
{
    int mid = lo + (hi - lo) / 2;

    cmp_count++;
    if (a[lo] < a[mid]) {
        cmp_count++;
        if (a[mid] < a[hi]) {
            return mid;
        }
        cmp_count++;
        return (a[lo] < a[hi]) ? hi : lo;
    } else {
        cmp_count++;
        if (a[lo] < a[hi]) {
            return lo;
        }
        cmp_count++;
        return (a[mid] < a[hi]) ? hi : mid;
    }
}

static int choose_pivot_index(const int a[], int lo, int hi, int algo)
{
    if (algo == ALGO_QUICK_FIRST) {
        return lo;
    }
    if (algo == ALGO_QUICK_RANDOM) {
        return lo + (int)(next_pivot_rand() % (unsigned int)(hi - lo + 1));
    }
    return median3_index(a, lo, hi);
}

static int partition_range(int a[], int lo, int hi, int algo)
{
    int p = choose_pivot_index(a, lo, hi, algo);
    int pivot;
    int i = lo - 1;
    int j;

    swap_int(&a[p], &a[hi]);
    pivot = a[hi];
    for (j = lo; j < hi; j++) {
        cmp_count++;
        if (a[j] < pivot) {
            i++;
            swap_int(&a[i], &a[j]);
        }
    }
    swap_int(&a[i + 1], &a[hi]);
    return i + 1;
}

static void quick_sort_rec(int a[], int lo, int hi, int algo)
{
    int p;

    if (lo >= hi) {
        return;
    }
    cur_depth++;
    if (cur_depth > max_depth) {
        max_depth = cur_depth;
    }
    p = partition_range(a, lo, hi, algo);
    quick_sort_rec(a, lo, p - 1, algo);
    quick_sort_rec(a, p + 1, hi, algo);
    cur_depth--;
}

static long long checksum(const int a[], int n)
{
    long long s = 0;
    int i;

    for (i = 0; i < n; i++) {
        s += a[i];
    }
    return s;
}

static int is_sorted_asc(const int a[], int n)
{
    int i;

    for (i = 1; i < n; i++) {
        if (a[i - 1] > a[i]) {
            return 0;
        }
    }
    return 1;
}

static void run_case(int algo, const int base[], int n, Result *r)
{
    int *work = malloc(sizeof(int) * (size_t)n);
    int *tmp = NULL;
    double total_ms = 0.0;
    long long base_sum = checksum(base, n);
    int run;

    if (algo == ALGO_MERGE) {
        tmp = malloc(sizeof(int) * (size_t)n);
    }

    for (run = 0; run < TIME_RUNS; run++) {
        clock_t s;
        clock_t e;

        memcpy(work, base, sizeof(int) * (size_t)n);
        pivot_state = PIVOT_SEED;
        cmp_count = 0;
        cur_depth = 0;
        max_depth = 0;

        s = clock();
        if (algo == ALGO_HEAP) {
            heap_sort(work, n);
        } else if (algo == ALGO_MERGE) {
            merge_sort_rec(work, tmp, 0, n - 1);
        } else {
            quick_sort_rec(work, 0, n - 1, algo);
        }
        e = clock();
        total_ms += (double)(e - s) * 1000.0 / CLOCKS_PER_SEC;

        if (run == 0) {
            r->comparisons = cmp_count;
            r->ok = is_sorted_asc(work, n) && checksum(work, n) == base_sum;
            if (algo == ALGO_HEAP) {
                /* 재귀 없음: heap_sort 프레임 1개 + sift_down 프레임 1개 (고정) */
                r->memory_bytes = FRAME_HEAP_SORT + FRAME_SIFT_DOWN;
            } else if (algo == ALGO_MERGE) {
                r->memory_bytes = sizeof(int) * (size_t)n + (size_t)max_depth * FRAME_MERGE;
            } else {
                r->memory_bytes = (size_t)max_depth * FRAME_QUICK;
            }
        }
    }

    r->time_ms = total_ms / TIME_RUNS;
    free(work);
    free(tmp);
}

int main(void)
{
    int sizes[] = {1000, 2000, 4000, 8000, 16000, 20000};
    int num_sizes = (int)(sizeof(sizes) / sizeof(sizes[0]));
    static int base[MAX_N];
    const char *names[ALGO_COUNT] = {"Heap Sort", "Merge Sort", "Quick (First)", "Quick (Random)", "Quick (Median3)"};
    int si;
    int algo;
    int all_ok = 1;

    for (si = 0; si < num_sizes; si++) {
        int n = sizes[si];
        int i;
        Result res[ALGO_COUNT];

        srand(DATA_SEED);
        for (i = 0; i < n; i++) {
            base[i] = rand() % 100000;
        }

        for (algo = 0; algo < ALGO_COUNT; algo++) {
            run_case(algo, base, n, &res[algo]);
            if (!res[algo].ok) {
                all_ok = 0;
            }
        }

        printf("Data Size : %d\n\n", n);
        printf("%-15s %-14s %-12s %s\n", "Algorithm", "Comparisons", "Time(ms)", "Memory(Bytes)");
        printf("--------------------------------------------------------\n");
        for (algo = 0; algo < ALGO_COUNT; algo++) {
            printf("%-15s %-14lld %-12.4f %zu\n", names[algo], res[algo].comparisons,
                   res[algo].time_ms, res[algo].memory_bytes);
        }
        printf("Sorted check : ");
        for (algo = 0; algo < ALGO_COUNT; algo++) {
            printf("%s%s", algo == 0 ? "" : ", ", res[algo].ok ? "OK" : "FAIL");
        }
        printf("\n\n");
    }

    printf("Overall verification : %s\n", all_ok ? "ALL OK" : "FAILED");
    return all_ok ? 0 : 1;
}
