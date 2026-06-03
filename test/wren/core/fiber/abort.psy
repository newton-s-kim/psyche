var fiber = thread(fun (thd) {
  thd.abort("Error message.");
});

print fiber.try(); // expect: Error message.
print fiber.isDone; // expect: true
print fiber.error; // expect: Error message.
