#include <TinyGPS++.h>
#include <SoftwareSerial.h>



TinyGPSPlus gps;  // The TinyGPS++ object


SoftwareSerial ss(4, 5); // The serial connection to the GPS device



float latitude , longitude;





//WiFiServer server(80);

void setup()

{

  Serial.begin(115200);

  ss.begin(9600);


}


void loop()

{


  while (ss.available() > 0)
  {

    if (gps.encode(ss.read()))

    {

      if (gps.location.isValid())

      {

        latitude = gps.location.lat();
        
        longitude = gps.location.lng();

      }


    }


  delay(100);


}
