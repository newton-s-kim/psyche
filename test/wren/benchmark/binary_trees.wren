// Ported from the Python version.

class Tree {
  init(item, depth) {
    this._item = item;
    if (depth > 0) {
      var item2 = item + item;
      depth = depth - 1;
      this._left = Tree(item2 - 1, depth);
      this._right = Tree(item2, depth);
    } else {
      this._left = nil;
      this._right = nil;
    }
  }

  check() {
    if (this._left == nil) {
      return this._item;
    }

    return this._item + this._left.check() - this._right.check();
  }
}

var minDepth = 4;
var maxDepth = 12;
var stretchDepth = maxDepth + 1;

var start = clock();

println("stretch tree of depth ", stretchDepth, " check: ",
    Tree(0, stretchDepth).check());

var longLivedTree = Tree(0, maxDepth);

var iterations = 2 ^ maxDepth;

var depth = minDepth;
while (depth <= stretchDepth) {
  var check = 0;
  for (var i = 1; i <= iterations; i = i + 1) {
    check = check + Tree(i, depth).check() + Tree(-i, depth).check();
  }

  println(iterations * 2, " trees of depth ", depth, " check: ", check);
  iterations = iterations / 4;
  depth = depth + 2;
}

println("long lived tree of depth ", maxDepth, " check: ", longLivedTree.check());
println("elapsed: ", clock() - start);
