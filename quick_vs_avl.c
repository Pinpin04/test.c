#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_N 20000
#define TIME_RUNS 20
#define DATA_SEED 12345u     /* 이전 과제와 동일한 데이터 시드 */
#define OPS_SEED 54321u      /* 삽입/삭제 연산용 시드 */
#define VALUE_RANGE 100000

/* 스택 프레임 추정치: 인자 + 지역 변수 (이전 과제와 같은 기준) */
#define FRAME_QUICK (sizeof(int *) + 3 * sizeof(int))   /* a, lo, hi, p */
#define FRAME_AVL   (2 * sizeof(void *))                /* node 포인터, 값(정렬 포함) */

typedef struct Node {
    int key;      /* 데이터 값 */
    int count;    /* 중복 개수 */
    int height;
    struct Node *left;
    struct Node *right;
} Node;

typedef struct {
    long long sort_cmp;
    long long sort_move;
    double time_ms;
    long long comparisons;
    long long movements;
    size_t memory_bytes;
    int final_count;
} Result;

typedef struct {
    int is_insert;
    int value;
} Op;

static long long cmp_count;
static long long move_count;
static int cur_depth;
static int max_depth;
static int node_count;
static int max_node_count;

/* ===================== 퀵 정렬 (Median-of-Three, 과제 13/14와 동일) ===================== */

static void swap_int(int *x, int *y)
{
    int t = *x;
    *x = *y;
    *y = t;
    move_count += 3;
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

static int partition_range(int a[], int lo, int hi)
{
    int p = median3_index(a, lo, hi);
    int pivot;
    int i = lo - 1;
    int j;

    swap_int(&a[p], &a[hi]);
    pivot = a[hi];
    move_count++;
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

static void quick_sort_rec(int a[], int lo, int hi)
{
    int p;

    if (lo >= hi) {
        return;
    }
    cur_depth++;
    if (cur_depth > max_depth) {
        max_depth = cur_depth;
    }
    p = partition_range(a, lo, hi);
    quick_sort_rec(a, lo, p - 1);
    quick_sort_rec(a, p + 1, hi);
    cur_depth--;
}

/* 정렬된 배열에 삽입: 선형 탐색으로 위치를 찾고 뒤 데이터를 한 칸씩 이동 */
static void array_insert(int a[], int *count, int x)
{
    int i = 0;
    int j;

    while (i < *count) {
        cmp_count++;
        if (a[i] > x) {          /* 같은 값 뒤에 삽입 */
            break;
        }
        i++;
    }
    for (j = *count; j > i; j--) {
        a[j] = a[j - 1];
        move_count++;
    }
    a[i] = x;
    move_count++;
    (*count)++;
}

/* 정렬된 배열에서 삭제: 선형 탐색으로 값을 찾고 뒤 데이터를 앞으로 이동 */
static int array_delete(int a[], int *count, int x)
{
    int i = 0;
    int j;

    while (i < *count) {
        cmp_count++;
        if (a[i] >= x) {
            break;
        }
        i++;
    }
    if (i == *count) {
        return 0;
    }
    cmp_count++;
    if (a[i] != x) {             /* 정렬되어 있으므로 더 볼 필요 없음: 값 없음 */
        return 0;
    }
    for (j = i; j < *count - 1; j++) {
        a[j] = a[j + 1];
        move_count++;
    }
    (*count)--;
    return 1;
}

/* ===================== AVL 트리 ===================== */

static int height(const Node *t)
{
    return t ? t->height : 0;
}

static void update_height(Node *t)
{
    int hl = height(t->left);
    int hr = height(t->right);
    t->height = (hl > hr ? hl : hr) + 1;
}

static Node *rotate_right(Node *y)
{
    Node *x = y->left;
    y->left = x->right;
    x->right = y;
    update_height(y);
    update_height(x);
    return x;
}

static Node *rotate_left(Node *x)
{
    Node *y = x->right;
    x->right = y->left;
    y->left = x;
    update_height(x);
    update_height(y);
    return y;
}

/* 균형 인수 검사는 데이터 값 비교가 아니므로 cmp_count에 포함하지 않음 */
static Node *rebalance(Node *t)
{
    int bf;

    update_height(t);
    bf = height(t->left) - height(t->right);
    if (bf > 1) {
        if (height(t->left->left) < height(t->left->right)) {
            t->left = rotate_left(t->left);      /* LR */
        }
        return rotate_right(t);                  /* LL */
    }
    if (bf < -1) {
        if (height(t->right->right) < height(t->right->left)) {
            t->right = rotate_right(t->right);   /* RL */
        }
        return rotate_left(t);                   /* RR */
    }
    return t;
}

static void enter(void)
{
    cur_depth++;
    if (cur_depth > max_depth) {
        max_depth = cur_depth;
    }
}

static Node *avl_insert(Node *t, int x)
{
    if (t == NULL) {
        Node *n = malloc(sizeof(Node));
        n->key = x;
        move_count++;
        n->count = 1;
        n->height = 1;
        n->left = NULL;
        n->right = NULL;
        node_count++;
        if (node_count > max_node_count) {
            max_node_count = node_count;
        }
        return n;
    }
    enter();
    cmp_count++;
    if (x < t->key) {
        t->left = avl_insert(t->left, x);
    } else {
        cmp_count++;
        if (x > t->key) {
            t->right = avl_insert(t->right, x);
        } else {
            t->count++;          /* 중복 값: 개수만 증가, 노드 추가 없음 */
            cur_depth--;
            return t;
        }
    }
    cur_depth--;
    return rebalance(t);
}

/* 오른쪽 서브트리의 최솟값 노드를 떼어냄 (포인터 이동만, 값 비교 없음) */
static Node *avl_detach_min(Node *t, Node **min_out)
{
    if (t->left == NULL) {
        *min_out = t;
        return t->right;
    }
    enter();
    t->left = avl_detach_min(t->left, min_out);
    cur_depth--;
    return rebalance(t);
}

static Node *avl_delete(Node *t, int x)
{
    if (t == NULL) {
        return NULL;             /* 값 없음: 삭제하지 않음 */
    }
    enter();
    cmp_count++;
    if (x < t->key) {
        t->left = avl_delete(t->left, x);
    } else {
        cmp_count++;
        if (x > t->key) {
            t->right = avl_delete(t->right, x);
        } else if (t->count > 1) {
            t->count--;          /* 중복 원소 하나만 제거 */
            cur_depth--;
            return t;
        } else if (t->left == NULL || t->right == NULL) {
            Node *child = t->left ? t->left : t->right;
            free(t);
            node_count--;
            cur_depth--;
            return child;
        } else {
            Node *succ;
            t->right = avl_detach_min(t->right, &succ);
            t->key = succ->key;  /* 후계자 값 복사: 데이터 이동 1회 */
            move_count++;
            t->count = succ->count;
            free(succ);
            node_count--;
        }
    }
    cur_depth--;
    return rebalance(t);
}

static void avl_free(Node *t)
{
    if (t) {
        avl_free(t->left);
        avl_free(t->right);
        free(t);
    }
}

/* 중위 순회 결과를 배열에 펼침 (검증용, 측정 구간 밖) */
static void avl_inorder(const Node *t, int out[], int *k)
{
    int c;

    if (t == NULL) {
        return;
    }
    avl_inorder(t->left, out, k);
    for (c = 0; c < t->count; c++) {
        out[(*k)++] = t->key;
    }
    avl_inorder(t->right, out, k);
}

/* ===================== 실험 ===================== */

static double now_ms(void)
{
    return (double)clock() * 1000.0 / CLOCKS_PER_SEC;
}

static void reset_counters(void)
{
    cmp_count = 0;
    move_count = 0;
    cur_depth = 0;
    max_depth = 0;
    node_count = 0;
    max_node_count = 0;
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

int main(void)
{
    int sizes[] = {1000, 2000, 4000, 8000, 16000, 20000};
    int num_sizes = (int)(sizeof(sizes) / sizeof(sizes[0]));
    int all_ok = 1;
    int si;

    printf("Pivot: Median-of-Three, Time: average of %d runs (clock())\n\n", TIME_RUNS);

    for (si = 0; si < num_sizes; si++) {
        int n = sizes[si];
        int m = n / 5;
        int cap = n + m;
        int *base = malloc(sizeof(int) * (size_t)n);
        int *arr = malloc(sizeof(int) * (size_t)cap);
        int *check = malloc(sizeof(int) * (size_t)cap);
        Op *ops = malloc(sizeof(Op) * (size_t)m);
        Result rq, ra;
        double tq = 0.0, ta = 0.0;
        int inserts = 0, deletes = 0, deleted = 0;
        int run, i;
        int sort_ok_q = 1, sort_ok_a = 1, same = 1;

        /* 데이터 및 연산 생성 (측정 구간 밖) */
        srand(DATA_SEED);
        for (i = 0; i < n; i++) {
            base[i] = rand() % VALUE_RANGE;
        }
        srand(OPS_SEED);
        for (i = 0; i < m; i++) {
            ops[i].is_insert = (rand() % 2 == 0);   /* 짝수: 삽입, 홀수: 삭제 */
            ops[i].value = rand() % VALUE_RANGE;    /* 값은 별도 난수 */
            if (ops[i].is_insert) inserts++; else deletes++;
        }

        for (run = 0; run < TIME_RUNS; run++) {
            int count;
            Node *root = NULL;
            double s;
            size_t sort_peak, op_peak;
            int k;

            /* ---------- 배열 + 퀵 정렬 ---------- */
            memcpy(arr, base, sizeof(int) * (size_t)n);
            count = n;
            reset_counters();
            s = now_ms();
            quick_sort_rec(arr, 0, n - 1);
            rq.sort_cmp = cmp_count;
            rq.sort_move = move_count;
            {
                int peak_count = count;
                for (i = 0; i < m; i++) {
                    if (ops[i].is_insert) {
                        array_insert(arr, &count, ops[i].value);
                    } else {
                        int d = array_delete(arr, &count, ops[i].value);
                        if (run == 0) deleted += d;
                    }
                    if (count > peak_count) peak_count = count;
                }
                tq += now_ms() - s;
                sort_peak = sizeof(int) * (size_t)n + (size_t)max_depth * FRAME_QUICK;
                op_peak = sizeof(int) * (size_t)peak_count;
            }
            if (run == 0) {
                rq.comparisons = cmp_count;
                rq.movements = move_count;
                rq.memory_bytes = sort_peak > op_peak ? sort_peak : op_peak;
                rq.final_count = count;
            }

            /* ---------- AVL 트리 ---------- */
            reset_counters();
            s = now_ms();
            for (i = 0; i < n; i++) {
                root = avl_insert(root, base[i]);
            }
            ra.sort_cmp = cmp_count;
            ra.sort_move = move_count;
            for (i = 0; i < m; i++) {
                if (ops[i].is_insert) {
                    root = avl_insert(root, ops[i].value);
                } else {
                    root = avl_delete(root, ops[i].value);
                }
            }
            ta += now_ms() - s;

            if (run == 0) {
                ra.comparisons = cmp_count;
                ra.movements = move_count;
                ra.memory_bytes = (size_t)max_node_count * sizeof(Node)
                                + (size_t)max_depth * FRAME_AVL;
                k = 0;
                avl_inorder(root, check, &k);
                ra.final_count = k;
                sort_ok_a = is_sorted_asc(check, k);
                same = (k == count) && memcmp(check, arr, sizeof(int) * (size_t)k) == 0;
            }
            avl_free(root);
        }
        rq.time_ms = tq / TIME_RUNS;
        ra.time_ms = ta / TIME_RUNS;

        /* 정렬 직후(삽입/삭제 전) 결과 확인: 퀵 정렬 오름차순, AVL 중위 순회 = 퀵 정렬 결과 */
        {
            Node *root = NULL;
            int k = 0;
            memcpy(arr, base, sizeof(int) * (size_t)n);
            reset_counters();
            quick_sort_rec(arr, 0, n - 1);
            sort_ok_q = is_sorted_asc(arr, n);
            for (i = 0; i < n; i++) root = avl_insert(root, base[i]);
            avl_inorder(root, check, &k);
            if (k != n || memcmp(check, arr, sizeof(int) * (size_t)n) != 0) sort_ok_a = 0;
            avl_free(root);
        }

        if (!(sort_ok_q && sort_ok_a && same)) all_ok = 0;

        {
            char q[32], v[32];
            printf("Data Size : %d\n", n);
            printf("Operations: %d\n\n", m);
            printf("%-18s%-16s%s\n", "", "Quick Sort", "AVL Tree");
            printf("------------------------------------------------\n");
            snprintf(q, sizeof q, "%.4f ms", rq.time_ms);
            snprintf(v, sizeof v, "%.4f ms", ra.time_ms);
            printf("%-18s%-16s%s\n", "Total Time", q, v);
            printf("%-18s%-16lld%lld\n", "Comparisons", rq.comparisons, ra.comparisons);
            printf("%-18s%-16lld%lld\n", "Data Movements", rq.movements, ra.movements);
            snprintf(q, sizeof q, "%zu Bytes", rq.memory_bytes);
            snprintf(v, sizeof v, "%zu Bytes", ra.memory_bytes);
            printf("%-18s%-16s%s\n", "Memory Usage", q, v);
            printf("\n%-18s%-16d%d\n", "Final Data Count", rq.final_count, ra.final_count);
            printf("\n[Detail] insert %d, delete %d (found %d, not found %d)\n",
                   inserts, deletes, deleted, deletes - deleted);
            printf("  Sort phase    cmp  : %-14lld %lld\n", rq.sort_cmp, ra.sort_cmp);
            printf("  Sort phase    move : %-14lld %lld\n", rq.sort_move, ra.sort_move);
            printf("  Ins/Del phase cmp  : %-14lld %lld\n", rq.comparisons - rq.sort_cmp, ra.comparisons - ra.sort_cmp);
            printf("  Ins/Del phase move : %-14lld %lld\n", rq.movements - rq.sort_move, ra.movements - ra.sort_move);
            printf("  Check: sorted(Quick)=%s, inorder(AVL)=%s, same data=%s\n\n",
                   sort_ok_q ? "OK" : "FAIL", sort_ok_a ? "OK" : "FAIL", same ? "OK" : "FAIL");
        }

        free(base);
        free(arr);
        free(check);
        free(ops);
    }

    printf("Overall verification : %s\n", all_ok ? "ALL OK" : "FAILED");
    return all_ok ? 0 : 1;
}
