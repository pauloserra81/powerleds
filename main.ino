/**
 * A modified BLE client that will read BLE Power 
 * and setup a led strip
 * author Paulo Serra
 */
#include <Arduino.h>
#include <Ticker.h>
#include "BLEDevice.h"
#include "FastLED.h"


// Power Zones
#define ZONE_1 1 //  recovery watts - violet 
#define ZONE_2 125 // endurance - blue
#define ZONE_3 160 // tempo - green
#define ZONE_4 210 // threshold - yellow
#define ZONE_5 270 // aerobic - orange
#define ZONE_6 370 // nm - red

//LED control
#define LED_PIN 25
#define NUM_LEDS 30

// Define the array of leds
CRGB leds[NUM_LEDS];

static int power1;
static int power2;

#define CYCLING_POWER_SERVICE_UUID "00001818-0000-1000-8000-00805F9B34FB"
#define POWER_CHARACTERISTIC_UUID "00002A63-0000-1000-8000-00805F9B34FB"

#define MY_BLE_ADDRESS "ce:01:fb:7f:d6:b1"  //address of the powermeter we want to use

// The remote service we wish to connect to.
static BLEUUID serviceUUID(CYCLING_POWER_SERVICE_UUID);
// The characteristic of the remote service we are interested in.
static BLEUUID charUUID(BLEUUID((uint16_t)0x2A63));
//0x2A63

static boolean doConnect = false;
static boolean connected = false;
static boolean notification = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    Serial.print("Power : ");
    Serial.print(pData[2] + 256*pData[3], DEC);
    Serial.println("Watts");

    static int power3s;

    power3s = ( power2 + power1 + (pData[2] + 256*pData[3]) )/3.0 ; 

    power2 = power1;
    power1 = pData[2] + 256*pData[3];

    static int on_leds;
    if (power3s >= ZONE_6)   {
      on_leds = NUM_LEDS;
    }
    else {
    on_leds =  (( power3s -1) / (float)ZONE_6)*NUM_LEDS + 1;
    }
    
    if(power3s == 0) {
      for(int i=1; i<=NUM_LEDS; i++){
      leds[i-1] = CRGB::Black;
      }
      leds[0] = CRGB::Purple;
      FastLED.show();
    }
    else if(power3s <= ZONE_1 && power3s > 0) {
      Serial.println("ZONE 0!");
      for(int i=1; i<=NUM_LEDS; i++){
      leds[i-1] = CRGB::Black;
      }
      for(int i=1; i<=on_leds; i++){
      leds[i-1] = CRGB::Violet;
      }
      FastLED.show();
      }
    else if(power3s > ZONE_1 && power3s <= ZONE_2) {
      Serial.println("ZONE 1!");
      for(int i=1; i<=NUM_LEDS; i++){
      leds[i-1] = CRGB::Black;
      }
      for(int i=1; i<=on_leds; i++){
      leds[i-1] = CRGB::Blue;
      }
      FastLED.show();
      }
    else if(power3s > ZONE_2 && power3s <= ZONE_3) {
      Serial.println("ZONE 2!");
      for(int i=1; i<=NUM_LEDS; i++){
      leds[i-1] = CRGB::Black;
      }
      for(int i=1; i<=on_leds; i++){
      leds[i-1] = CRGB::Cyan;
      }
      FastLED.show();
      }
    else if(power3s > ZONE_3 && power3s <= ZONE_4) {
      Serial.println("ZONE 3!");
      for(int i=1; i<=NUM_LEDS; i++){
      leds[i-1] = CRGB::Black;
      }
      for(int i=1; i<=on_leds; i++){
      leds[i-1] = CRGB::Green;
      }
      FastLED.show();
      }
    else if(power3s > ZONE_4 && power3s <= ZONE_5) {
      Serial.println("ZONE 4!");
      for(int i=1; i<=NUM_LEDS; i++){
      leds[i-1] = CRGB::Black;
      }
      for(int i=1; i<=on_leds; i++){
      leds[i-1] = CRGB::Yellow;
      }
      FastLED.show();
      }
    else if(power3s > ZONE_5 && power3s <= ZONE_6) {
      Serial.println("ZONE 5!");
      for(int i=1; i<=NUM_LEDS; i++){
      leds[i-1] = CRGB::Black;
      }
      for(int i=1; i<=on_leds; i++){
      leds[i-1] = CRGB::Orange;
      }
      FastLED.show();
      }      
    else if(power3s > ZONE_6) {
      Serial.println("ZONE 6!");
      for(int i=1; i<=NUM_LEDS; i++){
      leds[i-1] = CRGB::Red;
      }
      FastLED.show();
    }
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};

bool connectToServer() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");


    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic");

    // Read the value of the characteristic.
    if(pRemoteCharacteristic->canRead()) {
      std::string value = pRemoteCharacteristic->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }

    if(pRemoteCharacteristic->canNotify())
      pRemoteCharacteristic->registerForNotify(notifyCallback);

    connected = true;
    return true;
}
/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());
    //check for address here. use .getAddress
    static BLEAddress *Server_BLE_Address;
    String Scaned_BLE_Address;
    Server_BLE_Address = new BLEAddress(advertisedDevice.getAddress());
    Scaned_BLE_Address = Server_BLE_Address->toString().c_str();
    Serial.print("BLE Advertised Device address: ");
    Serial.println(Scaned_BLE_Address);


    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID) && Scaned_BLE_Address == MY_BLE_ADDRESS ) {

      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    } // Found our server
  } // onResult
}; // MyAdvertisedDeviceCallbacks


void setup() {
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);  // setup led - GRB ordering is assumed

  //LED starts white until connected
    //led is single LED, white
      for(int i=1; i<=NUM_LEDS; i++){
      leds[i-1] = CRGB::Black;
      }
      leds[0] = CRGB::White;
      FastLED.show();

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
} // End of setup.


// This is the Arduino main loop function.
void loop() {
  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }

  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.
  if (connected) {
    if (notification == false) {
        Serial.println("Turning Notification On");
        const uint8_t onPacket[] = {0x01, 0x0};
        pRemoteCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)onPacket, 2, true);
        notification = true;
    }
  }else if(doScan){
    BLEDevice::getScan()->start(5);  // this is just eample to start scan after disconnect, most likely there is better way to do it in arduino
  }

  //LED indicates connected status
  if (connected) {
      }
  else { 
    //led is single LED, white
      for(int i=1; i<=NUM_LEDS; i++){
      leds[i-1] = CRGB::Black;
      }
      leds[0] = CRGB::White;
      FastLED.show();
    BLEDevice::getScan()->start(5); //we are not connected, scan again
    }
  
  delay(1000); // Delay a second between loops.
} // End of loop
