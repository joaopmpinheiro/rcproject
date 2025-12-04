#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>  // For socket functions
#include <sys/socket.h>  // For socket functions
#include <sys/types.h>  // For data types
#include <netinet/in.h>  // For sockaddr_in
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>  // For time functions
#include <signal.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <pthread.h>

#include "../../common/common.h"

#define BUFFER_SIZE 1024 //TODO: confirmar
#define TIMEOUT_SECONDS 5
#define PASSWORD_LENGTH 8
#define MAX_EVENT_NAME 10
#define DATE_LENGTH 11
#define TIME_LENGTH 5
#define MIN_SEATS 10
#define MAX_SEATS 999
#define ERROR -1
#define MAX_TCP_CLIENTS 5 //TODO: isto é uma cena?



typedef struct{
    int EID;
    char description[MAX_EVENT_NAME];
    char* file;
    char date[DATE_LENGTH];
    char time[TIME_LENGTH];
    int total_seats;
    int available_seats;
    int status; //TODO: confirmar e defenir estados
    int num_reservations;
} Event;

typedef struct EventNode {
    Event event;
    struct EventNode* next;
} EventNode;

typedef struct {
    Event* event;
    int num_seats;
} Reservation;

typedef struct ReservationNode {
    Reservation reservation;
    struct ReservationNode* next;
} ReservationNode;

typedef struct {
    int status;
    int UID;
    char password[PASSWORD_LENGTH];
    EventNode* created_events; //linked list of event names
    ReservationNode* reserved_events; //linked list of event names 
} User;


typedef struct {
    int client_socket;
    struct sockaddr_in client_addr;
    socklen_t addr_len;
    int is_tcp;
    char buffer[BUFFER_SIZE];
} Request;

typedef struct {
    Request req[MAX_TCP_CLIENTS];
    int front;
    int rear;
    int count;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} RequestQueue;


typedef struct {
    int verbose;
    char* port;
    int udp_socket;
    int tcp_socket;
    fd_set read_fds;
    fd_set temp_fds;
    struct timeval timeout;
} Set;

#endif 