#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statvfs.h>
#include <dirent.h> 

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
