#include "qjdproc.h"

extern bool flag_devel;

#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>

char *strchar(const char *s,int k);
int mini_sscanf(const char *s1,const char *fmt, ...)
{
    va_list va;
    va_start(va,fmt);
    char *s=(char*)s1; // *** for gcc 4.4 特地如次写的？
    char *p;
    int k=0; // count
    while (*fmt and *s) 	//	if (*fmt==0 or *s==0) break;
    {
        if (*fmt=='%')
        {
            int n;
            if (fmt[1]=='S')
            {
                //if(fmt[2]!=0)
                p=strchr(s,fmt[2]); //get the token
                int len=p-s;
                p=va_arg(va,char*);
                strncpy(p,s,len);
                p[len]=0;
                s+=len+1; fmt+=3; k++;
            }
            else if (fmt[1]=='s')  // extract string
            {
                //printf("%s : %%s =%s \n",__FUNCTION__,s);
                k+=sscanf(s,"%s%n",va_arg(va,char *),&n);
                s+=n; fmt+=2;
            }
            else if (fmt[1]=='*' && fmt[2]=='s') // skip
            {
                p=strchr(s,' '); //0x20,0x00,0x0A, 123 435 54054
                if(p==0)
                {
                    //	printf("%s : %c [%s] \n",__FUNCTION__,fmt[1],s);
                    break;
                }
                s=p; fmt+=3;
            }
            else if (fmt[1]=='c')
            {
                p=va_arg(va,char*);
                //	printf("%s : %c [%c] \n",__FUNCTION__,fmt[1],*s);
                *p=*s;
                s++; k++; fmt+=2;
            }
            else if (fmt[1]=='f')
            {
                k+=sscanf(s,"%f%n",va_arg(va,float*),&n);
                s+=n; fmt+=2;
            }
            else if (fmt[1]=='d')
            {
                k+=sscanf(s,"%d%n",va_arg(va,int*),&n);
                s+=n; fmt+=2;
            }
            else if (fmt[1]=='u')
            {
                k+=sscanf(s,"%u%n",va_arg(va,int*),&n);
                s+=n; fmt+=2;
            }
            else if (fmt[1]=='l' && fmt[2]=='d')
            {
                k+=sscanf(s,"%ld%n",va_arg(va,long*),&n);
                s+=n; fmt+=3;
            }
            else if (fmt[1]=='l' && fmt[2]=='u')
            {
                k+=sscanf(s,"%lu%n",va_arg(va,long*),&n);
                s+=n; fmt+=3;
            }
            else if (fmt[1]=='l' && fmt[2]=='g')
            {
                k+=sscanf(s,"%lg%n",va_arg(va,double*),&n);
                s+=n; fmt+=3;
            }
            else
            {
                printf("%s: unsupported"" format '%c'",__FUNCTION__,fmt[1]);
                break;
            }
        }
        else
        {
            if (isspace(*fmt)) // isblank
            {
                while (isspace(*s)) s++;
                fmt++;
            }
            else
            {
                char sstr[32];
                int n=strcspn(fmt," %\n"); // find delimiters noSEGFAULT
                strncpy(sstr,fmt,n); sstr[n]=0;
                p=strstr(s,sstr);
                if(p==0)
                {
                    if(0 and flag_devel) printf("%s : can't found [%s] in %s\n",__FUNCTION__,sstr,s);
                    break;
                }
                s=p+n;
                fmt+=n;
                // ddd at %s, at%s
                // printf("opp_vsscanf: unexpected ""char in format '%s'",fmt);
                //break;
                //return k;
            }
        }

        if (*fmt=='\0' || *fmt=='#' or *s==0)
            break;
    }
    va_end(va);
    return k;
}


