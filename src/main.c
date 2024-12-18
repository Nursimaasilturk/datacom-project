// src/main.c
#include <stdio.h>
#include "../include/randevu.h"
#include "../include/menu.h"

int main() {
    struct RandevuSistemi sistem;
    sistem_baslat(&sistem);
    int secim;

    while (1) {
        menu_goster();
        secim = menu_secim_al();

        switch (secim) {
            case 1:
                randevu_olustur(&sistem);
                break;
            case 2:
                randevu_iptal(&sistem);
                break;
            case 3:
                randevulari_goster(&sistem);
                break;
            case 4:
                printf("Programdan cikiliyor...\n");
                return 0;
            default:
                printf("Gecersiz secim!\n");
        }
    }

    return 0;
}