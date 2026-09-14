#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LEN 500

// --- 스택 구현 ---
typedef struct {
    int data[MAX_LEN];
    int top;
} Stack;

void initStack(Stack* s) {
    s->top = -1;
}

int isEmpty(Stack* s) {
    return s->top == -1;
}

void push(Stack* s, int val) {
    if (s->top < MAX_LEN - 1) {
        s->data[++(s->top)] = val;
    }
}

int pop(Stack* s) {
    if (!isEmpty(s)) {
        return s->data[(s->top)--];
    }
    return 0;
}

int peek(Stack* s) {
    if (!isEmpty(s)) {
        return s->data[s->top];
    }
    return 0;
}

// --- 문자열 유효성 검사 ---
int isValidTreeString(const char* s) {
    int len = strlen(s);
    if (len == 0) return 0;

    Stack st;
    initStack(&st);

    for (int i = 0; i < len; ++i) {
        char ch = s[i];
        if (isupper((unsigned char)ch)) {
            if (i + 1 < len) {
                char next = s[i + 1];
                if (next != '(' && next != ',' && next != ')') return 0;
            }
        } else if (ch == '(') {
            if (i == 0 || !isupper((unsigned char)s[i - 1])) return 0;
            push(&st, '(');
        } else if (ch == ')') {
            if (isEmpty(&st)) return 0;
            pop(&st);
        } else if (ch == ',') {
            if (i == 0 || i == len - 1) return 0;
        } else {
            return 0; // 허용되지 않은 문자
        }
    }
    return isEmpty(&st);
}

int main(void) {
    char input[MAX_LEN];
    printf("트리 괄호 표기법 입력: ");
    if (scanf("%s", input) != 1) return 0;

    // 1. 유효성 검사
    if (!isValidTreeString(input)) {
        printf("오류: 올바르지 않은 트리의 괄호 표기법입니다.\n");
        return 0;
    }

    int totalNodes = 0;
    int leafNodes = 0;
    int nonLeafNodes = 0;
    int maxHeight = 0;
    int maxDegree = 0;

    // 스택 정의
    Stack nodeStack;       // 부모 노드 추적용 (문자 저장)
    Stack childCountStack; // 노드별 자식 수 카운트용 (정수 저장)
    initStack(&nodeStack);
    initStack(&childCountStack);

    char parentOfC = '\0';
    char childrenOfC[MAX_LEN];
    int childrenOfCCount = 0;

    int currentDepth = 0;
    int len = strlen(input);

    printf("\n[트리 계층 구조 출력]\n");

    for (int i = 0; i < len; ++i) {
        char ch = input[i];

        if (isupper((unsigned char)ch)) {
            totalNodes++;

            // 높이 갱신
            if (currentDepth + 1 > maxHeight) {
                maxHeight = currentDepth + 1;
            }

            // 노드 C의 부모 확인
            if (ch == 'C') {
                if (!isEmpty(&nodeStack)) {
                    parentOfC = (char)peek(&nodeStack);
                }
            }

            // 노드 C의 자식 확인
            if (!isEmpty(&nodeStack) && peek(&nodeStack) == 'C') {
                childrenOfC[childrenOfCCount++] = ch;
            }

            // 현재 노드가 자식을 가지는지 여부 (다음 문자가 '(' 인지)
            int hasChild = (i + 1 < len && input[i + 1] == '(');

            if (hasChild) {
                nonLeafNodes++;
            } else {
                leafNodes++;
                // 부모의 자식 수 증가
                if (!isEmpty(&childCountStack)) {
                    int cnt = pop(&childCountStack);
                    push(&childCountStack, cnt + 1);
                }
            }

            // 계층적 출력
            if (currentDepth == 0) {
                printf("%c\n", ch);
            } else {
                for (int d = 0; d < currentDepth - 1; ++d) {
                    printf("    ");
                }
                printf("+---%c\n", ch);
            }

        } else if (ch == '(') {
            char parent = input[i - 1];
            push(&nodeStack, parent);
            push(&childCountStack, 0);
            currentDepth++;

        } else if (ch == ')') {
            currentDepth--;

            int currentDegree = pop(&childCountStack);
            pop(&nodeStack);

            if (currentDegree > maxDegree) {
                maxDegree = currentDegree;
            }

            if (!isEmpty(&childCountStack)) {
                int cnt = pop(&childCountStack);
                push(&childCountStack, cnt + 1);
            }
        }
    }

    // 결과 출력
    printf("\n[트리 정보 출력]\n");
    printf("* 전체 노드의 수: %d\n", totalNodes);
    printf("* 단말 노드의 수: %d\n", leafNodes);
    printf("* 비단말 노드의 수: %d\n", nonLeafNodes);
    printf("* 트리의 높이: %d\n", maxHeight);
    printf("* 트리의 차수: %d\n", maxDegree);

    if (parentOfC != '\0') {
        printf("* 노드 C의 부모 노드: %c\n", parentOfC);
    } else {
        printf("* 노드 C의 부모 노드: 없음\n");
    }

    printf("* 노드 C의 자식 노드: ");
    if (childrenOfCCount == 0) {
        printf("없음\n");
    } else {
        for (int i = 0; i < childrenOfCCount; ++i) {
            printf("%c%s", childrenOfC[i], (i == childrenOfCCount - 1) ? "" : ", ");
        }
        printf("\n");
    }

    return 0;
}
