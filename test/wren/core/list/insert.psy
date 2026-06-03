// Add to empty list.
var a = List();
a.insert(0, 1);
print a; // expect: [1]

// Normal indices.
var b = List(1, 2, 3);
b.insert(0, 4);
print b; // expect: [4,1,2,3]

var c = List(1, 2, 3);
c.insert(1, 4);
print c; // expect: [1,4,2,3]

var d = List(1, 2, 3);
d.insert(2, 4);
print d; // expect: [1,2,4,3]

var e = List(1, 2, 3);
e.insert(3, 4);
print e; // expect: [1,2,3,4]

// Negative indices.
var f = List(1, 2, 3);
f.insert(-4, 4);
print f; // expect: [4,1,2,3]

var g = List(1, 2, 3);
g.insert(-3, 4);
print g; // expect: [1,4,2,3]

var h = List(1, 2, 3);
h.insert(-2, 4);
print h; // expect: [1,2,4,3]

var i = List(1, 2, 3);
i.insert(-1, 4);
print i; // expect: [1,2,3,4]

// Returns.inserted value.
print List(1, 2).insert(0, 3); // expect: 3
