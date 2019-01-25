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
int mySock;
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
    
    printf("========> THis is the mail server: %s\n",lastPtr);
    
    pclose(result);
    return lastPtr;
}

int isEmailValid(char* email)
{
    int charCountBeforeAt = 0;
    int atCount = 0;
    int dotCountAfterAt = 0;
    int charCountBeforeDot = 0;
    int charCountAfterDot = 0;
    int i = 0;
    
    for (i = 0; i < strlen(email); i++)
    {
        if (atCount == 0 && email[i] != '@')
        {
            charCountBeforeAt++;
        }
        if (email[i] == '@')
        {
                atCount++;
        }
        if (atCount > 0 && email[i] == '.')
        {
            dotCountAfterAt++;
        }
        
        if (atCount > 0 && email[i] != '.' && dotCountAfterAt == 0)
        {
            charCountBeforeDot++;
        }
        else if (atCount > 0 && email[i] != '.' && dotCountAfterAt != 0)
        {
            charCountAfterDot++;
        }
        
    }
    
    if (atCount != 1 || charCountBeforeAt <= 0 || dotCountAfterAt <= 0 || charCountBeforeDot <= 0 || charCountAfterDot <= 0 )
        return 0;
    
    return 1;
}


char* getAddressIp(char* hostname)
{
    struct hostent* hostInfo = gethostbyname(hostname);
    char* hostIp = inet_ntoa(*((struct in_addr*)hostInfo->h_addr_list[0]));
    //return hostInfo->h_addr_list[0];
    
    //printf("This is the host name: %s, this is ip: %s\n",hostInfo->h_name,hostIp);
    
    return hostIp;
}

void sendMsg(char* msg)
{
    printf("%s\n", msg);
    //int bytesSent = send( mySock, msg,strlen(msg),0);
    int bytesSent = send(mySock, msg, strlen(msg), 0);

}

void readMsg()
{
    
    char* buf = malloc(sizeof(char)*(BUFSIZ+1));
    memset(buf, '\0', sizeof(char)*(BUFSIZ+1));
    //printf("\nString length after malloc!!!!!: %d\n",strlen(buf));
    int readRsult = recv(mySock, buf, BUFSIZ, 0);
    printf("%s\n",buf);
    free(buf);
}





char* getEmailAddress(char* head)
{
    int i;
    int hasLangle = 0;
    char* emailAddr = malloc(sizeof(char)*(strlen(head)));
    memset(emailAddr, '\0', sizeof(char)*(strlen(head)));
    int indexE = 0;
    for ( i = 0; i < strlen(head);i++)
    {
        if ((head[i] != '<' && head[i] != '>') && hasLangle == 1)
        {
            emailAddr[indexE] = head[i];
            indexE++;
            
        }
        if (head[i] == '<')
        {
            hasLangle = 1;
        }
        
    }
    
    return emailAddr;
}

char* getName(char* head)
{
    int i;
    int hasQuote = 0;
    char* name = malloc(sizeof(char)*strlen(head));
    memset(name,'\0',sizeof(char)*strlen(head));
    int indexN = 0;
    
    for ( i = 0; i < strlen(head); i++)
    {
        if (head[i] != '"' && hasQuote == 1 )
        {
            name[indexN] = head[i];
        }
        
        if (head[i] == '"')
            hasQuote++;
    }
    
    return name;
}



char* constructHead(char* head,char* email)
{
    char* constructed = malloc(sizeof(char)*200);
    memset(constructed, '\0', 200);
    strcpy(constructed, head);
    strcat(constructed, " <");
    strcat(constructed, email);
    strcat(constructed, ">\r\n");
    
    return constructed;
}

char* constructFrom(char* name, char* email)
{
    char* fromHead = malloc(sizeof(char)*(256));
    memset(fromHead, '\0', 256);
    //From: Busy <busy@testing.com>
    strcpy(fromHead, "From: ");
    strcat(fromHead, name);
    strcat(fromHead, " <");
    strcat(fromHead, email);
    strcat(fromHead, ">\r\n");
    
    return fromHead;
}

char* constructBody(int n, char** entireEmail,char* name, char* email)
{
    int i;
    char* body = malloc(sizeof(char)*4096);
    memset(body, '\0', 4096);
    strcpy(body, constructFrom(name, email));

    for(i = 2; i < n; i++)
    {
        strcat(body, entireEmail[i]);
    }
    
    strcat(body, ".\r\n");
    return body;
}

int sendEmail()
{
    char* hostIp = getAddressIp(getMailUrl("uic.edu"));
    printf("Host ip: %s\n",hostIp);
    
    
    mySock = socket(AF_INET,SOCK_STREAM,0);
    
    struct sockaddr_in mySockAddr;
    memset(&mySockAddr, '0', sizeof(mySockAddr));
    
    if (mySock < 0)
    {
        printf("\nINvalid socket\n");
        return -1;
    }
    
    printf("Socket number: %d\n",mySock);
    
    
    if(inet_pton(AF_INET, hostIp, &(mySockAddr.sin_addr))<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    mySockAddr.sin_family = AF_INET;
    mySockAddr.sin_port = htons(25);
    
    
    printf("socket Connecting\n");
    int connectCode = connect(mySock, (struct sockaddr*)&mySockAddr, sizeof(mySockAddr));
    if (connectCode < 0)
    {
        printf("\n Error : Connect Failed %d\n",connectCode);
        return 1;
    }
    
    printf("mysocket connection: %d\n",connectCode);
    
    
    char helo[] = "helo Heng\r\n";
    char* from = constructHead("mail from:", "busy@testing.com");
    char* to = constructHead("rcpt to:", "hli212@uic.edu");
    char data[] = "data\r\n";
    
    char body[] = "From: Busy <busy@testing.com>\r\nSubject: Testing before mod\r\n\r\n TEST before mod!!!\r\n.\r\n";
    
    readMsg();
    sendMsg(helo);
    readMsg();
    
    sendMsg(from);
    readMsg();
    
    sendMsg(to);
    readMsg();
    
    sendMsg(data);
    readMsg();
    
    sendMsg(body);
    readMsg();
    sendMsg("QUIT\r\n");
    
    
    close(mySock);
    
    return 0;
}

void readFile(char* fileName)
{
    FILE* emailFile;
    
    emailFile = fopen(fileName, "r");
    
    if(emailFile == NULL)
    {
        printf("Error when opening %s\n",fileName);
        return;
    }
    
    char buffer[1025];
    //memset(buffer, '\0', sizeof(char)*(BUFSIZ+1));
    int numLine = 0;
    int j;
    char** fullEmail = (char**)malloc(sizeof(char*)*1024);
    while (fgets(buffer, 1024, emailFile))
    {
        printf("This is what I read: %s",buffer);
        printf("This is the length: %d\n",strlen(buffer));
        
        fullEmail[numLine] = malloc(sizeof(char)*(strlen(buffer)+1));
        
        memset(fullEmail[numLine], '\0', sizeof(char)*(strlen(buffer)+1));
        strcpy(fullEmail[numLine], buffer);
    }
    
    
    fclose(emailFile);
    
}

int main(int argc, char** argv)
{
    
    int i = 0;
    
    
    for (i = 1; i < argc; i++)
    {
        //printf("%s\n",argv[i]);
        
        readFile(argv[i]);
    }

    sendEmail();
    
    
    //printf("This is mail: %s\n",getMailUrl( "uic.edu"));
    return 0;
}
