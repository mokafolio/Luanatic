#ifndef LUANATIC_LUANATIC_HPP
#define LUANATIC_LUANATIC_HPP

#include <Stick/DynamicArray.hpp>
#include <Stick/Error.hpp>
#include <Stick/HashMap.hpp>
#include <Stick/StringConversion.hpp>
#include <Stick/TypeInfo.hpp>
#include <Stick/UniquePtr.hpp>
#include <Stick/Maybe.hpp>
#include <Stick/Result.hpp>
#include <Stick/URI.hpp>

#include <type_traits>
#include <functional> //for std::ref
#include <tuple>
#include <cstdint>
#include <cstring>

#include <cxxabi.h>

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

#define LUANATIC_KEY "__Luanatic"
#define LUANATIC_FUNCTION_1(x){\
 &luanatic::detail::FunctionWrapper<decltype(x), x>::func,\
 &luanatic::detail::FunctionWrapper<decltype(x), x>::score,\
 &luanatic::detail::FunctionWrapper<decltype(x), x>::signatureStr,\
 &luanatic::detail::FunctionWrapper<decltype(x), x>::argCount\
}
#define LUANATIC_FUNCTION_P(x, ...){\
&luanatic::detail::FunctionWrapper<decltype(x), x, __VA_ARGS__>::func,\
&luanatic::detail::FunctionWrapper<decltype(x), x, __VA_ARGS__>::score,\
&luanatic::detail::FunctionWrapper<decltype(x), x, __VA_ARGS__>::signatureStr,\
&luanatic::detail::FunctionWrapper<decltype(x), x, __VA_ARGS__>::argCount\
}
#define LUANATIC_GET_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,NAME,...) NAME
#define LUANATIC_FUNCTION(...) LUANATIC_GET_MACRO(__VA_ARGS__, \
LUANATIC_FUNCTION_P, \
LUANATIC_FUNCTION_P, \
LUANATIC_FUNCTION_P, \
LUANATIC_FUNCTION_P, \
LUANATIC_FUNCTION_P, \
LUANATIC_FUNCTION_P, \
LUANATIC_FUNCTION_P, \
LUANATIC_FUNCTION_P, \
LUANATIC_FUNCTION_P, \
LUANATIC_FUNCTION_1)(__VA_ARGS__)
#define LUANATIC_FUNCTION_OVERLOAD_2(sig, x) {\
 &luanatic::detail::FunctionWrapper<sig, x>::func,\
 &luanatic::detail::FunctionWrapper<sig, x>::score,\
 &luanatic::detail::FunctionWrapper<sig, x>::signatureStr,\
 &luanatic::detail::FunctionWrapper<sig, x>::argCount\
}
#define LUANATIC_FUNCTION_OVERLOAD_P(sig, x, ...) {\
&luanatic::detail::FunctionWrapper<sig, x, __VA_ARGS__>::func,\
&luanatic::detail::FunctionWrapper<sig, x, __VA_ARGS__>::score,\
&luanatic::detail::FunctionWrapper<sig, x, __VA_ARGS__>::signatureStr,\
&luanatic::detail::FunctionWrapper<sig, x, __VA_ARGS__>::argCount\
}
#define LUANATIC_FUNCTION_OVERLOAD(...) LUANATIC_GET_MACRO(__VA_ARGS__, \
LUANATIC_FUNCTION_OVERLOAD_P, \
LUANATIC_FUNCTION_OVERLOAD_P, \
LUANATIC_FUNCTION_OVERLOAD_P, \
LUANATIC_FUNCTION_OVERLOAD_P, \
LUANATIC_FUNCTION_OVERLOAD_P, \
LUANATIC_FUNCTION_OVERLOAD_P, \
LUANATIC_FUNCTION_OVERLOAD_P, \
LUANATIC_FUNCTION_OVERLOAD_P, \
LUANATIC_FUNCTION_OVERLOAD_2)(__VA_ARGS__)
#define LUANATIC_ATTRIBUTE(x) luanatic::detail::AttributeWrapper<decltype(x), x>::func
//#define LUANATIC_FUNCTION_DEFAULT_ARGS(x) luanatic::detail::DefaultArgFunctionWrapper<decltype(x), x>::func

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

    template <class T>
    inline stick::Int32 conversionScore(lua_State * _luaState, stick::Int32 _index);

    template <class T, class WT>
    inline bool pushWrapped(lua_State * _luaState, const WT * _obj, bool _bLuaOwnsObject = true);

    template <class T>
    inline bool push(lua_State * _luaState, const T * _obj, bool _bLuaOwnsObject = true);

    template <class T>
    inline stick::Int32 pushValueType(lua_State * _luaState, const T & _value);

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
        //function signature that computes an argument score
        //based on the current arguments on the stack (for signature matching)
        typedef stick::Int32 (*ArgScoreFunction) (lua_State *, stick::Int32, stick::Int32, stick::Int32 *);

        typedef stick::String (*SignatureStrFunction) (void);

        typedef stick::Size (*ArgCountFunction) (void);


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
            luaL_error(_state, _fmt, _args...);
        }

        // returns the raw type of T, removing pointer, reference and
        // const volatile. This is the behavior we want mainly for
        // custom registered types.
        template <class T>
        struct RawType
        {
            using Type = typename std::remove_cv<typename std::remove_pointer<
                         typename std::remove_reference<T>::type>::type>::type;
        };

        // for const char *  we don't want to remove const or pointer
        // as picking proper conversion of these types relies on that
        // extra information.
        template <>
        struct RawType<const char *>
        {
            using Type = const char*;
        };

        struct DefaultArgsBase
        {
            virtual ~DefaultArgsBase() = default;
            virtual void push(lua_State * _state, stick::Int32 _count) const = 0;
            virtual stick::Size argCount() const = 0;
        };

        template<class...Args>
        struct DefaultArgs;
    }

    struct STICK_API LuanaticFunction
    {
        LuanaticFunction(lua_CFunction _a = nullptr,
                         detail::ArgScoreFunction _b = nullptr,
                         detail::SignatureStrFunction _c = nullptr,
                         detail::ArgCountFunction _d = nullptr,
                         detail::DefaultArgsBase * _e = nullptr) :
            function(_a),
            scoreFunction(_b),
            signatureStrFunction(_c),
            argCountFunction(_d),
            defaultArgs(_e)
        {

        }

        lua_CFunction function;
        detail::ArgScoreFunction scoreFunction;
        detail::SignatureStrFunction signatureStrFunction;
        detail::ArgCountFunction argCountFunction;
        detail::DefaultArgsBase * defaultArgs;
    };

    namespace detail
    {
        template<class U, class Enable = void>
        struct DefaultValueTypeConverterImpl
        {
            static U convertAndCheck(lua_State * _state, stick::Int32 _index)
            {
                detail::luaErrorWithStackTrace(_state, _index, "No ValueTypeConverter implementation found.");
                return U();
            }

            static stick::Int32 push(lua_State * _state, const U & _value)
            {
                luaL_error(_state, "No ValueTypeConverter implementation found to push.");
                return 1;
            }
        };

        template<class U>
        struct DefaultValueTypeConverterImpl<U, typename std::enable_if < std::is_copy_constructible<U>::value>::type>
        {
            static U convertAndCheck(lua_State * _state, stick::Int32 _index)
            {
                detail::luaErrorWithStackTrace(_state, _index, "No ValueTypeConverter implementation found.");
                return U();
            }

            //implemented further down, as we need detail::LuanaticState for memory allocation
            static stick::Int32 push(lua_State * _state, const U & _value);
        };
    }

    template <class U, class Enable = void>
    struct DefaultValueTypeConverter
    {
        static constexpr bool __defaultConverterImpl = true;

        static U convertAndCheck(lua_State * _state, stick::Int32 _index)
        {
            return detail::DefaultValueTypeConverterImpl<U>::convertAndCheck(_state, _index);
        }

        //implemented further down, as we need detail::LuanaticState for memory allocation
        static stick::Int32 push(lua_State * _state, const U & _value)
        {
            return detail::DefaultValueTypeConverterImpl<U>::push(_state, _value);
        }
    };

    template <class U, class Enable = void>
    struct ValueTypeConverter
    {
        static constexpr bool __defaultConverterImpl = true;
    };

    template <class U>
    struct ValueTypeConverter<stick::UniquePtr<U>>
    {
        static U convertAndCheck(lua_State * _state, stick::Int32 _index)
        {
            //hmmmm? Not sure if this is the way to go in this case.
            detail::luaErrorWithStackTrace(_state, _index, "No ValueTypeConverter implementation found.");
            return U();
        }

        //implemented further down, as we need detail::LuanaticState for memory allocation
        static stick::Int32 push(lua_State * _state, const stick::UniquePtr<U> & _value);
    };

    template <>
    struct ValueTypeConverter<bool>
    {
        static bool convertAndCheck(lua_State * _state, stick::Int32 _index)
        {
            return lua_toboolean(_state, _index) == 1;
        }

        static stick::Int32 push(lua_State * _state, const bool & _value)
        {
            lua_pushboolean(_state, _value);
            return 1;
        }
    };

    template <>
    struct ValueTypeConverter<stick::Int32>
    {
        static stick::Int32 convertAndCheck(lua_State * _luaState,
                                            stick::Int32 _index)
        {
            return static_cast<stick::Int32>(luaL_checkinteger(_luaState, _index));
        }

        static stick::Int32 push(lua_State * _luaState, const stick::Int32 & _value)
        {
            lua_pushinteger(_luaState, _value);
            return 1;
        }
    };

    template <>
    struct ValueTypeConverter<stick::Int16>
    {
        static stick::Int16 convertAndCheck(lua_State * _luaState,
                                            stick::Int32 _index)
        {
            return static_cast<stick::Int16>(luaL_checkinteger(_luaState, _index));
        }

        static stick::Int32 push(lua_State * _luaState, const stick::Int16 & _value)
        {
            lua_pushinteger(_luaState, _value);
            return 1;
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

        static stick::Int32 push(lua_State * _luaState, const stick::UInt32 & _value)
        {
            lua_pushinteger(_luaState, _value);
            return 1;
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

        static stick::Int32 push(lua_State * _luaState, const stick::Size & _value)
        {
            lua_pushinteger(_luaState, _value);
            return 1;
        }
    };

    template <>
    struct ValueTypeConverter<stick::Float32>
    {
        static stick::Float32 convertAndCheck(lua_State * _luaState, stick::Int32 _index)
        {
            return static_cast<stick::Float32>(luaL_checknumber(_luaState, _index));
        }

        static stick::Int32 push(lua_State * _luaState, const stick::Float32 & _number)
        {
            lua_pushnumber(_luaState, _number);
            return 1;
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

        static stick::Int32 push(lua_State * _luaState, const stick::Float64 & _number)
        {
            lua_pushnumber(_luaState, _number);
            return 1;
        }
    };

    template <>
    struct ValueTypeConverter<stick::String>
    {
        static stick::String convertAndCheck(lua_State * _luaState,
                                             stick::Int32 _index)
        {
            return stick::String(luaL_checkstring(_luaState, _index));
        }

        static stick::Int32 push(lua_State * _luaState, const stick::String & _str)
        {
            lua_pushstring(_luaState, _str.cString());
            return 1;
        }
    };

    template <class T>
    struct ValueTypeConverter <T, typename std::enable_if<std::is_enum<T>::value>::type >
    {
        static T convertAndCheck(lua_State * _luaState, stick::Int32 _index)
        {
            return static_cast<T>(luaL_checkinteger(_luaState, _index));
        }

        static stick::Int32 push(lua_State * _luaState, const T & _value)
        {
            lua_pushinteger(_luaState, static_cast<stick::Int32>(_value));
            return 1;
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

        static stick::Int32 push(lua_State * _luaState, const stick::DynamicArray<T> & _array)
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
            return 1;
        }
    };

    //Value Converter for stick::Error
    template<>
    struct ValueTypeConverter<stick::Error>
    {
        static stick::Error convertAndCheck(lua_State * _state, stick::Int32 _index)
        {
            if (lua_isnil(_state, _index))
                return stick::Error();

            if (!lua_istable(_state, _index))
                detail::luaErrorWithStackTrace(_state, _index, "Table expected to convert to stick::Error");

            const char * message, * dsc, * fn;
            message = dsc = fn = nullptr;
            stick::Int32 line = 0;
            stick::Int32 code = -1;
            stick::ErrorCategory * category = nullptr;
            auto top = lua_gettop(_state);
            lua_getfield(_state, top, "message");
            if (!lua_isnil(_state, -1))
                message = luaL_checkstring(_state, -1);
            // lua_getfield(_state, _index, "description");
            // if (!lua_isnil(_state, -1))
            //     dsc = lua_tostring(_state, -1);
            lua_getfield(_state, top, "file");
            if (!lua_isnil(_state, -1))
                fn = luaL_checkstring(_state, -1);
            lua_getfield(_state, top, "line");
            if (!lua_isnil(_state, -1))
                line = luaL_checkinteger(_state, -1);
            lua_getfield(_state, top, "code");
            if (!lua_isnil(_state, -1))
                code = luaL_checkinteger(_state, -1);
            lua_getfield(_state, top, "category");
            if (!lua_isnil(_state, -1) && lua_isuserdata(_state, -1))
                category = reinterpret_cast<stick::ErrorCategory *>(lua_touserdata(_state, -1));
            lua_pop(_state, 5);
            if (category)
                return stick::Error(code, *category, message ? message : "", fn ? fn : "", line);
            else
                return stick::Error();
        }

        static stick::Int32 push(lua_State * _state, const stick::Error & _error)
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
                lua_pushlightuserdata(_state, (void *)&_error.category());
                lua_setfield(_state, -2, "category");
            }
            return 1;
        }
    };

    template<class T>
    class HasValueTypeConverter
    {
        template < class U, class = typename std::enable_if < !std::is_member_pointer<decltype(&ValueTypeConverter<U>::__defaultConverterImpl)>::value >::type >
        static std::false_type check(int);

        template <class>
        static std::true_type check(...);

    public:

        static constexpr bool value = decltype(check<T>(0))::value;
    };

    /*template<class T>
    struct ValueTypeConverter < T, typename std::enable_if < std::is_copy_constructible<T>::value &&
        !HasValueTypeConverter<T>::value >::type >
    {
        static constexpr bool __defaultConverterImpl = true;
        static constexpr bool __defaultConverterImplTwo = true;

        static T convertAndCheck(lua_State * _state, stick::Int32 _index)
        {
            detail::luaErrorWithStackTrace(_state, _index, "No ValueTypeConverter implementation found.");
            return T();
        }

        //implemented further down, as we need detail::LuanaticState for memory allocation
        static void push(lua_State * _state, const T & _value);
    };*/

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

        //@TODO: make a simplified TypeCaster that is exposed to allow
        //casts from custom wrappers in an easy way.
        template <class From, class To>
        struct TypeCaster
        {
            static UserData cast(const UserData & _ud, detail::LuanaticState & _state)
            {
                return {static_cast<To *>(static_cast<From *>(_ud.m_data)), _ud.m_bOwnedByLua, stick::TypeInfoT<To>::typeID()};
            }
        };

        template <class From, class To>
        struct TypeCaster<stick::UniquePtr<From>, To>
        {
            static UserData cast(const UserData & _ud, detail::LuanaticState & _state)
            {
                return {(To *)static_cast<stick::UniquePtr<From> *>(_ud.m_data)->get(), _ud.m_bOwnedByLua, stick::TypeInfoT<To>::typeID()};
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
            LuanaticFunction function;
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
            // if (_other.m_storageWrapperTmp)
            //     m_storageWrapperTmp = _other.m_storageWrapperTmp->clone();
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

            // if (_other.m_storageWrapperTmp)
            //     m_storageWrapperTmp = _other.m_storageWrapperTmp->clone();

            return *this;
        }

        virtual ~ClassWrapperBase() {}

        ClassWrapperBase & addMemberFunction(const stick::String & _name,
                                             LuanaticFunction _function)
        {
            m_members.append({ _name, _function });
            return *this;
        }

        template<class...Args>
        ClassWrapperBase & addMemberFunction(const stick::String & _name,
                                             LuanaticFunction _function, Args..._args)
        {
            _function.defaultArgs = stick::defaultAllocator().create<detail::DefaultArgs<Args...>>(_args...);
            m_members.append({ _name, _function });
            return *this;
        }

        ClassWrapperBase & addMemberFunction(const stick::String & _name,
                                             lua_CFunction _function)
        {
            m_members.append({ _name, {_function, NULL, NULL}});
            return *this;
        }

        ClassWrapperBase & addStaticFunction(const stick::String & _name,
                                             LuanaticFunction _function)
        {
            m_statics.append({ _name, _function });
            return *this;
        }

        template<class...Args>
        ClassWrapperBase & addStaticFunction(const stick::String & _name,
                                             LuanaticFunction _function, Args..._args)
        {
            _function.defaultArgs = stick::defaultAllocator().create<detail::DefaultArgs<Args...>>(_args...);
            m_statics.append({ _name, _function });
            return *this;
        }

        ClassWrapperBase & addStaticFunction(const stick::String & _name,
                                             lua_CFunction _function)
        {
            m_statics.append({ _name, {_function, NULL, NULL}});
            return *this;
        }

        ClassWrapperBase & addAttribute(const stick::String & _name,
                                        lua_CFunction _function)
        {
            m_attributes.append({ _name, {_function, NULL}});
            return *this;
        }

        void addCast(const detail::UserDataCastFunction & _cast)
        {
            m_casts.append(_cast);
        }

        stick::TypeID typeID() const { return m_typeID; }

        // virtual ClassWrapperUniquePtr clone() const = 0;

        stick::TypeID m_typeID;
        stick::TypeID m_storageTypeID; //used for custom wrappers around the obj
        stick::String m_className;
        NamedLuaFunctionArray m_members;
        NamedLuaFunctionArray m_statics;
        NamedLuaFunctionArray m_attributes;
        NamedLuaFunctionArray m_constructors;
        BaseArray m_bases;
        CastArray m_casts;
        //used if StorageT is not detail::RawPointerStorageFlag (see ClassWrapper) to describe the kind of storage and casts
        // ClassWrapperUniquePtr m_storageWrapperTmp; //this is only used before the class is registered to store the storage wrapper
    };

    template <class T>
    class ClassWrapper : public ClassWrapperBase
    {
    public:
        using ClassType = T;

        ClassWrapper(const stick::String & _name);

        // ~ClassWrapper();

        template<class...Args>
        ClassWrapper & addConstructor();

        template <class... Args>
        ClassWrapper & addConstructor(const stick::String & _str);

        template <class CD>
        ClassWrapper & addBase();

        template <class CD>
        ClassWrapper & addCast();

        // ClassWrapperUniquePtr clone() const final
        // {
        //     //we just use default allocator here for now...
        //     return stick::makeUnique<ClassWrapper>(stick::defaultAllocator(), *this);
        // }
    };

    namespace detail
    {
        //@TODO: LuanaticState should be STICK_API and in main namespace
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
            stick::DynamicArray<stick::UniquePtr<DefaultArgsBase>> m_defaultArgStorage;
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
                auto & casts = (*it).value.wrapper->m_casts;
                auto sit = casts.begin();

                for (; sit != casts.end(); ++sit)
                {
                    if ((*sit).m_typeID == stick::TypeInfoT<T>::typeID())
                        return true;

                    ret = checkBases<T>(_luanaticState, (*sit).m_typeID);
                    if (ret)
                        break;
                }
            }

            return ret;
        }

        struct FindCastFunctionResult
        {
            stick::Int32 castCount;
            UserData userData;
        };

        inline FindCastFunctionResult findCastFunctionImpl(LuanaticState & _luanaticState, const UserData & _currentUserData, stick::TypeID _targetTypeID, stick::Int32 _depth)
        {
            UserData ret = _currentUserData;

            auto fit = _luanaticState.m_typeIDClassMap.find(ret.m_typeID);
            if (fit == _luanaticState.m_typeIDClassMap.end())
                return { -1, ret};
            auto & casts = fit->value.wrapper->m_casts;
            auto it = casts.begin();

            // check direct bases
            for (; it != casts.end(); ++it)
            {
                ret = (*it).m_cast(_currentUserData, _luanaticState);
                if (ret.m_typeID == _targetTypeID)
                {
                    return { _depth, ret };
                }
            }

            it = casts.begin();

            // recursively walk the inheritance tree
            for (; it != casts.end(); ++it)
            {
                ret = (*it).m_cast(_currentUserData, _luanaticState);
                auto res = findCastFunctionImpl(_luanaticState, ret, _targetTypeID, _depth + 1);
                if (res.userData.m_typeID == _targetTypeID)
                    return res;
            }

            return { -1, ret };
        }

        inline FindCastFunctionResult findCastFunction(LuanaticState & _luanaticState, const UserData & _currentUserData, stick::TypeID _targetTypeID)
        {
            return findCastFunctionImpl(_luanaticState, _currentUserData, _targetTypeID, 0);
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

        using Overloads = stick::DynamicArray<LuanaticFunction>;

        struct EvaluatedOverload
        {
            LuanaticFunction function;
            stick::Int32 defArgsToPush;
        };

        inline stick::Int32 callOverloadedFunction(lua_State * _luaState)
        {
            stick::Int32 argCount = lua_gettop(_luaState);
            lua_pushvalue(_luaState, lua_upvalueindex(1));
            STICK_ASSERT(lua_isuserdata(_luaState, -1));
            Overloads * overloads = (Overloads *)lua_touserdata(_luaState, -1);
            lua_pop(_luaState, 1);
            EvaluatedOverload candidates[16];
            stick::Int32 defArgsToPush = 0;
            stick::Size idx = 0;
            stick::Int32 bestScore = std::numeric_limits<stick::Int32>::max();
            for (auto it = overloads->begin(); it != overloads->end(); ++it)
            {
                stick::Int32 score = (*it).scoreFunction(_luaState, argCount, (*it).defaultArgs ? (*it).defaultArgs->argCount() : 0, &defArgsToPush);
                if (score != std::numeric_limits<stick::Int32>::max()  && score == bestScore)
                {
                    candidates[idx++] = {*it, defArgsToPush};
                }
                else if (score < bestScore)
                {
                    idx = 0;
                    candidates[idx++] = {*it, defArgsToPush};
                    bestScore = score;
                }

                if (idx == 15)
                    break;
            }

            if (!idx)
            {
                stick::String str;
                for (stick::Size i = 0; i < overloads->count(); i++)
                {
                    if (i < overloads->count() - 1)
                        str.append(stick::AppendVariadicFlag(), "   ", (*overloads)[i].signatureStrFunction(), ",\n");
                    else
                        str.append(stick::AppendVariadicFlag(), "   ", (*overloads)[i].signatureStrFunction(), "\n");
                }
                luaErrorWithStackTrace(_luaState, 1, "\nCould not find candidate for overloaded function, Candidates:\n%s", str.cString());
            }
            else if (idx > 1)
            {
                stick::String str;
                for (stick::Int32 i = 0; i < idx; i++)
                {
                    if (i < idx - 1)
                        str.append(stick::AppendVariadicFlag(), "   ", candidates[i].function.signatureStrFunction(), ",\n");
                    else
                        str.append(stick::AppendVariadicFlag(), "   ", candidates[i].function.signatureStrFunction(), "\n");
                }
                luaErrorWithStackTrace(_luaState, 1, "\nAmbiguous call to overloaded function, Candidates:\n%s", str.cString());
            }
            else
            {
                //call the function we found it, make sure to push the default arguments if any
                if (candidates[0].function.defaultArgs && candidates[0].defArgsToPush > 0)
                {
                    candidates[0].function.defaultArgs->push(_luaState, candidates[0].defArgsToPush);
                }
                return candidates[0].function.function(_luaState);
            }

            return 0;
        }

        inline stick::Int32 callConstructorNice(lua_State * _luaState)
        {
            //for nice constructor we need to pop the first stack item
            lua_remove(_luaState, 1);

            stick::Int32 argCount = lua_gettop(_luaState);

            //push the actual constructor onto the stack
            stick::Int32 idx = lua_upvalueindex(1);
            lua_pushvalue(_luaState, idx);

            //insert it before the arguments
            lua_insert(_luaState, -1 - argCount);

            //and call it
            lua_call(_luaState, argCount, 1);

            return 1;
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

            //if there is no overload table in target scope, create it
            lua_getfield(_luaState, _targetTableIndex, "__overloads");
            if (lua_isnil(_luaState, -1))
            {
                lua_newtable(_luaState); // ... CT mT {}
                lua_setfield(_luaState, _targetTableIndex, "__overloads"); // ... CT mT
            }
            lua_pop(_luaState, 1);


            LuanaticState * lstate = luanaticState(_luaState);
            STICK_ASSERT(lstate);

            for (; it != _functions.end(); ++it)
            {
                name = _nameReplace.length() ? _nameReplace.cString()
                       : (*it).name.cString();
                lua_getfield(_luaState, -1, name); // ... CT mT mT[name]

                //the luastate owns the default args
                if ((*it).function.defaultArgs)
                    lstate->m_defaultArgStorage.append(stick::UniquePtr<DefaultArgsBase>((*it).function.defaultArgs, stick::defaultAllocator()));

                if (lua_isnil(_luaState, -1))
                {
                    //copy the function into the overloaded table (this is kinda not super memory efficient)
                    //if you don't use overloaded functions, but we need to store it somewhere, better ideas?
                    //@TODO: Better ideas that don't use this extra memory and don't compromise speed?
                    lua_getfield(_luaState, _targetTableIndex, "__overloads"); // ... CT mT nil __overloads
                    pushUnregisteredType(_luaState, Overloads()); // ... CT mT nil __overloads ola
                    lua_pushvalue(_luaState, -1); // ... CT mT nil __overloads ola ola
                    Overloads * overloads = (Overloads *)lua_touserdata(_luaState, -1);
                    overloads->append((*it).function);

                    lua_setfield(_luaState, -3, name); // ... CT mT nil __overloads ola

                    if (!(*it).function.defaultArgs)
                    {
                        lua_pop(_luaState, 2); // ... CT mT nil
                        lua_pushstring(_luaState, name); // ... CT mT nil name
                        lua_pushcfunction(_luaState, (*it).function.function); // ... CT mT nil name func
                        if (_bNiceConstructor)
                        {
                            lua_pushcclosure(_luaState, callConstructorNice, 1);
                        }
                        lua_settable(_luaState, -4); // ... CT mT nil
                        lua_pop(_luaState, 1); // ... CT mT
                    }
                    else
                    {
                        lua_pushcclosure(_luaState, callOverloadedFunction, 1); // ... CT mT nil __overloads closure
                        if (_bNiceConstructor)
                        {
                            lua_pushcclosure(_luaState, callConstructorNice, 1);
                        }
                        lua_setfield(_luaState, -4, name); // ... CT mT __overloads nil
                        lua_pop(_luaState, 2); // ... CT mT g
                    }
                }
                else
                {
                    lua_getfield(_luaState, _targetTableIndex, "__overloads"); // ... CT mT __overloads
                    lua_getfield(_luaState, -1, name); // ... CT mT __overloads namefield
                    STICK_ASSERT(lua_isuserdata(_luaState, -1));
                    Overloads * overloads = (Overloads *)lua_touserdata(_luaState, -1);
                    overloads->append((*it).function);

                    //push the c closure and set the overloaded
                    lua_pushcclosure(_luaState, callOverloadedFunction, 1); // ... CT mT __overloads namefield closure
                    if (_bNiceConstructor)
                    {
                        lua_pushcclosure(_luaState, callConstructorNice, 1);
                    }
                    lua_setfield(_luaState, -4, name); // ... CT mT __overloads namefield
                    lua_pop(_luaState, 2); // ... CT mT
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

            ClassWrapperUniquePtr cl = stick::makeUnique<CW>(*state->m_allocator, _wrapper);
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

            //add a metatable to the class table with a __call metamethod to allow nice constructors such as MyClass()
            //instead of MyClass.new()
            lua_newtable(_state); // ... CT {}
            stick::Int32 classMetaTable = lua_gettop(_state);
            registerFunctions(_state, classMetaTable, _wrapper.m_constructors, "__call", true);
            lua_setmetatable(_state, classTable); // ... CT

            //register static functions in class table
            registerFunctions(_state, classTable, _wrapper.m_statics);

            lua_pushliteral(_state, "__typeID");            // ... CT __typeID
            lua_pushlightuserdata(_state, myTypeID);        // ... CT __typeID id
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

    namespace detail
    {
        template <class U>
        stick::Int32 DefaultValueTypeConverterImpl < U, typename std::enable_if < std::is_copy_constructible<U>::value>::type >::push(lua_State * _state, const U & _value)
        {
            //By default we try pushing this as if it was a registered type,
            //hoping that it will succeed.
            detail::LuanaticState * state = detail::luanaticState(_state);
            STICK_ASSERT(state);
            luanatic::push(_state, state->m_allocator->create<U>(_value), true);
            return 1;
        }
    }

    template <class U>
    stick::Int32 ValueTypeConverter <stick::UniquePtr<U>>::push(lua_State * _state, const stick::UniquePtr<U> & _value)
    {
        detail::LuanaticState * state = detail::luanaticState(_state);
        STICK_ASSERT(state);
        //damn...this const cast feels hella sketchy let's see if we can come up with a better way to push
        //unique pointers from value. (I think this might be as good as it gets)
        luanatic::push(_state, state->m_allocator->create<stick::UniquePtr<U>>(std::move(const_cast<stick::UniquePtr<U>&>(_value))), true);
        return 1;
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
                stick::TypeID tid = (stick::TypeID)lua_touserdata(_luaState, -1);
                if (tid == stick::TypeInfoT<T>::typeID())
                {
                    ret = true;
                }

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

    namespace detail
    {
        //Note: Lua < 5.3 does not have isinteger function
#if LUA_VERSION_NUM < 503
        int lua_isinteger (lua_State * _state, stick::Int32 _index)
        {
            if (lua_type(_state, _index) == LUA_TNUMBER)
            {
                lua_Number n = lua_tonumber(_state, _index);
                lua_Integer i = lua_tointeger(_state, _index);
                if (i == n)
                    return 1;
            }
            return 0;
        }
#endif

        //helpers to generate conversion scores for default lua types and basic c++ value types
        template<class T>
        struct LuaTypeScore
        {
            static stick::Int32 score(lua_State * _luaState, stick::Int32 _index)
            {
                //@TODO: Check if we need to return a different value if a value type
                // converter exists to avoid ambiguities.
                //@TODO: HasValueTypeConverter will return true for any DynamicArray
                //without checking for the type that is requested. There should be
                //some check to validate that the requested type inside the DynamicArray
                //can be converted, too.
                if (HasValueTypeConverter<T>::value)
                    return 1;
                return std::numeric_limits<stick::Int32>::max();
            }
        };

        template<>
        struct LuaTypeScore<stick::UInt32>
        {
            static stick::Int32 score(lua_State * _luaState, stick::Int32 _index)
            {
                lua_Integer i = lua_tointeger(_luaState, _index);
                if (lua_isinteger(_luaState, _index) && i >= 0)
                    return 0;
                else if (lua_isnumber(_luaState, _index) && i >= 0)
                    return 1;
                return std::numeric_limits<stick::Int32>::max();
            }
        };

        template<>
        struct LuaTypeScore<stick::Int32>
        {
            static stick::Int32 score(lua_State * _luaState, stick::Int32 _index)
            {
                if (lua_isinteger(_luaState, _index))
                    return 0;
                else if (lua_isnumber(_luaState, _index))
                    return 1;
                return std::numeric_limits<stick::Int32>::max();
            }
        };

        template<>
        struct LuaTypeScore<stick::Float32>
        {
            static stick::Int32 score(lua_State * _luaState, stick::Int32 _index)
            {
                if (lua_type(_luaState, _index) == LUA_TNUMBER)
                {
                    return 0;
                }
                else if (lua_isnumber(_luaState, _index))
                    return 1;
                return std::numeric_limits<stick::Int32>::max();
            }
        };

        template<>
        struct LuaTypeScore<stick::Float64>
        {
            static stick::Int32 score(lua_State * _luaState, stick::Int32 _index)
            {
                if (lua_type(_luaState, _index) == LUA_TNUMBER)
                {
                    return 0;
                }
                else if (lua_isnumber(_luaState, _index))
                    return 1;
                return std::numeric_limits<stick::Int32>::max();
            }
        };

        template<>
        struct LuaTypeScore<stick::String>
        {
            static stick::Int32 score(lua_State * _luaState, stick::Int32 _index)
            {
                if (lua_type(_luaState, _index) == LUA_TSTRING)
                {
                    return 0;
                }
                else if (lua_isstring(_luaState, _index))
                    return 1;
                return std::numeric_limits<stick::Int32>::max();
            }
        };

        template<>
        struct LuaTypeScore<const char *>
        {
            static stick::Int32 score(lua_State * _luaState, stick::Int32 _index)
            {
                if (lua_type(_luaState, _index) == LUA_TSTRING)
                {
                    return 0;
                }
                else if (lua_isstring(_luaState, _index))
                    return 1;
                return std::numeric_limits<stick::Int32>::max();
            }
        };
    }

    template <class T>
    inline stick::Int32 conversionScore(lua_State * _luaState, stick::Int32 _index)
    {
        using RT = typename detail::RawType<T>::Type;
        if (!lua_isuserdata(_luaState, _index))
        {
            return detail::LuaTypeScore<RT>::score(_luaState, _index);
        }
        else if (isOfType<RT>(_luaState, _index, true))
        {
            return 0;
        }
        else
        {
            detail::UserData * pud = static_cast<detail::UserData *>(lua_touserdata(_luaState, _index));
            detail::LuanaticState * glua = detail::luanaticState(_luaState);
            STICK_ASSERT(glua != nullptr);
            auto cc = detail::findCastFunctionImpl(*glua, *pud, stick::TypeInfoT<RT>::typeID(), 1).castCount;
            if (cc != -1)
                return cc;
            else return std::numeric_limits<stick::Int32>::max();
        }
        return std::numeric_limits<stick::Int32>::max();
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

                auto result = detail::findCastFunction(*glua, *pud, stick::TypeInfoT<T>::typeID());
                if (result.castCount != -1)
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
                detail::luaErrorWithStackTrace(_luaState, _index, "%s expected, got %s",
                                               glua->m_typeIDClassMap[stick::TypeInfoT<T>::typeID()].wrapper->m_className.cString(),
                                               luaL_typename(_luaState, _index));
            }
            else
            {
                detail::luaErrorWithStackTrace(_luaState, _index, "Different unregistered type expected, got %s", luaL_typename(_luaState, _index));
            }
        }

        return ret;
    }

    namespace detail
    {
        template<class T>
        struct PickValueTypeConverter
        {
            using Converter = typename std::conditional<HasValueTypeConverter<T>::value, ValueTypeConverter<T>, DefaultValueTypeConverter<T>>::type;
        };
    }

    template <class T>
    inline T convertToValueTypeAndCheck(lua_State * _luaState, stick::Int32 _index)
    {
        // check if we can simply make a copy
        // @TODO: Provide a way to skip this (i.e. ValueTypeConverter allready does this step)
        T * other = convertToType<T>(_luaState, _index);
        if (other)
        {
            return *other;
        }

        // check if there is a value converted implemented
        return detail::PickValueTypeConverter<T>::Converter::convertAndCheck(_luaState, _index);
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

                //check if we find the object in the weaktable
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
                        return false;
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
                    // if this type is not found in the inheritance chain,
                    // we can assume, that the currently pushed type sits higher in the inheritance
                    // chain, so we therefore update the type id associated with this instance.
                    if (std::is_polymorphic<T>() && !isOfType<WT>(_luaState, -1, false))
                    {
                        // update the type id
                        detail::UserData * userData = static_cast<detail::UserData *>(lua_touserdata(_luaState, -1));
                        userData->m_typeID = stick::TypeInfoT<WT>::typeID();
                        // update the metatable
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
                    }
                    //and return the existing user data associated with this instance
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
    inline stick::Int32 pushValueType(lua_State * _luaState, const T & _value)
    {
        return detail::PickValueTypeConverter<T>::Converter::push(_luaState, _value);
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

    namespace placeHolders
    {
        struct Result {};
        struct One {};
        struct Two {};
        struct Three {};
        struct Four {};
        struct Five {};
        struct Six {};
        struct Seven {};
        struct Eight {};
        struct Nine {};
        struct Ten {};
    }

    namespace ph = placeHolders;

    namespace detail
    {
        struct NoPolicy {};

        template <class P>
        struct ApplyPolicyPtr
        {
            template<class T>
            static stick::Int32 apply(lua_State * _luaState, T * _val, const P & _policy)
            {
                return _policy.template push<typename RawType<T>::Type>(_luaState, _val);
            }
        };

        template <>
        struct ApplyPolicyPtr<NoPolicy>
        {
            template<class T>
            static stick::Int32 apply(lua_State * _luaState, T * _val, const NoPolicy & _p)
            {
                if (!_val)
                    lua_pushnil(_luaState);
                else
                    luanatic::push<typename RawType<T>::Type>(_luaState, _val, false);
                return 1;
            }
        };


        template <class P>
        struct ApplyPolicyValueType
        {
            template<class T>
            static stick::Int32 apply(lua_State * _luaState, const T & _val, const P & _policy)
            {
                return _policy.template push<typename RawType<T>::Type>(_luaState, _val);
            }
        };

        template <>
        struct ApplyPolicyValueType<NoPolicy>
        {
            template<class T>
            static stick::Int32 apply(lua_State * _luaState, const T & _val, const NoPolicy & _p)
            {
                return luanatic::pushValueType<typename RawType<T>::Type>(_luaState, _val);
            }
        };


        template <class T, class Enable = void>
        struct Pusher;

        template<>
        struct Pusher<void *>
        {
            template<class Policy>
            static stick::Int32 push(lua_State * _luaState, void * _val, const Policy & _policy = Policy())
            {
                if (!_val)
                    lua_pushnil(_luaState);
                else
                    lua_pushlightuserdata(_luaState, _val);
                return 1;
            }
        };

        //we need a special pusher for const char* as the rules for removing cv, reference and pointer
        //don't apply to it.
        template<>
        struct Pusher<const char *>
        {
            template<class Policy>
            static stick::Int32 push(lua_State * _luaState, const char * _val, const Policy & _policy = Policy())
            {
                if (!_val)
                    lua_pushnil(_luaState);
                else
                    lua_pushstring(_luaState, _val);
                return 1;
            }
        };

        template <class T>
        struct Pusher<T *>
        {
            template<class Policy>
            static stick::Int32 push(lua_State * _luaState, T * _val, const Policy & _policy = Policy())
            {
                return ApplyPolicyPtr<Policy>::apply(_luaState, _val, _policy);
            }
        };

        // // We need to do an overload using std::ref
        // template <class T>
        // struct Pusher<const std::reference_wrapper<T> & >
        // {
        //     template<class Policy>
        //     static void push(lua_State * _luaState, const std::reference_wrapper<T> & _val, const Policy & _policy = Policy())
        //     {
        //         luanatic::push<typename RawType<T>::Type>(_luaState, &_val.get(), false);
        //     }
        // };

        template <class T>
        struct Pusher<T &>
        {
            template<class Policy>
            static stick::Int32 push(lua_State * _luaState, T & _val, const Policy & _policy = Policy())
            {
                luanatic::push<typename RawType<T>::Type>(_luaState, &_val, false);
                return 1;
            }
        };

        template <class T>
        struct Pusher<const T &>
        {
            template<class Policy>
            static stick::Int32 push(lua_State * _luaState, const T & _val, const Policy & _policy = Policy())
            {
                return ApplyPolicyValueType<Policy>::apply(_luaState, _val, _policy);
            }
        };

        template <class T>
        struct Pusher<T>
        {
            template<class Policy>
            static stick::Int32 push(lua_State * _luaState, const T & _val, const Policy & _policy = Policy())
            {
                return ApplyPolicyValueType<Policy>::apply(_luaState, _val, _policy);
            }
        };

        // this is a specialization for char arrays to push them as a string ...
        template <stick::Size N>
        struct Pusher<const char (&) [N]>
        {
            template<class Policy>
            static stick::Int32 push(lua_State * _luaState, const char (&_val)[N], const Policy & _policy = Policy())
        {
            lua_pushstring(_luaState, _val);
            return 1;
        }
                    };

        template<class T>
        struct PickPusher
        {
            template<class Policy>
            static stick::Int32 push(lua_State * _state, T && _value, const Policy & _policy)
            {
                return Pusher<T>::push(_state, std::forward<T>(_value), _policy);
            }
        };

        //we forward references as const references by default...
        template<class T>
        struct PickPusher<T &>
        {
            template<class Policy>
            static stick::Int32 push(lua_State * _state, const T & _value, const Policy & _policy)
            {
                return Pusher<const T &>::push(_state, std::forward<const T &>(_value), _policy);
            }
        };

        //...unless std::ref is used
        template<class T>
        struct PickPusher<const std::reference_wrapper<T> &>
        {
            template<class Policy>
            static stick::Int32 push(lua_State * _state, const std::reference_wrapper<T> & _value, const Policy & _policy)
            {
                return Pusher<T &>::push(_state, std::forward<T &>(_value.get()), _policy);
            }
        };

        template<class T>
        struct PickPusher<T *& >
        {
            template<class Policy>
            static stick::Int32 push(lua_State * _state, T * _value, const Policy & _policy)
            {
                return Pusher<T *>::push(_state, std::forward<T *>(_value), _policy);
            }
        };
    }

    template<class F>
    struct Transfer
    {
        using Target = F;

        template<class T>
        stick::Int32 push(lua_State * _luaState, T * _val) const
        {
            luanatic::push<T>(_luaState, _val, true);
            return 1;
        }
    };

    template<class F>
    struct DefaultValue
    {

    };

    namespace detail
    {
        template<stick::Size N>
        struct IndexToPlaceHolder;

        template<>
        struct IndexToPlaceHolder<1> { using Value = ph::One; };

        template<>
        struct IndexToPlaceHolder<2> { using Value = ph::Two; };

        template<>
        struct IndexToPlaceHolder<3> { using Value = ph::Three; };

        template<>
        struct IndexToPlaceHolder<4> { using Value = ph::Four; };

        template<>
        struct IndexToPlaceHolder<5> { using Value = ph::Five; };

        template<>
        struct IndexToPlaceHolder<6> { using Value = ph::Six; };

        template<>
        struct IndexToPlaceHolder<7> { using Value = ph::Seven; };

        template<>
        struct IndexToPlaceHolder<8> { using Value = ph::Eight; };

        template<>
        struct IndexToPlaceHolder<9> { using Value = ph::Nine; };

        template<>
        struct IndexToPlaceHolder<10> { using Value = ph::Ten; };

        template <class T, class Enable = void>
        struct Converter;

        template <>
        struct Converter<void *>
        {
            using Ret = void*;

            static Ret convert(lua_State * _luaState, stick::Int32 _index)
            {
                //TODO Check if this is userdata
                return lua_touserdata(_luaState, _index);
            }
        };

        // we need a special converter to c string because
        // otherwise it tries to treat it like any other pointer.
        template <>
        struct Converter<const char *>
        {
            using Ret = const char*;

            static Ret convert(lua_State * _luaState, stick::Int32 _index)
            {
                return luaL_checkstring(_luaState, _index);
            }
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

        //helper to only forward to ValueTypeConverter for const reference types
        //if the type has a default constructor
        template<class T, class Enable = void>
        struct ConverterHelper
        {
            static const T * convert(lua_State * _luaState, stick::Int32 _index, LuanaticState * _lnstate)
            {
                //Try implicitly converting from lua, we need the return proxy to clean up
                //the tmp memory we need for this.
                auto ptr = _lnstate->m_allocator->create<T>(convertToValueTypeAndCheck<typename RawType<T>::Type>(_luaState, _index));
                return ptr;
            }
        };

        template<class T>
        struct ConverterHelper < T, typename std::enable_if < !std::is_default_constructible<T>::value >::type >
        {
            static const T * convert(lua_State * _luaState, stick::Int32 _index, LuanaticState * _lnstate)
            {
                return nullptr;
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

                ReturnProxy(const T * _ptr, stick::Allocator * _alloc) :
                    ptr(_ptr),
                    alloc(_alloc),
                    ref(*_ptr)
                {

                }

                ReturnProxy(ReturnProxy && _other) :
                    ptr(_other.ptr),
                    ref(*ptr)
                {
                    _other.ptr = nullptr;
                }

                ~ReturnProxy()
                {
                    if (ptr)
                    {
                        alloc->destroy(ptr);
                    }
                }

                ReturnProxy(const ReturnProxy & _other) = delete;
                ReturnProxy & operator = (const ReturnProxy & _other) = delete;

                operator const T & ()
                {
                    return ref;
                }

                const T * ptr;
                stick::Allocator * alloc;
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
                    return ReturnProxy(ConverterHelper<T>::convert(_luaState, _index, ls), ls->m_allocator);
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
            using type = typename next_integer_sequence <
                         typename make_int_seq_impl < T, I + 1, N >::type >::type;
        };

        template<class T, T N> struct make_int_seq_impl<T, N, N>
        {
            using type = integer_sequence<T>;
        };

        template<std::size_t... Ints>
        using index_sequence = integer_sequence<std::size_t, Ints...>;

        template<std::size_t N>
        using make_index_sequence = make_integer_sequence<std::size_t, N>;


        template<bool B, class Either, class Or>
        struct MatchResult;

        template<class Either, class Or>
        struct MatchResult<true, Either, Or>
        {
            using Result = Either;
        };

        template<class Either, class Or>
        struct MatchResult<false, Either, Or>
        {
            using Result = Or;
        };

        template<class For, class Default, class Current, class...Rest>
        struct GetPolicyImpl
        {
            using Policy = typename MatchResult<std::is_same<For, typename Current::Target>::value,
                  Current,
                  typename GetPolicyImpl<For, Default, Rest...>::Policy>::Result;
        };

        template<class For, class Default, class Current>
        struct GetPolicyImpl<For, Default, Current>
        {
            using Policy = typename MatchResult<std::is_same<For, typename Current::Target>::value,
                  Current, Default>::Result;
        };

        template<class For, class Default, class...Policies>
        struct GetPolicy
        {
            using Policy = typename GetPolicyImpl<For, Default, Policies...>::Policy;
        };

        template<class For, class Default>
        struct GetPolicy<For, Default>
        {
            using Policy = Default;
        };

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

        stick::String demangleTypeName(const char * _name)
        {
            stick::Int32 status;
            auto ret = abi::__cxa_demangle(_name, NULL, NULL, &status);
            return ret ? stick::String(ret) : "";
        }

        template<class T>
        struct RawTypeName
        {
            static stick::String name()
            {
                return demangleTypeName(typeid(T).name());
            }
        };

        template<class T>
        struct ArgName
        {
            static stick::String name(std::size_t _argCount, std::size_t _idx)
            {
                stick::String ret;
                ret.append(RawTypeName<T>::name());
                if (std::is_const<typename std::remove_reference<T>::type>::value)
                    ret.append(" const");
                if (std::is_lvalue_reference<T>::value)
                    ret.append("&");
                if (std::is_rvalue_reference<T>::value)
                    ret.append("&&");
                if (_idx < _argCount - 1)
                    ret.append(", ");
                return ret;
            }
        };


        template<class...Args>
        struct SignatureName
        {
            static stick::String name()
            {
                return nameImpl(make_index_sequence<sizeof...(Args)>());
            }

            template<std::size_t...N>
            static stick::String nameImpl(index_sequence<N...>)
            {
                return stick::String::concat("(", ArgName<Args>::name(sizeof...(Args), N)..., ")");
            }
        };

        template<class...Args>
        struct ArgScore
        {
            static stick::Int32 score(lua_State * _luaState,
                                      stick::Int32 _argCount,
                                      stick::Int32 _indexOff,
                                      stick::Int32 _defaultArgCount)
            {
                //@TODO: I think some of the arg count checks might
                //be redundant, double check!
                if (!_argCount)
                {
                    if (sizeof...(Args) - _defaultArgCount == 0)
                        return 0;
                    else
                        return std::numeric_limits<stick::Int32>::max();
                }

                if (_argCount > sizeof...(Args) || _argCount < sizeof...(Args) - _defaultArgCount)
                    return std::numeric_limits<stick::Int32>::max();
                stick::Int32 ret = 0;
                scoreImpl(_luaState, _indexOff, ret, _argCount + 1, make_index_sequence<sizeof...(Args)>());
                return ret;
            }

        private:

            template<class Arg>
            static stick::Int32 scoreHelper(lua_State * _luaState, stick::Int32 _index, stick::Int32 _maxIdx, stick::Int32 & _outResult)
            {
                if (_index <= _maxIdx)
                {
                    auto s = conversionScore<Arg>(_luaState, _index);
                    if (std::numeric_limits<stick::Int32>::max() - _outResult >= s)
                        _outResult += conversionScore<Arg>(_luaState, _index);
                    else
                        _outResult = std::numeric_limits<stick::Int32>::max();
                }
                return _outResult;
            }

            template<std::size_t...N>
            static void scoreImpl(lua_State * _luaState, stick::Int32 _indexOff, stick::Int32 & _outResult, stick::Int32 _maxIdx, index_sequence<N...>)
            {
                stick::Int32 xs[] = {scoreHelper<Args>(_luaState, 1 + N + _indexOff, _maxIdx, _outResult)...};
            }
        };

        template <class T, T Func, class...Policies>
        struct FunctionWrapper;

        template <class Ret, class... Args, Ret (*Func)(Args...), class...Policies>
        struct FunctionWrapper<Ret(*)(Args...), Func, Policies...>
        {
            static stick::Int32 score(lua_State * _luaState,
                                      stick::Int32 _argCount,
                                      stick::Int32 _defaultArgCount,
                                      stick::Int32 * _outDefaultArgsNeeded)
        {
            *_outDefaultArgsNeeded = sizeof...(Args) - _argCount;
            return ArgScore<Args...>::score(_luaState, _argCount, 0, _defaultArgCount);
        }

        static stick::String signatureStr()
        {
            return SignatureName<Args...>::name();
        }

        static stick::Size argCount()
        {
            return sizeof...(Args);
        }

        static stick::Int32 func(lua_State * _luaState)
        {
            return funcImpl(_luaState, make_index_sequence<sizeof...(Args)>());
        }

        template<std::size_t...N>
        static stick::Int32 funcImpl(lua_State * _luaState, index_sequence<N...>)
        {
            checkArgumentCountAndEmitLuaError(_luaState, sizeof...(Args), 0);
            using Policy = typename GetPolicy<ph::Result, NoPolicy, Policies...>::Policy;
            return Pusher<Ret>::push(_luaState, (*Func)(convert<Args>(_luaState, 1 + N)...), Policy());
        }
                };

        template <class... Args, void (*Func)(Args...), class...Policies>
        struct FunctionWrapper<void(*)(Args...), Func, Policies...>
        {

            static stick::Int32 score(lua_State * _luaState,
                                      stick::Int32 _argCount,
                                      stick::Int32 _defaultArgCount,
                                      stick::Int32 * _outDefaultArgsNeeded)
        {
            *_outDefaultArgsNeeded = sizeof...(Args) - _argCount;
            return ArgScore<Args...>::score(_luaState, _argCount, 0, _defaultArgCount);
        }

        static stick::String signatureStr()
        {
            return SignatureName<Args...>::name();
        }

        static stick::Size argCount()
        {
            return sizeof...(Args);
        }

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

        template <class Ret, class C, class... Args, Ret (C::*Func)(Args...), class...Policies>
        struct FunctionWrapper<Ret (C::*)(Args...), Func, Policies...>
        {
            static stick::Int32 score(lua_State * _luaState,
                                      stick::Int32 _argCount,
                                      stick::Int32 _defaultArgCount,
                                      stick::Int32 * _outDefaultArgsNeeded)
        {
            *_outDefaultArgsNeeded = sizeof...(Args) - (_argCount - 1);
            return ArgScore<Args...>::score(_luaState, _argCount - 1, 1, _defaultArgCount);
        }

        static stick::String signatureStr()
        {
            return SignatureName<Args...>::name();
        }

        static stick::Size argCount()
        {
            return sizeof...(Args);
        }

        static stick::Int32 func(lua_State * _luaState)
        {
            return funcImpl(_luaState, make_index_sequence<sizeof...(Args)>());
        }

        template<std::size_t...N>
        static stick::Int32 funcImpl(lua_State * _luaState, index_sequence<N...>)
        {
            checkArgumentCountAndEmitLuaError(_luaState, sizeof...(Args), -1); // - 1 for self
            C * obj = convertToTypeAndCheck<typename RawType<C>::Type>(_luaState, 1);
            using Policy = typename GetPolicy<ph::Result, NoPolicy, Policies...>::Policy;
            return Pusher<Ret>::push(_luaState, (obj->*Func)(convert<Args>(_luaState, 2 + N)...), Policy());
        }
                };

        template <class Ret, class C, class... Args, Ret (C::*Func)(Args...) const, class...Policies>
        struct FunctionWrapper<Ret (C::*)(Args...) const, Func, Policies...>
        {

            static stick::Int32 score(lua_State * _luaState,
                                      stick::Int32 _argCount,
                                      stick::Int32 _defaultArgCount,
                                      stick::Int32 * _outDefaultArgsNeeded)
        {
            *_outDefaultArgsNeeded = sizeof...(Args) - (_argCount - 1);
            return ArgScore<Args...>::score(_luaState, _argCount - 1, 1, _defaultArgCount);
        }

        static stick::String signatureStr()
        {
            return SignatureName<Args...>::name();
        }

        static stick::Size argCount()
        {
            return sizeof...(Args);
        }

        static stick::Int32 func(lua_State * _luaState)
        {
            return funcImpl(_luaState, make_index_sequence<sizeof...(Args)>());
        }

        template<std::size_t...N>
        static stick::Int32 funcImpl(lua_State * _luaState, index_sequence<N...>)
        {
            checkArgumentCountAndEmitLuaError(_luaState, sizeof...(Args), -1); // - 1 for self
            C * obj = convertToTypeAndCheck<typename RawType<C>::Type>(_luaState, 1);
            using Policy = typename GetPolicy<ph::Result, NoPolicy, Policies...>::Policy;
            return Pusher<Ret>::push(_luaState, (obj->*Func)(convert<Args>(_luaState, 2 + N)...), Policy());
        }
                };

        template <class C, class... Args, void (C::*Func)(Args...), class...Policies>
        struct FunctionWrapper<void (C::*)(Args...), Func, Policies...>
        {
            static stick::Int32 score(lua_State * _luaState,
                                      stick::Int32 _argCount,
                                      stick::Int32 _defaultArgCount,
                                      stick::Int32 * _outDefaultArgsNeeded)
        {
            *_outDefaultArgsNeeded = sizeof...(Args) - (_argCount - 1);
            return ArgScore<Args...>::score(_luaState, _argCount - 1, 1, _defaultArgCount);
        }

        static stick::String signatureStr()
        {
            return SignatureName<Args...>::name();
        }

        static stick::Size argCount()
        {
            return sizeof...(Args);
        }

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

        template <class C, class... Args, void (C::*Func)(Args...) const, class...Policies>
        struct FunctionWrapper<void (C::*)(Args...) const, Func, Policies...>
        {
            static stick::Int32 score(lua_State * _luaState,
                                      stick::Int32 _argCount,
                                      stick::Int32 _defaultArgCount,
                                      stick::Int32 * _outDefaultArgsNeeded)
        {
            *_outDefaultArgsNeeded = sizeof...(Args) - (_argCount - 1);
            return ArgScore<Args...>::score(_luaState, _argCount - 1, 1, _defaultArgCount);
        }

        static stick::String signatureStr()
        {
            return SignatureName<Args...>::name();
        }

        static stick::Size argCount()
        {
            return sizeof...(Args);
        }

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
        struct AttributePusher < T, typename std::enable_if < std::is_integral<T>::value || std::is_enum<T>::value || std::is_floating_point<T>::value >::type >
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
                    AttributePusher<Ret>::push(_luaState, obj->*Member);
                    return 1;
                }
                else
                {
                    obj->*Member = convertToValueTypeAndCheck<Ret>(_luaState, 2);
                    return 0;
                }
            }
        };

        template <class T, class... Args>
        struct ConstructorWrapper
        {
            static stick::Int32 score(lua_State * _luaState,
                                      stick::Int32 _argCount,
                                      stick::Int32 _defaultArgCount,
                                      stick::Int32 * _outDefaultArgsNeeded)
            {
                *_outDefaultArgsNeeded = sizeof...(Args) - (_argCount);
                return ArgScore<Args...>::score(_luaState, _argCount, 0, _defaultArgCount);
            }

            static stick::String signatureStr()
            {
                return SignatureName<Args...>::name();
            }

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
                typedef typename stick::IteratorTraits<Iterator>::ValueType ValueType;
                Pusher<ValueType &>::push(_luaState, **_it, detail::NoPolicy());
            }
        };

        template<class Iterator>
        struct IterValuePusher<Iterator, false>
        {
            static void push(lua_State * _luaState, Iterator * _it)
            {
                typedef typename stick::IteratorTraits<Iterator>::ValueType ValueType;
                Pusher<ValueType>::push(_luaState, **_it, detail::NoPolicy());
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
        inline stick::Int32 pushRange(lua_State * _luaState, Iterator _begin, Iterator _end)
        {
            pushUnregisteredType<Iterator>(_luaState, _begin);
            pushUnregisteredType<Iterator>(_luaState, _end);
            RangeFunctionPusher<Iterator, ForceReference>::push(_luaState);
            return 1;
        }

        template<class T>
        int _push(lua_State * _luaState, stick::Int32 _startIdx, stick::Int32 _idx, T _value)
        {
            if (_idx >= _startIdx)
                Pusher<T>::push(_luaState, std::forward<T>(_value), NoPolicy());
            return 0;
        }

        template<class...Args>
        struct DefaultArgs : public DefaultArgsBase
        {
            DefaultArgs()
            {
            }

            DefaultArgs(Args..._args) :
                values(_args...)
            {
            }

            void push(lua_State * _state, stick::Int32 _count) const final
            {
                pushHelper(_state, sizeof...(Args) - _count, make_index_sequence<sizeof...(Args)>());
            }

            stick::Size argCount() const final
            {
                return sizeof...(Args);
            }

            template<stick::Size...N>
            void pushHelper(lua_State * _state, stick::Int32 _startIdx, index_sequence<N...>) const
            {
                int tmp[] = {_push<Args>(_state, _startIdx, N, std::get<N>(values))...};
            }

            std::tuple<Args...> values;
        };
    }

    template<class G>
    struct ReturnIterator
    {
        using Target = G;

        template<class T>
        stick::Int32 push(lua_State * _luaState, const T & _container) const
        {
            return detail::pushRange<false>(_luaState, _container.begin(), _container.end());
        }
    };

    template<class G>
    struct ReturnRefIterator
    {
        using Target = G;

        template<class T>
        stick::Int32 push(lua_State * _luaState, const T & _container) const
        {
            return detail::pushRange<true>(_luaState, _container.begin(), _container.end());
        }
    };

    //ClassWrapper implementation

    template <class T>
    ClassWrapper<T>::ClassWrapper(const stick::String & _name)
        : ClassWrapperBase(stick::TypeInfoT<T>::typeID(), _name)
    {
    }

    // template <class T>
    // ClassWrapper<T>::~ClassWrapper()
    // {
    //     for (auto * wrapper : m_wrappers)
    //         stick::defaultAllocator().destroy(wrapper);
    // }

    template <class T>
    template <class... Args>
    ClassWrapper<T> & ClassWrapper<T>::addConstructor()
    {
        m_constructors.append({"", {&detail::ConstructorWrapper<T, Args...>::func, &detail::ConstructorWrapper<T, Args...>::score, &detail::ConstructorWrapper<T, Args...>::signatureStr}});
        return *this;
    }

    template <class T>
    template <class... Args>
    ClassWrapper<T> & ClassWrapper<T>::addConstructor(const stick::String & _str)
    {
        m_statics.append({ _str, {&detail::ConstructorWrapper<T, Args...>::func, &detail::ConstructorWrapper<T, Args...>::score, &detail::ConstructorWrapper<T, Args...>::signatureStr}});
        return *this;
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

    // template <class T>
    // template <class B>
    // ClassWrapper<T> & ClassWrapper<T>::addWrapper()
    // {
    //     auto * obj = stick::defaultAllocator().create<ClassWrapper<B>>(stick::String::concat(m_className, typeid(B).name(), "Wrapper"));
    //     obj->template addBase<T>();
    //     m_wrappers.append(obj);
    //     return *this;
    // }


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

        //@TODO: Rename and make private
        template<class T, class Policy>
        void setImpl(T && _value, const Policy & _policy)
        {
            detail::PickPusher<T>::push(m_state, std::forward<T>(_value), _policy);
        }

        //@TODO: Rename and make private
        //overload to handle set for another LuaValue
        template<class Policy>
        void setImpl(LuaValue & _value, const Policy & _policy)
        {
            _value.push();
        }

        template <class T, class Policy = detail::NoPolicy >
        void set(T && _value, const Policy & _policy = Policy())
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
                // detail::PickPusher<T>::push(m_state, std::forward<T>(_value), _policy);
                setImpl(std::forward<T>(_value), _policy);
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
                //_policy.template push<T>(m_state, std::forward<T>(_value));
                // detail::PickPusher<T>::push(m_state, std::forward<T>(_value), _policy);
                setImpl(std::forward<T>(_value), _policy);
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

        //@TODO: Rename to child
        template <class K>
        LuaValue getChild(K && _key)
        {
            STICK_ASSERT(m_type == LuaType::Table);
            detail::PickPusher<K>::push(m_state, std::forward<K>(_key), detail::NoPolicy());
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

        template <class A, class B>
        LuaValue & addWrapper()
        {
            /*detail::LuanaticState * glua = detail::luanaticState(_luaState);
            STICK_ASSERT(glua != nullptr);
            auto it = glua->m_typeIDClassMap.find(stick::TypeInfoT<A>::typeID());
            if(it == glua->m_typeIDClassMap.end())
            {

            }*/
            ClassWrapper<B> wrapper(stick::String::concat(typeid(A).name(), typeid(B).name(), "Wrapper"));
            wrapper.template addBase<A>();
            return registerClass(wrapper);
        }

        // @TODO: Not sure if we should keep this overload as
        // it will break overloading if multiple functions
        // are registered with the same name. It is useful though to
        // register hand rolled lua_CFunction functions...hmmm
        //
        // Maybe emit a warning/error if there is a function
        // allready registered with that name that has no score function?
        //
        // The reason it breaks overloading is because we don't provide
        // a score function that is used for signature matching.
        LuaValue & registerFunction(const stick::String & _name,
                                    lua_CFunction _function)
        {
            return registerFunction(_name, {_function, NULL});
        }

        LuaValue & registerFunction(const stick::String & _name,
                                    LuanaticFunction _function)
        {
            STICK_ASSERT(m_state && m_type == LuaType::Table);
            _function.defaultArgs = nullptr;
            push();
            ClassWrapperBase::NamedLuaFunctionArray tmp;
            tmp.append({ _name, _function });
            detail::registerFunctions(m_state, lua_gettop(m_state), tmp);
            lua_pop(m_state, 1);
            return *this;
        }

        template<class...Args>
        LuaValue & registerFunction(const stick::String & _name,
                                    LuanaticFunction _function, Args..._args)
        {
            STICK_ASSERT(m_state && m_type == LuaType::Table);
            push();
            _function.defaultArgs = stick::defaultAllocator().create<detail::DefaultArgs<Args...>>(_args...);
            ClassWrapperBase::NamedLuaFunctionArray tmp;
            tmp.append({ _name, _function});
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
                    detail::Pass{ (detail::Pusher<Args>::push(_state, std::forward<Args>(_args), detail::NoPolicy()),
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
                    detail::Pass{ (detail::Pusher<Args>::push(_state, std::forward<Args>(_args), detail::NoPolicy()),
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
        LuaValue findOrCreateTable(Key && _key)
        {
            STICK_ASSERT(m_type == LuaType::Table && m_state);

            LuaValue ret = getChild(_key);

            if (ret.type() != LuaType::Table)
            {
                ret.reset(); //need to reset to pop the key off the stack.
                detail::PickPusher<Key>::push(m_state, std::forward<Key>(_key), detail::NoPolicy()); //key
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
            lua_getfield(_luaState, -1, "__attributes"); // obj key val mt __attributes
            lua_pushvalue(_luaState, -4); // obj key val mt __attributes key
            lua_gettable(_luaState,
                         -2); // obj key val mt __attributes funcOrNil

            // if we found it we call it
            if (!lua_isnil(_luaState, -1))
            {
                // push the function arguments
                lua_pushvalue(_luaState,  1); // obj key val mt __attributes funcOrNil obj
                lua_pushvalue( _luaState, 3); // obj key val mt __attributes funcOrNil obj val
                lua_call(_luaState, 2, 0);
                return 0;
            }

            lua_settop(_luaState, 3); // obj key val
            lua_getfield(_luaState, LUA_REGISTRYINDEX, LUANATIC_KEY);             // obj key val gT
            lua_getfield(_luaState, -1, "storage"); // obj key val gT storage

            detail::ObjectIdentifier<T>::identify(_luaState, obj); // obj key gT storage id
            lua_pushvalue(_luaState, -1);               // obj key val gT storage id id
            lua_gettable(_luaState, -3); // obj key val gT storage id idtableOrNil

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

            lua_getfield(_luaState, LUA_REGISTRYINDEX, LUANATIC_KEY); // obj key gT
            lua_getfield(_luaState, -1, "storage"); // obj key gT storage

            ObjectIdentifier<T>::identify(_luaState,  obj); // obj key gT storage id
            lua_gettable(_luaState, -2);        // obj key gT storage objectStorage

            // check if storage table exists for this object
            if (!lua_isnil(_luaState, -1))
            {
                lua_pushvalue(_luaState, -4); // obj key gT storage objectStorage key
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
                    lua_getfield(_luaState, -1, "__attributes"); // obj key mt __attributes
                    lua_pushvalue(_luaState, -3); // obj key mt __attributes key
                    lua_gettable(_luaState, -2); // obj key mt __attributes funcOrNil

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
                    state->m_allocator->destroy(obj);
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
            lua_newtable(_state);                       //classTable ... objInstance
            lua_pushvalue(_state, 1);                   //classTable ... objInstance classTable
            lua_setmetatable(_state, -2);               //classTable ... objInstance
            lua_getfield(_state, 1, "__init");          //classTable ... objInstance initOrNil
            if (!lua_isnil(_state, -1))
            {
                //call the __init function
                //@TODO: this looks like there is a lot of room for optimization :)
                lua_replace(_state, 1);                     //init ... objInstance
                lua_insert(_state, 2);                      //init objInstance ...
                lua_pushvalue(_state, 2); //init objInstance ... objInstance
                lua_insert(_state, 1); //objInstance init objInstance ...
                lua_call(_state, argCount, 0); // objInstance
            }
            else
            {
                lua_pop(_state, 2); //... objInstance
            }
            STICK_ASSERT(lua_istable(_state, -1));
            return 1;
        }

        //creates a new luanatic style class from lua
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

        // static stick::Int32 gluaEnsureClass(lua_State * _state)
        // {
        //     STICK_ASSERT(lua_isuserdata(_state, 1));
        //     STICK_ASSERT(lua_istable(_state, 2));

        //     //This is sort of the fastest way of doing it, but not safe. Should we put more sanity
        //     //checks here? hmhmhmhm
        //     detail::UserData * userData = static_cast<detail::UserData*>(lua_touserdata(_state, 1));

        //     lua_getfield(_state, 2, "__typeID");

        //     if((stick::Size)userData->m_typeID != lua_tointeger(_state, -1))
        //     {
        //         userData->m_typeID = luaL_checkinteger(_state, -1);
        //         lua_pop(_state, 1);
        //         lua_setmetatable(_state, 1);
        //     }

        //     return 0;
        // }

        // static int luanaticCast(lua_State * _state)
        // {
        //     if(lua_istable(_state, 1) && lua_isuserdata(_state, 2))
        //     {
        //         lua_getfield(_state, 1, "__typeID") // 1 2 tid
        //         if(!lua_isnil(_state, -1))
        //         {
        //             if(lua_getmetatable(_state, 2)) // 1 2 tid mt
        //             {
        //                 lua_getfield(_state, -1, "__typeID") // 1 2 tid mt tid2
        //                 if(!lua_isnil(_state, -1))
        //                 {
        //                     if(lua_touserdata(_state, -1) == lua_touserdata)
        //                 }
        //             }
        //         }
        //     }
        // }
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
        if (_luaCode.length())
        {
            stick::Int32 result = luaL_dostring(_state, _luaCode.cString());
            if (result)
                return stick::Error(stick::ec::InvalidOperation, lua_tostring(_state, -1), STICK_FILE, STICK_LINE);
        }
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

    template <class T>
    struct ValueTypeConverter<stick::Maybe<T> >
    {
        static T convertAndCheck(lua_State * _state, stick::Int32 _index)
        {
            return detail::Converter<T>::convert(_state, _index);
        }

        static stick::Int32 push(lua_State * _state, stick::Maybe<T> _value)
        {
            if (!_value)
                lua_pushnil(_state);
            else
                return detail::Pusher<T>::push(_state, *_value, detail::NoPolicy());
        }
    };

    template <class T>
    struct ValueTypeConverter<stick::Result<T> >
    {
        static T convertAndCheck(lua_State * _state, stick::Int32 _index)
        {
            return detail::Converter<T>::convert(_state, _index);
        }

        static stick::Int32 push(lua_State * _state, const stick::Result<T> & _value)
        {
            if (!_value)
            {
                lua_pushnil(_state);
                lua_pushstring(_state, _value.error().message().cString());
                return 2;
            }
            else
            {
                return detail::Pusher<T>::push(_state, _value.get(), detail::NoPolicy());
            }
        }
    };

    template<>
    struct ValueTypeConverter<stick::URI>
    {
        static stick::URI convertAndCheck(lua_State * _state, stick::Int32 _index)
        {
            return stick::URI(luaL_checkstring(_state, _index));
        }

        static stick::Int32 push(lua_State * _state, const stick::URI & _value)
        {
            return detail::DefaultValueTypeConverterImpl<stick::URI>::push(_state, _value);
        }
    };
}

#endif // LUANATIC_LUANATIC_HPP
