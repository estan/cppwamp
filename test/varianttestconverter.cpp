/*------------------------------------------------------------------------------
                Copyright Butterfly Energy Systems 2014-2015.
           Distributed under the Boost Software License, Version 1.0.
              (See accompanying file LICENSE_1_0.txt or copy at
                    http://www.boost.org/LICENSE_1_0.txt)
------------------------------------------------------------------------------*/

#if CPPWAMP_TESTING_VARIANT

#include <vector>
#include <catch.hpp>
#include <cppwamp/conversion.hpp>

using namespace wamp;

namespace user
{

//------------------------------------------------------------------------------
struct SimpleDto
{
    bool b;
    int n;
    float x;
    std::string s;

    bool operator==(const SimpleDto& other) const
    {
        return  b == other.b &&
                n == other.n &&
                x == other.x &&
                s == other.s;
    }
};

template <typename TConverter>
void convert(TConverter& conv, SimpleDto& dto)
{
    conv ("b", dto.b) ("n", dto.n) ("x", dto.x) ("s", dto.s);
}

//------------------------------------------------------------------------------
struct IntrusiveSimpleDto
{
    bool b;
    int n;
    float x;
    std::string s;

    bool operator==(const IntrusiveSimpleDto& other) const
    {
        return  b == other.b &&
                n == other.n &&
                x == other.x &&
                s == other.s;
    }

private:
    template <typename TConverter>
    void convert(TConverter& conv)
    {
        conv ("b", b) ("n", n) ("x", x) ("s", s);
    }

    friend class wamp::ConversionAccess;
};

//------------------------------------------------------------------------------
struct CompositeDto
{
    SimpleDto sub1;
    IntrusiveSimpleDto sub2;

    bool operator==(const CompositeDto& other) const
    {
        return  sub1 == other.sub1 &&
                sub2 == other.sub2;
    }
};

template <typename TConverter>
void convert(TConverter& conv, CompositeDto& dto)
{
    conv ("sub1", dto.sub1) ("sub2", dto.sub2);
}

//------------------------------------------------------------------------------
struct IntrusiveCompositeDto
{
    SimpleDto sub1;
    IntrusiveSimpleDto sub2;

    bool operator==(const IntrusiveCompositeDto& other) const
    {
        return  sub1 == other.sub1 &&
                sub2 == other.sub2;
    }

private:
    template <typename TConverter>
    void convert(TConverter& conv)
    {
        conv ("sub1", sub1) ("sub2", sub2);
    }

    friend class wamp::ConversionAccess;
};

//------------------------------------------------------------------------------
struct SplitDto
{
    bool b;
    int n;
    float x;
    std::string s;

    bool operator==(const SplitDto& other) const
    {
        return  b == other.b &&
                n == other.n &&
                x == other.x &&
                s == other.s;
    }
};

void convertFrom(FromVariantConverter& conv, SplitDto& dto)
{
    conv ("b1", dto.b) ("n1", dto.n) ("x1", dto.x) ("s1", dto.s);
}

void convertTo(ToVariantConverter& conv, const SplitDto& dto)
{
    conv ("b2", dto.b) ("n2", dto.n) ("x2", dto.x) ("s2", dto.s);
}

CPPWAMP_CONVERSION_SPLIT_FREE(SplitDto)

//------------------------------------------------------------------------------
struct IntrusiveSplitDto
{
    bool b;
    int n;
    float x;
    std::string s;

    bool operator==(const IntrusiveSplitDto& other) const
    {
        return  b == other.b &&
                n == other.n &&
                x == other.x &&
                s == other.s;
    }

private:
    void convertFrom(FromVariantConverter& conv)
    {
        conv ("b1", b) ("n1", n) ("x1", x) ("s1", s);
    }

    void convertTo(ToVariantConverter& conv) const
    {
        conv ("b2", b) ("n2", n) ("x2", x) ("s2", s);
    }

    friend class wamp::ConversionAccess;
};

CPPWAMP_CONVERSION_SPLIT_MEMBER(IntrusiveSplitDto)

//------------------------------------------------------------------------------
class CustomContainer
{
public:
    std::vector<int> data;

private:
    void convertFrom(FromVariantConverter& conv)
    {
        data.clear();
        size_t size = conv.size();
        data.reserve(size);
        int elem = 0;
        for (; size > 0; --size)
        {
            conv[elem];
            data.push_back(elem);
        }
    }

    void convertTo(ToVariantConverter& conv) const
    {
        conv.size(data.size());
        for (auto elem: data)
            conv[elem];
    }

    friend class wamp::ConversionAccess;
};

CPPWAMP_CONVERSION_SPLIT_MEMBER(CustomContainer)

//------------------------------------------------------------------------------
class DerivedDto : public SimpleDto
{
public:
    std::string extra;

    bool operator==(const DerivedDto& other) const
    {
        return (static_cast<const Base&>(*this) ==
                static_cast<const Base&>(other)) &&
               (extra == other.extra);
    }

private:
    using Base = SimpleDto;

    template <typename TConverter>
    void convert(TConverter& conv)
    {
        conv(static_cast<Base&>(*this))
            ("extra", extra);
    }

    friend class wamp::ConversionAccess;
};

} // namespace user


//------------------------------------------------------------------------------
SCENARIO( "Using converters directly", "[Variant]" )
{
    Variant v;
    ToVariantConverter toConverter(v);
    FromVariantConverter fromConverter(v);
    Object object{{"b",true}, {"n",2}, {"x",3.0f}, {"s","4"}};

    GIVEN( "a simple DTO" )
    {
        user::SimpleDto dto{true, 2, 3.0f, "4"};

        WHEN( "Saving the DTO" )
        {
            toConverter(dto);
            CHECK( v == object );
        }

        WHEN( "Loading the DTO" )
        {
            v = object;
            user::SimpleDto loaded;
            fromConverter(loaded);
            CHECK( loaded == dto );
        }
    }

    GIVEN( "a simple DTO with intrusive converter" )
    {
        user::IntrusiveSimpleDto dto{true, 2, 3.0f, "4"};

        WHEN( "Saving the DTO" )
        {
            convert(toConverter, dto);
            CHECK( v == object );
        }

        WHEN( "Loading the DTO" )
        {
            v = object;
            user::IntrusiveSimpleDto loaded;
            convert(fromConverter, loaded);
            CHECK( loaded == dto );
        }
    }
}

//------------------------------------------------------------------------------
SCENARIO( "Converting to/from variants", "[Variant]" )
{
    Object object{{"b",true}, {"n",2}, {"x",3.0f}, {"s","4"}};

    GIVEN( "a simple DTO" )
    {
        user::SimpleDto dto{true, 2, 3.0f, "4"};

        WHEN( "Saving the DTO" )
        {
            auto v = Variant::from(dto);
            CHECK( v == object );
        }

        WHEN( "Loading the DTO" )
        {
            Variant v = object;
            auto loaded = v.to<user::SimpleDto>();
            CHECK( loaded == dto );
        }
    }

    GIVEN( "a simple DTO with intrusive converter" )
    {
        user::IntrusiveSimpleDto dto{true, 2, 3.0f, "4"};

        WHEN( "Saving the DTO" )
        {
            auto v = Variant::from(dto);
            CHECK( v == object );
        }

        WHEN( "Loading the DTO" )
        {
            Variant v = object;
            auto loaded = v.to<user::IntrusiveSimpleDto>();
            CHECK( loaded == dto );
        }
    }
}

//------------------------------------------------------------------------------
SCENARIO( "Composite DTOs", "[Variant]" )
{
    Object object1{{"b",true}, {"n",2}, {"x",3.0f}, {"s","4"}};
    Object object2{{"b",false}, {"n",-2}, {"x",-3.0f}, {"s","-4"}};
    Object compositeObject{{"sub1", object1}, {"sub2", object2}};

    GIVEN( "a composite DTO" )
    {
        user::CompositeDto dto{ {true, 2, 3.0f, "4"},
                                {false, -2, -3.0f, "-4"} };

        WHEN( "Saving the DTO" )
        {
            auto v = Variant::from(dto);
            CHECK( v == compositeObject );
        }

        WHEN( "Loading the DTO" )
        {
            Variant v = compositeObject;
            auto loaded = v.to<user::CompositeDto>();
            CHECK( loaded == dto );
        }
    }

    GIVEN( "a simple DTO with intrusive converter" )
    {
        user::IntrusiveCompositeDto dto{ {true, 2, 3.0f, "4"},
                                         {false, -2, -3.0f, "-4"} };

        WHEN( "Saving the DTO" )
        {
            auto v = Variant::from(dto);
            CHECK( v == compositeObject );
        }

        WHEN( "Loading the DTO" )
        {
            Variant v = compositeObject;
            auto loaded = v.to<user::IntrusiveCompositeDto>();
            CHECK( loaded == dto );
        }
    }
}

//------------------------------------------------------------------------------
SCENARIO( "Using split conversions", "[Variant]" )
{
    Object object1{{"b1",true}, {"n1",2}, {"x1",3.0f}, {"s1","4"}};
    Object object2{{"b2",true}, {"n2",2}, {"x2",3.0f}, {"s2","4"}};

    GIVEN( "a simple DTO" )
    {
        user::SplitDto dto{true, 2, 3.0f, "4"};

        WHEN( "Saving the DTO" )
        {
            auto v = Variant::from(dto);
            CHECK( v == object2 );
        }

        WHEN( "Loading the DTO" )
        {
            Variant v = object1;
            auto loaded = v.to<user::SplitDto>();
            CHECK( loaded == dto );
        }
    }

    GIVEN( "a simple DTO with intrusive converter" )
    {
        user::IntrusiveSplitDto dto{true, 2, 3.0f, "4"};

        WHEN( "Saving the DTO" )
        {
            auto v = Variant::from(dto);
            CHECK( v == object2 );
        }

        WHEN( "Loading the DTO" )
        {
            Variant v = object1;
            auto loaded = v.to<user::IntrusiveSplitDto>();
            CHECK( loaded == dto );
        }
    }
}

//------------------------------------------------------------------------------
SCENARIO( "Converting custom sequence collections", "[Variant]" )
{
    GIVEN( "a custom sequence collection" )
    {
        user::CustomContainer seq;
        seq.data = {1, 2, 3};

        WHEN( "converted to a variant" )
        {
            auto v = Variant::from(seq);

            THEN( "the variant is as expected" )
            {
                CHECK( v == (Array{1, 2, 3}) );
            }
        }
    }

    GIVEN( "an array variant" )
    {
        Variant v = Array{1.0, 2.0, 3.0};

        WHEN( "converted to a custom sequence collection" )
        {
            auto seq = v.to<user::CustomContainer>();

            THEN( "the collection is as expected" )
            {
                CHECK( seq.data == (std::vector<int>{1, 2, 3}) );
            }
        }
    }
}

//------------------------------------------------------------------------------
SCENARIO( "Derived DTOs", "[Variant]" )
{
    Variant derivedObject = Object{{"b",true}, {"n",2}, {"x",3.0f}, {"s","4"},
                                   {"extra", "5"}};

    GIVEN( "a derived DTO" )
    {
        user::DerivedDto dto;
        dto.b = true;
        dto.n = 2;
        dto.x = 3.0f;
        dto.s = "4";
        dto.extra = "5";

        WHEN( "Saving the DTO" )
        {
            auto v = Variant::from(dto);
            CHECK( v == derivedObject );
        }

        WHEN( "Loading the DTO" )
        {
            Variant v = derivedObject;
            auto loaded = v.to<user::DerivedDto>();
            CHECK( loaded == dto );
        }
    }
}

#endif // #if CPPWAMP_TESTING_VARIANT
