#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

#include "utility.h"
#include "smemory.h"
#include "internal.h"


void lwqq_ct_free(LwqqConfirmTable* table)
{
    if(table){
        s_free(table->title);
        s_free(table->body);
        s_free(table->input_label);
        s_free(table->yes_label);
        s_free(table->no_label);
        s_free(table->exans_label);
        s_free(table->input);
        s_free(table);
    }
}


LwqqOpCode lwqq_util_save_img(void* ptr,size_t len,const char* path,const char* dir)
{
    if(!ptr||!path) return LWQQ_OP_FAILED;
    char fullpath[1024];
    snprintf(fullpath,sizeof(fullpath),"%s%s%s",dir?:"",dir?"/":"",path);
    FILE* f = fopen(fullpath,"wb");
    if(f==NULL&&errno==2){
        if(dir){
            mkdir(dir,0755);
            f = fopen(fullpath,"wb");
        }else return LWQQ_OP_FAILED;
    }
    if(f==NULL) return LWQQ_OP_FAILED;

    fwrite(ptr,len,1,f);
    fclose(f);
    return LWQQ_OP_OK;
}

int lwqq_util_mapto_type(const struct LwqqTypeMap* maps,const char* key)
{
    while(maps->str != NULL){
        if(key&&!strncmp(maps->str,key,strlen(maps->str))) return maps->type;
        else if(key == NULL && maps->str == NULL) return maps->type;
        maps++;
    }
    return maps->type;
}

const char* lwqq_util_mapto_str(const struct LwqqTypeMap* maps,int type)
{
    while(maps->str != NULL){
        if(maps->type == type) return maps->str;
        maps++;
    }
    return NULL;
}


char* lwqq_util_hashN(const char* uin,const char* ptwebqq,void* unused)
{
    int alen=strlen(uin);
    int *c = malloc(sizeof(int)*strlen(uin));
    int d,b,k,clen;
    int elen=strlen(ptwebqq);
    const char* e = ptwebqq;
    int h;
    int i;
    for(d=0;d<alen;d++){
        c[d]=uin[d]-'0';
    }
    clen = d;
    for(b=0,k=-1,d=0;d<clen;d++){
        b += c[d];
        b %= elen;
        int f = 0;
        if(b+4>elen){
            int g;
            for(g=4+b-elen,h=0;h<4;h++)
                f |= h<g?((e[b+h]&255)<<(3-h)*8):((e[h-g]&255)<<(3-h)*8);
        }else{
            for(h=0;h<4;h++)
                f |= (e[b+h]&255)<<(3-h)*8;
        }
        k ^= f;
    }
    memset(c,0,sizeof(int)*alen);
    c[0] = k >> 24&255;
    c[1] = k >> 16&255;
    c[2] = k >> 8&255;
    c[3] = k & 255;
    const char* ch = "0123456789ABCDEF";
    char* ret = malloc(10);
    memset(ret,0,10);
    for(b=0,i=0;b<4;b++){
        ret[i++]=ch[c[b]>>4&15];
        ret[i++]=ch[c[b]&15];
    }
    free(c);
    return ret;
}
char* lwqq_util_hashO(const char* uin,const char* ptwebqq,void* unused)
{
    char* a = s_malloc0(strlen(ptwebqq)+strlen("password error")+3);
    const char* b = uin;
    strcat(strcpy(a,ptwebqq),"password error");
    size_t alen = strlen(a);
    char* s = s_malloc0(2048);
    int *j = malloc(sizeof(int)*alen);
    for(;;){
        if(strlen(s)<=alen){
            if(strcat(s,b),strlen(s)==alen) break;
        }else{
            s[alen]='\0';
            break;
        }
    }
    int d;
    for(d=0;d<strlen(s);d++){
        j[d]=s[d]^a[d];
    }
    const char* ch = "0123456789ABCDEF";
    s[0]=0;
    for(d=0;d<alen;d++){
        s[2*d]=ch[j[d]>>4&15];
        s[2*d+1]=ch[j[d]&15];
    }
    s_free(a);
    s_free(j);
    return s;
}
char* lwqq_util_hashP(const char* uin,const char* ptwebqq,void* unused)
{
    char a[4]={0};
    int i;
#ifdef WIN32
	unsigned __int64 uin_n = _strtoui64(uin,NULL,10);
#else
	unsigned long long uin_n = strtoull(uin,NULL,10);
#endif
    for(i=0;i<strlen(ptwebqq);i++)
        a[i%4] ^= ptwebqq[i];
    char* j[] = {"EC","OK"};
    char d[4];
    d[0] = (uin_n >>24 & 255) ^ j[0][0];
    d[1] = (uin_n >>16 & 255) ^ j[0][1];
    d[2] = (uin_n >>8  & 255) ^ j[1][0];
    d[3] = (uin_n & 255) ^ j[1][1];
    char j2[8];
    for (i=0;i<8;i++)
        j2[i] = i % 2 == 0 ? a[i >> 1] : d[i >> 1];
    char a2[] = "0123456789ABCDEF";
    char d2[17] = {0};
    for (i=0;i<8;i++){
        d2[i*2] = a2[j2[i] >> 4&15];
        d2[i*2+1] = a2[j2[i] & 15]; 
    } 
    return s_strdup(d2);
} 
struct hash_slice{
	int s;
	int e;
};
char* lwqq_util_hashQ(const char* uin,const char* ptwebqq,void* _unused)
{
	int r[4];
#ifdef WIN32
	unsigned __int64 uin_n = _strtoui64(uin,NULL,10);
#else
	unsigned long long uin_n = strtoull(uin,NULL,10);
#endif
	r[0] = uin_n>>24&255;
	r[1] = uin_n>>16&255;
	r[2] = uin_n>>8&255;
	r[3] = uin_n&255;
	char* j = s_strdup(ptwebqq);
	struct hash_slice e_ins;
	struct hash_slice e[1024];
	int e_idx = 0;
	e[e_idx++] = (e_ins.s = 0,e_ins.e = strlen(j)-1,e_ins);
	for(;e_idx>0;){
		struct hash_slice c;
		c = e[--e_idx];
		if(!(c.s>=c.e||c.s<0||c.e>=strlen(j))){
			if(c.s+1==c.e){
				if(j[c.s]>j[c.e]){
					int l=j[c.s];
					j[c.s]=j[c.e];
					j[c.e]=l;
				}
			}else{
				int l=c.s;
				int J=c.e;
				int f=j[c.s];
				for(;c.s<c.e;){
					for(;c.s<c.e&&j[c.e]>=f;){
						c.e--;
						r[0]=(r[0]+3)&255;
					}
					if(c.s<c.e){
						j[c.s]=j[c.e];
						c.s++;
						r[1]=(r[1]*13+43)&255;
					}
					for(;c.s<c.e&&j[c.s]<=f;){
						c.s++;
						r[2]=(r[2]-3)&255;
					}
					if(c.s<c.e){
						j[c.e]=j[c.s];
						c.e--;
						r[3]=(r[0]^r[1]^r[2]^(r[3]+1))&255;
					}
				}
				j[c.s]=f;
				e[e_idx++]=(e_ins.s=l,e_ins.e=c.s-1,e_ins);
				e[e_idx++]=(e_ins.s=c.s+1,e_ins.e=J,e_ins);
			}
        }
	}
	char j2[] = "0123456789ABCDEF";
	char e2[10] = {0};
	int c;
	for(c=0;c<4;c++){
		e2[c*2]=j2[r[c]>>4&15];
		e2[c*2+1]=j2[r[c]&15];
	}
	s_free(j);
	return s_strdup(e2);
}

void ds_cat_(struct ds* str,...)
{
    va_list args;
    const char* cat;
    va_start(args,str);
    while((cat = va_arg(args,const char*))!=0){
        ds_pokes(*str,cat);
    }
    va_end(args);
    ds_poke(*str,'\0');
}


const char* ds_itos(int n)
{
    static char buffer[64];
    snprintf(buffer, sizeof(buffer), "%d",n);
    return buffer;
}
