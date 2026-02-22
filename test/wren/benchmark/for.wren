var list = List();

var start = clock();
for (var i = 0; i < 1000000; i = i + 1) list.add(i);

var sum = 0;
for(var i = 0 ; i < list.size; i = i + 1) {
  sum = sum + list[i];
}

print sum;
println("elapsed: ", clock() - start);
