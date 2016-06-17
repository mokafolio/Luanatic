#ifndef LUANATIC_LUANATIC_HPP
#define LUANATIC_LUANATIC_HPP

#include <Stick/DynamicArray.hpp>
#include <Stick/Error.hpp>
#include <Stick/HashMap.hpp>
#include <Stick/StringConversion.hpp>
#include <Stick/TypeInfo.hpp>
#include <Stick/UniquePtr.hpp>

#include <type_traits>
#include <functional> //for std::ref

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

#define LUANATIC_KEY "__Luanatic"
#define LUANATIC_FUNCTION(x) luanatic::detail::FunctionWrapper<decltype(x), x>::func
#define LUANATIC_FUNCTION_OVERLOAD(sig, x) &luanatic::detail::FunctionWrapper<sig, x>::func //to manually specify the signature
#define LUANATIC_ATTRIBUTE(x) luanatic::detail::AttributeWrapper<decltype(x), x>::func
#define LUANATIC_RETURN_ITERATOR(x) luanatic::detail::IteratorWrapper<decltype(x), x, false>::func
#define LUANATIC_RETURN_REF_ITERATOR(x) luanatic::detail::IteratorWrapper<decltype(x), x, true>::func
#define LUANATIC_RETURN_ITERATOR_OVERLOAD(sig, x) luanatic::detail::IteratorWrapper<sig, x, false>::func
#define LUANATIC_RETURN_REF_ITERATOR_OVERLOAD(sig, x) luanatic::detail::IteratorWrapper<sig, x, true>::func

namespace luanatic
{
    class LuaValue;

    inline lua_State * createLuaState();

    inline void initialize(lua_State * _state, stick::Allocator & _allocator = stick::defaultAllocator());

    inline void openStandardLibraries(lua_State * _state);

    inline LuaValue globalsTable(lua_State * _state);

    inline LuaValue registryTable(lua_State * _state);

    inline stick::Error execute(lua_State * _state, const stick::String & _luaCode);

    inline void addPackagePath(lua_State * _state, const stick::String & _path);

    template <class T>
    inline bool isOfType(lua_State * _luaState, stick::Int32 _index, bool _bStrict = false);

    template <class T>
    inline T * convertToType(lua_State * _luaState, stick::Int32 _index, bool _bStrict = false);

    template <class T>
    inline T * convertToTypeAndCheck(lua_State * _luaState, stick::Int32 _index, bool _bStrict = false);

    template <class T>
    inline T convertToValueTypeAndCheck(lua_State * _luaState, stick::Int32 _index);

    template <class T, class WT>
    inline bool pushWrapped(lua_State * _luaState, const WT * _obj, bool _bLuaOwnsObject = true);

    template <class T>
    inline bool push(lua_State * _luaState, const T * _obj, bool _bLuaOwnsObject = true);

    template <class T>
    inline void pushValueType(lua_State * _luaState, const T & _value);

    template <class T, class ... Args>
    inline void constructUnregisteredType(lua_State * _luaState, Args ... _args);

    template <class T>
    inline void pushUnregisteredType(lua_State * _luaState, const T & _value);


    struct STICK_API Nil
    {
    };

    enum class STICK_API LuaType
    {
        None,
        Nil,
        Boolean,
        Number,
        String,
        UserData,
        Function,
        Thread,
        Table
    };

    //lua operator names
    const char * EqualOperatorFlag = "__eq";
    const char * LessEqualOperatorFlag = "__le";
    const char * LessOperatorFlag = "__lt";
    const char * AdditionOperatorFlag = "__add";
    const char * SubtractionOperatorFlag = "__sub";
    const char * MultiplicationOperatorFlag = "__mul";
    const char * DivisionOperatorFlag = "__div";
    const char * ToStringFlag = "__tostring";

    class LuaValue;

    namespace detail
    {
        inline stick::Size rawLen(lua_State * _state, int _index);

        inline lua_Debug debugInfo(lua_State * _state, int _level)
        {
            lua_Debug ar;
            lua_getstack(_state, _level, &ar);
            lua_getinfo(_state, "nSl", &ar);
            return ar;
        }

        template<class...Args>
        inline void luaErrorWithStackTrace(lua_State * _state, int _index, const char * _fmt, Args ... _args)
        {
            luaL_traceback(_state, _state, NULL, _index);
            printf("%s\n\n", lua_tostring(_state, -1));
            luaL_error(_state, _fmt, _args...);
        }
    }

    template <class U, class Enable = void>
    struct ValueTypeConverter
    {
        static U convertAndCheck(lua_State * _state, stick::Int32 _index)
        {
            detail::luaErrorWithStackTrace(_state, _index, "No ValueTypeConverter implementation found.");
            return U();
        }

        //implemented further down, as we need detail::LuanaticState for memory allocation
        static void push(lua_State * _state, const U & _value);
    };

    template <>
    struct ValueTypeConverter<bool>
    {
        static bool convertAndCheck(lua_State * _state, stick::Int32 _index)
        {
            return lua_toboolean(_state, _index) == 1;
        }

        static void push(lua_State * _state, const bool & _value)
        {
            lua_pushboolean(_state, _value);
        }
    };

    template <>
    struct ValueTypeConverter<stick::Int32>
    {
        static stick::Int32 convertAndCheck(lua_State * _luaState,
                                            stick::Int32 _index)
        {
            return static_cast<stick::Int32>(
                       luaL_checkinteger(_luaState, _index));
        }

        static void push(lua_State * _luaState, const stick::Int32 & _value)
        {
            lua_pushinteger(_luaState, _value);
        }
    };

    template <>
    struct ValueTypeConverter<stick::Int16>
    {
        static stick::Int16 convertAndCheck(lua_State * _luaState,
                                            stick::Int32 _index)
        {
            return static_cast<stick::Int32>(
                       luaL_checkinteger(_luaState, _index));
        }

        static void push(lua_State * _luaState, const stick::Int16 & _value)
        {
            lua_pushinteger(_luaState, _value);
        }
    };

    template <>
    struct ValueTypeConverter<stick::UInt32>
    {
        static stick::UInt32 convertAndCheck(lua_State * _luaState,
                                             stick::Int32 _index)
        {
            return static_cast<stick::UInt32>(luaL_checkinteger(_luaState, _index));
        }

        static void push(lua_State * _luaState, const stick::UInt32 & _value)
        {
            lua_pushinteger(_luaState, _value);
        }
    };

    template <>
    struct ValueTypeConverter<stick::Size>
    {
        static stick::Size convertAndCheck(lua_State * _luaState,
                                           stick::Int32 _index)
        {
            return static_cast<stick::Size>(
                       luaL_checkinteger(_luaState, _index));
        }

        static void push(lua_State * _luaState, const stick::Size & _value)
        {
            lua_pushinteger(_luaState, _value);
        }
    };

    template <>
    struct ValueTypeConverter<stick::Float32>
    {
        static stick::Float32 convertAndCheck(lua_State * _luaState, stick::Int32 _index)
        {
            return static_cast<stick::Float32>(luaL_checknumber(_luaState, _index));
        }

        static void push(lua_State * _luaState, const stick::Float32 & _number)
        {
            lua_pushnumber(_luaState, _number);
        }
    };

    template <>
    struct ValueTypeConverter<stick::Float64>
    {
        static stick::Float64 convertAndCheck(lua_State * _luaState,
                                              stick::Int32 _index)
        {
            return static_cast<stick::Float64>(luaL_checknumber(_luaState, _index));
        }

        static void push(lua_State * _luaState, const stick::Float64 & _number)
        {
            lua_pushnumber(_luaState, _number);
        }
    };

    template <>
    struct ValueTypeConverter<stick::String>
    {
        static stick::String convertAndCheck(lua_State * _luaState,
                                             stick::Int32 _index)
        {
            return static_cast<stick::String>(luaL_checkstring(_luaState, _index));
        }

        static void push(lua_State * _luaState, const stick::String & _str)
        {
            lua_pushstring(_luaState, _str.cString());
        }
    };

    template <typename T>
    struct ValueTypeConverter <T, typename std::enable_if<std::is_enum<T>::value>::type >
    {
        static T convertAndCheck(lua_State * _luaState, stick::Int32 _index)
        {
            return static_cast<T>(luaL_checkinteger(_luaState, _index));
        }

        static void push(lua_State * _luaState, const T & _value)
        {
            lua_pushnumber(_luaState, static_cast<stick::Int32>(_value));
        }
    };

    //to convert std::vector to lua table and vice versa
    template<class T>
    struct ValueTypeConverter<stick::DynamicArray<T> >
    {
        static stick::DynamicArray<T> convertAndCheck(lua_State * _luaState, stick::Int32 _index)
        {
            stick::DynamicArray<T> ret;

            if (lua_istable(_luaState, _index))
            {
                auto len = detail::rawLen(_luaState, _index);
                if (len <= 0)
                {
                    return ret;
                }
                ret.reserve(detail::rawLen(_luaState, _index));
                for (lua_pushnil(_luaState);
                        lua_next(_luaState, _index);
                        lua_pop(_luaState, 1))
                {
                    ret.append(convertToValueTypeAndCheck<T>(_luaState, -1));
                }
            }
            else
            {
                const char * msg = lua_pushfstring(_luaState, "Table expected, got %s", luaL_typename(_luaState, _index));
                luaL_argerror(_luaState, _index, msg);
            }

            return ret;
        }

        static void push(lua_State * _luaState, const stick::DynamicArray<T> & _array)
        {
            lua_newtable(_luaState);
            auto it = _array.begin();
            stick::Int32 i = 1;
            for (; it != _array.end(); ++it, ++i)
            {
                lua_pushinteger(_luaState, i);
                pushValueType<T>(_luaState, *it);
                lua_settable(_luaState, -3);
            }
        }
    };

    //Value Converter for stick::Error
    template<>
    struct ValueTypeConverter<stick::Error>
    {
        static stick::Error convertAndCheck(lua_State * _state, stick::Int32 _index)
        {
            return stick::Error();
        }

        static void push(lua_State * _state, const stick::Error & _error)
        {
            if (!_error)
            {
                lua_pushnil(_state);
            }
            else
            {
                lua_newtable(_state);
                lua_pushstring(_state, _error.message().cString());
                lua_setfield(_state, -2, "message");
                lua_pushstring(_state, _error.description().cString());
                lua_setfield(_state, -2, "description");
                lua_pushstring(_state, _error.file().cString());
                lua_setfield(_state, -2, "file");
                lua_pushinteger(_state, _error.line());
                lua_setfield(_state, -2, "line");
                lua_pushinteger(_state, _error.code());
                lua_setfield(_state, -2, "code");
            }
        }
    };

    /*
    template<class T>
    struct ValueTypeConverter<stick::DynamicArray<T *> >
    {
        static stick::DynamicArray<T> convertAndCheck(lua_State * _luaState, stick::Int32 _index)
        {
            stick::DynamicArray<T> ret;

            if (lua_istable(_luaState, _index))
            {
                ret.reserve(detail::rawLen(_luaState, _index));
                for (lua_pushnil(_luaState); lua_next(_luaState, _index); lua_pop(_luaState, 1))
                {
                    ret.push_back(convertToTypeAndCheck<T>(_luaState, -1));
                }
            }
            else
            {
                const char * msg = lua_pushfstring(_luaState, "Table expected, got %s", luaL_typename(_luaState, _index));
                luaL_argerror(_luaState, _index, msg);
            }

            return ret;
        }

        static void push(lua_State * _luaState, const stick::DynamicArray<T> & _array)
        {
            lua_newtable(_luaState);
            auto it = _array.begin();
            core::Int32 i = 1;
            for (; it != _array.end(); ++it, ++i)
            {
                lua_pushinteger(_luaState, i);
                pushToLua<T>(_luaState, *it);
                lua_settable(_luaState, -3);
            }
        }
    };*/

    namespace detail
    {
        struct LuanaticState;

        struct Pass
        {
            template <class... Args>
            Pass(Args...)
            {
                // nothing to do here, we need this proxy only for the variadic
                // pack
                // expansion to compile
            }
        };

        // RAII style helper to pop the lua stack
        class STICK_LOCAL StackPop
        {
        public:
            StackPop(lua_State * _luaState, stick::UInt32 _count)
                : m_state(_luaState)
                , m_count(_count)
            {
            }

            ~StackPop() { lua_pop(m_state, m_count); }

        private:
            lua_State * m_state;
            stick::UInt32 m_count;
        };

        struct STICK_LOCAL UserData
        {
            void * m_data;
            bool m_bOwnedByLua;
            stick::TypeID m_typeID;
        };

        struct STICK_LOCAL UserDataCastFunction
        {
            using CastFunction = UserData (*)(const UserData &, LuanaticState &);

            UserDataCastFunction(stick::TypeID _typeID, CastFunction _cast)
                : m_typeID(_typeID)
                , m_cast(_cast)
            {
            }

            stick::TypeID m_typeID;
            CastFunction m_cast;
        };

        template <class From, class To>
        struct TypeCaster
        {
            static UserData cast(const UserData & _ud, detail::LuanaticState & _state)
            {
                return {static_cast<To *>(static_cast<From *>(_ud.m_data)), _ud.m_bOwnedByLua, stick::TypeInfoT<To>::typeID()};
            }
        };
    }

    class ClassWrapperBase;
    using ClassWrapperUniquePtr = stick::UniquePtr<ClassWrapperBase>;

    class STICK_API ClassWrapperBase
    {
    public:
        using BaseArray = stick::DynamicArray<stick::TypeID>;
        using CastArray = stick::DynamicArray<detail::UserDataCastFunction>;

        struct NamedLuaFunction
        {
            stick::String name;
            lua_CFunction function;
        };

        using NamedLuaFunctionArray = stick::DynamicArray<NamedLuaFunction>;
        using TypeIDArray = stick::DynamicArray<stick::TypeID>;

        ClassWrapperBase(stick::TypeID _typeID, const stick::String & _className)
            : m_typeID(_typeID)
            , m_className(_className)
        {
        }

        ClassWrapperBase(const ClassWrapperBase & _other)
            : m_typeID(_other.m_typeID)
            , m_className(_other.m_className)
            , m_members(_other.m_members)
            , m_casts(_other.m_casts)
            , m_bases(_other.m_bases)
            , m_statics(_other.m_statics)
            , m_attributes(_other.m_attributes)
        {
        }

        ClassWrapperBase & operator=(const ClassWrapperBase & _other)
        {
            m_typeID = _other.m_typeID;
            m_className = _other.m_className;
            m_members = _other.m_members;
            m_statics = _other.m_statics;
            m_attributes = _other.m_attributes;
            m_bases = _other.m_bases;
            m_casts = _other.m_casts;

            return *this;
        }

        virtual ~ClassWrapperBase() {}

        ClassWrapperBase & addMemberFunction(const stick::String & _name,
                                             lua_CFunction _function)
        {
            m_members.append({ _name, _function });
            return *this;
        }

        ClassWrapperBase & addStaticFunction(const stick::String & _name,
                                             lua_CFunction _function)
        {
            m_statics.append({ _name, _function });
            return *this;
        }

        ClassWrapperBase & addAttribute(const stick::String & _name,
                                        lua_CFunction _function)
        {
            m_attributes.append({ _name, _function });
            return *this;
        }

        void addCast(const detail::UserDataCastFunction & _cast)
        {
            m_casts.append(_cast);
        }

        stick::TypeID typeID() const { return m_typeID; }

        virtual ClassWrapperUniquePtr clone() const = 0;

        // detail::ImplicitConverterUniquePtr m_implicitConverter;
        stick::TypeID m_typeID;
        stick::String m_className;
        NamedLuaFunctionArray m_members;
        NamedLuaFunctionArray m_statics;
        NamedLuaFunctionArray m_attributes;
        BaseArray m_bases;
        CastArray m_casts;
    };

    template <class T>
    class ClassWrapper : public ClassWrapperBase
    {
    public:
        using ClassType = T;

        ClassWrapper(const stick::String & _name);

        template <class... Args>
        ClassWrapper & addConstructor(const stick::String & _str);

        ClassWrapperUniquePtr clone() const;

        template <class CD>
        ClassWrapper & addBase();

        template <class CD>
        ClassWrapper & addCast();
    };

    namespace detail
    {
        struct STICK_LOCAL LuanaticState
        {
            struct WrappedClass
            {
                ClassWrapperUniquePtr wrapper;
                stick::Int32 namespaceIndex;
            };

            LuanaticState(stick::Allocator & _allocator) :
                m_allocator(&_allocator)
            {

            }

            stick::Allocator * m_allocator;
            stick::HashMap<stick::TypeID, WrappedClass> m_typeIDClassMap;
        };

        inline LuanaticState * luanaticState(lua_State * _luaState)
        {
            lua_getfield(_luaState, LUA_REGISTRYINDEX, LUANATIC_KEY);
            if (!lua_isnil(_luaState, -1))
            {
                lua_getfield(_luaState, -1, "LuanaticState");
                if (lua_isuserdata(_luaState, -1))
                {
                    LuanaticState * ret = static_cast<LuanaticState *>(lua_touserdata(_luaState, -1));
                    lua_pop(_luaState, 2);
                    return ret;
                }
            }
            else
                lua_pop(_luaState, 1);

            lua_pushstring(_luaState, "No Luanatic state associated with the provided lua_State!");
            lua_error(_luaState);
            return nullptr;
        }

        template <class T>
        struct ObjectIdentifier
        {
            static void identify(lua_State * _luaState, const T * _obj)
            {
                lua_pushlightuserdata(_luaState, (void *)_obj);
            }
        };

        template <class T>
        inline bool checkBases(LuanaticState & _luanaticState,
                               stick::TypeID _tid)
        {
            bool ret = false;
            auto it = _luanaticState.m_typeIDClassMap.find(_tid);
            if (it != _luanaticState.m_typeIDClassMap.end())
            {
                auto & casts = _luanaticState.m_typeIDClassMap[_tid].wrapper->m_casts;
                auto it = casts.begin();

                for (; it != casts.end(); ++it)
                {
                    if ((*it).m_typeID == stick::TypeInfoT<T>::typeID())
                        return true;

                    ret = checkBases<T>(_luanaticState, (*it).m_typeID);
                    if (ret)
                        break;
                }
            }

            return ret;
        }

        struct FindCastFunctionResult
        {
            bool bFound;
            UserData userData;
        };

        inline FindCastFunctionResult findCastFunction(LuanaticState & _luanaticState, const UserData & _currentUserData, stick::TypeID _targetTypeID)
        {
            UserData ret = _currentUserData;
            auto & casts = _luanaticState.m_typeIDClassMap[ret.m_typeID].wrapper->m_casts;
            auto it = casts.begin();

            // check direct bases
            for (; it != casts.end(); ++it)
            {
                ret = (*it).m_cast(_currentUserData, _luanaticState);
                if (ret.m_typeID == _targetTypeID)
                {
                    return { true, ret };
                }
            }

            it = casts.begin();

            // recursively walk the inheritance tree
            for (; it != casts.end(); ++it)
            {
                ret = (*it).m_cast(_currentUserData, _luanaticState);
                ret = findCastFunction(_luanaticState, ret, _targetTypeID).userData;
                if (ret.m_typeID == _targetTypeID)
                    return { true, ret };
            }

            return { false, ret };
        }

        inline void pushGlobalsTable(lua_State * _state)
        {
#if LUA_VERSION_NUM >= 502
            lua_pushglobaltable(_state);
#else
            lua_pushvalue(_state, LUA_GLOBALSINDEX);
#endif // LUA_VERSION_NUM >= 502
        }

        // wrapper around either lua_objlen (5.1) or lua_rawlen(5.2)
        inline stick::Size rawLen(lua_State * _state, int _index)
        {
#if LUA_VERSION_NUM >= 502
            return lua_rawlen(_state, _index);
#else
            return lua_objlen(_state, _index);
#endif // LUA_VERSION_NUM >= 502
        }

        void registerFunctions(lua_State * _luaState, stick::Int32 _targetTableIndex,
                               const ClassWrapperBase::NamedLuaFunctionArray & _functions,
                               const stick::String & _nameReplace = "",
                               bool _bNiceConstructor = false)
        {
            STICK_ASSERT(lua_istable(_luaState, _targetTableIndex));
            STICK_ASSERT(lua_istable(_luaState, -1));
            auto it = _functions.begin();
            const char * name;
            for (; it != _functions.end(); ++it)
            {
                name = _nameReplace.length() ? _nameReplace.cString()
                       : (*it).name.cString();
                lua_getfield(_luaState, -1, name); // ... CT mT mT[name]
                if (lua_isnil(_luaState, -1))
                {
                    lua_pushstring(_luaState, name); // ... CT mT nil name
                    lua_pushcfunction(_luaState, (*it).function); // ... CT mT nil name func
                    lua_settable(_luaState, -4); // ... CT mT nil
                    lua_pop(_luaState, 1); // ... CT mT
                }
                else
                {
                    // TODO: error
                }
            }
        }

        template <class T>
        inline stick::Int32 newIndex(lua_State * _luaState);

        template <class T>
        inline stick::Int32 index(lua_State * _luaState);

        template <class T>
        stick::Int32 destruct(lua_State * _luaState);

        template <class T>
        inline stick::Int32 destructUnregistered(lua_State * _luaState);


        template<bool bHasDestructor>
        struct DestructorRegistration;

        template<>
        struct DestructorRegistration<false>
        {
            template<class T>
            static void registerDestructor(lua_State * _state, stick::Int32 _classTableIndex)
            {

            }
        };

        template<>
        struct DestructorRegistration<true>
        {
            template<class T>
            static void registerDestructor(lua_State * _state, stick::Int32 _classTableIndex)
            {
                lua_pushliteral(_state, "__gc");                // ... CT mT __gc
                lua_pushcfunction(_state, destruct<T>); // ... CT mT  __gc gfunc
                lua_settable(_state, _classTableIndex);               // ... CT mT
            }
        };

        template <class CW, bool bHasDestructor = true>
        inline void registerClass(lua_State * _state, const CW & _wrapper)
        {
            using ClassType = typename CW::ClassType;

            //NOTE: Call this function with the namespace table that you want to
            //register the class to on top of the stack.

            LuanaticState * state = luanaticState(_state);
            STICK_ASSERT(state != nullptr);

            ClassWrapperUniquePtr cl(state->m_allocator->create<CW>(_wrapper));
            ClassWrapperBase * rep = cl.get();

            //make sure all the bases are registered
            auto it = _wrapper.m_bases.begin();
            for (; it != _wrapper.m_bases.end(); ++it)
            {
                auto mit = state->m_typeIDClassMap.find((*it));

                if (mit == state->m_typeIDClassMap.end())
                {
                    luaL_error(_state, "attempting to extend a type that has not been registered");
                }
            }

            //cl->m_bases = bases;
            stick::TypeID myTypeID = cl->m_typeID;
            lua_pushvalue(_state, -1);
            state->m_typeIDClassMap[_wrapper.m_typeID] = {std::move(cl), luaL_ref(_state, LUA_REGISTRYINDEX)};

            //the class table
            lua_newtable(_state); // ... CT
            stick::Int32 classTable = lua_gettop(_state);
            lua_pushstring(_state, _wrapper.m_className.cString()); // ... CT "className"
            lua_pushvalue(_state, classTable);                      // ... CT "className" CT
            lua_settable(_state, -4);                               // ... CT

            //register static functions in class table
            registerFunctions(_state, classTable, _wrapper.m_statics);

            lua_pushliteral(_state, "__typeID");            // ... CT __typeID
            lua_pushinteger(_state, (stick::Size)myTypeID); // ... CT __typeID id
            lua_settable(_state, classTable);               // ... CT

            lua_pushliteral(_state, "__index");             // ... CT mT __index
            lua_pushcfunction(_state, index<ClassType>);    // ... CT mT __index ifunc
            lua_settable(_state, classTable);               // ... CT mT
            lua_pushliteral(_state, "__newindex");          // ... CT mT __newindex
            lua_pushcfunction(_state, newIndex<ClassType>); // ... CT mT __newindex nifunc
            lua_settable(_state, classTable);               // ... CT mT
            //lua_pushliteral(_state, "__overloads");
            //lua_newtable(_state);
            //lua_settable(_state, classTable); // ... CT mT
            lua_pushliteral(_state, "__className");
            lua_pushstring(_state, _wrapper.m_className.cString());
            lua_settable(_state, classTable); // ... CT mT

            //register member functions
            registerFunctions(_state, classTable, _wrapper.m_members);

            //destructor
            using DesReg = DestructorRegistration<bHasDestructor>;
            DesReg::template registerDestructor<ClassType>(_state, classTable);

            lua_newtable(_state); // ... CT mT {}
            //register attributes if provided
            registerFunctions(_state, lua_gettop(_state), _wrapper.m_attributes); //... CT mT
            lua_setfield(_state, -2, "__attributes");                       // ... CT mT

            //create a table that holds the base classes
            lua_newtable(_state);                // ... CT {}
            lua_setfield(_state, -2, "__bases"); // ... CT

            lua_pop(_state, 1); // ...

            //now merge all the base metatables with this metatable for access speed
            auto sit = rep->m_bases.begin();

            stick::Size baseID = 1;
            for (; sit != rep->m_bases.end(); ++sit)
            {
                const LuanaticState::WrappedClass & wc = state->m_typeIDClassMap[*sit];
                //insert BASE class table in the __bases table of T's ClassTable
                lua_getfield(_state, -1, _wrapper.m_className.cString()); // ... CT
                lua_getfield(_state, -1, "__bases");                    // ... CT __bases
                stick::Int32 basesCount = baseID++;
                lua_pushinteger(_state, basesCount);                   // ... CT __bases i
                lua_rawgeti(_state, LUA_REGISTRYINDEX, wc.namespaceIndex); // ... CT __bases i nT
                lua_getfield(_state, -1, wc.wrapper->m_className.cString()); // ... CT __bases i nT CB
                lua_remove(_state, -2);                                // ... CT __bases i CB
                lua_settable(_state, -3);                              // ... CT __bases
                lua_pop(_state, 2);                                    // ...

                //for execution speed reasons we simply merge BASE's metatable into T's
                lua_getfield(_state, -1, _wrapper.m_className.cString()); // ... mt
                stick::Int32 toMetatable = lua_gettop(_state);
                lua_rawgeti(_state, LUA_REGISTRYINDEX, wc.namespaceIndex); // ... mt nt
                lua_getfield(_state, -1, wc.wrapper->m_className.cString()); // ... mt nt basemt
                lua_remove(_state, -2);                                // ... mt basemt

                STICK_ASSERT(!lua_isnil(_state, -1));

                //iterate over all the key value pairs in the base class meta table
                for (lua_pushnil(_state); lua_next(_state, -2); lua_pop(_state, 1))
                {
                    // TODO: use strcmp to avoid copies
                    if (stick::String(lua_tostring(_state, -2)) == "__bases" ||
                            //stick::String(lua_tostring(_state, -2)) == "__overloads" ||
                            stick::String(lua_tostring(_state, -2)) == "__typeID" ||
                            stick::String(lua_tostring(_state, -2)) == "__index" ||
                            stick::String(lua_tostring(_state, -2)) == "__newindex" ||
                            stick::String(lua_tostring(_state, -2)) == "__gc")
                    {
                        continue;
                    }
                    //check if this is the attributes sub table or another field
                    else if (stick::String(lua_tostring(_state, -2)) != "__attributes")
                    {
                        //check if the key allready exists in the target metatable
                        lua_pushvalue(_state, -2);         // mt basemt key value key
                        lua_pushvalue(_state, -1);         // mt basemt key value key key
                        lua_gettable(_state, toMetatable); // mt basemt key value key tableOrNil

                        //if not, copy the key value pair to T's metatable
                        if (lua_isnil(_state, -1))
                        {
                            lua_pop(_state, 1);        // mt basemt key value key
                            lua_pushvalue(_state, -2); // mt basemt key value key value
                            lua_settable(_state, toMetatable); // mt basemt key value
                        }
                        else
                            lua_pop(_state, 2); // mt basemt key value
                    }
                    else
                    {
                        //copy the attributes
                        for (lua_pushnil(_state); lua_next(_state, -2); lua_pop(_state, 1))
                        {
                            lua_getfield(_state, toMetatable, "__attributes"); // mt basemt key value key value mt.__attributes
                            lua_pushvalue(_state, -3);                         // mt basemt key value key value mt.__attributes key
                            lua_pushvalue(_state, -3);                         // mt basemt key value key value mt.__attributes key value
                            lua_settable(_state, -3);                          // mt basemt key value key value mt.__attributes
                            lua_pop(_state, 1);                                // mt basemt key value key value
                        }
                    }
                }

                lua_pop(_state, 2); //
            }
        }
    }

    template <class U, class Enable>
    void ValueTypeConverter<U, Enable>::push(lua_State * _state, const U & _value)
    {
        //By default we try pushing this as if it was a registered type,
        //hoping that it will succeed.
        detail::LuanaticState * state = detail::luanaticState(_state);
        STICK_ASSERT(state);
        luanatic::push(_state, state->m_allocator->create<U>(_value), true);
        //luaL_error(_state, "No ValueTypeConverter implementation found");
    }

    template <class T>
    inline bool isOfType(lua_State * _luaState, stick::Int32 _index, bool _bStrict)
    {
        bool ret = false;
        if (lua_isuserdata(_luaState, _index))
        {
            if (lua_isuserdata(_luaState, _index) &&
                    lua_getmetatable(_luaState, _index))
            {
                lua_getfield(_luaState, -1, "__typeID");
                stick::TypeID tid = (stick::TypeID)lua_tointeger(_luaState, -1);
                if (tid == stick::TypeInfoT<T>::typeID())
                    ret = true;

                if (!ret && !_bStrict)
                {
                    detail::LuanaticState * glua = detail::luanaticState(_luaState);
                    STICK_ASSERT(glua != nullptr);

                    ret = detail::checkBases<T>(*glua, tid);
                }

                lua_pop(_luaState, 2);
            }
        }

        return ret;
    }

    template <class T>
    inline T * convertToType(lua_State * _luaState, stick::Int32 _index, bool _bStrict)
    {
        if (isOfType<T>(_luaState, _index, _bStrict))
        {
            detail::UserData * pud = static_cast<detail::UserData *>(lua_touserdata(_luaState, _index));
            if (pud->m_typeID == stick::TypeInfoT<T>::typeID())
            {
                return static_cast<T *>(pud->m_data);
            }
            if (!_bStrict)
            {
                detail::LuanaticState * glua = detail::luanaticState(_luaState);
                STICK_ASSERT(glua != nullptr);

                auto result = detail::findCastFunction(
                                  *glua, *pud, stick::TypeInfoT<T>::typeID());
                if (result.bFound)
                {
                    return static_cast<T *>(result.userData.m_data);
                }
            }
        }

        return nullptr;
    }

    template <class T>
    inline T * convertToTypeAndCheck(lua_State * _luaState, stick::Int32 _index, bool _bStrict)
    {
        T * ret = convertToType<T>(_luaState, _index, _bStrict);

        if (!ret)
        {
            detail::LuanaticState * glua = detail::luanaticState(_luaState);
            STICK_ASSERT(glua != nullptr);

            auto it = glua->m_typeIDClassMap.find(stick::TypeInfoT<T>::typeID());
            if (it != glua->m_typeIDClassMap.end())
            {
                /*auto di = detail::debugInfo(_luaState, _index + 1);
                luaL_error(_luaState, "%s, line %d: %s expected, got %s", di.source, di.currentline,
                           glua->m_typeIDClassMap[stick::TypeInfoT<T>::typeID()].wrapper->m_className.cString(),
                           luaL_typename(_luaState, _index));*/
                detail::luaErrorWithStackTrace(_luaState, _index, "%s expected, got %s",
                                               glua->m_typeIDClassMap[stick::TypeInfoT<T>::typeID()].wrapper->m_className.cString(),
                                               luaL_typename(_luaState, _index));
            }
            else
            {
                /*auto di = detail::debugInfo(_luaState, _index + 1);
                printf("%s, %s\n", di.namewhat, di.name);
                luaL_traceback(_luaState, _luaState, NULL, _index);
                printf("%s\n", lua_tostring(_luaState, -1));
                luaL_error(_luaState, "%s, line %d: Different unregistered type expected, got %s", di.source, di.currentline, luaL_typename(_luaState, _index));
                */
                detail::luaErrorWithStackTrace(_luaState, _index, "Different unregistered type expected, got %s", luaL_typename(_luaState, _index));
            }
            //lua_error(_luaState);
        }

        return ret;
    }

    template <class T>
    inline T convertToValueTypeAndCheck(lua_State * _luaState, stick::Int32 _index)
    {
        // check if we can simply make a copy
        T * other = convertToType<T>(_luaState, _index);
        if (other)
        {
            return *other;
        }

        // check if there is a value converted implemented
        return ValueTypeConverter<T>::convertAndCheck(_luaState, _index);
    }

    template <class T, class WT>
    inline bool pushWrapped(lua_State * _luaState, const WT * _obj, bool _bLuaOwnsObject)
    {
        if (_obj)
        {
            if (_bLuaOwnsObject)
            {
                lua_getfield(_luaState, LUA_REGISTRYINDEX, LUANATIC_KEY); // glua
                lua_getfield(_luaState, -1, "weakTable");                 // glua glua.weakTable
                lua_remove(_luaState, -2);                                // glua.weakTable
                STICK_ASSERT(lua_istable(_luaState, -1));

                detail::ObjectIdentifier<WT>::identify(_luaState, _obj);
                STICK_ASSERT(lua_type(_luaState, -1) == LUA_TLIGHTUSERDATA);

                lua_pushvalue(_luaState, -1); // glua.weakTable id id
                lua_gettable(_luaState, -3);  // glua.weakTable id objOrNil

                if (lua_isnil(_luaState, -1))
                {
                    lua_pop(_luaState, 1); // glua.weakTable id
                    void * ptr = lua_newuserdata(_luaState, sizeof(detail::UserData));
                    detail::UserData * userData = static_cast<detail::UserData *>(ptr); // glua.weakTable id ud
                    userData->m_data = (void *)_obj;
                    userData->m_typeID = stick::TypeInfoT<WT>::typeID();
                    userData->m_bOwnedByLua = _bLuaOwnsObject;
                    detail::LuanaticState * glua = detail::luanaticState(_luaState);
                    STICK_ASSERT(glua != nullptr);

                    auto it = glua->m_typeIDClassMap.find(stick::TypeInfoT<T>::typeID());
                    if (it == glua->m_typeIDClassMap.end())
                    {
                        luaL_error(_luaState, "Can't push unregistered type to Lua!");
                    }

                    lua_rawgeti(_luaState, LUA_REGISTRYINDEX, (*it).value.namespaceIndex);
                    lua_getfield(_luaState, -1, (*it).value.wrapper->m_className.cString()); // glua.weakTable id ud namespace mt
                    lua_remove(_luaState, -2); // glua.weakTable id ud mt

                    lua_setmetatable(_luaState, -2); // glua.weakTable id ud

                    lua_pushvalue(_luaState, -1); // glua.weakTable id ud ud
                    lua_insert(_luaState, -4);    // ud glua.weakTable id ud
                    lua_settable(_luaState, -3);  // ud glua.weakTable
                    lua_pop(_luaState, 1);        // ud
                }
                else
                {
                    lua_replace(_luaState, -3); // ud id
                    lua_pop(_luaState, 1);      // ud
                    return true;
                }
            }
            else
            {
                detail::LuanaticState * glua = detail::luanaticState(_luaState);
                STICK_ASSERT(glua != nullptr);

                void * ptr = lua_newuserdata(_luaState, sizeof(detail::UserData));
                detail::UserData * userData = static_cast<detail::UserData *>(ptr); // ud
                userData->m_data = (void *)_obj;
                userData->m_typeID = stick::TypeInfoT<WT>::typeID();
                userData->m_bOwnedByLua = false;

                auto it = glua->m_typeIDClassMap.find(stick::TypeInfoT<T>::typeID());
                if (it == glua->m_typeIDClassMap.end())
                {
                    luaL_error(_luaState, "Can't push unregistered type to Lua!");
                }

                lua_rawgeti(_luaState, LUA_REGISTRYINDEX, (*it).value.namespaceIndex);
                lua_getfield(_luaState, -1, (*it).value.wrapper->m_className.cString()); // ud namespace mt
                lua_remove(_luaState, -2); // ud mt
                lua_setmetatable(_luaState, -2); // ud
            }
        }
        else
            lua_pushnil(_luaState);

        return false;
    }

    template <class T>
    inline bool push(lua_State * _luaState, const T * _obj, bool _bLuaOwnsObject)
    {
        return pushWrapped<T>(_luaState, _obj, _bLuaOwnsObject);
    }

    template <class T>
    inline void pushValueType(lua_State * _luaState, const T & _value)
    {
        return ValueTypeConverter<T>::push(_luaState, _value);
    }

    template <class T, class...Args>
    inline void constructUnregisteredType(lua_State * _luaState, Args ... _args)
    {
        void * ud = lua_newuserdata(_luaState, sizeof(T));
        lua_newtable(_luaState);
        lua_pushcfunction(_luaState, &detail::destructUnregistered<T>);
        lua_setfield(_luaState, -2, "__gc");
        lua_setmetatable(_luaState, -2);
        new (ud) T(std::forward<Args>(_args)...);
    }

    template <class T>
    inline void pushUnregisteredType(lua_State * _luaState, const T & _value)
    {
        void * ud = lua_newuserdata(_luaState, sizeof(T));
        lua_newtable(_luaState);
        lua_pushcfunction(_luaState, &detail::destructUnregistered<T>);
        lua_setfield(_luaState, -2, "__gc");
        lua_setmetatable(_luaState, -2);
        new (ud) T(_value);
    }

    namespace detail
    {
        template <class T, class Enable = void>
        struct Converter;

        template <class T>
        struct RawType
        {
            using Type = typename std::remove_cv<typename std::remove_pointer<
                         typename std::remove_reference<T>::type>::type>::type;
        };

        // Convert to a raw pointer type.
        template <class T>
        struct Converter<T *>
        {
            using Ret = T*;

            static Ret convert(lua_State * _luaState, stick::Int32 _index)
            {
                Ret ret = convertToTypeAndCheck<typename RawType<T>::Type>(
                              _luaState, _index);
                return ret;
            }
        };

        // Convert to a reference type
        template <class T>
        struct Converter<T &>
        {
            using Ret = T&;

            static Ret convert(lua_State * _luaState, stick::Int32 _index)
            {
                Ret ret = *(convertToTypeAndCheck<typename RawType<T>::Type>(
                                _luaState, _index));
                return ret;
            }
        };

        template <class T>
        struct Converter<const T &>
        {
            struct ReturnProxy
            {
                ReturnProxy(const T & _ref) :
                    ptr(nullptr),
                    ref(_ref)
                {

                }

                ReturnProxy(T * _ptr, stick::Allocator * _allocator) :
                    ptr(_ptr),
                    allocator(_allocator),
                    ref(*_ptr)
                {

                }

                ReturnProxy(ReturnProxy && _other) :
                    ptr(_other.ptr),
                    allocator(_other.allocator),
                    ref(*ptr)
                {
                    _other.ptr = nullptr;
                }

                ~ReturnProxy()
                {
                    if (ptr)
                    {
                        stick::destroy(ptr, *allocator);
                    }
                }

                ReturnProxy(const ReturnProxy & _other) = delete;
                ReturnProxy & operator = (const ReturnProxy & _other) = delete;

                operator const T & ()
                {
                    return ref;
                }

                T * ptr;
                stick::Allocator * allocator;
                const T & ref;
            };

            using Ret = ReturnProxy;

            static Ret convert(lua_State * _luaState, stick::Int32 _index)
            {
                T * ret = convertToType<typename RawType<T>::Type>(_luaState, _index);
                if (ret)
                {
                    return ReturnProxy(*ret);
                }
                else
                {
                    //Try implicitly converting from lua, we need the return proxy to clean up
                    //the tmp memory we need for this.
                    LuanaticState * ls = luanaticState(_luaState);
                    STICK_ASSERT(ls);
                    auto ptr = ls->m_allocator->create<T>(convertToValueTypeAndCheck<typename RawType<T>::Type>(_luaState, _index));
                    return ReturnProxy(ptr, ls->m_allocator);
                }
            }
        };

        // convert to a value type
        template <class T>
        struct Converter<T>
        {
            using Ret = T;

            static Ret convert(lua_State * _luaState, stick::Int32 _index)
            {
                Ret ret = convertToValueTypeAndCheck<typename RawType<T>::Type>(_luaState, _index);
                return ret;
            }
        };

        template<class T>
        typename Converter<T>::Ret convert(lua_State * _luaState, stick::Int32 _index)
        {
            return Converter<T>::convert(_luaState, _index);
        }

        struct TransferOwnership
        {
            template <class T>
            inline void push(lua_State * _luaState, const T * _obj) const
            {
                luanatic::push<T>(_luaState, _obj, true);
            }
        };

        struct KeepOwnership
        {
            template <class T>
            inline void push(lua_State * _luaState, const T * _obj) const
            {
                luanatic::push<T>(_luaState, _obj, false);
            }
        };

        template <class T, class Enable = void>
        struct Pusher;

        template <class T>
        struct Pusher<T *>
        {
            template <class OwnershipPolicy = KeepOwnership>
            static void push(lua_State * _luaState, T * _val, const OwnershipPolicy & _policy = OwnershipPolicy())
            {
                // for pointers, we acknowledge the ownership policy
                _policy.template push<typename RawType<T>::Type>(_luaState, _val);
            }
        };

        template <class T>
        struct Pusher<T &>
        {
            template <class OwnershipPolicy = KeepOwnership>
            static void push(lua_State * _luaState, T & _val, const OwnershipPolicy & _policy = OwnershipPolicy())
            {
                // ownership policy does not apply for references or value types
                luanatic::push<typename RawType<T>::Type>(_luaState, &_val, false);
            }
        };

        template <class T>
        struct Pusher<const T &>
        {
            template <class OwnershipPolicy = KeepOwnership>
            static void push(lua_State * _luaState, const T & _val, const OwnershipPolicy & _policy = OwnershipPolicy())
            {
                luanatic::pushValueType<typename RawType<T>::Type>(_luaState, _val);
            }
        };

        template <class T>
        struct Pusher<T>
        {
            template <class OwnershipPolicy = KeepOwnership>
            static void push(lua_State * _luaState, T _val, const OwnershipPolicy & _policy = OwnershipPolicy())
            {
                // ownership policy does not apply for value types
                luanatic::pushValueType<typename RawType<T>::Type>(_luaState, _val);
            }
        };

        // this is a specialization for char arrays to push them as a string ...
        template <stick::Size N>
        struct Pusher<char[N]>
        {
            template <class OwnershipPolicy = KeepOwnership>
            static void push(lua_State * _luaState, const char _val[N], const OwnershipPolicy & _policy = OwnershipPolicy())
            {
                lua_pushstring(_luaState, _val);
            }
        };

        inline bool checkArgumentCount(lua_State * _luaState,
                                       stick::UInt32 _targetCount,
                                       stick::UInt32 _luaArgCountAdjust,
                                       stick::UInt32 & _outActualCount)
        {
            _outActualCount = lua_gettop(_luaState) + _luaArgCountAdjust;
            if (_outActualCount == _targetCount)
                return true;
            return false;
        }

        inline bool checkArgumentCountAndEmitLuaError(lua_State * _luaState, stick::UInt32 _targetCount, stick::UInt32 _luaArgCountAdjust)
        {
            stick::UInt32 actualArgCount;
            if (!checkArgumentCount(_luaState, _targetCount, _luaArgCountAdjust, actualArgCount))
            {
                luaErrorWithStackTrace(_luaState, 0, "Expected %d, got %d", _targetCount, actualArgCount);
                return false;
            }
            return true;
        }

        template <class T, T Func>
        struct FunctionWrapper;

        struct Indexer
        {
            Indexer(stick::Int32 _idx) :
                idx(_idx)
            {

            }

            stick::Int32 increment()
            {
                return idx++;
            }

            stick::Int32 idx;
        };

        template<class T, T... Ints> struct integer_sequence
        {
        };

        template<class S> struct next_integer_sequence;

        template<class T, T... Ints> struct next_integer_sequence<integer_sequence<T, Ints...>>
        {
            using type = integer_sequence<T, Ints..., sizeof...(Ints)>;
        };

        template<class T, T I, T N> struct make_int_seq_impl;

        template<class T, T N>
            using make_integer_sequence = typename make_int_seq_impl<T, 0, N>::type;

        template<class T, T I, T N> struct make_int_seq_impl
        {
            using type = typename next_integer_sequence<
                typename make_int_seq_impl<T, I+1, N>::type>::type;
        };

        template<class T, T N> struct make_int_seq_impl<T, N, N>
        {
            using type = integer_sequence<T>;
        };

        template<std::size_t... Ints>
        using index_sequence = integer_sequence<std::size_t, Ints...>;

        template<std::size_t N>
        using make_index_sequence = make_integer_sequence<std::size_t, N>;

        template <class Ret, class F, class...Args, std::size_t...N>
        inline Ret callFunctionImpl(lua_State * _state, F _callable, index_sequence<N...>)
        {
            return _callable(convert<Args>(_state, N)...);
        }

        template<class Ret, class F, class...Args>
        inline Ret callFunction(lua_State * _state, F _callable)
        {
            return callFunctionImpl<Ret>(_state, _callable, make_index_sequence<sizeof...(Args)>());
        }

        template <class Ret, class... Args, Ret (*Func)(Args...)>
        struct FunctionWrapper<Ret(*)(Args...), Func>
        {

            static stick::Int32 func(lua_State * _luaState)
            {
                return funcImpl(_luaState, make_index_sequence<sizeof...(Args)>());
            }

            template<std::size_t...N>
            static stick::Int32 funcImpl(lua_State * _luaState, index_sequence<N...>)
        {
            checkArgumentCountAndEmitLuaError(_luaState, sizeof...(Args), 0);

            Indexer idx(1);
            Pusher<Ret>::push(_luaState, (*Func)(convert<Args>(_luaState, 1 + N)...));
            return 1;
        }
        };

        template <class... Args, void (*Func)(Args...)>
        struct FunctionWrapper<void(*)(Args...), Func>
        {
            static stick::Int32 func(lua_State * _luaState)
            {
                return funcImpl(_luaState, make_index_sequence<sizeof...(Args)>());
            }

            template<std::size_t...N>
            static stick::Int32 funcImpl(lua_State * _luaState, index_sequence<N...>)
        {
            checkArgumentCountAndEmitLuaError(_luaState, sizeof...(Args), 0);
            (*Func)(convert<Args>(_luaState, 1 + N)...);
            return 0;
        }
        };

        template <class Ret, class C, class... Args, Ret (C::*Func)(Args...)>
        struct FunctionWrapper<Ret (C::*)(Args...), Func>
        {
            static stick::Int32 func(lua_State * _luaState)
            {
                return funcImpl(_luaState, make_index_sequence<sizeof...(Args)>());
            }

            template<std::size_t...N>
            static stick::Int32 funcImpl(lua_State * _luaState, index_sequence<N...>)
        {
            checkArgumentCountAndEmitLuaError(_luaState, sizeof...(Args), -1); // - 1 for self
            C * obj = convertToTypeAndCheck<typename RawType<C>::Type>(_luaState, 1);
            Pusher<Ret>::push(_luaState, (obj->*Func)(convert<Args>(_luaState, 2 + N)...));
            return 1;
        }
        };

        template <class Ret, class C, class... Args, Ret (C::*Func)(Args...) const>
        struct FunctionWrapper<Ret (C::*)(Args...) const, Func>
        {

            static stick::Int32 func(lua_State * _luaState)
            {
                return funcImpl(_luaState, make_index_sequence<sizeof...(Args)>());
            }

            template<std::size_t...N>
            static stick::Int32 funcImpl(lua_State * _luaState, index_sequence<N...>)
        {
            checkArgumentCountAndEmitLuaError(_luaState, sizeof...(Args), -1); // - 1 for self
            C * obj = convertToTypeAndCheck<typename RawType<C>::Type>(_luaState, 1);
            Pusher<Ret>::push(_luaState, (obj->*Func)(convert<Args>(_luaState, 2 + N)...));
            return 1;
        }
        };

        template <class C, class... Args, void (C::*Func)(Args...)>
        struct FunctionWrapper<void (C::*)(Args...), Func>
        {

            static stick::Int32 func(lua_State * _luaState)
            {
                return funcImpl(_luaState, make_index_sequence<sizeof...(Args)>());
            }

            template<std::size_t...N>
            static stick::Int32 funcImpl(lua_State * _luaState, index_sequence<N...>)
        {
            checkArgumentCountAndEmitLuaError(_luaState, sizeof...(Args), -1); // - 1 for self
            C * obj = convertToTypeAndCheck<typename RawType<C>::Type>(_luaState, 1);
            (obj->*Func)(convert<Args>(_luaState, 2 + N)...);
            return 0;
        }
        };

        template <class C, class... Args, void (C::*Func)(Args...) const>
        struct FunctionWrapper<void (C::*)(Args...) const, Func>
        {

            static stick::Int32 func(lua_State * _luaState)
            {
                return funcImpl(_luaState, make_index_sequence<sizeof...(Args)>());
            }

            template<std::size_t...N>
            static stick::Int32 funcImpl(lua_State * _luaState, index_sequence<N...>)
        {
            checkArgumentCountAndEmitLuaError(_luaState, sizeof...(Args), -1); // - 1 for self
            C * obj = convertToTypeAndCheck<typename RawType<C>::Type>(_luaState, 1);
            (obj->*Func)(convert<Args>(_luaState, 2 + N)...);
            return 0;
        }
        };

        template <class T, T Attr>
        struct AttributeWrapper;

        template <class Ret, class C, Ret * C::*Member>
        struct AttributeWrapper<Ret * C::*, Member>
        {
            static stick::Int32 func(lua_State * _luaState)
            {
                C * obj = convertToTypeAndCheck<C>(_luaState, 1);
                if (lua_gettop(_luaState) == 1)
                {
                    //no arguments, so get
                    push<typename RawType<Ret>::Type>(_luaState, obj->*Member, false);
                    return 1;
                }
                else
                {
                    //there is an arugment on the stack, so set.
                    obj->*Member = convertToTypeAndCheck<typename RawType<Ret>::Type>(_luaState, 2);
                    return 0;
                }
            }
        };

        template<class T, class Enable = void>
        struct AttributePusher
        {
            static void push(lua_State * _luaState, T & _val)
            {
                luanatic::push(_luaState, &_val, false);
            }
        };

        template<class T>
        struct AttributePusher<T, typename std::enable_if<std::is_integral<T>::value || std::is_enum<T>::value || std::is_floating_point<T>::value>::type>
        {
            static void push(lua_State * _luaState, T & _val)
            {
                luanatic::pushValueType(_luaState, _val);
            }
        };

        template <class Ret, class C, Ret C::*Member>
        struct AttributeWrapper <Ret C::*, Member>
        {
            static stick::Int32 func(lua_State * _luaState)
            {
                C * obj = convertToTypeAndCheck<C>(_luaState, 1);

                if (lua_gettop(_luaState) == 1)
                {
                    //printf("GET ATTRIBUTE\n");
                    //no arguments, so get
                    //pushValueType<Ret>(_luaState, obj->*Member);
                    AttributePusher<Ret>::push(_luaState, obj->*Member);
                    return 1;
                }
                else
                {
                    //printf("SET ATTRIBUTE\n");
                    obj->*Member = convertToValueTypeAndCheck<Ret>(_luaState, 2);
                    return 0;
                }
            }
        };

        template <class T, class... Args>
        struct ConstructorWrapper
        {
            //this function serves to have an extra step between converting the arguments
            //from lua, so that in case of an error, we don't leak the newly created T obj
            static T * create(LuanaticState * _state, Args... _args)
            {
                return _state->m_allocator->create<T>(std::forward<Args>(_args)...);
            }


            static stick::Int32 func(lua_State * _luaState)
            {
                return funcImpl(_luaState, make_index_sequence<sizeof...(Args)>());
            }

            template<std::size_t...N>
            static stick::Int32 funcImpl(lua_State * _luaState, index_sequence<N...>)
            {
                LuanaticState * glua = luanaticState(_luaState);
                STICK_ASSERT(glua != nullptr);

                checkArgumentCountAndEmitLuaError(_luaState, sizeof...(Args), 0);

                T * obj = create(glua, convert<Args>(_luaState, 1 + N)...);

                if (obj)
                {
                    push<T>(_luaState, obj, true);
                    return 1;
                }

                return 0;
            }
        };

        template<class Iterator, bool ForceReference>
        struct IterValuePusher;

        template<class Iterator>
        struct IterValuePusher<Iterator, true>
        {
            static void push(lua_State * _luaState, Iterator * _it)
            {
                typedef typename stick::IteratorTraits<Iterator>::ReferenceType ReferenceType;
                Pusher<ReferenceType>::push(_luaState, **_it);
            }
        };

        template<class Iterator>
        struct IterValuePusher<Iterator, false>
        {
            static void push(lua_State * _luaState, Iterator * _it)
            {
                typedef typename stick::IteratorTraits<Iterator>::ValueType ValueType;
                Pusher<ValueType>::push(_luaState, **_it);
            }
        };

        template<class Iterator, bool ForceReference>
        stick::Int32 iteratorFunction(lua_State * _luaState)
        {
            Iterator * iter = static_cast<Iterator *>(lua_touserdata(_luaState, lua_upvalueindex(1)));
            Iterator * end = static_cast<Iterator *>(lua_touserdata(_luaState, lua_upvalueindex(2)));

            if (iter && end)
            {
                if (*iter != *end)
                {
                    //push the current iterator value and advance
                    IterValuePusher<Iterator, ForceReference>::push(_luaState, iter);
                    (*iter)++;
                }
                else
                {
                    //we reached the end
                    lua_pushnil(_luaState);
                }
            }
            else
            {
                //emit error
                lua_pushstring(_luaState, "Invalid c++ iterator pair");
                lua_error(_luaState);
            }

            return 1;
        }

        template<class Iterator, bool ForceReference>
        struct RangeFunctionPusher;

        template<class Iterator>
        struct RangeFunctionPusher<Iterator, true>
        {
            static void push(lua_State * _luaState)
            {
                lua_pushcclosure(_luaState, iteratorFunction<Iterator, true>, 2);
            }
        };

        template<class Iterator>
        struct RangeFunctionPusher<Iterator, false>
        {
            static void push(lua_State * _luaState)
            {
                lua_pushcclosure(_luaState, iteratorFunction<Iterator, false>, 2);
            }
        };

        template<bool ForceReference, class Iterator>
        inline void pushRange(lua_State * _luaState, Iterator _begin, Iterator _end)
        {
            pushUnregisteredType<Iterator>(_luaState, _begin);
            pushUnregisteredType<Iterator>(_luaState, _end);
            RangeFunctionPusher<Iterator, ForceReference>::push(_luaState);
        }

        template<class T, T Func, bool ForceReference>
        struct IteratorWrapper;

        template<class Ret, Ret(Func)(void), bool ForceReference>
        struct IteratorWrapper<Ret(*)(void), Func, ForceReference>
        {
            static stick::Int32 func(lua_State * _luaState)
        {
            Ret container = Func();
            pushRange<ForceReference>(_luaState, container.begin(), container.end());

            return 1;
        }
        };

        template<class Ret, class C, Ret(C::*Func)(void)const, bool ForceReference>
        struct IteratorWrapper<Ret(C::*)(void)const, Func, ForceReference>
        {
            static stick::Int32 func(lua_State * _luaState)
        {
            C * obj = convertToTypeAndCheck<C>(_luaState, 1);
            Ret container = (obj->*Func)();

            pushRange<ForceReference>(_luaState, container.begin(), container.end());

            return 1;
        }
        };

        template<class Ret, class C, Ret(C::*Func)(void), bool ForceReference>
        struct IteratorWrapper<Ret(C::*)(void), Func, ForceReference>
        {
            static stick::Int32 func(lua_State * _luaState)
        {
            C * obj = convertToTypeAndCheck<C>(_luaState, 1);
            Ret container = (obj->*Func)();

            pushRange<ForceReference>(_luaState, container.begin(), container.end());

            return 1;
        }
        };
    }

    //ClassWrapper implementation

    template <class T>
    ClassWrapper<T>::ClassWrapper(const stick::String & _name)
        : ClassWrapperBase(stick::TypeInfoT<T>::typeID(), _name)
    {
    }

    template <class T>
    template <class... Args>
    ClassWrapper<T> & ClassWrapper<T>::addConstructor(const stick::String & _str)
    {
        m_statics.append({ _str, detail::ConstructorWrapper<T, Args...>::func });
        return *this;
    }

    template <class T>
    ClassWrapperUniquePtr ClassWrapper<T>::clone() const
    {
        return ClassWrapperUniquePtr(stick::defaultAllocator().create<ClassWrapper>(*this), ClassWrapperUniquePtr::Cleanup());
    }

    template <class T>
    template <class B>
    ClassWrapper<T> & ClassWrapper<T>::addBase()
    {
        m_bases.append(stick::TypeInfoT<B>::typeID());
        m_casts.append(detail::UserDataCastFunction(stick::TypeInfoT<B>::typeID(), detail::TypeCaster<T, B>::cast));
        return *this;
    }

    template <class T>
    template <class B>
    ClassWrapper<T> & ClassWrapper<T>::addCast()
    {
        m_casts.append(detail::UserDataCastFunction(stick::TypeInfoT<B>::typeID(), detail::TypeCaster<T, B>::cast));
        return *this;
    }


    struct STICK_API NoDestructorFlag {};

    class STICK_API LuaValue
    {
    public:
        LuaValue()
            : m_state(nullptr)
            , m_index(LUA_NOREF)
            , m_parentTableIndex(LUA_NOREF)
            , m_keyIndex(LUA_NOREF)
            , m_type(LuaType::None)
            , m_bPopKey(false)
        {
        }

        LuaValue(lua_State * _state, stick::Int32 _index, stick::Int32 _parentIndex = LUA_NOREF, stick::Int32 _keyIndex = LUA_NOREF)
            : m_state(_state)
            , m_index(_index)
            , m_parentTableIndex(_parentIndex)
            , m_keyIndex(_keyIndex)
            , m_bPopKey(true)
        {
            lua_pushvalue(m_state, _index);
            init();
        }

        LuaValue(const LuaValue & _other)
            : m_state(_other.m_state)
            , m_index(LUA_NOREF)
            , m_parentTableIndex(_other.m_parentTableIndex)
            , m_keyIndex(_other.m_keyIndex)
            , m_type(_other.m_type)
            , m_bPopKey(false)
        {
            _other.m_bPopKey = false;
            if (m_state)
            {
                lua_rawgeti(m_state, LUA_REGISTRYINDEX, _other.m_index);
                m_index = luaL_ref(m_state, LUA_REGISTRYINDEX);
                if (m_keyIndex != LUA_NOREF)
                    m_bPopKey = true;
            }
        }

        ~LuaValue()
        {
            reset();
        }

        LuaValue & operator=(const LuaValue & _other)
        {
            reset();

            m_state = _other.m_state;
            m_index = LUA_NOREF;
            m_parentTableIndex = _other.m_parentTableIndex;
            m_keyIndex = _other.m_keyIndex;
            m_bPopKey = false;
            _other.m_bPopKey = false;
            m_type = _other.m_type;

            if (!m_state)
                return *this;

            lua_rawgeti(m_state, LUA_REGISTRYINDEX, _other.m_index);
            m_index = luaL_ref(m_state, LUA_REGISTRYINDEX);

            if (m_keyIndex != LUA_NOREF)
                m_bPopKey = true;

            return *this;
        }

        LuaValue & operator=(const Nil & _nil)
        {
            reset();
            return *this;
        }

        bool operator==(const LuaValue & _other) const
        {
            return m_state == _other.m_state && m_index == _other.m_index;
        }

        bool operator!=(const LuaValue & _other) const
        {
            return !(*this == _other);
        }

        void init()
        {
            switch (lua_type(m_state, -1))
            {
                case LUA_TNIL:
                    m_type = LuaType::Nil;
                    break;
                case LUA_TBOOLEAN:
                    m_type = LuaType::Boolean;
                    break;
                case LUA_TNUMBER:
                    m_type = LuaType::Number;
                    break;
                case LUA_TSTRING:
                    m_type = LuaType::String;
                    break;
                case LUA_TUSERDATA:
                    m_type = LuaType::UserData;
                    break;
                case LUA_TFUNCTION:
                    m_type = LuaType::Function;
                    break;
                case LUA_TTHREAD:
                    m_type = LuaType::Thread;
                    break;
                case LUA_TTABLE:
                    m_type = LuaType::Table;
                    break;
                default:
                    m_type = LuaType::None;
                    break;
            }

            m_index = luaL_ref(m_state, LUA_REGISTRYINDEX);
        }

        template <class T>
        LuaValue operator[](const T & _key)
        {
            return getChild(_key);
        }

        template <class T, class OwnershipPolicy = detail::KeepOwnership>
        void set(const T & _value, const OwnershipPolicy & _policy = OwnershipPolicy())
        {
            STICK_ASSERT(m_state);
            if (m_parentTableIndex != LUA_NOREF)
            {
                //delete the current reference if any
                if (m_index != LUA_NOREF)
                    luaL_unref(m_state, LUA_REGISTRYINDEX, m_index);

                m_index = LUA_NOREF;
                //push the parent table onto the stack
                lua_rawgeti(m_state, LUA_REGISTRYINDEX, m_parentTableIndex);
                //push the table key onto the stack
                lua_pushvalue(m_state, m_keyIndex);
                //push the new value onto the stack
                detail::Pusher<T>::push(m_state, _value, _policy);
                //init
                init();
                //push the new value onto the stack again
                lua_rawgeti(m_state, LUA_REGISTRYINDEX, m_index);
                //set the table
                lua_settable(m_state, -3);
                //pop the table off the stack
                lua_pop(m_state, 1);
            }
            else
            {
                reset();
                detail::Pusher<T>::push(m_state, _value, _policy);
                //create the reference, determine type and pop value
                init();
            }
        }

        template <class T>
        T get()
        {
            STICK_ASSERT(isValid());
            push();
            detail::StackPop popper(m_state, 1);
            return detail::convert<T>(m_state, -1);
        }

        template <class K>
        LuaValue getChild(const K & _key)
        {
            STICK_ASSERT(m_type == LuaType::Table);
            detail::Pusher<K>::push(m_state, _key);
            push();
            lua_pushvalue(m_state, -2);
            lua_gettable(m_state, -2);
            LuaValue ret(m_state, -1, m_index, lua_gettop(m_state) - 2);
            lua_pop(m_state, 2);
            return ret;
        }

        stick::Size childCount() const
        {
            if (m_type == LuaType::Table && m_state)
            {
                stick::Size ret = 0;
                push();
                //we manually count the children so that it also works with dictionary style tables
                for (lua_pushnil(m_state); lua_next(m_state, -2); lua_pop(m_state, 1))
                {
                    ret++;
                }
                lua_pop(m_state, 1);
                return ret;
            }
            return 0;
        }

        void reset()
        {
            if (m_state)
            {
                if (m_bPopKey && m_keyIndex != LUA_NOREF)
                {
                    lua_remove(m_state, m_keyIndex);
                    m_keyIndex = LUA_NOREF;
                }
                if (m_index != LUA_NOREF)
                {
                    luaL_unref(m_state, LUA_REGISTRYINDEX, m_index);
                    m_index = LUA_NOREF;
                }
            }
        }

        void push() const
        {
            lua_rawgeti(m_state, LUA_REGISTRYINDEX, m_index);
        }

        LuaType type() const
        {
            return m_type;
        }

        bool isValid() const
        {
            return m_state && m_index != LUA_NOREF;
        }

        template <class CW>
        LuaValue & registerClass(const CW & _wrapper)
        {
            STICK_ASSERT(m_state && m_type == LuaType::Table);
            push();
            detail::registerClass(m_state, _wrapper);
            lua_pop(m_state, 1);
            return *this;
        }

        template <class CW>
        LuaValue & registerClass(const CW & _wrapper, NoDestructorFlag _flag)
        {
            STICK_ASSERT(m_state && m_type == LuaType::Table);
            push();
            detail::registerClass<CW, false>(m_state, _wrapper);
            lua_pop(m_state, 1);
            return *this;
        }

        LuaValue & registerFunction(const stick::String & _name,
                                    lua_CFunction _function)
        {
            STICK_ASSERT(m_state && m_type == LuaType::Table);
            push();
            ClassWrapperBase::NamedLuaFunctionArray tmp;
            tmp.append({ _name, _function });
            detail::registerFunctions(m_state, lua_gettop(m_state), tmp);
            lua_pop(m_state, 1);
            return *this;
        }

        template<class Ret, class...Args>
        struct CallFunctionProxy
        {
            CallFunctionProxy(lua_State * _state, const stick::String & _name, Args... _args)
            {
                lua_getfield(_state, -1, _name.cString());
                if (!lua_isnil(_state, -1))
                {
                    detail::Pass{ (detail::Pusher<Args>::push(_state, std::forward<Args>(_args)),
                                   1)... };
                    lua_call(_state, sizeof...(Args), 1);
                }
                else
                {
                    lua_pushfstring(_state, "Lua function: %s not found!",
                                    _name.cString());
                    lua_error(_state);
                }

                result = detail::Converter<Ret>::convert(_state, -1);
                lua_pop(_state, 1);
            }

            operator Ret()
            {
                return result;
            }

            Ret result;
        };

        template<class...Args>
        struct CallFunctionProxy<void, Args...>
        {
            CallFunctionProxy(lua_State * _state, const stick::String & _name, Args... _args)
            {
                lua_getfield(_state, -1, _name.cString());
                if (!lua_isnil(_state, -1))
                {
                    detail::Pass{ (detail::Pusher<Args>::push(_state, _args),
                                   1)... };
                    lua_call(_state, sizeof...(Args), 1);
                }
                else
                {
                    lua_pushfstring(_state, "Lua function: %s not found!",
                                    _name.cString());
                    lua_error(_state);
                }
                lua_pop(_state, 1);
            }
        };

        template <class R, class... Args>
        CallFunctionProxy<R, Args...> callFunction(const stick::String & _name, Args... _args)
        {
            STICK_ASSERT(m_state && m_type == LuaType::Table);
            push();
            /*lua_getfield(m_state, -1, _name.cString());
            if (!lua_isnil(m_state, -1))
            {
                detail::Pass{ (detail::Pusher<Args>::push(m_state, _args),
                               1)... };
                lua_call(m_state, sizeof...(Args), 0);
            }
            else
            {
                lua_pushfstring(m_state, "Lua function: %s not found!",
                                _name.cString());
                lua_error(m_state);
            }*/
            detail::StackPop popper(m_state, 1);
            return CallFunctionProxy<R, Args...>(m_state, _name, std::forward<Args>(_args)...);
        }

        /*template <class Ret, class... Args>
        Ret callFunction(const stick::String & _name, Args... _args)
        {
            STICK_ASSERT(m_state && m_type == LuaType::Table);
            push();

            lua_getfield(m_state, -1, _name.cString());
            if (!lua_isnil(m_state, -1))
            {
                detail::Pass{ (detail::Pusher<Args>::push(m_state, _args),
                               1)... };
                lua_call(m_state, sizeof...(Args), 1);
            }
            else
            {
                lua_pushfstring(m_state, "Lua function: %s not found!",
                                _name.cString());
                lua_error(m_state);
            }

            detail::StackPop popper(m_state, 1);
            return detail::Converter<Ret>::convert(m_state, -1);
        }*/

        lua_State * luaState() const { return m_state; }

        template <class Key>
        LuaValue findOrCreateTable(const Key & _key)
        {
            STICK_ASSERT(m_type == LuaType::Table && m_state);

            LuaValue ret = getChild(_key);

            if (ret.type() != LuaType::Table)
            {
                ret.reset(); //need to reset to pop the key off the stack.
                detail::Pusher<Key>::push(m_state, _key); //key
                stick::Int32 keyIndex = lua_gettop(m_state);
                push(); //key me
                lua_newtable(m_state); //key me {}
                lua_pushvalue(m_state, -3); //key me {} key
                lua_pushvalue(m_state, -2); //key me {} key {}
                lua_settable(m_state, -4); //key me {}
                ret = LuaValue(m_state, -1, m_index, keyIndex);
                lua_pop(m_state, 2); //key
            }

            return ret;
        }

        lua_State * m_state;
        stick::Int32 m_index;
        stick::Int32 m_parentTableIndex;
        stick::Int32 m_keyIndex;
        mutable bool m_bPopKey; //true if this instance is responsible for popping the key off the lua stack.
        LuaType m_type;
    };

    namespace detail
    {
        template <class T>
        inline void removeStorage(lua_State * _state, const T * _obj)
        {
            lua_getfield(_state, LUA_REGISTRYINDEX, LUANATIC_KEY); // glua
            lua_getfield(_state, -1, "storage");                   // glua glua.storage

            ObjectIdentifier<T>::identify(_state, _obj); // glua.storage id

            lua_pushvalue(_state, -1); // glua.storage id id
            lua_gettable(_state, -3);  // glua.storage id objOrNil

            if (lua_isnil(_state, -1))
            {
                // if nil, there is nothing to invalidate
                lua_pop(_state, 3);
                return;
            }
            else
            {
                lua_pop(_state, 1);       // glua.storage id
                lua_pushnil(_state);      // glua.storage id nil
                lua_settable(_state, -3); // glua.storage
                lua_pop(_state, 1);       //
            }
        }

        template <class T>
        inline stick::Int32 newIndex(lua_State * _luaState)
        {
            T * obj = convertToTypeAndCheck<T>(_luaState, 1);

            // first we check if this is an attribute
            lua_getmetatable(_luaState, 1); // obj key val mt
            lua_getfield(_luaState, -1,
                         "__attributes"); // obj key val mt __attributes
            lua_pushvalue(_luaState, -4); // obj key val mt __attributes key
            lua_gettable(_luaState,
                         -2); // obj key val mt __attributes funcOrNil

            // if we found it we call it
            if (!lua_isnil(_luaState, -1))
            {
                // push the function arguments
                lua_pushvalue(_luaState,
                              1); // obj key val mt __attributes funcOrNil obj
                lua_pushvalue(
                    _luaState,
                    3); // obj key val mt __attributes funcOrNil obj val
                lua_call(_luaState, 2, 0);
                return 0;
            }

            lua_settop(_luaState, 3); // obj key val
            lua_getfield(_luaState, LUA_REGISTRYINDEX,
                         LUANATIC_KEY);             // obj key val gT
            lua_getfield(_luaState, -1, "storage"); // obj key val gT storage

            detail::ObjectIdentifier<T>::identify(_luaState,
                                                  obj); // obj key gT storage id
            lua_pushvalue(_luaState, -1);               // obj key val gT storage id id
            lua_gettable(_luaState,
                         -3); // obj key val gT storage id idtableOrNil

            // Add the storage table if there isn't one already
            if (lua_isnil(_luaState, -1))
            {
                lua_pop(_luaState, 1);        // obj key val gT storage id
                lua_newtable(_luaState);      // obj key val gT storage id {}
                lua_pushvalue(_luaState, -1); // obj key val gT storage id {} {}
                lua_insert(_luaState, -3);    // obj key gT val gT storage {} id {}
                lua_settable(_luaState, -4);  // obj key val gT storage {}
            }

            lua_pushvalue(_luaState, 2); // obj key val gT storage {} key
            lua_pushvalue(_luaState, 3); // obj key val gT storage {} key value
            lua_settable(_luaState, -3); // obj key val gT storage {key = value}

            return 0;
        }

        template <class T>
        inline stick::Int32 index(lua_State * _luaState)
        {
            T * obj = convertToType<T>(_luaState, 1);

            lua_getfield(_luaState, LUA_REGISTRYINDEX,
                         LUANATIC_KEY);             // obj key gT
            lua_getfield(_luaState, -1, "storage"); // obj key gT storage

            ObjectIdentifier<T>::identify(_luaState,
                                          obj); // obj key gT storage id
            lua_gettable(_luaState, -2);        // obj key gT storage objectStorage

            // check if storage table exists for this object
            if (!lua_isnil(_luaState, -1))
            {
                lua_pushvalue(_luaState,
                              -4); // obj key gT storage objectStorage key
                lua_gettable(_luaState, -2); // obj key gT storage objectStorage objectStorage[key]
            }

            // If either there is no storage table or the key wasn't found
            // then fall back to the metatable
            if (lua_isnil(_luaState, -1))
            {
                lua_settop(_luaState, 2);        // obj key
                lua_getmetatable(_luaState, -2); // obj key mt
                lua_pushvalue(_luaState, -2);    // obj key mt k
                lua_gettable(_luaState, -2);     // obj key mt mt[k]

                // look in the attributes table
                if (lua_isnil(_luaState, -1))
                {
                    lua_pop(_luaState, 1); // obj key mt
                    lua_getfield(_luaState, -1,
                                 "__attributes"); // obj key mt __attributes
                    lua_pushvalue(_luaState, -3); // obj key mt __attributes key
                    lua_gettable(_luaState,
                                 -2); // obj key mt __attributes funcOrNil

                    // if we found a function, we call it
                    if (!lua_isnil(_luaState, -1))
                    {
                        // call as getter
                        lua_pushvalue(_luaState, 1); // self
                        lua_call(_luaState, 1, 1);
                    }
                }
            }

            return 1;
        }

        template <class T>
        inline stick::Int32 destructUnregistered(lua_State * _luaState)
        {
            T * obj = static_cast<T *>(lua_touserdata(_luaState, 1));
            if (obj)
            {
                //lua will free the userdata, we just call the destructor
                obj->T::~T();
            }
            return 0;
        }

        template <class T>
        stick::Int32 destruct(lua_State * _luaState)
        {
            T * obj = convertToTypeAndCheck<T>(_luaState, 1);

            if (obj)
            {
                LuanaticState * state = luanaticState(_luaState);
                STICK_ASSERT(state != nullptr);

                removeStorage<T>(_luaState, obj);

                //for the ownership, we take the userdata addr as the identifier, because the pointer might have been
                //reused by the application, before lua collected it, which can cause conflicts. the userdata addr
                //on the other hand will be unique for the same object.
                UserData * addr = (UserData *)lua_touserdata(_luaState, 1);
                if (addr->m_bOwnedByLua)
                {
                    stick::destroy(obj, *state->m_allocator);
                }
            }

            return 0;
        }

        static stick::Int32 luanaticClassIndex(lua_State * _state)
        {
            lua_getmetatable(_state, 1);
            STICK_ASSERT(lua_istable(_state, -1));
            STICK_ASSERT(lua_isstring(_state, 2));
            const char * key = lua_tostring(_state, 2);
            lua_getfield(_state, -1, key); //... classTable fieldOrNil
            return 1;
        }

        static stick::Int32 luanaticNiceConstructor(lua_State * _state)
        {
            stick::Int32 argCount = lua_gettop(_state);
            lua_newtable(_state);                       //... objInstance
            lua_pushvalue(_state, lua_upvalueindex(1)); //... objInstance classTable
            lua_pushvalue(_state, -1);                  //... objInstance classTable classTable
            lua_setmetatable(_state, -3);               //... objInstance classTable
            lua_getfield(_state, -1, "__init");         //... objInstance classTable initOrNil
            if (!lua_isnil(_state, -1))
            {
                //call the __init function
                lua_insert(_state, 1);         //init ... objInstance classTable
                lua_insert(_state, 1);         //classTable init ... objInstance
                lua_insert(_state, 1);         //objInstance classTable init ...
                lua_call(_state, argCount, 0); // objInstance classTable
                lua_pop(_state, 1);            //objInstance
            }
            else
            {
                lua_pop(_state, 2); //... objInstance
            }
            return 1;
        }

        //creates a new glua style class from lua
        static stick::Int32 luanaticClassFunction(lua_State * _state)
        {
            lua_newtable(_state); //... classTable
            stick::Int32 toMetatable = lua_gettop(_state);
            stick::Int32 baseCount = lua_gettop(_state) - 1;
            lua_newtable(_state); //... classTable basesTable
            STICK_ASSERT(lua_istable(_state, -1));
            for (stick::Int32 i = 1; i <= baseCount; ++i)
            {
                lua_pushinteger(_state, i); //... classTable basesTable int
                lua_pushvalue(_state, i);   //... classTable basesTable int base
                lua_settable(_state, -3);   //... classTable basesTable
            }

            lua_setfield(_state, -2, "__bases"); //... classTable

            lua_pushcfunction(_state, luanaticClassIndex); //... classTable function
            lua_setfield(_state, -2, "__index");       //... classTable

            //metatable to make nice constructors
            lua_newtable(_state);                             //... classTable mt
            lua_pushvalue(_state, -2);                        //... classTable mt classTable
            lua_pushcclosure(_state, luanaticNiceConstructor, 1); //... classTable mt gluaNiceConstructor
            lua_setfield(_state, -2, "__call");               //... classTable mt
            lua_setmetatable(_state, -2);                     //... classTable

            //merge the bases into this classes metatable
            for (stick::Int32 i = 1; i <= baseCount; ++i)
            {
                lua_pushvalue(_state, i); //... classTable base
                STICK_ASSERT(lua_istable(_state, -1));
                for (lua_pushnil(_state); lua_next(_state, -2); lua_pop(_state, 1))
                {
                    //skip these
                    //TODO: Use strcmp
                    if (stick::String(lua_tostring(_state, -2)) == "__bases" ||
                            stick::String(lua_tostring(_state, -2)) == "__index")
                    {
                        continue;
                    }

                    //check if the key allready exists in the target metatable
                    lua_pushvalue(_state, -2);         //... classTable base key value key
                    lua_pushvalue(_state, -1);         //... classTable base key value key key
                    lua_gettable(_state, toMetatable); //... classTable base key value key tableOrNil

                    //if not, copy the key value pair to T's metatable
                    if (lua_isnil(_state, -1))
                    {
                        lua_pop(_state, 1);                //... classTable base key value key
                        lua_pushvalue(_state, -2);         //... classTable base key value key value
                        lua_settable(_state, toMetatable); //... classTable base key value
                    }
                    else
                        lua_pop(_state, 2); //... classTable basesTable base key value
                }
                lua_pop(_state, 1);
            }

            return 1;
        }

        static bool luanaticCheckBases(lua_State * _state)
        {
            lua_getfield(_state, -1, "__bases");
            STICK_ASSERT(lua_istable(_state, -1));
            for (lua_pushnil(_state); lua_next(_state, -2); lua_pop(_state, 1))
            {
                if (lua_rawequal(_state, 1, -1))
                {
                    return true;
                }
                else if (luanaticCheckBases(_state))
                {
                    return true;
                }
            }
            lua_pop(_state, 1);
            return false;
        }

        static stick::Int32 luanaticIsBaseOf(lua_State * _state)
        {
            if (lua_rawequal(_state, 1, 2))
            {
                lua_pushboolean(_state, true);
                return 1;
            }
            if (luanaticCheckBases(_state))
            {
                lua_pushboolean(_state, true);
                return 1;
            }
            lua_pushboolean(_state, false);
            return 1;
        }

        static stick::Int32 luanaticIsInstanceOf(lua_State * _state)
        {
            lua_getmetatable(_state, 1);
            lua_remove(_state, 1);
            return luanaticIsBaseOf(_state);
        }

        static stick::Int32 gluaEnsureClass(lua_State * _state)
        {
            /*STICK_ASSERT(lua_isuserdata(_state, 1));
            STICK_ASSERT(lua_istable(_state, 2));

            //This is sort of the fastest way of doing it, but not safe. Should we put more sanity
            //checks here? hmhmhmhm
            detail::UserData * userData = static_cast<detail::UserData*>(lua_touserdata(_state, 1));

            lua_getfield(_state, 2, "__typeID");

            if((stick::Size)userData->m_typeID != lua_tointeger(_state, -1))
            {
                userData->m_typeID = luaL_checkinteger(_state, -1);
                lua_pop(_state, 1);
                lua_setmetatable(_state, 1);
            }*/

            return 0;
        }
    }

    inline lua_State * createLuaState()
    {
#if LUA_VERSION_NUM >= 502
        return luaL_newstate();
#else
        return lua_open();
#endif
    }

    inline void initialize(lua_State * _state, stick::Allocator & _allocator)
    {
        //create the glua tables in the registry
        lua_getfield(_state, LUA_REGISTRYINDEX, LUANATIC_KEY);
        if (lua_isnil(_state, -1))
        {
            //lua_pushlightuserdata(_state, stick::defaultAllocator().create<detail::LuanaticState>());

            lua_newtable(_state);
            stick::Int32 luanaticTable = lua_gettop(_state);

            constructUnregisteredType<detail::LuanaticState>(_state, std::ref(_allocator));
            lua_setfield(_state, -2, "LuanaticState");

            lua_pushvalue(_state, luanaticTable);
            lua_setfield(_state, LUA_REGISTRYINDEX, LUANATIC_KEY);

            //create a storage table that c++ objects can store fields in that were created on the lua side
            //(so it feels similar to a normal lua object / table)
            lua_newtable(_state);
            lua_setfield(_state, luanaticTable, "storage");

            //weak table holding all the userdata that goes through this GluaState
            lua_newtable(_state);
            lua_newtable(_state);
            lua_pushstring(_state, "v");
            lua_setfield(_state, -2, "__mode");
            lua_setmetatable(_state, -2);
            lua_setfield(_state, luanaticTable, "weakTable");

            //utility functions to check bases and create glua style classes on the lua side
            LuaValue gt = globalsTable(_state);
            LuaValue lntcNamespace = gt.findOrCreateTable("luanatic");
            lntcNamespace.registerFunction("class", detail::luanaticClassFunction);
            lntcNamespace.registerFunction("isBaseOf", detail::luanaticIsBaseOf);
            lntcNamespace.registerFunction("isInstanceOf", detail::luanaticIsInstanceOf);
            //lntcNamespace.registerFunction("ensureClass", detail::gluaEnsureClass);
            lntcNamespace.reset(); //reset to pop off the key from the stack
            lua_pop(_state, 1);
        }
        lua_pop(_state, 1);
    }

    inline void openStandardLibraries(lua_State * _state)
    {
        luaL_openlibs(_state);
    }

    inline LuaValue globalsTable(lua_State * _state)
    {
        detail::StackPop popper(_state, 1);
        detail::pushGlobalsTable(_state);
        return LuaValue(_state, -1);
    }

    inline LuaValue registryTable(lua_State * _state)
    {
        detail::StackPop popper(_state, 1);
        lua_pushvalue(_state, LUA_REGISTRYINDEX);
        return LuaValue(_state, -1);
    }

    inline stick::Error execute(lua_State * _state, const stick::String & _luaCode)
    {
        stick::Int32 result = luaL_dostring(_state, _luaCode.cString());
        if (result)
            return stick::Error(stick::ec::InvalidOperation, lua_tostring(_state, -1), STICK_FILE, STICK_LINE);

        return stick::Error();
    }

    inline void addPackagePath(lua_State * _state, const stick::String & _path)
    {
        detail::pushGlobalsTable(_state);
        lua_getfield(_state, -1, "package");
        if (lua_isnil(_state, -1))
        {
            lua_pop(_state, 1);
            lua_pushliteral(_state, "package");
            lua_newtable(_state);
            lua_pushliteral(_state, "path");
            lua_pushliteral(_state, "");
            lua_settable(_state, -3);
            lua_settable(_state, -3);
            lua_getfield(_state, -1, "package");
        }
        lua_getfield(_state, -1, "path"); //package path

        const char * currentPath = luaL_checkstring(_state, -1);
        stick::String path = stick::String::concat(currentPath, ";", _path);
        lua_pushstring(_state, path.cString()); //package path newPath
        lua_setfield(_state, -3, "path");       //package path
        lua_pop(_state, 3);                     //
    }
}

#endif // LUANATIC_LUANATIC_HPP
