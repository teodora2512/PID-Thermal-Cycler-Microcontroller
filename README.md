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
| **Buton OK** | D6 | Selectare / Activare Perturbații |
| **Buton CANCEL** | D7 | Ieșire / Hard Reset la pornire |
| **Buton PREV** | D8 | Navigare înapoi / Scădere valoare |
| **Buton NEXT** | D9 | Navigare înainte / Creștere valoare |

## 🧱 Arhitectura Hardware și Selecția Componentelor

Sistemul a fost proiectat utilizând componente de grad industrial pentru a asigura stabilitatea buclei de control și fiabilitatea pe termen lung a actuatorilor.

### 🔌 Subsistemul de Putere și Execuție
* **Tranzistor MOSFET N-Channel MagnaChip MDP10N055:** Elementul central de comutație, ales pentru specificațiile sale superioare: tensiune de operare de până la **100V** și o capacitate de curent impresionantă de **130A**. Cu o putere disipată de **188W** în capsulă **TO-220**, acesta permite controlul PWM de înaltă frecvență fără pierderi termice semnificative, asigurând o modulare fină a energiei către elementul de încălzire.
* **Sursă de tensiune industrială Schneider Electric (12V):** Utilizată pentru alimentarea ramurii de putere, această sursă garantează un curent stabil, eliminând fluctuațiile de tensiune care ar putea introduce perturbații în calculul componentei derivative a algoritmului PID.
* **Conectori Terminali cu Șurub (High-Current):** Pentru a minimiza rezistența de contact și a preveni degradarea termică a conexiunilor la curenți mari, s-au utilizat terminale cu șurub dedicate, asigurând un transfer de energie sigur și eficient.

### 🎯 Subsistemul de Monitorizare și Interfațare
* **Senzor de Temperatură de Precizie LM35 (OKY3066-2):** Un traductor analogic cu calibrare liniară de **10 mV/°C**. Alegerea acestui senzor permite o rezoluție ridicată în procesul de achiziție de date, facilitând o monitorizare constantă în plaja 4-30V, esențială pentru stabilitatea referinței mobile în etapele de rampă.
* **Interfață Vizuală LCD 1602 Character Display (Deep Blue):** Ecranul monocrom cu fundal albastru și 16x2 caractere oferă un contrast ridicat pentru monitorizarea parametrilor critici. Prin integrarea protocolului de comunicare **I2C**, am optimizat resursele microcontrolerului, utilizând doar magistrala serială de date pentru o gestiune eficientă a afișajului.
* **Unitate de Procesare Arduino Uno R3 (SMD Edition):** Platforma de calcul bazată pe procesorul ATmega328P, responsabilă pentru eșantionarea datelor de la senzor la intervale de milisecunde și execuția logicii de control în timp real.

---

## 💻 Arhitectură Software: Matricea de Pointeri la Funcții

O particularitate avansată a acestui proiect este implementarea interfeței cu utilizatorul (UI) printr-o **Matrice de Pointeri la Funcții**, o metodă superioară din punct de vedere algoritmic față de structurile clasice de control.



### Concepte Avansate Implementate:
1.  **Gestiunea Evenimentelor prin State Machine:**
    Am definit o matrice bidimensională de tip `state_machine_handler_t`, unde indexarea se face pe baza stării curente a meniului și a evenimentului declanșat de utilizator (buton apăsat).
    
    ```cpp
    // Apelul dinamic al funcției specifice stării
    if (sm[menu][ev] != 0) sm[menu][ev]();
    ```

2.  **Modularitate și Decuplare:**
    Această arhitectură permite separarea completă a logicii de navigare de logica de execuție a procesului termic. Adăugarea unei noi funcționalități în meniu nu necesită modificarea algoritmului principal, ci doar extinderea matricei de pointeri.

3.  **Optimizarea Memoriei și a Vitezei:**
    Spre deosebire de un lanț lung de instrucțiuni `if-else` care ar fi evaluat secvențial, accesarea unei funcții prin pointer se realizează în **timp constant O(1)**, asigurând o latență minimă între interacțiunea utilizatorului și răspunsul sistemului.

---

## 🛡️ Siguranță și Stabilitate
* **Filtrare Digitală:** Citirile de la LM35 trec printr-un proces de mediere pentru a elimina zgomotul alb de pe linia analogică.
* **Anti-Windup Integral:** Am implementat limitarea matematică a termenului integral pentru a preveni fenomenul de saturație, asigurând o revenire rapidă a controlului în cazul unor perturbații externe majore.

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