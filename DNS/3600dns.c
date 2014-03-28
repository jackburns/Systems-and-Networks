/*
 * CS3600, Spring 2013
 * Project 3 Starter Code
 * (c) 2013 Alan Mislove
 *
 */

#include <math.h>
#include <ctype.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "3600dns.h"


int process_name(unsigned char* packet, char* qname, int offset );
void parse_server(char* serverPort);
void parse_name(char* name);


/**
 * This function will print a hex dump of the provided packet to the screen
 * to help facilitate debugging.  In your milestone and final submission, you 
 * MUST call dump_packet() with your packet right before calling sendto().  
 * You're welcome to use it at other times to help debug, but please comment those
 * out in your submissions.
 *
 * DO NOT MODIFY THIS FUNCTION
 *
 * data - The pointer to your packet buffer
 * size - The length of your packet
 */
static void dump_packet(unsigned char *data, int size) {
    unsigned char *p = data;
    unsigned char c;
    int n;
    char bytestr[4] = {0};
    char addrstr[10] = {0};
    char hexstr[ 16*3 + 5] = {0};
    char charstr[16*1 + 5] = {0};
    for(n=1;n<=size;n++) {
        if (n%16 == 1) {
            /* store address for this line */
            snprintf(addrstr, sizeof(addrstr), "%.4x",
               ((unsigned int)p-(unsigned int)data) );
        }
            
        c = *p;
        if (isprint(c) == 0) {
            c = '.';
        }

        /* store hex str (for left side) */
        snprintf(bytestr, sizeof(bytestr), "%02X ", *p);
        strncat(hexstr, bytestr, sizeof(hexstr)-strlen(hexstr)-1);

        /* store char str (for right side) */
        snprintf(bytestr, sizeof(bytestr), "%c", c);
        strncat(charstr, bytestr, sizeof(charstr)-strlen(charstr)-1);

        if(n%16 == 0) { 
            /* line completed */
            printf("[%4.4s]   %-50.50s  %s\n", addrstr, hexstr, charstr);
            hexstr[0] = 0;
            charstr[0] = 0;
        } else if(n%8 == 0) {
            /* half line: add whitespaces */
            strncat(hexstr, "  ", sizeof(hexstr)-strlen(hexstr)-1);
            strncat(charstr, " ", sizeof(charstr)-strlen(charstr)-1);
        }
        p++; /* next byte */
    }

    if (strlen(hexstr) > 0) {
        /* print rest of buffer if not empty */
        printf("[%4.4s]   %-50.50s  %s\n", addrstr, hexstr, charstr);
    }
}

int main(int argc, char *argv[]) {
  /**
   * I've included some basic code for opening a socket in C, sending
   * a UDP packet, and then receiving a response (or timeout).  You'll 
   * need to fill in many of the details, but this should be enough to
   * get you started.
   */

    if (argc < 3 || argc > 4) {
         printf("Usage: ./3600dns [-ns|-mx] @<server:port> <name>\n");
        exit(1);
    } 
  int serverType = 1;

  if (argc == 3) {
    parse_server(argv[1]);
    parse_name(argv[2]);
  }

    if (argc == 4) {
        if (strcmp(argv[1], "-ns")){
            serverType = 2;
        }
        if (strcmp(argv[1], "-mx")){
            serverType = 15;
        } 
        else {
            return -1;
        }
        parse_server(argv[2]);
        parse_name(argv[3]);
     }   
    unsigned char* packet = (unsigned char*) calloc(MAXPACKETSIZE, sizeof(char));
    header* newHeader = (header*) calloc(1, sizeof(header));
    question* newQuestion = (question*) calloc(1, sizeof(question));
    answer* newAnswer = (answer*) calloc(1, sizeof(answer));

    // construct header
    newHeader->qr = 0;
    newHeader->aa = 0;
    newHeader->tc = 0;
    newHeader->aa = 0;
    newHeader->ra = 0;
    newHeader->z = 0;
    newHeader->rcode = 0;
    newHeader->ancount = htons(0);
    newHeader->nscount = htons(0);
    newHeader->arcount = htons(0);

    newHeader->qdcount = htons(1);
    newHeader->id = htons(QUERYID);
    newHeader->rd = 1;

    // construct question
    newQuestion->qclass = htons(0x0001);


    if(serverType == 15){
      newQuestion->qtype = htons(15);
    }
    else if(serverType == 2){
      newQuestion->qtype = htons(2);
    }
    else if(serverType == 1){
      newQuestion->qtype = htons(1);
    }

    int packetLength = 0;
    memcpy(packet, newHeader, sizeof(header));
    packetLength = packetLength + sizeof(header);
    memcpy(packet + packetLength, name, nameSize);
    packetLength = packetLength + nameSize;

    memcpy(packet + packetLength, newQuestion, sizeof(question));
    packetLength = packetLength + sizeof(question);

   // send the DNS request (and call dump_packet with your request)
    dump_packet(packet, packetLength);

   // first, open a UDP socket  
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

   // next, construct the destination address
    struct sockaddr_in out;
    out.sin_family = AF_INET;
    out.sin_port = htons(port);
    out.sin_addr.s_addr = inet_addr(server);

    if (sendto(sock, packet, packetLength, 0, (struct sockaddr*)&out, sizeof(out)) < 0) {
        printf("ERROR \t in sendto()\n");
        return - 1;
    }

    //---------------------------------Milestone

    //clear buffer for receiving response 
    memset(newQuestion, 0, sizeof(question));
    memset(newHeader, 0, sizeof(header));

    memset(packet, 0, MAXPACKETSIZE);
    packetLength = 0;

    struct sockaddr_in in;
    socklen_t in_len = sizeof(struct sockaddr_in);

    fd_set socks;
    FD_ZERO(&socks);
    FD_SET(sock, &socks);

    struct timeval t;
    t.tv_sec = 5;
    t.tv_usec = 0;
    
     if (select(sock + 1, &socks, NULL, NULL, &t)) {
        in_len = sizeof(in);
        int status;
        status = recvfrom(sock, packet, MAXPACKETSIZE, 0, (struct sockaddr*) &in, &in_len);
        if ( status < 0) {
            printf("ERROR \t in recvfrom\n");
            exit(1);    
        }
        } else {

        printf("NORESPONSE");
        exit(1);
    }
        
    // parse question header
    memcpy(newHeader,packet,sizeof(header) );
    packetLength = sizeof(header);
    int ancount = ntohs(newHeader->ancount);

    // parse question qname
    char* qname = calloc(150, sizeof(char* ));
    int len = process_name( packet, qname, sizeof(header) );
    if (!strcmp((char*) argv[2],(char*) qname + 1)) {
        printf("ERROR\t name mismatch\n");
        exit(1);
    }
        
    packetLength += len;
    memcpy( newQuestion, packet + packetLength, sizeof(question) );
      
    packetLength += sizeof(question); 
    do {
    memset(qname,0,150);
    len = process_name(packet,qname,packetLength);
    if (!strcmp((char*)argv[2],(char*)qname+1)) {
        return -1;
    }
    packetLength += len;

    memcpy(newAnswer,packet+packetLength,sizeof(answer));
    int atype = ntohs(newAnswer->type);
    if (atype != 1 && 
        atype != 5 && 
        atype != 15 && 
        atype != 2 ||  
        ntohs(newAnswer->class) != 1 ) {
        printf("NOTFOUND\n");
        return 1;
    }

    packetLength += sizeof(answer) - 2;
    char* rdata = calloc(150,sizeof(char));
    short preference = 0;

    // printing if ip
    if (ntohs(newAnswer->type) == 1) {

     char answer = packet[packetLength];
    int pos = packetLength + 1;
    unsigned char segments[4];
    for (int i = 0; i < 4; i++) {
        segments[i] = answer;
        answer = packet[pos];
        pos++;
    }
    sprintf((char*) rdata,"%d.%d.%d.%d",segments[0],segments[1],segments[2],segments[3]);

    printf("IP\t%s",rdata);
    packetLength += ntohs(newAnswer->rdlength);

    // printing if cname
    } else if (ntohs(newAnswer->type) == 5) {
        len = process_name(packet,rdata,packetLength);
        printf("CNAME\t%s",rdata);
        packetLength += len;

    }
    
    // print authorization        
    if ( newHeader->aa) {
        printf("\tauth\n");
    }else{
        printf("\tnonauth\n");
    }

    ancount--;
      } while (ancount);
    
    free(newHeader);
    free(newQuestion);
    free(packet);
    free(newAnswer);
    return 0;
}

int process_name(unsigned char* packet, char* qname, int offset ) {
    unsigned char chars = packet[offset];
    int loc = offset + 1;
    int len = 0;
    int final_loc = 0;
    int x;
    while (chars != 0) {
        if (chars & 192) {
            chars = packet[loc];
            if (!final_loc) {
                final_loc = loc+1;
            }
            loc = chars;
        } else {
            for (int x = 0; x < chars; x++) {
                qname[len] = packet[loc];
                loc++;
                len++;
            }
            qname[len] = '.';
            len++;
        }
        chars = packet[loc];
        loc++;
    }

    if (qname[len - 1] == '.') {
        len--;
    }
    qname[len] = 0; 
    if (!final_loc) {
                final_loc = loc;
    }
    return final_loc - offset;
}


void parse_server(char* arg) {
  char* ip = strtok(arg, ":");

  char* portPos = strtok(NULL, ":");

  if(portPos != NULL) {
    port = atoi(portPos);
    }
  else {
    port = 53;
    }

  char* token = malloc(strlen(ip));
  strcpy(token, ip);
  token = strtok(token, ".");
  token++;

  int i;
  for(i = 0; i < 3; i++) {
    token = strtok(NULL, ".");
    if(atoi(token) > 255){
      perror("ERROR \t Invalid server format\n");
      exit(1);
    }
  }
  server = (ip + 1); 
}

void parse_name(char* arg) {

  nameSize = strlen(arg) + 2;
  char result[nameSize];
  char* seg = strtok(arg, ".");

  int j = 0;
  int len = 0;
  int i;

  while(seg != NULL) {
    len = strlen(seg);
    result[j] = len;
    j++;

    for(i = 0; i < len; i++) {
      result[j] = seg[i];
      j++;
    }
    seg = strtok(NULL, ".");
  }
  result[j] = 0;
  name = malloc(nameSize);
  memcpy(name, &result, nameSize);
}
