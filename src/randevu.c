// src/randevu.c
#include <stdio.h>
#include <string.h>
#include "../include/randevu.h"

void sistem_baslat(struct RandevuSistemi *sistem) {
    sistem->randevu_sayisi = 0;
    for (int i = 0; i < MAX_RANDEVU; i++) {
        sistem->randevular[i].aktif = 0;
    }
}
const char* DOKTORLAR[] = {
    "Dr. Ahmet Yilmaz - Kardiyoloji",
    "Dr. Ayse Kaya - Noroloji",
    "Dr. Mehmet Demir - Dahiliye",
    "Dr. Fatma Sahin - Goz Hastaliklari"
};
const int DOKTOR_SAYISI = 4;
const char* SAAT_DILIMLERI[] = {
    "09:00",
    "12:00",
    "15:00",
    "17:00"
};
const int SAAT_DILIMI_SAYISI = 4;
int randevu_olustur(struct RandevuSistemi *sistem) {
    if (sistem->randevu_sayisi >= MAX_RANDEVU) {
        printf("Randevu sistemi dolu!\n");
        return 0;
    }

    struct Randevu yeni_randevu;
    
    printf("\nMevcut Doktorlar:\n");
    for (int i = 0; i < DOKTOR_SAYISI; i++) {
        printf("%d. %s\n", i + 1, DOKTORLAR[i]);
    }

   int doktor_secim;
    printf("\nDoktor seciniz (1-%d): ", DOKTOR_SAYISI);
    scanf("%d", &doktor_secim);
    getchar(); // Buffer temizleme
    
    if (doktor_secim < 1 || doktor_secim > DOKTOR_SAYISI) {
        printf("Gecersiz doktor secimi!\n");
        return 0;
    }
    
    // Seçilen doktoru kaydet
    strcpy(yeni_randevu.doktor, DOKTORLAR[doktor_secim - 1]);

    printf("Hasta adi: ");
    fgets(yeni_randevu.hasta, MAX_ISIM, stdin);
    yeni_randevu.hasta[strcspn(yeni_randevu.hasta, "\n")] = 0;

   // Tarih girişi
    char tarih[11];
    printf("Tarih (GG.AA.YYYY): ");
    fgets(tarih, 11, stdin);
    tarih[strcspn(tarih, "\n")] = 0;

    // Saat dilimlerini göster
    printf("\nMevcut Saat Dilimleri:\n");
    for (int i = 0; i < SAAT_DILIMI_SAYISI; i++) {
        printf("%d. %s\n", i + 1, SAAT_DILIMLERI[i]);
    }

    int saat_secim;
    printf("\nSaat dilimi seciniz (1-%d): ", SAAT_DILIMI_SAYISI);
    scanf("%d", &saat_secim);
    getchar(); // Buffer temizleme

    if (saat_secim < 1 || saat_secim > SAAT_DILIMI_SAYISI) {
        printf("Gecersiz saat dilimi secimi!\n");
        return 0;
    }

    // Tarih ve saati birleştir
    sprintf(yeni_randevu.tarih, "%s %s", tarih, SAAT_DILIMLERI[saat_secim - 1]);


    // Aynı doktor ve tarihte randevu kontrolü
    for (int i = 0; i < sistem->randevu_sayisi; i++) {
        if (sistem->randevular[i].aktif && 
            strcmp(sistem->randevular[i].doktor, yeni_randevu.doktor) == 0 &&
            strcmp(sistem->randevular[i].tarih, yeni_randevu.tarih) == 0) {
            printf("Bu doktorun belirtilen tarihte baska randevusu var!\n");
            return 0;
        }
    }

    yeni_randevu.id = sistem->randevu_sayisi + 1;
    yeni_randevu.aktif = 1;
    
    sistem->randevular[sistem->randevu_sayisi] = yeni_randevu;
    sistem->randevu_sayisi++;

  
    printf("\nRandevu basariyla olusturuldu!\n");
    printf("Randevu numaraniz: %d\n", yeni_randevu.id);
    printf("Doktor: %s\n", yeni_randevu.doktor);
    printf("Tarih ve Saat: %s\n", yeni_randevu.tarih);
    return 1;
}

int randevu_iptal(struct RandevuSistemi *sistem) {
    int id;
    printf("Iptal edilecek randevu numarasi: ");
    scanf("%d", &id);
    getchar(); // Buffer temizleme

    for (int i = 0; i < sistem->randevu_sayisi; i++) {
        if (sistem->randevular[i].id == id && sistem->randevular[i].aktif) {
            sistem->randevular[i].aktif = 0;
            printf("Randevu basariyla iptal edildi.\n");
            return 1;
        }
    }

    printf("Randevu bulunamadi!\n");
    return 0;
}

void randevulari_goster(struct RandevuSistemi *sistem) {
    if (sistem->randevu_sayisi == 0) {
        printf("Hic randevu bulunmamaktadir.\n");
        return;
    }

    printf("\nMevcut Randevular:\n");
    printf("--------------------------------------------------\n");
    
    for (int i = 0; i < sistem->randevu_sayisi; i++) {
        if (sistem->randevular[i].aktif) {
            printf("Randevu No: %d\n", sistem->randevular[i].id);
            printf("Doktor: %s\n", sistem->randevular[i].doktor);
            printf("Hasta: %s\n", sistem->randevular[i].hasta);
            printf("Tarih: %s\n", sistem->randevular[i].tarih);
            printf("--------------------------------------------------\n");
        }
    }
}