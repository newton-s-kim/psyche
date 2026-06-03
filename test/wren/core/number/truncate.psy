print 123.truncate;      // expect: 123
print (-123).truncate;   // expect: -123
print 0.truncate;        // expect: 0
print (-0).truncate;     // expect: 0
print 0.123.truncate;    // expect: 0
print 12.3.truncate;     // expect: 12
print (-0.123).truncate; // expect: 0
print (-12.3).truncate;  // expect: -12

// Using 32-bit representation, values "beyond" those  two will lead to
// approximation.
print (12345678901234.5).truncate;   // expect: 12345678901234
print (-12345678901234.5).truncate;  // expect: -12345678901234
