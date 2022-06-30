#include <stdio.h>
#include <minizip/unzip.h>
#include <string.h>
#include <dirent.h>
#include <switch.h>
#include <string.h>

#include "unzip.h"

#define WRITEBUFFERSIZE 0x25000 // 4KiB 
#define MAXFILENAME     0x301

bool prefix(const char* pre, const char *str){
    return strncmp(pre, str, strlen(pre)) == 0;
}

int unzip(const char *output)
{
    // FILE *logfile = fopen("log.txt", "w");
    // Define your first sub-folder in the zip with the last "/" explicitly, set to empty if you don't have a sub-folder in your zip
    char project_subfolder_in_zip[] = "switch_AIO_LS_pack-main/";
    unzFile zfile = unzOpen(output);
    unz_global_info gi = {0};
    unzGetGlobalInfo(zfile, &gi);
    int first_subfolder_passed = 0;
    if (strcmp(project_subfolder_in_zip, "") == 0) {
        first_subfolder_passed = 1;
    }

    for (int i = 0; i < gi.number_entry; i++)
    {
        char filename_inzip[MAXFILENAME] = {0};
        unz_file_info file_info = {0};
        unzOpenCurrentFile(zfile);
        unzGetCurrentFileInfo(zfile, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);

        char filename_on_sd[MAXFILENAME];
        int k = 0;
        for (int j = strlen(project_subfolder_in_zip); j < strlen(filename_inzip); j++)
         {
            filename_on_sd[k] = filename_inzip[j];
            k++;
        }
        filename_on_sd[k] = '\0';
        // printf("Test nom du fichier sur la SD: %s\n", filename_on_sd);
        // fputs (filename_on_sd, logfile);
        if (strcmp(filename_inzip, project_subfolder_in_zip) == 0 && first_subfolder_passed == 0){
            first_subfolder_passed = 1;
            unzCloseCurrentFile(zfile);
            unzGoToNextFile(zfile);
            consoleUpdate(NULL);
            continue;
        }
        // check if the string ends with a /, if so, then its a directory.
        else if ((filename_inzip[strlen(filename_inzip) - 1]) == '/')
        {
            // check if directory exists
            DIR *dir = opendir(filename_on_sd);
            if (dir) closedir(dir);
            else
            {
                printf("Création du répertoir: %s\n", filename_on_sd);
                mkdir(filename_on_sd, 0777);
            }
        }    

        else if (strcmp(filename_on_sd, "atmosphere/package3") == 0){
            FILE *outfile = fopen("atmosphere/package3.temp", "wb");
            void *buf = malloc(WRITEBUFFERSIZE);

            printf ("\033[0;31mDANS PACKAGE3! NE PAS ETEINDRE LA CONSOLE!\033[0;37m\n");
            consoleUpdate(NULL);
            sleep(2);

            for (int j = unzReadCurrentFile(zfile, buf, WRITEBUFFERSIZE); j > 0; j = unzReadCurrentFile(zfile, buf, WRITEBUFFERSIZE))
                fwrite(buf, 1, j, outfile);

            fclose(outfile);
            free(buf);
        }

        else if (strcmp(filename_on_sd, "atmosphere/stratosphere.romfs") == 0){
            FILE *outfile = fopen("atmosphere/stratosphere.romfs.temp", "wb");
            void *buf = malloc(WRITEBUFFERSIZE);

            printf ("\033[0;31mDANS STRATOSPHERE.ROMFS! NE PAS ETEINDRE LA CONSOLE!\033[0;37m\n");
            consoleUpdate(NULL);
            sleep(2);

            for (int j = unzReadCurrentFile(zfile, buf, WRITEBUFFERSIZE); j > 0; j = unzReadCurrentFile(zfile, buf, WRITEBUFFERSIZE))
                fwrite(buf, 1, j, outfile);

            fclose(outfile);
            free(buf);
        }

        else
        {
            const char *write_filename = filename_on_sd;
            FILE *outfile = fopen(write_filename, "wb");
            void *buf = malloc(WRITEBUFFERSIZE);

            printf("Extraction de: %s\n", filename_on_sd);
            consoleUpdate(NULL);

            for (int j = unzReadCurrentFile(zfile, buf, WRITEBUFFERSIZE); j > 0; j = unzReadCurrentFile(zfile, buf, WRITEBUFFERSIZE))
                fwrite(buf, 1, j, outfile);

            fclose(outfile);
            free(buf);
        }

        unzCloseCurrentFile(zfile);
        unzGoToNextFile(zfile);
        consoleUpdate(NULL);
    }

    unzClose(zfile);
    //remove(output);
    
    printf("\033[0;32m\nFinis!\n\nRedemarage automatique dans 5 secondes :)\n");
    consoleUpdate(NULL);

    sleep(5);
// fclose(logfile);
    return 0;
}
