#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define SEM_RESOURCE_MAX  1
#define SEMMSL 32

union semun {
  int             val;
  struct semid_ds *buf;
  unsigned short  *array;
  struct seminfo  *__buf;
};

void opensem(int *sid, key_t key);
void createsem(int *sid, key_t key, int members);
void locksem(int sid, int member);
void unlocksem(int sid, int member);
void removesem(int sid);
unsigned short get_member_count(int sid);
int getval(int sid, int member);
void dispval(int sid, int member);
void changemode(int sid, char *mode);
void usage(void);

int main(int argc, char *argv[])
{
  key_t key;
  int   semset_id;

  if (argc == 1) usage();
  key = ftok(".", 's');

  switch(tolower(argv[1][0]))
  {
    case 'c':
      if (argc != 3) usage();
      createsem(&semset_id, key,  atoi(argv[2]));
      break;
    case 'l':
      if (argc != 3) usage();
      opensem(&semset_id, key);
      locksem(semset_id, atoi(argv[2]));
      break;
    case 'u':
      if (argc != 3) usage();
      opensem(&semset_id, key);
      unlocksem(semset_id, atoi(argv[2]));
      break;
    case 'd':
      opensem(&semset_id, key);
      removesem(semset_id);
      break;
    case 'm':
      opensem(&semset_id, key);
      changemode(semset_id, argv[2]);
      break;
     default:
      usage();
  }

  return 0;
}

void opensem(int *sid, key_t key)
{
  /* Open the semaphore set - do not create! */

  if ((*sid = semget(key, 0, 0666)) == -1)
  {
    printf("Semaphore set does not exist!\n");
    exit(1);
  }
}

void createsem(int *sid, key_t key, int members)
{
  int cntr;
  union semun semopts;

  if (members > SEMMSL) {
    printf("Sorry, max number of semaphores in a set is %d\n", SEMMSL);
    exit(1);
  }

  printf("Attempting to create new semaphore set with %d members\n", members);

  if ((*sid = semget(key, members, IPC_CREAT|IPC_EXCL|0666)) == -1)
  {
    fprintf(stderr, "Semaphore set already exists!\n");
    exit(1);
  }

  semopts.val = SEM_RESOURCE_MAX;

  /* Initialize all members (could be done with SETALL) */
  for (cntr = 0; cntr < members; cntr++) {
    semctl(*sid, cntr, SETVAL, semopts);
  }
}
void usage(void)
{
  fprintf(stderr, "semtool - A utility for tinkering with semaphores\n");
  fprintf(stderr, "\nUSAGE:  semtool4 (c)reate <semcount>\n");
  fprintf(stderr, "     (l)ock <sem #>\n");
  fprintf(stderr, "     (u)nlock <sem #>\n");
  fprintf(stderr, "     (d)elete\n");
  fprintf(stderr, "     (m)ode <mode>\n");
  exit(1);
}