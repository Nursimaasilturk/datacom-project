#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_DOCTORS 10  // Doktor sayısı için maksimum limit
#define MAX_APPOINTMENTS 100  // Randevu limiti

int doctors[MAX_DOCTORS];  // Doktorların durumlarını tutan dizi
int appointments[MAX_APPOINTMENTS];  // Randevuları tutan dizi

void save_doctor_status() {
    FILE* file = fopen("doctors.txt", "w");
    if (file == NULL) {
        perror("File open failed");
        return;
    }
    for (int i = 0; i < MAX_DOCTORS; i++) {
        fprintf(file, "Doctor %d: %s\n", i + 1, doctors[i] == 0 ? "Available" : "Occupied");
    }
    fclose(file);
}

void save_appointment_status() {
    FILE* file = fopen("appointments.txt", "w");
    if (file == NULL) {
        perror("File open failed");
        return;
    }
    for (int i = 0; i < MAX_APPOINTMENTS; i++) {
        fprintf(file, "Appointment %d: %s\n", i + 1, appointments[i] == 0 ? "Available" : "Booked");
    }
    fclose(file);
}

void handle_appointment(int client_socket, int client_id) {
    for (int i = 0; i < MAX_DOCTORS; i++) {
        if (doctors[i] == 0) {
            doctors[i] = client_id;  // Doktor ataması
            printf("Doctor %d assigned to Client %d\n", i + 1, client_id);
            char message[64];
            snprintf(message, sizeof(message), "Doctor %d assigned to your appointment.\n", i + 1);
            send_message(client_socket, message);
            save_doctor_status();
            return;
        }
    }
    send_message(client_socket, "All doctors are booked.\n");
}

void handle_randevu_release(int client_id) {
    for (int i = 0; i < MAX_DOCTORS; i++) {
        if (doctors[i] == client_id) {
            doctors[i] = 0;
            printf("Doctor %d freed by Client %d\n", i + 1, client_id);
            save_doctor_status();
            return;
        }
    }
}

void *handle_client(void *client_socket_ptr) {
    int client_socket = *(int *)client_socket_ptr;
    free(client_socket_ptr);

    char buffer[1024];
    int read_size;
    int client_id = client_socket;

    while ((read_size = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[read_size] = '\0';
        buffer[strcspn(buffer, "\r\n")] = '\0'; 

        printf("Client %d command: %s\n", client_id, buffer);

        if (strcmp(buffer, "exit") == 0) {
            handle_randevu_release(client_id);
            printf("Client %d disconnected.\n", client_id);
            break;
        } else if (strcmp(buffer, "show") == 0) {
            save_doctor_status();  // Mevcut doktor durumlarını görüntüle
        } else if (strcmp(buffer, "book") == 0) {
            handle_appointment(client_socket, client_id);
        } else {
            send_message(client_socket, "Invalid command. Available commands: exit, show, book\n");
        }
    }

    close(client_socket);
    return NULL;
}
