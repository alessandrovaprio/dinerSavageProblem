#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
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
//static int myServing = 0;
static int numberOfFillPot = 0;
static int cookHasRounds = 0;
static int tmp_food = 0;
static int notFinished=1;

// prendo una porzione di cibo dalla pentola
int getServingsFromPot(int savage_id)
{
  int retVal;

  //controllo che ci sia cibo rimanente

  if (foods <= 0) /* se non è rimasto cibo */
  {
    
    // prende il valore del semaforo e lo salva nella variabile tmp_sem
    int tmp_sem=0;
    sem_getvalue(&emptyPot,&tmp_sem);

    // se il semaforo ha il valore zero, allora invio il segnale al semaforo. In questo modo non invio segnali multipli al cuoco
    if (tmp_sem ==0){
      sem_post(&emptyPot);
      
    }
    // ritorno il valore zero com cibo rimanente
    retVal = 0;
  }
  else /* se e' presente cibo decremento*/
  {
    foods--; /* decremente la variabile food*/
    retVal = foods;
  }
  //ritorno il cibo
  return retVal;
}

//riempe la pentola
int putServingsInPot(int num)
{
  pthread_mutex_lock(&savage_finish_mutex); /* faccio un lock della variabile savageLeft */
  int savageLeft = notFinished;             /* inizializzo la variabile savageLeft uguale al numero di selvaggi che non hanno ancora completato tutti i giri*/
  pthread_mutex_unlock(&savage_finish_mutex); /* sblocco la variabile savageLeft */
  //prima di riempire controllo che la pentola sia vuota e che ci siano ancora selvaggi che devono mangiare.
  if(foods<=0 && savageLeft>0){

    foods = num; /* la varibale foods diventa il numero di porzioni dichiarate in fase iniziale*/
    
  }else{
    return 0; // ritorno zero se il cuoco non ha aggiunto cibo
  }

  return 1; // ritorno zero se il cuoco ha aggiunto cibo
}

void *cook(void *id) // processo del cuoco
{
  
  
  //ciclo all'infinito, escluse clausole di break
  while (1)
  {
    
    // lock la variabile foods
    pthread_mutex_lock(&servings_mutex);
    int tmp = foods; // salvo la variabile foods in una temporanea
    //unlock la variabile
    pthread_mutex_unlock(&servings_mutex);
    //controllo se e' rimasto cibo
    if(tmp>0){ // se e' rimasto cibo

      pthread_mutex_lock(&print_mutex);
      printf("\nCook is sleeping\n\n");
      pthread_mutex_unlock(&print_mutex);
      //il cuoco si addormenta in attesa del segnale di riempimento
      sem_wait(&emptyPot);

      pthread_mutex_lock(&savage_finish_mutex); //lock variabile notFinished
      int savageLeft = notFinished;             //inizializzo una variabile con il numero dei selvaggi che non hanno ancora finito i giri
      pthread_mutex_unlock(&savage_finish_mutex);
      
      if (savageLeft > 0) // se ancora ci sono selvaggi
      {
      
        pthread_mutex_lock(&servings_mutex); //lock della variabile foods
        int hasAdded = putServingsInPot(tmp_food); //chiamo la funzione per riempire la pentola passandogli il valore preso a riga di comando, salvo se effetivamente il cuoco ha aggiunto le porzioni
        
        //se il cuoco ha aggiunto porzioni alla pentola
        if(hasAdded == 1){
          //scrivo messaggio che il cuoco ha riempito la pentola
          pthread_mutex_lock(&print_mutex);
          printf("\nCook he was woken up and filled pot\n\n");
          pthread_mutex_unlock(&print_mutex); //sblocco la variabile foods

          numberOfFillPot++; // incremento il contatore che conta le volte di riempimento
          for (int i = 1; i < num_savages; i++) //invio il segnale al semaforo di riempiemnto per ogni selvaggio
            sem_post(&fullPot);

        }else //altrimenti stampo il messaggio che il cuoco non ha fatto nulla
        {
          pthread_mutex_lock(&print_mutex);
          printf("\nCook do nothing\n");
          pthread_mutex_unlock(&print_mutex);
        }
        pthread_mutex_unlock(&servings_mutex); // sblocco la variabile foods
        
        
        //sleep(1);
        
      }
      else{
        pthread_mutex_lock(&print_mutex);
        printf("\n\nAll the savages have eaten, the cook can go home!!\n\n");
        pthread_mutex_unlock(&print_mutex);
        return NULL;  /* esco dal ciclo, tutti i selvaggi hanno mangiato*/
      }
    }
    else{
       pthread_mutex_lock(&savage_finish_mutex);
       int savageLeft = notFinished;                // inizializzo una variaible col numero dei selvaggi che ancora devono finire il giro
       pthread_mutex_unlock(&savage_finish_mutex);
       
       if(savageLeft>0){ // controllo se ci sono ancora selvaggi che devono finire il giro

          pthread_mutex_lock(&servings_mutex); //lock variabile foods 
          int hasAdded = putServingsInPot(tmp_food); // riempio la pentola e inserisco nella variabile hasAdded se ha inserito porzioni nella pentola
          pthread_mutex_unlock(&servings_mutex);

          if (hasAdded > 0){ //se il cuoco ha aggiunto porzioni
            pthread_mutex_lock(&print_mutex); //lock the console printing
            printf("\nCook see no more food, he cooks and filled pot\n\n");
            pthread_mutex_unlock(&print_mutex);
            numberOfFillPot++; // incremento le volte in cui la pentola e' stata riempita
            for (int i = 1; i < num_savages; i++) //mando il segnale al semaforo fullPot ad ogni selvaggio
              sem_post(&fullPot);
          }else{ /* altrimenti il cuoco non fa nulla e lo stampo a video*/
            pthread_mutex_lock(&print_mutex);
            printf("\nCook do nothing\n");
            pthread_mutex_unlock(&print_mutex);
          }

          //sleep(1);
          //unlock the semaphore for each savage
          

           pthread_mutex_lock(&print_mutex);
           printf("\nCook is sleeping\n\n");
           pthread_mutex_unlock(&print_mutex);
           // il cuoco aspetta il segnale di pentola vuota
           sem_wait(&emptyPot);
        }  
        else // se non ci sono selvaggi che devono ancora mangiare il cuoco puo' terminare
        {
          pthread_mutex_lock(&print_mutex);
          printf("\n\nAll the savages have eaten, the cook can go home\n\n");
          pthread_mutex_unlock(&print_mutex);
          return NULL; // esce dal ciclo
        }
       
    }
  }

  return NULL; //esce dal processo
}

void *savage(void *id)
{
  //salvo l'id del processo come id del selvaggio
  int savage_id = *(int *)id;
  //sleep(savage_id);

  //dichiaro la variabile tmp_rounds che mi servira' per sapere quante volte il selvaggio deve mangiare 
  int tmp_rounds = rounds;
  pthread_mutex_lock(&servings_mutex);
  //salvo la variabile foods in una temporanea in modo da non averla valida per il contesto del singolo thread
  int tmp_foods = foods;
  pthread_mutex_unlock(&servings_mutex);
  int myServing = 0;

  //ciclo per il numero di volte in cui ogni selvaggio deve mangiare
  while (tmp_rounds)
  {
    //sleep(1);
    //faccio un lock sulla variabile foods in modo che altri processi non possano modificarla
    pthread_mutex_lock(&servings_mutex);
    tmp_foods= foods; //inserisco nella variabile temporanea tmp_foods il valore attuale di foods
    
     if ((tmp_foods <= 0)) // se non ci sono piu' porzioni 
    {
        getServingsFromPot(savage_id); // cerca di prendere porzioni ma la funzione mandera' il segnale di emptyPot

        pthread_mutex_lock(&print_mutex);
        printf("\nsavage %i is waiting\n",savage_id);
        pthread_mutex_unlock(&print_mutex);

        //unlock the variable
        pthread_mutex_unlock(&servings_mutex); //sblocco la variabile foods prima del WAIT (molto importante)
        
        //aspetto il segnale del semaforo fullPot che indica il riempimento della pentola
        sem_wait(&fullPot);

     
    }
    else{ // se ci sono porzioni

      myServing = getServingsFromPot(savage_id); //chiamo la funzione per prendere una porzione dalla pentola e salvo il cibo rimanente per poi stamparlo

      // lock il console printing
      pthread_mutex_lock(&print_mutex);
      printf("Savage: %i is eating and food remained is %i\n", savage_id, myServing); // indico informazioni sul cibo rimasto e che stia effettivamente mangiando
      printf("Savage: %i is DONE eating\n", savage_id);
      pthread_mutex_unlock(&print_mutex);

      //unlock the variable
      pthread_mutex_unlock(&servings_mutex); //sblocco la variabile foods
      //sleep(1);

      tmp_rounds--; //decremento la varibaile in modo da avere in ciclo finito
        
    }
  }
  
  pthread_mutex_lock(&savage_finish_mutex); // blocco la variabile notFinished
  notFinished--;                            // decremento il numero dei selvaggi che ancora devono finire i giri
  pthread_mutex_unlock(&savage_finish_mutex); // sblocco la variabile notFinished

  pthread_mutex_lock(&print_mutex);
  printf("...Savage: %i is exiting ...\n", savage_id);
  pthread_mutex_unlock(&print_mutex);

  
  return NULL; //esco dal processo
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
    int i, id[num_savages + 1]; 
    pthread_t tid[num_savages + 1]; //inizializzo un numero di thread pari al numero dei selvaggi + il cuoco
    
    // inizializzo le mutex locks
    pthread_mutex_init(&servings_mutex, NULL);
    pthread_mutex_init(&print_mutex, NULL);
    pthread_mutex_init(&savage_finish_mutex, NULL);

    // inizializzo the semaphores
    sem_init(&emptyPot, 0, 0);
    sem_init(&fullPot, 0, 0);
    

    //creo un thread per ogni selvaggio
    for (i = 0; i < num_savages; i++)
    {
      id[i] = i;
      pthread_create(&tid[i], NULL, savage, (void *)&id[i]);
    }
    //creo thread per il cuoco
    pthread_create(&tid[i], NULL, cook, (void *)&id[i]);

    // aspetto che ogni selvaggio ritorni
    for (i = 0; i < num_savages; i++)
    {
      pthread_join(tid[i], NULL);
    }
    
    
    
    //pthread_kill(tid[num_savages], 3);
    //send message to cook in order to make it understand he has finished
    //invio un messaggio di riempire la pentola al cuoco, in modo da fargli verificare che non deve riempire ancora e quindi uscire
    sem_post(&emptyPot);
    //aspetto che il cuoco finisca
    pthread_join(tid[i], NULL);

    printf("\nFood remained %d \n", foods);
    printf("The cook fill %d times the pot \n",numberOfFillPot);
    //
    }
  else //se i parametri non sono corretti stampo un errore
  {
    printf("Incorrect numbers of paramters. 3 params needed: Number of savages, Number of Portions and Number of Rounds \n");
  }
}