var words = "";
var a = List("One", "Two", "Three");
a.each(fun (word) {
  //println(words,"+=",word);
  words = words + word;
});
print words; // expect: OneTwoThree
