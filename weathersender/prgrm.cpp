#include <iostream>
#include <unistd.h>
#include "DHTxx.h"
#include <stdio.h>
#include <curl/curl.h>
#include <time.h>
 
using namespace std;
 
int main() {
  char buffer[100];
  CURL *curl;
  CURLcode res;
  
    
  float humidity;
  float temperature;
  int pin = 7;
 
  DHTxx dht22(pin, DHT22);
 
  if (dht22.init() == 0) {
    //for (int i = 0; i < 5; ++i) {
    while (true) {
      curl = curl_easy_init();
      int result = dht22.read(&humidity, &temperature);
      switch (result) {
      case DHT_ERROR_TIMEOUT:
        cout << "DHT_ERROR_TIMEOUT" << endl;
        break;
      case DHT_ERROR_CHECKSUM:
        cout << "DHT_ERROR_CHECKSUM" << endl;
        break;
      case DHT_ERROR_ARGUMENT:
        cout << "DHT_ERROR_ARGUMENT" << endl;
        break;
      case DHT_ERROR_GPIO:
        cout << "DHT_ERROR_GPIO" << endl;
        break;
      case DHT_SUCCESS:
        cout << "humidity = " << humidity << " %" << endl;
        cout << "temperature = " << temperature << " °C" << endl;
        if (curl) {
          curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.31.129:8080/api/add");
          //sprintf(buffer, "time=%lu&temp=%f&hum=%f", (unsigned long)time(NULL) + 3600 * 7, temperature, humidity); //UTC+0 to UTC+7
          sprintf(buffer, "time=%lu&temp=%f&hum=%f", (unsigned long)time(NULL), temperature, humidity); //UTC+0 to UTC+7

          printf("TIME SENT: %lu\n", (unsigned long)time(NULL) + 3600 * 7);
          curl_easy_setopt(curl, CURLOPT_POSTFIELDS, buffer);
          res = curl_easy_perform(curl);
          if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
          curl_easy_cleanup(curl);
      }
        break;
      default:
        break;
      }
      sleep(2);
    }
  }
  curl_global_cleanup();
  return 0;
}
