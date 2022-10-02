#ifndef _SEM_SHM_DEFS_H
#define _SEM_SHM_DEFS_H

#include <semaphore.h>

#define POSIX_IPC_NAME_PREFIX "/cdimegl1-"

#define SHM_NAME POSIX_IPC_NAME_PREFIX "knn-shm"
#define SERVER_SEM_NAME POSIX_IPC_NAME_PREFIX "server"
#define REQUEST_SEM_NAME POSIX_IPC_NAME_PREFIX "request"
#define RESPONSE_SEM_NAME POSIX_IPC_NAME_PREFIX "response"

enum {
	SERVER_SEM,
	REQUEST_SEM,
	RESPONSE_SEM,
	N_SEMS
};

#define ALL_RW_PERMS (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)

static void
semWait(sem_t *sem, const char *posixName)
{
  if (sem_wait(sem) < 0) {
    panic("cannot wait on sem %s", posixName);
  }
}

static void
semPost(sem_t *sem, const char *posixName)
{
  if (sem_post(sem) < 0) {
    panic("cannot post sem %s", posixName);
  }
}

#endif

