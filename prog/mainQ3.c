#include "tri_fusion.c"

//Constante
#define EPSILON 1e-3
#define MAXEVENT 10000000
#define NBSERVEUR 10
#define MU 10
#define TEMPSMAX 1000000

//Structure
typedef struct evenement{
  int type;
  double t;
  int etat;
  int serveur;
  long int ticket;
}event;

typedef struct echeancier{
  event T[MAXEVENT];
  long int taille;
}echeancier;

typedef struct tableau{
  double T[MAXEVENT];
  long int taille;
}tab;

//Variable globale
double LAMBDA=0.0;
echeancier ech;
tab tempsMoy;
double T=0.0;
long int n[NBSERVEUR];
long int N=0;
long int ticket[NBSERVEUR];
int occ[NBSERVEUR];
double cumul=0;
int compteur=0;

double Eth=0.0;
double t90th=0.0;

//Fonction
void ajoutEvent(event e){
  if(ech.taille==MAXEVENT){
    printf("echeancier plein !!!\n");
  }
  else{
    ech.T[ech.taille]=e;
    ech.taille++;
  }
}

double Exponentielle(double lbda){
  double r = (double) random()/RAND_MAX;

  while(r==0 || r==1){
    r = (double) random()/RAND_MAX;
  }
  return -log(r)/(lbda*1.0);
}

void initEcheancier(){
  ech.taille=0;
  event e;
  e.type=0;
  e.t=0.0;
  e.etat=0;
  e.ticket=-1;
  e.serveur=-1;
  ajoutEvent(e);
}

void initN(){
  for(int i=0;i<NBSERVEUR;i++){
    n[i]=0;
  }
}

void initTempsMoy(){
  for(int i=0;i<MAXEVENT;i++){
    tempsMoy.T[i]=0.0;
  }
  tempsMoy.taille=0;
}

void initTicket(){
  for(int i=0;i<NBSERVEUR;i++){
    ticket[i]=0;
  }
}

void initVariable(){
  T=0.0;
  N=0;
  cumul=0;
  compteur=0;
}

void initSimulation(){
  initEcheancier();
  initN();
  initTempsMoy();
  initTicket();
  initVariable();
}

//Fonction pour savoir qu'elle est la file contenant le moins de client
long int fileM(){
  long int indice=0;
  for(int i=1;i<NBSERVEUR;i++){
    if(n[indice]>n[i])
      indice=i;
  }
  return indice;
}

event rechercheEvent(long int ticket, int s){
  event e;
  e.ticket;
  for(int i=0;i<ech.taille;i++){
    if(ech.T[i].etat==1 && ech.T[i].type==0 && ech.T[i].ticket==ticket && ech.T[i].serveur==s){
      e=ech.T[i];
    }
  }
  return e;
}

void arriveeClient(int i){
  event e=ech.T[i];
  int indiceM = fileM();
  N++;
  n[indiceM]++;
  e.ticket=ticket[indiceM];
  e.serveur=indiceM;
  ticket[indiceM]++;

  event e1;
  e1.type=0;
  e1.t=e.t+Exponentielle(LAMBDA);
  e1.etat=0;
  e1.ticket=-1; //on ne sait pas dans quel serveur ira le client
  e1.serveur=-1;
  ajoutEvent(e1);

  if(n[indiceM]==1){
    event e2;
    e2.type=1;
    e2.t=e.t+Exponentielle(MU);
    e2.etat=0;
    e2.ticket=e.ticket;
    e2.serveur=e.serveur;
    ajoutEvent(e2);
  }
  ech.T[i]=e;
  T=e.t;
}

void finService(int i){
  event e = ech.T[i];
  int serveur=e.serveur;
  if(n[serveur]>0){
    N--;
    n[serveur]--;
    event p = rechercheEvent(e.ticket,serveur);
    //printf(" type  %d ticket %ld temps %f     event fin %ld temps %f\n",p.type,p.ticket,p.t,e.ticket,e.t );
    tempsMoy.T[tempsMoy.taille]=fabs(p.t-e.t);
    tempsMoy.taille++;
    if(n[serveur]>0){
      event e1;
      e1.type=1;
      e1.t=e.t+Exponentielle(MU);
      e1.serveur=serveur;
      e1.ticket=e.ticket+1;
      e1.etat=0;
      ajoutEvent(e1);
    }
  }
  T=e.t;
}

int extrait(){
  long int indiceM;
  for(int i=0;i<ech.taille;i++){
    if(ech.T[i].etat==0){
      indiceM=i;
      break;
    }
  }

  for(int i=0;i<ech.taille;i++){
    if(ech.T[i].etat==0 && ech.T[i].t<ech.T[indiceM].t){
      indiceM=i;
    }
  }
  ech.T[indiceM].etat=1;
  return indiceM;
}

int condition_arret (long double old, long double new){
  if(fabs(old-new)<EPSILON && T>1000){
    compteur++;
  }
  if(compteur>1e3){
    return 1;
  }
  if(T>TEMPSMAX) return 1;
  return 0;
}



void simulation(FILE* resultat){
  initSimulation();

  long double oldNmoyen=0.0;
  long double Nmoyen=0.0;

  //int a=0;

  while(condition_arret(oldNmoyen,Nmoyen)==0){
  //while(a<1000){
    //a++;
    int i = extrait();

    cumul+=(ech.T[i].t-T)*N; //intervalle de temps * le nombre de client dans cette intervalle

    oldNmoyen=Nmoyen;
    Nmoyen=cumul/T;

    if(ech.T[i].type==0) arriveeClient(i);
    if(ech.T[i].type==1) finService(i);

  }
  //Ecriture dans le fichier
  //LAMBDA Nmoy E[A] T90
  double Nmoy;
  double E;
  double t90;
  if(T<TEMPSMAX){
    Nmoy=cumul/T;
    for(int i=0;i<tempsMoy.taille;i++){
      E+=tempsMoy.T[i];
    }
    E=E/tempsMoy.taille;

    tri_fusion(tempsMoy.T,tempsMoy.taille);
    t90=tempsMoy.T[(tempsMoy.taille*90)/100];
  }
  else{
    Nmoy=-1;
    E=-1;
    t90=-1;
  }
  fprintf(resultat, "%f %f %f %f %f\n",LAMBDA, E,t90,Eth,t90th);

  /*for(int i=0;i<tempsMoy.taille/4;i=i+4){
    printf("%f  %f  %f  %f\n",tempsMoy.T[i],tempsMoy.T[i+1], tempsMoy.T[i+2],tempsMoy.T[i+3] );
  }*/

}

int main(){
  //Fichier ouvert en mode lecture seul, contient les valeurs de lambda
  FILE* f = fopen("lambda.txt","r");
  //Fichier ouvert en mode ecriture, va contenir pour chaque ligne lambda e[A] t90
  FILE* resultat = fopen("resultat3.txt","w");
  srandom(getpid()+ time(NULL));
  if(f!=NULL){
    int l=0;
    int m=0;
    //On lit le fichier lambda.txt tant qu'il reste une valeur à lambda
    printf("Execution du model 3\nValeur de lambda :\n");
    do{
      l = fgetc(f);
      m=fgetc(f);
      if(m==10){
        //Bloc la derniere ligne si c'est juste un retour à la ligne
        //48 est la valeur decimal du caractère 0.
        if(l-48>0){
          LAMBDA = l-48;
          printf("%f\n", LAMBDA);
          simulation(resultat);
        }
      }
      else{
        if(l-48>0 && m-48>=0){
          LAMBDA = (l-48)*10+(m-48);
          printf("%f\n",LAMBDA );
          simulation(resultat);
        }
        //On passe le caractère \n (retour à la ligne)
        fgetc(f);
      }
    }
    while(l!=EOF && m!=EOF);

    //Fermeture des fichiers
    fclose(f);
    fclose(resultat);
  }
  else{
    printf("Le fichier n'a pas été chargé !!!\n");
  }
}
