print 123 == 123;  // expect: true
print 123 == 124;  // expect: false
print -3 == 3;     // expect: false
print 0 == -0;     // expect: true

// Not equal to other types.
print 123 == "123"; // expect: false
print 1 == true;    // expect: false
print 0 == false;   // expect: false

print 123 != 123;  // expect: false
print 123 != 124;  // expect: true
print -3 != 3;     // expect: true
print 0 != -0;     // expect: false

// Not equal to other types.
print 123 != "123"; // expect: true
print 1 != true;    // expect: true
print 0 != false;   // expect: true
