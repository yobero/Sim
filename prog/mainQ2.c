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
long int n=0;
long int ticket=1;
double cumul=0;
int compteur=0;

double rho=0.0;
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

void variableTh(){
  rho = LAMBDA/MU;
}

void theorieE(){
  Eth = rho*((1.0/MU)/(1-rho));
}

void theorieT90(){
  t90th = (Eth/rho)*log(10*rho);
  if(t90th<0) t90th=0;
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
  ticket=1;
  cumul=0;
  compteur=0;
  Eth=0.0;
  rho=0.0;
  t90th=0.0;
}

void initSimulation(){
  initEcheancier();
  initTempsMoy();
  initVariable();
}

event rechercheEvent(long int ticket){
  event e;
  e.ticket=-1;
  for(int i=0;i<ech.taille;i++){
    if(ech.T[i].etat==1 && ech.T[i].type==0 && ech.T[i].ticket==ticket){
      e=ech.T[i];
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

  if(n==1){
    event e2;
    e2.type=1;
    e2.t=e.t+Exponentielle(MU);
    e2.etat=0;
    e2.ticket=e.ticket;
    ajoutEvent(e2);

    //Ce client n'attent pas
    tempsMoy.T[tempsMoy.taille]=0.0;
    tempsMoy.taille++;
  }
  T=e.t;
}

void finService(event e){
  if(n>0){
    n--;
    if(n>0){
      event e1;
      e1.type=1;
      e1.t=e.t+Exponentielle(MU);
      e1.etat=0;
      e1.ticket=e.ticket+1;
      ajoutEvent(e1);

      //le client qui commence le service a attendu
      event p = rechercheEvent(e.ticket+1);
      tempsMoy.T[tempsMoy.taille]=fabs(p.t-e.t);
      tempsMoy.taille++;
    }
  }
  T=e.t;
}

event extrait(){
  long int indiceM=0;
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
  return ech.T[indiceM];
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
  printf("========================taille de l'echeancier : %ld ==== %ld\n", ech.taille,n);
  for(int i=0;i<ech.taille;i++){
    printf("position : %d", i);
    printf(", type : %d", ech.T[i].type);
    printf(", ticket : %ld", ech.T[i].ticket);
    printf(", t : %f",ech.T[i].t);
    printf(", etat : %d\n\n", ech.T[i].etat);
  }
}

void simulation(FILE* resultat){
  initSimulation();

  long double oldNmoyen=0.0;
  long double Nmoyen=0.0;
  //int a=0;
  while(condition_arret(oldNmoyen,Nmoyen)==0){
  //while(a<5){
    //a++;
    event e = extrait();

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
    printf("nombre moyen de client : %f\n",Nmoy);
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

  variableTh();
  theorieE();
  theorieT90();

  fprintf(resultat, "%f %f %f %f %f\n",LAMBDA*NBSERVEUR, E*NBSERVEUR,t90*NBSERVEUR, Eth*NBSERVEUR, t90th*NBSERVEUR );
}

int main(){
  //Fichier ouvert en mode lecture seul, contient les valeurs de lambda
  FILE* f = fopen("lambda.txt","r");
  //Fichier ouvert en mode ecriture, va contenir pour chaque ligne lambda e[A] t90
  FILE* resultat = fopen("resultat2.txt","w");
  srandom(getpid()+ time(NULL));
  if(f!=NULL){
    int l=0;
    int m=0;
    //On lit le fichier lambda.txt tant qu'il reste une valeur à lambda
    printf("Execution du model 2\nValeur de lambda / %d\n", NBSERVEUR);
    do{
      l = fgetc(f);
      m=fgetc(f);
      if(m==10){
        //Bloc la derniere ligne si c'est juste un retour à la ligne
        //48 est la valeur decimal du caractère 0.
        if(l-48>0){
          LAMBDA = l-48;
          LAMBDA=LAMBDA/NBSERVEUR;
          printf("%f\n", LAMBDA);
          simulation(resultat);
        }
      }
      else{
        if(l-48>0 && m-48>=0){
          LAMBDA = (l-48)*10+(m-48);
          LAMBDA=LAMBDA/NBSERVEUR;
          printf("%f \n",LAMBDA );
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
