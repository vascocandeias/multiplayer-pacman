/*****************************************************************************
 * File name: communication.h
 *
 *  Author: Vasco Candeias nº 84196
 *
 *  Release date: 29/04/2020
 *
 *  Description: Headers file for the communication.c functions
 *
 ****************************************************************************/
#ifndef COMMUNICATION_H
#define COMMUNICATION_H

int init_server(int port);
void* thread_accept(void* arg);

#endif
