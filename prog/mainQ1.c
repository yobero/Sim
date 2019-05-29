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
double T=0.0;
long int n=0;
long int ticket=1;
long int ticketCourant=0;
tab tempsMoy;
double cumul=0;
int compteur=0;

double rho = 0.0;
double Eth=0.0;
double t90th=0.0;
double p0=0.0;
double q=0.0;

//Fonction
double Exponentielle(double lbda){
  double r = (double) random()/RAND_MAX;

  while(r==0 || r==1){
    r = (double) random()/RAND_MAX;
  }
  return -log(r)/(lbda*1.0);
}

double factoriel(double v){
  double res=1;
  for(int i=v;i>0;i--){
    res=res*i;
  }
  return res;
}

void variableTh(){
  double s;
  double d;
  rho = LAMBDA/(NBSERVEUR*MU);
  for(int i=1;i<NBSERVEUR;i++){
    s+=(pow(NBSERVEUR*rho,i))/factoriel(i);
  }
  d=1+((pow(NBSERVEUR*rho,NBSERVEUR))/factoriel(NBSERVEUR)*(1-rho));

  p0 = pow(d+s,-1);

  q = ((pow(NBSERVEUR*rho,NBSERVEUR))/(factoriel(NBSERVEUR)*(1-rho)))*p0;
}

void theorieE(){
  Eth=q/(NBSERVEUR*MU*(1-rho));
}

void theorieT90(){
  t90th=(Eth/q)*log(10*q);
  if(t90th<0) t90th=0;
}

//Fonction pour ajouter un event dans l'echeancier
//tester et OK
void ajoutEvent(event e){
  ech.T[ech.taille]=e;
  ech.taille++;
}

void initVariable(){
  n=0;
  T=0.0;
  ticket=1;
  ticketCourant=0;
  cumul=0;
  compteur=0;
  Eth=0.0;
  t90th=0.0;
  rho=0.0;
}

void initTempsMoy(){
  for(int i=0;i<MAXEVENT;i++){
    tempsMoy.T[i]=0.0;
  }
  tempsMoy.taille=0;
}

void initEcheancier(){
  ech.taille=0;
  event e;
  e.type=0;
  e.t=0.0;
  e.ticket=0;
  e.etat=0;
  ajoutEvent(e);
}

void initSimulation(){
  initVariable();
  initTempsMoy();
  initEcheancier();
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

event rechercheEvent(long int ticket){
  int indice=0;
  for(int i=0;i<ech.taille;i++){
    if(ech.T[i].etat==1 && ech.T[i].ticket==ticket){
      indice=i;
      break;
    }
  }
  return ech.T[indice];
}

void arriveeClient(event e){
  n++;

  //Preparation de l'arrivée d'un nouveau client
  event e1;
  e1.type=0;
  e1.t=e.t+Exponentielle(LAMBDA);
  e1.ticket=ticket;
  e1.etat=0;
  ajoutEvent(e1);
  ticket++;

  //Si la condition est valide alors il reste au moins un serveur disponible
  if(n<=NBSERVEUR){
    event e2;
    e2.type=1;
    e2.t=e.t+Exponentielle(n*MU);
    e2.ticket=e.ticket;
    e2.etat=0;
    ajoutEvent(e2);

    //On passe au client suivant qui n'a pas encore de FS
    ticketCourant++;
  }
  T=e.t;
}

void finService(event e){
  if(n>0){
    n--;
    event ac = rechercheEvent(e.ticket);
    tempsMoy.T[tempsMoy.taille]=fabs(ac.t-e.t);
    tempsMoy.taille++;

    if(n>=NBSERVEUR){
      event e1;
      e1.type=1;
      e1.t=e.t+Exponentielle(NBSERVEUR*MU);
      e1.ticket=ticketCourant;
      e1.etat=0;
      ajoutEvent(e1);
      ticketCourant++;
    }
  }
  T=e.t;
}

//Fonction pour cherche le prochain evenement
//Tester et OK
event extrait(){
  int indice;
  for(int i=0;i<ech.taille;i++){
    if(ech.T[i].etat==0){
      indice=i;
      break;
    }
  }
  for(int i=0;i<ech.taille;i++){
    if(ech.T[i].etat==0 && ech.T[i].t<ech.T[indice].t)
      indice=i;
  }
  ech.T[indice].etat=1;
  return ech.T[indice];
}

void simulation(FILE* resultat){
  initSimulation();

  long double oldNmoyen=0.0;
  long double Nmoyen=0.0;

  while(condition_arret(oldNmoyen,Nmoyen)==0){
    event e = extrait();

    cumul+=(e.t-T)*n; //intervalle de temps * le nombre de client dans cette intervalle

    oldNmoyen=Nmoyen;
    Nmoyen=cumul/T;

    if(e.type==0) arriveeClient(e);
    if(e.type==1) finService(e);
    if(n>10) break;
  }
  double E=0.0;
  double t90=0.0;
  if(T<TEMPSMAX){
    //Calcul de E[A]
    for(int i=0;i<tempsMoy.taille;i++){
      E+=tempsMoy.T[i];
    }
    E=E/tempsMoy.taille;

    //Calcul de t901
    tri_fusion(tempsMoy.T,tempsMoy.taille);
    int w = tempsMoy.taille*90/100;
    t90=tempsMoy.T[w];
  }
  else{
    E=-1;
  }

  //Partie theorique
  variableTh();
  theorieE();
  theorieT90();

  fprintf(resultat, "%f %f %f %f %f\n",LAMBDA, E,t90, Eth, t90th );
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
