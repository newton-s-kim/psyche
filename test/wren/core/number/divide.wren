print 8 / 2;         // expect: 4
print 12.34 / -0.4;  // expect: -30.85

// Divide by zero.
print 3 / 0;         // expect: inf
print -3 / 0;        // expect: -inf
print 0 / 0;         // expect: nan
print -0 / 0;        // expect: -nan
print 3 / -0;        // expect: -inf
print -3 / -0;       // expect: inf
print 0 / -0;        // expect: -nan
print -0 / -0;       // expect: -nan
