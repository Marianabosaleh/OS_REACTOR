CC = gcc
CFLAGS = -Wall -Wextra -pedantic -pthread

# List of source files for server and client
SERVER_SOURCES = st_reactor.c reactor_server.c
CLIENT_SOURCES = client.c

# Object files for server and client
SERVER_OBJECTS = $(SERVER_SOURCES:.c=.o)
CLIENT_OBJECTS = $(CLIENT_SOURCES:.c=.o)

# Executable names
SERVER_EXECUTABLE = reactor_server
CLIENT_EXECUTABLE = client

all: $(SERVER_EXECUTABLE) $(CLIENT_EXECUTABLE)

$(SERVER_EXECUTABLE): $(SERVER_OBJECTS)
	$(CC) $(CFLAGS) $(SERVER_OBJECTS) -o $(SERVER_EXECUTABLE)

$(CLIENT_EXECUTABLE): $(CLIENT_OBJECTS)
	$(CC) $(CFLAGS) $(CLIENT_OBJECTS) -o $(CLIENT_EXECUTABLE)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SERVER_OBJECTS) $(CLIENT_OBJECTS) $(SERVER_EXECUTABLE) $(CLIENT_EXECUTABLE)
