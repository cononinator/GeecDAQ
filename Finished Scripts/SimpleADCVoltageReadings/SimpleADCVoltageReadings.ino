/*
 * Author: Breandán Gillanders
 * Date: 11/02/2024
 * 
 * Got this script up and running on the Arduino Nano ESP32
 * Interfaces with DFRobot ADC - Prints 4 voltage readings every second to serial monitor
 */
#include <Wire.h>
#include <DFRobot_ADS1115.h>

DFRobot_ADS1115 ads(&Wire);
#define DFROBOT_ADS1115_CONVERSIONDELAY 10

void setup(void)
{
    Serial.begin(115200);
    Serial.println(DFROBOT_ADS1115_CONVERSIONDELAY);
    ads.setAddr_ADS1115(ADS1115_IIC_ADDRESS0);   // 0x48
    ads.setGain(eGAIN_TWOTHIRDS);   // 2/3x gain
    ads.setMode(eMODE_SINGLE);       // single-shot mode
    ads.setRate(eRATE_128);          // 128SPS (default) - REFER TO DFRobot_ADS1115.h FOR DIFFERENT DATA RATES
    ads.setOSMode(eOSMODE_SINGLE);   // Set to start a single-conversion
    ads.init();
}

void loop(void)
{
    if (ads.checkADS1115())
    {
        long adc0, adc1, adc2, adc3;
        double ibat, imot, vbat, vmot;

        adc0 = ads.readVoltage(0); //ibat
        adc1 = ads.readVoltage(1); //imot
        adc2 = ads.readVoltage(2); //vmot
        adc3 = ads.readVoltage(3); //vbat

        ibat = (10 * (adc0/1000.0 - 0.493));
        imot = (10 * (adc1/1000.0 - 0.493));
        vmot = (7.51*adc2/1000.0);
        vbat = (9.01*adc3/1000.0);
        
        Serial.printf("Ibat: %fA, Imot: %fA, Vmot: %fV, Vbat: %fV\n", ibat, imot, vmot, vbat);
    }
    else
    {
        Serial.println("ADS1115 Disconnected!");
    }

    delay(1000);
}