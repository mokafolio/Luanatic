#include <Luanatic/Luanatic.hpp>
#include <Stick/Test.hpp>

using namespace stick;

struct TestClass
{
    TestClass()
    {

    }

    TestClass(Int32 _val) :
        val(_val)
    {

        printf("TestClass\n");
    }

    TestClass(const TestClass & _other) :
        val(_other.val),
        other(_other.other)
    {
        printf("COPY TESTCLASS\n");
    }

    ~TestClass()
    {
        s_destructionCounter++;
    }

    void add(const TestClass & _other)
    {
        val += _other.val;
    }

    void addViaPointer(const TestClass * _other)
    {
        val += _other->val;
    }

    Int32 get() const
    {
        return val;
    }

    static void staticFunction()
    {
        s_staticFuncCounter++;
    }

    static Int32 s_destructionCounter;
    static Int32 s_staticFuncCounter;

    Int32 val;
    TestClass * other;
};

Int32 TestClass::s_destructionCounter = 0;
Int32 TestClass::s_staticFuncCounter = 0;

struct A
{
    A()
    {

    }

    A(Float32 _a) :
        a(_a)
    {

    }

    virtual ~A()
    {
        printf("DESTRUCT A\n");
    }

    void doubleA()
    {
        a *= 2.0;
    }

    void doubleA(Float32 _overloaded)
    {
        a *= _overloaded;
    }

    Float32 a;
};

struct B
{
    B()
    {

    }

    B(Float32 _a) :
        b(_a)
    {

    }

    virtual ~B()
    {
        printf("DESTRUCT B\n");
    }

    void halfB()
    {
        b *= 0.5;
    }

    Float32 b;
};

struct CoolClass : public A
{
    CoolClass()
    {

    }

    CoolClass(Float32 _a, Float32 _c) :
        A(_a),
        c(_c)
    {

    }

    virtual ~CoolClass()
    {
        printf("DESTRUCT C\n");
    }

    Float32 c;
};

void printA(const A & _a)
{
    printf("I am printing an A: %f\n", _a.a);
}

struct D : public A, public B
{
    D()
    {

    }

    D(Float32 _a, Float32 _b, Float32 _d) :
        A(_a),
        B(_b),
        d(_d)
    {

    }

    virtual ~D()
    {
        printf("DESTRUCT D\n");
    }

    Float32 d;
};

struct CustomValueType
{
    Int32 a, b;
};

static const DynamicArray<Int32> & numbers()
{
    static DynamicArray<Int32> s_numbers = {2, 4, 6, 8, 10, 12};
    return s_numbers;
}

static DynamicArray<Int32> numbersCopy()
{
    return {2, 4, 6, 8, 10, 12};
}

namespace luanatic
{
    template <>
    struct ValueTypeConverter<CustomValueType>
    {
        static CustomValueType convertAndCheck(lua_State * _state, stick::Int32 _index)
        {
            if (lua_istable(_state, -1))
            {
                Int32 a, b;
                lua_pushinteger(_state, 1);
                lua_gettable(_state, -2);
                a = luaL_checkinteger(_state, -1);
                lua_pushinteger(_state, 2);
                lua_gettable(_state, -3);
                b = luaL_checkinteger(_state, -1);
                lua_pop(_state, 2);
                return {a, b};
            }
            else
            {
                luaL_error(_state, "Table expected.");
            }
            return CustomValueType();
        }

        static void push(lua_State * _state, const CustomValueType & _value)
        {
            lua_newtable(_state);
            lua_pushinteger(_state, 1);
            lua_pushinteger(_state, _value.a);
            lua_settable(_state, -3);
            lua_pushinteger(_state, 2);
            lua_pushinteger(_state, _value.b);
            lua_settable(_state, -3);
        }
    };
}

static Float32 add(Float32 _a, Float32 _b)
{
    return _a + _b;
}

static Float32 subtract(Float32 _a, Float32 _b)
{
    return _a - _b;
}

static Int32 passThrough(const Int32 _i)
{
    return _i;
}

static bool bStaticFlag = false;

static void setStaticFlat()
{
    bStaticFlag = true;
}

struct Base
{
    virtual ~Base() {};
    virtual Int32 number() const = 0;
};

struct Derived : public Base
{
    Int32 number() const
    {
        return 3;
    }

    Int32 yoyo() const
    {
        return 4;
    }
};

struct Factory
{
    Base * makeBase()
    {
        return defaultAllocator().create<Derived>();
    }

    Derived * castToDerived(Base * _b)
    {
        return static_cast<Derived *>(_b);
    }
};

struct FirstFlag {};
struct SecondFlag {};
struct ThirdFlag {};

struct EmptyPolicy
{
    static void print()
    {
        printf("EmptyPolicy\n");
    }
};

template<class W>
struct DefaultPolicy
{
    using Target = W;

    static void print()
    {
        printf("DefaultPolicy\n");
    }
};

template<class W>
struct AnotherPolicy
{
    using Target = W;

    static void print()
    {
        printf("AnotherPolicy\n");
    }
};

template<class T>
struct Bar;

template<class T>
struct Bar<T *>
{
    static void print(T * _arg)
    {
        printf("POINTER\n");
    }
};

template<class T>
struct Bar<T &>
{
    static void print(T & _arg)
    {
        printf("REF\n");
    }
};

template<class T>
struct Bar<const T &>
{
    static void print(const T & _arg)
    {
        printf("CONST REF\n");
    }
};

template<std::size_t N>
struct Bar<const char(&)[N]>
{
    static void print(const char (&_arg)[N])
    {
        printf("C STRING\n");
    }
};

template<class T>
void func(T && _element)
{
    Bar<T>::print(std::forward<T>(_element));
}

const Suite spec[] =
{
    SUITE("Basic Tests")
    {
        func("test");
        if (std::is_pointer<TestClass ***&>::value)
            printf("WE GOT A POIIIINTER\n");
        lua_State * state = luanatic::createLuaState();
        EXPECT(lua_gettop(state) == 0);
        {
            luanatic::LuaValue globals = luanatic::globalsTable(state);
            EXPECT(globals.isValid());
            EXPECT(globals.type() == luanatic::LuaType::Table);
            EXPECT(lua_gettop(state) == 0);
            globals.push();
            EXPECT(lua_gettop(state) == 1);
            lua_pop(state, 1);

            luanatic::LuaValue registry = luanatic::registryTable(state);
            EXPECT(registry.isValid());
            EXPECT(registry.type() == luanatic::LuaType::Table);
            EXPECT(lua_gettop(state) == 0);

            EXPECT(globals.childCount() == 0);
            luanatic::LuaValue val = globals["test"];
            EXPECT(val.type() == luanatic::LuaType::Nil);
            printf("TOP %i\n", lua_gettop(state));

            val.set(1.5f);
            EXPECT(globals.childCount() == 1);
            EXPECT(val.get<Float32>() == 1.5f);
            EXPECT(globals["test"].get<Float32>() == 1.5f);

            val.set("String bro");
            // EXPECT(val.get<String>() == "String bro");
            // val.reset();
            // printf("TOP2 %i\n", lua_gettop(state));
            // EXPECT(lua_gettop(state) == 0);

            // luanatic::LuaValue testTable = globals.findOrCreateTable("testTable");
            // EXPECT(testTable.type() == luanatic::LuaType::Table);
            // EXPECT(globals.childCount() == 2);

            // luanatic::LuaValue testTableRef2 = globals.getChild("testTable");
            // EXPECT(testTableRef2.type() == luanatic::LuaType::Table);
            // EXPECT(testTable.childCount() == 0);
            // testTable["blubb"].set(5.0f);
            // EXPECT(testTableRef2["blubb"].get<Float32>() == 5.0f);
            // EXPECT(testTable.childCount() == 1);
        }
        EXPECT(lua_gettop(state) == 0);
        {
            luanatic::LuaValue globals = luanatic::globalsTable(state);
            luanatic::initialize(state);
            EXPECT(lua_gettop(state) == 0);

            luanatic::LuaValue luanaticNamespace = globals.getChild("luanatic");
            EXPECT(luanaticNamespace.isValid());
            EXPECT(luanaticNamespace.type() == luanatic::LuaType::Table);

            luanatic::LuaValue classFunc = luanaticNamespace["class"];
            EXPECT(classFunc.type() == luanatic::LuaType::Function);
            luanatic::LuaValue iboFunc = luanaticNamespace["isBaseOf"];
            EXPECT(iboFunc.type() == luanatic::LuaType::Function);
            luanatic::LuaValue iioFunc = luanaticNamespace["isInstanceOf"];
            EXPECT(iioFunc.type() == luanatic::LuaType::Function);

        }
        EXPECT(lua_gettop(state) == 0);
        printf("ARGH %i\n", lua_gettop(state));
        lua_close(state);
    },
    SUITE("Function Tests")
    {
        lua_State * state = luanatic::createLuaState();
        {
            luanatic::initialize(state);
            luanatic::LuaValue globals = luanatic::globalsTable(state);
            globals.
            registerFunction("add", LUANATIC_FUNCTION(&add)).
            registerFunction("subtract", LUANATIC_FUNCTION(&subtract)).
            registerFunction("passThrough", LUANATIC_FUNCTION(&passThrough)).
            registerFunction("setStaticFlat", LUANATIC_FUNCTION(&setStaticFlat));
            EXPECT(lua_gettop(state) == 0);

            Float32 res = globals.callFunction<Float32>("add", 1.0f, 1.5f);
            EXPECT(res == 2.5f);

            EXPECT(lua_gettop(state) == 0);

            Float32 res2 = globals.callFunction<Float32>("subtract", 5.0f, 1.5f);
            printf("%f\n", res2);
            EXPECT(res2 == 3.5f);

            EXPECT(lua_gettop(state) == 0);

            Int32 res3 = globals.callFunction<Int32>("passThrough", 123);
            EXPECT(res3 == 123);

            EXPECT(lua_gettop(state) == 0);

            globals.callFunction<void>("setStaticFlat");
            EXPECT(bStaticFlag);
            EXPECT(lua_gettop(state) == 0);
        }
        lua_close(state);
    },
    SUITE("Basic Class Tests")
    {
        lua_State * state = luanatic::createLuaState();
        {
            luanatic::openStandardLibraries(state);
            luanatic::initialize(state);
            luanatic::LuaValue globals = luanatic::globalsTable(state);

            luanatic::ClassWrapper<TestClass> tw("TestClass");
            tw.
            addConstructor<Int32>("new").
            addMemberFunction("add", LUANATIC_FUNCTION(&TestClass::add)).
            addMemberFunction("addViaPointer", LUANATIC_FUNCTION(&TestClass::addViaPointer)).
            addMemberFunction("get", LUANATIC_FUNCTION(&TestClass::get)).
            addAttribute("val", LUANATIC_ATTRIBUTE(&TestClass::val)).
            addAttribute("other", LUANATIC_ATTRIBUTE(&TestClass::other)).
            addStaticFunction("staticFunction", LUANATIC_FUNCTION(&TestClass::staticFunction));

            globals.registerClass(tw);

            EXPECT(TestClass::s_destructionCounter == 0);
            TestClass * obj = defaultAllocator().create<TestClass>(3);
            globals["someVariable"].set(obj, luanatic::Transfer<luanatic::ph::Result>());

            EXPECT(TestClass::s_destructionCounter == 0);
            String luaCode = "local anotherVar = TestClass.new(1)\n"
                             "assert(anotherVar:get() == 1) assert(someVariable:get() == 3)\n"
                             "anotherVar:add(someVariable) assert(anotherVar:get() == 4) assert(anotherVar.val == 4)\n"
                             "anotherVar:addViaPointer(anotherVar) assert(anotherVar:get() == 8) assert(anotherVar.val == 8)\n"
                             "anotherVar.val = 10 assert(anotherVar:get() == 10)"
                             "anotherVar.other = someVariable\n"
                             "anotherVar.other:add(someVariable)\n"
                             "assert(anotherVar.other.val == 6)\n"
                             "TestClass.staticFunction()";

            auto err = luanatic::execute(state, luaCode);
            if (err)
                printf("%s\n", err.message().cString());
            EXPECT(!err);

            // auto sref = TestClass
        }
        EXPECT(lua_gettop(state) == 0);
        lua_close(state);
        printf("DC BABY: %i\n", TestClass::s_destructionCounter);
        EXPECT(TestClass::s_destructionCounter == 2);
        EXPECT(TestClass::s_staticFuncCounter == 1);
    },
    SUITE("Inheritance Tests")
    {
        lua_State * state = luanatic::createLuaState();
        {
            luanatic::openStandardLibraries(state);
            luanatic::initialize(state);
            luanatic::LuaValue globals = luanatic::globalsTable(state);

            luanatic::ClassWrapper<A> aw("A");
            aw.
            addConstructor<Float32>("new").
            addMemberFunction("doubleA", LUANATIC_FUNCTION_OVERLOAD(void(A::*)(void), &A::doubleA)).
            addMemberFunction("doubleABy", LUANATIC_FUNCTION_OVERLOAD(void(A::*)(Float32), &A::doubleA)).
            addAttribute("a", LUANATIC_ATTRIBUTE(&A::a));

            luanatic::ClassWrapper<B> bw("B");
            bw.
            addConstructor<Float32>("new").
            addMemberFunction("halfB", LUANATIC_FUNCTION(&B::halfB)).
            addAttribute("b", LUANATIC_ATTRIBUTE(&B::b));

            luanatic::ClassWrapper<CoolClass> cw("CoolClass");
            cw.
            addBase<A>().
            addConstructor<Float32, Float32>("new").
            addAttribute("c", LUANATIC_ATTRIBUTE(&CoolClass::c));

            luanatic::ClassWrapper<D> dw("D");
            dw.
            addConstructor<Float32, Float32, Float32>("new").
            addBase<A>().
            addBase<B>().
            addAttribute("d", LUANATIC_ATTRIBUTE(&D::d));

            globals.
            registerClass(aw).
            registerClass(bw).
            registerClass(cw).
            registerClass(dw).
            registerFunction("printA", LUANATIC_FUNCTION(&printA));

            String luaCode = "local anotherVar = CoolClass.new(1.5, 2.5)\n"
                             "assert(luanatic.isInstanceOf(anotherVar, A))\n"
                             "assert(luanatic.isInstanceOf(anotherVar, CoolClass))\n"
                             "print(anotherVar.a, anotherVar.c)\n"
                             "assert(anotherVar.c == 2.5)\n"
                             "assert(anotherVar.a == 1.5)\n"
                             "anotherVar:doubleA()\n"
                             "assert(anotherVar.a == 3.0)\n"
                             "local dvar = D.new(0.1, 0.2, 0.3)\n"
                             "assert(luanatic.isInstanceOf(dvar, A))\n"
                             "assert(luanatic.isInstanceOf(dvar, B))\n"
                             "local epsilon = 0.0000001\n"
                             "assert(math.abs(dvar.a - 0.1) < epsilon)\n"
                             "assert(math.abs(dvar.b - 0.2) < epsilon)\n"
                             "assert(math.abs(dvar.d - 0.3) < epsilon)\n"
                             "dvar:doubleABy(20.0) print('pre assert') assert(math.abs(dvar.a - 2.0) < epsilon) print('post assert')\n"
                             "printA(dvar)\n";

            auto err = luanatic::execute(state, luaCode);
            if (err)
                printf("%s\n", err.message().cString());
            EXPECT(!err);
        }
        {
            using namespace luanatic;
            LuaValue globals = luanatic::globalsTable(state);

            ClassWrapper<Base> bw("Base");
            bw.
            addMemberFunction("number", LUANATIC_FUNCTION(&Base::number));

            ClassWrapper<Derived> dw("Derived");
            dw.
            addBase<Base>().
            addMemberFunction("yoyo", LUANATIC_FUNCTION(&Derived::yoyo));

            ClassWrapper<Factory> fw("Factory");
            fw.
            addConstructor<>("new").
            addMemberFunction("makeBase", LUANATIC_FUNCTION(&Factory::makeBase, Transfer<ph::Result>)).
            //Derived * castToDerived(Base * _b)
            addMemberFunction("castToDerived", LUANATIC_FUNCTION(&Factory::castToDerived, Transfer<ph::Result>));

            globals.
            registerClass(bw).
            registerClass(dw).
            registerClass(fw);

            String luaCode = "local factory = Factory.new()\n"
                             "local base = factory:makeBase()\n"
                             "assert(base:number() == 3.0)\n"
                             "local derived = factory:castToDerived(base)\n"
                             "assert(derived:number() == 3.0)\n"
                             "assert(derived:yoyo() == 4.0)\n";
            auto err = luanatic::execute(state, luaCode);
            if (err)
                printf("%s\n", err.message().cString());
            EXPECT(!err);
        }
        EXPECT(lua_gettop(state) == 0);
        lua_close(state);
    },
    SUITE("ValueTypeConverter Tests")
    {
        //This test tests using a custom luanatic::ValueTypeConverter to convert between the type
        //and a lua table, aswell as the other way around.
        lua_State * state = luanatic::createLuaState();
        {
            luanatic::openStandardLibraries(state);
            luanatic::initialize(state);
            luanatic::LuaValue globals = luanatic::globalsTable(state);

            CustomValueType cvt = {1, 99};
            globals["myVar"].set(cvt);
            String luaCode = "assert(type(myVar) == 'table') assert(myVar[1] == 1) assert(myVar[2] == 99) myVar2 = {66, 23}\n";

            auto err = luanatic::execute(state, luaCode);
            if (err)
                printf("%s\n", err.message().cString());

            EXPECT(!err);

            CustomValueType cvt2 = globals["myVar2"].get<CustomValueType>();
            EXPECT(cvt2.a == 66);
            EXPECT(cvt2.b == 23);
        }
        EXPECT(lua_gettop(state) == 0);
        lua_close(state);
    },
    SUITE("Iterator Tests")
    {
        lua_State * state = luanatic::createLuaState();
        {
            luanatic::openStandardLibraries(state);
            luanatic::initialize(state);
            luanatic::LuaValue globals = luanatic::globalsTable(state);

            globals.registerFunction("numbers", LUANATIC_FUNCTION(&numbers, luanatic::ReturnIterator<luanatic::ph::Result>));
            globals.registerFunction("numbersCopy", LUANATIC_FUNCTION(&numbersCopy));
            //check if the number sequence iteration works as expected
            String luaCode = "local i = 1 for obj in numbers() do assert(obj == i*2) i = i + 1 end\n"
                             "local expected = {2, 4, 6, 8, 10, 12}\n"
                             "local nmbs = numbersCopy()\n"
                             "assert(#nmbs == 6)"
                             "for k,v in pairs(nmbs) do assert(expected[k] == v) end\n"; //this should convert the numbers to a lua table

            auto err = luanatic::execute(state, luaCode);
            if (err)
                printf("%s\n", err.message().cString());

            EXPECT(!err);
        }
        EXPECT(lua_gettop(state) == 0);
        lua_close(state);
    }
};

int main(int _argc, const char * _args[])
{
    return runTests(spec, _argc, _args);
}
