#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "debug.h"
#include "client_registry.h"
#include "transaction.h"
#include "store.h"

#include "student/util.h"
#include "student/options.h"
#include "student/signaling.h"
#include "student/server_ops.h"

static void terminate(int status);

void sighup_handler(int sig)
{
    free(connfdp);
    debug("SIGHUP received, terminating");
    terminate(EXIT_SUCCESS);
}

CLIENT_REGISTRY *client_registry;

int main(int argc, char* argv[]){
    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen.
    int opt;
    int opt_index = 0;

    int port = -1;

    while ((opt = getopt_long(argc, argv, STUDENT_SHORT_OPTIONS, STUDENT_LONG_OPTIONS, &opt_index)) != -1)
    {
        switch (opt)
        {
        case 'p':
            if (str_to_int(optarg, &port) == NULL)
            {
                fprintf(stderr, "Invalid port number\n");
                exit(EXIT_FAILURE);
            }
            break;
        default:
            fprintf(stderr, "Usage: %s -p <port>\n", argv[0]);
            exit(EXIT_SUCCESS);
        }
    }

    if (port < 0)
    {
        fprintf(stderr, "Usage: %s -p <port>\n", argv[0]);
        exit(EXIT_SUCCESS);
    }

    info("running on port: %d", port);

    // Perform required initializations of the client_registry,
    // transaction manager, and object store.
    client_registry = creg_init();
    trans_init();
    store_init();

    // TODO: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function xacto_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.

    // Install SIGHUP handler
    install_signal_handler(SIGHUP, sighup_handler, SA_RESTART);
    // int i = 1;
    // loop to accept connections
    // while (1)
    // {
        listen_for_connections(port);
    // }
    debug("SIGHUP received, terminating");
    terminate(EXIT_SUCCESS);
    /* while (i--)
    {
        // listen_for_connections(port);
        test_client_registry(client_registry);
        terminate(EXIT_SUCCESS);
    } */
}

/*
 * Function called to cleanly shut down the server.
 */
void terminate(int status) {
    // Shutdown all client connections.
    // This will trigger the eventual termination of service threads.
    creg_shutdown_all(client_registry);
    
    debug("Waiting for service threads to terminate...");
    creg_wait_for_empty(client_registry);
    debug("All service threads terminated.");

    // Finalize modules.
    creg_fini(client_registry);
    trans_fini();
    store_fini();

    debug("Xacto server terminating");
    exit(status);
}
