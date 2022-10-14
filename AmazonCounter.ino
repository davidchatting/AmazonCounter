#include <Approximate.h>  //https://github.com/davidchatting/Approximate
#include <SevSeg.h>       //https://github.com/DeanIsMe/SevSeg

#include "target-oui.h"
int amazonOUI[] = TARGET_OUI;
int numberOfAmazonOUIs = (sizeof(amazonOUI) / sizeof(amazonOUI[0]));

PacketSniffer *packetSniffer = PacketSniffer::getInstance();
SevSeg numerialDisplay;

List<MacAddr> amazonDevicesSeen;
char ma[18];

const int ledPin =  2;

void setup() {
  Serial.begin(115200);

  byte numDigits = 1;
  byte digitPins[] = {};
  byte segmentPins[] = {15, 0, 4, 16, 14, 13, 12, 5};
  bool resistorsOnSegments = true;

  byte hardwareConfig = COMMON_CATHODE;
  numerialDisplay.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments);
  numerialDisplay.setBrightness(90);
  numerialDisplay.setNumber(0);

  pinMode(ledPin, OUTPUT);

  packetSniffer -> init(1, true);
  packetSniffer -> setPacketEventHandler(packetEventHandler);
  packetSniffer -> begin();
}

void loop() {
  packetSniffer -> loop(); 
  numerialDisplay.refreshDisplay();
}

bool packetEventHandler(wifi_promiscuous_pkt_t *wifi_pkt, uint16_t len, int type, int subtype) {
  digitalWrite(ledPin, LOW);
  wifi_80211_data_frame *frame = (wifi_80211_data_frame *) wifi_pkt -> payload;

  MacAddr device;
  if (isAmazonDevice(frame, device)) {
    if (!amazonDevicesSeen.Contains(device)) {
      amazonDevicesSeen.Add(device);

      Approximate::MacAddr_to_c_str(&device, ma);
      Serial.printf("%s\t%i\t%i\n", ma, amazonDevicesSeen.Count(), wifi_pkt -> rx_ctrl.rssi);
      numerialDisplay.setNumber(amazonDevicesSeen.Count());
    }
  }
  //Serial.printf(".");
  digitalWrite(ledPin, HIGH);

  return(true);
}

bool isAmazonDevice(wifi_80211_data_frame* frame, MacAddr &device) {
  bool result = true;

  int daOui, saOui;
  Approximate::MacAddr_to_oui(&(frame -> da), daOui);
  Approximate::MacAddr_to_oui(&(frame -> sa), saOui);

  if (isAmazonDevice(daOui))       Approximate::MacAddr_to_MacAddr(&(frame -> da), device);
  else if (isAmazonDevice(saOui))  Approximate::MacAddr_to_MacAddr(&(frame -> sa), device);
  else result = false;

  return (result);
}

bool isAmazonDevice(int oui) {
  bool result = false;

  for (int n = 0; n < numberOfAmazonOUIs && !result; n++) result = (oui == amazonOUI[n]);

  return (result);
}
