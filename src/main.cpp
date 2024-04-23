/*
  Titre       DetectionUltrasonic
  Auteur      Crepin Vardin Fouelefack
  Date        25/02/2024
  Description Vérification de la présence d'un véhicule
  Version     0.0.1
  Liens utiles
       code de base Capteur ultrasonique   
                    https://wiki.dfrobot.com/URM09_Ultrasonic_Sensor_(Gravity_Analog)_SKU_SEN0307
 */

#include <Arduino.h>
#include <SPI.h>
#include <WiFi101.h>
#include <PubSubClient.h>

// Constantes pour la conversion de la distance à partir de la valeur analogique
#define MAX_RANG      (520)       // La valeur maximale de mesure du module est de 520 cm
#define ADC_RESOLUTION (4095.0)   // Précision ADC d'un Arduino UC1000 est de 12 bits
#define VCC           (5.0)       // Tension d'alimentation de l'Arduino

// Paramètres du réseau WiFi et du serveur MQTT
const char* ssid = "SM-A520W4657";
const char* password = "crepin23";
const char* mqtt_server = "192.168.203.215";
const char* mqtt_topic = "uc1000/distance";

// Déclaration des objets pour le WiFi et le client MQTT
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;

// Broche à laquelle le capteur ultrasonique est connecté
int analogPin = A3;
// Intervalle de temps entre chaque publication de la distance en millisecondes
const long interval = 5000;

// Configuration du réseau WiFi
void setup_wifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("Début du programme...");

  // Initialisation du réseau WiFi
  setup_wifi();
  Serial.println("WiFi connecté...");

  // Configuration du serveur MQTT
  client.setServer(mqtt_server, 1883);
  Serial.println("Serveur MQTT configuré...");
}

// Fonction pour reconnecter au serveur MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.println("Tentative de connexion au serveur MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("Connecté au serveur MQTT...");
      // Abonnement au sujet MQTT pour recevoir des commandes
      client.subscribe(mqtt_topic);
    } else {
      Serial.println("Échec de la connexion au serveur MQTT, nouvelle tentative dans 5 secondes...");
      delay(5000);
    }
  }
}

void loop() {
  // Vérification de la connexion au serveur MQTT
  if (!client.connected()) {
    reconnect();
  }
  // Gestion des messages MQTT
  client.loop();

  // Mesure de la durée écoulée depuis la dernière publication
  long currentMillis = millis();
  // Si l'intervalle est écoulé, publier la distance
  if (currentMillis - lastMsg > interval) {
    lastMsg = currentMillis;

    // Lecture de la valeur analogique du capteur
    int sensorValue = analogRead(analogPin);
    // Conversion de la valeur analogique en tension
    float voltage = (sensorValue / ADC_RESOLUTION) * VCC;
    // Calcul de la distance en centimètres
    float distance_cm = voltage * MAX_RANG / VCC;

    // Affichage de la distance sur le moniteur série
    Serial.print("Distance : ");
    Serial.print(distance_cm);
    Serial.println(" cm");

    // Convertir la distance en string
    String message = String(distance_cm, 2);

    // Publier la distance sur le serveur MQTT
    client.publish(mqtt_topic, message.c_str());
  }
}