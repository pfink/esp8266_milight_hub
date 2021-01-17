#include <MiLightDiscoveryServer.h>
#include <Size.h>
#ifdef ETHERNET
#include <Ethernet.h>
#else
#include <ESP8266WiFi.h>
#endif

const char V3_SEARCH_STRING[] = "Link_Wi-Fi";
const char V6_SEARCH_STRING[] = "HF-A11ASSISTHREAD";

MiLightDiscoveryServer::MiLightDiscoveryServer(Settings& settings)
  : settings(settings)
{ }

MiLightDiscoveryServer::MiLightDiscoveryServer(MiLightDiscoveryServer& other)
  : settings(other.settings)
{ }

MiLightDiscoveryServer& MiLightDiscoveryServer::operator=(MiLightDiscoveryServer other) {
  this->settings = other.settings;
  this->socket = other.socket;
  return *this;
}

MiLightDiscoveryServer::~MiLightDiscoveryServer() {
  socket.stop();
}

void MiLightDiscoveryServer::begin() {

  #ifdef MILIGHT_UDP_DEBUG
  Serial.printf("[MiLightDiscoveryServer port %d]\n", settings.discoveryPort);
  #endif

  uint8_t ret = socket.begin(settings.discoveryPort);

  #ifdef MILIGHT_UDP_DEBUG
  Serial.printf("[MiLightDiscoveryServer port %d] Begin returned %d\n", settings.discoveryPort, ret);
  #endif
}

void MiLightDiscoveryServer::handleClient() {
  size_t packetSize = socket.parsePacket();

  if (packetSize) {
    char buffer[size(V6_SEARCH_STRING) + 1];
    socket.read(buffer, packetSize);
    buffer[packetSize] = 0;

#ifdef MILIGHT_UDP_DEBUG
    printf("[MiLightUdpServer port %d] - Handling packet: ", settings.discoveryPort);
    for (size_t i = 0; i < packetSize; i++) {
      printf("%02X ", buffer[i]);
    }
    printf("\n");

    printf("Got discovery packet: %s\n", buffer);
#endif

    if (strcmp(buffer, V3_SEARCH_STRING) == 0) {
      handleDiscovery(5);
    } else if (strcmp(buffer, V6_SEARCH_STRING) == 0) {
      handleDiscovery(6);
    }
  }
}

void MiLightDiscoveryServer::handleDiscovery(uint8_t version) {
#ifdef MILIGHT_UDP_DEBUG
  printf_P(PSTR("Handling discovery for version: %u, %d configs to consider\n"), version, settings.gatewayConfigs.size());
#endif

  char buffer[40];

  for (size_t i = 0; i < settings.gatewayConfigs.size(); i++) {
    const GatewayConfig& config = *settings.gatewayConfigs[i];

    if (config.protocolVersion != version) {
      continue;
    }
    #ifdef ETHERNET
    IPAddress addr;
    addr.fromString(settings.wifiStaticIP);
    #else
    IPAddress addr = WiFi.localIP();
    #endif
    char* ptr = buffer;
    ptr += sprintf_P(
      buffer,
      PSTR("%d.%d.%d.%d,00000000%02X%02X"),
      addr[0], addr[1], addr[2], addr[3],
      (config.deviceId >> 8), (config.deviceId & 0xFF)
    );

    if (config.protocolVersion == 5) {
      sendResponse(buffer, strlen(buffer));
    } else {
      sprintf_P(ptr, PSTR(",HF-LPB100"));
      sendResponse(buffer, strlen(buffer));
    }
  }
}

void MiLightDiscoveryServer::sendResponse(char* buffer, size_t size) {
#ifdef MILIGHT_UDP_DEBUG
  printf_P(PSTR("Sending response: %s\n"), buffer);
  printf("[MiLightUdpServer port %d] - Sending packet: ", settings.discoveryPort);
  for (size_t i = 0; i < size; i++) {
    printf("%02X ", buffer[i]);
  }
  printf("\n");
#endif

  socket.beginPacket(socket.remoteIP(), socket.remotePort());
  socket.write((const uint8_t*) buffer, size);
  socket.endPacket();

}
