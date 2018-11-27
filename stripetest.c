// Sam Siewert
//

#include <stdio.h>

#include "raidlib.h"

int main(int argc, char *argv[])
{
    int bytesWritten, bytesRestored;

    if(argc < 3)
    {
        printf("Useage: stripetest inputfile outputfile\n");
    }

    bytesWritten=stripeFile(argv[1], 0); 
    bytesRestored=restoreFile(argv[2], 0, bytesWritten); 
    printf("FINISHED\n");
        
}
