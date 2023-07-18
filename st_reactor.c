#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include "st_reactor.h"

reactor_t *createReactor()
{
    reactor_t *reactor = (reactor_t *)malloc(sizeof(reactor_t));
    if (reactor == NULL)
    {
        perror("Failed to allocate memory for reactor");
        return NULL;
    }
    reactor->active = 0;
    pthread_mutex_init(&(reactor->lock), NULL);
    reactor->registry = NULL;   // initialize registry pointer
    reactor->registry_size = 0; // initialize registry size
    return reactor;
}

void *reactor_loop(void *arg)
{
    reactor_t *reactor = (reactor_t *)arg;
    while (reactor->active)
    {
        waitfor(reactor);
    }
    return NULL;
}

void stopReactor(reactor_t *reactor)
{
    reactor->active = 0;
    for (int i = 0; i < reactor->registry_size; i++) {
        if (reactor->registry[i].fd != 0) {
            close(reactor->registry[i].fd);
        }
    }
    pthread_join(reactor->thread_id, NULL);
    free(reactor->registry);
    reactor->registry = NULL;
    reactor->registry_size = 0;
}



void startReactor(reactor_t *reactor)
{
    reactor->active = 1;
    if (pthread_create(&reactor->thread_id, NULL, &reactor_loop, reactor))
    {
        perror("Error creating thread");
        reactor->active = 0;
    }
}

void addFd(reactor_t *reactor, int fd, handler_t handler)
{
    pthread_mutex_lock(&(reactor->lock));
    reactor->registry_size++; // increment the size
    reactor->registry = (registry_entry_t *)realloc(reactor->registry, reactor->registry_size * sizeof(registry_entry_t));
    if (reactor->registry == NULL)
    {
        perror("Failed to allocate memory for registry");
        reactor->registry_size--;
        pthread_mutex_unlock(&(reactor->lock));
        return;
    }
    reactor->registry[reactor->registry_size - 1].fd = fd;
    reactor->registry[reactor->registry_size - 1].handler = handler;
    reactor->registry[reactor->registry_size - 1].reactor = reactor; // set the reactor pointer
    pthread_mutex_unlock(&(reactor->lock));
}

void waitfor(reactor_t *reactor)
{
    int i;
    int maxfd = 0;
    fd_set readset;
    FD_ZERO(&readset);

    pthread_mutex_lock(&(reactor->lock));
    for (i = 0; i < reactor->registry_size; i++)
    {
        if (reactor->registry[i].fd != 0)
        {
            FD_SET(reactor->registry[i].fd, &readset);
            if (reactor->registry[i].fd > maxfd)
            {
                maxfd = reactor->registry[i].fd;
            }
        }
    }
    pthread_mutex_unlock(&(reactor->lock));

    select(maxfd + 1, &readset, NULL, NULL, NULL);

    for (i = 0; i < reactor->registry_size; i++)
    {
        if (reactor->registry[i].fd != 0)
        {
            if (FD_ISSET(reactor->registry[i].fd, &readset))
            {
                reactor->registry[i].handler(reactor->registry[i].fd, reactor->registry[i].reactor); // pass the reactor pointer to the handler
            }
        }
    }
}


