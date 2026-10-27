#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static long long cnt;

void insertion_sort(int *a, int n) {
    for (int i = 1; i < n; i++) {
        int key = a[i];
        int j = i - 1;
        while (j >= 0) {
            cnt++;
            if (a[j] > key) {
                a[j + 1] = a[j];
                j--;
            } else break;
        }
        a[j + 1] = key;
    }
}

void shell_gap(int *a, int n, int h) {
    for (int i = h; i < n; i++) {
        int key = a[i];
        int j = i - h;
        while (j >= 0) {
            cnt++;
            if (a[j] > key) {
                a[j + h] = a[j];
                j -= h;
            } else break;
        }
        a[j + h] = key;
    }
}

void shell_original(int *a, int n) {
    for (int h = n / 2; h >= 1; h /= 2) shell_gap(a, n, h);
}

void shell_knuth(int *a, int n) {
    int h = 1;
    while (3 * h + 1 < n) h = 3 * h + 1;
    for (; h >= 1; h = (h - 1) / 3) shell_gap(a, n, h);
}

int is_sorted(int *a, int n) {
    for (int i = 1; i < n; i++) if (a[i - 1] > a[i]) return 0;
    return 1;
}

int main(void) {
    int sizes[] = {1000, 2000, 4000, 8000, 16000, 20000};
    srand(12345);
    for (int s = 0; s < 6; s++) {
        int n = sizes[s];
        int *orig = malloc(n * sizeof(int));
        int *a = malloc(n * sizeof(int));
        for (int i = 0; i < n; i++) orig[i] = rand();

        printf("Data Size : %d\n\n", n);

        memcpy(a, orig, n * sizeof(int));
        cnt = 0; insertion_sort(a, n);
        printf("Insertion Sort       : %lld comparisons%s\n", cnt, is_sorted(a, n) ? "" : " (SORT ERROR)");

        memcpy(a, orig, n * sizeof(int));
        cnt = 0; shell_original(a, n);
        printf("Shell Sort (Original): %lld comparisons%s\n", cnt, is_sorted(a, n) ? "" : " (SORT ERROR)");

        memcpy(a, orig, n * sizeof(int));
        cnt = 0; shell_knuth(a, n);
        printf("Shell Sort (Knuth)   : %lld comparisons%s\n\n", cnt, is_sorted(a, n) ? "" : " (SORT ERROR)");

        free(orig); free(a);
    }
    return 0;
}
