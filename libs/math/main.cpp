extern "C" {
#include "vm.h"
}
#include <cmath>

extern "C" Value asinNative(int argc, Value* args)
{
    Value value = NIL_VAL;
    if (1 != argc)
        runtimeError("invalid arguments");
    if (IS_NUMBER(args[0])) {
        value = NUMBER_VAL(asin(AS_NUMBER(args[0])));
    }
    /*
    else if (IS_COL(args[0])) {
        ObjCol* c = factory->newCol();
        c->value = asin(AS_COL(args[0])->value);
        value = OBJ_VAL(c);
    }
    */
    else {
        runtimeError("number is expected");
    }
    return value;
}

extern "C" Value atanNative(int argc, Value* args)
{
    Value value = NIL_VAL;
    if (1 != argc)
        runtimeError("invalid arguments");
    if (IS_NUMBER(args[0])) {
        value = NUMBER_VAL(atan(AS_NUMBER(args[0])));
    }
    /*
    else if (IS_COL(args[0])) {
        ObjCol* c = factory->newCol();
        c->value = atan(AS_COL(args[0])->value);
        value = OBJ_VAL(c);
    }
    */
    else {
        runtimeError("number is expected");
    }
    return value;
}

extern "C" Value acosNative(int argc, Value* args)
{
    Value value = NIL_VAL;
    if (1 != argc)
        runtimeError("invalid arguments");
    if (IS_NUMBER(args[0])) {
        value = NUMBER_VAL(acos(AS_NUMBER(args[0])));
    }
    /*
    else if (IS_COL(args[0])) {
        ObjCol* c = factory->newCol();
        c->value = acos(AS_COL(args[0])->value);
        value = OBJ_VAL(c);
    }
    */
    else {
        runtimeError("number is expected");
    }
    return value;
}

extern "C" Value sinNative(int argc, Value* args)
{
    Value value = NIL_VAL;
    if (1 != argc)
        runtimeError("invalid arguments");
    if (IS_NUMBER(args[0])) {
        value = NUMBER_VAL(sin(AS_NUMBER(args[0])));
    }
    /*
    else if (IS_COL(args[0])) {
        ObjCol* c = factory->newCol();
        c->value = sin(AS_COL(args[0])->value);
        value = OBJ_VAL(c);
    }
    */
    else {
        runtimeError("number is expected");
    }
    return value;
}

extern "C" Value cosNative(int argc, Value* args)
{
    Value value = NIL_VAL;
    if (1 != argc)
        runtimeError("invalid arguments");
    if (IS_NUMBER(args[0])) {
        return NUMBER_VAL(cos(AS_NUMBER(args[0])));
    }
    /*
    else if (IS_COL(args[0])) {
        ObjCol* c = factory->newCol();
        c->value = cos(AS_COL(args[0])->value);
        value = OBJ_VAL(c);
    }
    */
    else {
        runtimeError("number is expected");
    }
    return value;
}

extern "C" Value tanNative(int argc, Value* args)
{
    Value value = NIL_VAL;
    if (1 != argc)
        runtimeError("invalid arguments");
    if (IS_NUMBER(args[0])) {
        return NUMBER_VAL(tan(AS_NUMBER(args[0])));
    }
    /*
    else if (IS_COL(args[0])) {
        ObjCol* c = factory->newCol();
        c->value = tan(AS_COL(args[0])->value);
        value = OBJ_VAL(c);
    }
    */
    else {
        runtimeError("number is expected");
    }
    return value;
}

extern "C" Value sqrtNative(int argc, Value* args)
{
    Value value = NIL_VAL;
    if (1 != argc)
        runtimeError("invalid arguments");
    if (IS_NUMBER(args[0])) {
        double v = AS_NUMBER(args[0]);
        if (0 <= v) {
            value = NUMBER_VAL(sqrt(AS_NUMBER(args[0])));
        }
        /*
        else {
            ObjComplex* cmplx = factory->newComplex(std::complex<double>(0, sqrt(-AS_NUMBER(args[0]))));
            value = OBJ_VAL(cmplx);
        }
        */
        else {
            runtimeError("number is expected");
        }
    }
    else {
        runtimeError("number is expected");
    }
    return value;
}

extern "C" Value cbrtNative(int argc, Value* args)
{
    Value value = NIL_VAL;
    if (1 != argc)
        runtimeError("invalid arguments");
    if (IS_NUMBER(args[0])) {
        value = NUMBER_VAL(cbrt(AS_NUMBER(args[0])));
    }
    else {
        runtimeError("number is expected");
    }
    return value;
}

extern "C" Value phaseNative(int argc, Value* args)
{
    (void)args;
    if (1 != argc)
        runtimeError("invalid arguments");
    /*
    if (IS_COMPLEX(args[0])) {
        return NUMBER_VAL(arg(AS_COMPLEX(args[0])->value));
    }
    */
    else {
        runtimeError("complex number is expected");
    }
    return NIL_VAL;
}

extern "C" Value logNative(int argc, Value* args)
{
    if (1 != argc)
        runtimeError("invalid arguments");
    Value r = NIL_VAL;
    if (IS_NUMBER(args[0])) {
        r = NUMBER_VAL(log(AS_NUMBER(args[0])));
    }
    /*
    else if (IS_COL(args[0])) {
        ObjCol* c = AS_COL(args[0]);
        ObjCol* ret = factory->newCol();
        ret->value = log(c->value);
        r = OBJ_VAL(ret);
    }
    else if (IS_MAT(args[0])) {
        ObjMat* m = AS_MAT(args[0]);
        ObjMat* ret = factory->newMat();
        ret->value = log(m->value);
        r = OBJ_VAL(ret);
    }
    */
    else {
        runtimeError("number is expected.");
    }
    return r;
}

extern "C" Value log2Native(int argc, Value* args)
{
    if (1 != argc)
        runtimeError("invalid arguments");
    Value r = NIL_VAL;
    if (IS_NUMBER(args[0])) {
        r = NUMBER_VAL(log2(AS_NUMBER(args[0])));
    }
    /*
    else if (IS_COL(args[0])) {
        ObjCol* c = AS_COL(args[0]);
        ObjCol* ret = factory->newCol();
        ret->value = log2(c->value);
        r = OBJ_VAL(ret);
    }
    else if (IS_MAT(args[0])) {
        ObjMat* m = AS_MAT(args[0]);
        ObjMat* ret = factory->newMat();
        ret->value = log2(m->value);
        r = OBJ_VAL(ret);
    }
    */
    else {
        runtimeError("number is expected.");
    }
    return r;
}

extern "C" Value log10Native(int argc, Value* args)
{
    if (1 != argc)
        runtimeError("invalid arguments");
    Value r = NIL_VAL;
    if (IS_NUMBER(args[0])) {
        r = NUMBER_VAL(log10(AS_NUMBER(args[0])));
    }
    /*
    else if (IS_COL(args[0])) {
        ObjCol* c = AS_COL(args[0]);
        ObjCol* ret = factory->newCol();
        ret->value = log10(c->value);
        r = OBJ_VAL(ret);
    }
    else if (IS_MAT(args[0])) {
        ObjMat* m = AS_MAT(args[0]);
        ObjMat* ret = factory->newMat();
        ret->value = log10(m->value);
        r = OBJ_VAL(ret);
    }
    */
    else {
        runtimeError("number is expected.");
    }
    return r;
}

extern "C" Value expNative(int argc, Value* args)
{
    if (1 != argc)
        runtimeError("invalid arguments");
    Value r = NIL_VAL;
    if (IS_NUMBER(args[0])) {
        double v = AS_NUMBER(args[0]);
        r = NUMBER_VAL(exp(v));
    }
    /*
    else if (IS_COMPLEX(args[0])) {
        ObjComplex* v = AS_COMPLEX(args[0]);
        std::complex<double> cv = exp(v->value);
        ObjComplex* rv = factory->newComplex(cv);
        r = OBJ_VAL(rv);
    }
    else if (IS_COL(args[0])) {
        ObjCol* c = AS_COL(args[0]);
        ObjCol* ret = factory->newCol();
        ret->value = exp(c->value);
        r = OBJ_VAL(ret);
    }
    else if (IS_MAT(args[0])) {
        ObjMat* m = AS_MAT(args[0]);
        ObjMat* ret = factory->newMat();
        ret->value = exp(m->value);
        r = OBJ_VAL(ret);
    }
    */
    else {
        runtimeError("number is expected.");
    }
    return r;
}

extern "C" void math_symbols(const char*** names, Value** values)
{
    static const char* math_names[] = {"acos",  "asin",  "atan", "sin", "cos", "tan", "sqrt", "cbrt",
                                       "phase", "log10", "log2", "log", "exp", "pi",  NULL};
    static Value math_syms[15];
    int cnt = 0;

    math_syms[cnt++] = OBJ_VAL(newNative(acosNative));
    math_syms[cnt++] = OBJ_VAL(newNative(asinNative));
    math_syms[cnt++] = OBJ_VAL(newNative(atanNative));
    math_syms[cnt++] = OBJ_VAL(newNative(sinNative));
    math_syms[cnt++] = OBJ_VAL(newNative(cosNative));
    math_syms[cnt++] = OBJ_VAL(newNative(tanNative));
    math_syms[cnt++] = OBJ_VAL(newNative(sqrtNative));
    math_syms[cnt++] = OBJ_VAL(newNative(cbrtNative));
    math_syms[cnt++] = OBJ_VAL(newNative(phaseNative));
    math_syms[cnt++] = OBJ_VAL(newNative(log10Native));
    math_syms[cnt++] = OBJ_VAL(newNative(log2Native));
    math_syms[cnt++] = OBJ_VAL(newNative(logNative));
    math_syms[cnt++] = OBJ_VAL(newNative(expNative));
    math_syms[cnt++] = OBJ_VAL(newNative(expNative));
    math_syms[cnt++] = NUMBER_VAL(M_PI);

    *names = math_names;
    *values = math_syms;
}
