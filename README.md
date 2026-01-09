# 🌡️ Sistem Industrial de Control Termic (PID & Ramp-Soak)

Acest proiect reprezintă un sistem avansat de control al temperaturii, implementat pe platforma Arduino. Utilizează un algoritm **PID (Proporțional-Integrator-Derivativ)** pentru a gestiona un profil termic complex format din rampe de încălzire, perioade de menținere și răcire controlată.

---

## 🚀 Caracteristici Principale

* **Algoritm PID de Înaltă Precizie:** Control fin al puterii prin PWM pentru evitarea oscilațiilor.
* **Profil Ramp-Soak:** Trei etape de proces (Încălzire, Menținere, Răcire) complet configurabile.
* **Gestiune Memorie (EEPROM):** Salvarea automată a parametrilor (Tset, Kp, Ki, Kd, Timpi) pentru a fi păstrați după oprire.
* **Simulator de Perturbații:** Modul de testare a robusteții prin injectarea de zgomot aleatoriu (TRNG) în bucla de feedback.
* **Interfață Utilizator (FSM):** Navigare stabilă prin meniuri folosind 4 butoane și afișaj LCD 16x2 I2C (fără flicker).
* **Protecție Anti-Windup:** Limitează acumularea integralei pentru a preveni supraîncălzirea în caz de erori hardware.

---

## 🛠️ Configurație Hardware

| Componentă | Pin Arduino | Detalii |
| :--- | :--- | :--- |
| **Senzor LM35** | A0 | Senzor analogic (10mV/°C) |
| **Heater (Bec/Rezistență)** | D10 | Comandă PWM prin tranzistor MOSFET |
| **LCD 16x2 I2C** | SDA/SCL | Adresa I2C: 0x27 |
| **Buton OK** | D6 | Selectare / Activare Perturbații |
| **Buton CANCEL** | D7 | Ieșire / Hard Reset la pornire |
| **Buton PREV** | D8 | Navigare înapoi / Scădere valoare |
| **Buton NEXT** | D9 | Navigare înainte / Creștere valoare |

---

## 📈 Profilul Termic de Funcționare

Procesul este împărțit în trei faze distincte pentru a asigura integritatea piesei încălzite:

1.  **TInc (Încălzire):** Temperatura urcă liniar de la temperatura camerei la temperatura țintă.
2.  **TMen (Menținere):** Stabilizarea și menținerea temperaturii la punctul setat pentru o durată fixă.
3.  **TRac (Răcire):** Scăderea controlată a temperaturii pentru prevenirea șocurilor termice.

---

## 💻 Instalare și Utilizare

1.  **Biblioteci necesare:** Instalați `LiquidCrystal_I2C` din Library Manager.
2.  **Conectare:** Realizați montajul conform tabelului de pini de mai sus.
3.  **Configurare:** * Folosiți butoanele pentru a regla $K_p, K_i, K_d$ și timpii de proces.
    * Setările se salvează automat în EEPROM la ieșirea din sub-meniul de editare.
4.  **Lansare:** Navigați la `START` și apăsați `OK`.
5.  **Perturbații:** Apăsați `OK` în timpul rulării pentru a testa cum reacționează PID-ul la zgomot (indicatorul `P` va apărea pe ecran).
6.  **Resetare:** Dacă doriți revenirea la setările din fabrică, folosiți meniul `RESET FACTORY` sau țineți apăsat `CANCEL` la pornirea aparatului.

---

## Galerie Media

### Montaj Hardware
![Poza Montaj](imagini/montaj_hardware.jpg)

### Funcționare în Timp Real


### Demonstrație Video
[Faceți clic aici pentru a viziona clipul video cu funcționarea sistemului](video/demonstratie_video.mp4)

---

## 👨‍💻 Autor
**Teodora Otelariu**