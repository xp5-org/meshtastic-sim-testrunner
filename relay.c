#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>

#define PORT 1883
#define MAX_CLIENTS 2
#define BUFFER_SIZE 1024


#define MQTT_CONNECT 0x10
#define MQTT_CONNACK 0x20
#define MQTT_PUBLISH 0x30
#define MQTT_PINGREQ 0xC0
#define MQTT_PINGRESP 0xD0

int main(void)
{
    int server_fd, client_fd[MAX_CLIENTS];
    struct sockaddr_in addr;
    int i, max_sd, activity;
    fd_set readfds;
    unsigned char buffer[BUFFER_SIZE];

    for(i = 0; i < MAX_CLIENTS; i++) client_fd[i] = -1;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd < 0) { perror("socket"); exit(1); }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if(bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind"); exit(1); 
    }

    if(listen(server_fd, MAX_CLIENTS) < 0) { perror("listen"); exit(1); }

    printf("MQTT relay server running on port %d\n", PORT);

    while(1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        max_sd = server_fd;

        for(i = 0; i < MAX_CLIENTS; i++) {
            if(client_fd[i] > 0) FD_SET(client_fd[i], &readfds);
            if(client_fd[i] > max_sd) max_sd = client_fd[i];
        }

        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if(activity < 0) { perror("select"); continue; }

        // accept new connection
        if(FD_ISSET(server_fd, &readfds)) {
            int new_sd = accept(server_fd, NULL, NULL);
            if(new_sd < 0) { perror("accept"); continue; }

            for(i = 0; i < MAX_CLIENTS; i++) {
                if(client_fd[i] < 0) {
                    client_fd[i] = new_sd;
                    printf("[CONNECT] New client connected, slot %d\n", i);
                    break;
                }
            }
        }

        // check client for data
        for(i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_fd[i];
            int j;
            if(sd < 0) continue;

            if(FD_ISSET(sd, &readfds)) {
                int valread = read(sd, buffer, BUFFER_SIZE);
                if(valread <= 0) {
                    printf("[DISCONNECT] client slot %d disconnected\n", i);
                    close(sd);
                    client_fd[i] = -1;
                    continue;
                }

                unsigned char pkt_type = buffer[0] & 0xF0;

                switch(pkt_type) {
                    case MQTT_CONNECT:
                        printf("[CONNECT] Client slot %d sent CONNECT\n", i);
                        /* Send CONNACK: 0x20 0x02 0x00 0x00 (success) */
                        buffer[0] = MQTT_CONNACK;
                        buffer[1] = 0x02;
                        buffer[2] = 0x00;
                        buffer[3] = 0x00;
                        write(sd, buffer, 4);
                        printf("[CONNACK] sent to client slot %d\n", i);
                        break;

                    case MQTT_PUBLISH:
                        printf("[PUBLISH] client slot %d sent %d bytes\n", i, valread);
                        
                        // echo bytes to other clients
                        for(j = 0; j < MAX_CLIENTS; j++) {
                            if(j != i && client_fd[j] >= 0) {
                                write(client_fd[j], buffer, valread);
                                printf("[ECHO] Sent %d bytes from slot %d to slot %d\n", valread, i, j);
                            }
                        }
                        break;

                    case MQTT_PINGREQ:
                        printf("[PINGREQ] from client slot %d\n", i);
                        buffer[0] = MQTT_PINGRESP;
                        buffer[1] = 0x00;
                        write(sd, buffer, 2);
                        printf("[PINGRESP] sent to client slot %d\n", i);
                        break;

                    default:
                        printf("[UNKNOWN] client slot %d sent type 0x%02X\n", i, pkt_type);
                        break;
                }
            }
        }
    }

    return 0;
}
