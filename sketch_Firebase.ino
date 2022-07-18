/*
 * Monitor o volume na sua caixa d'água Estágio UNINTER 
 * Autor: Thiago (1220215)
 * Biblioteca da Placa: "esp8266 by ESP8266 Community versão 2.3.0"
 * Placa: "NodeMCU 1.0 (ESP-12E Module)"
 * Upload Speed: "115200"
 * CPU Frequency: "160MHz"
*/
//=====================================================================
// --- Inclusão de bibliotecas ---
#include <ESP8266WiFi.h> // biblioteca de conexão do wifi
#include <Firebase_ESP_Client.h> // bibioteca para configuração do Firebase
//#include <addons/TokenHelper.h> // Provide the token generation process info.
//#include <addons/RTDBHelper.h> // Provide the RTDB payload printing info and other helper functions.

// --- Dados de Acesso do seu roteador ---
#define WIFI_SSID     "---"
#define WIFI_PASSWORD "---"

// Dados de acesso ao Firebase e Real Time DataBase
#define API_KEY "---"
#define DATABASE_URL "---"

// Dados de autenticação do Firebase
#define USER_EMAIL "---"
#define USER_PASSWORD "---"

// Definindo objetos do Firebase
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseJson json;

// --- Mapeamento de Hardware ---
#define trigPin 5  //D1 - PINO DE TRIGGER PARA SENSOR ULTRASSONICO
#define echoPin 4  //D2 - PINO DE ECHO PARA SENSOR ULTRASSONICO

// --- Variáveis Globais ---
unsigned long sendDataPrevMillis = 0;
long  duracao   = 0;
float distancia = 0;
float volumevar = 0;
  
void setup() {
  
  delay(2000);
  Serial.begin(115200); // inicia comunicação serial com velocidade 115200
  Serial.println();
  
  // Começa a conectar com o WIFI 
  Serial.print("Conectando ao ");
  Serial.println(WIFI_SSID);
  // Conecta ao ponto de acesso WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.println("WiFi Conectado");
  Serial.println("IP endereço: "); Serial.println(WiFi.localIP()); 

  // --- Configuração IO ---
  pinMode(trigPin, OUTPUT); // pino D1 como saída para TRIGGER
  pinMode(echoPin, INPUT);  // pino D2 como entrada para ECHO
  
  // Colocando API_key no Firebaseconfig
  config.api_key = API_KEY;

  // Colocando nomes nas credencias de acesso em FirebaseAuth
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Colocando URL no Firebaseconfig
  config.database_url = DATABASE_URL;

  //Logar no Firebase
  Firebase.begin(&config, &auth);

  // Reconectar caso caia
  Firebase.reconnectWiFi(true);

}

void loop() {
  // manda nova requisição a cada 
if (Firebase.ready() && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0))
  {
   sendDataPrevMillis = millis();
   
  // Leitura dos dados do Sensor HC104
  digitalWrite(trigPin, LOW);    // Limpa trigPin
  delayMicroseconds(2);          // aguarda 2 microsegundos
 
  digitalWrite(trigPin, HIGH);   // Seta trigPin HIGH aguarda 
  delayMicroseconds(10);         // aguada 10 microsegundos
  digitalWrite(trigPin, LOW);    // Seta trigPin LOW 
   
  // Leitura do echoPin, retorna a onda de som em microsegundos
  duracao = pulseIn(echoPin, HIGH);
  distancia= duracao*0.034/2;
 
  if (distancia > 22){  // leitura mínima. Reservatório vazio
    distancia = 22;
  }
    if (distancia < 0){  // leitura máxima. Reservatório vazio
    distancia = 0;
  }
  volumevar = map(distancia, 1, 20, 100, 0); 
   Serial.println();
   Serial.print("distancia:"); // imprime "distancia:"
   Serial.println(distancia);  // imprime a variavel distancia
   Serial.print("volume:");    // imprime "volume:"
   Serial.println(volumevar); // imprime a variavel volume
  /* Remapeia o range de leitura
   * Ao invés de ler de 1 a 20, lerá de 100 a 0 para expressar a porcentagem*/

      
   json.set("volume", volumevar);
   json.set("distancia", distancia);
  //Envio dos dados para o Firabase   
    if (Firebase.RTDB.pushJSON(&fbdo, "dados", &json)) {
      
      Firebase.RTDB.setTimestamp(&fbdo, "dados/" + fbdo.pushName()+ "/data");
      
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.print("NOME: ");
      Serial.println(fbdo.pushName());
      Serial.println("TYPE: " + fbdo.dataType());
    }else {
     Serial.println("FAILED");
     Serial.println("REASON: " + fbdo.errorReason());
   }
  }
}
