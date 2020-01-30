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
sem_t *mutex;

void cook();
void savage(int id);


static int num_savages =0;
static int foods = 0;
static int rounds=0;
//static int myServing = 0;
static int numberOfFillPot = 0;
static int cookHasRounds = 0;
static int tmp_food = 0;
static int notFinished=1;
static int *ptrfood, *ptrrounds, *ptrnotFinished;
static int shmid2, shmid1, shmid3, shmid4, shmid5,shmid6,shmid7,shmid8;
// prendo una porzione di cibo dalla pentola
int getServingsFromPot(int savage_id)
{
  int retVal;

  //controllo che ci sia cibo rimanente

  if (*ptrfood <= 0) /* se non è rimasto cibo */
  {
    printf("cibo finito per il selvaggio %d \n",savage_id);
    // prende il valore del semaforo e lo salva nella variabile tmp_sem
    
    // ritorno il valore zero com cibo rimanente
    retVal = 0;
  }
  else /* se e' presente cibo decremento*/
  {
    (*ptrfood)--; /* decremente la variabile food*/
    retVal = *ptrfood;
  }
  //ritorno il cibo
  return retVal;
}

//riempe la pentola
int putServingsInPot(int num)
{
  //prima di riempire controllo che la pentola sia vuota e che ci siano ancora selvaggi che devono mangiare.
  //printf("before put in pot %d",*ptrfood);
  if(*ptrfood<=0 && *ptrnotFinished>0){

    *ptrfood = num; /* la varibale foods diventa il numero di porzioni dichiarate in fase iniziale*/

    }else{
    return 0; // ritorno zero se il cuoco non ha aggiunto cibo
  }

  return 1; // ritorno 1 se il cuoco ha aggiunto cibo
}

//void *cook(void *id) // processo del cuoco
void cook()
{
  int tmp_sem=0, generalFood = tmp_food;
  //ciclo all'infinito, escluse clausole di break
  while (1)
  {
    //sleep(1);
    // lock la variabile foods
    ////pthread_mutex_lock(&servings_mutex);
    //sem_wait(&emptyPot);
    sem_wait(mutex); //entro nella sezione critica e aspetto per avere accesso esclusivo
    
    int tmp = *ptrfood; // salvo la variabile foods in una temporanea
    sem_post(mutex); //rilascio zona critica
    
    //controllo se e' rimasto cibo
    if(tmp>0){ // se e' rimasto cibo

      printf("\nCuoco sta dormento \n\n");
      //il cuoco si addormenta in attesa del segnale di riempimento
      sem_wait(emptyPot);

      sem_wait(mutex); //aspetto di entrare nella zona critica
      printf("\nCuoco è stato svegliato \n\n");
      int savageLeft = *ptrnotFinished; 
      sem_post(mutex);               //inizializzo una variabile con il numero dei selvaggi che non hanno ancora finito i giri
      if (savageLeft > 0) // se ancora ci sono selvaggi
      {
      
        ////pthread_mutex_lock(&servings_mutex); //lock della variabile foods

        sem_wait(mutex);
        
        int hasAdded = putServingsInPot(generalFood); //chiamo la funzione per riempire la pentola passandogli il valore preso a riga di comando, salvo se effetivamente il cuoco ha aggiunto le porzioni
        sem_post(mutex);
        
        //se il cuoco ha aggiunto porzioni alla pentola
        if(hasAdded == 1){

          numberOfFillPot++; // incremento il contatore che conta le volte di riempimento
          sem_post(fullPot);

        }else //altrimenti stampo il messaggio che il cuoco non ha fatto nulla
        {
          printf("\nCuoco non fa nulla\n");
        }
        
        
        //sleep(1);
        
      }
      else{
        printf("\n\nTutti i selvaggi hanno mangiato, il cuoco può andare a casa!!\n\n");
        return;
      }
    }
    else{
       sem_wait(mutex);
       int savageLeft = *ptrnotFinished; // inizializzo una variaible col numero dei selvaggi che ancora devono finire il giro 
       sem_post(mutex);               
       
       if(savageLeft>0){ // controllo se ci sono ancora selvaggi che devono finire il giro

          
          sem_wait(mutex); //aspetto per entrare nella sezione critica
          int hasAdded = putServingsInPot(generalFood); // riempio la pentola e inserisco nella variabile hasAdded se ha inserito porzioni nella pentola
          sem_post(mutex); //rilascio sezione critica

          if (hasAdded > 0){ //se il cuoco ha aggiunto porzioni
            printf("\nil cuoco vede pentola vuota, cucina e la riempie\n\n");
            numberOfFillPot++; // incremento le volte in cui la pentola e' stata riempita
            
            sem_post(fullPot); //segnalo ai selvaggi che aspettano che la pentola è di nuovo piena
              //sleep(3);
          }else{ /* altrimenti il cuoco non fa nulla e lo stampo a video*/
            printf("\nIl cuoco non fa nulla\n");
          }

          //sleep(1);
        }  
        else // se non ci sono selvaggi che devono ancora mangiare il cuoco puo' terminare
        {
          printf("\n\nTutti i selvaggi hanno mangiato, il cuoco può andare a casa\n\n"); 
        }
       
    }
  }
  exit(0);
  //return NULL; //esce dal processo
}

//void *savage(void *id)
void savage(int id)
{
  
  sem_wait(mutex); //entro zona critica
  int savage_id = id;
  
  //dichiaro la variabile tmp_rounds che mi servira' per sapere quante volte il selvaggio deve mangiare
  int tmp_rounds = *ptrrounds;

  //salvo la variabile foods in una temporanea in modo da non averla valida per il contesto del singolo processo
  int tmp_foods = *ptrfood;
  
  int myServing = 0;

  sem_post(mutex);  //rilascio zona critica
  //ciclo per il numero di volte in cui ogni selvaggio deve mangiare
  while (tmp_rounds)
  {
    sem_wait(mutex);
    
    tmp_foods = *ptrfood; //inserisco nella variabile temporanea tmp_foods il valore attuale di foods
    
    sem_post(mutex); //rilascio zona critica
    
    if ((tmp_foods <= 0)) // se non ci sono piu' porzioni 
    {
      printf("\nnon ci sono porzioni\n");
      sem_wait(mutex);
      
      //getServingsFromPot(savage_id); // cerca di prendere porzioni ma la funzione mandera' il segnale di emptyPot
      printf("\nil selvaggio %i chiama il cuoco e aspetta\n", savage_id);
      
      sem_post(mutex); //rilascio zona critica
      
      int tmp_sem=0;
      // sem_getvalue(emptyPot,&tmp_sem);
      // printf("\nsvage %d before send empty pot to cook sem %d\n",savage_id,tmp_sem);
      // sem_getvalue(emptyPot,&tmp_sem);
      // printf("\nsavage %d sended empty pot to cook sem %d\n",savage_id,tmp_sem);
      //sleep(3);
      sem_post(emptyPot);
      sem_wait(fullPot);  // aspetto che il cuoco dia il segnale di pentola riempita

     
    }
    else{ // se ci sono porzioni
      
      //printf("\n savage %d ci sono porzioni\n",savage_id);
      sem_wait(mutex); //entro zona critica 
      //printf("\n savage %d wait row 275 \n",savage_id);
      myServing = getServingsFromPot(savage_id); //chiamo la funzione per prendere una porzione dalla pentola e salvo il cibo rimanente per poi stamparlo
      sem_post(mutex);
      //printf("\n savage %d unlock wait row 275 \n",savage_id);
      // lock il console printing
     
      printf("Selvaggio %i sta mangiando, porzioni rimaste in pentola %i\n", savage_id, myServing); // indico informazioni sul cibo rimasto e che stia effettivamente mangiando
      printf("Selvaggio %i ha mangiato\n", savage_id);
     
     

      tmp_rounds--; //decremento la varibaile in modo da avere in ciclo finito
      
    }
    //sem_post(&finishOperation);
  }
  
  //pthread_mutex_lock(&savage_finish_mutex); // blocco la variabile notFinished
  sem_wait(mutex);
  (*ptrnotFinished)--;  //decremento variabile che tiene conto dei selvaggi che devono ancora mangiare
  printf("...Selvaggio: %i esce, rimanenti...\n", savage_id,*ptrnotFinished);
  sem_post(mutex);                          
 
  exit(0);
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
    emptyPot = ( sem_t *) shmat ( shmid6 ,( void *) 0, 0) ;
    fullPot = ( sem_t *) shmat ( shmid7 ,( void *) 0, 0) ;
    mutex = ( sem_t *) shmat ( shmid8 ,( void *) 0, 0) ;

    *ptrfood = foods;
    *ptrrounds = rounds;
    *ptrnotFinished = notFinished;

    // inizializzo i semafori condivisi () secondo parametro a 1)
    sem_init(emptyPot, 1, 0);
    sem_init(fullPot, 1, 0);
    sem_init(mutex, 1, 1); 

    
    int cpid= fork();
    if (cpid == 0)
    {
      int pid;
      int tmp_num_Savages = 0;
      for (i = 0; i < num_savages; i++)
      {
        pid=fork(); //creo un processo per ogni selvaggio
        if(pid==0){
          savage(i);
        }else{
          //printf("\nAspetto %d ,food %d\n", i,foods);
          pid = wait(0);
          //printf("\nAspettato %d, food %d\n", i,foods);
          tmp_num_Savages++;
        }
        
      }
      if(tmp_num_Savages == num_savages){
        sem_post(emptyPot);

      }
      // exit(0);
    }
    else
    {
        cook();
      
        printf("\nCibo avanzato %d \n", *ptrfood);
        printf("Il cuoco ha riempito %d volte la pentola \n", numberOfFillPot);
        sem_destroy(emptyPot); 
        sem_destroy(fullPot);
        sem_destroy(mutex); 
        shmctl(shmid1, IPC_RMID, NULL);
        shmctl(shmid2, IPC_RMID, NULL);
        shmctl(shmid3, IPC_RMID, NULL);
        shmctl(shmid6, IPC_RMID, NULL);
        shmctl(shmid7, IPC_RMID, NULL);
        shmctl(shmid8, IPC_RMID, NULL);

    
      

      
    }
    // aspetto che ogni selvaggio ritorni
    
    }
  else //se i parametri non sono corretti stampo un errore
  {
    printf("Incorrect numbers of paramters. 3 params needed: Number of savages, Number of Portions and Number of Rounds \n");
  }
}