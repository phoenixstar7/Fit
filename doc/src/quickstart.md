Quick Start
===========

Function Objects
----------------

In C++, a function object is just a class that overrides the call operator like this:

    // A sum function object
    struct sum_f
    {
        template<class T, class U>
        auto operator()(T x, U y) const
        {
            return x + y;
        }
    };

There are few things to note about this. First, the call operator member function is always declared `const`, which is generally required to be used with Fit.(Note: The `mutable_` adaptor can be used to make a mutable function object have a `const` call operator, but this should generally be avoided). Secondly, the `sum_f` class must be constructed first before it can be called:

    auto three = sum_f()(1, 2);

We can make it behave like a regular function if we construct the class as a global variable. However, we want to statically initialize it(ie initialize it at compile-time) to avoid the static initializaion order fiasco. We can do that using `constexpr` like this:

    const constexpr sum_f sum = {};

Adaptors
--------

Now we have defined the function as a function object we can add new "enhancements" to the function. We could make the function pipable using the pipable adaptor:

    const constexpr pipable_adaptor<sum_f> sum = {};

So it could be called like this:

    auto three = 1 | sum(2);

Or we could make it an infix named operator as well using the infix adaptor:

    const constexpr infix_adaptor<sum_f> sum = {};

And it could be called like this:

    auto three = 1 <sum> 2;

Additionally each of the adaptors have a corresponding function version without the `_adaptor` suffix. So we could pass `sum` as a variable to the adaptors to make new functions. So we can do things like partial application and function composition if we wanted to:

    auto add_1 = partial(sum)(1);
    auto add_2 = compose(add_1, add_1);
    auto three = add_2(1);

Lambdas
-------

Instead of writing function objects which can be a little verbose, we can write the functions as lambdas instead. However, by default lambdas cannot be statically initialized at compile time. So we can use the `FIT_STATIC_LAMBDA` to initialize them at compile time:

    const constexpr auto sum = FIT_STATIC_LAMBDA(auto x, auto y)
    {
        return x + y;
    };

And we can apply the same adaptors as well:

    // Pipable sum
    const constexpr auto sum = pipable(FIT_STATIC_LAMBDA(auto x, auto y)
    {
        return x + y;
    });

Overloading
-----------

Now, Fit provides two ways of doing overloading. The `match` adaptor will call a function based on C++ overload resolution, which tries to find the best match, like this:

    const constexpr auto print = match(
        FIT_STATIC_LAMBDA(int x)
        {
            std::cout << "Integer: " << x << std::endl;
        },
        FIT_STATIC_LAMBDA(const std::string& x)
        {
            std::cout << "String: " << x << std::endl;
        }
    );

However, when trying to do overloading involving something more generic, it can lead to ambiguities. So the `conditional` adaptor will pick the first function that is callable. This allows ordering the functions based on which one is more important. So if we wanted to write a function to print all the values in a type including primitive types as well as ranges, we could write something like this:

    #define REQUIRES(...) typename std::enable_if<(__VA_ARGS__), int>::type = 0

    const constexpr auto print = conditional(
        FIT_STATIC_LAMBDA(auto x, REQUIRES(std::is_fundamental<decltype(x)>()))
        {
            std::cout << x << std::endl;
        },
        FIT_STATIC_LAMBDA(const std::string& x)
        {
            std::cout << x << std::endl;
        },
        FIT_STATIC_LAMBDA(const auto& range)
        {
            for(const auto& x:range) 
                std::cout << x << std::endl;
        }
    );

We constraint the first lambda to just fundamental types using `REQUIRES`. The last lambda we assume it is a range(we should check if it is a range but that is beyond the scope of this guide), and it is only called if the first two lambdas cannot be called(ie the type is not a fundamental nor a string).

Recursive
---------

Additionally, we could go a step further and make the `print` function recursive. We can use the `fix` adaptor, which implements a fix point combinator. So the first parameter of the function will be the function itself:

    const constexpr auto print = fix(conditional(
        FIT_STATIC_LAMBDA(auto, auto x, REQUIRES(std::is_fundamental<decltype(x)>()))
        {
            std::cout << x << std::endl;
        },
        FIT_STATIC_LAMBDA(auto, const std::string& x)
        {
            std::cout << x << std::endl;
        },
        FIT_STATIC_LAMBDA(auto self, const auto& range)
        {
            for(const auto& x:range) self(x);
        }
    ));
