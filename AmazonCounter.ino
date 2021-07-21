#include <Approximate.h>
#include <SevSeg.h>

PacketSniffer *packetSniffer = PacketSniffer::getInstance();
SevSeg numerialDisplay;

//Original data comes from http://standards.ieee.org/regauth/oui/oui.txt
int amazonOUI[] = {0x007147, 0x00BB3A, 0x00FC8B, 0x0812A5, 0x087C39, 0x08849D, 0x08A6BC, 0x0C47C9, 0x0CEE99, 0x109693, 0x140AC5, 0x149138, 0x18742E, 0x1C12B0, 0x1C4D66, 0x1CFE2B, 0x20A171, 0x244CE3, 0x34AFB3, 0x34D270, 0x38F73D, 0x3C5CC4, 0x40A2DB, 0x40A9CF, 0x40B4CD, 0x440049, 0x44650D, 0x4843DD, 0x4C1744, 0x4CEFC0, 0x50DCE7, 0x50F5DA, 0x5C415A, 0x6837E9, 0x6854FD, 0x689A87, 0x68DBF5, 0x6C5697, 0x7458F3, 0x747548, 0x74A7EA, 0x74AB93, 0x74C246, 0x74D637, 0x74ECB2, 0x78E103, 0x7C6166, 0x7CD566, 0x84D6D0, 0x8871E5, 0xA002DC, 0xA0D0DC, 0xA40801, 0xAC63BE, 0xB0FC0D, 0xB47C9C, 0xB85F98, 0xC49500, 0xC86C3D, 0xCC9EA2, 0xCCF735, 0xD4910F, 0xDC54D7, 0xDC91BF, 0xEC0DE4, 0xF0272D, 0xF04F7C, 0xF08173, 0xF0A225, 0xF0D2F1, 0xF0F0A4, 0xF4032A, 0xF854B8, 0xFC492D, 0xFC65DE, 0xFCA183, 0xFCA667};
int numberOfAmazonOUIs = (sizeof(amazonOUI) / sizeof(amazonOUI[0]));

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

bool packetEventHandler(wifi_promiscuous_pkt_t *wifi_pkt, uint16_t len, int type) {
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
  Serial.printf(".");
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
