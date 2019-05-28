#include "tri_fusion.c"

//Constante
#define EPSILON 1e-2
#define MAXEVENT 10000000
#define NBSERVEUR 10
#define MU 10
#define TEMPSMAX 1000000

//Structure
typedef struct evenement{
  int type;
  double t;
  int etat;
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
int LAMBDA=0;
echeancier ech;
double T=0.0;
long int n=0;
long int ticket=1;
long int ticketCourant=0;
tab tempsMoy;
double cumul=0;
int compteur=0;
int indice=0;

//Fonction
double Exponentielle(double lbda){
  double r = (double) random()/RAND_MAX;

  while(r==0 || r==1){
    r = (double) random()/RAND_MAX;
  }
  return -log(r)/(lbda*1.0);
}

long int min(long int n, long int m){
  if(n<m) return n;
  else return m;
}

int extrait(){
  int indiceM;
  for(int i=0;i<ech.taille;i++){
    if(ech.T[i].etat==0){
      indiceM=i;
      break;
    }
  }
    for(int i=0;i<ech.taille;i++){
      if(ech.T[i].etat==0 && ech.T[indiceM].t>ech.T[i].t){
        indiceM=i;
      }
    }
    ech.T[indiceM].etat=1;
    return indiceM;
}

void ajoutEvent(event e){
  if(ech.taille==MAXEVENT){
    printf("echeancier plein !!!\n");
  }
  else{
    ech.T[ech.taille]=e;
    ech.taille++;
  }
}
void initEcheancier(){
  ech.taille=0;
  event e;
  e.type=0;
  e.t=0.0;
  e.etat=0;
  e.ticket=0;
  ajoutEvent(e);
}

void initTempsMoy(){
  for(int i=0;i<MAXEVENT;i++){
    tempsMoy.T[i]=0.0;
  }
  tempsMoy.taille=0;
}

void initVariable(){
  T=0.0;
  n=0;
  ticket=0;
  ticketCourant=0;
  cumul=0;
  compteur=0;
  indice=0;
}

void initSimulation(){
  initEcheancier();
  initTempsMoy();
  initVariable();
}


event rechercherClient(long int ticket){
  event e;
  e.ticket=-1;
  for(int i=0;i<ech.taille;i++){
    if(ech.T[i].etat==1 && ech.T[i].type==0 && ech.T[i].ticket==ticket){
      e=ech.T[i];
      break;
    }
  }
  return e;
}

void arriveeClient(event e){
  n++;
  event e1;
  e1.type=0;
  e1.t=e.t+Exponentielle(LAMBDA);
  e1.etat=0;
  e1.ticket=ticket;
  ajoutEvent(e1);
  ticket++;

  if(n<=NBSERVEUR && n>0){
    event e2;
    e2.type=1;
    e2.t=e.t+Exponentielle(MU*n);
    e2.etat=0;
    e2.ticket=e.ticket;
    ajoutEvent(e2);

    //Ce client n'attend pas
    tempsMoy.T[tempsMoy.taille]=0.0;
    tempsMoy.taille++;
  }
  T=e.t;
}

//FS
void finService(event e){
  if(n>0){
    n--;
    if(n>=NBSERVEUR){
      //printf("BONJOUR\n");
      //Cas ou il y a plus de NBSERVEUR clients dans la file et donc le serveur liberer peut prendre le prochain client
      //grâce a e on connait le serveur disponible
      event e1;
      e1.type=1;
      e1.t=e.t+Exponentielle(MU*NBSERVEUR);
      e1.ticket=ticketCourant;
      ticketCourant++;
      e1.etat=0;
      ajoutEvent(e1);

      event p = rechercherClient(ticketCourant);
      tempsMoy.T[tempsMoy.taille]=fabs(p.t-e.t);
      tempsMoy.taille++;
    }
  }
  T=e.t;
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

void etatEch(){
  printf("========================taille de l'echeancier : %ld ========= %ld\n", ech.taille,n);
  for(int i=0;i<ech.taille;i++){
    printf("position : %d", i);
    printf(", type : %d", ech.T[i].type);
    printf(", ticket : %ld", ech.T[i].ticket);
    printf(", t : %f",ech.T[i].t);
    printf(", etat : %d\n\n", ech.T[i].etat);
  }
}

//Fonction simulation
void simulation(FILE* resultat){
  initSimulation();

  long double oldNmoyen=0.0;
    long double Nmoyen=0.0;


  while(condition_arret(oldNmoyen,Nmoyen)==0){
  //while(n<100){
    indice=extrait();
    event e = ech.T[indice];
    //printf("%d\n",n);

    cumul+=(e.t-T)*n; //intervalle de temps * le nombre de client dans cette intervalle

    oldNmoyen=Nmoyen;
    Nmoyen=cumul/T;

    if(e.type==0) arriveeClient(e);
    if(e.type==1) finService(e);
    //etatEch();
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
    t90=0.0;
  }
  else{
    Nmoy=-1;
    E=-1;
    t90=-1;
  }
  fprintf(resultat, "%d %f %f %f\n",LAMBDA, Nmoy, E, t90);
}

int main(){
  //Fichier ouvert en mode lecture seul, contient les valeurs de lambda
  FILE* f = fopen("lambda.txt","r");
  //Fichier ouvert en mode ecriture, va contenir pour chaque ligne lambda e[A] t90
  FILE* resultat = fopen("resultat1.txt","w");
  srandom(getpid()+ time(NULL));
  if(f!=NULL){
    int l=0;
    int m=0;
    //On lit le fichier lambda.txt tant qu'il reste une valeur à lambda
    printf("Execution du model 1\nValeur de lambda :\n");
    do{
      l = fgetc(f);
      m=fgetc(f);
      if(m==10){
        //Bloc la derniere ligne si c'est juste un retour à la ligne
        //48 est la valeur decimal du caractère 0.
        if(l-48>0){
          LAMBDA = l-48;
          printf("%d\n", LAMBDA);
          simulation(resultat);
        }
      }
      else{
        if(l-48>0 && m-48>=0){
          LAMBDA = (l-48)*10+(m-48);
          printf("%d\n",LAMBDA );
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
