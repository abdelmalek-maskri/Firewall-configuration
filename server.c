#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <string.h>

#define BUFFERLENGTH 1024
#define MAX_MATCHED_QUERIES 100
int MAX_RULES = 25;

// Global variables for rules and their counts
char (*rules)[BUFFERLENGTH]; // Declare a pointer to a 2D array
int *ruleCounts;

char matchedQueries[BUFFERLENGTH][MAX_MATCHED_QUERIES][BUFFERLENGTH]; // Array to store matched querie
int matchedQueriesCount[BUFFERLENGTH] = {0};
pthread_mutex_t mutex; // Mutex for thread safety

void error(char *msg)
{
    perror(msg);
    exit(1);
}
int isIpAddress1SmallerThan2(char *ipAddress1, char *ipAddress2)
{
    long nums1[4];
    long nums2[4];
    int i;

    // printf("the ip1 is %s\n", ipAddress1);
    // printf("the ip2 is %s\n", ipAddress2);

    // Populate nums1
    char *duplicatedIpAddress = strdup(ipAddress1);
    char *token = strtok(duplicatedIpAddress, ".");

    i = 0;
    while (token != NULL)
    {
        nums1[i] = strtol(token, NULL, 10);
        token = (char *)strtok(NULL, ".");
        // token = (char*)strtok(NULL, ".");
        // printf("Num1[%d] is %li\n",i, nums1[i]);
        i++;
    }
    free(duplicatedIpAddress);

    // Populate nums2
    char *duplicatedIpAddress2 = strdup(ipAddress2);
    char *token2 = strtok(duplicatedIpAddress2, ".");

    i = 0;
    while (token2 != NULL)
    {
        nums2[i] = strtol(token2, NULL, 10);
        token2 = (char *)strtok(NULL, ".");
        i++;
    }
    free(duplicatedIpAddress2);

    for (i = 0; i < 4; i++)
    {
        // printf("\nComparing index %d with values %lu (from num1) and %lu (from num2)",i,nums1[i],nums2[i]);
        if (nums1[i] == nums2[i])
        {
            printf("Result: continue\n");
            continue;
        }
        else if (nums1[i] < nums2[i])
        {
            printf("Result: True\n");
            return 1;
        }
        else if (nums1[i] > nums2[i])
        {
            printf("Result : False\n");
            return 0;
        }
    }
    return 0; // If the ip addresses are exactly the same, return false
}

int isValidNumber(char *str)
{
    // printf("\n\nVALIDATING IF THIS IS AN ACTUAL NUMBER: %s\n\n", str);
    while (*str)
    {
        // printf("\nCurrentChar: %c %d",*str, *str=='\n');
        if (!isdigit(*str) && *str != '\n')
        {
            // Return 0 if character is not a digit, and not a newline character. (Had to whitelist newline characters because otherwise port n-0 would not validate.)
            // printf("\nTHIS IS NOT AN ACTUAL NUMBER.\n\n");
            return 0;
        }
        str++;
    }
    // printf("\nTHIS IS AN ACTUAL NUMBER!! :)\n\n");
    return 1;
}

int ValidateIPAddress(char *ipAddress)
{
    char *str = strdup(ipAddress); // Have to duplicate the string because strtok changes it
    // printf ("str points to %p\n", str);
    // printf ("str = %s\n", str);
    char *token = strtok(str, ".");
    //  printf ("token points to %p\n", token);
    //  printf ("token = %s\n", token);
    //   printf ("str = %s\n", str);
    // printf("\nValidating IP Address: %s", ipAddress);

    int numNumbers = 0;
    while (token != NULL)
    {
        numNumbers++;
        long number = strtol(token, NULL, 10);
        // printf("\nNUMBER: %lu TOKEN: %s",number,token);
        if (number >= 0 && number <= 255 && isValidNumber(token) == 1)
        { // strtol returns 0 whenever a non-number is entered into it. So we need to do isValidNumber check to see if the number 0 was actually entered or not. Also if you input "1A93", it outputs 1. This is why we need to iterate & check if ALL characters are a digit too.
            // Ok
            // printf(" VALID");
        }
        else
        {
            // printf(" INVALID");
            free(str);
            return 0;
        }
        token = strtok(NULL, ".");
    }
    if (numNumbers != 4)
    {
        free(str);
        return 0;
    }
    free(str);
    return 1;
}

int ValidateIPAddresses(char *ipAddresses)
{
    char *token = strtok(ipAddresses, "-");
    int numIPAddresses = 0;
    /*
    while (token != NULL) {
        numIPAddresses++;
        //printf("\nValidating IP Address: %s",token);
        if (ValidateIPAddress(token) == 0) {
           return 0;
        }
        token = strtok(NULL,"-");
    }
    */
    // Doesn't work because you can't strtok on 2 different strings interchangeably at the same time (as it records internal state which we access by passing NULL to extract further tokens in the current string)

    char *ipAddress1 = NULL;
    char *ipAddress2 = NULL;
    while (token != NULL)
    {
        numIPAddresses++;
        if (numIPAddresses == 1)
        {
            ipAddress1 = token;
        }
        else if (numIPAddresses == 2)
        {
            ipAddress2 = token;
        }
        token = strtok(NULL, "-");
    }

    // printf("\nNum IP Address Tokens: %d", numIPAddresses);
    if (numIPAddresses >= 1 && numIPAddresses <= 2)
    {
        if ((ValidateIPAddress(ipAddress1) == 1) && (ipAddress2 == NULL || (ValidateIPAddress(ipAddress2) && isIpAddress1SmallerThan2(ipAddress1, ipAddress2) == 1)))
        {
            return 1;
        }
    }
    return 0;
}

int ValidatePorts(char *portsToValidate)
{
    char *ports = strdup(portsToValidate);

    char *token = strtok(ports, "-");
    int numPorts = 0;
    long port1;

    while (token != NULL)
    {
        numPorts++;

        if (!isValidNumber(token))
        {
            free(ports);
            return 0;
        }

        long number = strtol(token, NULL, 10);
        if (number > 0 && number <= 65535)
        {
            // Ok
        }
        else if (number == 0 && isValidNumber(token) == 1)
        {
            // Ok
        }
        else
        {
            free(ports);
            return 0; // Invalid Port
        }

        if (numPorts == 1)
        {
            port1 = number;
        }
        else if (numPorts == 2)
        {
            // <port1> of <port1>-<port2> must be smaller than <port2>
            if (port1 < number)
            {
                // Ok
            }
            else
            {
                free(ports);
                return 0;
            }
        }

        token = strtok(NULL, "-");
    }

    if (numPorts >= 1 && numPorts <= 2)
    {
        free(ports);
        return 1;
    }
    else
    {
        free(ports);
        return 0;
    }
}

int validateRule(char *ruleToValidate)
{
    char *rule = strdup(ruleToValidate);
    // Rule consists of IP Address and Port separated by space delimiter. Split them apart.
    char *token = strtok(rule, " ");
    int tokenNumber = 0;

    // get IP Addresses and Ports
    char *ipAddresses;
    char *ports;

    while (token != NULL)
    {
        tokenNumber++;
        // printf("\nToken %d: %s",tokenNumber,token);
        if (tokenNumber == 1)
        {
            ipAddresses = token;
        }
        else if (tokenNumber == 2)
        {
            ports = token;
        }

        token = strtok(NULL, " ");
    }
    if (tokenNumber != 2)
    {
        free(rule);
        return 0;
    }

    // printf("\nGot IP Addresses: %s, and Ports: %s", ipAddresses, ports);
    int areIpAddressesValid = ValidateIPAddresses(ipAddresses);
    int arePortsValid = ValidatePorts(ports);
    // printf("\nIP Addresses are Valid?: %d; Ports are Valid?: %d",areIpAddressesValid,arePortsValid);

    if (areIpAddressesValid == 1 && arePortsValid == 1)
    {
        free(rule);
        return 1;
    }
    else
    {
        free(rule);
        return 0;
    }
}
void initializeArrays()
{
    rules = malloc(MAX_RULES * sizeof(*rules)); // Allocate memory for rules
    ruleCounts = malloc(sizeof(*ruleCounts));   // Allocate memory for ruleCounts
    if (rules == NULL || ruleCounts == NULL)
    {
        // Handle allocation failure
        printf("Memory allocation error");
        perror("Memory allocation error");
        // exit(EXIT_FAILURE);
    }
    ruleCounts[0] = 0;
}

void addRule(char *rule, int client_socket)
{
    char response[BUFFERLENGTH];
    pthread_mutex_lock(&mutex);
    sprintf(response, "Rule added\n");
    int totalRules = ruleCounts[0];

    // Check if the rules array is full
    if (totalRules >= MAX_RULES)
    {

        // Double the capacity using realloc
        char(*newRules)[BUFFERLENGTH] = realloc(rules, 2 * MAX_RULES * sizeof(*rules));
        int *newRuleCounts = realloc(ruleCounts, 2 * MAX_RULES * sizeof(*ruleCounts));

        if (newRules == NULL || newRuleCounts == NULL)
        {
            printf("Memory allocation error");
            // Handle reallocation failure
            perror("Memory reallocation error");
            // exit(EXIT_FAILURE);
        }

        // Update the pointers to the new memory
        rules = newRules;
        ruleCounts = newRuleCounts;

        MAX_RULES = totalRules * 2;
    }

    strncpy(rules[totalRules], rule, BUFFERLENGTH - 1);
    ruleCounts[0]++;
    int n = write(client_socket, response, strlen(response));
    if (n < 0)
    {
        perror("ERROR writing to socket\n");
        exit(EXIT_FAILURE);
    }

    pthread_mutex_unlock(&mutex);
}
int isIPInRange(const char *ipAddress, const char *ruleIpAddresses)
{
    char startIP[BUFFERLENGTH];
    char endIP[BUFFERLENGTH];

    // Attempt to parse the ruleIpAddresses into startIP and endIP
    if (sscanf(ruleIpAddresses, "%s-%s", startIP, endIP) == 2)
    {
        // Check if the given IP matches either the start IP or the end IP
        return (strcmp(ipAddress, startIP) == 0 && strcmp(ipAddress, endIP) == 0);
    }
    else
    {
        // Check if the given IP matches the single IP address
        return (strcmp(ipAddress, ruleIpAddresses) == 0);
    }

    return 0;
}
int isPortInRange(const char *port, const char *rulePorts)
{
    char startPort[BUFFERLENGTH];
    char endPort[BUFFERLENGTH];

    if (sscanf(rulePorts, "%s-%s", startPort, endPort) == 2)
    {
        // Check if the given port falls within the range
        int checkPort = atoi(port);
        int startPortNum = atoi(startPort);
        int endPortNum = atoi(endPort);

        return (checkPort == startPortNum || checkPort == endPortNum);
    }
    else
    {
        // Check if the given port matches the single port
        return (atoi(port) == atoi(rulePorts));
    }

    return 0;
}
int isValidMatch(const char *ipAddress, const char *port, const char *rule)
{
    char ruleIpAddresses[BUFFERLENGTH];
    char rulePorts[BUFFERLENGTH];

    // Rule consists of IP Address and Port separated by space delimiter. Split them apart.
    if (sscanf(rule, "%s %s", ruleIpAddresses, rulePorts) != 2)
    {
        return 0; // Invalid rule format
    }
    // printf("\nRULE IP: %s", ruleIpAddresses);
    // printf("\nRULE Ports: %s", rulePorts);
    char *hyphenPos = strchr(ruleIpAddresses, '-');
    if (hyphenPos != NULL)
    {
        // Rule has a range of IP addresses, check if it's in the range
        char startIP[BUFFERLENGTH];
        char endIP[BUFFERLENGTH];

        // Copy substring before hyphen into startIP
        strncpy(startIP, ruleIpAddresses, hyphenPos - ruleIpAddresses);
        startIP[hyphenPos - ruleIpAddresses] = '\0';

        // Copy substring after hyphen into endIP
        strcpy(endIP, hyphenPos + 1);

        // printf("\nTHE START IP is : %s", startIP);
        // printf("\nTHE END IP is : %s", endIP);
        if (strcmp(ipAddress, startIP) == 0 || strcmp(ipAddress, endIP) == 0)
        {
            // IP is within the range
            // printf("\n Mathch found");
        }
        else
        {
            // printf("HEre is the problem");
            return 0; // IP is outside the range
        }
    }
    else
    {
        // Rule has a single IP address, check for an exact match
        if (strcmp(ipAddress, ruleIpAddresses) != 0)
        {
            return 0; // IP does not match
        }
    }

    if (strstr(rulePorts, "-") != NULL)
    {
        // Rule has a range of ports, check if it's in the range
        int startPort, endPort;

        if (sscanf(rulePorts, "%d-%d", &startPort, &endPort) != 2)
        {
            return 0; // Invalid port range format
        }

        int checkPort = atoi(port);

        if (checkPort == startPort || checkPort == endPort)
        {
            // Port is within the range
            // printf("\nALL GOOD");
        }
        else
        {
            // printf("OOOPS");
            return 0; // Port is outside the range
        }
    }
    else
    {
        // Rule has a single port, check for an exact match
        int rulePort = atoi(rulePorts);
        int checkPort = atoi(port);

        if (checkPort != rulePort)
        {
            return 0; // Port does not match
        }
    }

    return 1; // Rule matched
}
void addMatchedQuery(const char *ip, const char *port, int ruleIndex)
{
    // Check if the query already exists for this rule
    for (int i = 0; i < matchedQueriesCount[ruleIndex]; i++)
    {
        if (strcmp(matchedQueries[ruleIndex][i], ip) == 0 && strcmp(matchedQueries[ruleIndex][i], port) == 0)
        {
            return; // The query already exists, no need to add it again
        }
    }
    // Add the matched query to the list
    snprintf(matchedQueries[ruleIndex][matchedQueriesCount[ruleIndex]], BUFFERLENGTH, "%s %s", ip, port);
    matchedQueriesCount[ruleIndex]++;
}

// void printAllMatchedQueries()
// {
//     printf("\nMatched queries stored are : ");
//     for (int i = 0; i < ruleCounts[0]; i++) {
//         printf("\nRule %d:\n", i + 1);
//         for (int j = 0; j < matchedQueriesCount[i]; j++) {
//             printf("%s\n", matchedQueries[i][j]);
//         }
//     }
//     printf("\n");
// }
void checkIpAddress(const char *ip, const char *port, int client_socket)
{
    pthread_mutex_lock(&mutex);
    // printf("the IP is %s and the PORT is %s.\n", ip, port);

    // Check if the IP address and port match any rule
    int matchFound = 0;
    for (int i = 0; i < ruleCounts[0]; i++)
    {
        if (isValidMatch(ip, port, rules[i]))
        {
            // Update the state accordingly
            matchFound = 1;
            addMatchedQuery(ip, port, i);
            // printf("Connection accepted for rule: %s\n", rules[i]);
            int n = write(client_socket, "Connection accepted\n", strlen("Connection accepted\n"));
            if (n < 0)
            {
                error("ERROR writing to socket\n");
            }
            // printAllMatchedQueries() ;
            // addMatchedQuery(i+1, ip, port);
            break; // Assuming only one rule should match
        }
    }

    if (!matchFound)
    {
        // If no match is found, add the IP address and port to the list of matched queries for this rule
        // printf("Connection rejected\n");
        int n = write(client_socket, "Connection rejected\n", strlen("Connection rejected\n"));
        if (n < 0)
        {
            error("ERROR writing to socket\n");
        }
    }
    pthread_mutex_unlock(&mutex);
}
// Function to find the index of a rule
int findRule(const char *rule)
{
    for (int i = 0; i < ruleCounts[0]; i++)
    {
        if (strcmp(rules[i], rule) == 0)
        {
            return i;
        }
    }
    return -1; // Rule not found
}
void deleteRule(const char *rule, int client_socket)
{
    pthread_mutex_lock(&mutex);
    // Check if the rule exists
    int ruleIndex = findRule(rule);
    if (ruleIndex == -1)
    {
        // printf("Rule not found\n");
        // printf("The rules stored are still\n");

        // for (int i = 0; i < ruleCounts[0]; i++) {
        //     printf("\n%s", rules[i]);
        // }
        // printf("\n");
        int n = write(client_socket, "Rule not found\n", strlen("Rule not found\n"));
        if (n < 0)
        {
            error("ERROR writing to socket\n");
        }
    }
    else
    {
        // Parse the rule into IP and port parts
        char ipPart[BUFFERLENGTH];
        char portPart[BUFFERLENGTH];
        if (sscanf(rule, "%s %s", ipPart, portPart) != 2)
        {
            // printf("Invalid rule format\n");
            return;
        }

        // Check if the IP range and port range of the rule match exactly
        if (strcmp(rules[ruleIndex], rule) == 0)
        {
            // Delete the rule and shift the remaining rules
            for (int i = ruleIndex; i < ruleCounts[0] - 1; i++)
            {
                strncpy(rules[i], rules[i + 1], BUFFERLENGTH - 1);
            }
            ruleCounts[0]--;

            // printf("So the rules stored after the deletion\n");
            //  for (int i = 0; i < ruleCounts[0]; i++) {
            //      printf("\n%s", rules[i]);
            //  }
            //  printf("\n");

            // Additional logic to delete the list of IP addresses and ports for this rule
            // ...

            // printf("Rule deleted");
            int n = write(client_socket, "Rule deleted\n", strlen("Rule deleted\n"));
            if (n < 0)
            {
                error("ERROR writing to socket\n");
            }
        }
        else
        {
            // Rule does not match exactly
            // printf("Rule not found\n");
            int n = write(client_socket, "Rule not found\n", strlen("Rule not found\n"));
            if (n < 0)
            {
                error("ERROR writing to socket\n");
            }
        }
    }
    pthread_mutex_unlock(&mutex);
}
void sendAllMatchedQueries(int client_socket)
{
    pthread_mutex_lock(&mutex);

    // Calculate the total length of the response
    size_t totalLength = 0;
    for (int i = 0; i < ruleCounts[0]; i++)
    {
        totalLength += strlen(rules[i]) + 8; // "Rule: " + "\n"
        // printf("the siize length is %ld\n",totalLength);
        for (int j = 0; j < matchedQueriesCount[i]; j++)
        {
            totalLength += strlen(matchedQueries[i][j]) + 8; // "Query: " + "\n"
            // printf("the size2 length is %ld\n",totalLength);
        }
    }

    // Dynamically allocate memory for the response
    char *answer = (char *)malloc(totalLength + 1);

    if (answer == NULL)
    {
        perror("Error allocating memory\n");
        printf("the problem is right here");
        pthread_mutex_unlock(&mutex);
        return;
    }

    answer[0] = '\0';           // Initialize the string
    size_t currentPosition = 0; // Initialize the position variable

    // Construct the response
    for (int i = 0; i < ruleCounts[0]; i++)
    {
        currentPosition += sprintf(answer + currentPosition, "Rule: %s\n", rules[i]);

        for (int j = 0; j < matchedQueriesCount[i]; j++)
        {

            currentPosition += sprintf(answer + currentPosition, "Query: %s\n", matchedQueries[i][j]);
        }
    }

    // Send the response to the client
    int n = write(client_socket, answer, strlen(answer));
    if (n < 0)
    {
        perror("ERROR writing to socket\n");
    }

    // Free dynamically allocated memory
    free(answer);

    pthread_mutex_unlock(&mutex);
}

void *handleClient(void *arg)
{
    int client_socket = *((int *)arg);
    free(arg);
    char buffer[BUFFERLENGTH];
    int n;
    // int tmp;

    bzero(buffer, BUFFERLENGTH);
    n = read(client_socket, buffer, BUFFERLENGTH - 1);
    if (n < 0)
    {

        error("ERROR reading from socket\n");
    }
    buffer[n] = '\0';

    // printf ("Here is the message: %s\n",buffer);
    if (strncmp(buffer, "A", 1) == 0)
    {
        // printf("I GET IT THAT U ENTERED AN A.\n");
        // Skip the command identifier
        if (validateRule(buffer + 2) == 1)
        {

            addRule(buffer + 2, client_socket);
        }
        else
        {
            int n = write(client_socket, "Invalid rule\n", strlen("Invalid rule\n"));
            if (n < 0)
            {
                error("Error writing to the client");
            }
        }
    }

    else if (strncmp(buffer, "C", 1) == 0)
    {
        // printf("I GET IT THAT U ENTERED a C.\n");
        if (validateRule(buffer + 2) == 1)
        {
            // printf("rule valid.\n");
            char ip[BUFFERLENGTH], port[BUFFERLENGTH];
            sscanf(buffer + 2, "%s %s", ip, port);
            checkIpAddress(ip, port, client_socket);
        }
        else
        {
            // printf("IIllegal IP address or port specified.\n");
            int n = write(client_socket, "IIllegal IP address or port specified\n", strlen("IIllegal IP address or port specified\n"));
            if (n < 0)
            {
                error("ERROR writing to socket\n");
            }
        }
    }
    else if (strncmp(buffer, "D", 1) == 0)
    {
        // printf("I GET IT THAT U ENTERED AN D.\n");
        if (validateRule(buffer + 2) == 1)
        {
            // printf("Rule to be deleted is valid SO now im about to check whether it exists .\n");
            deleteRule(buffer + 2, client_socket);
        }
        else
        {
            // printf("Rule Invalid");
            int n = write(client_socket, "Rule invalid\n", strlen("Rule invalid\n"));
            if (n < 0)
            {
                error("ERROR writing to socket\n");
            }
        }
    }
    else if (strncmp(buffer, "L", 1) == 0)
    {
        //    printf("I GET IT THAT U ENTERED AN L.\n");
        sendAllMatchedQueries(client_socket);
    }
    else
    {
        // Illegal request
        send(client_socket, "Illegal request\n", 16, 0);
    }
    if (n < 0)
        error("ERROR writing to socket\n");

    /* important to avoid memory leak */
    close(client_socket);

    return NULL;
}

int main(int argc, char **argv)
{
    int socketfd, client_socket, portno, n;
    struct sockaddr_in6 server_addr, client_addr;
    socklen_t client_len;
    pthread_t thread;

    if (argc < 2)
    {
        error("No port provided\n");
    }

    /*creating a socket */
    socketfd = socket(AF_INET6, SOCK_STREAM, 0);
    // printf("SOCKET DONE .\n");

    if (socketfd < 0)
    {
        error("CANNNOT open the socket\n");
    }

    memset(&server_addr, 0, sizeof(server_addr));
    /*get the port number from the comman line*/
    portno = atoi(argv[1]);
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_addr = in6addr_any;
    server_addr.sin6_port = htons(portno);
    // printf("DETAILS DONE AS WELL .\n");

    if (bind(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        error("Error binding the socket\n");
    }

    /*listen to upcoming connections */
    n = listen(socketfd, 5);
    if (n < 0)
    {
        error("Error cannot listen to connections\n");
    }
    // printf("Server listening on port :%d.\n", portno);
    client_len = sizeof(client_addr);

    /*Initialize the mutex */
    if (pthread_mutex_init(&mutex, NULL) != 0)
    {
        perror("pthread_mutex_init");
        return EXIT_FAILURE;
    }

    /*enter an infinite loop so that the server runs forever */
    initializeArrays();
    while (1)
    {
        client_socket = accept(socketfd, (struct sockaddr *)&server_addr, &client_len);
        if (client_socket < 0)
        {
            error("Error accepting the connection\n");
        }
        // printf("COnnection accepted\n");

        /*create a socket to handle the client request*/
        int *client_socket_ptr = malloc(sizeof(int));
        *client_socket_ptr = client_socket;

        if (pthread_create(&thread, NULL, handleClient, (void *)client_socket_ptr) != 0)
        {
            error("Error thread creation failed\n");
        }

        // Detach the thread to clean up resources automatically
        pthread_detach(thread);
    }
    /* Close the server socket */
    close(socketfd);

    /*destroy the mutex */
    pthread_mutex_destroy(&mutex);
    free(rules);
    free(ruleCounts);
    return 0;
}
