#include <stdio.h>
#include "../include/menu.h"

void menu_goster(void) {
    printf("\n=== Doktor Randevu Sistemi ===\n");
    printf("1. Randevu Olustur\n");
    printf("2. Randevu Iptal Et\n");
    printf("3. Randevulari Goster\n");
    printf("4. Cikis\n");
}

int menu_secim_al(void) {
    int secim;
    printf("Seciminiz (1-4): ");
    scanf("%d", &secim);
    getchar(); // Buffer temizleme
    return secim;
}