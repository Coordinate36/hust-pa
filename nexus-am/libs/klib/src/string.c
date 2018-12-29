#include "klib.h"

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t len;
  for (len = 0; s[len] != '\0'; len++);
  return len;
}

char *strcpy(char* dst,const char* src) {
  assert(dst != NULL && src != NULL);
  char* r = dst;
  while ((*r++ = *src++) != '\0');
  return dst;
}

char* strncpy(char* dst, const char* src, size_t n) {
  assert(dst != NULL && src != NULL);
  size_t i;
  for (i = 0; i < n && src[i] != '\0'; i++) {
    dst[i] = src[i];
  }
  while (i < n) {
    dst[i++] = '\0';
  }
  return dst;
}

char* strcat(char* dst, const char* src) {
  assert(dst != NULL && src != NULL);
  char* s;
  for (s = dst; *s != '\0'; s++);
  while ((*s++ = *src++) != '\0');
  return dst;
}

int strcmp(const char* s1, const char* s2) {
  assert(s1 != NULL && s2 != NULL);
  size_t i;
  for (i = 0; s1[i] != '\0' && s1[i] == s2[i]; i++);
  if (s1[i] > s2[i]) {
    return 1;
  }
  if (s1[i] < s2[i]) {
    return -1;
  }
  return 0;
}

int strncmp(const char* s1, const char* s2, size_t n) {
  assert(s1 != NULL && s2 != NULL);
  size_t i;
  n--;
  for (i = 0; s1[i] != '\0' && s1[i] == s2[i] && i < n; i++);
  if (s1[i] > s2[i]) {
    return 1;
  }
  if (s1[i] < s2[i]) {
    return -1;
  }
  return 0;
}

void* memset(void* v,int c,size_t n) {
  unsigned char* s = (unsigned char*)v;
  for (int i = 0; i < n; i++) {
    s[i] = c;
  }
  return v;
}

void* memcpy(void* out, const void* in, size_t n) {
  unsigned long long* dst = (unsigned long long*)out;
  unsigned long long* src = (unsigned long long*)in;
  size_t num_8 = n >> 3;
  int i;
  for (i = 0; i < num_8; i++) {
    dst[i] = src[i];
  }
  num_8 <<= 3;
  unsigned char* d = (unsigned char*)out + num_8;
  unsigned char* s = (unsigned char*)in + num_8;
  n -= num_8;
  for (i = 0; i < n; i++) {
    d[i] = s[i];
  }
  return out;
}

int memcmp(const void* s1, const void* s2, size_t n) {
  unsigned char* p1 = (unsigned char*)s1;
  unsigned char* p2 = (unsigned char*)s2;
  size_t i;
  n--;
  for (i = 0; p1[i] != '\0' && p1[i] == p2[i] && i < n; i++);
  if (p1[i] > p2[i]) {
    return 1;
  }
  if (p1[i] < p2[i]) {
    return -1;
  }
  return 0;
}

#endif
