# enum_cast: Type-Safe Enum Conversion Utility

A modern C++20 header-only library that provides type-safe conversions between enum types from different libraries or modules that represent the same logical concepts.

## Requirements

- C++20 compatible compiler
- Standard library with concepts and ranges support

## Overview

When integrating multiple libraries that define their own enum types for the same concepts (like colors, shapes, etc.), you often need to convert between these types. This library provides a clean, type-safe way to perform these conversions without resorting to unsafe casts.

```C++
// Converting between different library enum types
lib_a::Color color_a = enum_cast<lib_a::Color>(lib_b::Color::Red);
lib_c::Shape shape_c = enum_cast<lib_c::Shape>(lib_a::Shape::Circle);
lib_a::Permission a = enum_flag_bits_cast<lib_a::Permission>(lib_b::READ | lib_b::WRITE);
```

## Usage

### enum_cast

1. Define your enum categories and mappings:

   ```C++
   // Define a tag for your enum category
   struct EnumColorTag {};
   
   // Associate enums with their category
   template <> struct enum_category<lib_a::Color> { using type = EnumColorTag; };
   template <> struct enum_category<lib_b::Color> { using type = EnumColorTag; };
   template <> struct enum_category<lib_c::Color> { using type = EnumColorTag; };
   
   // Define mappings between equivalent enum values
   template <>
   struct enum_mapping_traits<EnumColorTag> {
       using mapping_type = std::tuple<lib_a::Color, lib_b::Color, lib_c::Color>;
       constexpr static mapping_type mappings[] = {
           { lib_a::Color::Red, lib_b::Color::Red, lib_c::Color::Red },
           { lib_a::Color::Green, lib_b::Color::Green, lib_c::Color::Green },
           { lib_a::Color::Blue, lib_b::Color::Blue, lib_c::Color::Blue }
       };
   };
   ```

2. Use `enum_cast` to convert between enum types:

   ```C++
   lib_a::Color a_color = enum_cast<lib_a::Color>(lib_b::Color::Red);
   ```

### enum_flag_bits_cast

1. Ensure that the result of bitwise operations on enums is of the enum type, not an int type:

   ```C++
   template <EnumConcept Enum>
   Enum operator|(Enum a, Enum b)
   {
       using UnderlyingType = std::underlying_type_t<Enum>;
       return static_cast<Enum>(static_cast<UnderlyingType>(a) | static_cast<UnderlyingType>(b));
   }
   ```

   

2. Define your enum categories and mappings:

   ```C++
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
   }
   ```

3. Use `enum_cast` to convert between enum types:

   ```C++
   lib_a::Permission a = enum_flag_bits_cast<lib_a::Permission>(lib_b::READ | lib_b::WRITE);
   ```


## License

MIT License - See LICENSE file for details

## Contributing

Contributions welcome! Please feel free to submit a Pull Request.