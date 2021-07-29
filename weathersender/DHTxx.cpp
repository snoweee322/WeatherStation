#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <sched.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <wiringPi.h>
#include "DHTxx.h"
 
DHTxx::DHTxx(int pin, int sensor) {
  this->pin = pin;
  this->sensor = sensor;
}
 
DHTxx::~DHTxx() {
}
 
int DHTxx::init() {
  /*
   * Initialize GPIO library.
   */
  if (wiringPiSetup() == -1) {
    return DHT_ERROR_GPIO;
  }
  return DHT_SUCCESS;
}
 
int DHTxx::read(float* humidity, float* temperature) {
  /* 
   * Validate humidity and temperature arguments and set them to zero.
   */
  if (humidity == NULL || temperature == NULL) {
    return DHT_ERROR_ARGUMENT;
  }
  *temperature = 0.0f;
  *humidity = 0.0f;
 
  /*
   * Store the count that each DHT bit pulse is low and high.
   * Make sure array is initialized to start at zero.
   */
  uint32_t pulseCounts[DHT_PULSES * 2] = { 0 };
  /*
   * Set pin to output.
   */
  pinMode(pin, OUTPUT);
  /*
   * Bump up process priority and change scheduler to try to try to make process more 'real time'.
   */
  setMaxPriority();
  /*
   * Set pin high for ~500 milliseconds.
   */
  digitalWrite(pin, HIGH);
  sleepMilliseconds(500);
  /*
   * The next calls are timing critical and care should be taken
   * to ensure no unnecssary work is done below.
   */
  /*
   * Set pin low for ~20 milliseconds.
   */
  digitalWrite(pin, LOW);
  busyWaitMilliseconds(20);
  /*
   * Set pin at input.
   */
  pinMode(pin, INPUT);
  /*
   * Need a very short delay before reading pins or else value is sometimes still low.
   */
  for (volatile int i = 0; i < 50; ++i) {
  }
  /*
   * Wait for DHT to pull pin low.
   */
  uint32_t count = 0;
  while (digitalRead(pin)) {
    if (++count >= DHT_MAXCOUNT) {
      /*
       * Timeout waiting for response.
       */
      setDefaultPriority();
      return DHT_ERROR_TIMEOUT;
    }
  }
  /*
   * Record pulse widths for the expected result bits.
   */
  for (int i = 0; i < DHT_PULSES * 2; i += 2) {
    /*
     * Count how long pin is low and store in pulseCounts[i]
     */
    while (!digitalRead(pin)) {
      if (++pulseCounts[i] >= DHT_MAXCOUNT) {
        /*
         * Timeout waiting for response.
         */
        setDefaultPriority();
        return DHT_ERROR_TIMEOUT;
      }
    }
    /*
     * Count how long pin is high and store in pulseCounts[i+1]
     */
    while (digitalRead(pin)) {
      if (++pulseCounts[i + 1] >= DHT_MAXCOUNT) {
        /*
         * Timeout waiting for response.
         */
        setDefaultPriority();
        return DHT_ERROR_TIMEOUT;
      }
    }
  }
  /*
   * Done with timing critical code, now interpret the results.
   */
  /*
   * Drop back to normal priority.
   */
  setDefaultPriority();
  /*
   * Compute the average low pulse width to use as a 50 microsecond reference threshold.
   * Ignore the first two readings because they are a constant 80 microsecond pulse.
   */
  uint32_t threshold = 0;
  for (int i = 2; i < DHT_PULSES * 2; i += 2) {
    threshold += pulseCounts[i];
  }
  threshold /= DHT_PULSES - 1;
  /*
   * Interpret each high pulse as a 0 or 1 by comparing it to the 50us reference.
   * If the count is less than 50us it must be a ~28us 0 pulse, and if it's higher
   * then it must be a ~70us 1 pulse.
   */
  uint8_t data[5] = { 0 };
  for (int i = 3; i < DHT_PULSES * 2; i += 2) {
    int index = (i - 3) / 16;
    data[index] <<= 1;
    if (pulseCounts[i] >= threshold) {
      /*
       * One bit for long pulse.
       */
      data[index] |= 1;
    }
    /*
     * Else zero bit for short pulse.
     */
  }
  /*
   * Verify checksum of received data.
   */
  if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
    if (sensor == DHT11) {
      /*
       * Get humidity and temp for DHT11 sensor.
       */
      *humidity = (float) data[0];
      if (data[1] > 0) {
        if (data[1] <= 9) {
          *humidity += data[1] / 10.0;
        } else {
          *humidity += data[1] / 100.0;
        }
      }
      *temperature = (float) data[2];
      if (data[3] > 0) {
        if (data[3] <= 9) {
          *temperature += data[3] / 10.0;
        } else {
          *temperature += data[3] / 100.0;
        }
      }
    } else if (sensor == DHT22) {
      /*
       * Calculate humidity and temp for DHT22 sensor.
       */
      *humidity = (data[0] * 256 + data[1]) / 10.0f;
      *temperature = ((data[2] & 0x7F) * 256 + data[3]) / 10.0f;
      if (data[2] & 0x80) {
        *temperature *= -1.0f;
      }
    }
    return DHT_SUCCESS;
  } else {
    return DHT_ERROR_CHECKSUM;
  }
}
 
int DHTxx::getPin() const {
  return pin;
}
 
void DHTxx::setPin(int pin) {
  this->pin = pin;
}
 
int DHTxx::getSensor() const {
  return sensor;
}
 
void DHTxx::setSensor(int sensor) {
  this->sensor = sensor;
}
 
void DHTxx::busyWaitMilliseconds(uint32_t millis) {
  /*
   * Set delay time period.
   */
  struct timeval deltatime;
  deltatime.tv_sec = millis / 1000;
  deltatime.tv_usec = (millis % 1000) * 1000;
  struct timeval walltime;
  /*
   * Get current time and add delay to find end time.
   */
  gettimeofday(&walltime, NULL);
  struct timeval endtime;
  timeradd(&walltime, &deltatime, &endtime);
  /*
   * Tight loop to waste time (and CPU) until enough time as elapsed.
   */
  while (timercmp(&walltime, &endtime, <)) {
    gettimeofday(&walltime, NULL);
  }
}
 
void DHTxx::sleepMilliseconds(uint32_t millis) {
  struct timespec sleep;
  sleep.tv_sec = millis / 1000;
  sleep.tv_nsec = (millis % 1000) * 1000000L;
  while (clock_nanosleep(CLOCK_MONOTONIC, 0, &sleep, &sleep) && errno == EINTR)
    ;
}
 
void DHTxx::setMaxPriority(void) {
  struct sched_param sched;
  memset(&sched, 0, sizeof(sched));
  /*
   * Use FIFO scheduler with highest priority for the lowest chance of the kernel context switching.
   */
  sched.sched_priority = sched_get_priority_max(SCHED_FIFO);
  sched_setscheduler(0, SCHED_FIFO, &sched);
}
 
void DHTxx::setDefaultPriority(void) {
  struct sched_param sched;
  memset(&sched, 0, sizeof(sched));
  /*
   * Go back to default scheduler with default 0 priority.
   */
  sched.sched_priority = 0;
  sched_setscheduler(0, SCHED_OTHER, &sched);
}
