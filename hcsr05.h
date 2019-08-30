#include <NewPing.h>

#define TRIGGER_PIN D2
#define ECHO_PIN D1

int distance = 0;
int calibrationMode = 0;
int maxDistance = 19 * 2.54; // in cm = inches * 2.54
int scaledDistance = 0;

NewPing sonar(TRIGGER_PIN, ECHO_PIN, 450); // NewPing setup of pins and maximum distance.

void setupHcsr05()
{
    //   pinMode(TRIGGER_PIN, OUTPUT);
    //   pinMode(ECHO_PIN, INPUT);
}

void readDistance()
{
    EVERY_N_MILLIS(50)
    {
        // digitalWrite(TRIG_PIN, LOW);
        // delayMicroseconds(2);

        // digitalWrite(TRIG_PIN, HIGH);
        // delayMicroseconds(10);

        // digitalWrite(TRIG_PIN, LOW);

        // const unsigned long duration = pulseIn(ECHO_PIN, HIGH);

        // distance = duration / 29 / 2;

        distance = sonar.ping_cm();

        if (distance > 450)
            distance = 450;

        if (calibrationMode != 0)
        {
            if (distance < maxDistance)
            {
                maxDistance = distance;
            }
        }

        int clippedDistance = distance;
        if (clippedDistance > maxDistance)
            clippedDistance = maxDistance;

        scaledDistance = map(clippedDistance, 0, maxDistance, 0, 255);

        Serial.print(distance);
        Serial.print(" ");
        Serial.print(maxDistance);
        Serial.print(" ");
        Serial.println(scaledDistance);
    }
}