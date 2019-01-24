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
    //return hostInfo->h_addr_list[0];
    
    //printf("This is the host name: %s, this is ip: %s\n",hostInfo->h_name,hostIp);
    
    return hostIp;
}

int main(int argc, char** arv)
{
    
    
    char from[] = "From: Busy Beaver <beaver@busy.com>\r\n";
    char to[] = "To: Jakob Eriksson <jakob@uic.edu>\r\n";
    char subject[] = "Subject: Give me an A\r\n\n";
    char body[] = "My program sends emails. Thus, Ideserve an A.\r\n";
    
    
    
    char* hostIp = getAddressIp("uic.edu");
    printf("Host ip: %s\n",hostIp);
    
    int mySock = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in mySockAddr;
    
    

    if(inet_pton(AF_INET, hostIp, &(mySockAddr.sin_addr))<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    printf("\n\n hostIp length--: %d\n",mySockAddr.sin_addr);
    mySockAddr.sin_port = htons(25);
    
    
    printf("socket Connecting\n");
    int connectCode = connect(mySock, (struct sockaddr*)&mySockAddr, sizeof(mySockAddr));
    if (connectCode < 0)
    {
        printf("\n Error : Connect Failed \n");
        return 1;
    }
    
    
    printf("mysocket connection: %d\n",connectCode);
    
    int sendCode = send(mySock,from,strlen(from),0);
    
    //getMailUrl("uic.edu");
    
    //getAddressInfo("uic.edu");
    
    return 0;
}
