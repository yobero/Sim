#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

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
  int numServeur; //-1 si type=0 (AC)
  long int ticket;
}event;

typedef struct echeancier{
  event T[MAXEVENT];
  long int taille;
}echeancier;

//Variable globale
int LAMBDA=0;
echeancier ech;
double T=0.0;
long int n=0;
long int ticket=1;
int occ[NBSERVEUR];
double cumul=0;
int compteur=0;

//Fonction
double Exponentielle(int lbda){
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

event extrait(){
  int indiceM=0;
  for(int i=0;i<ech.taille;i++){
    if(ech.T[i].etat==0){
      indiceM=i;
      break;
    }
  }
    for(int i=1;i<ech.taille;i++){
      if(ech.T[indiceM].ticket>ech.T[i].ticket && ech.T[i].etat==0){
        indiceM=i;
      }
    }
    ech.T[indiceM].etat=1;
    return ech.T[indiceM];
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
  e.numServeur=-1;
  e.ticket=0;
  ajoutEvent(e);
}

void initOcc(){
  for(int i=0;i<NBSERVEUR;i++){
    occ[i]=0;
  }
}

void initVariable(){
  T=0.0;
  n=0;
  ticket=1;
  cumul=0;
  compteur=0;
}

void initSimulation(){
  initEcheancier();
  initOcc();
  initVariable();
}

//FOnction pour rechercher un serveur disponible
long int rechercheServeurLibre(){
  for(int i=0;i<NBSERVEUR;i++){
    if(occ[i]==0) return i;
  }
  return -1; //Aucun serveur disponible
}

//AC
void arriveeClient(event e){
  n++;

  //Preparation d'un nouveau AC
  event e1;
  e1.type=0;
  e1.numServeur=-1;
  e1.ticket=ticket;
  ticket++;
  e1.etat=0;
  e1.t=e.t+Exponentielle(LAMBDA);
  ajoutEvent(e1);

  if(n<NBSERVEUR){//Il reste au moins 1 serveur de disponible
    event e2;
    e2.type=1;
    long int m = min(n,NBSERVEUR);
    long int s = rechercheServeurLibre();
    occ[s]=1;
    e2.numServeur=s;
    e2.ticket=e.ticket;
    e2.etat=0;
    e2.t=e.t+Exponentielle(m*MU);
    ajoutEvent(e2);
  }
  T=e.t;
}

//FS
void finService(event e){
  if(n>0){
    n--;
    if(n>NBSERVEUR){
      //Cas ou il y a plus de NBSERVEUR dans la file et donc le serveur liberer peut prendre le prochain client
      int indice=-1;
        for(int i=0;i<ech.taille;i++){
          if(ech.T[i].type==0 && ech.T[i].etat==1){
            int j=0;
            for(j=i+1;j<ech.taille;j++){
              if(ech.T[j].type==1 && ech.T[j].ticket==ech.T[i].ticket)
                break;
            }
            if(j==ech.taille){//ech.T[i] n'a pas de FS associer
            //On peut simplifier ce bloc
              if(indice==-1)indice=i;
              else{
                if(ech.T[indice].ticket>ech.T[i].ticket)
                  indice=i;
              }
            }
          }
        }
        //ech.T[i] est le client ayant le plus petit ticket qui n'est pas en train d'être servie
        event e1;
        e1.type=1;
        e1.ticket=ech.T[indice].ticket;
        e1.t=e.t+Exponentielle(min(n,NBSERVEUR)*MU);
        e1.numServeur=e.numServeur;
        e1.etat=0;

    }
    else{
      occ[e.numServeur]=0;
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

//Fonction simulation
void simulation(FILE* resultat){
  initSimulation();

  long double oldNmoyen=0.0;
  long double Nmoyen=0.0;

  while(condition_arret(oldNmoyen,Nmoyen)==0){
  //while(T<10000){
    event e = extrait();

    cumul+=(e.t-T)*n; //intervalle de temps * le nombre de client dans cette intervalle

    oldNmoyen=Nmoyen;
    Nmoyen=cumul/T;

    if(e.type==0) arriveeClient(e);
    if(e.type==1) finService(e);
  }
  //Ecriture dans le fichier
  //LAMBDA E[A] T90
  double E;
  double t90;
  if(T<TEMPSMAX){
    E=0.0;
    t90=0.0;
  }
  else{
    E=-1;
    t90=-1;
  }
  fprintf(resultat, "%d %f %f\n",LAMBDA, E,t90 );
}

int main(){
  //Fichier ouvert en mode lecture seul, contient les valeurs de lambda
  FILE* f = fopen("lambda.txt","r");
  //Fichier ouvert en mode ecriture, va contenir pour chaque ligne lambda e[A] t90
  FILE* resultat = fopen("resultat1.txt","w");
  srandom(getpid()+ time(NULL));
  if(f!=NULL){
    int l=0;
    //On lit le fichier lambda.txt tant qu'il reste une valeur à lambda
    do{
      l = fgetc(f);
      //Bloc la derniere ligne si c'est juste un retour à la ligne
      //48 est la valeur decimal du caractère 0.
      if(l-48>0){
        LAMBDA = l-48;
        simulation(resultat);
      }
      //On passe le caractère \n (retour à la ligne)
      fgetc(f);
    }
    while(l!=EOF);

    //Fermeture des fichiers
    fclose(f);
    fclose(resultat);
  }
  else{
    printf("Le fichier n'a pas été chargé !!!\n");
  }
}
