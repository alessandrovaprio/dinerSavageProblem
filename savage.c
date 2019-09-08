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
static int notFinished=1;
int getServingsFromPot(void)
{
  int retVal;

  if (foods <= 0)
  {
    // pthread_mutex_lock(&print_mutex);
    // printf("invio empty pot ");
    // pthread_mutex_unlock(&print_mutex);

    // send to semaphore emptyPot
    sem_post(&emptyPot);
    retVal = 0;
  }
  else
  {
    foods--;
    retVal = foods;
  }

  return retVal;
}

void putServingsInPot(int num)
{

  foods += num;
  //send to fullPot semaphore
  sem_post(&fullPot);
}

void *cook(void *id)
{
  //int cook_id = *(int *)id;
  
  //cycle on rounds time
  while (1)
  {
    // lock the access of the foods variable
    pthread_mutex_lock(&servings_mutex);
    int tmp = foods;
    //unlock the variable
    pthread_mutex_unlock(&servings_mutex);
    if(tmp>0){
      pthread_mutex_lock(&print_mutex);
      printf("\nCook is sleeping\n\n");
      pthread_mutex_unlock(&print_mutex);
      //wait for the request to fill the pot
      sem_wait(&emptyPot);
      
      // if(notFinished==0){
      //   break;
      // }
        
      pthread_mutex_lock(&servings_mutex);
      putServingsInPot(tmp_food);
      pthread_mutex_unlock(&servings_mutex);

      pthread_mutex_lock(&print_mutex);
      printf("\nCook he was woken up and filled pot\n\n");
      pthread_mutex_unlock(&print_mutex);
      // take count of many times the cook fills the pot
      numberOfFillPot++;
      //sleep(1);
      
      //unlock the semaphore for each savage
      for (int i = 0; i < num_savages; i++)
        sem_post(&fullPot);
     }
     else{
      pthread_mutex_lock(&print_mutex);
      printf("\nCook see no more food, he cooks and filled pot\n\n");
      pthread_mutex_unlock(&print_mutex);
      numberOfFillPot++;
      pthread_mutex_lock(&servings_mutex);
      putServingsInPot(tmp_food);
      pthread_mutex_unlock(&servings_mutex);

      pthread_mutex_lock(&servings_mutex);
      pthread_mutex_unlock(&servings_mutex);
      //sleep(1);
       //unlock the semaphore for each savage
      for (int i = 0; i < num_savages; i++)
        sem_post(&fullPot);
    }
    // put the food in pot
  }

  pthread_mutex_lock(&print_mutex);
  printf("\n\nAll the savages have eaten , the cook can go home\n\n");
  pthread_mutex_unlock(&print_mutex);
  //rounds--;
  return NULL;
}

void *savage(void *id)
{
  
  int savage_id = *(int *)id;
  //sleep(savage_id);
  int tmp_rounds = rounds;
  pthread_mutex_lock(&servings_mutex);
  int tmp_foods = foods;
  pthread_mutex_unlock(&servings_mutex);
  // pthread_mutex_lock(&print_mutex);
  // printf("Savage: %i enter and servings is %i\n", savage_id, tmp_foods);
  // pthread_mutex_unlock(&print_mutex);
  
  //cicle infinite loop
  while (tmp_rounds)
  {
    //sleep(1);
    pthread_mutex_lock(&servings_mutex);
    tmp_foods= foods;
    //int tmp_rounds = rounds;
    pthread_mutex_unlock(&servings_mutex);
    // pthread_mutex_lock(&print_mutex);
    // printf("Savage: %i is inside loop is %i\n", savage_id, tmp_foods);
    // pthread_mutex_unlock(&print_mutex);
    if ((tmp_foods == -1))
    {
      //lock the access of the foods variable
      pthread_mutex_lock(&servings_mutex);
      myServing = getServingsFromPot();
      //unlock the variable
      pthread_mutex_unlock(&servings_mutex);
      // check if food left is zero
      if (myServing == 0)
      {
        pthread_mutex_lock(&print_mutex);
        printf("\nsavage %i is waiting\n",savage_id);
        pthread_mutex_unlock(&print_mutex);

        //pthread_mutex_unlock(&servings_mutex);
        //wait until the pot is full again
        sem_wait(&fullPot);
        
      }
     
    }
    else{
      //if ((tmp_foods == foods)){
        // lock the acess to the shared variable foods
        pthread_mutex_lock(&servings_mutex);
        myServing = getServingsFromPot();
        pthread_mutex_unlock(&servings_mutex);
        // lock the cmd printing
        pthread_mutex_lock(&print_mutex);
        printf("Savage: %i is eating and food left is %i\n", savage_id, myServing);
        pthread_mutex_unlock(&print_mutex);

        sleep(1);

        pthread_mutex_lock(&print_mutex);
        printf("Savage: %i is DONE eating\n",savage_id);
        pthread_mutex_unlock(&print_mutex);
        tmp_rounds--;
        //}
    }
    //pthread_mutex_unlock(&servings_mutex);
    //myServing--;
    
    }
  
  

  return NULL;
}

int main(int argc, char *argv[])
{
  
  if (argc == 4)
  {
    num_savages=atoi(argv[1]);
    tmp_food = atoi(argv[2]);
    foods= tmp_food;
    rounds = atoi(argv[3]);

    int i, id[num_savages + 1];
    pthread_t tid[num_savages + 1];
    //myServing = foods;
    //cookHasRounds = rounds;
    // Initialize the mutex locks
    pthread_mutex_init(&servings_mutex, NULL);
    pthread_mutex_init(&print_mutex, NULL);

    // Initialize the semaphores
    sem_init(&emptyPot, 0, 0);
    sem_init(&fullPot, 0, 0);

    // create one thread for each savage
    for (i = 0; i < num_savages; i++)
    {
      id[i] = i;
      pthread_create(&tid[i], NULL, savage, (void *)&id[i]);
    }
    pthread_create(&tid[i], NULL, cook, (void *)&id[i]);

    // wait for all threads to return
    for (i = 0; i < num_savages; i++)
    {
      pthread_join(tid[i], NULL);
    }
    
    // notFinished==-1;
    // foods = 0;
    // //to unlock the cook
    // getServingsFromPot();
    // //wait for the cook
    // pthread_join(tid[num_savages], NULL);
    // count and print the times the pot is filled
    printf("The cook fill %d times the pot \n",numberOfFillPot);
  }
  else
  {
    printf("Incorrect numbers of paramters. 3 params needed: Number of savages, Number of Portions and Number of Rounds \n");
  }
}