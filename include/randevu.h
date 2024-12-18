// include/randevu.h
#ifndef RANDEVU_H
#define RANDEVU_H

#define MAX_RANDEVU 20
#define MAX_ISIM 50

struct Randevu {
    int id;
    char doktor[MAX_ISIM];
    char hasta[MAX_ISIM];
    char tarih[20];
    int aktif;
};

struct RandevuSistemi {
    struct Randevu randevular[MAX_RANDEVU];
    int randevu_sayisi;
};

void sistem_baslat(struct RandevuSistemi *sistem);
int randevu_olustur(struct RandevuSistemi *sistem);
int randevu_iptal(struct RandevuSistemi *sistem);
void randevulari_goster(struct RandevuSistemi *sistem);

#endif