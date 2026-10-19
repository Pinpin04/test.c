#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define V 100
typedef struct Node { int v; struct Node *next; } Node;
typedef struct { unsigned char m[V][V]; } Matrix;
typedef struct { Node *head[V]; size_t mem; } List;
typedef struct { int u, v; } Pair;
typedef struct { size_t mem; long ins, del, conn, adj; } Result;

static int m_insert(Matrix *g,int u,int v,long *c){ (*c)++; if(g->m[u][v]) return 0; g->m[u][v]=g->m[v][u]=1; return 1; }
static int m_delete(Matrix *g,int u,int v,long *c){ (*c)++; if(!g->m[u][v]) return 0; g->m[u][v]=g->m[v][u]=0; return 1; }
static int m_conn(Matrix *g,int u,int v,long *c){ (*c)++; return g->m[u][v]; }
static void m_adj(Matrix *g,int u,long *c,int pr){ for(int i=0;i<V;i++){ (*c)++; if(g->m[u][i]&&pr) printf("%d ",i);} }

static void l_push(List *g,int u,int v){ Node *n=malloc(sizeof(Node)); n->v=v; n->next=g->head[u]; g->head[u]=n; g->mem+=sizeof(Node); }

static int l_insert(List *g,int u,int v,long *c){
  for(Node *p=g->head[u];p;p=p->next){ (*c)++; if(p->v==v) return 0; }
  l_push(g,u,v); l_push(g,v,u); return 1; }
static int l_unlink(List *g,int u,int v,long *c){
  Node *prev=NULL;
  for(Node *p=g->head[u];p;prev=p,p=p->next){ (*c)++;
    if(p->v==v){ if(prev) prev->next=p->next; else g->head[u]=p->next; free(p); g->mem-=sizeof(Node); return 1; } }
  return 0; }

static int l_delete(List *g,int u,int v,long *c){ int a=l_unlink(g,u,v,c); int b=l_unlink(g,v,u,c); return a&&b; }
static int l_conn(List *g,int u,int v,long *c){ for(Node *p=g->head[u];p;p=p->next){ (*c)++; if(p->v==v) return 1; } return 0; }
static void l_adj(List *g,int u,long *c,int pr){ for(Node *p=g->head[u];p;p=p->next){ (*c)++; if(pr) printf("%d ",p->v);} }

static void gen(int E,Pair *edges,unsigned char (*chk)[V]){
  memset(chk,0,V*V); int n=0;
  while(n<E){ int u=rand()%V,v=rand()%V; if(u==v||chk[u][v]) continue;
    chk[u][v]=chk[v][u]=1; edges[n].u=u; edges[n].v=v; n++; } }

static void run(const char *name,int E,int caseM,int caseL,Result *rm,Result *rl,double *avg){
  static Pair edges[4000]; static unsigned char chk[V][V];
  gen(E,edges,chk);
  Matrix *M=calloc(1,sizeof(Matrix)); List *L=calloc(1,sizeof(List));
  long dummy=0;
  for(int i=0;i<E;i++){ m_insert(M,edges[i].u,edges[i].v,&dummy); l_insert(L,edges[i].u,edges[i].v,&dummy); }
  rm->mem=sizeof(Matrix); rl->mem=sizeof(List)-sizeof(size_t)+L->mem;

  Pair ins,del,cn; int a;
  do{ ins.u=rand()%V; ins.v=rand()%V; }while(ins.u==ins.v||chk[ins.u][ins.v]);
  del=edges[rand()%E];
  do{ cn.u=rand()%V; cn.v=rand()%V; }while(cn.u==cn.v);
  a=rand()%V;
  printf("== %s: ins=(%d,%d) del=(%d,%d) conn=(%d,%d) adjv=%d\n",name,ins.u,ins.v,del.u,del.v,cn.u,cn.v,a);
  rm->ins=rm->del=rm->conn=rm->adj=0; *rl=(Result){rl->mem,0,0,0,0};
  m_insert(M,ins.u,ins.v,&rm->ins); l_insert(L,ins.u,ins.v,&rl->ins);
  m_delete(M,del.u,del.v,&rm->del); l_delete(L,del.u,del.v,&rl->del);
  int c1=m_conn(M,cn.u,cn.v,&rm->conn), c2=l_conn(L,cn.u,cn.v,&rl->conn);
  printf("connected? matrix=%d list=%d\nneighbors(matrix): ",c1,c2); m_adj(M,a,&rm->adj,1);
  printf("\nneighbors(list)  : "); l_adj(L,a,&rl->adj,1); printf("\n");

  long mi=0,li=0,md=0,ld=0,mc=0,lc=0,ma=0,la=0; int T=1000;
  for(int t=0;t<T;t++){
    int u,v; do{u=rand()%V;v=rand()%V;}while(u==v||M->m[u][v]);
    long x=0,y=0; m_insert(M,u,v,&x); l_insert(L,u,v,&y); mi+=x; li+=y;
    x=y=0; m_delete(M,u,v,&x); l_delete(L,u,v,&y); md+=x; ld+=y;
    do{u=rand()%V;v=rand()%V;}while(u==v||!M->m[u][v]);
    x=y=0; m_delete(M,u,v,&x); l_delete(L,u,v,&y); md+=x; ld+=y; m_insert(M,u,v,&x); l_insert(L,u,v,&y);
    do{u=rand()%V;v=rand()%V;}while(u==v);
    x=y=0; m_conn(M,u,v,&x); l_conn(L,u,v,&y); mc+=x; lc+=y;
    u=rand()%V; x=y=0; m_adj(M,u,&x,0); l_adj(L,u,&y,0); ma+=x; la+=y; }

  avg[0]=(double)mi/T; avg[1]=(double)li/T; avg[2]=(double)md/(2*T); avg[3]=(double)ld/(2*T);
  avg[4]=(double)mc/T; avg[5]=(double)lc/T; avg[6]=(double)ma/T; avg[7]=(double)la/T;
  free(M); for(int i=0;i<V;i++) for(Node *p=L->head[i],*q;p;p=q){q=p->next;free(p);} free(L);
}
static void show(const char *t,Result *r){
  printf("%s\nMemory                         : %zu Bytes\nEdge insertion comparisons     : %ld\nEdge deletion comparisons      : %ld\nConnectivity check comparisons : %ld\nAdjacent vertices comparisons  : %ld\n\n",t,r->mem,r->ins,r->del,r->conn,r->adj); }
static void showavg(const char *n,double *a){
  printf("[%s] avg of 1000 trials  (matrix / list)\n  insert %.2f / %.2f\n  delete %.2f / %.2f\n  connect %.2f / %.2f\n  adjacent %.2f / %.2f\n",n,a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7]); }

int main(void){
  srand(2024);
  Result sm,sl,dm,dl; double as[8],ad[8];
  run("Sparse",100,0,0,&sm,&sl,as);
  run("Dense",4000,0,0,&dm,&dl,ad);
  printf("\n");
  show("Case 1 : Sparse Graph - Adjacency Matrix",&sm);
  show("Case 2 : Sparse Graph - Adjacency List",&sl);
  show("Case 3 : Dense Graph - Adjacency Matrix",&dm);
  show("Case 4 : Dense Graph - Adjacency List",&dl);
  showavg("Sparse",as); showavg("Dense",ad);
  return 0; }
