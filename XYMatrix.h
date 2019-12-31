// From XYMatrix FastLED example: https://github.com/FastLED/FastLED/blob/master/examples/XYMatrix/XYMatrix.ino

// Helper functions for an two-dimensional XY matrix of pixels.
// Simple 2-D demo code is included as well.
//
//     XY(x,y) takes x and y coordinates and returns an LED index number,
//             for use like this:  leds[ XY(x,y) ] == CRGB::Red;
//             No error checking is performed on the ranges of x and y.
//
//     XYsafe(x,y) takes x and y coordinates and returns an LED index number,
//             for use like this:  leds[ XY(x,y) ] == CRGB::Red;
//             Error checking IS performed on the ranges of x and y, and an
//             index of "-1" is returned.  Special instructions below
//             explain how to use this without having to do your own error
//             checking every time you use this function.  
//             This is a slightly more advanced technique, and 
//             it REQUIRES SPECIAL ADDITIONAL setup, described below.

// Param for different pixel layouts
const bool    MatrixSerpentineLayout = true;
const bool    MatrixRotate = true;
// Set 'MatrixSerpentineLayout' to false if your pixels are 
// laid out all running the same way, like this:
//
//     0 >  1 >  2 >  3 >  4
//                         |
//     .----<----<----<----'
//     |
//     5 >  6 >  7 >  8 >  9
//                         |
//     .----<----<----<----'
//     |
//    10 > 11 > 12 > 13 > 14
//                         |
//     .----<----<----<----'
//     |
//    15 > 16 > 17 > 18 > 19
//
// Set 'MatrixSerpentineLayout' to true if your pixels are 
// laid out back-and-forth, like this:
//
//     0 >  1 >  2 >  3 >  4
//                         |
//                         |
//     9 <  8 <  7 <  6 <  5
//     |
//     |
//    10 > 11 > 12 > 13 > 14
//                        |
//                        |
//    19 < 18 < 17 < 16 < 15
//
// Bonus vocabulary word: anything that goes one way 
// in one row, and then backwards in the next row, and so on
// is call "boustrophedon", meaning "as the ox plows."

// Set 'MatrixRotate` to true if your pixels are laid out vertically:
//   4 >  5    14 >  15
//   |    |    |     |
//   |    |    |     |
//   3    6    13    16
//   |    |    |     |
//   |    |    |     |
//   2    7    12    17
//   |    |    |     |
//   |    |    |     |
//   1    8    11    18
//   |    |    |     |
//   |    |    |     |
//   0    9 >  10    19

// This function will return the right 'led index number' for 
// a given set of X and Y coordinates on your matrix.  
// IT DOES NOT CHECK THE COORDINATE BOUNDARIES.  
// That's up to you.  Don't pass it bogus values.
//
// Use the "XY" function like this:
//
//    for( uint8_t x = 0; x < MatrixWidth; x++) {
//      for( uint8_t y = 0; y < MatrixHeight; y++) {
//      
//        // Here's the x, y to 'led index' in action: 
//        leds[ XY( x, y) ] = CHSV( random8(), 255, 255);
//      
//      }
//    }
//
//
uint16_t XY( uint8_t x, uint8_t y)
{
  uint16_t i;
  
  if( MatrixSerpentineLayout == false && MatrixRotate == false) {
    i = (y * MatrixWidth) + x;
  }

  if( MatrixSerpentineLayout == false && MatrixRotate == true) {
    // TODO: fill out non-serpentine rotated XY
  }

  if( MatrixSerpentineLayout == true && MatrixRotate == false) {
    if( y & 0x01) {
      // Odd rows run backwards
      uint8_t reverseX = (MatrixWidth - 1) - x;
      i = (y * MatrixWidth) + reverseX;
    } else {
      // Even rows run forwards
      i = (y * MatrixWidth) + x;
    }
  }

  if( MatrixSerpentineLayout == true && MatrixRotate == true) {
    if( x & 0x01) {
      // Odd columns run backwards
      uint8_t reverseY = (MatrixHeight - 1) - y;
      i = (x * MatrixHeight) + reverseY;
    } else {
      // Even columns run forwards
      i = (x * MatrixHeight) + y;
    }
  }
  
  return i;
}

// pre-computed LED XY coordinates
uint8_t coordsX[NUM_LEDS];
uint8_t coordsY[NUM_LEDS];

void setupCoords() {
  for (uint8_t x = 0; x < MatrixWidth; x++) {
    for (uint8_t y = 0; y < MatrixHeight; y++) {
      uint16_t i = XY(x, y);
      coordsX[i] = x;
      coordsY[i] = y;
    }
  }
}
