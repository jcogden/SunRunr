//-------------------------------------------------------------------

#include "PotholeReporter.h"
#include "PotholeDetector.h"
#include "PotholeLocation.h"
#include <AssetTracker.h>
#include <queue>

//-------------------------------------------------------------------

using namespace std;

//-------------------------------------------------------------------

#define ONE_DAY_MILLIS (24 * 60 * 60 * 1000)
unsigned long lastSync = millis();

//-------------------------------------------------------------------

bool executeStateMachines = false;

//-------------------------------------------------------------------

AssetTracker locationTracker = AssetTracker();
queue<PotholeLocation> potholeLocations;
PotholeDetector potholeDetector(locationTracker, potholeLocations, 2, 10, 10000.0);
PotholeReporter potholeReporter(locationTracker, potholeLocations);

//-------------------------------------------------------------------

void stateMachineScheduler() {
    executeStateMachines = true;
}

Timer stateMachineTimer(10, stateMachineScheduler);

//-------------------------------------------------------------------

void responseHandler(const char *event, const char *data) {
    // Formatting output
    String output = String::format("POST Response:\n  %s\n  %s\n", event, data);
    // Log to serial console
    Serial.println(output);
}

//-------------------------------------------------------------------

void setup() {
    Serial.begin(9600);

    // Initialize the gps and turn it on
    locationTracker.begin();
    locationTracker.gpsOn();

    // Handler for response from POSTing location to server
    Particle.subscribe("hook-response/holz", responseHandler, MY_DEVICES);

    stateMachineTimer.start();
}

//-------------------------------------------------------------------

void loop() {

    // Request time synchronization from the Particle Cloud once per day
    if (millis() - lastSync > ONE_DAY_MILLIS) {
        Particle.syncTime();
        lastSync = millis();
    }

    if (executeStateMachines) {
        locationTracker.updateGPS();
        potholeDetector.execute();
        potholeReporter.execute();
        executeStateMachines = false;
    }
}

//-------------------------------------------------------------------
