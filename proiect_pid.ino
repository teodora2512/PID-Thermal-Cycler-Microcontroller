#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

/* ================== PINI ================== */
#define BUTTON_OK      6
#define BUTTON_CANCEL  7
#define BUTTON_PREV    8
#define BUTTON_NEXT    9

#define PIN_HEATER     10
#define TEMP_PIN       A0

/* ================== LCD ================== */
LiquidCrystal_I2C lcd(0x27, 16, 2);

/* ================== ENUM-URI ================== */
enum Buttons {
  EV_OK,
  EV_CANCEL,
  EV_NEXT,
  EV_PREV,
  EV_NONE,
  EV_MAX_NUM
};

enum Menus {
  MENU_MAIN = 0,
  MENU_TEMP,
  MENU_KP,
  MENU_KI,
  MENU_KD,
  MENU_TINC,
  MENU_TMEN,
  MENU_TRAC,
  MENU_START,
  MENU_RESET,
  MENU_MAX_NUM
};

enum ProcessState {
  PROC_IDLE,
  PROC_RUN
};

/* ================== VARIABILE ================== */
Menus scroll_menu = MENU_MAIN;
Menus current_menu = MENU_MAIN;

ProcessState proc_state = PROC_IDLE;

double Tset = 40.0;
double kp = 20.0, ki = 0.2, kd = 1.0;

unsigned long timp_inc = 10;
unsigned long timp_men = 5;
unsigned long timp_rac = 8;

unsigned long uptime = 0;
unsigned long last_tick = 0;

double moving_sp = 0;
double Tcur = 0;

/* PID */
double error = 0, prev_error = 0, integral = 0;
double pwm = 0;

double T_start_proces = 25.0; // Va fi actualizată la START

Menus last_menu = MENU_MAX_NUM; // Ne ajută să detectăm când se schimbă meniul

/* ===== PERTURBATII ===== */
bool perturbatii_active = false;
double perturbatie_curenta = 0;
unsigned long last_perturb = 0;

double aplicaPerturbatii(double temp) {
  if (!perturbatii_active) return temp;

  if (millis() - last_perturb > 1000) { // la 1 sec
    perturbatie_curenta = random(-20, 21) / 10.0; // -2.0 .. +2.0 C
    last_perturb = millis();
  }

  return temp + perturbatie_curenta;
}


void saveSettings() {
  int addr = 0;
  EEPROM.put(addr, Tset); addr += sizeof(double);
  EEPROM.put(addr, kp);   addr += sizeof(double);
  EEPROM.put(addr, ki);   addr += sizeof(double);
  EEPROM.put(addr, kd);   addr += sizeof(double);
  EEPROM.put(addr, timp_inc); addr += sizeof(unsigned long);
  EEPROM.put(addr, timp_men); addr += sizeof(unsigned long);
  EEPROM.put(addr, timp_rac); addr += sizeof(unsigned long);
}

void loadSettings() {
  int addr = 0;
  double testVal;
  EEPROM.get(0, testVal);
  
  // Verificăm dacă EEPROM are date valide (dacă e prima rulare, Tset va fi NaN sau 0)
  if (!isnan(testVal) && testVal > 0 && testVal < 150) {
    EEPROM.get(addr, Tset); addr += sizeof(double);
    EEPROM.get(addr, kp);   addr += sizeof(double);
    EEPROM.get(addr, ki);   addr += sizeof(double);
    EEPROM.get(addr, kd);   addr += sizeof(double);
    EEPROM.get(addr, timp_inc); addr += sizeof(unsigned long);
    EEPROM.get(addr, timp_men); addr += sizeof(unsigned long);
    EEPROM.get(addr, timp_rac); addr += sizeof(unsigned long);
  }
}
void resetToFactory() {
  Tset = 40.0;
  kp = 20.0; ki = 0.2; kd = 1.0;
  timp_inc = 10; timp_men = 5; timp_rac = 8;
  
  saveSettings(); // Salvează valorile de fabrică în EEPROM
  
  lcd.clear();
  lcd.print("RESETARE...");
  delay(1000);
  go_home();
}

/* ================== DECLARAȚII ================== */
typedef void (*state_machine_handler_t)(void);
Buttons GetButtons(void);
void state_machine(Menus menu, Buttons ev);
void print_menu(Menus menu);
void afisare_timp(void);

/* ================== BUTOANE ================== */
Buttons GetButtons(void) {
  if (digitalRead(BUTTON_OK))     return EV_OK;
  if (digitalRead(BUTTON_CANCEL)) return EV_CANCEL;
  if (digitalRead(BUTTON_NEXT))   return EV_NEXT;
  if (digitalRead(BUTTON_PREV))   return EV_PREV;
  return EV_NONE;
}

/* ================== MENIU ================== */
void enter_menu() { current_menu = scroll_menu; }
void go_home() { scroll_menu = MENU_MAIN; current_menu = MENU_MAIN; saveSettings(); }

void go_next() {
  scroll_menu = (Menus)((scroll_menu + 1) % MENU_MAX_NUM);
}

void go_prev() {
  scroll_menu = (Menus)((scroll_menu + MENU_MAX_NUM - 1) % MENU_MAX_NUM);
}

/* ---- modificări parametri ---- */
void inc_temp() { Tset += 0.5; }
void dec_temp() { Tset -= 0.5; }

void inc_kp() { kp += 1; }
void dec_kp() { kp -= 1; }

void inc_ki() { ki += 0.05; }
void dec_ki() { ki -= 0.05; }

void inc_kd() { kd += 0.2; }
void dec_kd() { kd -= 0.2; }

void inc_tinc() { timp_inc++; }
void dec_tinc() { if (timp_inc > 1) timp_inc--; }

void inc_tmen() { timp_men++; }
void dec_tmen() { if (timp_men > 1) timp_men--; }

void inc_trac() { timp_rac++; }
void dec_trac() { if (timp_rac > 1) timp_rac--; }

void start_process() {
  proc_state = PROC_RUN;
  uptime = 0;
  last_tick = millis();
  integral = 0;
  prev_error = 0;
  T_start_proces = Tcur; // Salvăm temperatura curentă ca punct de plecare
  go_home();
}

/* ================== MATRICE STATE MACHINE ================== */
state_machine_handler_t sm[MENU_MAX_NUM][EV_MAX_NUM] = {
  {enter_menu, go_home, go_next, go_prev}, // MAIN
  {go_home, go_home, inc_temp, dec_temp}, // TEMP
  {go_home, go_home, inc_kp, dec_kp},     // KP
  {go_home, go_home, inc_ki, dec_ki},     // KI
  {go_home, go_home, inc_kd, dec_kd},     // KD
  {go_home, go_home, inc_tinc, dec_tinc}, // TINC
  {go_home, go_home, inc_tmen, dec_tmen}, // TMEN
  {go_home, go_home, inc_trac, dec_trac}, // TRAC
  {start_process, go_home, 0, 0},           // START
  {resetToFactory, go_home, 0, 0}          // RESET (OK apelează resetToFactory)
};

void state_machine(Menus menu, Buttons ev) {
  if (sm[menu][ev] != 0) sm[menu][ev]();
}

/* ================== TEMPERATURA ================== */
double citesteTemperatura() {
  int v = analogRead(TEMP_PIN);
  double volt = v * 5.0 / 1023.0;
  return volt * 100.0; // LM35
}

/* ================== PID ================== */
void pid_run() {
  error = moving_sp - Tcur;
  integral += error * 0.1;

  // Anti-Windup: limităm integrala pentru a preveni overshoot-ul masiv
  integral = constrain(integral, -100, 100);

  double deriv = error - prev_error;

  pwm = kp * error + ki * integral + kd * deriv;
  pwm = constrain(pwm, 0, 255);

  analogWrite(PIN_HEATER, (int)pwm);
  prev_error = error;
}

/* ================== AFIȘARE TIMP ================== */
void afisare_timp() {
  int remaining = 0;
  lcd.setCursor(0, 1);

  if (uptime <= timp_inc) {
    lcd.print("TInc ");
    remaining = timp_inc - uptime;
    //moving_sp = Tset * (float)uptime / timp_inc;
    // Rampa urcă de la T_start la Tset
    moving_sp = T_start_proces + (Tset - T_start_proces) * (float)uptime / timp_inc;
  }
  else if (uptime <= timp_inc + timp_men) {
    lcd.print("TMen ");
    remaining = timp_inc + timp_men - uptime;
    moving_sp = Tset;
  }
  else if (uptime <= timp_inc + timp_men + timp_rac) {
    lcd.print("TRac ");
    remaining = timp_inc + timp_men + timp_rac - uptime;
    moving_sp = Tset * (float)remaining / timp_rac;
  }
  else {
    lcd.print("Gata ");
    proc_state = PROC_IDLE;
    analogWrite(PIN_HEATER, 0);
    return;
  }

  int min = remaining / 60;
  int sec = remaining % 60;
  lcd.print(min);
  lcd.print(":");
  if (sec < 10) lcd.print("0");
  lcd.print(sec);
}

/* ================== LCD ================== */
void print_menu(Menus menu) {
  // Ștergem ecranul DOAR când se schimbă meniul sau trecem din IDLE în RUN
  static ProcessState last_proc_state = PROC_IDLE;
  
  if (menu != last_menu || proc_state != last_proc_state) {
    lcd.clear();
    last_menu = menu;
    last_proc_state = proc_state;
  }


  if (proc_state == PROC_RUN) {
    // Rândul 1
    lcd.setCursor(0, 0);
    lcd.print("C:"); lcd.print(Tcur, 1);
    lcd.print(" S:"); lcd.print(moving_sp, 1);
  
    // Afișăm "P" dacă perturbațiile sunt active, lângă PWM
    lcd.setCursor(10, 1);
    if (perturbatii_active) lcd.print("P"); 
    else lcd.print(" ");

    // Rândul 2 (Procentul PWM)
    int procent_pwm = map((int)pwm, 0, 255, 0, 100);
    lcd.setCursor(12, 1);
    if(procent_pwm < 100) lcd.print(" "); // Aliniere
    if(procent_pwm < 10)  lcd.print(" ");
    lcd.print(procent_pwm);
    lcd.print("%");
    return; 
  }

  // Afișare Meniu Normal (când procesul este oprit)
  lcd.setCursor(0, 0);
  switch (menu) {
    case MENU_TEMP:  lcd.print("TSET=");  lcd.print(Tset); break;
    case MENU_KP:    lcd.print("KP=");    lcd.print(kp); break;
    case MENU_KI:    lcd.print("KI=");    lcd.print(ki); break;
    case MENU_KD:    lcd.print("KD=");    lcd.print(kd); break;
    case MENU_TINC:  lcd.print("T INC="); lcd.print(timp_inc); break;
    case MENU_TMEN:  lcd.print("T MEN="); lcd.print(timp_men); break;
    case MENU_TRAC:  lcd.print("T RAC="); lcd.print(timp_rac); break;
    case MENU_START: lcd.print("START"); break;
    case MENU_RESET: lcd.print("RESET FACTORY"); break;
    default:         lcd.print("PID SYSTEM"); break;
  }

  lcd.print("      "); // Curăță restul rândului

  if (current_menu != MENU_MAIN) {
    lcd.setCursor(0, 1);
    lcd.print("modifica");
  }
}

/* ================== SETUP ================== */
void setup() {
  Wire.begin();
  lcd.init();
  lcd.backlight();


  pinMode(BUTTON_CANCEL, INPUT);
  // Dacă ții apăsat CANCEL când pornești Arduino, se resetează
  if (digitalRead(BUTTON_CANCEL) == HIGH) {
    lcd.print("FACTORY RESET...");
    resetToFactory();
    delay(2000);
  }


  pinMode(BUTTON_OK, INPUT);
  pinMode(BUTTON_NEXT, INPUT);
  pinMode(BUTTON_PREV, INPUT);

  pinMode(PIN_HEATER, OUTPUT);

  loadSettings();

  randomSeed(analogRead(A5)); 



}


/* ================== LOOP ================== */
void loop() {
  Tcur = aplicaPerturbatii(citesteTemperatura()); // Măsurăm temperatura cu tot cu zgomot

  Buttons ev = GetButtons();
  
  // Logica specială pentru OK în timpul RUN (Perturbații)
  static bool last_ok = false;
  bool ok_now = (ev == EV_OK); 
  
  if (proc_state == PROC_RUN && ok_now && !last_ok) {
    perturbatii_active = !perturbatii_active;
    // Nu trimitem evenimentul OK mai departe către state_machine dacă suntem în RUN
    // pentru a nu intra accidental în sub-meniuri
    ev = EV_NONE; 
  }
  last_ok = ok_now;

  if (ev != EV_NONE) state_machine(current_menu, ev);

  print_menu(scroll_menu);

  if (proc_state == PROC_RUN) {
    if (millis() - last_tick >= 1000) {
      uptime++;
      last_tick = millis();
    }
    pid_run();
    afisare_timp();
  }
  delay(150);
}
