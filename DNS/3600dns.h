/*
 * CS3600, Spring 2013
 * Project 2 Starter Code
 * (c) 2013 Alan Mislove
 *
 */

#ifndef __3600DNS_H__
#define __3600DNS_H__

#define MAXPACKETSIZE 65540
#define QUERYID 1337

int port;
unsigned char* server;
int packetLength;
unsigned char* name;
int nameSize;

typedef struct header_s{
    unsigned int id : 16; 
    unsigned int rd : 1;
    unsigned int tc : 1;
    unsigned int aa : 1;
    unsigned int opcode : 4;
    unsigned int qr : 1;
    unsigned int ra : 1;
    unsigned int z : 3;  
    unsigned int rcode : 4;
    unsigned int qdcount : 16;
    unsigned int ancount : 16;
    unsigned int nscount : 16;
    unsigned int arcount : 16;     
} header;

typedef struct answer_S{
    unsigned int type : 16; 
    unsigned int class : 16;
    unsigned int ttl : 32;
    unsigned int rdlength : 16;
} answer;

typedef struct question_s{
    unsigned int qtype : 16;
    unsigned int qclass : 16;  
} question;


#endif