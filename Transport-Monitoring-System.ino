#include "arduino_secrets.h"

#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "TinyGPS++.h"

#define BTN_CAMBIO 6
#define BTN_POSICION 5
#define BTN_GUARDAR 8
#define BTN_ENVIAR 7

#define TESTIGO_1 2
#define TESTIGO_2 3
#define TESTIGO_3 4




SoftwareSerial gpsSerial(11, 12); // RX, TX
SoftwareSerial sim900(9, 10); // RX, TX
TinyGPSPlus gps;
LiquidCrystal_I2C lcd(0x27, 16, 2);

char numero[11] = "5551785271"; // NÃºmero inicial
int posicion = 0;
bool editando = false;
String estadoTransporte = "Normal"; 



void setup() {
  pinMode(BTN_CAMBIO, INPUT_PULLUP);
  pinMode(BTN_POSICION, INPUT_PULLUP);
  pinMode(BTN_GUARDAR, INPUT_PULLUP);
  pinMode(BTN_ENVIAR, INPUT_PULLUP);
  
  pinMode(TESTIGO_1, INPUT_PULLUP);  // LSB
  pinMode(TESTIGO_2, INPUT_PULLUP);
  pinMode(TESTIGO_3, INPUT_PULLUP);  // MSB
  
  gpsSerial.begin(9600);
  sim900.begin(19200);
  Serial.begin(19200);
  lcd.init();
  lcd.backlight();
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Esperando IA");
  delay(10000);
    
  while(1){
    int b0 = digitalRead(2);  // bit 0 (LSB)
    int b1 = digitalRead(3);  // bit 1
    int b2 = digitalRead(4);  // bit 2 (MSB)
    int code = (b2 << 2) | (b1 << 1) | b0;
    if (code == 0)
      break;  
  }


  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IA Ok!");
  delay(3000);

    // Leer nÃºmero almacenado en EEPROM
  for (int i = 0; i < 10; i++) {
    numero[i] = EEPROM.read(i);
    if (numero[i] < '0' || numero[i] > '9') numero[i] = '0';
  }
  numero[10] = '\0';

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Esperando GPS...");
  
  while (!gpsSerial.available()) {
    delay(50);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("GPS Listo!");
  delay(10000);
 
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Esperando RED");
  delay(10000);

  esperarRed();  

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("RED encontrada");
  delay(3000);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TransLowNet");
  lcd.setCursor(0, 1);
  lcd.print("inicializado");
  delay(3000); // Muestra el mensaje inicial por 3 segundos

  mostrarMensajeInicial();
}

void loop() {
    statusTrasporte ();
    botonesMenu ();
    actualizarGPS();
    ///realizarLlamadaYEnviarEstado();
}

void statusTrasporte (){

  int b0 = digitalRead(2);  // bit 0 (LSB)
    int b1 = digitalRead(3);  // bit 1
    int b2 = digitalRead(4);  // bit 2 (MSB)
    int code = (b2 << 2) | (b1 << 1) | b0;
    String mensajeAlerta = "";  // Variable para guardar el texto

    switch (code) {
      case 0: mensajeAlerta = "Normal"; break;
      case 1: mensajeAlerta = "Abuso"; break;
      case 2: mensajeAlerta = "Agresion"; break;
      case 3: mensajeAlerta = "Asalto"; break;
      case 4: mensajeAlerta = "Peleas"; break;
      case 5: mensajeAlerta = "Disparos"; break;
      case 6: mensajeAlerta = "Vandalismo"; break;
      case 7: mensajeAlerta = "No identificado"; break;
      default: mensajeAlerta = "Error"; break;
    }

    
    /*if (code != 0)
      enviarMensajeAlertaJetson(mensajeAlerta);*/
  
}

void botonesMenu (){
  if (editando) {
        if (digitalRead(BTN_POSICION) == LOW) {
            delay(300);
            posicion = (posicion + 1) % 10;
            mostrarNumeroEditable();
        }

        if (digitalRead(BTN_CAMBIO) == LOW) {
            delay(300);
            numero[posicion] = ((numero[posicion] - '0' + 1) % 10) + '0';
            mostrarNumeroEditable();
        }

        if (digitalRead(BTN_GUARDAR) == LOW) {
            delay(300);
            for (int i = 0; i < 10; i++) {
                EEPROM.write(i, numero[i]);
            }
            editando = false;
            mostrarMensajeDos();
        }
    } else {
        if (digitalRead(BTN_GUARDAR) == LOW) {
            delay(300);
            editando = true;
            mostrarNumeroEditable();
        }

        if (digitalRead(BTN_ENVIAR) == LOW) {
            delay(300);
            enviarMensajeAlerta();
            mostrarMensajeTres();
        }
    }
  }

void mostrarMensajeInicial() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Numero guardado:");
    lcd.setCursor(0, 1);
    lcd.print(numero);
    delay(2000);


    enviarPrimerMensaje();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Mensaje enviado,");
    lcd.setCursor(0, 1);
    lcd.print("estado Normal");
    delay(5000);

    

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Monitoreando");
    lcd.setCursor(0, 1);
    lcd.print("transporte.");
}

void mostrarNumeroEditable() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Editando Numero:");
    lcd.setCursor(0, 1);
    lcd.print(numero);
    lcd.setCursor(posicion, 1);
    lcd.cursor();
}

void enviarMensajeAlerta() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enviando alerta.");
    
    sim900.print("AT+CMGF=1\r");
    delay(500);
    sim900.print("AT+CMGS=\"");
    sim900.print(numero);
    sim900.println("\"");
    delay(500);

    sim900.println("Alerta de incidente, favor de comunicarse");
    sim900.println("con el chofer");
    sim900.println((char)26); // Ctrl+Z para enviar el mensaje
    delay(5000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Alerta enviada a");
    lcd.setCursor(0, 1);
    lcd.print(numero);


}

void mostrarMensajeDos(){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Numero guardado:");
    lcd.setCursor(0, 1);
    lcd.print(numero);
    delay(2000);
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Monitoreando");
    lcd.setCursor(0, 1);
    lcd.print("transporte.");
  }


void mostrarMensajeTres(){    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Monitoreando");
    lcd.setCursor(0, 1);
    lcd.print("transporte.");
  }


void enviarPrimerMensaje() {
    sim900.print("AT+CMGF=1\r");
    delay(500);
    sim900.print("AT+CMGS=\"");
    sim900.print(numero);
    sim900.println("\"");
    delay(500);

    sim900.println("Trasporte 1 ha empezado su recorrido.");
    sim900.println("Estado: Normal");
    sim900.println((char)26); // Ctrl+Z para enviar el mensaje
    delay(5000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Alerta enviada a");
    lcd.setCursor(0, 1);
    lcd.print(numero);
}


void realizarLlamadaYEnviarEstado() {
    if (sim900.available()) {
        String respuesta = sim900.readString(); // Leer la respuesta del SIM900
        int indexCLIP = respuesta.indexOf("+CLIP:"); // +CLIP es el identificador de llamada entrante
        if (indexCLIP != -1) {
            sim900.println("ATH");
            enviarSMSConEstado();
        }
    }  
  
}

void enviarSMSConEstado() {
    
    sim900.print("AT+CMGF=1\r"); // Configura el modo de texto para SMS
    delay(500);
    sim900.print("AT+CMGS=\"");
    sim900.print(numero);
    sim900.println("\"");
    delay(500);

    // Contenido del mensaje
    sim900.print("Estado del transporte: ");
    sim900.println(estadoTransporte);
    sim900.println((char)26); // Ctrl+Z para enviar el mensaje
    delay(5000);

    // Mostrar en LCD que el mensaje fue enviado
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SMS enviado:");
    lcd.setCursor(0, 1);
    lcd.print(estadoTransporte);
}


void esperarRed() {
    bool redDisponible = false;
    lcd.clear();
    lcd.print("Buscando red");

    while (!redDisponible) {
        if (tieneRed()) {
            redDisponible = true;
            lcd.clear();
            lcd.print("Red encontrada");
        } else {
            lcd.setCursor(0, 1);
            lcd.print("Esperando...");
            delay(5000); // Espera 5 segundos antes de volver a chequear
        }
    }
}

bool tieneRed() {
    sim900.println("AT+CREG?"); // EnvÃ­a el comando AT para chequear el registro en la red
    delay(1000); // Da tiempo para recibir la respuesta

    // Lee la respuesta del SIM900
    if (sim900.available()) {
        String respuesta = sim900.readString();
        Serial.print("Respuesta de Red: "); // Opcional: imprime la respuesta en el Serial
        Serial.println(respuesta);
        
        // Verifica si la respuesta incluye ",1" o ",5", lo que indica que estÃ¡ registrado en la red
        if (respuesta.indexOf(",1") != -1 || respuesta.indexOf(",5") != -1) {
            return true;
        }
    }
    return false;
}

void enviarMensajeAlertaJetson(String mensajeAlerta) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enviando alerta.");
    
    sim900.print("AT+CMGF=1\r"); // Modo texto SMS
    delay(500);
    sim900.print("AT+CMGS=\"");
    sim900.print(numero);
    sim900.println("\"");
    delay(500);

    // ð Obtener datos de GPS
    float latitud = gps.location.lat();
    float longitud = gps.location.lng();

    // â  Mensaje con formato personalizado â 
    sim900.println("Alerta, el vehÃ­culo 1 presenta seÃ±ales de un evento violento:");
    sim900.print("Evento: ");
    sim900.println(mensajeAlerta);
    sim900.println("UbicaciÃ³n del vehÃ­culo:");
    sim900.print(latitud, 6); // 6 decimales para precisiÃ³n
    sim900.print(" , ");
    sim900.println(longitud, 6);

    sim900.println((char)26); // Ctrl+Z para enviar el mensaje
    delay(5000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Alerta enviada");
    delay(3000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Monitoreando");
    lcd.setCursor(0, 1);
    lcd.print("transporte.");
}


void actualizarGPS() {
    while (gpsSerial.available() > 0) {
        gps.encode(gpsSerial.read());
    }
}

void mostrarPosicion() {
    actualizarGPS();
    
    lcd.clear();
    lcd.setCursor(0, 0);
    if (gps.location.isValid()) {
        lcd.print("Lat:");
        lcd.print(gps.location.lat(), 6);
        lcd.setCursor(0, 1);
        lcd.print("Lon:");
        lcd.print(gps.location.lng(), 6);
    } else {
        lcd.print("GPS no valido");
    }
    delay(3000);
}

void enviarUbicacion() {
    actualizarGPS();
    
    if (gps.location.isValid()) {
        sim900.print("AT+CMGF=1\r");
        delay(500);
        sim900.print("AT+CMGS=\"");
        sim900.print(numero);
        sim900.println("\"");
        delay(500);

        sim900.print("Ubicacion: ");
        sim900.print("Lat: ");
        sim900.print(gps.location.lat(), 6);
        sim900.print(", Lon: ");
        sim900.print(gps.location.lng(), 6);
        sim900.println((char)26); // Ctrl+Z para enviar
        delay(5000);
    } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("GPS no valido");
        delay(3000);
    }
}
