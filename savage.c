#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/ipc.h>
//#include <sys/sem.h>
#include <sys/shm.h>

#define NUM_SAVAGES 3

sem_t *emptyPot;
sem_t *fullPot;
sem_t *finishOperation;
sem_t *doOperation;
sem_t *mutex;

// void *savage(void *);
// void *cook(void *);
void cook(int id);
void savage(int id);

// static //pthread_mutex_t servings_mutex;
// static //pthread_mutex_t print_mutex;
// static //pthread_mutex_t savage_finish_mutex;
static int num_savages =0;
static int foods = 0;
static int rounds=0;
//static int myServing = 0;
static int numberOfFillPot = 0;
static int cookHasRounds = 0;
static int tmp_food = 0;
static int notFinished=1;
static int *ptrfood, *ptrrounds, *ptrnotFinished, *ptrtmp_food, *ptrIsFirst, *savageIndex;
static int shmid2, shmid1, shmid3, shmid4, shmid5,shmid6,shmid7,shmid8;
// prendo una porzione di cibo dalla pentola
int getServingsFromPot(int savage_id)
{
  int retVal;

  //controllo che ci sia cibo rimanente

  if (*ptrfood <= 0) /* se non è rimasto cibo */
  {
    printf("food finished for savage %d \n",savage_id);
    // prende il valore del semaforo e lo salva nella variabile tmp_sem
    int tmp_sem=0;
    //sem_getvalue(&emptyPot,&tmp_sem);

    // se il semaforo ha il valore zero, allora invio il segnale al semaforo. In questo modo non invio segnali multipli al cuoco
    //sem_post(&emptyPot);
    
    // ritorno il valore zero com cibo rimanente
    retVal = 0;
  }
  else /* se e' presente cibo decremento*/
  {
    printf("food in pot %d \n",*ptrfood);
    (*ptrfood)--; /* decremente la variabile food*/
    retVal = *ptrfood;
    printf("food in pot retval%d \n",retVal);
  }
  //ritorno il cibo
  return retVal;
}

//riempe la pentola
int putServingsInPot(int num)
{
  //pthread_mutex_lock(&savage_finish_mutex); /* faccio un lock della variabile savageLeft */
  //int savageLeft = notFinished;             /* inizializzo la variabile savageLeft uguale al numero di selvaggi che non hanno ancora completato tutti i giri*/
  //pthread_mutex_unlock(&savage_finish_mutex); /* sblocco la variabile savageLeft */
  //prima di riempire controllo che la pentola sia vuota e che ci siano ancora selvaggi che devono mangiare.
  printf("before put in pot ");
  if(*ptrfood<=0 && *ptrnotFinished>0){

    *ptrfood = num; /* la varibale foods diventa il numero di porzioni dichiarate in fase iniziale*/

    }else{
    return 0; // ritorno zero se il cuoco non ha aggiunto cibo
  }

  return 1; // ritorno zero se il cuoco ha aggiunto cibo
}

//void *cook(void *id) // processo del cuoco
void cook(int id)
{
  int tmp_sem=0, generalFood = tmp_food;

  printf("\n--------Cook is enter with %i\n\n", *ptrfood);
  //ciclo all'infinito, escluse clausole di break
  while (1)
  {
    sleep(1);
    // lock la variabile foods
    ////pthread_mutex_lock(&servings_mutex);
    //sem_wait(&emptyPot);
    sem_wait(mutex);
    
    int tmp = *ptrfood; // salvo la variabile foods in una temporanea
    sem_post(mutex);
    //unlock la variabile
    ////pthread_mutex_unlock(&servings_mutex);
    //controllo se e' rimasto cibo
    if(tmp>0){ // se e' rimasto cibo

      ////pthread_mutex_lock(&print_mutex);
      sem_getvalue(emptyPot,&tmp_sem);
      printf("\nCook is sleeping with sem %d \n\n",tmp_sem);
      //pthread_mutex_unlock(&print_mutex);
      //il cuoco si addormenta in attesa del segnale di riempimento
      sem_wait(emptyPot);
      printf("\nCook received signal %d food\n\n",tmp);

      sem_getvalue(emptyPot,&tmp_sem);
      printf("\nCook awake with sem %d \n\n",tmp_sem);
      ////pthread_mutex_lock(&savage_finish_mutex); //lock variabile notFinished
      sem_wait(mutex);
      int savageLeft = *ptrnotFinished; 
      sem_post(mutex);               //inizializzo una variabile con il numero dei selvaggi che non hanno ancora finito i giri
      ////pthread_mutex_unlock(&savage_finish_mutex);
        printf("\nCook see %d savage left\n\n",savageLeft);
      if (savageLeft > 0) // se ancora ci sono selvaggi
      {
      
        ////pthread_mutex_lock(&servings_mutex); //lock della variabile foods

        printf("\ncook - before wait row 121 \n");
        sem_wait(mutex);
        printf("\ncook - wait row 121 \n");
        printf("\ncook - general \n");
        
        int hasAdded = putServingsInPot(generalFood); //chiamo la funzione per riempire la pentola passandogli il valore preso a riga di comando, salvo se effetivamente il cuoco ha aggiunto le porzioni
        sem_post(mutex);
        printf("\ncook - unlock wait row 121 \n");
        
        //se il cuoco ha aggiunto porzioni alla pentola
        if(hasAdded == 1){
          //scrivo messaggio che il cuoco ha riempito la pentola
          ////pthread_mutex_lock(&print_mutex);
          printf("\nCook he was woken up and filled pot\n\n");
          ////pthread_mutex_unlock(&print_mutex); //sblocco la variabile foods

          numberOfFillPot++; // incremento il contatore che conta le volte di riempimento
          //for (int i = 1; i < num_savages; i++) //invio il segnale al semaforo di riempiemnto per ogni selvaggio
            sem_post(fullPot);

        }else //altrimenti stampo il messaggio che il cuoco non ha fatto nulla
        {
         // //pthread_mutex_lock(&print_mutex);
          printf("\nCook do nothing\n");
          ////pthread_mutex_unlock(&print_mutex);
        }
        ////pthread_mutex_unlock(&servings_mutex); // sblocco la variabile foods
        
        
        //sleep(1);
        
      }
      else{
        ////pthread_mutex_lock(&print_mutex);
        printf("\n\nAll the savages have eaten, the cook can go home!!\n\n");
        ////pthread_mutex_unlock(&print_mutex);
        return NULL;  /* esco dal ciclo, tutti i selvaggi hanno mangiato*/
      }
    }
    else{
       ////pthread_mutex_lock(&savage_finish_mutex);
       sem_wait(mutex);
       int savageLeft = *ptrnotFinished; 
       sem_post(mutex);               // inizializzo una variaible col numero dei selvaggi che ancora devono finire il giro
       ////pthread_mutex_unlock(&savage_finish_mutex);
       
       if(savageLeft>0){ // controllo se ci sono ancora selvaggi che devono finire il giro

          ////pthread_mutex_lock(&servings_mutex); //lock variabile foods 
          printf("\ncook - before wait row 162 \n");
          sem_wait(mutex);
          printf("\ncook - wait row 162 \n");
          int hasAdded = putServingsInPot(generalFood); // riempio la pentola e inserisco nella variabile hasAdded se ha inserito porzioni nella pentola
          ////pthread_mutex_unlock(&servings_mutex);
          sem_post(mutex);
          printf("\ncook - unlock wait row 162 \n");

          if (hasAdded > 0){ //se il cuoco ha aggiunto porzioni
            ////pthread_mutex_lock(&print_mutex); //lock the console printing
            printf("\nCook see no more food, he cooks and filled pot\n\n");
            ////pthread_mutex_unlock(&print_mutex);
            numberOfFillPot++; // incremento le volte in cui la pentola e' stata riempita
            //for (int i = 1; i < num_savages; i++) //mando il segnale al semaforo fullPot ad ogni selvaggio
              sem_post(fullPot);
              sleep(3);
          }else{ /* altrimenti il cuoco non fa nulla e lo stampo a video*/
            ////pthread_mutex_lock(&print_mutex);
            printf("\nCook do nothing\n");
            ////pthread_mutex_unlock(&print_mutex);
          }

          //sleep(1);
          //unlock the semaphore for each savage
          

           ////pthread_mutex_lock(&print_mutex);
           printf("\n 2 Cook is sleeping\n\n");
           ////pthread_mutex_unlock(&print_mutex);
           // il cuoco aspetta il segnale di pentola vuota
           //sem_wait(&emptyPot);
        }  
        else // se non ci sono selvaggi che devono ancora mangiare il cuoco puo' terminare
        {
          ////pthread_mutex_lock(&print_mutex);
          printf("\n\nAll the savages have eaten, the cook can go home\n\n");
          ////pthread_mutex_unlock(&print_mutex);
          return NULL; // esce dal ciclo
        }
       
    }
  }

  return NULL; //esce dal processo
}

//void *savage(void *id)
void savage(int id)
{
  sleep(2);
  printf("\nbefore wait row 208 \n");
  sem_wait(mutex);
  printf("\nwait row 208 \n"); 
  //id = rand()%13;
  int tmp_sem = 0;
  int tmp_isFirst = *ptrIsFirst;
  // if (tmp_isFirst == 1)
  // {
  //   printf("isfirst %i\n", tmp_isFirst);
  //   *ptrIsFirst==0;
  // }else{
  //   printf("aspetto per isfirst %i\n", tmp_isFirst);
  //   sem_wait(&finishOperation);
  // }
  sem_getvalue(&finishOperation, &tmp_sem);
  printf("shmat processo figlio al segmento di sem: %i\n", tmp_sem);
  //sem_wait(&finishOperation);
  //sem_post(&finishOperation);
  sem_getvalue(&finishOperation, &tmp_sem);
  printf("after shmat processo figlio al segmento di sem: %i\n", tmp_sem);
  //salvo l'id del processo come id del selvaggio
  int savage_id = id;
  
    //sleep(savage_id);

      printf("\nrounds %i\n", *ptrrounds);
  //dichiaro la variabile tmp_rounds che mi servira' per sapere quante volte il selvaggio deve mangiare
  int tmp_rounds = *ptrrounds;
  ////pthread_mutex_lock(&servings_mutex);
  //salvo la variabile foods in una temporanea in modo da non averla valida per il contesto del singolo thread
  int tmp_foods = *ptrfood;
  ////pthread_mutex_unlock(&servings_mutex);
  int myServing = 0;
  printf("\nprima di while %i, savage %d\n",tmp_rounds,id);

  sem_post(mutex); 
  printf("\nunlock wait row 241 \n");
  //ciclo per il numero di volte in cui ogni selvaggio deve mangiare
  while (tmp_rounds)
  {
    printf("\nbefore wait row 245 \n");
    sem_wait(mutex);
    printf("\nwait row 245 \n");
    //sleep(1);
    //faccio un lock sulla variabile foods in modo che altri processi non possano modificarla
    //pthread_mutex_lock(&servings_mutex);
    //sem_wait(&finishOperation);
    tmp_foods = *ptrfood; //inserisco nella variabile temporanea tmp_foods il valore attuale di foods
    sem_post(mutex);
    printf("\nunlock wait row 245 \n");
    if ((tmp_foods <= 0)) // se non ci sono piu' porzioni 
    {
      printf("\nnon ci sono porzioni\n");
      sem_wait(mutex);
      printf("\nwait row 256 \n");
      //getServingsFromPot(savage_id); // cerca di prendere porzioni ma la funzione mandera' il segnale di emptyPot
      printf("\nsavage %i is waiting\n", savage_id);
      //pthread_mutex_unlock(&print_mutex);

      //unlock the variable
      //pthread_mutex_unlock(&servings_mutex); //sblocco la variabile foods prima del WAIT (molto importante)

      //aspetto il segnale del semaforo fullPot che indica il riempimento della pentola
      sem_post(mutex);
      printf("\nunlock wait row 256 \n");
      int tmp_sem=0;
      sem_getvalue(emptyPot,&tmp_sem);
      printf("\nsvage %d before send empty pot to cook sem %d\n",savage_id,tmp_sem);
      sem_post(emptyPot);
      sem_getvalue(emptyPot,&tmp_sem);
      printf("\nsavage %d sended empty pot to cook sem %d\n",savage_id,tmp_sem);
      sleep(3);
      sem_wait(fullPot);

     
    }
    else{ // se ci sono porzioni
      //sem_wait(&mutex);
      printf("\n savage %d ci sono porzioni\n",savage_id);
      sem_wait(mutex);
      printf("\n savage %d wait row 275 \n",savage_id);
      myServing = getServingsFromPot(savage_id); //chiamo la funzione per prendere una porzione dalla pentola e salvo il cibo rimanente per poi stamparlo
      sem_post(mutex);
      printf("\n savage %d unlock wait row 275 \n",savage_id);
      // lock il console printing
      //pthread_mutex_lock(&print_mutex);
      printf("Savage: %i is eating and food remained is %i\n", savage_id, myServing); // indico informazioni sul cibo rimasto e che stia effettivamente mangiando
      printf("Savage: %i is DONE eating\n", savage_id);
      //pthread_mutex_unlock(&print_mutex);

      //unlock the variable
      //pthread_mutex_unlock(&servings_mutex); //sblocco la variabile foods
      //sleep(1);

      tmp_rounds--; //decremento la varibaile in modo da avere in ciclo finito
      
    }
    //sem_post(&finishOperation);
  }
  
  //pthread_mutex_lock(&savage_finish_mutex); // blocco la variabile notFinished
  sem_wait(mutex);
  (*ptrnotFinished)--;  
  printf("...Savage: %i is exiting, left...\n", savage_id,*ptrnotFinished);
  sem_post(mutex);                          // decremento il numero dei selvaggi che ancora devono finire i giri
 
  //pthread_mutex_unlock(&print_mutex);

  
  return NULL; //esco dal processo
  //exit(0);
}

int main(int argc, char *argv[]) 
{
  
  if (argc == 4) //controllo che il numero di parametri sia 4
  {
    num_savages=atoi(argv[1]);  // salvo il primo parametro come numero di selvaggi
    tmp_food = atoi(argv[2]);   // salvo il secondo parametro come numero porzioni disponibili, verra' usata per riempire la pentola dal cuoco
    foods= tmp_food;            // salvo la variabile condivisa foods con il valore di tmp_food
    rounds = atoi(argv[3]);     // salvo il terzo parametro come numero del numero di volte che ogni singolo selvaggio dovra' mangiare
    notFinished=num_savages;    // inizializzo la variabile notFinished con il numero dei selvaggi. Questa varabile verra' decrementata ogni volta che un selvaggio completa i suoi giri
    int i, id[num_savages + 1], status, wpid; 
    pthread_t tid[num_savages + 1]; //inizializzo un numero di thread pari al numero dei selvaggi + il cuoco
    
    
    
    // // inizializzo le mutex locks
    // //pthread_mutex_init(&servings_mutex, NULL);
    // //pthread_mutex_init(&print_mutex, NULL);
    // //pthread_mutex_init(&savage_finish_mutex, NULL);

    /* alloca memoria condivisa per un array di 100 interi e uno di 100 caratteri */

    shmid1 = shmget(IPC_PRIVATE, 100 * sizeof(int), 0600);
    if (shmid1 == -1)
      perror("Creazione memoria condivisa");

    shmid2 = shmget(IPC_PRIVATE, 100 * sizeof(int), 0600);
    if (shmid2 == -1)
      perror("Creazione memoria condivisa");

    shmid3 = shmget(IPC_PRIVATE, 100 * sizeof(int), 0600);
    if (shmid3 == -1)
      perror("Creazione memoria condivisa");

    shmid4 = shmget(IPC_PRIVATE, 100 * sizeof(int), 0600);
    if (shmid4 == -1)
      perror("Creazione memoria condivisa");

    shmid5 = shmget(IPC_PRIVATE, 100 * sizeof(int), 0600);
    if (shmid4 == -1)
      perror("Creazione memoria condivisa");

    shmid6 = shmget(IPC_PRIVATE, sizeof ( sem_t ), 0666);
    if (shmid6 == -1)
      perror("Creazione memoria condivisa");

    shmid7 = shmget(IPC_PRIVATE, sizeof ( sem_t ), 0666);
    if (shmid7 == -1)
      perror("Creazione memoria condivisa");

    shmid8 = shmget(IPC_PRIVATE, sizeof ( sem_t ), 0666);
    if (shmid8 == -1)
      perror("Creazione memoria condivisa");

    ptrfood = (int *)shmat(shmid1, NULL, 0);
    ptrrounds = (int *)shmat(shmid2, NULL, 0);
    ptrnotFinished = (int *)shmat(shmid3, NULL, 0);
    ptrtmp_food = (int *)shmat(shmid4, NULL, 0);
    ptrIsFirst = (int *)shmat(shmid4, NULL, 0);
    savageIndex = (int *)shmat(shmid5, NULL, 0);
    emptyPot = ( sem_t *) shmat ( shmid6 ,( void *) 0, 0) ;
    fullPot = ( sem_t *) shmat ( shmid7 ,( void *) 0, 0) ;
    mutex = ( sem_t *) shmat ( shmid8 ,( void *) 0, 0) ;

    *ptrfood = foods;
    *ptrrounds = rounds;
    *ptrnotFinished = notFinished;
    *ptrtmp_food = tmp_food;
    *ptrIsFirst = 1;
    *savageIndex=0;

        // inizializzo the semaphores
    sem_init(emptyPot, 1, 0);
    sem_init(fullPot, 1, 0);
    // sem_init(&doOperation, 0, 0);
    // sem_init(&finishOperation, 0, 0);
    sem_init(mutex, 1, 1); 

    //creo un thread per ogni selvaggio
    // for (i = 0; i < num_savages; i++)
    // {
    //   id[i] = i;
    //   pthread_create(&tid[i], NULL, savage, (void *)&id[i]);
    // }
    //creo thread per il cuoco
    // pthread_create(&tid[i], NULL, cook, (void *)&id[i]);
    sem_post(&finishOperation);
    int cpid= fork();
    if (cpid == 0)
    {
      int pid;
      int tmp_num_Savages = 0;
      for (i = 0; i < num_savages; i++)
      {
        //pthread_join(tid[i], NULL);
        pid=fork();
        if(pid==0){
          savage(i);
          //printf("\nSavage n %d \n", i);
        }else{
          printf("\nAspetto %d ,food %d\n", i,foods);
          pid = wait(0);
          printf("\nAspettato %d, food %d\n", i,foods);
          tmp_num_Savages++;
        }
        
      }
      if(tmp_num_Savages == num_savages){
        printf("\nFiniti tutti \n", i,foods);
        sem_post(emptyPot);

      }
      // exit(0);
    }
    else
    {
        cook(1000);
        printf("\nCIAO\n");
        //waitpid(cpid,&status,NULL);
        //while ((wpid = wait(&status)) > 0);
        printf("\nFood remained %d \n", *ptrfood);
        printf("The cook fill %d times the pot \n", numberOfFillPot);
        shmctl(shmid1, IPC_RMID, NULL);
        shmctl(shmid2, IPC_RMID, NULL);

      //sem_post(&emptyPot);
      //aspetto che il cuoco finisca
      //pthread_join(tid[i], NULL);
      

      
    }
    // aspetto che ogni selvaggio ritorni
    
    
    
    //pthread_kill(tid[num_savages], 3);
    //send message to cook in order to make it understand he has finished
    //invio un messaggio di riempire la pentola al cuoco, in modo da fargli verificare che non deve riempire ancora e quindi uscire
    // sem_post(&emptyPot);
    // //aspetto che il cuoco finisca
    // //pthread_join(tid[i], NULL);

    // printf("\nFood remained %d \n", foods);
    // printf("The cook fill %d times the pot \n",numberOfFillPot);
    //
    
    }
  else //se i parametri non sono corretti stampo un errore
  {
    printf("Incorrect numbers of paramters. 3 params needed: Number of savages, Number of Portions and Number of Rounds \n");
  }
}