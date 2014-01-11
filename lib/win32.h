#ifndef LWQQ_WIN32_H_H
#define LWQQ_WIN32_H_H
//force let time return a long int
#define time(a) (long int)time(a)

double drand48(void);
void srand48(unsigned int i);

#define mkdir(a,b) mkdir(a)

#endif