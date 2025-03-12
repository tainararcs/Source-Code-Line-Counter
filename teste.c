#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

int main (void)  
{  
    DIR *dp;  
    struct dirent *ep;  
    dp = opendir ("./");  

    /* 
    a  
    */  

   /*  
   */ printf("Oi.");  
   
    if (dp != NULL)  
    {   
        // Lendo até não achar mais entradas.  
        while (ep = readdir (dp))  
            puts (ep->d_name);  
            
        (void) closedir (dp); 
    }  

    else  
        printf ("Couldn't open the directory");  
    return 0;  
} 
