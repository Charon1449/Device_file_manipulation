#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statvfs.h>
#include <dirent.h>


typedef struct chaine chaine;
struct chaine
{
  char nom[50];
};
void Afficher_Fdel(char nom_disque[],int num_partition);
int cluster_suivant(FILE* disk,int cluster_courant,int debut_fat);

void hexDump(const char *desc, const void *addr, const int len)
{
    int i;
    unsigned char buff[17];
    const unsigned char *pc = (const unsigned char *)addr;

    if (desc != NULL)
        printf("%s bn\n", desc);

    if (len == 0)
    {
        printf("  ZERO LENGTH\n");
        return;
    }
    else if (len < 0)
    {
        printf("  NEGATIVE LENGTH: %d\n", len);
        return;
    }
    for (i = 0; i < len; i++)
    {
        if ((i % 16) == 0)
        {
            if (i != 0)
                printf("  %s\n", buff);
            printf("  %04x ", i);
        }
        printf(" %02x", pc[i]);

        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    while ((i % 16) != 0)
    {
        printf("   ");
        i++;
    }

    printf("  %s\n", buff);
}
void read_sector(const char *disk_name ,int num_sector ,unsigned char *sector){
    FILE *disk;
    char disk_path[40];
    strcpy(disk_path,"/dev/");
    strcat(disk_path,disk_name);
    disk = fopen(disk_path, "r");
    if ((disk == NULL))
    {
        printf("Error opening file \n");
        return;
    }
    if (fseek(disk, 512*num_sector, SEEK_SET) == 0)
          fread(sector, 512, 1, disk);
}
void show_sector(const char *disk_name ,int num_sector){
    unsigned char sec[512];
    char num[2];
    sprintf(num ,"%d" ,num_sector);
    read_sector(disk_name ,num_sector ,sec);
    hexDump("offset|" ,sec ,sizeof(sec));


}
void list_disks(){
    struct dirent *de;
    DIR *dr = opendir("/dev");
    if (dr == NULL)
    {
        printf("Could not open current directory" );
        return 0;
    }

    while ((de = readdir(dr)) != NULL) {
        if ((strstr(de->d_name,"sd") != NULL)){
            printf("%s\n", de->d_name);
        }
    }
    closedir(dr);

}
void get_disk_info(const char *disk_name){
    char disk_path[40];
    strcpy(disk_path,"/dev/");
    strcat(disk_path,disk_name);
    struct statvfs stfs;
    statvfs(disk_path, &stfs);
    printf("Total Blocks in the Filesystem %d", 4 * stfs.f_blocks);
    printf("\n");
    printf("Block Size %d", stfs.f_bsize / 4);
    printf("\n");
    printf("Total Free Blocks in FS %d", stfs.f_bfree);
    printf("\n");
    printf("Total Blocks Used %d", 4 * (stfs.f_blocks - stfs.f_bfree));
    printf("\n");
    printf("Total Blocks Available to non Super User %d", 4 * stfs.f_bavail);
    printf("\n");
    printf("Total File Nodes %d", stfs.f_ffree);
    printf("\n");
    printf("Total Blocks in the Filesystem %d", stfs.f_blocks);
    printf("\n");
}
int main(int argc, char **argv)
{


    list_disks();
    get_disk_info("sda");
    show_sector("sda" , 0);
    return 0;
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



int cluster_suivant(FILE* disk,int cluster_courant,int debut_fat){//retourne le cluster suivant du numero donné en parametre(cluster_courant)
  int result;
  fseek(disk,debut_fat*512+(cluster_courant)*4,SEEK_SET);
  fread(&result,4,1,disk);
  if(result>=0x0FFFFFF8) result=-1;//dernier cluster de la chaine
  return result;
}
