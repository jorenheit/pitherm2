#include <wiringPi.h>
#include <mcp3004.h>
#include <iostream>
#include <fstream>
#include <cmath>

enum SPI_Params
{
    SPI_CHANNEL = 0,
    BASE = 100,
    TEMP_SENSOR = BASE
};

enum ControlPins
{
    RELAY_PIN = 0
};

enum Limits
{
    MIN_TEMP = 15,
    MAX_TEMP = 25
};

double halfRound(double x)
{
    return (int)x + std::round((x - (int)x) * 2) * 0.5;
}

double getTemp()
{
    enum SensorProperties
    {
        VOLTAGE = 5000, // mV
        RANGE = 1024,
        RESOLUTION = 10 // mV / degree
    };
    
    double val = analogRead(TEMP_SENSOR);
    double mv = (val / RANGE) * VOLTAGE;
    double celcius = mv / RESOLUTION;

    return halfRound(celcius);
}
    
double getSetting(std::string const &fname)
{
    std::ifstream settingFile(fname);
    double setting;
    settingFile >> setting;

    if (setting < MIN_TEMP)
        return MIN_TEMP;
    else if (setting > MAX_TEMP)
        return MAX_TEMP;
    else
        return halfRound(setting);
}

int main(int argc, char**argv)
{
    if (argc != 2)
    {
        std::cerr << "Syntax: " << argv[0] << " [setting-file]\n";
        return 1;
    }
            
    if (wiringPiSetup() == -1)
    {
        std::cerr << "Failed to initialize wiringPi\n";
        return 1;
    }
    
    mcp3004Setup(BASE, SPI_CHANNEL);
    pinMode(RELAY_PIN, OUTPUT);

    std::string fname = argv[1];
    bool heating = false;
    digitalWrite(RELAY_PIN, false);
    
    while (true)
    {
        double temp = getTemp();
        double setting = getSetting(fname);

        std::cout << temp << ' '  << setting << '\n';
        
        if (!heating && temp < setting)
        {
            digitalWrite(RELAY_PIN, true);
            heating = true;
        }
        else if (heating && temp > setting)
        {
            digitalWrite(RELAY_PIN, false);
            heating = false;
        }
        
        delay(2000);
    }
}

