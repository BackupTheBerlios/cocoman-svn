/*
 * Soapbox - A way to deny processes to write files outside some directories
 *
 * Copyright (C) 2003 by Dag Wieers <dag@wieers.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * Functions that we need to implement:
 *
 * ACTION halt,error
 *   chown, chmod, link, mkdir, mknod, open, open64,
 *   rename, rmdir, symlink, unlink, utime
 *
 * ACTION warn
 *   fprintf, fscanf, read, write
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <utime.h>
#include <fcntl.h>
#include <limits.h>

#include <time.h>

static int (*_real_chmod)	(const char *, mode_t);
//static int (*_real_fchmod)	(int, mode_t);

static int (*_real_chown)	(const char *, uid_t, gid_t);
//static int (*_real_fchown)	(int, uid_t, gid_t);
static int (*_real_lchown)	(const char *, uid_t, gid_t);


// natan (stop calls to fork, vfork, execve, socketcall and clone)
static int (*_real_clone)       (int, int , void*);
static int (*_real_fork)        (void);
static int (*_real_vfork)        (void);
static int (*_real_execve)        (const char*, char *const[], char *const[]);
static int (*_real_socketcall)        (int, unsigned long *);
static int (*_real_socket)        (int, int, int);

static int (*_real_link)	(const char *, const char *);
static int (*_real_mkdir)	(const char *, mode_t);
static int (*_real_mkfifo)	(const char *, mode_t);

static int (*_real_mknod)	(const char *, mode_t, dev_t);
static int (*_real___xmknod)	(int, const char *, mode_t, dev_t *);

static int (*_real_open)	(const char *, int, ...);
static int (*_real_open64)	(const char *, int, ...);
static int (*_real_creat)	(const char *, mode_t);
static int (*_real_creat64)	(const char *, mode_t);

//static FILE (*_real_fopen)	(const char *, const char *);
//static FILE (*_real_fdopen)	(int, const char *);
//static FILE (*_real_freopen)	(const char *, const char *, FILE *);

static int (*_real_remove)	(const char *);
static int (*_real_rename)	(const char *, const char *);
static int (*_real_rmdir)	(const char *);
static int (*_real_symlink)	(const char *, const char *);
static int (*_real_unlink)	(const char *);
static int (*_real_utime)	(const char *, const struct utimbuf *);
static int (*_real_utimes)	(const char *, const struct timeval *);

// Allowed path seperators
#define PATHSEP ":,;|"

// Rewrite as a file or as a link ?
enum { R_FILE, R_LINK };

// List of actions
enum { A_UNKN, A_WARN, A_ERR, A_HALT };
static char *(actions[]) = { "warn", "err", "halt", };

// Default action (TODO: create array of strings for the different actions !)
#define DEFAULT_ACTION actions[0]
#define A_DEFAULT A_WARN

static char *soapboxpath;
static char *soapboxrun;
static int devnull, action=A_UNKN, soapboxdebug=0;
static FILE *stdlog;

// Print error message and die
static void sb_die(FILE *out, const char *str, ...) {
	va_list argptr;
	fprintf(out, "soapbox: ");
	va_start(argptr, str);
	vfprintf(out, str, argptr);
	va_end(argptr);
	fprintf(out, "\n");
	exit(-1);
}

// Print error message with optional debug level
static void sb_log(int level, const char *str, ...) {
	va_list argptr;
	if (soapboxdebug & level || level==0) {
		if (level!=0)
			fprintf(stdlog, "soapbox: debug%i: ", level);
		else
			fprintf(stdlog, "soapbox: ");
		va_start(argptr, str);
		vfprintf(stdlog, str, argptr);
		va_end(argptr);
		fprintf(stdlog, "\n");
	}
}

// Check of dlsym worked
static void dlcheck(const char *err) {
//	sb_log(0,"TEST: --%s--\n", err);
	if (err!=NULL) sb_die(stdlog, "%s", err);
}

void _init(int argc, char *argv[]) {
	char *soapboxaction, *soapboxdbg, *soapboxlog;
	// Make output unbuffered
	setvbuf(stdout, (char *) NULL, _IONBF, 0);
	setvbuf(stderr, (char *) NULL, _IONBF, 0);

	// Handle and unset logging environment variable
	soapboxlog=getenv("SOAPBOXLOG"); // unsetenv("SOAPBOXLOG");
	if (!soapboxlog || *soapboxlog=='\0') {
		stdlog=stderr;
	} else {
		stdlog=fopen(soapboxlog, "a");
		if (stdlog==NULL) {
			stdlog=stderr;
			sb_die(stdlog, "%s: %s (%i)", soapboxlog, strerror(errno), errno);
		}
		setvbuf(stdlog, (char *)NULL, _IONBF, 0);
	}

	// Handle and unset debugging environment variable
	soapboxdbg=getenv("SOAPBOXDEBUG"); // unsetenv("SOAPBOXDEBUG");
	if (soapboxdbg) soapboxdebug=atoi(soapboxdbg);
	sb_log(8, "Variable SOAPBOXDEBUG is set to %i.", soapboxdebug);

	// Handle and unset path environment variable
	soapboxpath=getenv("SOAPBOXPATH"); // unsetenv("SOAPBOXPATH");
	if (!soapboxpath) soapboxpath="";
	if (!soapboxpath || *soapboxpath=='\0')
		sb_log(8, "Variable SOAPBOXPATH is not set. Not allowed to write anywhere.");
	else
		sb_log(8, "Variable SOAPBOXPATH is set to \"%s\".", soapboxpath);

        // Handle and unset bin path environment variable
        soapboxrun=getenv("SOAPBOXRUN"); // unsetenv("SOAPBOXPATH");
        if (!soapboxrun) soapboxrun="";
        if (!soapboxrun || *soapboxrun=='\0')
                sb_log(8, "Variable SOAPBOXRUN is not set. Not allowed to run anything.");
        else
                sb_log(8, "Variable SOAPBOXRUN is set to \"%s\".", soapboxrun);

        
	// Handle and unset action environment variable
	soapboxaction=getenv("SOAPBOXACTION"); // unsetenv("SOAPBOXACTION");
	if (!soapboxaction || *soapboxaction=='\0') {
	       	soapboxaction=DEFAULT_ACTION;
		sb_log(8, "Variable SOAPBOXACTION is not set. Using \"%s\" by default.", DEFAULT_ACTION);
	} else {
		sb_log(8, "Variable SOAPBOXACTION is set to \"%s\".", soapboxaction);
	}

	if (!strcmp(soapboxaction, "warn")) action=A_WARN;
	else if (!strcmp(soapboxaction, "err")) action=A_ERR;
	else if (!strcmp(soapboxaction, "halt")) action=A_HALT;

	if (action==A_UNKN) {
		sb_log(8, "Variable SOAPBOXACTION=\"%s\" is unknown. Using \"%s\" by default.", soapboxaction, DEFAULT_ACTION);
		action=A_DEFAULT;
	}

	// Get all symbols from glibc. (TODO: dlerror is always NULL, weird.)
	_real_chmod=dlsym(RTLD_NEXT, "chmod"); dlcheck(dlerror());

	_real_chown=dlsym(RTLD_NEXT, "chown"); dlcheck(dlerror());
//	_real_fchown=dlsym(RTLD_NEXT, "fchown"); dlcheck(dlerror());
	_real_lchown=dlsym(RTLD_NEXT, "lchown"); dlcheck(dlerror());
//      stop fork/clone system calls (natan)
        _real_clone=dlsym(RTLD_NEXT, "clone"); dlcheck(dlerror());
        _real_fork=dlsym(RTLD_NEXT, "fork"); dlcheck(dlerror());
        _real_vfork=dlsym(RTLD_NEXT, "vfork"); dlcheck(dlerror());
        _real_execve=dlsym(RTLD_NEXT, "execve"); dlcheck(dlerror());
        _real_socketcall=dlsym(RTLD_NEXT, "socketcall"); dlcheck(dlerror());
        _real_socket=dlsym(RTLD_NEXT, "socket"); dlcheck(dlerror());



	_real_link=dlsym(RTLD_NEXT, "link"); dlcheck(dlerror());
	_real_mkdir=dlsym(RTLD_NEXT, "mkdir"); dlcheck(dlerror());
	_real_mkfifo=dlsym(RTLD_NEXT, "mkfifo"); dlcheck(dlerror());

	_real_mknod=dlsym(RTLD_NEXT, "mknod"); dlcheck(dlerror());
	_real___xmknod=dlsym(RTLD_NEXT, "__xmknod"); dlcheck(dlerror());

	_real_open=dlsym(RTLD_NEXT, "open"); dlcheck(dlerror());
	_real_open64=dlsym(RTLD_NEXT, "open64"); dlcheck(dlerror());
	_real_creat=dlsym(RTLD_NEXT, "creat"); dlcheck(dlerror());
	_real_creat64=dlsym(RTLD_NEXT, "creat64"); dlcheck(dlerror());

//	_real_fopen=dlsym(RTLD_NEXT, "fopen"); dlcheck(dlerror());

	_real_remove=dlsym(RTLD_NEXT, "remove"); dlcheck(dlerror());
	_real_rename=dlsym(RTLD_NEXT, "rename"); dlcheck(dlerror());
	_real_rmdir=dlsym(RTLD_NEXT, "rmdir"); dlcheck(dlerror());
	_real_symlink=dlsym(RTLD_NEXT, "symlink"); dlcheck(dlerror());
	_real_unlink=dlsym(RTLD_NEXT, "unlink"); dlcheck(dlerror());
	_real_utime=dlsym(RTLD_NEXT, "utime"); dlcheck(dlerror());
	_real_utimes=dlsym(RTLD_NEXT, "utimes"); dlcheck(dlerror());

//	devnull=_real_open("/dev/null", O_RDWR);
	devnull=_real_open("/dev/zero", O_RDWR);
	if (devnull==-1) sb_die(stdlog, "/dev/null: %s", strerror(errno));

	// Print each process execution
	if (soapboxdebug & 1) {
		int i;
		fprintf(stdlog, "soapbox: debug1: Executing (pid: %i) \"", getpid());
		for(i=0; i<argc; i++) fprintf(stdlog, "%s ",argv[i]);
		fprintf(stdlog, "\"\n");
	}
}

void _fini(void) {
	close(devnull);
	if (stdlog!=stderr)
		fclose(stdlog);
}


/* Start of custom functions */

/* Is a known macro in string.h ! Kept for reference
const char *basename(const char *path) {
	const char *ptr=strrchr(path, '/');
	if (ptr==NULL) return path;
	else return ptr+1;
}*/

// Cut string and convert to pointer
/* I'm not going to replace one-liners by one-liners ;) Kept for reference.
char *str_cut(const char *str) {
	return strndup(str, strlen(str));
}*/

// Return dirname of path
static char *dirname(const char *path) {
	char *ptr;
	char safe[PATH_MAX+1];
	safe[0]='\0'; safe[PATH_MAX]='\0'; // Terminate string for safety :)

	if (strrchr(path, '/')==NULL) {
		getcwd(safe, PATH_MAX);
	} else {
		snprintf(safe, PATH_MAX, "%s", path);
		ptr=strrchr(safe, '/');
		*ptr='\0';
	}
	return strndup(safe, strlen(safe));
}

// Rewrite relative path to absolute.
static char *rewrite(const char *path, const int flag) {
	char *linkdir, *out;
	struct stat *buf;
	char temp[PATH_MAX+1], safe[PATH_MAX+1];

	temp[0]='\0'; temp[PATH_MAX]='\0'; // Terminate string for safety :)
	safe[0]='\0'; safe[PATH_MAX]='\0'; // Terminate string for safety :)

	// To make sure path is not empty and defined. Return empty string
	if (!path || *path=='\0')
		return strndup(safe, 0);

	// Check if file exists
	buf=malloc(sizeof(struct stat));
	if (lstat(path, buf)==0) {
		int type=(buf->st_mode & 0170000);
		int mode=(buf->st_mode & 07777);
		if (soapboxdebug & 8) {
			switch(type) {
				case S_IFLNK: sb_log(8, "File \"%s\" is a symlink. (%04o)", path, mode); break;;
				case S_IFREG: sb_log(8, "File \"%s\" is a regular file. (%04o)", path, mode); break;;
				case S_IFDIR: sb_log(8, "File \"%s\" is a directory. (%04o)", path, mode); break;;
				case S_IFCHR: sb_log(8, "File \"%s\" is a character device. (%04o)", path, mode); break;;
				case S_IFBLK: sb_log(8, "File \"%s\" is a block device. (%04o)", path, mode); break;;
				default: sb_log(8, "File \"%s\" is an unknown file type. (%04o)", path, mode); break;;
			}
		}

		// If it is a symlink and if asked (R_LINK), we should make its dirname absolute
		// (not the symlink) and add its basename
		if ((type==S_IFLNK) && (flag==R_LINK)) {
			linkdir=rewrite(dirname(path), R_LINK);
			snprintf(safe, PATH_MAX, "%s/%s", linkdir, basename(path));
			free(linkdir);
		} else {
			realpath(path, safe);
		}
	} else {
		sb_log(8, "File \"%s\" does not exist.", path);
		realpath(path, safe);	// TODO: Problem with non-existing files !!
/*		realpath(path, temp);

		// If the file doesn't exist, realpath() doesn't return its basename, so we need to add it ourselves
		// (TODO: If more than one subdirectory does not exist, this is WRONG. Alternative to realpath ?)
		if (strcmp(path, temp))
			snprintf(safe, PATH_MAX, "%s/%s", temp, basename(path));
		else
			snprintf(safe, PATH_MAX, "%s", temp);
*/
	}
	free(buf);

	out=strndup(safe, strlen(safe));

	if (strcmp(path, out))
		sb_log(4, "File \"%s\" is actually \"%s\".", path, out);

	return out;
}

// Verify if program has access to a path.
static int has_access(char *path) {
	int found=0;
	char *pathlist, *curpath;
        
	pathlist=strndup(soapboxpath, strlen(soapboxpath));
	curpath=strtok(pathlist, PATHSEP);
	while (curpath!=NULL&&!found) {
		if (curpath!='\0' && strstr(path, curpath)) {
			found=1;
			sb_log(4, "Allow access to \"%s\" (in \"%s\").", path, curpath);
		}
		curpath=strtok(NULL, PATHSEP);
	};
	free(pathlist);

	return found;
}

// Verify if a program can call an executable.
static int has_run_access(const char *path) {
        int found=0;
        char *pathlist, *curpath;
        
        pathlist=strndup(soapboxrun, strlen(soapboxrun));
        curpath=strtok(pathlist, PATHSEP);
        while (curpath!=NULL&&!found) {
                if (curpath!='\0' && strstr(path, curpath)) {
                        found=1;
                        sb_log(4, "Allow access to \"%s\" (in \"%s\").", path, curpath);
                }
                curpath=strtok(NULL, PATHSEP);
        };
        free(pathlist);

        return found;
}
// end changes


static void str_cmode(char *str, const char *cmode) {
	char *temp=strndup(str,strlen(str));
	if (strlen(str))
		snprintf(str, PATH_MAX, "%s|%s", temp, cmode);
	else
		snprintf(str, PATH_MAX, "%s%s", temp, cmode);
	free(temp);
}

static char *str_flags(const int flags) {
	char str[PATH_MAX+1];
	str[0]='\0'; str[PATH_MAX]='\0'; // Terminate string for safety :)

	if (flags & O_WRONLY) str_cmode(str, "O_WRONLY");
	else if (flags & O_RDWR) str_cmode(str, "O_RDWR");
	else str_cmode(str, "O_RDONLY");

	if (flags & O_EXCL) str_cmode(str, "O_EXCL");
	if (flags & O_TRUNC) str_cmode(str, "O_TRUNC");
	if (flags & O_APPEND) str_cmode(str, "O_APPEND");
	if (flags & O_NONBLOCK) str_cmode(str, "O_NONBLOCK");
	if (flags & O_CREAT) str_cmode(str, "O_CREAT");
	if (flags & O_NOCTTY) str_cmode(str, "O_NOCTTY");
	if (flags & O_SYNC) str_cmode(str, "O_SYNC");
	if (flags & O_NOFOLLOW) str_cmode(str, "O_NOFOLLOW");
	if (flags & O_DIRECT) str_cmode(str, "O_DIRECT");
	if (flags & O_ASYNC) str_cmode(str, "O_ASYNC");
	if (flags & O_LARGEFILE) str_cmode(str, "O_LARGEFILE");
	if (flags & O_DIRECTORY) str_cmode(str, "O_DIRECTORY");

	return strndup(str, strlen(str));
}

static char *str_timet(time_t time) {
	struct tm *t=localtime(&time);
	char str[PATH_MAX+1];
	str[0]='\0'; str[PATH_MAX]='\0'; // Terminate string for safety :)

	sprintf(str, "%04i/%02i/%02i-%02i:%02i:%02i", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
	free(t);

	return strndup(str, strlen(str));
}

static char *str_utimbuf(const struct utimbuf *buf) {
	char str[PATH_MAX+1];
	str[0]='\0'; str[PATH_MAX]='\0'; // Terminate string for safety :)

	if (buf==NULL)
		sprintf(str, "NULL");
	else
		sprintf(str, "[%s, %s]", str_timet(buf->actime), str_timet(buf->modtime));

	return strndup(str, strlen(str));
}

/* Start of actual overloaded libc functions */

int chmod(const char *path, mode_t mode) {
	char *rpath;
	sb_log(2, "Start chmod(\"%s\", %04o).", path, mode);
	rpath=rewrite(path, R_FILE);
	if (has_access(rpath)) {
		sb_log(4, "Do chmod(\"%s\", %04o).", path, mode);
		return _real_chmod(path, mode);
	}
	if (action==A_HALT) exit(0);
	sb_log(0, "Attempt to chmod(\"%s\", %04o).", rpath, mode);
	free(rpath);
	if (action==A_WARN) return 0;
	errno=EACCES;
	return -1;
}

int chown(const char *path, uid_t owner, gid_t group) {
	char *rpath;
	sb_log(2, "Start chown(\"%s\", %i, %i).", path, owner, group);
	rpath=rewrite(path, R_LINK);
	if (has_access(rpath)) {
		sb_log(4, "Do chown(\"%s\", %i, %i).", path, owner, group);
		return _real_chown(path, owner, group);
	}
	if (action==A_HALT) exit(0);
	sb_log(0, "Attempt to chown(\"%s\", %i, %i).", rpath, owner, group);
	free(rpath);
	if (action==A_WARN) return 0;
	errno=EACCES;
	return -1;
}

int lchown(const char *path, uid_t owner, gid_t group) {
	char *rpath;
	sb_log(2, "Start lchown(\"%s\", %i, %i).", path, owner, group);
	rpath=rewrite(path, R_LINK);
	if (has_access(rpath)) {
		sb_log(4, "Do lchown(\"%s\", %i, %i).", path, owner, group);
		return _real_lchown(path, owner, group);
	}
	if (action==A_HALT) exit(0);
	sb_log(0, "Attempt to lchown(\"%s\", %i, %i).", rpath, owner, group);
	free(rpath);
	if (action==A_WARN) return 0;
	errno=EACCES;
	return -1;
}

// Stop children processes from being created with clone, fork, vfork.
int clone(int child_stack, int flags, void* child_tidptr) {
        if (action==A_HALT) exit(0);
        sb_log(0, "Attempt to clone().");
        if (action==A_WARN) return 0;
        errno=EACCES;
        return -1;
}

int fork(void) {
        if (action==A_HALT) exit(0);
        sb_log(0, "Attempt to fork().");
        if (action==A_WARN) return 0;
        errno=EACCES;
        return -1;
}


int vfork(void) {
        if (action==A_HALT) exit(0);
        sb_log(0, "Attempt to vfork().");
        if (action==A_WARN) return 0;
        errno=EACCES;
        return -1;
}

// Stop calls to outside executables.
int execve(const char*filename, char *const argv[], char *const envp[]) {
    if (action==A_HALT) exit(0);
    if (has_run_access(filename)) {
            sb_log(4, "Allowing \"%s\" to be execve", filename);
            return _real_execve(filename, argv, envp);
    }
    sb_log(0, "Attempt to execve(%s).", filename);
    if (action==A_WARN) return 0;
    errno=EACCES;
    return -1;
}

// Stop sockets from being opened.
int socketcall(int call, unsigned long *args) {
    if (action==A_HALT) exit(0);
    sb_log(0, "Attempt to use sockets.");
    if (action==A_WARN) return 0;
    errno=EACCES;
    return -1;
}

int socket(int domain, int type, int protocol) {
    if (action==A_HALT) exit(0);
    sb_log(0, "Attempt to use sockets.");
    if (action==A_WARN) return 0;
    errno=EACCES;
    return -1;
}
// end of changes.

int link(const char *oldpath, const char *newpath) {
	char *oldrpath, *newrpath;
	sb_log(2, "Start link(\"%s\", \"%s\").", oldpath, newpath);
	oldrpath=rewrite(oldpath, R_FILE);
	newrpath=rewrite(newpath, R_FILE);
	if (has_access(oldrpath) && has_access(newrpath)) {
		sb_log(4, "Do link(\"%s\", \"%s\").", oldpath, newpath);
		return _real_link(oldpath, newpath);
	}
	if (action==A_HALT) exit(0);
	sb_log(0, "Attempt to link(\"%s\", \"%s\").", oldrpath, newrpath);
	free(oldrpath); free(newrpath);
	if (action==A_WARN) return 0;
	errno=EACCES;
	return -1;
}

int mkdir(const char *path, mode_t mode) {
	char *rpath;
	sb_log(2, "Start mkdir(\"%s\", %04o).", path, mode);
	rpath=rewrite(path, R_FILE);
	if (has_access(rpath)) {
		sb_log(4, "Do mkdir(\"%s\", %04o).", path, mode);
		return _real_mkdir(path, mode);
	}
	if (action==A_HALT) exit(0);
	sb_log(0, "Attempt to mkdir(\"%s\", %04o).", rpath, mode);
	free(rpath);
	if (action==A_WARN) return 0;
	errno=EACCES;
	return -1;
}

int mkfifo(const char *path, mode_t mode) {
	char *rpath;
	sb_log(2, "Start mkfifo(\"%s\", %04o).", path, mode);
	rpath=rewrite(path, R_FILE);
	if (has_access(rpath)) {
		sb_log(4, "Do mkfifo(\"%s\", %04o).", path, mode);
		return _real_mkfifo(path, mode);
	}
	if (action==A_HALT) exit(0);
	sb_log(0, "Attempt to mkfifo(\"%s\", %04o).", rpath, mode);
	free(rpath);
	if (action==A_WARN) return 0;
	errno=EACCES;
	return -1;
}

int mknod(const char *path, mode_t mode, dev_t dev) {
	char *rpath;
	sb_log(2, "Start mknod(\"%s\", %04o).", path, mode);
	rpath=rewrite(path, R_LINK);
	if (has_access(rpath)) {
		sb_log(4, "Do mknod(\"%s\", %04o).", path, mode);
		return _real_mknod(path, mode, dev);
	}
	if (action==A_HALT) exit(0);
	sb_log(0, "Attempt to mknod(\"%s\", %04o).", rpath, mode);
	free(rpath);
	if (action==A_WARN) return 0;
	errno=EACCES;
	return -1;
}

int __xmknod(int ver, const char *path, mode_t mode, dev_t *dev) {
	char *rpath;
	sb_log(2, "Start __xmknod(%i, \"%s\", %04o).", ver, path, mode);
	rpath=rewrite(path, R_LINK);
	if (has_access(rpath)) {
		sb_log(4, "Do __xmknod(%i, \"%s\", %04o).", ver, path, mode);
		return _real___xmknod(ver, path, mode, dev);
	}
	if (action==A_HALT) exit(0);
	sb_log(0, "Attempt to __xmknod(%i, \"%s\", %04o).", ver, rpath, mode);
	free(rpath);
	if (action==A_WARN) return 0;
	errno=EACCES;
	return -1;
}

int open(const char *path, int flags, ...) {
	char *rpath, *strflags=str_flags(flags);
	va_list argptr;
	mode_t mode;
	int found;

	// If O_CREAT then mode is not set.
	if (flags & O_CREAT) {
		va_start(argptr, flags);
		mode=va_arg(argptr, mode_t);
		va_end(argptr);
	} else {
		mode=0;
	}

	sb_log(2, "Start open(\"%s\", %s, %04o).", path, strflags, mode);
	rpath=rewrite(path, R_FILE);
	if ((found=has_access(rpath)) || ! (flags & (O_WRONLY|O_RDWR))) {
		// Disable O_CREAT when O_RDONLY
		if (!found) {
			flags&=~O_CREAT;
			strflags=str_flags(flags);
		}
		sb_log(4, "Do open(\"%s\", %s, %04o).", path, strflags, mode);
		return _real_open(path, flags, mode);
	}
	if (action==A_HALT) exit(0);
	sb_log(0, "Attempt to open(\"%s\", %s, %04o).", rpath, strflags, mode);
	free(rpath); free(strflags);
	if (action==A_WARN) return devnull;
	errno=EACCES;
	return -1;
}

int open64(const char *path, int flags, ...) {
	char *rpath, *strflags=str_flags(flags);
	va_list argptr;
	mode_t mode;
	int found;

	// If O_CREAT then mode is not set.
	if (flags & O_CREAT) {
		va_start(argptr, flags);
		mode=va_arg(argptr, mode_t);
		va_end(argptr);
	} else {
		mode=0;
	}

	sb_log(2, "Start open64(\"%s\", %s, %04o).", path, strflags, mode);
	rpath=rewrite(path, R_FILE);
	if ((found=has_access(rpath)) || ! (flags & (O_WRONLY|O_RDWR))) {
		// Disable O_CREAT when O_RDONLY
		if (!found) {
			flags&=~O_CREAT;
			strflags=str_flags(flags);
		}
		sb_log(4, "Do open64(\"%s\", %s, %04o).", path, strflags, mode);
		return _real_open64(path, flags, mode);
	}
	if (action==A_HALT) exit(0);
	sb_log(0, "Attempt to open64(\"%s\", %s, %04o).", rpath, strflags, mode);
	free(rpath); free(strflags);
	if (action==A_WARN) return devnull;
	errno=EACCES;
	return -1;
}

int creat(const char *path, mode_t mode) {
	char *rpath;
	sb_log(2, "Start creat(\"%s\", %04o).", path, mode);
	rpath=rewrite(path, R_FILE);
	if (has_access(rpath)) {
		sb_log(4, "Do creat(\"%s\", %04o).", path, mode);
		return _real_creat(path, mode);
	}
	if (action==A_HALT) exit(0);
	sb_log(0, "Attempt to creat(\"%s\", %04o).", rpath, mode);
	free(rpath);
	if (action==A_WARN) return devnull;
	errno=EACCES;
	return -1;
}

int creat64(const char *path, mode_t mode) {
	char *rpath;
	sb_log(2, "Start creat64(\"%s\", %04o).", path, mode);
	rpath=rewrite(path, R_FILE);
	if (has_access(rpath)) {
		sb_log(4, "Do creat64(\"%s\", %04o).", path, mode);
		return _real_creat64(path, mode);
	}
	if (action==A_HALT) exit(0);
	sb_log(0, "Attempt to creat64(\"%s\", %04o).", rpath, mode);
	free(rpath);
	if (action==A_WARN) return devnull;
	errno=EACCES;
	return -1;
}

/*
// TODO: mode should be checked for read-only, write or create
FILE *fopen(const char *path, const char *mode) {
	char *rpath;
	sb_log(2, "Start fopen(\"%s\", \"%s\").", path, mode);
	rpath=rewrite(path, R_FILE);
	if (has_access(rpath)) {
		sb_log(4, "Do fopen(\"%s\", \"%s\").", path, mode);
		return (FILE *) _real_fopen(path, mode);
	}
	if (action==A_HALT) exit(0);
	sb_log(0, "Attempt to fopen(\"%s\", \"%s\").", rpath, mode);
	free(rpath);
	if (action==A_WARN) return devnull;
	errno=EACCES;
	return -1;
}
*/

int remove(const char *path) {
	char *rpath;
	sb_log(2, "Start remove(\"%s\").", path);
	rpath=rewrite(path, R_LINK);
	if (has_access(rpath)) {
		sb_log(4, "Do remove(\"%s\").", path);
		return _real_remove(path);
	}
	if (action==A_HALT) exit(0);
	sb_log(0, "Attempt to remove(\"%s\").", rpath);
	free(rpath);
	if (action==A_WARN) return 0;
	errno=EACCES;
	return -1;
}

int rename(const char *oldpath, const char *newpath) {
	char *oldrpath, *newrpath;
	sb_log(2, "Start rename(\"%s\", \"%s\").", oldpath, newpath);
	oldrpath=rewrite(oldpath, R_LINK);
	newrpath=rewrite(newpath, R_FILE);
	if (has_access(oldrpath) && has_access(newrpath)) {
		sb_log(4, "Do rename(\"%s\", \"%s\").", oldpath, newpath);
		return _real_rename(oldpath, newpath);
	}
	if (action==A_HALT) exit(0);
	sb_log(0, "Attempt to rename(\"%s\", \"%s\").", oldrpath, newrpath);
	free(oldrpath); free(newrpath);
	if (action==A_WARN) return 0;
	errno=EACCES;
	return -1;
}

int rmdir(const char *path) {
	char *rpath;
	sb_log(2, "Start rmdir(\"%s\").", path);
	rpath=rewrite(path, R_LINK);
	if (has_access(rpath)) {
		sb_log(4, "Do rmdir(\"%s\").", path);
		return _real_rmdir(path);
	}
	if (action==A_HALT) exit(0);
	sb_log(0, "Attempt to rmdir(\"%s\").", rpath);
	free(rpath);
	if (action==A_WARN) return 0;
	errno=EACCES;
	return -1;
}

int symlink(const char *oldpath, const char *newpath) {
//	char *oldrpath;
	char *newrpath;
	sb_log(2, "Start symlink(\"%s\", \"%s\").", oldpath, newpath);
//	oldrpath=rewrite(oldpath, R_FILE);
	newrpath=rewrite(newpath, R_LINK);
	if (has_access(newrpath)) {
		sb_log(4, "Do symlink(\"%s\", \"%s\").", oldpath, newpath);
		return _real_symlink(oldpath, newpath);
	}
	if (action==A_HALT) exit(0);
	sb_log(0, "Attempt to symlink(\"%s\", \"%s\").", oldpath, newrpath);
	free(newrpath);
	if (action==A_WARN) return 0;
	errno=EACCES;
	return -1;
}

int unlink(const char *path) {
	char *rpath;
	sb_log(2, "Start unlink(\"%s\").", path);
	rpath=rewrite(path, R_LINK);
	if (has_access(rpath)) {
		sb_log(4, "Do unlink(\"%s\").", path);
		return _real_unlink(path);
	}
	if (action==A_HALT) exit(0);
	sb_log(0, "Attempt to unlink(\"%s\").", rpath);
	free(rpath);
	if (action==A_WARN) return 0;
	errno=EACCES;
	return -1;
}

int utime(const char *path, const struct utimbuf *buf) {
	char *rpath;
	sb_log(2, "Start utime(\"%s\", NULL).", path);
	rpath=rewrite(path, R_FILE);
	if (has_access(rpath)) {
		sb_log(4, "Do utime(\"%s\", NULL).", path);
		return _real_utime(path, buf);
	}
	if (action==A_HALT) exit(0);
	sb_log(0, "Attempt to utime(\"%s\", %s).", rpath, str_utimbuf(buf));
	free(rpath);
	if (action==A_WARN) return 0;
	errno=EACCES;
	return -1;
}

int utimes(const char *path, const struct timeval *tvp) {
	char *rpath;
	sb_log(2, "Start utimes(\"%s\", NULL).", path);
	rpath=rewrite(path, R_FILE);
	if (has_access(rpath)) {
		sb_log(4, "Do utimes(\"%s\", NULL).", path);
		return _real_utimes(path, tvp);
	}
	if (action==A_HALT) exit(0);
	sb_log(0, "Attempt to utimes(\"%s\", NULL).", rpath);
	free(rpath);
	if (action==A_WARN) return 0;
	errno=EACCES;
	return -1;
}
