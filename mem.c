/* 
   stampando il valore di alcuni puntatori, 
   illustra come vengono assegnati gli indirizzi (virtuali) 
   dal compilatore, e quali vengono restituiti dalla malloc e dalla shmat 

   il risultato su una particolare macchina e' 
   illustrato graficamente in mem.jpg
*/

#include <malloc.h>
#include <unistd.h>
#include <stdio.h>
#include <wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

int ext1, ext2;

int main()

{
  int n;
  int shmid1, *addr1;
  int shmid2;
  char *addr2;

  /* alloca memoria condivisa per un array di 100 interi e uno di 100 caratteri */

  shmid1 = shmget(IPC_PRIVATE, 100 * sizeof(int), 0600);
  if (shmid1 == -1)
    perror("Creazione memoria condivisa");

  shmid2 = shmget(IPC_PRIVATE, 100 * sizeof(char), 0600);
  if (shmid2 == -1)
    perror("Creazione memoria condivisa");

  n = fork();
  if (n == -1)
    perror("Fork");

  if (n == 0)
  { /* processo figlio */
    addr1 = (int *)shmat(shmid1, NULL, 0);
    addr2 = (char *)shmat(shmid2, NULL, 0);
    printf("shmat processo figlio al segmento di 100 interi: %p\n", (void *)addr1);
    printf("shmat processo figlio al segmento di 100 char: %p\n", (void *)addr2);
    *addr1 = 10;
    *addr2 = 'P';
  }
  else
  { /* processo padre */
    addr1 = (int *)shmat(shmid1, NULL, 0);
    addr2 = (char *)shmat(shmid2, NULL, 0);
    wait(NULL);
    printf("shmat processo padre al segmento di 100 interi: %p\n", (void *)addr1);
    printf("shmat processo padre al segmento di 100 char: %p\n", (void *)addr2);
    printf("Valore memoria in addr1: %d\n", *addr1);
    printf("Valore memoria in addr2: %c\n", *addr2);

    shmctl(shmid1, IPC_RMID, NULL);
    shmctl(shmid2, IPC_RMID, NULL);
  }
}