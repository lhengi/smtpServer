//
//  hw1.c
//
//
//  Created by Heng Li on 1/23/19.
//
#include <fnmatch.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <dirent.h>
#include <netdb.h>
#include <arpa/inet.h>
char* getMailUrl(char* hostName)
{
    char* tmpMail = malloc(sizeof(char)*80);
    tmpMail[79] = '\0';
    
    char* command = malloc(sizeof(char)*80);
    strcat(command, "host -t MX ");
    strcat(command, hostName);
    
    FILE* result = popen(command,"r");
    if (result == NULL)
    {
        printf("Woops something went wrong with popen()\n");
        return NULL;
    }
    fgets(tmpMail,79,result);
    printf(tmpMail);
    
    char* ptr = strtok(tmpMail," ");
    char* lastPtr = ptr;
    
    while(ptr != NULL)
    {
        lastPtr = ptr;
        //printf("*%s\n",lastPtr);
        ptr = strtok(NULL," ");
    }

    lastPtr[strlen(lastPtr)-2] = '\0';
    return lastPtr;
}

char* getAddressIp(char* hostname)
{
    struct hostent* hostInfo = gethostbyname(hostname);
    char* hostIp = inet_ntoa(*((struct in_addr*)hostInfo->h_addr_list[0]));
    return hostInfo->h_addr_list[0];
    
    //printf("This is the host name: %s, this is ip: %s\n",hostInfo->h_name,hostIp);
    
    //return hostIp;
}

int main(int argc, char** arv)
{
    
    
    char from[] = "From: Busy Beaver <beaver@busy.com>\n";
    char to[] = "To: Jakob Eriksson <jakob@uic.edu>\n";
    char subject[] = "Subject: Give me an A\n\n";
    char body[] = "My program sends emails. Thus, Ideserve an A.";
    
    
    
    char* hostIp = getAddressIp("uic.edu");
    
    int mySock = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr mySockAddr;
    mySockAddr.sa_family = AF_INET;
    strcpy(mySockAddr.sa_data, hostIp);
    printf("\n\n hostIp length--: %s\n",mySockAddr.sa_data);
    mySockAddr.sa_len = htons(25);
    //int sockConnect = connect(mySock, (struct sockaddr*)&mySockAddr, sizeof(mySockAddr));
    
    
    //printf("mysocket connection: %d\n",sockConnect);
    
    //getMailUrl("uic.edu");
    
    //getAddressInfo("uic.edu");
    
    
    
    
    
    
    return 0;
}
