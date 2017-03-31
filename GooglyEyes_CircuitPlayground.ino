

// Googly Eye Goggles
// By Bill Earl
// For Adafruit Industries
// from https://learn.adafruit.com/steam-punk-goggles/overview?view=all
// cktplayground contains this accelerometer https://learn.adafruit.com/adafruit-lis3dh-triple-axis-accelerometer-breakout?view=all
//
// The googly eye effect is based on a physical model of a pendulum.
// The pendulum motion is driven by accelerations in 2 axis.
// Eye color varies with orientation of the magnetometer

//rem'd out the magnetometer use
//now replace LSM303 with built in circuit playground gyro

#include <Wire.h>
#include <Adafruit_Sensor.h>
//#include <Adafruit_LSM303_U.h>
#include <Adafruit_CircuitPlayground.h>
#include <Adafruit_NeoPixel.h>

#define neoPixelPin 10

// We could do this as 2 16-pixel rings wired in parallel.
// But keeping them separate lets us do the right and left
// eyes separately if we want.
Adafruit_NeoPixel strip = Adafruit_NeoPixel(32, neoPixelPin, NEO_GRB + NEO_KHZ800);

//Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(54321);
// adafruit unified does not seem to support the lis accel found on cktplayground
//Adafruit_LSM303_Mag_Unified mag = Adafruit_LSM303_Mag_Unified(12345);
 
float pos = 8;  // Starting center position of pupil
float increment = 2 * 3.14159 / 16; // distance between pixels in radians
float MomentumH = 0; // horizontal component of pupil rotational inertia
float MomentumV = 0; // vertical component of pupil rotational inertia

// Tuning constants. (a.k.a. "Fudge Factors)  
// These can be tweaked to adjust the liveliness and sensitivity of the eyes.
const float friction = 0.995; // frictional damping constant.  1.0 is no friction.
const float swing = 60;  // arbitrary divisor for gravitational force
const float gravity = 200;  // arbitrary divisor for lateral acceleration
const float nod = 7.5; // accelerometer threshold for toggling modes

long nodStart = 0;
long nodTime = 2000;

bool antiGravity = false;  // The pendulum will anti-gravitate to the top.
bool mirroredEyes = false; // The left eye will mirror the right.

const float halfWidth = 1.25; // half-width of pupil (in pixels)

// Pi for calculations - not the raspberry type
const float Pi = 3.14159;

void setup(void) 
{
   strip.begin();
   strip.show(); // Initialize all pixels to 'off'  sensor_t sensor;

   // Initialize the sensors
   //accel.begin();
   //mag.begin();
   //I think we can do cktplayground.begin() instead of just accel.begin()
  CircuitPlayground.begin();
   
   resetModes();
}

// main processing loop
void loop(void) 
{
   // Read the magnetometer and determine the compass heading:
   sensors_event_t event; 
   //mag.getEvent(&event);

   // Calculate the angle of the vector y,x from magnetic North
   //float heading = (atan2(event.magnetic.y,event.magnetic.x) * 180) / Pi;
   float heading = 0.0; 

   // Normalize to 0-360 for a compass heading
   if (heading < 0)
   {
      heading = 360 + heading;
   }

   // Now read the accelerometer to control the motion.
   CircuitPlayground.lis.getEvent(&event);

   // Check for mode change commands
   CheckForNods(event);

   // apply a little frictional damping to keep things in control and prevent perpetual motion
   MomentumH *= friction;
   MomentumV *= friction;

   // Calculate the horizontal and vertical effect on the virtual pendulum
   // 'pos' is a pixel address, so we multiply by 'increment' to get radians.
   float TorqueH = cos(pos * increment);  // peaks at top and bottom of the swing
   float TorqueV = sin(pos * increment);    // peaks when the pendulum is horizontal

   // Add the incremental acceleration to the existing momentum
   // This code assumes that the accelerometer is mounted upside-down, level
   // and with the X-axis pointed forward.  So the Y axis reads the horizontal
   // acceleration and the inverse of the Z axis is gravity.
   // For other orientations of the sensor, just change the axis to match.
   MomentumH += TorqueH * event.acceleration.y / swing;
   if (antiGravity)
   {
     MomentumV += TorqueV * event.acceleration.z / gravity;
   }
   else
   {
     MomentumV -= TorqueV * event.acceleration.z / gravity;
   }

   // Calculate the new position
   pos += MomentumH + MomentumV;
   
   // handle the wrap-arounds at the top
   while (round(pos) < 0) pos += 16.0;
   while (round(pos) > 15) pos -= 16.0;

   // Now re-compute the display
   for (int i = 0; i < 16; i++)
   {
      // Compute the distance bewteen the pixel and the center
      // point of the virtual pendulum.
      float diff = i - pos;

      // Light up nearby pixels proportional to their proximity to 'pos'
      if (fabs(diff) <= halfWidth) 
      {
         uint32_t color;
         float proximity = halfWidth - fabs(diff) * 200;

         // pick a color based on heading & proximity to 'pos'
         color = selectColor(heading, proximity);
         
         // do both eyes
         strip.setPixelColor(i, color);
         if (mirroredEyes)
         {
           strip.setPixelColor(31 - i, color);
         }
         else
         {
           strip.setPixelColor(i + 16, color);
         }
      }
      else // all others are off
      {
         strip.setPixelColor(i, 0);
         if (mirroredEyes)
         {
           strip.setPixelColor(31 - i, 0);
         }
         else
         {
           strip.setPixelColor(i + 16, 0);
         }
      }
   }
   // Now show it!
   strip.show();
}

// choose a color based on the compass heading and proximity to "pos".
uint32_t selectColor(float heading, float proximity)
{
     uint32_t color;

     // Choose eye color based on the compass heading
     if (heading < 60)
     {
        color = strip.Color(0, 0, proximity);
     }
     else if (heading < 120)
     {
        color = strip.Color(0, proximity, proximity);
     }
     else if (heading < 180)
     {
        color = strip.Color(0, proximity, 0);
     }
     else if (heading < 240)
     {
        color = strip.Color(proximity, proximity, 0);
     }
     else if (heading < 300)
     {
        color = strip.Color(proximity, 0, 0);
     }
     else // 300-360
     {
        color = strip.Color(proximity, 0, proximity);
     }
}

// monitor orientation for mode-change 'gestures'
void CheckForNods(sensors_event_t event)
{
   if (event.acceleration.x > nod)
   {
     if (millis() - nodStart > nodTime)
     {
       antiGravity = false;  
       nodStart = millis(); // reset timer     
       spinDown();
     }
   }
   else if (event.acceleration.x < -(nod + 1))
   {
     if (millis() - nodStart > nodTime)
     {
       antiGravity = true;  
       spinUp();
       nodStart = millis(); // reset timer     
     }
   }
   else if (event.acceleration.y > nod)
   {
     if (millis() - nodStart > nodTime)
     {
       mirroredEyes = false;  
       spinDown();
       nodStart = millis(); // reset timer     
     }
   }
   else if (event.acceleration.y < -nod)
   {
     if (millis() - nodStart > nodTime)
     {
       mirroredEyes = true;  
       spinUp();
       nodStart = millis(); // reset timer     
      }
   }
   else // no nods in progress
   {
     nodStart = millis(); // reset timer
   }
}

// Reset to default
void resetModes()
{
   antiGravity = false;
   mirroredEyes = false;
   
   /// spin-up
   spin(strip.Color(255,0,0), 1, 500);
   spin(strip.Color(0,255,0), 1, 500);
   spin(strip.Color(0,0,255), 1, 500);
   spinUp();
}

// gradual spin up
void spinUp()
{
   for (int i = 300; i > 0;  i -= 20)
   {
     spin(strip.Color(255,255,255), 1, i);
   }
   pos = 0;
   // leave it with some momentum and let it 'coast' to a stop
   MomentumH = 3;  
}

// Gradual spin down
void spinDown()
{
   for (int i = 1; i < 300; i++)
   {
     spin(strip.Color(255,255,255), 1, i += 20);
   }
   // Stop it dead at the top and let it swing to the bottom on its own
   pos = 0;
   MomentumH = MomentumV = 0;
}


// utility function for feedback on mode changes.
void spin(uint32_t color, int count, int time)
{
  for (int j = 0; j < count; j++)
  {
    for (int i = 0; i < 16; i++)
    {
      strip.setPixelColor(i, color);
      strip.setPixelColor(31 - i, color);
      strip.show();
      delay(max(time / 16, 1));
      strip.setPixelColor(i, 0);
      strip.setPixelColor(31 - i, 0);
      strip.show();
    }
  }
}
