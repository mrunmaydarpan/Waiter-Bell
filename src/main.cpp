#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Ticker.h>
#include <JC_Button.h>

#define CHANNEL 1
#define device_id 2
// uint8_t nodeAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t nodeAddress[] = {0x5C, 0xCF, 0x7F, 0xB2, 0xBB, 0xF0}; // 5C:CF:7F:B2:BB:F0

byte BUTTON_PIN(D2);
Button bellButton(BUTTON_PIN);

// Must match the receiver structure
typedef struct struct_message
{
  int deviceID;
  bool callState;
  bool respond;
} struct_message;

struct_message callData;
struct_message got;
Ticker t;

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus)
{
  Serial.print("Last Packet Send Status: ");
  Serial.println(sendStatus == 0 ? "sent" : "failed");
}

void gotData(uint8_t *mac, uint8_t *getdata, uint8_t len) // when data received
{
  memcpy(&got, getdata, sizeof(got));
  if (got.respond == true)
  {
    Serial.println("waiter responded");
  }
}

void data()
{
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  esp_now_send(nodeAddress, (uint8_t *)&callData, sizeof(callData)); // Send message via ESP-NOW
}

void setup()
{
  Serial.begin(9600); // Init Serial Monitor
  Serial.printf("\n\n\n");
  Serial.println("Starting Device " + callData.deviceID);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  bellButton.begin();
  WiFi.mode(WIFI_AP_STA);

  if (esp_now_init() != ERR_OK) // Init ESP-NOW
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  else
  {
    Serial.println("All OK");
    Serial.println(WiFi.macAddress());
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(gotData);
  esp_now_add_peer(nodeAddress, ESP_NOW_ROLE_COMBO, CHANNEL, NULL, 0); // Register peer
  // t.attach(2, data);
}

void loop()
{
  bellButton.read();
  if (bellButton.wasReleased())
  {
    callData.deviceID = device_id;
    callData.callState = true;
    Serial.println("calling waiter");
    esp_now_send(nodeAddress, (uint8_t *)&callData, sizeof(callData)); // Send message via ESP-NOW
  }
}