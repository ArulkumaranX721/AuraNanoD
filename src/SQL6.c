#include "SQL6.h"
#include "TableManager.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>

#define S6_MAX_TABLES 32
#define S6_MAX_COLUMNS 16
#define S6_MAX_ROWS 1000
#define S6_NAME 32
#define S6_VALUE 128
#define S6_TYPE 24
#define S6_SCHEMA_FILE "./data/schema.dat"
#define S6_ROWS_FILE "./data/schema_rows.dat"
#define S6_MAGIC 0x4C583653u
#define S6_VERSION 1u

typedef struct { char name[S6_NAME]; char type[S6_TYPE]; } S6Column;
typedef struct { int table_id; char name[64]; int column_count; S6Column columns[S6_MAX_COLUMNS]; } S6Table;
typedef struct { uint32_t magic, version; int count; S6Table tables[S6_MAX_TABLES]; } S6SchemaFile;
typedef struct { int table_id; char values[S6_MAX_COLUMNS][S6_VALUE]; } S6Row;
typedef struct { uint32_t magic, version; int count; S6Row rows[S6_MAX_ROWS]; } S6RowFile;
typedef struct { char text[S6_VALUE]; int quoted; } S6Tok;
static S6SchemaFile schema;
static S6RowFile rows;

static int ieq(const char *a,const char *b) { while(*a&&*b){if(tolower((unsigned char)*a)!=tolower((unsigned char)*b))return 0;a++;b++;}return *a==*b; }
static void copy_text(char *dst,size_t cap,const char *src){if(!cap)return;size_t n=strlen(src);if(n>=cap)n=cap-1;memcpy(dst,src,n);dst[n]=0;}
static int save_schema(void){ FILE*f=fopen(S6_SCHEMA_FILE,"wb"); if(!f)return 0; int ok=fwrite(&schema,sizeof(schema),1,f)==1; fclose(f); return ok; }
static int save_rows(void){ FILE*f=fopen(S6_ROWS_FILE,"wb"); if(!f)return 0; int ok=fwrite(&rows,sizeof(rows),1,f)==1; fclose(f); return ok; }
int sql6_initialize(void){
 FILE*f=fopen(S6_SCHEMA_FILE,"rb"); if(!f){memset(&schema,0,sizeof(schema));schema.magic=S6_MAGIC;schema.version=S6_VERSION;if(!save_schema())return 0;}
 else {S6SchemaFile x;int ok=fread(&x,sizeof(x),1,f)==1;fclose(f);if(!ok||x.magic!=S6_MAGIC||x.version!=S6_VERSION||x.count<0||x.count>S6_MAX_TABLES){puts("Sprint 6: invalid schema metadata.");return 0;}schema=x;}
 f=fopen(S6_ROWS_FILE,"rb"); if(!f){memset(&rows,0,sizeof(rows));rows.magic=S6_MAGIC;rows.version=S6_VERSION;if(!save_rows())return 0;}
 else {S6RowFile x;int ok=fread(&x,sizeof(x),1,f)==1;fclose(f);if(!ok||x.magic!=S6_MAGIC||x.version!=S6_VERSION||x.count<0||x.count>S6_MAX_ROWS){puts("Sprint 6: invalid row storage.");return 0;}rows=x;}
 return 1;
}
static S6Table *find_table(const char*n){for(int i=0;i<schema.count;i++)if(ieq(schema.tables[i].name,n))return &schema.tables[i];return NULL;}
static int col_index(const S6Table*t,const char*n){for(int i=0;i<t->column_count;i++)if(ieq(t->columns[i].name,n))return i;return -1;}
static int numeric(const char*s,double*v){char*e;*v=strtod(s,&e);return s[0]&&*e==0;}
static int starts_ci(const char*s,const char*p){while(*p){if(tolower((unsigned char)*s++)!=tolower((unsigned char)*p++))return 0;}return 1;}
static int tokenize(const char*s,S6Tok*t,int*count){int n=0;while(*s){while(isspace((unsigned char)*s))s++;if(!*s)break;if(*s==';'){s++;continue;}if(n>=256)return 0;memset(&t[n],0,sizeof(t[n]));
 if(*s=='\''){t[n].quoted=1;s++;size_t k=0;while(*s){if(*s=='\''&&s[1]=='\''){if(k+1<S6_VALUE)t[n].text[k++]='\'';s+=2;continue;}if(*s=='\''){s++;break;}if(k+1<S6_VALUE)t[n].text[k++]=*s;s++;}t[n].text[k]=0;n++;continue;}
 if(isalnum((unsigned char)*s)||*s=='_'||*s=='.'){size_t k=0;while(isalnum((unsigned char)*s)||*s=='_'||*s=='.'){if(k+1<S6_VALUE)t[n].text[k++]=*s;s++;}t[n].text[k]=0;n++;continue;}
 t[n].text[0]=*s++;t[n].text[1]=0;if((*t[n].text=='>'||*t[n].text=='<'||*t[n].text=='!'||*t[n].text=='=')&&*s=='='){t[n].text[1]='=';t[n].text[2]=0;s++;}else if(*t[n].text=='<'&&*s=='>'){t[n].text[1]='>';t[n].text[2]=0;s++;}n++;}
 *count=n;return 1;}
static int create_table(S6Tok*t,int n){
 if(n<5||!ieq(t[0].text,"create")||!ieq(t[1].text,"table")||strcmp(t[3].text,"(")!=0){puts("Sprint 6 SQL: expected CREATE TABLE name (column type, ...).");return 1;}
 const char*name=t[2].text;if(find_table(name)){printf("Sprint 6: table already has a schema: %s\n",name);return 1;}
 TM_Table tm;if(!table_manager_find(name,&tm)){if(!table_manager_create(name)||!table_manager_find(name,&tm)){puts("Sprint 6: could not create table.");return 1;}}
 if(schema.count>=S6_MAX_TABLES){puts("Sprint 6: maximum schema table count reached.");return 1;}
 S6Table tmp;memset(&tmp,0,sizeof(tmp));tmp.table_id=tm.table_id;copy_text(tmp.name,sizeof(tmp.name),name);int i=4;
 while(i<n&&strcmp(t[i].text,")")!=0){if(tmp.column_count>=S6_MAX_COLUMNS){puts("Sprint 6: too many columns (max 16).");return 1;}S6Column*c=&tmp.columns[tmp.column_count];copy_text(c->name,sizeof(c->name),t[i++].text);if(i>=n){puts("Sprint 6: missing column type.");return 1;}copy_text(c->type,sizeof(c->type),t[i++].text);if(i<n&&strcmp(t[i].text,"(")==0){i++;if(i<n){size_t z=strlen(c->type);if(z+strlen(t[i].text)+3<sizeof(c->type)){strcat(c->type,"(");strcat(c->type,t[i].text);strcat(c->type,")");}i++;}if(i<n&&strcmp(t[i].text,")")==0)i++;}tmp.column_count++;if(i<n&&strcmp(t[i].text,",")==0)i++;else if(i<n&&strcmp(t[i].text,")")!=0){puts("Sprint 6: expected comma between columns.");return 1;}}
 if(tmp.column_count==0){puts("Sprint 6: table needs at least one column.");return 1;}for(int a=0;a<tmp.column_count;a++)for(int b=a+1;b<tmp.column_count;b++)if(ieq(tmp.columns[a].name,tmp.columns[b].name)){puts("Sprint 6: duplicate column name.");return 1;}
 schema.tables[schema.count++]=tmp;if(!save_schema()){schema.count--;puts("Sprint 6: could not save schema.");return 1;}printf("Sprint 6 table created: %s (%d columns)\n",name,tmp.column_count);return 1;
}
static int insert_row(S6Tok*t,int n){if(n<7||!ieq(t[0].text,"insert")||!ieq(t[1].text,"into"))return 0;S6Table*tb=find_table(t[2].text);if(!tb)return 0;if(!ieq(t[3].text,"values")||strcmp(t[4].text,"(")!=0){puts("Sprint 6 INSERT: use INSERT INTO table VALUES (...).");return 1;}if(rows.count>=S6_MAX_ROWS){puts("Sprint 6: row capacity reached (1000).");return 1;}
 S6Row r;memset(&r,0,sizeof(r));r.table_id=tb->table_id;int i=5,c=0;while(i<n&&strcmp(t[i].text,")")!=0){if(c>=tb->column_count){puts("Sprint 6 INSERT: too many values.");return 1;}snprintf(r.values[c],S6_VALUE,"%s",t[i].text);c++;i++;if(i<n&&strcmp(t[i].text,",")==0)i++;else if(i<n&&strcmp(t[i].text,")")!=0){puts("Sprint 6 INSERT: expected comma.");return 1;}}
 if(c!=tb->column_count){printf("Sprint 6 INSERT: expected %d values, got %d.\n",tb->column_count,c);return 1;}
 for(int k=0;k<tb->column_count;k++){
  const char *ty=tb->columns[k].type; double number;
  if((ieq(ty,"INT")||ieq(ty,"INTEGER")||ieq(ty,"BIGINT"))&&!numeric(r.values[k],&number)){printf("Sprint 6 INSERT: column %s requires a numeric value.\n",tb->columns[k].name);return 1;}
  if(starts_ci(ty,"VARCHAR")||starts_ci(ty,"CHAR")){
   const char *open=strchr(ty,'(');if(open){int maxlen=atoi(open+1);if(maxlen>0&&(int)strlen(r.values[k])>maxlen){printf("Sprint 6 INSERT: value too long for column %s (max %d).\n",tb->columns[k].name,maxlen);return 1;}}
  }
 }
 rows.rows[rows.count++]=r;if(!save_rows()){rows.count--;puts("Sprint 6: failed to persist inserted row.");return 1;}printf("1 row inserted into %s.\n",tb->name);return 1;}
static int cmpval(const char*a,const char*b){double x,y;if(numeric(a,&x)&&numeric(b,&y))return x>y?1:x<y?-1:0;return strcmp(a,b);}
static int row_matches(const S6Table*t,const S6Row*r,int where,const char*wc,const char*op,const char*wv){if(!where)return 1;int ci=col_index(t,wc);if(ci<0){printf("Sprint 6 WHERE: unknown column %s.\n",wc);return 0;}int c=cmpval(r->values[ci],wv);if(!strcmp(op,"="))return c==0;if(!strcmp(op,"!=")||!strcmp(op,"<>"))return c!=0;if(!strcmp(op,">"))return c>0;if(!strcmp(op,"<"))return c<0;if(!strcmp(op,">="))return c>=0;if(!strcmp(op,"<="))return c<=0;return 0;}
static void print_header(S6Table*t,int*proj,int pc,int count){for(int i=0;i<pc;i++)printf("%-18s",count&&i==pc-1?"COUNT(*)":t->columns[proj[i]].name);puts("");for(int i=0;i<pc;i++)printf("------------------");puts("");}
static int select_query(S6Tok*t,int n){if(n<4||!ieq(t[0].text,"select"))return 0;int from=-1;for(int i=1;i<n;i++)if(ieq(t[i].text,"from")){from=i;break;}if(from<0||from+1>=n)return 0;S6Table*tb=find_table(t[from+1].text);if(!tb)return 0;
 int proj[S6_MAX_COLUMNS],pc=0,star=0,count=0;for(int i=1;i<from;){if(strcmp(t[i].text,"*")==0){star=1;i++;break;}if(ieq(t[i].text,"count")&&i+3<from&&strcmp(t[i+1].text,"(")==0&&strcmp(t[i+2].text,"*")==0&&strcmp(t[i+3].text,")")==0){count=1;i+=4;break;}int ci=col_index(tb,t[i].text);if(ci<0){printf("Sprint 6 SELECT: unknown column %s.\n",t[i].text);return 1;}proj[pc++]=ci;i++;if(i<from&&strcmp(t[i].text,",")==0)i++;else break;}
 if(star){pc=tb->column_count;for(int i=0;i<pc;i++)proj[i]=i;}if(count&&pc==0)proj[pc++]=0;
 int where=0,group=-1,order=-1,desc=0,limit=-1;char wc[64]="",op[4]="",wv[S6_VALUE]="";int i=from+2;
 while(i<n){if(ieq(t[i].text,"where")&&i+3<n){where=1;copy_text(wc,sizeof(wc),t[i+1].text);copy_text(op,sizeof(op),t[i+2].text);copy_text(wv,sizeof(wv),t[i+3].text);i+=4;}
 else if(ieq(t[i].text,"group")&&i+2<n&&ieq(t[i+1].text,"by")){group=col_index(tb,t[i+2].text);if(group<0){printf("Sprint 6 GROUP BY: unknown column %s.\n",t[i+2].text);return 1;}i+=3;}
 else if(ieq(t[i].text,"order")&&i+2<n&&ieq(t[i+1].text,"by")){order=col_index(tb,t[i+2].text);if(order<0){printf("Sprint 6 ORDER BY: unknown column %s.\n",t[i+2].text);return 1;}i+=3;if(i<n&&ieq(t[i].text,"desc")){desc=1;i++;}else if(i<n&&ieq(t[i].text,"asc"))i++;}
 else if(ieq(t[i].text,"limit")&&i+1<n){limit=atoi(t[i+1].text);if(limit<0)limit=0;i+=2;}else i++;}
 int ix[S6_MAX_ROWS],m=0;for(int r=0;r<rows.count;r++)if(rows.rows[r].table_id==tb->table_id&&row_matches(tb,&rows.rows[r],where,wc,op,wv))ix[m++]=r;
 if(group>=0&&count){printf("%-18s%-18s\n",tb->columns[group].name,"COUNT(*)");printf("------------------------------------\n");int used[S6_MAX_ROWS]={0};for(int a=0;a<m;a++){int ri=ix[a];if(used[ri])continue;int cnt=0;for(int b=a;b<m;b++)if(!strcmp(rows.rows[ix[b]].values[group],rows.rows[ri].values[group])){cnt++;used[ix[b]]=1;}printf("%-18s%-18d\n",rows.rows[ri].values[group],cnt);}return 1;}
 if(count&&group<0){printf("COUNT(*)\n--------\n%d\n",m);printf("1 row(s) selected.\n");return 1;}
 if(order>=0)for(int a=0;a<m;a++)for(int b=a+1;b<m;b++){int c=cmpval(rows.rows[ix[a]].values[order],rows.rows[ix[b]].values[order]);if((!desc&&c>0)||(desc&&c<0)){int z=ix[a];ix[a]=ix[b];ix[b]=z;}}
 if(limit>=0&&m>limit) m=limit;
 print_header(tb,proj,pc,count);
 for(int a=0;a<m;a++){
  S6Row*r=&rows.rows[ix[a]];
  if(count) printf("%-18d\n",1);
  else { for(int p=0;p<pc;p++) printf("%-18s",r->values[proj[p]]); puts(""); }
 }
 printf("%d row(s) selected.\n",m);return 1;}
int sql6_execute(const char*sql){if(!sql)return 0;S6Tok t[256];int n=0;if(!tokenize(sql,t,&n)||n==0)return 0;if(ieq(t[0].text,"create")&&n>3&&strcmp(t[3].text,"(")==0)return create_table(t,n);if(ieq(t[0].text,"insert"))return insert_row(t,n);if(ieq(t[0].text,"select"))return select_query(t,n);return 0;}
