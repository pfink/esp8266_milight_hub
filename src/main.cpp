#ifndef UNIT_TEST

#include <ArduinoJson.h>
#include <stdlib.h>
#include <IntParsing.h>
#include <Size.h>
//#include <LinkedList.h>
#include <GroupStateStore.h>
#include <MiLightRadioConfig.h>
#include <MiLightRemoteConfig.h>
#include <MiLightRemoteType.h>
#include <Settings.h>
#include <MiLightUdpServer.h>
#include <RGBConverter.h>
#include <MiLightDiscoveryServer.h>
#include <MiLightClient.h>
#include <RadioSwitchboard.h>
#include <PacketSender.h>
#include <TransitionController.h>
#include <BridgeUdp.h>
#include <Console.h>

#include <vector>
#include <memory>

Settings settings;

MiLightClient* milightClient = NULL;
RadioSwitchboard* radios = nullptr;
PacketSender* packetSender = nullptr;
std::shared_ptr<MiLightRadioFactory> radioFactory;
MiLightDiscoveryServer* discoveryServer = NULL;
uint8_t currentRadioType = 0;

// For tracking and managing group state
GroupStateStore* stateStore = NULL;
TransitionController transitions;

int numUdpServers = 0;
std::vector<std::shared_ptr<MiLightUdpServer>> udpServers;

/**
 * Set up UDP servers (both v5 and v6).  Clean up old ones if necessary.
 */
void initMilightUdpServers() {
  udpServers.clear();

  for (size_t i = 0; i < settings.gatewayConfigs.size(); ++i) {
    const GatewayConfig& config = *settings.gatewayConfigs[i];

    std::shared_ptr<MiLightUdpServer> server = MiLightUdpServer::fromVersion(
      config.protocolVersion,
      milightClient,
      config.port,
      config.deviceId
    );

    if (server == NULL) {
      Serial.print(F("Error creating UDP server with protocol version: "));
      Serial.println(config.protocolVersion);
    } else {
      udpServers.push_back(std::move(server));
      udpServers[i]->begin();
    }
  }
}

/**
 * Milight RF packet handler.
 *
 * Called both when a packet is sent locally, and when an intercepted packet
 * is read.
 */
void onPacketSentHandler(uint8_t* packet, const MiLightRemoteConfig& config) {
  StaticJsonDocument<200> buffer;
  JsonObject result = buffer.to<JsonObject>();

  BulbId bulbId = config.packetFormatter->parsePacket(packet, result);

  if (&bulbId == &DEFAULT_BULB_ID) {
    Serial.println(F("Skipping packet handler because packet was not decoded"));
    return;
  }

  const MiLightRemoteConfig& remoteConfig =
    *MiLightRemoteConfig::fromType(bulbId.deviceType);

  // update state to reflect changes from this packet
  GroupState* groupState = stateStore->get(bulbId);

  // pass in previous scratch state as well
  const GroupState stateUpdates(groupState, result);

  if (groupState != NULL) {
    groupState->patch(stateUpdates);

    // Copy state before setting it to avoid group 0 re-initialization clobbering it
    stateStore->set(bulbId, stateUpdates);
  }

}

/**
 * Listen for packets on one radio config.  Cycles through all configs as its
 * called.
 */
void handleListen() {
  // Do not handle listens while there are packets enqueued to be sent
  // Doing so causes the radio module to need to be reinitialized inbetween
  // repeats, which slows things down.
  if (! settings.listenRepeats || packetSender->isSending()) {
    return;
  }

  std::shared_ptr<MiLightRadio> radio = radios->switchRadio(currentRadioType++ % radios->getNumRadios());

  for (size_t i = 0; i < settings.listenRepeats; i++) {
    if (radios->available()) {
      uint8_t readPacket[MILIGHT_MAX_PACKET_LENGTH];
      size_t packetLen = radios->read(readPacket);

      const MiLightRemoteConfig* remoteConfig = MiLightRemoteConfig::fromReceivedPacket(
        radio->config(),
        readPacket,
        packetLen
      );

      if (remoteConfig == NULL) {
        // This can happen under normal circumstances, so not an error condition
#ifdef DEBUG_PRINTF
        Serial.println(F("WARNING: Couldn't find remote for received packet"));
#endif
        return;
      }

      // update state to reflect this packet
      onPacketSentHandler(readPacket, *remoteConfig);
    }
  }
}

/**
 * Apply what's in the Settings object.
 */
void applySettings() {
  if (milightClient) {
    delete milightClient;
  }
  if (stateStore) {
    delete stateStore;
  }
  if (packetSender) {
    delete packetSender;
  }
  if (radios) {
    delete radios;
  }

  transitions.setDefaultPeriod(settings.defaultTransitionPeriod);

  radioFactory = MiLightRadioFactory::fromSettings(settings);

  if (radioFactory == NULL) {
    Serial.println(F("ERROR: unable to construct radio factory"));
  }

  stateStore = new GroupStateStore(MILIGHT_MAX_STATE_ITEMS, settings.stateFlushInterval);

  radios = new RadioSwitchboard(radioFactory, stateStore, settings);
  packetSender = new PacketSender(*radios, settings, onPacketSentHandler);

  milightClient = new MiLightClient(
    *radios,
    *packetSender,
    stateStore,
    settings,
    transitions
  );

  initMilightUdpServers();

  if (discoveryServer) {
    delete discoveryServer;
    discoveryServer = NULL;
  }
  if (settings.discoveryPort != 0) {
    discoveryServer = new MiLightDiscoveryServer(settings);
    discoveryServer->begin();
  }

}

/**
 *
 */
bool shouldRestart() {
  if (! settings.isAutoRestartEnabled()) {
    return false;
  }

  return settings.getAutoRestartPeriod()*60*1000 < millis();
}

// give a bit of time to update the status LED
void handleLED() {

}

// Called when a group is deleted via the REST API.  Will publish an empty message to
// the MQTT topic to delete retained state
void onGroupDeleted(const BulbId& id) {

}

void setup() {

  // load up our persistent settings from the file system
  Settings::load(settings);

  printf("ce pin: %u, csn pin %u, ip %s\n", settings.cePin, settings.csnPin, settings.wifiStaticIP.c_str());

  applySettings();

  transitions.addListener(
    [](const BulbId& bulbId, GroupStateField field, uint16_t value) {
      StaticJsonDocument<100> buffer;

      const char* fieldName = GroupStateFieldHelpers::getFieldName(field);
      buffer[fieldName] = value;

      milightClient->prepare(bulbId.deviceType, bulbId.deviceId, bulbId.groupId);
      milightClient->update(buffer.as<JsonObject>());
    }
  );

  Serial.printf_P(PSTR("Setup complete (version %s)\n"), QUOTE(MILIGHT_HUB_VERSION));
}

void loop() {

  for (size_t i = 0; i < udpServers.size(); i++) {
    udpServers[i]->handleClient();
  }

  if (discoveryServer) {
    discoveryServer->handleClient();
  }

  handleListen();

  stateStore->limitedFlush();
  packetSender->loop();

  transitions.loop();
}

#endif