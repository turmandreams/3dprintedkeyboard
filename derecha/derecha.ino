#include <esp_now.h>
#include <WiFi.h>
#include <driver/rtc_io.h>

#define pinled 21

#define pcolumna0 GPIO_NUM_32
#define pcolumna1 GPIO_NUM_33
#define pcolumna2 GPIO_NUM_25
#define pcolumna3 GPIO_NUM_27
#define pcolumna4 GPIO_NUM_14
#define pcolumna5 GPIO_NUM_12
#define pcolumna6 GPIO_NUM_13
#define pcolumna7 GPIO_NUM_15


#define columna0 32
#define columna1 33
#define columna2 25
#define columna3 27
#define columna4 14
#define columna5 12
#define columna6 13
#define columna7 15


#define pfila0 GPIO_NUM_19
#define pfila1 GPIO_NUM_18
#define pfila2 GPIO_NUM_5
#define pfila3 GPIO_NUM_17
#define pfila4 GPIO_NUM_16
#define pfila5 GPIO_NUM_4

#define fila0 19
#define fila1 18
#define fila2 5
#define fila3 17
#define fila4 16
#define fila5 4


boolean teclas[6][8];
boolean teclasant[6][8];

int contador=0;


// Dirección MAC del ESP32 Receptor (¡CORTA Y PEGA AQUÍ LA TUYA!)
uint8_t broadcastAddress[] = {0xE4, 0x65, 0xB8, 0x84, 0x59, 0xA8};

// La estructura DEBE ser idéntica a la del receptor
typedef struct struct_message {
    char comando[32]; // Array de 32 caracteres para el texto
} struct_message;

struct_message myData;

esp_now_peer_info_t peerInfo;

// Callback para saber si el mensaje llegó con éxito al receptor
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    //Serial.print("\r\nEstado del último envío: ");
    //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Envío Exitoso" : "Fallo en el Envío");
}


void compruebacolumnas(int fila){

    if(!digitalRead(columna0)){ teclas[fila][0]=true;}
    if(!digitalRead(columna1)){ teclas[fila][1]=true;}
    if(!digitalRead(columna2)){ teclas[fila][2]=true;}
    if(!digitalRead(columna3)){ teclas[fila][3]=true;}
    if(!digitalRead(columna4)){ teclas[fila][4]=true;}
    if(!digitalRead(columna5)){ teclas[fila][5]=true;}
    if(!digitalRead(columna6)){ teclas[fila][6]=true;}
    if(!digitalRead(columna7)){ teclas[fila][7]=true;}
    
}


void setup() {
    Serial.begin(115200);

    // Configurar Wi-Fi en modo Estación
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    pinMode(pinled,OUTPUT);

    // Inicializar ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error inicializando ESP-NOW");
        return;
    }

    pinMode(columna0,INPUT_PULLUP);
    pinMode(columna1,INPUT_PULLUP);
    pinMode(columna2,INPUT_PULLUP);
    pinMode(columna3,INPUT_PULLUP);
    pinMode(columna4,INPUT_PULLUP);
    pinMode(columna5,INPUT_PULLUP);
    pinMode(columna6,INPUT_PULLUP);
    pinMode(columna7,INPUT_PULLUP);

    
    
    if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_UNDEFINED) {
        rtc_gpio_hold_dis(pfila0);
        rtc_gpio_hold_dis(pfila1);
        rtc_gpio_hold_dis(pfila2);
        rtc_gpio_hold_dis(pfila3);
        rtc_gpio_hold_dis(pfila4);
        rtc_gpio_hold_dis(pfila5);
    }


    pinMode(fila0,OUTPUT); digitalWrite(fila0,HIGH);
    pinMode(fila1,OUTPUT); digitalWrite(fila1,HIGH);
    pinMode(fila2,OUTPUT); digitalWrite(fila2,HIGH);
    pinMode(fila3,OUTPUT); digitalWrite(fila3,HIGH);
    pinMode(fila4,OUTPUT); digitalWrite(fila4,HIGH);
    pinMode(fila5,OUTPUT); digitalWrite(fila5,HIGH);


    // Registrar el callback de envío
    esp_now_register_send_cb(OnDataSent);
    
    // Registrar el dispositivo receptor (Peer)
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 1;  // Asegúrate de que coincida con el canal del receptor
    peerInfo.encrypt = false;
    
    // Añadir el peer        
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Error al añadir el receptor");
        return;
    }

    digitalWrite(pinled,HIGH);
    
}

void loop() {

    for(int i=0;i<6;i++){  
        for(int j=0;j<8;j++){  
            teclasant[i][j]=teclas[i][j];
            teclas[i][j]=false;            
        }
    }
    
    
    
    digitalWrite(fila0,LOW); compruebacolumnas(0);digitalWrite(fila0,HIGH);
    digitalWrite(fila1,LOW); compruebacolumnas(1);digitalWrite(fila1,HIGH);
    digitalWrite(fila2,LOW); compruebacolumnas(2);digitalWrite(fila2,HIGH);
    digitalWrite(fila3,LOW); compruebacolumnas(3);digitalWrite(fila3,HIGH);
    digitalWrite(fila4,LOW); compruebacolumnas(4);digitalWrite(fila4,HIGH);
    digitalWrite(fila5,LOW); compruebacolumnas(5);digitalWrite(fila5,HIGH);

    int contadorteclas=0;
    
    String dato2="";
    
    boolean ninguno=true;
    for(int i=0;i<6;i++){  
        for(int j=0;j<8;j++){  
            if(teclasant[i][j]!=teclas[i][j]){
                          
                String mensaje="Tecla (";
                mensaje+=i;mensaje+=",";mensaje+=j;
                mensaje+=") ";

                dato2+=i;
                dato2+=j;
                
                if(teclas[i][j]){ mensaje+="Pulsada";dato2+="1";}
                else { mensaje+="Soltada";dato2+="0";}
                
                //Serial.println(mensaje);
                
                ninguno=false;
                contadorteclas++;               
                
            }
        }
    }

    //if(ninguno){ Serial.println("No se ha pulsado ninguna tecla");}

    if(!ninguno){
      contador=0;
      String dato="1";  //Identificador de teclado ( Izquierda,Derecha o Cursores )
      dato+=contadorteclas;
      dato+=dato2;
    
      int len = dato.length() + 1;    
      dato.toCharArray(myData.comando,len);        
      //Serial.print("Enviando texto... ");
      esp_err_t result1 = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
      
    }

    delay(20);
    contador++;

    if(contador>30000){

        digitalWrite(pinled,LOW);
        
        digitalWrite(fila0,LOW);
        digitalWrite(fila1,LOW);
        digitalWrite(fila2,LOW);
        digitalWrite(fila3,LOW);
        digitalWrite(fila4,LOW);
        digitalWrite(fila5,LOW);        
        
        rtc_gpio_hold_en(pfila0);
        rtc_gpio_hold_en(pfila1);
        rtc_gpio_hold_en(pfila2);
        rtc_gpio_hold_en(pfila3);
        rtc_gpio_hold_en(pfila4);
        rtc_gpio_hold_en(pfila5);
  

        esp_sleep_enable_ext0_wakeup(pcolumna0, 0);   
        esp_sleep_enable_ext0_wakeup(pcolumna1, 0);   
        esp_sleep_enable_ext0_wakeup(pcolumna2, 0);   
        esp_sleep_enable_ext0_wakeup(pcolumna3, 0);   
        esp_sleep_enable_ext0_wakeup(pcolumna4, 0);   
        esp_sleep_enable_ext0_wakeup(pcolumna5, 0);   
        esp_sleep_enable_ext0_wakeup(pcolumna6, 0);   
        esp_sleep_enable_ext0_wakeup(pcolumna7, 0);   
        
        esp_deep_sleep_start();      
        
    }    
    
}
