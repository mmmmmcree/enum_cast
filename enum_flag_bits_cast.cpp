/*
 * enum_flag_bits_cast.h - Utility for safely casting between different enum types
 *
 * This header provides a type-safe mechanism for converting between enum values
 * from different libraries that represent the same logical concepts.
 * 
 * Key components:
 * - EnumConcept: Ensures template parameters are enum types
 * - MappingTraitsConcept: Validates mapping trait structures
 * - enum_category: Associates enums with their conceptual category
 * - enum_mapping_traits: Defines mappings between enum values
 * - enum_flag_bits_cast: Performs the actual enum flag bits conversion
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

template <EnumConcept Enum>
struct enum_category;

template <EnumConcept Enum>
using enum_category_t = typename enum_category<Enum>::type;

template <typename Tag>
struct enum_mapping_traits;


/**
 * @brief Converts bitwise flag enum values from one type to another within the same category
 * @tparam Dst The destination enum type
 * @tparam Src The source enum type
 * @param src The source enum flags value to convert
 * @return The equivalent enum flags value in the destination type
 * @note Source and destination enums must belong to the same category
 * @note Each bit position in the source is mapped to its corresponding bit in the destination
 * @note If a bit has no mapping, it will be dropped in the conversion
 */
template <EnumConcept Dst, EnumConcept Src>
constexpr Dst enum_flag_bits_cast(Src src)
{
    static_assert(std::is_same_v<enum_category_t<Src>, enum_category_t<Dst>>,
                 "Source and destination enums must be of the same category");
    using Category = enum_category_t<Src>;
    using MappingTraits = enum_mapping_traits<Category>;
    using UnderlyingType = std::underlying_type_t<Dst>;
    UnderlyingType dst;
    for (const auto& mapping : MappingTraits::mappings) {
        if (std::get<Src>(mapping) & src) {
            dst |= static_cast<UnderlyingType>(std::get<Dst>(mapping));
        }
    }
    return static_cast<Dst>(dst);
}

namespace lib_a {
    enum Permission
    {
        None = 0x00,
        Read = 0x01,
        Write = 0x02,
        Execute = 0x04,
        ReadWrite = Read | Write,
        ReadExecute = Read | Execute,
        WriteExecute = Write | Execute,
        All = Read | Write | Execute
    };
}

namespace lib_b {
    enum Permission
    {
        NONE = 0,
        READ = 1 << 2,
        WRITE = 1 << 3,
        EXECUTE = 1 << 4,
    };
}

/*
 * It is important to ensure that the result of bitwise operations on enums is of the enum type, not an int type.
 * In other words, if the corresponding library does not implement this, it needs to be manually implemented.
*/

template <EnumConcept Enum>
Enum operator|(Enum a, Enum b)
{
    using UnderlyingType = std::underlying_type_t<Enum>;
    return static_cast<Enum>(static_cast<UnderlyingType>(a) | static_cast<UnderlyingType>(b));
}

#include <tuple>

struct PermissionTag {};
template <> struct enum_category<lib_a::Permission> { using type = PermissionTag; };
template <> struct enum_category<lib_b::Permission> { using type = PermissionTag; };

template <>
struct enum_mapping_traits<PermissionTag>
{
    using mapping_type = std::tuple<lib_a::Permission, lib_b::Permission>;
    constexpr static mapping_type mappings[] = {
        {lib_a::Permission::None, lib_b::Permission::NONE},
        {lib_a::Permission::Read, lib_b::Permission::READ},
        {lib_a::Permission::Write, lib_b::Permission::WRITE},
        {lib_a::Permission::Execute, lib_b::Permission::EXECUTE},
    };
};


#include <iostream>
#include <bitset>

template <typename E>
void print_flag_enum(E value)
{
    std::cout << "Value: " << static_cast<int>(value)
    << " (0b" << std::bitset<8>(static_cast<int>(value)) << ")" << std::endl;
}

int main()
{
    lib_a::Permission a = enum_flag_bits_cast<lib_a::Permission>(lib_b::READ | lib_b::WRITE);
    print_flag_enum(a);
    lib_b::Permission b = enum_flag_bits_cast<lib_b::Permission>(lib_a::Permission::Read | lib_a::Permission::Write);
    print_flag_enum(b);
    return 0;
}