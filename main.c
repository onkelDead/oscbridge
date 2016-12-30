/*
 *  Copyright (C) 2016 Detlef Urban (onkel@paraair.de)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as
 *  published by the Free Software Foundation; either version 2.1 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <lo/lo.h>

#define VERSION "0.1.12"

#define MAX_CLIENTS 16

lo_address ardour_client = NULL;
lo_server_thread ardour_server;

lo_address ardmix_client[MAX_CLIENTS];
int cClients;
lo_server_thread ardmix_server;

typedef struct {
    char* client_port;
    char* server_port;
    char* server_ip;
    int debug_level;

} Arguments;

static char default_client[] = {"7770"};
static char default_udp[] = {"3819"};
static char default_host[] = {"127.0.0.1"};
int done = 0;
Arguments arg;

static void
usage (FILE *f, const char *me) {
	fprintf (f, "OSC TCP/UDP bridge %s.\n", VERSION);
	fprintf (f, "Usage: oscbridge [OPTIONS]\n"
		"\n"
		"parameters:\n"
		"  -c TCP_PORT      The port the OSC client connects to\n"
                "  -u UDP_PORT      The udp port of the server\n"
                "  -s HOST_IP       The ip of the udp server\n"
		"  -h               This help\n"
                "  -d DEBUG_LEVEL   Enable debugging."
		"\n"
		);
}

static void
parse_arguments(int argc, char **argv, Arguments* arg) {
    int c;
    
    arg->debug_level = 0;
    arg->server_ip = strdup(default_host);
    arg->server_port = strdup(default_udp);
    arg->client_port = strdup(default_client);
    for (;;) {
        static const char *short_options = "t:u:s:hd:";

        c = getopt(argc, argv, short_options);
        if (c == -1)
            break;
        switch (c) {
            case 'h':
                usage(stdout, argv[0]);
                exit(0);
            case 't':
                arg->client_port = strdup(optarg);
                break;
            case 'u':
                arg->server_port = strdup(optarg);
                break;
            case 's':
                arg->server_ip = strdup(optarg);
                break;
                
            case 'd':
                arg->debug_level = atoi(optarg);
                break;
                
            default:
                fprintf(stderr, "?? getopt returned character code 0%o ??\n", c);
                break;
        }
    }
}


void ctrlc(int sig) {
    done = 1;
}

void dump_message(const char* path, const char *types, lo_arg ** argv,
        int argc ) {
    int i;
    
    fprintf(stdout, "ardour path: <%s> - ", path);
    for (i = 0; i < argc; i++) {
        fprintf(stdout, " '%c':", types[i]);
        lo_arg_pp((lo_type) types[i], argv[i]);
    }
    fprintf(stdout, "\n");
    fflush(stdout);
    
}

void ardour_err_handler(int num, const char *msg, const char *where) {
    fprintf(stderr, "ARDOUR_ERROR %d: %s at %s\n", num, msg, where);
}

void ardmix_err_handler(int num, const char *msg, const char *where) {
    fprintf(stderr, "ARDMIX_ERROR %d: %s at %s\n", num, msg, where);
}

const int client_exists(lo_address client) {
    int i;
    for( i = 0; i < MAX_CLIENTS; i++ ) {
        if( ardmix_client[i] && !strcmp(lo_address_get_url(lo_message_get_source(client)), lo_address_get_url(ardmix_client[i])))
            return 1;
    }
    return 0;
}

void new_client(lo_address client) {
    int i;
    for(i = 0; i < MAX_CLIENTS; i++) {
        if( ardmix_client[i] == NULL) {
           ardmix_client[i] = lo_message_get_source(client); 
            break;
        }
    }
}

int ardour_handler(const char *path, const char *types, lo_arg ** argv, int argc, lo_message data, void *user_data) {
    int ret = 0;
    int i;
    
    if( arg.debug_level & 2 )
        dump_message(path, types, argv, argc);

    for( i = 0; i < MAX_CLIENTS; i++ ) {
        if( ardmix_client[i]) {
            ret = lo_send_message_from(ardmix_client[i], ardmix_server, path, data);

            if (ret == -1) {
                fprintf(stderr, "OSC client error %d: %s on %s\n", 
                        lo_address_errno(ardmix_client[i]), lo_address_errstr(ardmix_client[i]), lo_address_get_hostname(lo_message_get_source(data)));
            }
        }
    }


    return 0;
}

int ardmix_handler(const char *path, const char *types, lo_arg ** argv, int argc, lo_message data, void *user_data) {
    int ret;

    if( arg.debug_level & 1)
        dump_message(path, types, argv, argc);


    if (!client_exists(data)) {
        fprintf(stdout, "new client: %s\n", lo_address_get_url(lo_message_get_source(data)));
        fflush (stdout);
        new_client(data);
        
    }

    ret = lo_send_message(ardour_client, path, data);

    if (ret == -1) {
        fprintf(stderr, "OSC server error %d: %s on %s\n", lo_address_errno(ardour_client), lo_address_errstr(ardour_client), lo_address_get_url(ardour_client));
    }
    return 0;
}

int main(int argc, char *argv[]) {
    int i;
    
    parse_arguments(argc, argv, &arg);

    ardmix_server = lo_server_thread_new_with_proto(arg.client_port, LO_TCP, ardmix_err_handler);
    lo_server_thread_add_method(ardmix_server, NULL, NULL, ardmix_handler, NULL);
    lo_server_thread_start(ardmix_server);

    for( i = 0; i < MAX_CLIENTS; i++ ) {
        ardmix_client[i] = NULL;
    }
    
    ardour_server = lo_server_thread_new("3820", ardour_err_handler);
    lo_server_thread_add_method(ardour_server, NULL, NULL, ardour_handler, NULL);
    lo_server_thread_start(ardour_server);

    ardour_client = lo_address_new("127.0.0.1", arg.server_port);

    signal(SIGINT, ctrlc);

    while (!done) {
        sleep(1);
    };

    lo_server_thread_free(ardour_server);
    lo_server_thread_free(ardmix_server);

    fprintf(stdout, "\ngood bye :)\n");
    
    free(arg.server_ip);
    free(arg.server_port);
    free(arg.client_port);
    
    return 0;
}
