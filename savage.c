/*
 * Dining Savages problem
 *
 * This program demostrates a solution to this less classical problem.
 * The solution is based on pseudo code in Allen Downey's,
 * "The Little Book of Semaphores", Version 2.1.5
 *
 */

#include <stdio.h>
#include <unistd.h>

#include <pthread.h>
#include <semaphore.h>

#define NUM_SAVAGES 3

sem_t emptyPot;
sem_t fullPot;

void *savage(void *);
void *cook(void *);

static pthread_mutex_t servings_mutex;
static pthread_mutex_t print_mutex;
static int num_savages =0;
static int foods = 0;
static int rounds=0;
static int myServing = 0;
static int numberOfFillPot = 0;
static int cookHasRounds = 0;
static int tmp_food = 0;

int getServingsFromPot(void)
{
  int retVal;

  if (foods <= 0)
  {
    // pthread_mutex_lock(&print_mutex);
    // printf("invio empty pot ");
    // pthread_mutex_unlock(&print_mutex);
    sem_post(&emptyPot);
    retVal = 0;
  }
  else
  {
    foods--;
    retVal = foods;
  }

  //pthread_mutex_unlock(&servings_mutex);

  return retVal;
}

void putServingsInPot(int num)
{

  foods += num;
  sem_post(&fullPot);
}

void *cook(void *id)
{
  int cook_id = *(int *)id;
  //int meals = 2;
  int i;
  //because we assume the pot is full
  rounds --;
  while (rounds)
  {
    pthread_mutex_lock(&print_mutex);
    printf("\nCook is sleeping\n\n");
    pthread_mutex_unlock(&print_mutex);
    //wait for the 
    sem_wait(&emptyPot);
    pthread_mutex_lock(&print_mutex);
    printf("\nCook finish wait\n\n");
    pthread_mutex_unlock(&print_mutex);
    putServingsInPot(tmp_food);
    rounds--;

    pthread_mutex_lock(&print_mutex);
    printf("\nCook filled pot\n\n");
    printf("\nservings %d\n",foods);
    printf("\nmeals %d\n", foods);
    pthread_mutex_unlock(&print_mutex);
    numberOfFillPot++;
    sleep(1);
    //cookHasRounds--;
    //unlock the semaphore
    for (i = 0; i < num_savages; i++)
      sem_post(&fullPot);
  }
  rounds--;
  return NULL;
}

void *savage(void *id)
{
  int savage_id = *(int *)id;
  
  pthread_mutex_lock(&print_mutex);
  printf("Savage: %i enter and servings is %i\n", savage_id, foods);
  pthread_mutex_unlock(&print_mutex);
  // if (foods <= 0 && rounds <=0)
  // {
  //   pthread_mutex_lock(&print_mutex);
  //   printf("Savage: %i sono entrato giusto %i\n", savage_id, foods);
  //   pthread_mutex_unlock(&print_mutex);
  //   sem_wait(&fullPot);
  //   // pthread_mutex_lock(&servings_mutex);
  //   // myServing = getServingsFromPot();
  //   // pthread_mutex_unlock(&servings_mutex);
  //   //sem_wait(&fullPot);
  // }
  while (1)
  {
    //printf("\nServing %d for savage %d\n\n",servings,id);
    //sleep(10);

    if (foods == 0)
    {
      pthread_mutex_lock(&servings_mutex);
      myServing = getServingsFromPot();
      pthread_mutex_unlock(&servings_mutex);
      if (rounds > 0)
      {
        pthread_mutex_lock(&print_mutex);
        printf("\nsavage %i is waiting\n",savage_id);
        pthread_mutex_unlock(&print_mutex);
        sem_wait(&fullPot);
        
      }else{
        pthread_mutex_lock(&print_mutex);
        printf("\nsavage %i is exiting\n", savage_id);
        pthread_mutex_unlock(&print_mutex);
        return NULL;
      }
    }
    else{
      pthread_mutex_lock(&servings_mutex);
      myServing = getServingsFromPot();
      pthread_mutex_unlock(&servings_mutex);
      pthread_mutex_lock(&print_mutex);
      printf("Savage: %i is eating and servings is %i\n", savage_id, foods);
      pthread_mutex_unlock(&print_mutex);

      sleep(1);

      pthread_mutex_lock(&print_mutex);
      printf("Savage: %i is DONE eating and servings is %i\n", savage_id, foods);
      pthread_mutex_unlock(&print_mutex);
    }

    //myServing--;

    
    }
  
  

  return NULL;
}
// void *savage(void *id)
// {
//   int savage_id = *(int *)id;
//   int myServing;
//   int savage_servings = 2;

//   while (savage_servings > 0)
//   {
//     //printf("\nServing %d for savage %d\n\n",servings,id);
//     pthread_mutex_lock(&servings_mutex);

//     myServing = getServingsFromPot();
//     if (foods == 0)
//     {
//       sem_wait(&fullPot);
//       myServing = getServingsFromPot();
//     }

//     pthread_mutex_unlock(&servings_mutex);

//     savage_servings--;

//     pthread_mutex_lock(&print_mutex);
//     printf("Savage: %i is eating and servings is %i\n", savage_id, foods);
//     pthread_mutex_unlock(&print_mutex);

//     sleep(1);

//     pthread_mutex_lock(&print_mutex);
//     printf("Savage: %i is DONE eating and servings is %i\n", savage_id, foods);
//     pthread_mutex_unlock(&print_mutex);
//   }

//   return NULL;
// }
int main(int argc, char *argv[])
{
  // num_savages = 0;
  if (argc > 4)
  {
    printf("Too many arguments supplied.\n");
  }
  else if (argc < 4)
  {
    printf("3 parameters needed: Number of savages, Number of Portions and Number of Rounds \n");
  }

  if (argc == 4)
  {
    num_savages=atoi(argv[1]);
    tmp_food = atoi(argv[2]);
    foods= tmp_food;
    rounds = atoi(argv[3]);

    int i, id[num_savages + 1];
    pthread_t tid[num_savages + 1];
    myServing = foods;
    //cookHasRounds = rounds;
    // Initialize the mutex locks
    pthread_mutex_init(&servings_mutex, NULL);
    pthread_mutex_init(&print_mutex, NULL);

    // Initialize the semaphores
    sem_init(&emptyPot, 0, 0);
    sem_init(&fullPot, 0, 0);

    
    for (i = 0; i < num_savages; i++)
    {
      id[i] = i;
      pthread_create(&tid[i], NULL, savage, (void *)&id[i]);
    }
    pthread_create(&tid[i], NULL, cook, (void *)&id[i]);

    for (i = 0; i < num_savages; i++)
    {
      pthread_join(tid[i], NULL);
    }
    printf("The cook fill %d times the pot \n",numberOfFillPot);
  }
}