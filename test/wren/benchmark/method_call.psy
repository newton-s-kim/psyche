class Toggle {
  init(startState) {
    this._state = startState;
  }

  value() { return this._state; }
  activate() {
    this._state = !this._state;
    return this;
  }
}

class NthToggle < Toggle {
  init(startState, maxCounter) {
    super.init(startState);
    this._countMax = maxCounter;
    this._count = 0;
  }

  activate() {
    this._count = this._count + 1;
    if (this._count >= this._countMax) {
      super.activate();
      this._count = 0;
    }

    return this;
  }
}

var start = clock();
var n = 100000;
var val = true;
var toggle = Toggle(val);

for (var i = 0; i < n; i = i + 1) {
  val = toggle.activate().value();
  val = toggle.activate().value();
  val = toggle.activate().value();
  val = toggle.activate().value();
  val = toggle.activate().value();
  val = toggle.activate().value();
  val = toggle.activate().value();
  val = toggle.activate().value();
  val = toggle.activate().value();
  val = toggle.activate().value();
}

print toggle.value();

val = true;
var ntoggle = NthToggle(val, 3);

for (var i = 0; i < n; i = i + 1) {
  val = ntoggle.activate().value();
  val = ntoggle.activate().value();
  val = ntoggle.activate().value();
  val = ntoggle.activate().value();
  val = ntoggle.activate().value();
  val = ntoggle.activate().value();
  val = ntoggle.activate().value();
  val = ntoggle.activate().value();
  val = ntoggle.activate().value();
  val = ntoggle.activate().value();
}

print ntoggle.value();
println("elapsed: ", clock() - start);
