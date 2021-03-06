#include <Luanatic/Luanatic.hpp>
#include <Stick/Test.hpp>

#include <cmath>

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
    A() :
        a(0.25f)
    {
        printf("CONSTRUCT A\n");
    }

    A(Float32 _a) :
        a(_a)
    {
        printf("CONSTRUCT A2\n");
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

Float32 printA(const A & _a)
{
    printf("I am printing an A: %f\n", _a.a);
    return _a.a;
}

Float32 printB(const B & _a)
{
    printf("I am printing an B: %f\n", _a.b);
    return _a.b;
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

struct E : public CoolClass
{
    E()
    {

    }

    E(Float32 _a, Float32 _b, Float32 _d) :
        CoolClass(_a, _b),
        d(_d)
    {

    }

    E(const E &) = default;

    virtual ~E()
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

static void printCString(const char * _str)
{
    printf("A C STRING: %s\n", _str);
}

static Int32 valueTypeOverload(CustomValueType _a)
{
    return 1;
}

static Int32 valueTypeOverload(CustomValueType _a, CustomValueType _b)
{
    return 2;
}

class WrappedClass
{
public:

    WrappedClass() = default;

    WrappedClass(const WrappedClass &) = default;

    Int32 number() const
    {
        return 45;
    }
};

template<class T>
class Wrapper
{
public:

    Wrapper() = default;

    Wrapper(const Wrapper &) = default;

    T wrpd;
};

namespace luanatic
{
    namespace detail
    {
        template <class From, class To>
        struct TypeCaster<Wrapper<From>, To>
        {
            static UserData cast(const UserData & _ud, detail::LuanaticState & _state)
            {
                return {(To *) & static_cast<Wrapper<From> *>(_ud.m_data)->wrpd, _ud.m_bOwnedByLua, stick::TypeInfoT<To>::typeID()};
            }
        };
    }
}

static Wrapper<WrappedClass> createWrappedClass()
{
    return Wrapper<WrappedClass>();
}

static stick::UniquePtr<WrappedClass> createWrappedUniquePtrClass()
{
    return stick::makeUnique<WrappedClass>(stick::defaultAllocator());
}

static stick::Result<stick::Int32> returnValidResult()
{
    return 65;
}

static stick::Result<stick::Int32> returnInvalidResult()
{
    return stick::Error(stick::ec::InvalidOperation, "Something went wrong.", STICK_FILE, STICK_LINE);
}

static stick::Variant<stick::Int32, stick::String, TestClass*> returnVariant()
{
    return 10;
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

        static stick::Int32 push(lua_State * _state, const CustomValueType & _value)
        {
            lua_newtable(_state);
            lua_pushinteger(_state, 1);
            lua_pushinteger(_state, _value.a);
            lua_settable(_state, -3);
            lua_pushinteger(_state, 2);
            lua_pushinteger(_state, _value.b);
            lua_settable(_state, -3);
            return 1;
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

//for overload testing
Int32 overloadedFunction(Int32 _a, UInt32 _b)
{
    return _a + _b;
}

Float32 overloadedFunction(Float32 _a, Float32 _b)
{
    return _a + _b;
}

const char * overloadedFunction(const char * _str)
{
    return _str;
}

const Suite spec[] =
{
    SUITE("Basic Tests")
    {
        func("test");
        if (std::is_pointer<TestClass ** *& >::value)
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
            globals.registerFunction("printCString", LUANATIC_FUNCTION(&printCString));
            String luaCode = "assert(type(myVar) == 'table') assert(myVar[1] == 1) assert(myVar[2] == 99) myVar2 = {66, 23} printCString(\"TEST YOOO!!!\") \n";

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
    },
    SUITE("Overload Prep Tests")
    {
        DynamicArray<TypeID> signature;
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

            luanatic::ClassWrapper<E> dw("E");
            dw.
            addConstructor<Float32, Float32, Float32>("new").
            addBase<CoolClass>().
            addAttribute("d", LUANATIC_ATTRIBUTE(&E::d));

            globals.
            registerClass(aw).
            registerClass(bw).
            registerClass(cw).
            registerClass(dw);

            EXPECT(luanatic::HasValueTypeConverter<stick::Int32>::value);
            EXPECT(luanatic::HasValueTypeConverter<stick::String>::value);
            EXPECT(!luanatic::HasValueTypeConverter<A>::value);
            EXPECT(luanatic::HasValueTypeConverter<stick::DynamicArray<stick::Float32>>::value);

            //Test if conversion score works properly for types that have a ValueTypeConverter implemented
            lua_newtable(state);
            lua_pushinteger(state, 1);
            lua_pushnumber(state, 0.5);
            lua_settable(state, -3);
            lua_pushinteger(state, 2);
            lua_pushnumber(state, 1.5);
            lua_settable(state, -3);
            EXPECT(lua_istable(state, -1));
            EXPECT(luanatic::conversionScore<stick::DynamicArray<stick::Float32>>(state, -1) == 1);
            lua_pop(state, 1);

            // test if overloads using value type converters are resolved as expected.
            globals.
            registerFunction("valueTypeOverload", LUANATIC_FUNCTION_OVERLOAD(int(*)(CustomValueType), &valueTypeOverload)).
            registerFunction("valueTypeOverload", LUANATIC_FUNCTION_OVERLOAD(int(*)(CustomValueType, CustomValueType), &valueTypeOverload));
            Int32 ra = globals.callFunction<Int32>("valueTypeOverload", CustomValueType());
            EXPECT(ra == 1);
            Int32 ra2 = globals.callFunction<Int32>("valueTypeOverload", CustomValueType(), CustomValueType());
            EXPECT(ra2 == 2);
            STICK_ASSERT(luanatic::HasValueTypeConverter<CustomValueType>::value);
            auto err = luanatic::execute(state, "local a = valueTypeOverload({3, 99}) assert(a == 1) local b = valueTypeOverload({3, 99}, {2, 1}) assert(b == 2)");
            EXPECT(!err);

            //Test if conversion score works properly for custom types
            EXPECT(!luanatic::HasValueTypeConverter<E>::value);
            EXPECT(std::is_copy_constructible<E>::value);
            globals["test"].set(E(0.1, 0.2, 0.3));
            luanatic::LuaValue t = globals["test"];
            t.push();
            EXPECT(lua_isuserdata(state, -1));
            EXPECT(luanatic::conversionScore<A>(state, -1) == 3);
            printf("DA SCORE %i\n", luanatic::conversionScore<B>(state, -1));
            EXPECT(luanatic::conversionScore<const B *>(state, -1) == std::numeric_limits<stick::Int32>::max());
            EXPECT(luanatic::conversionScore<CoolClass &>(state, -1) == 2);
            EXPECT(luanatic::conversionScore<const E>(state, -1) == 1);

            //test conversion scoring for basic types
            lua_pushinteger(state, 3);
            EXPECT(luanatic::conversionScore<const B *>(state, -1) == std::numeric_limits<stick::Int32>::max());
            EXPECT(luanatic::conversionScore<stick::Int32>(state, -1) == 0);
            EXPECT(luanatic::conversionScore<stick::UInt32>(state, -1) == 0);
            EXPECT(luanatic::conversionScore<stick::Float32>(state, -1) == 0);

            lua_pushinteger(state, -9);
            EXPECT(luanatic::conversionScore<stick::Int32>(state, -1) == 0);
            EXPECT(luanatic::conversionScore<stick::UInt32>(state, -1) == std::numeric_limits<stick::Int32>::max());

            lua_pushnumber(state, 3.5);
            EXPECT(luanatic::conversionScore<stick::Int32>(state, -1) == 1);
            EXPECT(luanatic::conversionScore<stick::Float32>(state, -1) == 0);
            EXPECT(luanatic::conversionScore<stick::String>(state, -1) == 1);
            EXPECT(luanatic::conversionScore<const char *>(state, -1) == 1);

            lua_pushstring(state, "3.5");
            EXPECT(luanatic::conversionScore<stick::Float32>(state, -1) == 1);
            EXPECT(luanatic::conversionScore<stick::Float64>(state, -1) == 1);
            EXPECT(luanatic::conversionScore<stick::String>(state, -1) == 0);
            EXPECT(luanatic::conversionScore<const char *>(state, -1) == 0);

            lua_pop(state, 5);
        }
        EXPECT(lua_gettop(state) == 0);
        lua_close(state);
    },
    SUITE("Overload Tests")
    {
        lua_State * state = luanatic::createLuaState();
        {
            luanatic::openStandardLibraries(state);
            luanatic::initialize(state);
            luanatic::LuaValue globals = luanatic::globalsTable(state);

            globals.registerFunction("overloadedFunction", LUANATIC_FUNCTION_OVERLOAD(Int32(*)(Int32, UInt32), &overloadedFunction));
            globals.registerFunction("overloadedFunction", LUANATIC_FUNCTION_OVERLOAD(const char * (*)(const char *), &overloadedFunction));
            //globals.registerFunction("overloadedFunction", LUANATIC_FUNCTION_OVERLOAD(Float32 (*)(Float32, Float32), &overloadedFunction));

            luanatic::ClassWrapper<A> aw("A");
            aw.
            addConstructor<>().
            addConstructor<Float32>().
            addMemberFunction("doubleA", LUANATIC_FUNCTION_OVERLOAD(void(A::*)(void), &A::doubleA)).
            addMemberFunction("doubleABy", LUANATIC_FUNCTION_OVERLOAD(void(A::*)(Float32), &A::doubleA)).
            addAttribute("a", LUANATIC_ATTRIBUTE(&A::a));

            luanatic::ClassWrapper<B> bw("B");
            bw.
            addConstructor<Float32>().
            addMemberFunction("halfB", LUANATIC_FUNCTION(&B::halfB)).
            addAttribute("b", LUANATIC_ATTRIBUTE(&B::b));

            globals.registerClass(aw);
            globals.registerClass(bw);
            globals.registerFunction("printOverload", LUANATIC_FUNCTION(&printA));
            globals.registerFunction("printOverload", LUANATIC_FUNCTION(&printB));

            String luaCode = "local a = overloadedFunction(1, 2)\n"
                             "assert(a == 3)\n"
                             "local b = overloadedFunction(\"hello world!\")\n"
                             "assert(b == \"hello world!\")\n"
                             "local c = A(2.5)\n"
                             "assert(c.a == 2.5)"
                             "local d = A()\n"
                             "assert(d.a == 0.25)\n"
                             "local e = B(1.0)\n"
                             "assert(e.b == 1.0)\n"
                             "assert(printOverload(d) == 0.25)\n"
                             "assert(printOverload(e) == 1.0)\n"
                             ;

            auto err = luanatic::execute(state, luaCode);
            if (err)
                printf("%s\n", err.message().cString());

            EXPECT(!err);
        }

        EXPECT(lua_gettop(state) == 0);
        lua_close(state);
    },
    SUITE("Default Argument Tests")
    {
        lua_State * state = luanatic::createLuaState();
        {
            luanatic::openStandardLibraries(state);
            luanatic::initialize(state);
            luanatic::LuaValue globals = luanatic::globalsTable(state);

            globals.registerFunction("overloadedFunction", LUANATIC_FUNCTION_OVERLOAD(Int32(*)(Int32, UInt32), &overloadedFunction), 1, 2);
            globals.registerFunction("overloadedFunction", LUANATIC_FUNCTION_OVERLOAD(const char * (*)(const char *), &overloadedFunction));


            String luaCode = "local a = overloadedFunction()\n"
                             "assert(a == 3)\n"
                             ;

            auto err = luanatic::execute(state, luaCode);
            if (err)
                printf("%s\n", err.message().cString());

            EXPECT(!err);

            luanatic::detail::DefaultArgs<int, float, const char *> d(1, 2.5f, "test");
            d.push(state, 3);
            EXPECT(luaL_checkinteger(state, -3) == 1);
            EXPECT(luaL_checknumber(state, -2) == 2.5);
            EXPECT(std::strcmp(luaL_checkstring(state, -1), "test") == 0);

            lua_pop(state, 3);
        }
        EXPECT(lua_gettop(state) == 0);
        lua_close(state);
    },
    SUITE("Wrapped Class Tests")
    {
        lua_State * state = luanatic::createLuaState();
        {
            luanatic::openStandardLibraries(state);
            luanatic::initialize(state);
            luanatic::LuaValue globals = luanatic::globalsTable(state);

            luanatic::ClassWrapper<WrappedClass> aw("WrappedClass");
            aw.
            addConstructor<>().
            addMemberFunction("number", LUANATIC_FUNCTION(&WrappedClass::number));

            globals.registerClass(aw);
            globals.addWrapper<WrappedClass, Wrapper<WrappedClass>>();
            globals.addWrapper<WrappedClass, stick::UniquePtr<WrappedClass>>();
            globals.registerFunction("createWrappedClass", LUANATIC_FUNCTION(&createWrappedClass));
            globals.registerFunction("createWrappedUniquePtrClass", LUANATIC_FUNCTION(&createWrappedUniquePtrClass));

            //@TODO: Write a better test...
            String luaCode = "local a = createWrappedClass()\n"
                             "local b = createWrappedUniquePtrClass()\n"
                             "assert(a) assert(b)\n";

            auto err = luanatic::execute(state, luaCode);
            if (err)
                printf("%s\n", err.message().cString());
        }
        EXPECT(lua_gettop(state) == 0);
        lua_close(state);
    },
    SUITE("Result push Tests")
    {
        lua_State * state = luanatic::createLuaState();
        {
            luanatic::openStandardLibraries(state);
            luanatic::initialize(state);
            luanatic::LuaValue globals = luanatic::globalsTable(state);

            globals.registerFunction("returnValidResult", LUANATIC_FUNCTION(&returnValidResult));
            globals.registerFunction("returnInvalidResult", LUANATIC_FUNCTION(&returnInvalidResult));

            //@TODO: Write a better test...
            String luaCode = "local a = returnValidResult()\n"
                             "assert(a == 65)\n"
                             "local b, err = returnInvalidResult()\n"
                             "print(b, err)\n"
                             "assert(not b)\n";

            auto err = luanatic::execute(state, luaCode);
            EXPECT(!err);
            if (err)
                printf("%s\n", err.message().cString());
        }
        EXPECT(lua_gettop(state) == 0);
        lua_close(state);
    },
    SUITE("Maybe Tests")
    {
        lua_State * state = luanatic::createLuaState();
        {
            luanatic::openStandardLibraries(state);
            luanatic::initialize(state);
            luanatic::LuaValue globals = luanatic::globalsTable(state);

            globals["maybe"].set(Maybe<Int32>(99));
            String luaCode = "assert(maybe == 99)\n";

            auto err = luanatic::execute(state, luaCode);
            EXPECT(!err);
            if (err)
                printf("%s\n", err.message().cString());

            auto m = globals["maybe"].get<Maybe<Int32>>();
            EXPECT(m);
            EXPECT(*m == 99);

            auto m2 = globals["nonexistent"].get<Maybe<Int32>>();
            EXPECT(!m2);
        }
        EXPECT(lua_gettop(state) == 0);
        lua_close(state);
    },
    SUITE("Variant Tests")
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
            globals.registerFunction("returnVariant", LUANATIC_FUNCTION(&returnVariant));

            TestClass tc(99);
            globals["testInstance"].set(&tc);

            //see if we can push a variant
            globals["testVariant"].set(Variant<String, TestClass*, Int32>("Hello!!!"));
            globals["testVariantTwo"].set(Variant<String, TestClass*, Int32>(&tc));

            String luaCode = "test = 'blubb'\n"
                             "assert(returnVariant() == 10)\n"
                             "assert(testInstance.val == testVariantTwo.val)\n"
                             "assert(testVariant == 'Hello!!!', 'Wrong Variant String')\n";

            auto err = luanatic::execute(state, luaCode);
            EXPECT(!err);
            if (err)
                printf("%s\n", err.message().cString());

            auto v = globals["test"].get<Variant<Int32, String, Float64>>();
            EXPECT(v.isValid());
            EXPECT(v.is<String>());
            EXPECT(v.get<String>() == "blubb");

            auto v2 = globals["nonexistent"].get<Variant<Int32, String>>();
            EXPECT(!v2.isValid());

            auto v3 = globals["testInstance"].get<Variant<Int32, TestClass*>>();
            EXPECT(v3.isValid());
            EXPECT(v3.is<TestClass*>());
            EXPECT(v3.get<TestClass*>() == &tc);
        }
        EXPECT(lua_gettop(state) == 0);
        lua_close(state);
    }
};

int main(int _argc, const char * _args[])
{
    return runTests(spec, _argc, _args);
}
