# powerleds
You need to define the following : (look for them in the code).

// Thresholds for your Power Zones

#define ZONE_1 1 //  recovery watts - violet 

#define ZONE_2 125 // endurance - blue

#define ZONE_3 160 // tempo - green

#define ZONE_4 210 // threshold - yellow

#define ZONE_5 270 // aerobic - orange

#define ZONE_6 370 // nm - red



//LED control - Number of LEDS in your strip and the GPIO pin connected to the DATA input of the strip.
#define LED_PIN 25
#define NUM_LEDS 30

//Set here the specific address of the powermeter you want use. (You need to do a bluetooth search for it).
#define MY_BLE_ADDRESS "ce:01:fb:7f:d6:b1"  //address of the powermeter we want to use

If you want to connect to any available powermeter : replace line on line 25

    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID) && Scaned_BLE_Address == MY_BLE_ADDRESS ) {
with this : 

    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID) ) {





