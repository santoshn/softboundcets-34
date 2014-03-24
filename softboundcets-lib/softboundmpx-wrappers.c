//=== softboundmpx-wrappers.c- SoftBound wrappers for libraries --*- C -*===// 
// Copyright (c) 2014 Santosh Nagarakatte, Milo M. K. Martin. All rights reserved.
//
// Developed by: Santosh Nagarakatte,
//               Department of Computer Science, Rutgers University
//               https://github.com/santoshn/softboundcets-34/
//               http://www.cs.rutgers.edu/~santosh.nagarakatte/
//
//               in collaboration with 
//
//               Milo M.K. Martin, Jianzhou Zhao, Steve Zdancewic
//               Department of Computer and Information Sciences,
//               University of Pennsylvania
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal with the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

//   1. Redistributions of source code must retain the above copyright notice,
//      this list of conditions and the following disclaimers.

//   2. Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimers in the
//      documentation and/or other materials provided with the distribution.

//   3. Neither the names of Santosh Nagarakatte, Milo M. K. Martin,
//      Jianzhou Zhao, Steve Zdancewic, University of Pennsylvania, nor
//      the names of its contributors may be used to endorse or promote
//      products derived from this Software without specific prior
//      written permission.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// WITH THE SOFTWARE.
//===---------------------------------------------------------------------===//

#include <arpa/inet.h>

#if defined(__linux__)
#include<bits/errno.h>
#include<sys/wait.h>
#include <wait.h>
#include <obstack.h>
#include <libintl.h>
#endif

#include<sys/mman.h>
#include<sys/times.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/time.h>
#include<sys/resource.h>
#include<sys/socket.h>
#include<fnmatch.h>
#include <wchar.h>


#include<netinet/in.h>

#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <grp.h>
#include <getopt.h>
#include <glob.h>
#include <limits.h>
#include <math.h>
#include <netdb.h>
#include <pwd.h>
#include <syslog.h>
#include <setjmp.h>
#include <string.h>
#include <signal.h>
#include <stdarg.h>
#include<stdio.h>
#include<stdlib.h>

#include <ttyent.h>
#include <time.h>
#include <unistd.h>
#include <langinfo.h>
#include <regex.h>

#ifdef HAVE_ICONV_H
#include <iconv.h>
#endif

#include <utime.h>
#include <math.h>
#include <locale.h>

#include <fcntl.h>
#include <wctype.h>

#include "softboundmpx.h"

typedef void(*sighandler_t)(int);
typedef void(*void_func_ptr)(void);


__WEAK_INLINE void 
__softboundmpx_read_shadow_stack_metadata_store(char** endptr, int arg_num){
  
  void* nptr_base = __softboundmpx_load_base_shadow_stack(arg_num);
  void* nptr_bound = __softboundmpx_load_bound_shadow_stack(arg_num);
  
  __softboundmpx_metadata_store(endptr, nptr_base, nptr_bound, (*endptr));
}

__WEAK_INLINE void 
__softboundmpx_propagate_metadata_shadow_stack_from(int from_argnum, 
                                                     int to_argnum){
  
  void* base = __softboundmpx_load_base_shadow_stack(from_argnum);
  void* bound = __softboundmpx_load_bound_shadow_stack(from_argnum);
  __softboundmpx_store_base_shadow_stack(base, to_argnum);
  __softboundmpx_store_bound_shadow_stack(bound, to_argnum);

}

__WEAK_INLINE void __softboundmpx_store_null_return_metadata(){

  __softboundmpx_store_base_shadow_stack(NULL, 0);
  __softboundmpx_store_bound_shadow_stack(NULL, 0);

}

__WEAK_INLINE void 
__softboundmpx_store_return_metadata(void* base, void* bound){

  __softboundmpx_store_base_shadow_stack(base, 0);
  __softboundmpx_store_bound_shadow_stack(bound, 0);

}



/* wrappers for library calls (incomplete) */
//////////////////////system wrappers //////////////////////

__WEAK_INLINE int 
softboundmpx_setenv(const char *name, const char *value, int overwrite){

  return setenv(name, value, overwrite);
}

__WEAK_INLINE 
int softboundmpx_unsetenv(const char *name){
  
  return unsetenv(name);
}


__WEAK_INLINE int softboundmpx_system(char* ptr){

  return system(ptr);
}

__WEAK_INLINE int softboundmpx_setreuid(uid_t ruid, uid_t euid) {

  /* tested */
  return setreuid(ruid, euid);
}

__WEAK_INLINE int softboundmpx_mkstemp(char* template) {
  
  /* tested */
  return mkstemp(template);
}

__WEAK_INLINE uid_t softboundmpx_geteuid(){
  return geteuid();
}

__WEAK_INLINE uid_t softboundmpx_getuid(void) {

  /* tested */
  return getuid();
}

__WEAK_INLINE int 
softboundmpx_getrlimit(int resource, struct rlimit* rlim){

  /* tested */
  return getrlimit(resource, rlim);
}

__WEAK_INLINE int 
softboundmpx_setrlimit(int resource, const struct rlimit* rlim){

  /* tested */
  return setrlimit(resource, rlim);
}

__WEAK_INLINE size_t
softboundmpx_fread_unlocked(void *ptr, size_t size, 
                             size_t n, FILE *stream){
  
  return fread_unlocked(ptr, size, n, stream);
}

#if 0
__WEAK_INLINE int 
softboundmpx_fputs_unlocked(const char *s, FILE *stream){
  return fputs_unlocked(s, stream);
}
#endif

__WEAK_INLINE size_t 
softboundmpx_fread(void *ptr, size_t size, size_t nmemb, FILE *stream){
  /* tested */
  return fread(ptr, size, nmemb, stream);
}

__WEAK_INLINE mode_t softboundmpx_umask(mode_t mask) {

  /* tested */
  return umask(mask);
}

__WEAK_INLINE int softboundmpx_mkdir(const char* pathname, mode_t mode){
  
  /* tested */
  return mkdir(pathname, mode);
}

__WEAK_INLINE int softboundmpx_chroot(const char* path){
  /* tested */
  return chroot(path);
}

__WEAK_INLINE int softboundmpx_rmdir(const char* pathname){

  /* tested */
  return rmdir(pathname);
}

__WEAK_INLINE int softboundmpx_stat(const char* path, struct stat* buf){
  /* tested */
  return stat(path, buf);
}

__WEAK_INLINE int softboundmpx_fputc(int c, FILE* stream){

  /* tested */
  return fputc(c, stream);
}

__WEAK_INLINE int softboundmpx_fileno(FILE* stream){

  return fileno(stream);
}

__WEAK_INLINE int softboundmpx_fgetc(FILE* stream){

  return fgetc(stream);
}

__WEAK_INLINE int softboundmpx_ungetc(int c,  FILE* stream){
  
  return ungetc(c, stream);
}

__WEAK_INLINE int 
softboundmpx_strncmp(const char* s1, const char* s2, size_t n){
  return strncmp(s1, s2, n);
}

__WEAK_INLINE double softboundmpx_log(double x) {

  return log(x);
}


__WEAK_INLINE  long long 
softboundmpx_fwrite(char* ptr, size_t size, size_t nmemb, FILE* stream){
  return fwrite(ptr, size, nmemb, stream);
}

__WEAK_INLINE double softboundmpx_atof(char* ptr){
  return atof(ptr);
}

__WEAK_INLINE int softboundmpx_feof(FILE* stream){
  return feof(stream);
}

__WEAK_INLINE int softboundmpx_remove(const char* pathname){

  return remove(pathname);
}

///////////////////// Math library functions here //////////////////

__WEAK_INLINE double softboundmpx_acos(double x) {

  return acos(x);
}

__WEAK_INLINE double softboundmpx_atan2(double y, double x) {

  return atan2(y, x);
}
__WEAK_INLINE float softboundmpx_sqrtf(float x) {

  return sqrtf(x);
}

__WEAK_INLINE float softboundmpx_expf(float x) {

  return expf(x);
}

double exp2(double);

__WEAK_INLINE double softboundmpx_exp2(double x) {

  return exp2(x);
}


__WEAK_INLINE float softboundmpx_floorf(float x) {

  return floorf(x);
}

__WEAK_INLINE double softboundmpx_ceil(double x){

  return ceil(x);
}

__WEAK_INLINE float softboundmpx_ceilf(float x) {

  return ceilf(x);
}
__WEAK_INLINE double softboundmpx_floor(double x) {

  return floor(x);
}

__WEAK_INLINE double softboundmpx_sqrt(double x) {

  return sqrt(x);
}

__WEAK_INLINE double softboundmpx_fabs(double x) {

  return fabs(x);
}

__WEAK_INLINE int softboundmpx_abs(int j){
  return abs(j);
}

__WEAK_INLINE void softboundmpx_srand(unsigned int seed){
  srand(seed);
}

__WEAK_INLINE void softboundmpx_srand48(long int seed){
  srand48(seed);
}


__WEAK_INLINE double softboundmpx_pow(double x, double y) {
  
  return pow(x,y);

}

__WEAK_INLINE float softboundmpx_fabsf(float x) {

  return fabsf(x);
}

__WEAK_INLINE double softboundmpx_tan(double x ) {

  return tan(x);
}

__WEAK_INLINE float softboundmpx_tanf(float x) {

  return tanf(x);
}

__WEAK_INLINE long double softboundmpx_tanl(long double x) {


  return tanl(x);
}

__WEAK_INLINE double softboundmpx_log10(double x) {

  return log10(x);
}
__WEAK_INLINE double softboundmpx_sin(double x) {

  return sin(x);
}

__WEAK_INLINE float softboundmpx_sinf(float x) {

  return sinf(x);
}

__WEAK_INLINE long double softboundmpx_sinl(long double x) {

  return sinl(x);
}

__WEAK_INLINE double softboundmpx_cos(double x) {

  return cos(x);
}

__WEAK_INLINE float softboundmpx_cosf(float x) {

  return cosf(x);
}

__WEAK_INLINE long double softboundmpx_cosl(long double x) {

  return cosl(x);
}

__WEAK_INLINE double softboundmpx_exp(double x) {

  return exp(x);
}

__WEAK_INLINE double softboundmpx_ldexp(double x, int exp) {
  
  return ldexp(x, exp);
}


////////////////File Library Wrappers here //////////////////////

__WEAK_INLINE FILE* softboundmpx_tmpfile(void) {

  void* ret_ptr = tmpfile();
  void* ret_ptr_bound = (char*) ret_ptr + sizeof(FILE);
  __softboundmpx_store_return_metadata(ret_ptr, ret_ptr_bound);
  return ret_ptr;
}

__WEAK_INLINE int softboundmpx_ferror(FILE* stream){

  return ferror(stream);
}

__WEAK_INLINE long softboundmpx_ftell(FILE* stream){ 

  return ftell(stream);
}

__WEAK_INLINE int softboundmpx_fstat(int filedes, struct stat* buff){

  return fstat(filedes, buff);
}

__WEAK_INLINE int softboundmpx___lxstat (int __ver, const char *__filename,     
                                          struct stat *__stat_buf) {

  return __lxstat(__ver, __filename, __stat_buf);
}

__WEAK_INLINE size_t softboundmpx_mbrtowc(wchar_t *pwc, const char *s, 
                                           size_t n, mbstate_t *ps){
  return mbrtowc(pwc, s, n, ps);
}


__WEAK_INLINE int softboundmpx_mbsinit(const mbstate_t *ps){
  return mbsinit(ps);
}



__WEAK_INLINE int softboundmpx___fxstat(int ver, int file_des, struct stat * stat_struct){
  return __fxstat(ver, file_des, stat_struct);
}

__WEAK_INLINE int softboundmpx___fxstatat(int ver, int file_des, const char* filename, struct stat * stat_struct, int flag){
  return __fxstatat(ver, file_des, filename, stat_struct, flag);
}


__WEAK_INLINE int softboundmpx_fflush(FILE* stream){

  return fflush(stream);
}

__WEAK_INLINE int softboundmpx_fputs(const char* s, FILE* stream){
  
  return fputs(s, stream);
}

__WEAK_INLINE int softboundmpx_fsync(int fd){
  return fsync(fd);
}

__WEAK_INLINE DIR* softboundmpx_fdopendir(int fd){
  
  void* ret_ptr = (void*) fdopendir(fd);
  void* ret_ptr_bound = (char *) ret_ptr + 1024 * 1024;
  __softboundmpx_store_return_metadata(ret_ptr, ret_ptr_bound);
  return (DIR*)ret_ptr;
  
}

__WEAK_INLINE int softboundmpx_fseeko(FILE *stream, off_t offset, int whence){
  
  return fseeko(stream, offset, whence);
}

__WEAK_INLINE  char * softboundmpx_mkdtemp(char *template){
  
  char* ret_ptr = mkdtemp(template);
  __softboundmpx_propagate_metadata_shadow_stack_from(1, 0);
  return ret_ptr;
}

__WEAK_INLINE int softboundmpx_raise(int sig){
  return raise(sig);
}

__WEAK_INLINE int softboundmpx_linkat(int olddirfd, const char *oldpath,
                                       int newdirfd, const char *newpath, 
                                       int flags){

  return  linkat(olddirfd, oldpath, newdirfd, newpath, flags);
}

__WEAK_INLINE int softboundmpx_utimes(const char *filename, 
                                       const struct timeval times[2]){
  
  return utimes(filename, times);
}

#if 0
__WEAK_INLINE int softboundmpx_futimesat(int dirfd, const char *pathname,
                                          const struct timeval times[2]){
  
  return futimesat(dirfd, pathname, times);
}
#endif

__WEAK_INLINE int softboundmpx_futimens(int fd, const struct timespec times[2]){

  return futimens(fd, times);
}

__WEAK_INLINE int 
softboundmpx_utimensat(int dirfd, const char *pathname,
                        const struct timespec times[2], int flags){

  return utimensat(dirfd, pathname, times, flags);
}

__WEAK_INLINE size_t 
softboundmpx___ctype_get_mb_cur_max(void){
  return __ctype_get_mb_cur_max();  
}

__WEAK_INLINE int 
softboundmpx_iswprint(wint_t wc){
  return iswprint(wc);
}

__WEAK_INLINE int 
softboundmpx_getpagesize(void){
  return getpagesize();
}

__WEAK_INLINE int softboundmpx_dirfd(DIR *dirp){
  return dirfd(dirp);
}

__WEAK_INLINE struct lconv *softboundmpx_localeconv(void){
  struct lconv* temp = localeconv();

  __softboundmpx_store_return_metadata(temp, temp + 1024);
  
  return temp;
}

__WEAK_INLINE struct tm *softboundmpx_gmtime(const time_t *timep){

  struct tm * temp = gmtime(timep);

  __softboundmpx_store_return_metadata(temp, temp + 1024);

  return temp;
}

__WEAK_INLINE void *
softboundmpx_bsearch(const void *key, const void *base,
                      size_t nmemb, size_t size,
                      int (*compar)(const void *, const void *)){
  
  void* ret_ptr = bsearch(key, base, nmemb, size, compar);

  __softboundmpx_propagate_metadata_shadow_stack_from(2, 0);
    return ret_ptr;

}

__WEAK_INLINE 
struct group *softboundmpx_getgrnam(const char *name){
  void* ret_ptr = getgrnam(name);
  __softboundmpx_store_return_metadata(ret_ptr, (char*) ret_ptr + 1024 * 1024);
  return ret_ptr;  
}

__WEAK_INLINE
int softboundmpx_rpmatch(const char *response){
  return rpmatch(response);
}

__WEAK_INLINE
int softboundmpx_regcomp(regex_t *preg, const char *regex, int cflags){
  return regcomp(preg, regex, cflags);
}

__WEAK_INLINE
size_t softboundmpx_regerror(int errcode, const regex_t *preg, char *errbuf,
                              size_t errbuf_size){

  return regerror(errcode, preg, errbuf, errbuf_size);
}

__WEAK_INLINE
int softboundmpx_regexec(const regex_t *preg, const char *string, 
                          size_t nmatch,
                          regmatch_t pmatch[], int eflags){
  return regexec(preg, string, nmatch, pmatch, eflags);
}


#ifdef HAVE_ICONV_H
__WEAK_INLINE
size_t softboundmpx_iconv(iconv_t cd,
                           char **inbuf, size_t *inbytesleft,
                           char **outbuf, size_t *outbytesleft){

  return iconv(cd, inbuf, inbytesleft, outbuf, outbytesleft);
}

__WEAK_INLINE
iconv_t softboundmpx_iconv_open(const char *tocode, const char *fromcode){

  return iconv_open(tocode, fromcode);
}
#endif




__WEAK_INLINE 
struct passwd * softboundmpx_getpwnam(const char *name){
  void* ret_ptr = getpwnam(name);
  __softboundmpx_store_return_metadata(ret_ptr, (char*) ret_ptr + 1024 * 1024);
  return ret_ptr;    
}


__WEAK_INLINE struct passwd *softboundmpx_getpwuid(uid_t uid){
  void* ret_ptr= getpwuid(uid);

  __softboundmpx_store_return_metadata(ret_ptr, (char*) ret_ptr + 1024 * 1024); 
  return ret_ptr;  
}

__WEAK_INLINE struct group *softboundmpx_getgrgid(gid_t gid){
  
  void* ret_ptr = getgrgid(gid);
  __softboundmpx_store_return_metadata(ret_ptr, (char*) ret_ptr + 1024 * 1024);
  return ret_ptr;  
}




__WEAK_INLINE FILE* softboundmpx_fopen(const char* path, const char* mode){                                  

  void* ret_ptr = (void*) fopen(path, mode);
  void* ret_ptr_bound = (char*) ret_ptr + sizeof(FILE);

  __softboundmpx_store_return_metadata(ret_ptr, ret_ptr_bound); 
  return (FILE*)ret_ptr;
}

__WEAK_INLINE FILE* softboundmpx_fdopen(int fildes, const char* mode){

  void* ret_ptr = (void*) fdopen(fildes, mode);
  void* ret_ptr_bound = (char*) ret_ptr + sizeof(FILE);

  __softboundmpx_store_return_metadata(ret_ptr, ret_ptr_bound);
  return (FILE*)ret_ptr;
}


__WEAK_INLINE int softboundmpx_fseek(FILE* stream, long offset, int whence){

  return fseek(stream, offset, whence);
}

__WEAK_INLINE int softboundmpx_ftruncate(int fd, off_t length) {
  return ftruncate(fd, length);
}



__WEAK_INLINE FILE* softboundmpx_popen(const char* command, const char* type){

  void* ret_ptr = (void*) popen(command, type);
  void* ret_ptr_bound = (char*)ret_ptr + sizeof(FILE);

  __softboundmpx_store_return_metadata(ret_ptr, ret_ptr_bound);
  return (FILE*)ret_ptr;
}

__WEAK_INLINE int softboundmpx_fclose(FILE* fp){
  return fclose(fp);
}


__WEAK_INLINE int softboundmpx_pclose(FILE* stream){

  return pclose(stream);
}

__WEAK_INLINE void softboundmpx_rewind(FILE* stream){ 
  
  rewind(stream);
}

__WEAK_INLINE struct dirent*  softboundmpx_readdir(DIR* dir){

  void* ret_ptr = (void*) readdir(dir);
  void* ret_ptr_bound = (char*)ret_ptr + sizeof(struct dirent);

  __softboundmpx_store_return_metadata(ret_ptr, ret_ptr_bound); 
  return (struct dirent*)ret_ptr;
}

__WEAK_INLINE  int
softboundmpx_creat(const char *pathname, mode_t mode){
  
  return creat(pathname, mode);
}

__WEAK_INLINE int 
softboundmpx_fnmatch(const char *pattern, const char *string, int flags){

  return fnmatch(pattern, string, flags);
}

__WEAK_INLINE DIR* softboundmpx_opendir(const char* name){

  void* ret_ptr = opendir(name);

  /* FIX Required, don't know the sizeof(DIR) */
  void* ret_ptr_bound = (char*) ret_ptr + 1024* 1024;

  __softboundmpx_store_return_metadata(ret_ptr, ret_ptr_bound);

  return (DIR*)ret_ptr;
}

__WEAK_INLINE int softboundmpx_closedir(DIR* dir){

  return closedir(dir);
}

__WEAK_INLINE int 
softboundmpx_rename(const char* old_path, const char* new_path){

  return rename(old_path, new_path);
}


////////////////////unistd.h wrappers ////////////////////////////////

__WEAK_INLINE unsigned int softboundmpx_sleep(unsigned int seconds) {

  return sleep(seconds);
}

__WEAK_INLINE char* softboundmpx_getcwd(char* buf, size_t size){ 

  if(buf == NULL) {
    printf("This case not handled, requesting memory from system\n");
    __softboundmpx_abort();
  }
  
  char* base = (char*)__softboundmpx_load_base_shadow_stack(1);
  char* bound = (char*)__softboundmpx_load_bound_shadow_stack(1);
  
  if (buf < base || buf + size > bound){
    __softboundmpx_printf("[getcwd], overflow in buf in getcwd\n");
    __softboundmpx_abort();
  }
  

  char* ret_ptr = getcwd(buf, size);
  __softboundmpx_propagate_metadata_shadow_stack_from(1, 0);
  return ret_ptr;
}

__WEAK_INLINE int softboundmpx_setgid(gid_t gid){
  return setgid(gid);
}

__WEAK_INLINE gid_t softboundmpx_getgid(void){
  return getgid();
}

__WEAK_INLINE gid_t softboundmpx_getegid(void){
  return getegid();
}
__WEAK_INLINE
int softboundmpx_readlinkat(int dirfd, const char *pathname,
                             char *buf, size_t bufsiz){
  return readlinkat(dirfd, pathname, buf, bufsiz);
}

__WEAK_INLINE
int softboundmpx_renameat(int olddirfd, const char *oldpath,
                           int newdirfd, const char *newpath){
  return renameat(olddirfd, oldpath, newdirfd, newpath);
}

__WEAK_INLINE 
int softboundmpx_unlinkat(int dirfd, const char *pathname, int flags){
  return unlinkat(dirfd, pathname, flags);
}

__WEAK_INLINE
int softboundmpx_symlinkat(const char *oldpath, int newdirfd, 
                            const char *newpath){

  return symlinkat(oldpath, newdirfd, newpath);
}

__WEAK_INLINE
int softboundmpx_mkdirat(int dirfd, const char *pathname, mode_t mode){

  return mkdirat(dirfd, pathname, mode);
}

__WEAK_INLINE
int softboundmpx_fchown(int fd, uid_t owner, gid_t group){
  return fchown(fd, owner, group);
}

__WEAK_INLINE
int softboundmpx_fchownat(int dirfd, const char *pathname,
                           uid_t owner, gid_t group, int flags){
  
  return fchownat(dirfd, pathname, owner, group, flags);
}

__WEAK_INLINE
int softboundmpx_fchmod(int fd, mode_t mode){
  return fchmod(fd, mode);
}

__WEAK_INLINE
int softboundmpx_chmod(const char *path, mode_t mode){
  return chmod(path, mode);
}

__WEAK_INLINE 
int softboundmpx_openat(int dirfd, const char *pathname, int flags){
  return openat(dirfd, pathname, flags);
}

__WEAK_INLINE
int softboundmpx_fchmodat(int dirfd, const char *pathname, 
                           mode_t mode, int flags){
  return fchmodat(dirfd, pathname, mode, flags);
}

#if defined (__linux__)

__WEAK_INLINE 
int softboundmpx___xmknodat (int __ver, int __fd, const char *__path,
                              __mode_t __mode, __dev_t *__dev){
 return  __xmknodat(__ver, __fd, __path, __mode, __dev);
}

__WEAK_INLINE
int softboundmpx_mkfifoat(int dirfd, const char *pathname, mode_t mode){
  return mkfifoat(dirfd, pathname, mode);
}

#endif

__WEAK_INLINE 
pid_t softboundmpx_getpid(void){
  return getpid();
}

__WEAK_INLINE pid_t softboundmpx_getppid(void){
  return getppid();
}


#if 0
__WEAK_INLINE 
int softboundmpx_openat(int dirfd, const char *pathname, int flags, mode_t mode){
  return opennat(dirfd, pathname, flags, mode);
}
#endif



__WEAK_INLINE int 
softboundmpx_chown(const char* path, uid_t owner, gid_t group){  
  return chown(path, owner, group);
  
}

__WEAK_INLINE
wint_t softboundmpx_towlower(wint_t wc){
  return towlower(wc);
}



__WEAK_INLINE int softboundmpx_isatty(int desc) {

  return isatty(desc);
}

__WEAK_INLINE int softboundmpx_chdir(const char* path){
  return chdir(path);
}

__WEAK_INLINE
int softboundmpx_fchdir(int fd){
  return fchdir(fd);
}


///////////////////String related wrappers ////////////////////////////


__WEAK_INLINE int softboundmpx_strcmp(const char* s1, const char* s2){
  
  return strcmp(s1, s2);
}


__WEAK_INLINE int softboundmpx_strcasecmp(const char* s1, const char* s2){

  return strcasecmp(s1,s2);
}

__WEAK_INLINE int 
softboundmpx_strncasecmp(const char* s1, const char* s2, size_t n){
  return strncasecmp(s1, s2, n);
}

__WEAK_INLINE size_t softboundmpx_strlen(const char* s){

  return strlen(s);
}

__WEAK_INLINE char* softboundmpx_strpbrk(const char* s, const char* accept){ 

  char* ret_ptr = strpbrk(s, accept);
  if(ret_ptr != NULL) {

    __softboundmpx_propagate_metadata_shadow_stack_from(1, 0);
  }
  else {

    __softboundmpx_store_null_return_metadata();
  }

  return ret_ptr;
}



__WEAK_INLINE char* softboundmpx_gets(char* s){ 

  printf("[SBCETS] gets used and should not be used\n");
  __softboundmpx_abort();
#if 0
  printf("[Softboundmpx][Warning] Should not use gets\n");
  char* ret_ptr = gets(s);
  __softboundmpx_propagate_metadata_shadow_stack_from(1, 0);
  return ret_ptr;
#endif
  return NULL;

}

__WEAK_INLINE char* softboundmpx_fgets(char* s, int size, FILE* stream){

  char* ret_ptr = fgets(s, size, stream);
  __softboundmpx_propagate_metadata_shadow_stack_from(1,0);

  return ret_ptr;
}


__WEAK_INLINE void softboundmpx_perror(const char* s){
  perror(s);
}

__WEAK_INLINE size_t softboundmpx_strspn(const char* s, const char* accept){
  
  return strspn(s, accept);
}

__WEAK_INLINE size_t softboundmpx_strcspn(const char* s, const char* reject){
  
  return strcspn(s, reject);
}

#ifdef _GNU_SOURCE

__WEAK_INLINE void* softboundmpx_mempcpy(void * dest, const void * src, size_t n){

  // IMP: need to copy the metadata 
  void* ret_ptr = mempcpy(dest, src, n);
  __softboundmpx_propagate_metadata_shadow_stack_from(1,0);
  return ret_ptr;
}

#endif

__WEAK_INLINE int 
softboundmpx_memcmp(const void* s1, const void* s2, size_t n){
  return memcmp(s1, s2, n);
}

#ifdef _GNU_SOURCE

__WEAK_INLINE void* softboundmpx_memrchr(const void * s, int c, size_t n){  
  void* ret_ptr = memrchr(s, c, n);
  if(ret_ptr != NULL) {
    __softboundmpx_propagate_metadata_shadow_stack_from(1, 0);
  }
  else{
    __softboundmpx_store_null_return_metadata();
  }
  return ret_ptr;
}
#endif

__WEAK_INLINE void softboundmpx_rewinddir(DIR *dirp){
  rewinddir(dirp);  
}



__WEAK_INLINE void* softboundmpx_memchr(const void * s, int c, size_t n){  
  void* ret_ptr = memchr(s, c, n);
  if(ret_ptr != NULL) {
    __softboundmpx_propagate_metadata_shadow_stack_from(1, 0);
  }
  else{
    __softboundmpx_store_null_return_metadata();
  }
  return ret_ptr;
}

__WEAK_INLINE char* softboundmpx_rindex(char* s, int c){

  char* ret_ptr = rindex(s,c);
  __softboundmpx_propagate_metadata_shadow_stack_from(1,0);
  return ret_ptr;
}

__WEAK_INLINE ssize_t 
softboundmpx___getdelim(char **lineptr, size_t *n, int delim, FILE *stream){
  
  int metadata_prop = 1;
  if(*lineptr == NULL){
    metadata_prop = 0;
  }

  ssize_t ret_val = getdelim(lineptr, n, delim, stream);
  
  if(metadata_prop){
    __softboundmpx_read_shadow_stack_metadata_store(lineptr, 1);
  }
  else{
    __softboundmpx_store_return_metadata(*lineptr, 
					 (*lineptr) + strlen(*lineptr));
  }
  
  return ret_val;
  

}

__WEAK_INLINE unsigned long int 
softboundmpx_strtoul(const char* nptr, char ** endptr, int base){

  unsigned long temp = strtoul(nptr, endptr, base);
  if(endptr != NULL){
    __softboundmpx_read_shadow_stack_metadata_store(endptr, 1);
    
  }

  return temp;
}

__WEAK_INLINE double softboundmpx_strtod(const char* nptr, char** endptr){

  double temp = strtod(nptr, endptr);
  
  if(endptr != NULL) {
    __softboundmpx_read_shadow_stack_metadata_store(endptr, 1);
  }
  return temp;
 }
 
__WEAK_INLINE long 
softboundmpx_strtol(const char* nptr, char **endptr, int base){
 
   long temp = strtol(nptr, endptr, base);
   if(endptr != NULL) {
     //    __softboundmpx_printf("*endptr=%p\n", *endptr);
     __softboundmpx_read_shadow_stack_metadata_store(endptr, 1);
  }
  return temp;
}

#ifdef _GNU_SOURCE

__WEAK_INLINE char* softboundmpx_strchrnul(const char* s, int c){

  char* ret_ptr = strchrnul(s, c);
   __softboundmpx_propagate_metadata_shadow_stack_from(1, 0);
   return ret_ptr;
}
#endif

__WEAK_INLINE char* softboundmpx_strchr(const char* s, int c){

  char* ret_ptr = strchr(s, c);
   __softboundmpx_propagate_metadata_shadow_stack_from(1, 0);
   return ret_ptr;
}

__WEAK_INLINE char* softboundmpx_strrchr(const char* s, int c){

  char* ret_ptr = strrchr(s, c);
  __softboundmpx_propagate_metadata_shadow_stack_from(1, 0);
  return ret_ptr;
}

__WEAK_INLINE char* softboundmpx_stpcpy(char* dest, char* src){

  void* ret_ptr = stpcpy(dest, src);
  __softboundmpx_propagate_metadata_shadow_stack_from(1, 0);
  return ret_ptr;
}

__WEAK_INLINE char* softboundmpx_strcpy(char* dest, char* src){
  
  char* dest_base = __softboundmpx_load_base_shadow_stack(1);
  char* dest_bound = __softboundmpx_load_bound_shadow_stack(1);

  char* src_base = __softboundmpx_load_base_shadow_stack(2);
  char* src_bound = __softboundmpx_load_bound_shadow_stack(2);

  /* There will be an out-of-bound read before we trigger an error as
     we currently use strlen. Can either (dest + size) or (src + size)
     overflow?
  */
  size_t size = strlen(src);
  if(dest < dest_base || (dest > dest_bound - size -1) || (size > (size_t) dest_bound)){
    printf("[strcpy] overflow in strcpy with dest\n");
    __softboundmpx_abort();
  }  
  if(src < src_base || (src > src_bound -size -1) || (size > (size_t) src_bound)){
    printf("[strcpy] overflow in strcpy with src\n");
    __softboundmpx_abort();
  }

  void * ret_ptr = strcpy(dest, src);
  __softboundmpx_propagate_metadata_shadow_stack_from(1, 0);
  return ret_ptr;
}


__WEAK_INLINE void softboundmpx_abort() {
  abort();
}

 __WEAK_INLINE int softboundmpx_rand() {
   return rand();
 }
 
 /////////////////* TODO *///////////////////////////

 __WEAK_INLINE int softboundmpx_atoi(const char* ptr){

  if(ptr == NULL) {
    __softboundmpx_abort();
  }
  return atoi(ptr);    
 }
 
 __WEAK_INLINE void softboundmpx_puts(char* ptr){
   puts(ptr);
 }


__WEAK_INLINE void softboundmpx_exit(int status) {
  
  exit(status);
}

__WEAK_INLINE char*  softboundmpx_strtok(char* str, const char* delim){
  
  char* ret_ptr = strtok(str, delim);   
  size_t value = 1;
  value = value <<48;
  __softboundmpx_store_return_metadata((void*)0, (void*)(value));
  return ret_ptr;
}

__WEAK_INLINE void __softboundmpx_strdup_handler(void* ret_ptr){

  if(ret_ptr == NULL) {
    __softboundmpx_store_null_return_metadata();
  }
  else {
    //    printf("strndup malloced pointer %p\n", ret_ptr);    
    __softboundmpx_store_return_metadata(ret_ptr, 
                                          (void*)
					 ((char*)ret_ptr + strlen(ret_ptr) + 1)); 
  } 
}

//strdup, allocates memory from the system using malloc, thus can be freed
__WEAK_INLINE char* softboundmpx_strndup(const char* s, size_t n){
  
  /* IMP: strndup just copies the string s */  
  char* ret_ptr = strndup(s, n);
  __softboundmpx_strdup_handler(ret_ptr);  
  return ret_ptr;
 }


//strdup, allocates memory from the system using malloc, thus can be freed
__WEAK_INLINE char* softboundmpx_strdup(const char* s){
  
  /* IMP: strdup just copies the string s */  
  void* ret_ptr = strdup(s);
  
  __softboundmpx_strdup_handler(ret_ptr);
  return ret_ptr;
 }

__WEAK_INLINE char* softboundmpx___strdup(const char* s){

  void* ret_ptr = strdup(s);
  __softboundmpx_strdup_handler(ret_ptr);
  return ret_ptr;
}


 __WEAK_INLINE char* softboundmpx_strcat (char* dest, const char* src){

   // TODO: Look at uncommenting the check 
#if 0
  if(dest + strlen(dest) + strlen(src) > dest_bound){
    printf("overflow with strcat, dest = %p, strlen(dest)=%d, 
            strlen(src)=%d, dest_bound=%p \n", 
           dest, strlen(dest), strlen(src), dest_bound);
    __softboundmpx_abort();
  } 
#endif
  
  char* ret_ptr = strcat(dest, src);
  __softboundmpx_propagate_metadata_shadow_stack_from(1, 0);
  return ret_ptr;
}

__WEAK_INLINE char* 
softboundmpx_strncat (char* dest,const char* src, size_t n){

  char* ret_ptr = strncat(dest, src, n);
  __softboundmpx_propagate_metadata_shadow_stack_from(1, 0);
  return ret_ptr;
}

__WEAK_INLINE char* 
softboundmpx_strncpy(char* dest, char* src, size_t n){
 

  char* dest_base = __softboundmpx_load_base_shadow_stack(1);
  char* dest_bound = __softboundmpx_load_bound_shadow_stack(1);

  char* src_base = __softboundmpx_load_base_shadow_stack(2);
  char* src_bound = __softboundmpx_load_bound_shadow_stack(2);

  /* Can either (dest + n) or (src + n) overflow? */
  if(dest < dest_base || (dest > dest_bound - n) || (n > (size_t) dest_bound)){
    printf("[strncpy] overflow in strncpy with dest\n");
    __softboundmpx_abort();
  }  
  if(src < src_base || (src > src_bound -n) || (n > (size_t) src_bound)){
    __softboundmpx_abort();
  }

  char* ret_ptr = strncpy(dest, src, n);
  __softboundmpx_propagate_metadata_shadow_stack_from(1, 0);
  return ret_ptr;
}

__WEAK_INLINE char* 
softboundmpx_strstr(const char* haystack, const char* needle){
  
  char* ret_ptr = strstr(haystack, needle);
  if(ret_ptr != NULL) {    
    __softboundmpx_propagate_metadata_shadow_stack_from(1, 0);
  }
  else {
    __softboundmpx_store_null_return_metadata();
  }
  return ret_ptr;
}

__WEAK_INLINE sighandler_t 
softboundmpx_signal(int signum, sighandler_t handler){

  sighandler_t ptr = signal(signum, handler);
  __softboundmpx_store_return_metadata((void*)ptr, (void*) ptr); 
  return ptr;
}

__WEAK_INLINE clock_t softboundmpx_clock(void){
  return clock();
}


__WEAK_INLINE long softboundmpx_atol(const char* nptr){ 
  return atol(nptr);
}

__WEAK_INLINE void* softboundmpx_realloc(void* ptr, size_t size){
  

  /* TODO: may be necessary to copy metadata */

   void* ret_ptr = realloc(ptr, size);

   __softboundmpx_store_return_metadata(ret_ptr, 
					(char*)(ret_ptr) + size);

   if(ret_ptr != ptr){
     __softboundmpx_copy_metadata(ret_ptr, ptr, size);
   }
   
   return ret_ptr;
 }

 __WEAK_INLINE void* softboundmpx_calloc(size_t nmemb, size_t size) {
   
  
   void* ret_ptr = calloc(nmemb, size);   
   if(ret_ptr != NULL) {    
     
     __softboundmpx_store_return_metadata(ret_ptr, 
					  ((char*)(ret_ptr) + (nmemb * size)));
   }
   else{
     __softboundmpx_store_null_return_metadata();
   } 
   return ret_ptr;
 }

__WEAK_INLINE void* softboundmpx_mmap(void* addr, size_t length, 
                                       int prot, int flags, int fd, 
                                       off_t offset){


  char* ret_ptr = mmap(addr, length, prot, flags, fd, offset);
  if(ret_ptr == (void*) -1){
    __softboundmpx_store_null_return_metadata();
  }
  else{

    char* ret_bound = ret_ptr + length;
    __softboundmpx_store_return_metadata(ret_ptr, ret_bound); 
  }
  return ret_ptr;
}
 
__WEAK_INLINE void* softboundmpx_malloc(size_t size) {
  
  char* ret_ptr = (char*)malloc(size);
  if(ret_ptr == NULL){
    __softboundmpx_store_null_return_metadata();
  }
  else{

    char* ret_bound = ret_ptr + size;
    __softboundmpx_store_return_metadata(ret_ptr, ret_bound); 
  }
  return ret_ptr;
}


__WEAK_INLINE int softboundmpx_putchar(int c) {

  return putchar(c);
}


 __WEAK_INLINE clock_t softboundmpx_times(struct tms* buf){
  return times(buf);
}

__WEAK_INLINE size_t 
softboundmpx_strftime(char* s, size_t max, 
                       const char* format, const struct tm *tm){
  
  return strftime(s, max, format, tm);
}


__WEAK_INLINE time_t softboundmpx_mktime(struct tm *tm){
  return mktime(tm);
}

__WEAK_INLINE long softboundmpx_pathconf(char *path, int name){
  return pathconf(path, name);
}


__WEAK_INLINE struct tm* softboundmpx_localtime(const time_t* timep){

  struct tm * ret_ptr = localtime(timep);
  __softboundmpx_store_return_metadata(ret_ptr, 
				       (char*)ret_ptr + sizeof(struct tm));
  return ret_ptr;
}

 __WEAK_INLINE time_t softboundmpx_time(time_t* t){

   return time(t);
 }

__WEAK_INLINE double softboundmpx_drand48(){

  return drand48();
}

__WEAK_INLINE void softboundmpx_free(void* ptr){
   free(ptr);
}


__WEAK_INLINE long int softboundmpx_lrand48(){
  return lrand48();
}


/* ////////////////////Time Related Library Wrappers///////////////////////// */


__WEAK_INLINE char* softboundmpx_ctime( const time_t* timep){
  
  char* ret_ptr = ctime(timep);

  if(ret_ptr == NULL){
    __softboundmpx_store_null_return_metadata();
  }
  else {
    __softboundmpx_store_return_metadata(ret_ptr, ret_ptr + strlen(ret_ptr) + 1);
  }
  return ret_ptr;

}



__WEAK_INLINE double softboundmpx_difftime(time_t time1, time_t time0) {  
  return difftime(time1, time0);
}

__WEAK_INLINE int softboundmpx_toupper(int c) {

  return toupper(c);
}

__WEAK_INLINE int softboundmpx_tolower(int c){

  return tolower(c);
}

__WEAK_INLINE void softboundmpx_setbuf(FILE* stream, char* buf){  
  setbuf(stream, buf);
}

__WEAK_INLINE char* softboundmpx_getenv(const char* name){
   
  char* ret_ptr = getenv(name);
   
  if(ret_ptr != NULL){
    __softboundmpx_store_return_metadata(ret_ptr, 
					 ret_ptr + strlen(ret_ptr) + 1);
  }
  else {
    __softboundmpx_store_null_return_metadata();
  }

  return ret_ptr;
}

__WEAK_INLINE int softboundmpx_atexit(void_func_ptr function){
  return atexit(function);

}

#ifdef _GNU_SOURCE
__WEAK_INLINE char* softboundmpx_strerror_r(int errnum, char* buf, 
                                             size_t buf_len) {

  void* ret_ptr = strerror_r(errnum, buf, buf_len);
  __softboundmpx_store_return_metadata(ret_ptr, 
                                        (void*)
				       ((char*)ret_ptr + strlen(ret_ptr) +1));
  return ret_ptr;
}
#endif

__WEAK_INLINE char* softboundmpx_strerror(int errnum) {

  void* ret_ptr = strerror(errnum);
  __softboundmpx_store_return_metadata(ret_ptr, 
                                        (void*)
				       ((char*)ret_ptr + strlen(ret_ptr) +1));
  return ret_ptr;
}


__WEAK_INLINE int softboundmpx_unlink(const char* pathname){
  return unlink(pathname);
}


__WEAK_INLINE int softboundmpx_close(int fd) {

  return close(fd);
}


__WEAK_INLINE int softboundmpx_open(const char *pathname, int flags){  
  return open(pathname, flags);

}

__WEAK_INLINE ssize_t softboundmpx_read(int fd, void* buf, size_t count){
  
  return read(fd, buf, count);
}

__WEAK_INLINE ssize_t softboundmpx_write(int fd, void* buf, size_t count){
  return write(fd, buf, count);
}


__WEAK_INLINE off_t softboundmpx_lseek(int fildes, off_t offset, int whence) {

  return lseek(fildes, offset, whence);
}


__WEAK_INLINE int 
softboundmpx_gettimeofday(struct timeval* tv, struct timezone* tz){
  return gettimeofday(tv, tz);
}


__WEAK_INLINE int 
softboundmpx_select(int nfds, fd_set* readfds, fd_set* writefds,
                     fd_set* exceptfds, struct timeval* timeout){
  return select(nfds, readfds, writefds, exceptfds, timeout);
}

#if defined (__linux__)

__WEAK_INLINE char* 
softboundmpx_setlocale(int category, const char* locale){
  
  void* ret_ptr = setlocale(category, locale);
  __softboundmpx_store_return_metadata(ret_ptr, 
                                        (void*) 
				       ((char*) ret_ptr+ strlen(ret_ptr)));
  return ret_ptr;  

}

__WEAK_INLINE char*
softboundmpx_textdomain(const char* domainname){
  
  void* ret_ptr = textdomain(domainname);
  __softboundmpx_store_return_metadata(ret_ptr,
                                        (void *)
				       ((char*) ret_ptr + strlen(ret_ptr)));
  
  return ret_ptr;
  
}


__WEAK_INLINE char*
softboundmpx_bindtextdomain(const char* domainname, const char* dirname){
  
  void* ret_ptr = bindtextdomain(domainname, dirname);
  __softboundmpx_store_return_metadata(ret_ptr,
                                        (void *)
				       ((char*) ret_ptr + strlen(ret_ptr)));

  return ret_ptr;
}

__WEAK_INLINE char * 
softboundmpx_gettext(const char * msgid){
  
  void* ret_ptr = gettext(msgid);
  __softboundmpx_store_return_metadata(ret_ptr,
                                        (void*)
				       ((char*) ret_ptr + strlen(ret_ptr)));

  return ret_ptr;  

}


__WEAK_INLINE char * 
softboundmpx_dcngettext (const char * domainname,
                          const char * msgid, const char * msgid_plural,
                          unsigned long int n, int category){
  
  void* ret_ptr = dcngettext(domainname, msgid, msgid_plural, n, category);
  __softboundmpx_store_return_metadata(ret_ptr,
                                        (void*)
				       ((char*) ret_ptr + strlen(ret_ptr)));

  return ret_ptr;  
    
}


/* IMP: struct hostent may have pointers in the structure being returned,
   we need to store the metadata for all those pointers */
__WEAK_INLINE 
struct hostent * softboundmpx_gethostbyname(const char *name){
  
  struct hostent * ret_ptr = gethostbyname(name);

  void* ret_bound = ret_ptr + sizeof(struct hostent);
  __softboundmpx_store_return_metadata(ret_ptr,
				       ret_bound);
  
  return ret_ptr;  
}



__WEAK_INLINE char*
softboundmpx_dcgettext (const char * domainname, 
                         const char * msgid,
                         int category) {

  void* ret_ptr = dcgettext(domainname, msgid, category);
  __softboundmpx_store_return_metadata(ret_ptr,
                                        (void*)
				       ((char*) ret_ptr + strlen(ret_ptr)));
  return ret_ptr;  
  
}

#endif

#if defined(__linux__)
__WEAK_INLINE int* softboundmpx___errno_location() {
  void* ret_ptr = (int *)__errno_location();
  //  printf("ERRNO: ptr is %lx", ptrs->ptr);
  __softboundmpx_store_return_metadata(ret_ptr, 
				       (void*)((char*)ret_ptr + sizeof(int*)));
  
  return ret_ptr;
}

__WEAK_INLINE unsigned short const** 
softboundmpx___ctype_b_loc(void) {

  unsigned short const** ret_ptr =__ctype_b_loc();
  __softboundmpx_store_return_metadata((void*) ret_ptr, 
                                        (void*)
				       ((char*) ret_ptr + sizeof(int*)));
  return ret_ptr;
}

__WEAK_INLINE int const**  softboundmpx___ctype_toupper_loc(void) {
  
  int const ** ret_ptr  =  __ctype_toupper_loc();  
  __softboundmpx_store_return_metadata((void*) ret_ptr, 
                                        (void*)
				       ((char*)ret_ptr + sizeof(int*)));
  return ret_ptr;

}


__WEAK_INLINE int const**  softboundmpx___ctype_tolower_loc(void) {
  
  int const ** ret_ptr  =  __ctype_tolower_loc();  
  __softboundmpx_store_return_metadata((void*) ret_ptr, 
				       (void*) ((char*)ret_ptr + sizeof(int*)));
  return ret_ptr;

}
#endif

/* This is a custom implementation of qsort */

static int 
compare_elements_helper(void* base, size_t element_size, 
                        int idx1, int idx2, 
                        int (*comparer)(const void*, const void*)){
  
  char* base_bytes = base;
  return comparer(&base_bytes[idx1 * element_size], 
                  &base_bytes[idx2*element_size]);
}

#define element_less_than(i,j) (compare_elements_helper(base, element_size, (i), (j), comparer) < 0)

static void 
exchange_elements_helper(void* base, size_t element_size, 
                         int idx1, int idx2){

  char* base_bytes = base;
  size_t i;

  for (i=0; i < element_size; i++){
    char temp = base_bytes[idx1* element_size + i];
    base_bytes[idx1 * element_size + i] = base_bytes[idx2 * element_size + i];
    base_bytes[idx2 * element_size + i] = temp;
  }  

  for(i=0; i < element_size; i+= 8){
    void* base_idx1;
    void* bound_idx1;

    void* base_idx2;
    void* bound_idx2;

    char* addr_idx1 = &base_bytes[idx1 * element_size + i];
    char* addr_idx2 = &base_bytes[idx2 * element_size + i];

    //    printf("addr_idx1= %p, addr_idx2=%p\n", addr_idx1, addr_idx2);

    __softboundmpx_metadata_load(addr_idx1, &base_idx1, &bound_idx1);
    __softboundmpx_metadata_load(addr_idx2, &base_idx2, &bound_idx2);

    __softboundmpx_metadata_store(addr_idx1, base_idx2, bound_idx2, *((void**)addr_idx1));
    __softboundmpx_metadata_store(addr_idx2, base_idx1, bound_idx1, *((void**)addr_idx2));
        
  }

}

#define exchange_elements(i,j) (exchange_elements_helper(base, element_size, (i), (j)))

#define MIN_QSORT_LIST_SIZE 32

__WEAK__ 
void my_qsort(void* base, size_t num_elements, 
              size_t element_size, 
              int (*comparer)(const void*, const void*)){

  size_t i;

  for(i = 0; i < num_elements; i++){
    int j;
    for (j = i - 1; j >= 0; j--){      
      if(element_less_than(j, j + 1)) break;
      exchange_elements(j, j + 1);
    }
  }
  /* may be implement qsort here */

}


__WEAK_INLINE void 
softboundmpx_qsort(void* base, size_t nmemb, size_t size, 
                    int (*compar)(const void*, const void*)){

  my_qsort(base, nmemb, size, compar);
}

#if defined(__linux__)

__WEAK_INLINE 
void softboundmpx__obstack_newchunk(struct obstack *obj, int b){
  
  _obstack_newchunk(obj, b);
}

__WEAK_INLINE
int softboundmpx__obstack_begin(struct obstack * obj, int a, int b, 
                                 void *(foo) (long), void (bar) (void *)){
  return _obstack_begin(obj, a, b, foo, bar);
}

__WEAK_INLINE
void softboundmpx_obstack_free(struct obstack *obj, void *object){
  obstack_free(obj, object);
}


__WEAK_INLINE 
char * softboundmpx_nl_langinfo(nl_item item){
  
  char* ret_ptr = nl_langinfo(item);

  __softboundmpx_store_return_metadata(ret_ptr, 
				       ret_ptr + 1024 * 1024);
  
  return ret_ptr;
}

__WEAK_INLINE
int softboundmpx_clock_gettime(clockid_t clk_id, struct timespec *tp){
  return clock_gettime(clk_id, tp);
}

#endif

#if 0

int softboundmpx__obstack_memory_used(struct obstack *h){
  return _obstack_memory_used(h);
}

#endif
