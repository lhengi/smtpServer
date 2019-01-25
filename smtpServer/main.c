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
    
    char* tmpMail = malloc(sizeof(char)*200);
    memset(tmpMail, '\0', 200);
    
    char* command = malloc(sizeof(char)*256);
    memset(command, '\0', 256);
    strcat(command, "host -t MX ");
    strcat(command, hostName);
    
    FILE* result = popen(command,"r");
    if (result == NULL)
    {
        
        free(command);
        printf("Woops something went wrong with popen()\n");
        pclose(result);
        return NULL;
    }
    fgets(tmpMail,256,result);
    printf("!\n\nTMP Mail%s\n",tmpMail);
    
    char* ptr = strtok(tmpMail," ");
    char* lastPtr = ptr;
    
    int preNot = 0;
    while(ptr != NULL)
    {
        if(ptr!= NULL)
            lastPtr = ptr;
        //printf("*%s\n",lastPtr);
        
        if(ptr != NULL && (strcmp(ptr, "3(NXDOMAIN)\n") == 0 || strcmp(ptr, "3(NXDOMAIN)") == 0))
        {
            
            free(command);
            pclose(result);
            return NULL;
        }
        ptr = strtok(NULL," ");
                  
    }
    if(lastPtr == NULL)
    {
        
        free(command);
        pclose(result);
        return NULL;
    }

    lastPtr[strlen(lastPtr)-2] = '\0';
    
    pclose(result);
    
    free(command);
    
    //printf("lastPtr: %s\n",lastPtr);
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

    //printf("###########\nThis is the host name in get Address IP: %s| %d\n", hostname,strlen(hostname));
    //printf("###########\nThis is the host name in get Address IP: %d\n", hostname);

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

int readMsg()
{
    
    char* buf = malloc(sizeof(char)*(BUFSIZ+1));
    memset(buf, '\0', sizeof(char)*(BUFSIZ+1));
    //printf("\nString length after malloc!!!!!: %d\n",strlen(buf));
    int readRsult = recv(mySock, buf, BUFSIZ, 0);
    printf("%s\n",buf);
    free(buf);
    
    
    char* ptr = strtok(buf," ");
    
    if(ptr != NULL && (strcmp(ptr, "450") == 0 ||strcmp(ptr, "554") == 0 || strcmp(ptr, "502") == 0 || strcmp(ptr, "550") == 0  ))
    {
        return 0;
    }

    
    
    return 1;
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
        
        if (head[i] == '>')
        {
            break;
            
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
            indexN++;
        }
        
        if (head[i] == '"')
            hasQuote++;
        if (hasQuote == 2)
            break;
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
    //printf("Construting body with name: %s\n",name);
    strcpy(body, constructFrom(name, email));

    for(i = 2; i < n; i++)
    {
        strcat(body, entireEmail[i]);
    }
    
    strcat(body, ".\r\n");
    return body;
}

char* getHostURL(char* email)
{
    int i;
    int passedAt = 0;
    char* url = malloc(sizeof(char)*strlen(email));
    memset(url, '\0', sizeof(char)*strlen(email));
    int index = 0;
    for(i = 0; i < strlen(email); i++)
    {
        if(email[i] != '@' && passedAt == 1)
        {
            url[index] = email[i];
            index++;
        }
        
        if(email[i] == '@')
        {
            passedAt = 1;
        }
        
    }
    
    return url;
}

int sendEmailWithParm(int n, char** entireEmail)
{
    char* fromAddr = getEmailAddress(entireEmail[0]);
    char* rcptAddr = getEmailAddress(entireEmail[1]);
    char* senderName = getName(entireEmail[0]);
    
    if (isEmailValid(rcptAddr) == 0)
    {
        printf("%s  is not a valid email address\n",rcptAddr);
        return -1;
    }
    
    char* url = getHostURL(rcptAddr);
    //printf("In sendEmail with Parm Host url: %s\n",url);
    //printf("url length: %d\n",strlen(url));
    char* mailServer = getMailUrl(url);
    
    if(mailServer == NULL)
    {
        printf("mailServer = %s\n",mailServer);
        printf("%s not a valid url\n",url);
        return -1;
    }
    
    printf("!!!!!!!!!!Mail Server: %s\n",mailServer);
    printf("!!!!!!!!!!Mail Server: %d\n",mailServer);
    char* hostIp = getAddressIp(mailServer);
    //printf("Host ip: %s\n",hostIp);
    
    mySock = socket(AF_INET,SOCK_STREAM,0);
    
    struct sockaddr_in mySockAddr;
    memset(&mySockAddr, '0', sizeof(mySockAddr));
    
    if (mySock < 0)
    {
        printf("\nINvalid socket\n");
        return -1;
    }
    
    
    if(inet_pton(AF_INET, hostIp, &(mySockAddr.sin_addr))<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    mySockAddr.sin_family = AF_INET;
    mySockAddr.sin_port = htons(25);
    
    
    int connectCode = connect(mySock, (struct sockaddr*)&mySockAddr, sizeof(mySockAddr));
    if (connectCode < 0)
    {
        printf("\n Error : Connect Failed %d\n",connectCode);
        return 1;
    }
    
    
    char helo[] = "helo Heng\r\n";
    char* from = constructHead("mail from:", fromAddr);
    char* to = constructHead("rcpt to:", rcptAddr);
    char data[] = "data\r\n";
    
    char* body = constructBody(n, entireEmail, senderName, fromAddr);
    readMsg();
    sendMsg(helo);
    if(readMsg() == 0)
    {
        close(mySock);
  
        return 0;
    }
    
    sendMsg(from);
    if(readMsg() == 0)
    {
        close(mySock);

        return 0;
    }
    
    sendMsg(to);
    if(readMsg() == 0)
    {
        close(mySock);

        return 0;
    }
    
    sendMsg(data);
    if(readMsg() == 0)
    {
        close(mySock);

        return 0;
    }
    
    sendMsg(body);
    if(readMsg() == 0)
    {
        close(mySock);

        return 0;
    }
    sendMsg("QUIT\r\n");
    readMsg();
    
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
    char** fullEmail = (char**)malloc(sizeof(char*)*1024);
    while (fgets(buffer, 1024, emailFile))
    {
        //printf("This is what I read: %s",buffer);
        //printf("This is the length: %d\n",strlen(buffer));
        
        fullEmail[numLine] = malloc(sizeof(char)*(strlen(buffer)+1));
        
        memset(fullEmail[numLine], '\0', sizeof(char)*(strlen(buffer)+1));
        strcpy(fullEmail[numLine], buffer);
        numLine++;
    }
    
    fclose(emailFile);
    sendEmailWithParm(numLine, fullEmail);
    
}

int main(int argc, char** argv)
{
    
    int i = 0;
    
    //getMailUrl("cs.uic.edu");
    
    
    for (i = 1; i < argc; i++)
    {
        printf("========================================\n");
        printf("%s\n",argv[i]);
        
        readFile(argv[i]);
        printf("========================================\n");
    }

    
    return 0;
}
