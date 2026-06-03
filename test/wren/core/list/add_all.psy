var a = List(1);
a.add(2, 3);
print a; // expect: [1,2,3]
a.add();
print a; // expect: [1,2,3]
a.add(range(4,6));
print a; // expect: [1,2,3,[4,5,6]]

// Returns argument.
var range = range(7,9);
print a.add(range) == range; // expect: false
