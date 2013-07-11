#ifndef LWQQ_WIN32_H_H
#define LWQQ_WIN32_H_H

char *  strsep(char **stringp, const char* delim);
double drand48(void);

int pipe (int filedes [2]);

#define mkdir(a,b) mkdir(a)

#endif