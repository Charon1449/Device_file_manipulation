#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>


typedef struct chaine chaine;
struct chaine
{
  char nom[50];
};

void hexDump(const char *desc, const void *addr, const int len);
void Liste_Disques();
void Lire_secteur(const char *disque_physique, int Num_sect, unsigned char *sector);
void Afficher_secteur(const char *disque_physique, int Num_sect);
int cluster_suivant(FILE* disk,int cluster_courant,int debut_fat);
void Afficher_Fdel(char nom_disque[],int num_partition);


int main()

{
    printf("\n\n\t\tESI –1CS                             Juin 2020\n");
    printf("\n\n\t\t        TP de Systèmes d’exploitation      \n");
    printf("\n\t\t     Designation des disques sous Linux\n");
    printf("\n\n\t\tBinome :\n");
    printf("\n\t\t-Deroues Nawfel(G07)   -Yaici Walid(G07)\n");
    printf("\t\t hn_deroues@esi.dz      hw_yaici@esi.dz\n\n\n");

    int choice, num, i;
    char disque[15];
    unsigned long int fact;
    char disk_path[40];

    while (1)
    {
        printf("Choisissez une fonction: \n\n");
        printf("1. Liste_Disques() :Affiche la liste des disques physiques connectes   \n");
        printf("2. Afficher_secteur (disque_physique, Num_sect): lire le secteur Num_sect du disque disque_physique et affiche son contenu, en hexadecimal.\n");
        printf("3. Afficher_Fdel(disque_physique, partition) : Afficher les fichiers/répertoires de la partion spécifiée en entrée de type FAT32\n");
        printf("4. Exit\n\n\n");
        printf("Enter your choice :  ");
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
            printf("\n----------------------------------------\n");
            Liste_Disques();
            printf("----------------------------------------\n\n");
            break;

        case 2:
            printf("\n----------------------------------------\n");
            printf("Donner le numero du secteur:\n");
            scanf("%d", &num);
            printf("Donner le nom du disque:\n");
            scanf("%s", disque);
            Afficher_secteur(disque, num);
            printf("----------------------------------------\n\n");
            break;

        case 3:
            printf("\n----------------------------------------\n");
            printf("Donner le numero de la partition:\n");
            scanf("%d", &num);
            printf("Donner le nom du disque:\n");
            scanf("%s", disque);
            strcpy(disk_path, "/dev/");
            strcat(disk_path, disque);
            Afficher_Fdel(disk_path ,num);
            printf("\n----------------------------------------\n\n");
            break;

        case 4:
            exit(0);
        }
    }
    return 0;
}

void hexDump(const char *desc, const void *addr, const int len)
{
    int i;
    unsigned char buff[17];
    const unsigned char *pc = (const unsigned char *)addr;

    if (desc != NULL)
        printf("%s bn\n", desc);

    if (len == 0)
    {
        printf("  TIALE ZERO\n"); //la taille du secteur est de 0
        return;
    }
    else if (len < 0)
    {
        printf("  TAILLE NEGATIVE : %d\n", len); //la taille du secteur est nagative
        return;
    }
    for (i = 0; i < len; i++)
    {
        if ((i % 16) == 0)
        {
            if (i != 0)
                printf("  %s\n", buff); //On affiche le contenue en hexa
            printf("  %04x ", i);
        }
        printf(" %02x", pc[i]); //affichage du offset

        if ((pc[i] < 0x20) || (pc[i] > 0x7e)) //on affichle les carectere ASSCI
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0'; //saut de ligne si on depace les 16 colone
    }

    while ((i % 16) != 0)
    {
        printf("   ");
        i++;
    }

    printf("  %s\n", buff);
}
void Lire_secteur(const char *disque_physique, int Num_sect, unsigned char *sector)
{
    FILE *disk;
    char disk_path[40];
    strcpy(disk_path, "/dev/");
    strcat(disk_path, disque_physique); //creation du chemin vers le fichier du disque dans /dev
    disk = fopen(disk_path, "r");
    if ((disk == NULL))
    {
        printf("Fichier non ouvert \n");
        return;
    }
    if (fseek(disk, 512 * Num_sect, SEEK_SET) == 0) //on choisie l'offset du secteur
        fread(sector, 512, 1, disk);                //on lit un secteur de 512 bytes
}
void Afficher_secteur(const char *disque_physique, int Num_sect)
{
    unsigned char sec[512];
    char num[2];
    sprintf(num, "%d", Num_sect);
    Lire_secteur(disque_physique, Num_sect, sec); //on lit le secteur
    printf("Lecture du secteur numero %d du disque %s :\n\n", Num_sect, disque_physique);
    hexDump("offset|", sec, sizeof(sec)); //on utilise la fonction hexDump pour avoir l'affichage
}
void Liste_Disques()
{
    struct dirent *de, *dpart;
    DIR *dr = opendir("/dev");
    DIR *dprt = opendir("/dev");
    if (dr == NULL)
    {
        printf("Ouverture du dossier impossible");
        return;
    }
    printf("Les disques disponibles sont : \n");
    while ((de = readdir(dr)) != NULL)
    {
        if ((strstr(de->d_name, "sd") != NULL) && (strlen(de->d_name) < 4))
        {
            printf("\tdisque :%s\n", de->d_name);
            DIR *dprt = opendir("/dev");
            int i = 1;
            while ((dpart = readdir(dprt)) != NULL)
            {
                if ((strstr(dpart->d_name, de->d_name) != NULL) && (strlen(dpart->d_name) > 3))
                {
                    if (i == 1)
                        printf("\tses partitions:\n");
                    printf("\t  %s\n", dpart->d_name);
                    i = 0;
                }
            }
            printf("\n");
        }
    }
    closedir(dr);
}
int cluster_suivant(FILE* disk,int cluster_courant,int debut_fat){//retourne le cluster suivant du numero donné en parametre(cluster_courant)
  int result;
  fseek(disk,debut_fat*512+(cluster_courant)*4,SEEK_SET);
  fread(&result,4,1,disk);
  if(result>=0x0FFFFFF8) result=-1;//dernier cluster de la chaine
  return result;
}
void Afficher_Fdel(char nom_disque[],int num_partition){
  //Afficher les fichiers/répertoires de la partition spécifiée en entrée de type FAT32 en donnant pour chacun le nom en format
  //court, sa taille en octets, son N° du premier cluster dans la FAT , le nom du répertoire père et la adte de derniere modification.
  unsigned int err,cluster_sivant,adresse,nombre_fate,nombre_secteur_par_cluster;
  unsigned int adresse_debut_partition,num_cluster_1_data_region,taille_fate,taille_zone_reserve,nombre_octet_secteur,adresse_data_region,adresse_debut_fat;
  FILE* disk=NULL;
  char nomfichier[12],pere[15]="/";
  unsigned char buffer[512],buff[512];
  int k=0,m=0,s=0,taille_tableau_dossier=0;
  unsigned int adresse_premier_cluster_dossier[10000];
  chaine nom_pere[10000];
  bool arret=false,fin;
  disk=fopen(nom_disque,"rb");//ouvrir le fichier du disque en lecture
  if(disk==NULL) printf("Erreur nom du disk\n");//le fichier ne c'est pas ouvert
  else{//le fichier c'est ouvert correctement
    err=fread(buffer,512,1,disk);
    if(err<=0) printf("erreur de lecture");
    else{
      int num=0x1c2+(num_partition-1)*(0x10);//octet du master boot qui permet de verifier le type de partition
      if(buffer[num]==0x0c){
        adresse_debut_partition=0;
        printf("\nInformations utiles:\n\n" );
        printf ("\033[1;32mpartition de type FAT32\n");
        fseek(disk,num+4,SEEK_SET);
        fread(&adresse_debut_partition,4,1,disk);//lecture du humero du premier secteur de la partition demandé

        fseek(disk,adresse_debut_partition*512+13,SEEK_SET);
        fread(&nombre_secteur_par_cluster,1,1,disk);//lecture du nombre de secteur par cluster

        fseek(disk,adresse_debut_partition*512+11,SEEK_SET);
        fread(&nombre_octet_secteur,2,1,disk);//lecture du nombre d'octet par secteur

        fseek(disk,adresse_debut_partition*512+44,SEEK_SET);
        fread(&num_cluster_1_data_region,4,1,disk);////lecture du numero du premier cluster de la data region(generalement 2)

        fseek(disk,adresse_debut_partition*512+14,SEEK_SET);
        fread(&taille_zone_reserve,2,1,disk);//lecture de la taille de la zone reservé

        fseek(disk,adresse_debut_partition*512+16,SEEK_SET);
        fread(&nombre_fate,1,1,disk);//lecture du nombre de fat

        fseek(disk,adresse_debut_partition*512+36,SEEK_SET);
        fread(&taille_fate,4,1,disk);//lecture de la taille d'une fate en nombre de secteur

        adresse_debut_fat=adresse_debut_partition+taille_zone_reserve;//calcule de l'adresse de debut de la table fat
        adresse_data_region=adresse_debut_fat+(nombre_fate)*taille_fate;//calcule de l'adresse debut de la data region

        printf("adresse de debut de partition(numero secteur):%d\n",adresse_debut_partition );
        printf("taille de la zone reserve(en nombre de secteur):%d\n",taille_zone_reserve );
        printf("nombre de secteur par cluster:%x\n",nombre_secteur_par_cluster );
        printf("numero du premier cluster de la data region(repertoir racine):%d\n",num_cluster_1_data_region );
        printf("nombre de fate:%x\n",nombre_fate );
        printf("taille de la fate:%d\n",taille_fate );
        printf("nombre d'octet par secteur:%d\n",nombre_octet_secteur );
        printf("adresse de debut de la fat(numero de secteur):%d\n",adresse_debut_fat );
        printf("adresse de debut de la data region(numero de secteur):%d\n\033[00m\n",adresse_data_region );

        cluster_sivant=num_cluster_1_data_region;
        fin=false;//variable pour indiquer la fin du traitement
        printf ("\033[1;31mCONTENUE DU REPERTOIR : %s\033[00m\n",pere);

        while (!fin) {
          arret=false;//variable pour indiquer la fin d'un repertoir
          m=0;//variable pour indiquer le nombre d'entré courte d'un repertoir
          while (!arret) {
            adresse=adresse_data_region+(cluster_sivant-2)*(nombre_secteur_par_cluster);//le numero du premier secteur du clusteur qu'on veut afficher
            fseek(disk,adresse*512,SEEK_SET);
            fread(buff,512,1,disk);//lecture du premier secteur
            k=0;
            s=0;//variable utiliser pour verifier si tout le secteur a ete lu
            while (k<(16*(nombre_secteur_par_cluster))&&(!arret)){//boucle pour parcourir tout le cluster ou jusqu'a ca fin
              if(!(buff[s*32+0]==0xE5)){//cas ou l'entre a ete supprimer
                if(!(buff[s*32+0]==0x00)){//cas ou l'entre est libre
                  if(buff[s*32+11]==0x0F){//cas ou l'entre est de type longue
                    k++;
                    s++;
                    if( s==16){//si on a lu toutes les entres du secteur
                      s=0;
                      fread(buff,512,1,disk);
                    }
                  }else{//cas ou l'entre est de type courte
                    m++;
                    printf ("\033[34;01m\nEntré courte N°%d\033[00m\n",m);
                    printf("    Nom du fichier format court: ");
                    for(int i=0;i<8;i++) printf("%c",buff[s*32+i] );//affichage du nom du fichier au format court
                    if(buff[s*32+11]!=0x10){
                      printf("." );
                      for(int i=8;i<11;i++) printf("%c",buff[s*32+i] );
                    }
                    printf("\n" );
                    printf("    Nom du repertoir parent: %s\n",pere);//nom du repertoir parent
                    printf("    Taille du fichier: ");
                    int *taille;
                    taille=&buff[s*32+28];
                    printf("%d octets\n",*taille);//affichage de la taille du fichier
                    char premier_cluster[4];
                    premier_cluster[0]=buff[s*32+26];
                    premier_cluster[1]=buff[s*32+27];
                    premier_cluster[2]=buff[s*32+20];
                    premier_cluster[3]=buff[s*32+21];
                    int *premier_cluster_fichier=&premier_cluster;
                    printf("    numero premier cluster: %d\n",*premier_cluster_fichier );//affichage du numero du premier cluster du fichier en question
                    unsigned short *date=&buff[s*32+16];
                    unsigned short nb=0xFE00;
                    unsigned short annee=*date&nb;
                    printf("    Date de Derniére modification: %d/%d/%d\n",*date&0x001F,(*date&0x01E0)/32,annee/512+1980 );

                    printf("\n\n\n");
                    if((buff[s*32+11]==0x10)&&(k!=0)&&(k!=1)&&(*premier_cluster_fichier!=0)){//si l'entre est de type repertoir on enregistre son nom et
                    //le numero du premier cluster de ce repertoir dans un tableau afin de le parcourir ultérieurement
                      adresse_premier_cluster_dossier[taille_tableau_dossier]=*premier_cluster_fichier;
                      for(int m=0;m<11;m++) nom_pere[taille_tableau_dossier].nom[m]=buff[s*32+m];
                      nom_pere[taille_tableau_dossier].nom[12]='\0';
                      taille_tableau_dossier++;
                    }
                    k++;
                    s++;
                    if( s==16) {//si on a lu toutes les entres du secteur
                      s=0;
                      fread(buff,512,1,disk);
                    }
                  }
                }else{
                  arret=true;
                }
              }else{
                k++;
                s++;
                if( s==16) {//si on a lu toutes les entres du secteur
                  s=0;
                  fread(buff,512,1,disk);
                }
              }
            }
            cluster_sivant=cluster_suivant(disk,cluster_sivant,adresse_debut_fat);//retourne l'adresse du prochain cluster du repertoir courant
            if(cluster_sivant<0){//si fin de repertoir
              printf("pas de suivant\n");
              arret=true;
            }
          }
          if(taille_tableau_dossier>0){//si il y a toujours des dossiers a parcourir
            m=0;
            cluster_sivant=adresse_premier_cluster_dossier[taille_tableau_dossier-1];
            strcpy(pere,nom_pere[taille_tableau_dossier-1].nom);
            printf ("\033[1;31mCONTENUE DU REPERTOIR PARENT DE NOM: %s\033[00m\n",pere);
            taille_tableau_dossier--;
          }else{//si fin de tous les repertoir
            fin=true;
          }
        }
      }
      else {
        printf("cette partition n'est pas de type FAT32");
      }

    }

  }
}



