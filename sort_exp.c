#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 10000
#define TRIALS 100
#define MAXKEY 1000000

typedef struct { int k1, k2, ord; } Data;

static long long cmpCnt, movCnt;

#define MOVE(dst, src) do { (dst) = (src); movCnt++; } while (0)
#define SWAP(x, y) do { Data t_; MOVE(t_, (x)); MOVE((x), (y)); MOVE((y), t_); } while (0)

void selection_sort(Data *a, int n) {
    for (int i = 0; i < n - 1; i++) {
        int m = i;
        for (int j = i + 1; j < n; j++) {
            cmpCnt++;
            if (a[j].k1 < a[m].k1) m = j;
        }
        if (m != i) SWAP(a[i], a[m]);
    }
}

void bubble_sort(Data *a, int n, int early) {
    for (int p = 0; p < n - 1; p++) {
        int swapped = 0;
        for (int j = 0; j < n - 1 - p; j++) {
            cmpCnt++;
            if (a[j].k1 > a[j + 1].k1) { SWAP(a[j], a[j + 1]); swapped = 1; }
        }
        if (early && !swapped) break;
    }
}

void insertion_sort(Data *a, int n, int eq) {
    for (int i = 1; i < n; i++) {
        Data cur;
        MOVE(cur, a[i]);
        int j = i - 1;
        while (j >= 0) {
            cmpCnt++;
            int shift = eq ? (a[j].k1 >= cur.k1) : (a[j].k1 > cur.k1);
            if (!shift) break;
            MOVE(a[j + 1], a[j]);
            j--;
        }
        MOVE(a[j + 1], cur);
    }
}

static const char *NAMES[] = {"Selection Sort", "Bubble Sort", "Bubble Sort (Early Termination)",
                              "Insertion Sort", "Insertion Sort (>=)"};

void run(int alg, Data *a, int n) {
    switch (alg) {
        case 0: selection_sort(a, n); break;
        case 1: bubble_sort(a, n, 0); break;
        case 2: bubble_sort(a, n, 1); break;
        case 3: insertion_sort(a, n, 0); break;
        case 4: insertion_sort(a, n, 1); break;
    }
}

int is_sorted(Data *a, int n) {
    for (int i = 1; i < n; i++) if (a[i - 1].k1 > a[i].k1) return 0;
    return 1;
}

int is_stable(Data *a, int n, int *viol) {
    int v = 0;
    for (int i = 1; i < n; i++)
        if (a[i - 1].k1 == a[i].k1 && a[i - 1].ord > a[i].ord) v++;
    if (viol) *viol = v;
    return v == 0;
}

int rnd(int hi) {
    return (int)(((long long)rand() * 32768 + rand()) % ((long long)hi + 1));
}
void gen(Data *a, int n, int hi) {
    for (int i = 0; i < n; i++) { a[i].k1 = rnd(hi); a[i].k2 = rnd(MAXKEY); a[i].ord = i; }
}
int cmp_asc(const void *x, const void *y) {
    const Data *p = x, *q = y;
    if (p->k1 != q->k1) return p->k1 < q->k1 ? -1 : 1;
    return p->ord < q->ord ? -1 : (p->ord > q->ord);
}
int cmp_desc(const void *x, const void *y) { return cmp_asc(y, x); }

void print_alg_result(int alg, Data *base, int n) {
    static Data w[N];
    memcpy(w, base, sizeof(Data) * n);
    cmpCnt = movCnt = 0;
    run(alg, w, n);
    printf("%s\nComparisons : %lld\nMovements   : %lld\nSorted OK   : %s\n\n",
           NAMES[alg], cmpCnt, movCnt, is_sorted(w, n) ? "YES" : "NO");
}

int main(void) {
    static Data base[N], work[N], tmp[N];
    srand(20240601);
    printf("Data Count : %d\n\nRandom Data\nNumber of Trials : %d\n\n", N, TRIALS);

    long long tc[5] = {0}, tm[5] = {0};
    int allOk = 1;
    for (int t = 0; t < TRIALS; t++) {
        gen(base, N, MAXKEY);
        for (int alg = 0; alg < 5; alg++) {
            memcpy(work, base, sizeof(base));
            cmpCnt = movCnt = 0;
            run(alg, work, N);
            tc[alg] += cmpCnt; tm[alg] += movCnt;
            if (!is_sorted(work, N)) allOk = 0;
        }
    }
    int order[5] = {0, 1, 2, 3, 4};
    for (int k = 0; k < 5; k++) {
        int a = order[k];
        printf("%s\nTotal Comparisons   : %lld\nAverage Comparisons : %.2f\nAverage Movements   : %.2f\n\n",
               NAMES[a], tc[a], (double)tc[a] / TRIALS, (double)tm[a] / TRIALS);
    }
    printf("All random results sorted correctly : %s\n", allOk ? "YES" : "NO");
    printf("Bubble - Early Termination avg comparison difference : %.2f\n",
           (double)(tc[1] - tc[2]) / TRIALS);

    gen(base, N, MAXKEY);
    Data sorted[N], rev[N], part[N];
    memcpy(sorted, base, sizeof(base)); qsort(sorted, N, sizeof(Data), cmp_asc);
    memcpy(rev, base, sizeof(base));    qsort(rev, N, sizeof(Data), cmp_desc);

    memcpy(part, sorted, sizeof(sorted));
    for (int s = 0; s < 500; s++) {
        int i = rnd(N - 1), j = rnd(N - 1);
        Data t = part[i]; part[i] = part[j]; part[j] = t;
    }
    Data *sets[3] = {sorted, rev, part};
    const char *sn[3] = {"Sorted Data", "Reverse-Sorted Data", "Partially Sorted Data (500 random swaps)"};
    for (int s = 0; s < 3; s++) {
        printf("\n------------------------------------\n\n%s\n\n", sn[s]);
        for (int alg = 0; alg < 5; alg++) print_alg_result(alg, sets[s], N);
    }

    printf("------------------------------------\n\nStability Verification\n\n");

    Data ex[4] = {{527, 100, 0}, {527, 200, 1}, {125, 300, 2}, {892, 400, 3}};

    gen(tmp, N, 99);
    for (int alg = 0; alg < 5; alg++) {
        if (alg == 2) continue;
        Data e[4]; memcpy(e, ex, sizeof(ex));
        cmpCnt = movCnt = 0; run(alg, e, 4);
        memcpy(work, tmp, sizeof(tmp));
        cmpCnt = movCnt = 0; run(alg, work, N);
        int viol; int st = is_stable(work, N, &viol);
        printf("%-20s : %s (example: ", NAMES[alg], st ? "STABLE" : "NOT STABLE");
        for (int i = 0; i < 4; i++) printf("(%d,%d)", e[i].k1, e[i].k2);
        printf(", dup-test order violations = %d, sorted=%s)\n", viol, is_sorted(work, N) ? "YES" : "NO");
    }

    memcpy(work, tmp, sizeof(tmp)); run(2, work, N);
    int viol; int st = is_stable(work, N, &viol);
    printf("%-20s : %s (violations = %d)\n", "Bubble (Early)", st ? "STABLE" : "NOT STABLE", viol);
    return 0;
}
