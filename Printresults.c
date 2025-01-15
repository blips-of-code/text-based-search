#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>




//Given the full HTML search results,
// priont them in condensed human-readable format
void print_results(char *s)
{
    int c =0;
    int len = strlen(s);
    //First, copy the string into a new string.
    // This prevents issues in C where the string is read-only
    // (i.e. const) and we want to write to it.
    char newstr[len+1];
    strncpy(newstr, s, len);
    newstr[len] = '\0';


    //Pointer to where we are in the string 
    // (starts at the beginning)
    char *cur = &(newstr[0]);
    while(1) // while there is more to process
    {
        //This tag sequence happens to start each search 
        // result, so update the pointer to its location
        char *startSeq = "<h5><a rel=\"nofollow\" target=\"_blank\" href=\"/o.php?ux=";
        cur = strstr(cur, startSeq);
        //If we couldn't find the tag sequence there are no more results
        if(cur == NULL)
        {
            break;
        }
        cur += strlen(startSeq)+1;




        // This will be the start of the actual string with the result title
        char *startPtr = strchr(cur, '>');
        if(startPtr == NULL)
        {
            printf("Error! could not find > in string %s\n", cur);
            break;
        }
        //This will be the end of the displayed result title.
        char *endPtr = strchr(startPtr, '<');
        if(endPtr == NULL)
        {
            printf("Error! could not find < in string %s\n", startPtr);
            break;
        }


    
        //The next token starts at the end of this one
        char *next = endPtr;
        //Put a null to terminate the string so we can search before it
        *next = '\0';


        //The tring begins with the > attached to the link
        // so search for the LAST > character


        //Print the result
        printf("Result %d: %s\n", (c+1), (startPtr+1));


        //Move it past the null terminator
        cur = next+1;
        c++;
    }
    printf("done with results!\n");


}
