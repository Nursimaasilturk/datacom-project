#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_PATIENTS 10
#define BUFFER_SIZE 1024

int appointments[20] = {0}; // Örnek randevu durumu
int patients[MAX_PATIENTS] = {-1};
int patient_count = 0;
pthread_mutex_t patient_lock = PTHREAD_MUTEX_INITIALIZER;

void send_message(int socket, const char* message) {
    send(socket, message, strlen(message), 0);
}

void view_appointments(int patient_socket) {
    char message[BUFFER_SIZE] = "Available Appointments:\n";
    char appointment_info[32];
    for (int i = 0; i < 4; i++) {
        snprintf(appointment_info, sizeof(appointment_info), "Doctor %d:\n", i + 1);
        strcat(message, appointment_info);
        for (int j = 0; j < 5; j++) {
            int index = i * 5 + j;
            if (appointments[index] == 0) {
                snprintf(appointment_info, sizeof(appointment_info), "  [ %d ]\n", j + 1);
                strcat(message, appointment_info);
            }
        }
    }
    send_message(patient_socket, message);
}

void book_appointment(int patient_socket, int patient_id) {
    char message[64];
    int doctor_id, appointment_id;
    send_message(patient_socket, "Enter doctor number (1-4): ");
    recv(patient_socket, message, sizeof(message), 0);
    doctor_id = atoi(message) - 1;

    if (doctor_id < 0 || doctor_id >= 4) {
        send_message(patient_socket, "Invalid doctor number.\n");
        return;
    }

    send_message(patient_socket, "Enter appointment number (1-5): ");
    recv(patient_socket, message, sizeof(message), 0);
    appointment_id = atoi(message) - 1;

    if (appointment_id < 0 || appointment_id >= 5) {
        send_message(patient_socket, "Invalid appointment number.\n");
        return;
    }

    int index = doctor_id * 5 + appointment_id;
    if (appointments[index] == 0) {
        appointments[index] = patient_id;
        snprintf(message, sizeof(message), "Appointment %d with Doctor %d booked for you.\n", appointment_id + 1, doctor_id + 1);
        send_message(patient_socket, message);
    } else {
        send_message(patient_socket, "This appointment is already booked.\n");
    }
}

void cancel_appointment(int patient_id) {
    for (int i = 0; i < 20; i++) {
        if (appointments[i] == patient_id) {
            appointments[i] = 0;
        }
    }
}

void* handle_patient(void* patient_socket_ptr) {
    int patient_socket = *(int*)patient_socket_ptr;
    free(patient_socket_ptr);

    char buffer[BUFFER_SIZE];
    int read_size;
    int patient_id = patient_socket;

    printf("New patient connected: Patient %d\n", patient_id);

    // Send welcome message and available commands once
    send_message(patient_socket, "Welcome to the Doctor Appointment System!\n");
    send_message(patient_socket, "Please choose an option:\n");
    send_message(patient_socket, "1. View Appointments\n");
    send_message(patient_socket, "2. Book Appointment\n");
    send_message(patient_socket, "3. Cancel Appointment\n");
    send_message(patient_socket, "4. Exit\n");

    while ((read_size = recv(patient_socket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[read_size] = '\0';
        buffer[strcspn(buffer, "\r\n")] = '\0'; // Remove newline characters
        printf("Patient %d command: %s\n", patient_id, buffer);

        if (strcmp(buffer, "4") == 0) { // Exit
            printf("Patient %d disconnected.\n", patient_id);
            cancel_appointment(patient_id);
            break;
        } else if (strcmp(buffer, "1") == 0) { // View Appointments
            view_appointments(patient_socket);
        } else if (strcmp(buffer, "2") == 0) { // Book Appointment
            book_appointment(patient_socket, patient_id);
        } else if (strcmp(buffer, "3") == 0) { // Cancel Appointment
            cancel_appointment(patient_id);
            send_message(patient_socket, "Your appointment has been canceled.\n");
        } else {
            send_message(patient_socket, "Invalid command. Please enter a number between 1 and 4.\n");
        }
    }

    pthread_mutex_lock(&patient_lock);
    for (int i = 0; i < MAX_PATIENTS; i++) {
        if (patients[i] == patient_socket) {
            patients[i] = -1;
            patient_count--;
            break;
        }
    }
    pthread_mutex_unlock(&patient_lock);

    close(patient_socket);
    return NULL;
}

int main() {
    int server_socket, patient_socket, *new_sock;
    struct sockaddr_in server_addr, patient_addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);

    // Sunucu soketi oluştur
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Could not create socket");
        return 1;
    }

    // Sunucu adresini ayarla
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    // Sunucu soketini bağla
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        return 1;
    }

    // Dinlemeye başla
    listen(server_socket, 3);
    printf("Waiting for incoming connections...\n");

    // Bağlantıları kabul et
    while ((patient_socket = accept(server_socket, (struct sockaddr*)&patient_addr, &addr_size))) {
        printf("Connection accepted\n");

        pthread_t patient_thread;
        new_sock = malloc(1);
        *new_sock = patient_socket;

        if (pthread_create(&patient_thread, NULL, handle_patient, (void*)new_sock) < 0) {
            perror("Could not create thread");
            return 1;
        }

        printf("Handler assigned\n");
    }

    if (patient_socket < 0) {
        perror("Accept failed");
        return 1;
    }

    return 0;
}