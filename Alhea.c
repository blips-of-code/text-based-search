#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define REQUEST_BUFFER_SIZE 2000

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

int main() {
    // Address information setup
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;

    struct addrinfo *result;
    //Step 2
    int addr_suc = getaddrinfo("alhea.com", "80", &hints, &result);
    if (addr_suc != 0) {
        printf("Error in getaddrinfo: \n");
        return 1;
    }
    // Create socket
    int sfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    
    // Connect to the server
    int connect_ret = connect(sfd, result->ai_addr, result->ai_addrlen);

    if (connect_ret < 0)
    {
        printf("Failed to connect %d!\n", connect_ret);
        return 1;
    } 
    else 
    {
        printf("Connected succesfully to alhea's server!\n");
    }

    // Get search term from user
    char search_term[100];
    printf("Enter search term: ");
    fgets(search_term, 100, stdin);

    // Remove newline character from search_term
    search_term[strcspn(search_term, "\n")] = '\0'; 

    
    // Step 3: Send HTTP request 
    char request[REQUEST_BUFFER_SIZE];
    snprintf(request, REQUEST_BUFFER_SIZE, "GET /find?q=%s HTTP/1.1\r\nHost: alhea.com\r\nAccept-Encoding: identity\r\n\r\n", search_term);
    //char *request = "GET /find?q=work HTTP/1.1\r\nHost: alhea.com\r\nAccept-Encoding: identity\r\n\r\n";
    //printf("Request:\n%s\n", request); 
    write(sfd, request, strlen(request)+1);

    // Set receive timeout
    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    setsockopt(sfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    // Dynamic allocation of buffer
    char *buffer;
    size_t BUFFER_SIZE = 2000; // Initial size
    buffer = (char *)malloc(BUFFER_SIZE * sizeof(char));
    if (buffer == NULL) {
        printf("Failed to allocate memory for buffer\n");
        return 1;
    }

    // Step 4: Read server response
    size_t total_bytes_read = 0;

    while (total_bytes_read < BUFFER_SIZE) {
        size_t remaining_space = BUFFER_SIZE - total_bytes_read; 
        ssize_t bytes_read = read(sfd, buffer + total_bytes_read, remaining_space);
        total_bytes_read += bytes_read;

        // Check if the buffer needs to be resized
        if (total_bytes_read >= BUFFER_SIZE) {
            BUFFER_SIZE *= 2; // Double the buffer size
            buffer = (char *)realloc(buffer, BUFFER_SIZE * sizeof(char));
            if (buffer == NULL) {
                printf("Failed to reallocate memory for buffer\n");
                return 1;
            }
        }
        if (bytes_read == 0) { 
            printf("Reached end of file\n");
            break;
        }
    }

    buffer[total_bytes_read] = '\0';
    // Print the received data
    //printf("Received data: %s\n", buffer);

    print_results(buffer);

    // Clean up
    freeaddrinfo(result);
    close(sfd);

    return 0;
}
