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
static pthread_mutex_t savage_finish_mutex;
static int num_savages =0;
static int foods = 0;
static int rounds=0;
static int myServing = 0;
static int numberOfFillPot = 0;
static int cookHasRounds = 0;
static int tmp_food = 0;
static int notFinished=1;
int getServingsFromPot(int savage_id)
{
  int retVal;

  if (foods <= 0)
  {
    

    // send to semaphore emptyPot
    int tmp_sem=0;
    sem_getvalue(&emptyPot,&tmp_sem);
    
   //check if the semaphore is zero
    if (tmp_sem ==0){
      sem_post(&emptyPot);
      
    }
    retVal = 0;
  }
  else
  {
    
    foods--;
    retVal = foods;
  }

  return retVal;
}

int putServingsInPot(int num)
{

  pthread_mutex_lock(&savage_finish_mutex);
  int savageLeft = notFinished;
  pthread_mutex_unlock(&savage_finish_mutex);
  if(foods<=0 && savageLeft>0){
    // pthread_mutex_lock(&print_mutex);
    // printf("\n---Food is lower or equals than zero,  %i---\n",foods);
    // pthread_mutex_unlock(&print_mutex);
    
    foods = num;
    
    sem_post(&fullPot);
  }else{
    // pthread_mutex_lock(&print_mutex);
    // printf("\n***Food is %i***\n",foods);
    // pthread_mutex_unlock(&print_mutex);

    return 0;

  }
  
  return 1;
}

void *cook(void *id)
{
  
  
  //cycle
  while (1)
  {
    
    // lock the access of the foods variable
    pthread_mutex_lock(&servings_mutex);
    int tmp = foods;
    //unlock the variable
    pthread_mutex_unlock(&servings_mutex);
    //check if there is food in the pot
    if(tmp>0){
      pthread_mutex_lock(&print_mutex);
      printf("\nCook is sleeping\n\n");
      pthread_mutex_unlock(&print_mutex);
      //wait for the request to fill the pot
      sem_wait(&emptyPot);
      pthread_mutex_lock(&savage_finish_mutex);
      int savageLeft = notFinished;
      pthread_mutex_unlock(&savage_finish_mutex);
      //if there aro no savage left the cook can go home
      if (savageLeft > 0)
      {
      
        pthread_mutex_lock(&servings_mutex);
        int hasAdded = putServingsInPot(tmp_food);
        
        //Check if the cook fill the pot
        if(hasAdded == 1){
          pthread_mutex_lock(&print_mutex);
          printf("\nCook he was woken up and filled pot\n\n");
          pthread_mutex_unlock(&print_mutex);
        }else
        {
          pthread_mutex_lock(&print_mutex);
          printf("\nCook do nothing\n");
          pthread_mutex_unlock(&print_mutex);
        }
        pthread_mutex_unlock(&servings_mutex);
        // take count of many times the cook fills the pot
        if (hasAdded>0)
          numberOfFillPot++;
        //sleep(1);
        
        //unlock the semaphore for each savage
        for (int i = 1; i < num_savages; i++)
          sem_post(&fullPot);

      }
      else{
        pthread_mutex_lock(&print_mutex);
        printf("\n\nAll the savages have eaten , the cook can go home!!\n\n");
        pthread_mutex_unlock(&print_mutex);
        return NULL;
      }
    }
    else{
       pthread_mutex_lock(&savage_finish_mutex);
       int savageLeft = notFinished;
       pthread_mutex_unlock(&savage_finish_mutex);
       //if there aro no savage left the cook can go home
       if(savageLeft>0){

          pthread_mutex_lock(&servings_mutex);
          int hasAdded = putServingsInPot(tmp_food);
          pthread_mutex_unlock(&servings_mutex);

          if (hasAdded > 0){
            pthread_mutex_lock(&print_mutex);
            printf("\nCook see no more food, he cooks and filled pot\n\n");
            pthread_mutex_unlock(&print_mutex);
            numberOfFillPot++;
          }else{
            pthread_mutex_lock(&print_mutex);
            printf("\nCook do nothing\n");
            pthread_mutex_unlock(&print_mutex);
          }

          //sleep(1);
          //unlock the semaphore for each savage
          for (int i = 1; i < num_savages; i++)
            sem_post(&fullPot);

           pthread_mutex_lock(&print_mutex);
           printf("\nCook is sleeping\n\n");
           pthread_mutex_unlock(&print_mutex);
           //wait for the request to fill the pot
           sem_wait(&emptyPot);
       }
       else
       {
         pthread_mutex_lock(&print_mutex);
         printf("\n\nAll the savages have eaten, the cook can go home\n\n");
         pthread_mutex_unlock(&print_mutex);
         return NULL;
       }
       // put the food in pot
    }
  }

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
  
  
  //loop
  while (tmp_rounds)
  {
    //sleep(1);
    //lock the access of the foods variable
    pthread_mutex_lock(&servings_mutex);
    tmp_foods= foods;
    
    if ((tmp_foods <= 0))
    {
      myServing = getServingsFromPot(savage_id);

        pthread_mutex_lock(&print_mutex);
        printf("\nsavage %i is waiting\n",savage_id);
        pthread_mutex_unlock(&print_mutex);

        //unlock the variable
        pthread_mutex_unlock(&servings_mutex);
        
        //wait until the pot is full again
        sem_wait(&fullPot);

     
    }
    else{
      
        myServing = getServingsFromPot(savage_id);
        
        // lock the console printing
        pthread_mutex_lock(&print_mutex);
        printf("Savage: %i is eating and food remained is %i\n", savage_id, myServing);
        printf("Savage: %i is DONE eating\n", savage_id);
        pthread_mutex_unlock(&print_mutex);

        //unlock the variable
        pthread_mutex_unlock(&servings_mutex);
        //sleep(1);

        
        tmp_rounds--;
        
    }
  }
  // decrease the notFinish variable
  pthread_mutex_lock(&savage_finish_mutex);
  notFinished--;
  pthread_mutex_unlock(&savage_finish_mutex);
  
  pthread_mutex_lock(&print_mutex);
  printf("...Savage: %i is exiting and food remained is %i...\n", savage_id, myServing);
  pthread_mutex_unlock(&print_mutex);

  
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
    notFinished=num_savages;
    int i, id[num_savages + 1];
    pthread_t tid[num_savages + 1];
    //myServing = foods;
    //cookHasRounds = rounds;
    // Initialize the mutex locks
    pthread_mutex_init(&servings_mutex, NULL);
    pthread_mutex_init(&print_mutex, NULL);
    pthread_mutex_init(&savage_finish_mutex, NULL);

    // Initialize the semaphores
    sem_init(&emptyPot, 0, 0);
    sem_init(&fullPot, 0, 0);
    

    // pthread_create(&tid[num_savages], NULL, cook, (void *)&id[num_savages]);
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
    
    
    
    //pthread_kill(tid[num_savages], 3);
    //send message to cook in order to make it understand he has finished
    sem_post(&emptyPot);
    //wait the cook until he finished
    pthread_join(tid[i], NULL);

    printf("\nFood remained %d \n", foods);
    printf("The cook fill %d times the pot \n",numberOfFillPot);
    //
    }
  else
  {
    printf("Incorrect numbers of paramters. 3 params needed: Number of savages, Number of Portions and Number of Rounds \n");
  }
}