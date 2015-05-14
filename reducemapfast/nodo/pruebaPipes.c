#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>	/* for open flags */
#include <limits.h>	/* for PATH_MAX */

int main(void)
{
        int     fd[2], nbytes, temp;
        pid_t   childpid;
        char*    string = "Hello, world!\nBuana la loca \nAu novia puta\n";
        char    readbuffer[80];
        static char template[] = "/tmp/mytemporalXXXXXX";
        char fname[PATH_MAX];

        pipe(fd);
        
        if((childpid = fork()) == -1)
        {
                perror("fork");
                exit(1);
        }

        if(childpid == 0)
        {
                /* Proceso hijo cierra la entrada del pipe */
                close(fd[0]);

                /* Manda 'string' por la salida del pipe*/
                write(fd[1], string, (strlen(string)+1));



                close(fd);				/* Cierro el temporal */
        }
        else
        {
                /* Proceso padre cierra la salida del pipe */
                close(fd[1]);
                /* Leo un string por el pipe */
                nbytes = read(fd[0], readbuffer, sizeof(readbuffer));
                /* Creo un archivo temporal a donde voy a hacer sort*/
                strcpy(fname,template);
                temp =  mkstemp(fname);
                write(temp, string, (strlen(string)+1));
                printf("Filename is %s\n", fname);
                printf("Received string: %s", readbuffer);
                /* Ejecuto sort sobre el archivo temporal*/
                execlp("sort","sort",fname,NULL);
        }
        return(0);
}
