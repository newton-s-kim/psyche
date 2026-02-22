
var list = List(0, 1, 2, 3, 4);
print list.indexOf(4);   // expect: 4
print list.indexOf(2);   // expect: 2
print list.indexOf(3);   // expect: 3
print list.indexOf(0);   // expect: 0
print list.indexOf(100); // expect: -1
print list.indexOf(-1);  // expect: -1
