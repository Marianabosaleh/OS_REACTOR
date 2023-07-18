#ifndef ST_REACTOR_H
#define ST_REACTOR_H

#include <pthread.h>

#define MAX_DATA_SIZE 1024

typedef struct reactor_t reactor_t;

typedef void (*handler_t)(int fd, reactor_t *reactor);

typedef struct
{
    int fd;
    handler_t handler;
    reactor_t *reactor;
} registry_entry_t;

struct reactor_t
{
    pthread_t thread_id;
    int active;
    pthread_mutex_t lock;
    registry_entry_t *registry;
    int registry_size;
};

reactor_t *createReactor();
void stopReactor(reactor_t *reactor);
void startReactor(reactor_t *reactor);
void addFd(reactor_t *reactor, int fd, handler_t handler);
void waitfor(reactor_t *reactor);
void echo_handler(int fd, reactor_t *reactor);
void accept_handler(int fd, reactor_t *reactor);

#endif
