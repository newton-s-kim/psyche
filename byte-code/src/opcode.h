//> op-constant
OPCODE(OP_CONSTANT)
//< op-constant
//> Types of Values literal-ops
OPCODE(OP_NIL)
OPCODE(OP_TRUE)
OPCODE(OP_FALSE)
//< Types of Values literal-ops
//> Global Variables pop-op
OPCODE(OP_POP)
//< Global Variables pop-op
//> Local Variables get-local-op
OPCODE(OP_GET_LOCAL)
//< Local Variables get-local-op
//> Local Variables set-local-op
OPCODE(OP_SET_LOCAL)
//< Local Variables set-local-op
//> Global Variables get-global-op
OPCODE(OP_GET_GLOBAL)
//< Global Variables get-global-op
//> Global Variables define-global-op
OPCODE(OP_DEFINE_GLOBAL)
//< Global Variables define-global-op
//> Global Variables set-global-op
OPCODE(OP_SET_GLOBAL)
//< Global Variables set-global-op
//> Closures upvalue-ops
OPCODE(OP_GET_UPVALUE)
OPCODE(OP_SET_UPVALUE)
//< Closures upvalue-ops
//> Classes and Instances property-ops
OPCODE(OP_GET_PROPERTY)
OPCODE(OP_SET_PROPERTY)
//< Classes and Instances property-ops
OPCODE(OP_GET_ELEMENT)
OPCODE(OP_SET_ELEMENT)
//> Superclasses get-super-op
OPCODE(OP_GET_SUPER)
//< Superclasses get-super-op
//> Types of Values comparison-ops
OPCODE(OP_EQUAL)
OPCODE(OP_GREATER)
OPCODE(OP_LESS)
//< Types of Values comparison-ops
//> A Virtual Machine binary-ops
OPCODE(OP_ADD)
OPCODE(OP_SUBTRACT)
OPCODE(OP_MULTIPLY)
OPCODE(OP_DIVIDE)
OPCODE(OP_MODULO)
OPCODE(OP_EXPONENT)
//> Types of Values not-op
OPCODE(OP_NOT)
//< Types of Values not-op
//< A Virtual Machine binary-ops
//> A Virtual Machine negate-op
OPCODE(OP_NEGATE)
//< A Virtual Machine negate-op
//> Global Variables op-print
OPCODE(OP_PRINT)
//< Global Variables op-print
//> Jumping Back and Forth jump-op
OPCODE(OP_JUMP)
//< Jumping Back and Forth jump-op
//> Jumping Back and Forth jump-if-false-op
OPCODE(OP_JUMP_IF_FALSE)
//< Jumping Back and Forth jump-if-false-op
//> Jumping Back and Forth loop-op
OPCODE(OP_LOOP)
//< Jumping Back and Forth loop-op
//> Calls and Functions op-call
OPCODE(OP_CALL)
//< Calls and Functions op-call
//> Methods and Initializers invoke-op
OPCODE(OP_INVOKE)
//< Methods and Initializers invoke-op
//> Superclasses super-invoke-op
OPCODE(OP_SUPER_INVOKE)
//< Superclasses super-invoke-op
//> Closures closure-op
OPCODE(OP_CLOSURE)
//< Closures closure-op
//> Closures close-upvalue-op
OPCODE(OP_CLOSE_UPVALUE)
//< Closures close-upvalue-op
OPCODE(OP_RETURN)
//> Classes and Instances class-op
OPCODE(OP_CLASS)
//< Classes and Instances class-op
OPCODE(OP_LIST)
OPCODE(OP_MAP)
//> Superclasses inherit-op
OPCODE(OP_INHERIT)
//< Superclasses inherit-op
//> Methods and Initializers method-op
OPCODE(OP_METHOD)
//< Methods and Initializers method-op
