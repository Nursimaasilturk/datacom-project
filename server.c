#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_PATIENTS 10
#define APPOINTMENT_COUNT 5

int patients[MAX_PATIENTS];
int patient_count = 0;
pthread_mutex_t patient_lock;

int appointments[APPOINTMENT_COUNT];

// Function to log each command to a file
void log_command(int patient_id, const char* command) {
    FILE* file = fopen("commands.log", "a");
    if (file == NULL) {
        perror("File open failed");
        return;
    }
    fprintf(file, "Patient %d command: %s\n", patient_id, command);
    fclose(file);
}

// Save appointment statuses to a file
void save_appointment_status() {
    FILE* file = fopen("appointments.txt", "w");
    if (file == NULL) {
        perror("File open failed");
        return;
    }
    for (int i = 0; i < APPOINTMENT_COUNT; i++) {
        fprintf(file, "Appointment %d: %s\n", i + 1, appointments[i] == 0 ? "Available" : "Booked");
    }
    fclose(file);
}

// Initialize appointments
void initialize_appointments() {
    for (int i = 0; i < APPOINTMENT_COUNT; i++) {
        appointments[i] = 0;
    }
    save_appointment_status();
}

void send_message(int patient_socket, const char* message) {
    send(patient_socket, message, strlen(message), 0);
}

void view_appointments(int patient_socket) {
    char message[256] = "Available Appointments: ";
    char appointment_info[16];
    for (int i = 0; i < APPOINTMENT_COUNT; i++) {
        if (appointments[i] == 0) {
            snprintf(appointment_info, sizeof(appointment_info), "[ %d ]", i + 1);
            strcat(message, appointment_info);
        }
    }
    strcat(message, "\n");
    send_message(patient_socket, message);
}

void book_appointment(int patient_socket, int patient_id) {
    for (int i = 0; i < APPOINTMENT_COUNT; i++) {
        if (appointments[i] == 0) {
            appointments[i] = patient_id; // Store patient ID instead of 1
            printf("Appointment %d booked by Patient %d\n", i + 1, patient_id);
            char message[64];
            snprintf(message, sizeof(message), "Appointment %d booked for you.\n", i + 1);
            send_message(patient_socket, message);
            save_appointment_status(); // Save appointment statuses
            return;
        }
    }
    send_message(patient_socket, "All appointments are booked.\n");
}

void cancel_appointment(int patient_id) {
    for (int i = 0; i < APPOINTMENT_COUNT; i++) {
        if (appointments[i] == patient_id) {
            appointments[i] = 0;
            printf("Appointment %d canceled by Patient %d\n", i + 1, patient_id);
            save_appointment_status(); // Save appointment statuses
            return;
        }
    }
}

void* handle_patient(void* patient_socket_ptr) {
    int patient_socket = *(int*)patient_socket_ptr;
    free(patient_socket_ptr);

    char buffer[1024];
    int read_size;
    int patient_id = patient_socket;

    printf("New patient connected: Patient %d\n", patient_id);

    while ((read_size = recv(patient_socket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[read_size] = '\0';
        buffer[strcspn(buffer, "\r\n")] = '\0'; // Remove newline characters
        printf("Patient %d command: %s\n", patient_id, buffer);

        // Log the received command
        log_command(patient_id, buffer);

        if (strcmp(buffer, "exit") == 0) {
            printf("Patient %d disconnected.\n", patient_id);
            cancel_appointment(patient_id);
            break;
        } else if (strcmp(buffer, "view") == 0) {
            view_appointments(patient_socket);
        } else if (strcmp(buffer, "book") == 0) {
            book_appointment(patient_socket, patient_id);
        } else {
            send_message(patient_socket, "Invalid command. Available commands: exit, view, book\n");
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
    int server_socket, new_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len;

    pthread_mutex_init(&patient_lock, NULL);
    initialize_appointments();

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        return 1;
    }

    if (listen(server_socket, 3) < 0) {
        perror("Listen failed");
        return 1;
    }

    printf("Doctor Appointment Server Started. Port: %d\n", PORT);

    for (int i = 0; i < MAX_PATIENTS; i++) {
        patients[i] = -1;
    }

    while (1) {
        addr_len = sizeof(client_addr);
        int* new_patient_socket = malloc(sizeof(int));

        if ((*new_patient_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len)) < 0) {
            perror("Accept failed");
            free(new_patient_socket);
            continue;
        }

        pthread_mutex_lock(&patient_lock);
        if (patient_count >= MAX_PATIENTS) {
            send_message(*new_patient_socket, "Server is at full capacity.\n");
            close(*new_patient_socket);
            free(new_patient_socket);
        } else {
            for (int i = 0; i < MAX_PATIENTS; i++) {
                if (patients[i] == -1) {
                    patients[i] = *new_patient_socket;
                    patient_count++;
                    break;
                }
            }
            pthread_t patient_thread;
            pthread_create(&patient_thread, NULL, handle_patient, new_patient_socket);
            pthread_detach(patient_thread);
        }
        pthread_mutex_unlock(&patient_lock);
    }

    pthread_mutex_destroy(&patient_lock);
    close(server_socket);
    return 0;
}