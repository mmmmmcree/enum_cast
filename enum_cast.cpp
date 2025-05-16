/*
 * enum_cast.h - Utility for safely casting between different enum types
 *
 * This header provides a type-safe mechanism for converting between enum values
 * from different libraries that represent the same logical concepts.
 * 
 * Key components:
 * - EnumConcept: Ensures template parameters are enum types
 * - MappingTraitsConcept: Validates mapping trait structures
 * - enum_category: Associates enums with their conceptual category
 * - enum_mapping_traits: Defines mappings between enum values
 * - enum_cast: Performs the actual enum conversion
 */

#include <ranges>
#include <type_traits>


template <typename T>
concept EnumConcept = std::is_enum_v<T>;

template <typename T>
concept EnumMappingTraitsConcept = requires {
    typename T::mapping_type;
    { T::mappings } -> std::ranges::range;
    { *std::begin(T::mappings) } -> std::convertible_to<typename T::mapping_type>;
};

// Trait detection - determines which category an enum belongs to
template <EnumConcept Enum>
struct enum_category;

template <EnumConcept Enum>
using enum_category_t = typename enum_category<Enum>::type;

// Enum mapping traits declaration - implemented per enum category
template <typename Tag>
struct enum_mapping_traits;

/**
 * Converts an enum value from one type to another within the same category
 * 
 * @tparam Dst The destination enum type
 * @tparam Src The source enum type
 * @param src The source enum value to convert
 * @return The equivalent enum value in the destination type
 * 
 * @note Source and destination enums must belong to the same category
 * @note Returns the first enum value (0) if no mapping is found
 */
template <EnumConcept Dst, EnumConcept Src>
constexpr Dst enum_cast(Src src)
{
    static_assert(std::is_same_v<enum_category_t<Src>, enum_category_t<Dst>>,
                 "Source and destination enums must be of the same category");
    using Category = enum_category_t<Src>;
    using MappingTraits = enum_mapping_traits<Category>;
    for (const auto& mapping : MappingTraits::mappings) {
        if (std::get<Src>(mapping) == src) {
            return std::get<Dst>(mapping);
        }
    }
    return static_cast<Dst>(0);
}

/*
 * Example usage - This section would be in your main application code
 * The following examples show how to use the enum_cast utility with
 * different enum types from multiple libraries.
 */

/*
 * Recommended Directory Structure:
 *   - utilities/
 *   -- enum_cast.h          // enum_cast function and related support classes
 *   - Color/
 *   -- ColorDefs.h          // Color enum mappings
 *   - Shape/
 *   -- ShapeDefs.h          // Shape enum mappings
 *   - concepts/
 *   -- enum_concept.h       // EnumConcept definition
 *   -- enum_mapping_traits_concept.h     // MappingTraitsConcept definition
 */

namespace lib_a {
    enum class Color { Red = 3, Green = 4, Blue = 5 };
    enum class Shape { Circle = 1, Square = 9, Triangle = 16 };
}

namespace lib_b {
    enum class Color { Red, Green, Blue, Yellow };
    enum class Shape { Circle, Square, Triangle };
}

namespace lib_c {
    enum class Color { Red = 9, Green = 100, Blue = 101 };
    enum class Shape { Circle = 2, Square = 3, Triangle = 4 };
}

#include <tuple>

/*
 * ColorDefs.h - Contains enum category and mapping definitions for colors
 * 
 * This file defines the enum category tag and mapping tables for color enums
 * from different libraries, enabling conversion between them.
 */
struct EnumColorTag {};
template <> struct enum_category<lib_a::Color> { using type = EnumColorTag; };
template <> struct enum_category<lib_b::Color> { using type = EnumColorTag; };
template <> struct enum_category<lib_c::Color> { using type = EnumColorTag; };

template <>
struct enum_mapping_traits<EnumColorTag>
{
    using mapping_type = std::tuple<lib_a::Color, lib_b::Color, lib_c::Color>;
    constexpr static mapping_type mappings[] = {
        { lib_a::Color::Red, lib_b::Color::Red, lib_c::Color::Red },
        { lib_a::Color::Green, lib_b::Color::Green, lib_c::Color::Green },
        { lib_a::Color::Blue, lib_b::Color::Blue, lib_c::Color::Blue }
    };
};

/*
 * ShapeDefs.h - Contains enum category and mapping definitions for shapes
 * 
 * This file defines the enum category tag and mapping tables for shape enums
 * from different libraries, enabling conversion between them.
 */
struct EnumShapeTag {};
template <> struct enum_category<lib_a::Shape> { using type = EnumShapeTag; };
template <> struct enum_category<lib_b::Shape> { using type = EnumShapeTag; };
template <> struct enum_category<lib_c::Shape> { using type = EnumShapeTag; };

template <>
struct enum_mapping_traits<EnumShapeTag>
{
    using mapping_type = std::tuple<lib_a::Shape, lib_b::Shape, lib_c::Shape>;
    constexpr static mapping_type mappings[] = {
        { lib_a::Shape::Circle, lib_b::Shape::Circle, lib_c::Shape::Circle },
        { lib_a::Shape::Square, lib_b::Shape::Square, lib_c::Shape::Square },
        { lib_a::Shape::Triangle, lib_b::Shape::Triangle, lib_c::Shape::Triangle }
    };
};

#include <iostream>

int main()
{
    lib_a::Color a_color = enum_cast<lib_a::Color>(lib_c::Color::Red);
    std::cout << static_cast<int>(a_color) << std::endl;

    lib_b::Color b_color = enum_cast<lib_b::Color>(a_color);
    std::cout << static_cast<int>(b_color) << std::endl;

    lib_a::Shape a_shape = enum_cast<lib_a::Shape>(lib_b::Shape::Circle);
    std::cout << static_cast<int>(a_shape) << std::endl;

    return 0;
}