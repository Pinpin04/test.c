#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_LEN 500

typedef struct {
    int data[MAX_LEN];
    int top;
} Stack;

/* Initialize stack */
void initStack(Stack* s) {
    s->top = -1;
}

/* Check empty */
int isEmpty(Stack* s) {
    return s->top == -1;
}

/* Push data */
void push(Stack* s, int val) {
    if (s->top < MAX_LEN - 1) {
        s->data[++(s->top)] = val;
    }
}

/* Pop data */
int pop(Stack* s) {
    if (!isEmpty(s)) {
        return s->data[(s->top)--];
    }
    return 0;
}

/* Check top */
int peek(Stack* s) {
    if (!isEmpty(s)) {
        return s->data[s->top];
    }
    return 0;
}

/* Check tree notation */
int isValidTreeString(const char* s) {
    int len = strlen(s);

    if (len == 0)
        return 0;

    Stack st;
    initStack(&st);

    int expectingNode = 1;
    int nodeCount = 0;

    for (int i = 0; i < len; i++) {
        char ch = s[i];

        if (isupper((unsigned char)ch)) {

            if (!expectingNode)
                return 0;

            nodeCount++;
            expectingNode = 0;

        } else if (ch == '(') {

            if (expectingNode)
                return 0;

            push(&st, '(');
            expectingNode = 1;

        } else if (ch == ',') {

            if (isEmpty(&st) || expectingNode)
                return 0;

            expectingNode = 1;

        } else if (ch == ')') {

            if (isEmpty(&st) || expectingNode)
                return 0;

            pop(&st);
            expectingNode = 0;

        } else {
            return 0;
        }
    }

    if (!isEmpty(&st) || expectingNode)
        return 0;

    if (nodeCount == 0)
        return 0;

    return 1;
}

int main(void) {
    char input[MAX_LEN];

    printf("트리 괄호 표기법 입력: ");

    if (scanf("%499s", input) != 1) {
        return 0;
    }

    if (!isValidTreeString(input)) {
        printf("오류: 올바르지 않은 트리의 괄호 표기법입니다.\n");
        return 0;
    }

    int totalNodes = 0;
    int leafNodes = 0;
    int nonLeafNodes = 0;
    int maxHeight = 0;
    int maxDegree = 0;

    Stack nodeStack;
    Stack childCountStack;

    initStack(&nodeStack);
    initStack(&childCountStack);

    char parentOfC = '\0';
    char childrenOfC[MAX_LEN];
    int childrenOfCCount = 0;

    int currentDepth = 0;
    int len = strlen(input);

    printf("\n[트리 계층 구조 출력]\n");

    for (int i = 0; i < len; i++) {
        char ch = input[i];

        /* Process node */
        if (isupper((unsigned char)ch)) {

            totalNodes++;

            /* Calculate height */
            if (currentDepth + 1 > maxHeight) {
                maxHeight = currentDepth + 1;
            }

            /* Find C's parent */
            if (ch == 'C' && !isEmpty(&nodeStack)) {
                parentOfC = (char)peek(&nodeStack);
            }

            /* Find C's children */
            if (!isEmpty(&nodeStack) &&
                peek(&nodeStack) == 'C') {

                childrenOfC[childrenOfCCount++] = ch;
            }

            /* Check leaf */
            int hasChild =
                (i + 1 < len && input[i + 1] == '(');

            if (hasChild) {
                nonLeafNodes++;
            } else {
                leafNodes++;

                /* Count child */
                if (!isEmpty(&childCountStack)) {
                    int cnt = pop(&childCountStack);
                    push(&childCountStack, cnt + 1);
                }
            }

            /* Print tree */
            if (currentDepth == 0) {
                printf("%c\n", ch);
            } else {
                for (int d = 0; d < currentDepth - 1; d++) {
                    printf("    ");
                }

                printf("+---%c\n", ch);
            }

        /* Open parenthesis */
        } else if (ch == '(') {

            char parent = input[i - 1];

            push(&nodeStack, parent);
            push(&childCountStack, 0);

            currentDepth++;

        /* Close parenthesis */
        } else if (ch == ')') {

            currentDepth--;

            int currentDegree = pop(&childCountStack);

            pop(&nodeStack);

            /* Update degree */
            if (currentDegree > maxDegree) {
                maxDegree = currentDegree;
            }

            /* Count subtree */
            if (!isEmpty(&childCountStack)) {
                int cnt = pop(&childCountStack);
                push(&childCountStack, cnt + 1);
            }
        }
    }

    printf("\n[트리 정보 출력]\n");

    printf("* 전체 노드의 수: %d\n", totalNodes);
    printf("* 단말 노드의 수: %d\n", leafNodes);
    printf("* 비단말 노드의 수: %d\n", nonLeafNodes);
    printf("* 트리의 높이: %d\n", maxHeight);
    printf("* 트리의 차수: %d\n", maxDegree);

    /* Print C's parent */
    if (parentOfC != '\0') {
        printf("* 노드 C의 부모 노드: %c\n", parentOfC);
    } else {
        printf("* 노드 C의 부모 노드: 없음\n");
    }

    /* Print C's children */
    printf("* 노드 C의 자식 노드: ");

    if (childrenOfCCount == 0) {
        printf("없음\n");
    } else {
        for (int i = 0; i < childrenOfCCount; i++) {
            printf("%c%s",
                   childrenOfC[i],
                   (i == childrenOfCCount - 1) ? "" : ", ");
        }
        printf("\n");
    }

    return 0;
}
