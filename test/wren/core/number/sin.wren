include "math";
print sin(0);       // expect: 0
print sin(pi / 2);  // expect: 1

// these should of course be 0, but it's not that precise
print sin(pi).abs < 1.0e-15;        // expect: true
print sin(2 * pi).abs < 1.0e-15;  // expect: true
