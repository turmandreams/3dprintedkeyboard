#include <WiFi.h>
#include <esp_now.h>

#include <Arduino.h>
#include "esp32-ps2dev.h"  // Emulate a PS/2 device

const int CLK_PIN = 19;
const int DATA_PIN = 18;

esp32_ps2dev::PS2Keyboard keyboard(CLK_PIN, DATA_PIN);

// Estructura para recibir los datos de ESP-NOW
typedef struct struct_message {
    char comando[32];
} struct_message;

struct_message myData;

int teclaspulsadas[3][6][8];
int teclaspulsadastiempo[3][6][8];


bool esperandoByteLEDs = false;
bool esperandoByteRate = false;



void compruebaps2(){

  uint8_t comandoBios;

  // Escuchar de forma constante si la BIOS nos envía un byte
  if (keyboard.read(&comandoBios) == 0) {
    
    Serial.print("[BIOS -> PS2]: Recibido 0x");
    if (comandoBios < 0x10) Serial.print("0");
    Serial.println(comandoBios, HEX);

    // --- GESTIÓN DE SEGUNDOS BYTES (Argumentos de comandos previos) ---
    
    if (esperandoByteLEDs) {
      // Este byte contiene el estado real de los LEDs: bit0=ScrollLock, bit1=NumLock, bit2=CapsLock
      Serial.print("   -> Configuración de LEDs aplicada: ");
      Serial.println(comandoBios, BIN);
      
      keyboard.write(0xFA); // ACK para confirmar que procesamos los datos del LED
      esperandoByteLEDs = false;
      return;
    }

    if (esperandoByteRate) {
      // Este byte contiene la tasa de repetición y el retraso (delay)
      Serial.print("   -> Tasa de repetición aplicada: ");
      Serial.println(comandoBios, HEX);
      
      keyboard.write(0xFA); // ACK para confirmar que procesamos la tasa de repetición
      esperandoByteRate = false;
      return;
    }

    // --- GESTIÓN DE COMANDOS PRINCIPALES ---
    
    switch (comandoBios) {
      
      case 0xFF: // Comando RESET enviado por la BIOS
        Serial.println("   -> La BIOS pide reiniciar el teclado.");
        keyboard.write(0xFA); // 1. Responder con ACK (Entendido)
        delay(200);      // 2. Simular un breve tiempo de reinicio interno
        keyboard.write(0xAA); // 3. Volver a enviar el código de éxito del BAT
        Serial.println("[PS2 -> BIOS]: Reenviado 0xAA tras Reset.");
        break;

      case 0xF2: // Comando READ ID (¿Eres teclado o ratón?)
        Serial.println("   -> La BIOS solicita identificación de dispositivo.");
        keyboard.write(0xFA); // 1. Responder con ACK
        delay(2);
        keyboard.write(0xAB); // 2. Primer byte de ID de teclado
        keyboard.write(0x83); // 3. Segundo byte de ID de teclado
        Serial.println("[PS2 -> BIOS]: Enviado ID de Teclado Estándar (0xAB, 0x83)");
        break;

      case 0xED: // Comando SET LEDS (Viene un segundo byte)
        Serial.println("   -> La BIOS se prepara para cambiar luces indicadoras.");
        keyboard.write(0xFA); // Responder ACK para que la BIOS envíe el byte de datos
        esperandoByteLEDs = true; // Activar bandera para atrapar el próximo byte en el loop
        break;

      case 0xF3: // Comando SET TYPEMATIC RATE (Viene un segundo byte)
        Serial.println("   -> La BIOS se prepara para cambiar tasa de repetición.");
        keyboard.write(0xFA); // Responder ACK
        esperandoByteRate = true; // Activar bandera para atrapar el parámetro
        break;

      case 0xEE: // Comando ECHO (Herramienta de diagnóstico básico)
        Serial.println("   -> Diagnóstico Echo recibido.");
        keyboard.write(0xEE); // El protocolo exige responder exactamente con el mismo byte 0xEE (Sin ACK)
        Serial.println("[PS2 -> BIOS]: Respondido 0xEE (Echo)");
        break;

      case 0xF4: // ENABLE (La BIOS permite al teclado empezar a mandar teclas pulsadas)
        Serial.println("   -> Teclado habilitado por la BIOS. Listo para enviar pulsaciones.");
        keyboard.write(0xFA); // Responder ACK
        break;

      case 0xF5: // DISABLE (La BIOS duerme el teclado provisionalmente)
        Serial.println("   -> Teclado deshabilitado temporalmente.");
        keyboard.write(0xFA); // Responder ACK
        break;

      default: // Cualquier otro comando estándar (0xF6, etc.)
        Serial.println("   -> Comando genérico recibido.");
        keyboard.write(0xFA); // Responder ACK por defecto para no colgar el bus
        break;
    }
  }
}

void teclas(int id,int pulsada,int fila,int columna,boolean reinicia){

      if(pulsada){ 
          teclaspulsadas[id][fila][columna]=millis(); 
      }else {  
          teclaspulsadas[id][fila][columna]=0;           
          if(reinicia){  teclaspulsadastiempo[id][fila][columna]=300;  }          
      }

      if(id==0){
           
            if(pulsada==1){
              
                if(fila==5){
                  
                    if(columna==1) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_ESCAPE); }
                    else if(columna==2) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_F1); }
                    else if(columna==3) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_F2); }
                    else if(columna==4) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_F3); }
                    else if(columna==5) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_F4); }
                    else if(columna==6) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_F5); }
                    else if(columna==7) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_F6); }
                    
                }else if(fila==4){   
                  
                    if(columna==0) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_BACKQUOTE); }
                    else if(columna==1) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_1); }
                    else if(columna==2) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_2); }
                    else if(columna==3) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_3); }
                    else if(columna==4) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_4); }
                    else if(columna==5) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_5); }
                    else if(columna==6) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_6); }
                    else if(columna==7) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_7); }
                    
                }else if(fila==3){   
                  
                    if(columna==1) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_TAB); }
                    else if(columna==2) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_Q); }
                    else if(columna==3) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_W); }
                    else if(columna==4) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_E); }
                    else if(columna==5) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_R); }
                    else if(columna==6) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_T); }
                    else if(columna==7) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_Y); }
                    
                }else if(fila==2){   
                  
                    if(columna==2) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_CAPSLOCK); }
                    else if(columna==3) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_A); }
                    else if(columna==4) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_S); }
                    else if(columna==5) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_D); }
                    else if(columna==6) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_F); }
                    else if(columna==7) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_G); }
                    
                }else if(fila==1){
    
                    if(columna==1) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_LSHIFT); }
                    else if(columna==2) { keyboard.write(0x61); }
                    else if(columna==3) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_Z); }
                    else if(columna==4) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_X); }
                    else if(columna==5) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_C); }
                    else if(columna==6) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_V); }
                    else if(columna==7) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_B); }
                  
                }else if(fila==0){
    
                    if(columna==3) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_LCTRL); }
                    else if(columna==4) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_LSUPER); }
                    else if(columna==5) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_LALT); }
                    else if(columna==7) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_SPACE); }               
                  
                } 
                
                
            }else{
              
                if(fila==5){
                    
                    if(columna==1) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_ESCAPE); }
                    else if(columna==2) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_F1); }
                    else if(columna==3) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_F2); }
                    else if(columna==4) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_F3); }
                    else if(columna==5) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_F4); }
                    else if(columna==6) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_F5); }
                    else if(columna==7) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_F6); }
                    
                }else if(fila==4){
                  
                    if(columna==0) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_BACKQUOTE); }
                    else if(columna==1) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_1); }
                    else if(columna==2) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_2); }
                    else if(columna==3) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_3); }
                    else if(columna==4) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_4); }
                    else if(columna==5) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_5); }
                    else if(columna==6) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_6); }
                    else if(columna==7) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_7); }
                    
                }else if(fila==3){   
                  
                    if(columna==1) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_TAB); }
                    else if(columna==2) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_Q); }
                    else if(columna==3) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_W); }
                    else if(columna==4) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_E); }
                    else if(columna==5) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_R); }
                    else if(columna==6) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_T); }
                    else if(columna==7) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_Y); }
                    
                }else if(fila==2){   
                  
                    if(columna==2) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_CAPSLOCK); }
                    else if(columna==3) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_A); }
                    else if(columna==4) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_S); }
                    else if(columna==5) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_D); }
                    else if(columna==6) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_F); }
                    else if(columna==7) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_G); }
                    
                }else if(fila==1){
    
                    if(columna==1) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_LSHIFT); }
                    else if(columna==2) { keyboard.write(0x61); }
                    else if(columna==3) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_Z); }
                    else if(columna==4) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_X); }
                    else if(columna==5) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_C); }
                    else if(columna==6) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_V); }
                    else if(columna==7) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_B); }
                  
                }else if(fila==0){
    
                    if(columna==3) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_LCTRL); }
                    else if(columna==4) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_LSUPER); }
                    else if(columna==5) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_LALT); }
                    else if(columna==7) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_SPACE); }               
                  
                }            
            }

        }else if(id==1){
                
            if(pulsada==1){
              
                if(fila==5){
                  
                    if(columna==2) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_F7); }
                    else if(columna==3) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_F8); }
                    else if(columna==4) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_F9); }
                    else if(columna==5) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_F10); }
                    else if(columna==6) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_F11); }
                    else if(columna==7) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_F12); }
                    
                }else if(fila==4){   
                  
                    if(columna==2) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_8); }
                    else if(columna==3) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_9); }
                    else if(columna==4) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_0); }
                    else if(columna==5) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_MINUS); }
                    else if(columna==6) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_EQUALS); }
                    else if(columna==7) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_BACKSPACE); }
                    
                }else if(fila==3){   
                  
                    if(columna==1) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_U); }
                    else if(columna==2) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_I); }
                    else if(columna==3) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_O); }
                    else if(columna==4) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_P); }
                    else if(columna==5) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_LEFTBRACKET); }
                    else if(columna==6) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_RIGHTBRACKET); }
                    
                    
                }else if(fila==2){   
                  
                    if(columna==0) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_H); }
                    else if(columna==1) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_J); }
                    else if(columna==2) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_K); }
                    else if(columna==3) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_L); }
                    else if(columna==4) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_SEMICOLON); }
                    else if(columna==5) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_QUOTE); }
                    else if(columna==6) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_BACKSLASH); }
                    else if(columna==7) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_RETURN); }
                    
                }else if(fila==1){
    
                    if(columna==2) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_N); }
                    else if(columna==3) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_M); }
                    else if(columna==4) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_COMMA); }
                    else if(columna==5) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_PERIOD); }
                    else if(columna==6) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_SLASH); }
                    else if(columna==7) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_RSHIFT); }
                  
                }else if(fila==0){
    
                    if(columna==3) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_SPACE); }
                    else if(columna==4) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_RALT); }
                    else if(columna==5) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_RSUPER); }
                    else if(columna==6) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_MENU); }
                    else if(columna==7) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_RCTRL); }               
                  
                } 
                
                
            }else{
              
                if(fila==5){
                    
                     if(columna==2) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_F7); }
                    else if(columna==3) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_F8); }
                    else if(columna==4) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_F9); }
                    else if(columna==5) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_F10); }
                    else if(columna==6) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_F11); }
                    else if(columna==7) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_F12); }
                    
                }else if(fila==4){
                  
                    if(columna==2) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_8); }
                    else if(columna==3) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_9); }
                    else if(columna==4) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_0); }
                    else if(columna==5) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_MINUS); }
                    else if(columna==6) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_EQUALS); }
                    else if(columna==7) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_BACKSPACE); }
                    
                }else if(fila==3){   
                  
                    if(columna==1) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_U); }
                    else if(columna==2) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_I); }
                    else if(columna==3) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_O); }
                    else if(columna==4) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_P); }
                    else if(columna==5) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_LEFTBRACKET); }
                    else if(columna==6) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_RIGHTBRACKET); }
                    
                    
                }else if(fila==2){   
                  
                    if(columna==0) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_H); }
                    else if(columna==1) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_J); }
                    else if(columna==2) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_K); }
                    else if(columna==3) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_L); }
                    else if(columna==4) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_SEMICOLON); }
                    else if(columna==5) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_QUOTE); }
                    else if(columna==6) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_BACKSLASH); }
                    else if(columna==7) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_RETURN); }
                    
                }else if(fila==1){
    
                    if(columna==2) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_N); }
                    else if(columna==3) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_M); }
                    else if(columna==4) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_COMMA); }
                    else if(columna==5) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_PERIOD); }
                    else if(columna==6) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_SLASH); }
                    else if(columna==7) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_RSHIFT); }
                  
                }else if(fila==0){
    
                    if(columna==3) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_SPACE); }
                    else if(columna==4) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_RALT); }
                    else if(columna==5) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_RSUPER); }
                    else if(columna==6) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_MENU); }
                    else if(columna==7) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_RCTRL); }               
                  
                }   
            }

        }else if(id==2){
                
            if(pulsada==1){
              
                if(fila==3){   
                  
                    if(columna==2) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_DOWN); }
                    else if(columna==3) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_LEFT); }
                    else if(columna==4) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_UP); }
                    else if(columna==5) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_RIGHT); }
                    
                    
                }else if(fila==2){   
                  
                    if(columna==3) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_DELETE); }
                    else if(columna==4) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_END); }
                    else if(columna==5) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_PAGEDOWN); }
                    
                }else if(fila==1){
    
                    if(columna==3) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_INSERT); }
                    else if(columna==4) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_HOME); }
                    else if(columna==5) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_PAGEUP); }
                    
                }else if(fila==0){
    
                    if(columna==3) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_PRINT); }
                    else if(columna==4) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_SCROLLOCK); }
                    else if(columna==5) { keyboard.keydown(esp32_ps2dev::scancodes::Key::K_PAUSE); }                  
                  
                } 
                
                
            }else{
              
                if(fila==3){   
                  
                    if(columna==2) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_DOWN); }
                    else if(columna==3) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_LEFT); }
                    else if(columna==4) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_UP); }
                    else if(columna==5) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_RIGHT); }
                    
                    
                }else if(fila==2){   
                  
                    if(columna==3) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_DELETE); }
                    else if(columna==4) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_END); }
                    else if(columna==5) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_PAGEDOWN); }
                    
                }else if(fila==1){
    
                    if(columna==3) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_INSERT); }
                    else if(columna==4) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_HOME); }
                    else if(columna==5) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_PAGEUP); }

                  
                }else if(fila==0){
    
                    if(columna==3) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_PRINT); }
                    else if(columna==4) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_SCROLLOCK); }
                    else if(columna==5) { keyboard.keyup(esp32_ps2dev::scancodes::Key::K_PAUSE); }                  
          
                  
                }   
            }

        }


}



void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len) {
  
    memcpy(&myData, incomingData, sizeof(myData));
   
    //Serial.print("Se ha recibido dato : ");
    Serial.println(myData.comando);

    int id=myData.comando[0] - '0'; 
    int numeroteclas = myData.comando[1] - '0'; 

    
    for(int i=0;i<(numeroteclas*3);i+=3){

        int fila=myData.comando[i+2] - '0'; 
        int columna=myData.comando[i+3] - '0'; 
        int pulsada=myData.comando[i+4] - '0'; 

        //Serial.print(id);Serial.print(":");Serial.print(fila);Serial.print(columna);Serial.println(pulsada);
        
        teclas(id,pulsada,fila,columna,true);
       
    }
    
    
 
    
}



void setup() { 

    Serial.begin(115200);

    WiFi.mode(WIFI_STA);
      WiFi.disconnect();
      
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error inicializando ESP-NOW");
        return;
    }

    esp_now_register_recv_cb(OnDataRecv);
        
    keyboard.begin(); 

    for(int i=0;i<3;i++){
        for(int j=0;j<6;j++){
            for(int k=0;k<8;k++){
                teclaspulsadas[i][j][k]=0;
                teclaspulsadastiempo[i][j][k]=300;
            }
        }
    }

    delay(500); // Simulamos el tiempo que tarda el teclado en encenderse  
    keyboard.write(0xAA);
    
    
}

void loop() {

    
    
    for(int i=0;i<3;i++){
        for(int j=0;j<6;j++){
            for(int k=0;k<8;k++){

                
                if((i==0)&&(j==1)&&(k==1)) {         //LSHIFT               
                }else if((i==0)&&(j==0)&&(k==3)) {   //LCTRL
                }else if((i==0)&&(j==0)&&(k==5)) {   //LALT
                }else if((i==1)&&(j==1)&&(k==7)) {   //RSHIFT               
                }else if((i==1)&&(j==0)&&(k==4)) {   //RALT               
                }else if((i==1)&&(j==0)&&(k==7)) {   //RCTRL               
                  
                                  
                }else{
                  
                    if(teclaspulsadas[i][j][k]!=0){ 
    
                        if((millis()-teclaspulsadas[i][j][k])>teclaspulsadastiempo[i][j][k]){
                              
                              if(teclaspulsadastiempo[i][j][k]>50){  teclaspulsadastiempo[i][j][k]-=50;  }
                              if(teclaspulsadastiempo[i][j][k]>20){  teclaspulsadastiempo[i][j][k]-=20;  }
                              
                              teclas(i,false,j,k,false);                                                    
                              if(teclaspulsadastiempo[i][j][k]!=300){  teclas(i,true,j,k,false); }    
                              
                             
                        }                      
                    }
                  
                }
                                                
            }
        }
    }

    for(int i=0;i<10;i++){
      compruebaps2();
      delay(1);
    }
      
    
}
