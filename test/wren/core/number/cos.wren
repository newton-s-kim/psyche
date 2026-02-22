include "math";
print cos(0);       // expect: 1
print cos(pi);      // expect: -1
print cos(2 * pi);  // expect: 1

// this should of course be 0, but it's not that precise
print cos(pi / 2).abs < 1.0e-16; // expect: true
