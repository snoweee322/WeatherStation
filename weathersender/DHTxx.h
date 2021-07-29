#ifndef DHTXX_H_
#define DHTXX_H_
 
#include <stdint.h>
 
/*
 * Define errors and return values.
 */
#define DHT_ERROR_TIMEOUT  -1
#define DHT_ERROR_CHECKSUM -2
#define DHT_ERROR_ARGUMENT -3
#define DHT_ERROR_GPIO     -4
#define DHT_SUCCESS         0
 
/*
 * Define sensor types.
 */
#define DHT11              11
#define DHT22              22
#define AM2302             22
 
/**
 * This is the only processor specific magic value, the maximum amount of time to
 * spin in a loop before bailing out and considering the read a timeout.  This should
 * be a high value, but if you're running on a much faster platform than a Raspberry
 * Pi or Beaglebone Black then it might need to be increased.
 */
#define DHT_MAXCOUNT    32000
 
/**
 * Number of bit pulses to expect from the DHT.  Note that this is 41 because
 * the first pulse is a constant 50 microsecond pulse, with 40 pulses to represent
 * the data afterwards.
 */
#define DHT_PULSES         41
 
class DHTxx {
private:
  int pin;
  int sensor;
 
public:
  DHTxx(int pin, int sensor);
  virtual ~DHTxx();
 
  int init();
 
  /**
   * Read DHT sensor connected to GPIO pin (using BCM numbering).  Humidity and temperature will be
   * returned in the provided parameters. If a successfull reading could be made a value of 0
   * (DHT_SUCCESS) will be returned.  If there was an error reading the sensor a negative value will
   * be returned.  Some errors can be ignored and retried, specifically DHT_ERROR_TIMEOUT or DHT_ERROR_CHECKSUM.
   */
  int read(float* humidity, float* temperature);
 
  int getPin() const;
 
  void setPin(int pin);
 
  int getSensor() const;
 
  void setSensor(int sensor);
 
private:
  /**
   * Busy wait delay for most accurate timing, but high CPU usage.
   * Only use this for short periods of time (a few hundred milliseconds at most)!
   */
  void busyWaitMilliseconds(uint32_t millis);
 
  /**
   * General delay that sleeps so CPU usage is low, but accuracy is potentially bad.
   */
  void sleepMilliseconds(uint32_t millis);
 
  /**
   * Increase scheduling priority and algorithm to try to get 'real time' results.
   */
  void setMaxPriority(void);
 
  /**
   * Drop scheduling priority back to normal/default.
   */
  void setDefaultPriority(void);
};
 
#endif /* DHTXX_H_ */
