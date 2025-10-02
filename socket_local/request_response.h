//
// Created by ivan on 27.02.25.
//

#ifndef SOCKET_REQUEST_RESPONSE_H
#define SOCKET_REQUEST_RESPONSE_H

#define SOCK_NAME "serv.sock"

struct request_t
{
    double first_operand, second_operand;
    char operation;
};

typedef struct request_t request_t;

#endif //SOCKET_REQUEST_RESPONSE_H
