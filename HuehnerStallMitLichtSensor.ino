// Pins für Motor
int pinMotorOut1 = 12;
int pinMotorOut2 = A0;
int pinMotorSensor = A3;

int ledManuellerModeAktiv = 13;
int knopfMnuellerMode = 7;
int knopfTuerAufZuInManuellenMode = 4;
int pinFotoSensor = A5;

bool mnuellerModeAktiv = false;
bool tuerIstOffen = false; // Tür ist geschlossen

int foroSensorWert = 0; //Variable für den foroSensorWert mit 0 als Startwert
int esIstDunkelSeitMessungen = 0;
int esIstHellSeitMessungen = 0;

const int ANZHL_RELEVANTE_MESSUNGEN = 10;
const int WERT_FUER_DUNKEL = 290;
const int WERT_FUER_HELL_GENUG = 510;
const int VERZOEGERUNG_BEIM_MOTOR_SENSOR_MS = 2000; //Konzevik
const int ZEIT_ZUR_AUTOMATISCHEN_MOTOR_AUSSCHALTUNG_SEK = 60;


void setup()
{
  Serial.begin(9600); //Die Kommunikation mit dem seriellen Port wird gestartet. Das benötigt man, um sich den tatsächlich ausgelesenen Wert später im serial monitor anzeigen zu lassen.

  pinMode (ledManuellerModeAktiv, OUTPUT); //Der Pin mit der LED (Pin 13) ist jetzt ein Ausgang
  digitalWrite(ledManuellerModeAktiv, LOW);

  pinMode(knopfMnuellerMode, INPUT);
  pinMode(knopfTuerAufZuInManuellenMode, INPUT);

  pinMode(pinMotorOut1, OUTPUT); // Мотор 1-1
  pinMode(pinMotorOut2, OUTPUT); // Мотор 1-2

  schalteDenMotorAus();
}

void loop()
{
  pruefeBetriebsmodus();

  if (mnuellerModeAktiv == false) {
    fuehreLichtMessungDurch();
  }
  else {
    pruefeManuelleEingabe();
  }
  delay (500);//warte halbe Sekunde bis nächstes Mal sensor ausgelesen wird
}


void pruefeBetriebsmodus() {
  if (digitalRead(knopfTuerAufZuInManuellenMode) == HIGH) {
    mnuellerModeAktiv = !mnuellerModeAktiv;
    aktualisiereBetriebsModus();
    delay (1000);
  }
}

void pruefeManuelleEingabe() {
  if (digitalRead(knopfMnuellerMode) == HIGH) {
    if (tuerIstOffen == true) // ist die Tür offen?
    {
      macheDieTuerZu();
    }
    else
    { // Die Tür ist nicht offen -> also ist die Tür wohl zu
      macheDieTuerAuf();
    }
  }
}

void aktualisiereBetriebsModus() {
  if (mnuellerModeAktiv == true) {
    digitalWrite(ledManuellerModeAktiv, HIGH);
    Serial.println("Manueller Mode aktiviert!");
  } else {
    digitalWrite(ledManuellerModeAktiv, LOW);
    Serial.println("Manueller Mode deaktiviert!");
  }
  setzeDieMessungswerteZuruek();
}

void fuehreLichtMessungDurch() {
  pruefeDieAuktuelleHelligkeit();
  gebeDieDatenAus();
  if (tuerIstOffen == true) // ist die Tür offen?
  {
    if (istEsDunkel()) // Und es wird dunkel?
    {
      // Dann mach die verdamte Tür zu, es zieht!
      macheDieTuerZu();
    }
  }
  else
  { // Die Tür ist nicht offen -> also ist die Tür wohl zu
    if (istEsHellGenug()) // Ist es hell genug?
    {
      //Ja! Dann mach die Tür auf
      macheDieTuerAuf();
    }
  }
  delay (500);
}

void gebeDieDatenAus() {
  Serial.print("Ist Tür auf? = " );
  Serial.println(tuerIstOffen);
  Serial.print("foroSensorWert = " ); //Ausgabe am Serial-Monitor: Das Wort „foroSensorWert: „
  Serial.print(foroSensorWert); //Ausgabe am Serial-Monitor. Mit dem Befehl Serial.print wird der foroSensorWert des Fotowiderstandes in Form einer Zahl zwischen 0 und 1023 an den serial monitor gesendet.
  Serial.print("; Dunkel seit = " );
  Serial.print(esIstDunkelSeitMessungen);
  Serial.print("; Hell seit = " );
  Serial.println(esIstHellSeitMessungen);
}


void macheDieTuerAuf() {
  Serial.println("Die Tür wird geöffnet...");
  digitalWrite(pinMotorOut1, HIGH);
  digitalWrite(pinMotorOut2, LOW);

  warteAufMotorSensor();
  schalteDenMotorAus();

  tuerIstOffen = true;
  Serial.println("Die Tür ist geöffnet!");
}

void macheDieTuerZu() {
  Serial.println("Die Tür wird geschlossen...");
  digitalWrite(pinMotorOut1, LOW);
  digitalWrite(pinMotorOut2, HIGH);

  warteAufMotorSensor();
  schalteDenMotorAus();
  tuerIstOffen = false;
  Serial.println("Die Tür ist geschlossen!");
}

void warteAufMotorSensor() {
  delay(VERZOEGERUNG_BEIM_MOTOR_SENSOR_MS);

  for (int i = 0; i < ZEIT_ZUR_AUTOMATISCHEN_MOTOR_AUSSCHALTUNG_SEK * 10; i++) {
    if (digitalRead(pinMotorSensor) == HIGH) {
      Serial.println("Sensor erreicht");
      break;
    }
    delay(100);
  }
}

void schalteDenMotorAus() {
  digitalWrite(pinMotorOut1, HIGH);
  digitalWrite(pinMotorOut2, HIGH);
}


void pruefeDieAuktuelleHelligkeit() {
  foroSensorWert = analogRead(pinFotoSensor); //Die Spannung an dem Fotowiderstand auslesen und unter der Variable „foroSensorWert“ abspeichern.
  if (foroSensorWert <= WERT_FUER_DUNKEL )
  {
    esIstDunkelSeitMessungen++; //Zähle eins dazu
    esIstHellSeitMessungen = 0;
  } else if (foroSensorWert >= WERT_FUER_HELL_GENUG )
  {
    esIstHellSeitMessungen++; //Zähle eins dazu
    esIstDunkelSeitMessungen = 0;
  }
  else {
    setzeDieMessungswerteZuruek();
  }
}

void setzeDieMessungswerteZuruek() {
  esIstHellSeitMessungen = 0;
  esIstDunkelSeitMessungen = 0;
}

bool istEsDunkel() {
  return esIstDunkelSeitMessungen >= ANZHL_RELEVANTE_MESSUNGEN;
}


bool istEsHellGenug() {
  return esIstHellSeitMessungen >= ANZHL_RELEVANTE_MESSUNGEN;
}





