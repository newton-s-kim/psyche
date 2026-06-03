print 123.fraction;      // expect: 0
print (-123).fraction;   // expect: 0
print 0.fraction;        // expect: 0
print (-0).fraction;     // expect: -0
print 0.123.fraction;    // expect: 0.123
print 12.3.fraction;     // expect: 0.3
print (-0.123).fraction; // expect: -0.123
print (-12.3).fraction;  // expect: -0.3

// Using 32-bit representation, a longer mantissa will lead to
// approximation.
print (1.23456789012345).fraction;  // expect: 0.23456789012345
print (-1.23456789012345).fraction;  // expect: -0.23456789012345

print (0.000000000000000000000000000000000000000001).fraction;  // expect: 1e-42
print (-0.000000000000000000000000000000000000000001).fraction;  // expect: -1e-42

print (1.000000000000000000000000000000000000000001).fraction;  // expect: 0
print (-1.000000000000000000000000000000000000000001).fraction;  // expect: 0
