#include "server.h"

int main(int argc, char **argv) {
    // Flags parsing
    int port = DEFAULT_PORT, debug = 1;
    for (int opt; (opt = getopt(argc, argv, "::pd") != -1);) {
        switch (opt)
        {
        case 'p':
            port = atoi(optarg);
            break;

        case 'd':
            debug = 1;
            break;
        }
    }

    // Variable setup
    int sserver = 0, sclient;
    struct sockaddr_in sserver_address, sclient_address;
    socklen_t address_length = sizeof(sclient_address);

    // Initialize a socket
    if (init_socket(sserver, sserver_address, port, debug) == -1) {
        return -1;
    }

    // Keeping the server alive until EOF
    char *eof_buff = malloc(sizeof(char));
    for (; fgets(eof_buff, 1, stdin);) {
        // Case in which a client connects
        sclient = accept(sserver, (struct sockaddr *) &sclient_address, (socklen_t *) &address_length);
        if (sclient == -1) {
            perror("Error");
            continue;
        }
        if (debug) printf("Connection established");
        
        // Creating a thread for handling the client request
        pthread_t connection_thread;
        int *ptr_sclient = malloc(sizeof(int));
        *ptr_sclient = sclient;
        pthread_create(&connection_thread, NULL, success_handler, ptr_sclient);

        // Closing connection when done
        close(sclient);
    }

    // Cleaning up
    free(eof_buff);
    close(sserver);
    return 0;
}

int init_socket(int sserver, struct sockaddr_in sserver_address, int port, int debug) {
    // Requesting a TCP socket
    if (debug) printf("Requesting a socket from the system...\n");
    sserver = socket(AF_INET, SOCK_STREAM, 0);
    if (sserver == -1) {
        perror("Error");
        return -1;
    }

    // Setting the server to listen for any internet addresses on set port
    if (debug) printf("Setting the server to listen for any internet addresses on port %d...\n", port);
    sserver_address.sin_family = AF_INET;
    sserver_address.sin_addr.s_addr = INADDR_ANY;
    sserver_address.sin_port = htons(port);

    // Setting the socket to listen on the server address
    if (debug) printf("Setting the socket to listen on the server address...\n");
    if (bind(sserver, (struct sockaddr*) &sserver_address, sizeof(sserver_address)) == -1) {
        perror("Error");
        close(sserver);
        return -1;
    }

    // Listening on the socket
    if (debug) printf("Listening on the socket...\n");
    if (listen(sserver, SOCKET_QUEUE_MAX) == -1) {
        perror("Error");
        close(sserver);
        return -1;
    }

    return 0;
}

void *success_handler(void *ptr_sclient) {
    // Making the argument a local variable
    int sclient = *((int*) ptr_sclient);
    free(ptr_sclient);

    // Creating a request_buffer and filling it 0s
    char request_buffer[BUFFER_MAX];
    memset(request_buffer, 0, BUFFER_MAX);

    // Reading from client socket into the request_buffer while there are still bits in there
    size_t bytes;
    int message_size = 0;
    for (;(bytes = read(sclient, request_buffer + message_size, sizeof(request_buffer) - message_size - 1)) > 0; message_size += bytes) {
        if (message_size > BUFFER_MAX - 1) {
            break;
        }

        if (request_buffer[message_size - 1] == '\n') {
            break;
        }
    }

    // Verifing path is valid
    char resolved_path[PATH_MAX];
    if (realpath(request_buffer, resolved_path) == NULL) {
        perror("Error");
        close(sclient);
        return NULL;
    }

    // Openening the mf requested file
    FILE *fp = fopen(resolved_path, "r");
    if (!fp) {
        perror("Error");
        close(sclient);
        return NULL;
    }

    // Responding to the client
    char response_buffer[BUFFER_MAX];
    for (;(bytes = fread(response_buffer, 1, BUFFER_MAX, fp)) > 0; write(sclient, response_buffer, bytes));
    return NULL;
}