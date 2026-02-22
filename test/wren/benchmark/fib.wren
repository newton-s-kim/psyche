fun fib(n) {
    if (n < 2) return n;
    return fib(n - 1) + fib(n - 2);
}

var start = clock();
for (var i = 1; i < 6 ; i = i + 1) {
    print fib(28);
}
println("elapsed: ", clock() - start);
