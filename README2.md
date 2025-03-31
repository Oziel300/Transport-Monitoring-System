    Sistema de Monitoreo de Transporte
Este proyecto implementa un sistema de monitoreo para transporte público utilizando GPS y GSM. Permite enviar alertas y ubicación en tiempo real.

    Características
-Obtención de coordenadas GPS.
-Envío de alertas vía SMS utilizando el módulo SIM900.
-Interacción mediante botones físicos.
-Almacenamiento de número de emergencia en EEPROM.
-Interfaz con LCD I2C para visualización de estado.
-Requisitos

	Hardware:
-Arduino (Uno, Mega, etc.), el cual es usado para la programacion del AVR.
-Módulo GPS (NEO-6M)
-Módulo GSM (SIM900)
-LCD I2C 16x2
-EEPROM (integrada en el AVR)
-Botones o Pulsadores

	Librerías de Arduino:
-SoftwareSerial
-EEPROM
-Wire
-LiquidCrystal_I2C
-TinyGPS++

	Funcionamiento

Al iniciar, el sistema espera la señal de una IA enviada desde una Jetson, asu vez, espera la señal del GPS y la red GSM.
Los botones permiten modificar el número de emergencia y enviar alertas (Funcionando como un boton de panico).
Se detectan eventos predefinidos (asalto, disparos, vandalismo) y se notifican automáticamente mediante streaming.

	Contacto
Si tienes dudas o sugerencias, puedes abrir un issue en este repositorio.

Autores: Jonathan Mauricio Flores Monroy y Oziel David Hernández Hernández 
Fecha de publicación: 30 Marzo 2025

