/*****************************************************************************
 * File name: communication.h
 *
 *  Author: Vasco Candeias (vascocandeias@tecnico.ulisboa.pt)
 *
 *  Release date: 01/06/2020
 *
 *  Description: Headers file for the communication.c functions
 *
 ****************************************************************************/
#ifndef COMMUNICATION_H
#define COMMUNICATION_H

/*
 * description: initialize the server
 *
 * argument: port [int] - port to which the socket should be binded
 * returns: int - server socket file descriptor
 */
int init_server(int port);

/*
 * description: thread responsible for accepting new clients
 *
 * argument: arg [int*] - server socket file descriptor
 * returns: void* - NULL
 */
void* thread_accept(void* arg);

#endif
