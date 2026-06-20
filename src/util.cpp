/*******************************************************************************
 * SAD - C++ Utility Module
 *
 * C++ utility functions for string parsing and radix conversion
 *
 * Copyright (c) 1997-2026 John Kiernan
 * Licensed under MIT License - see LICENSE file for details
 ******************************************************************************/

#include <algorithm>
#include <math.h>
#include <stdint.h>
#include <string>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wignored-qualifiers"

using namespace std;

inline const char make_ucase(const char ch)
{ return ((const char)std::toupper((int)ch)); }

/*****************************************************************************\
 Function: string_to_upper()

 Description: Returns a copy of a string in all upper case.

 Returns: std::string

\*****************************************************************************/
string string_to_upper(string const& str)
{
    string temp_str(str);
    std::transform(str.begin(), str.end(), temp_str.begin(), make_ucase);
    return temp_str;
} /* string string_to_upper(string const& str) */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/*****************************************************************************\
 Function: _strnum()

 Description: converts a string into an integer

 Returns: uint64_t

\*****************************************************************************/
uint64_t _strnum(string value, int radix, bool& valid)
{
    uint64_t ret = 0;
    valid        = true;

    switch (radix) {
        case 2:
            for (unsigned long i = 0; i < value.length(); ++i) {
                char cur_char = value[(value.length() - 1) - i];
                if (cur_char == '1')
                    ret += 1 * (1 << i);
                else if (cur_char != '0') {
                    valid = false;
                    return (0);
                }
            }
            break;

        case 10:
            for (unsigned long i = 0; i < value.length(); ++i) {
                char cur_char = value[(value.length() - 1) - i];
                if ((cur_char >= '0') && (cur_char <= '9')) {
                    uint64_t t = (cur_char - '0');
                    ret += t * static_cast<uint64_t>(pow((double)radix, (double)i));
                }
                else {
                    valid = false;
                    return (0);
                }
            }
            break;

        case 8: {
            for (unsigned long i = 0; i < value.length(); ++i) {
                char cur_char = value[(value.length() - 1) - i];
                if ((cur_char >= '0') && (cur_char <= '7')) {
                    uint64_t t = (cur_char - '0');
                    ret += t * static_cast<uint64_t>(pow((double)radix, (double)i));
                }
                else {
                    valid = false;
                    return (0);
                }
            }
        } /* case 8: */
        break;

        case 16: {
            string up_value = string_to_upper(value);
            for (unsigned long i = 0; i < value.length(); ++i) {
                char cur_char = up_value[(value.length() - 1) - i];
                if ((cur_char >= '0') && (cur_char <= '9')) {
                    uint64_t t = (cur_char - '0');
                    ret += t * static_cast<uint64_t>(pow((double)radix, (double)i));
                }
                else if ((cur_char >= 'A') && (cur_char <= 'F')) {
                    uint64_t t = (cur_char - 'A') + 10;
                    ret += t * static_cast<uint64_t>(pow((double)radix, (double)i));
                }
                else {
                    valid = false;
                    return (0);
                }
            }
        } break;

        default:
            valid = false;
            ret   = 0;
            break;
    };

    return ret;
}
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/*****************************************************************************\
 Function: trim_left()

 Description: 'trims' a strings left side, removing whitepsace.

               optional char_list can be characters to remove instead of
               whitespace

 Returns: std::string

\*****************************************************************************/
string trim_left(string const& str, const char* char_list /*=NULL*/)
{
    string char_list_str;

    if (char_list == NULL) {
        // Default is to remove 'whitespace'
        char_list_str = " \t\n\r";
    } /* if (char_list == NULL) */
    else
        char_list_str = char_list;

    string new_string(str);
    if (new_string.empty()) return new_string;

    string::size_type keep_pos = new_string.find_first_not_of(char_list_str);
    new_string.erase(0, keep_pos);

    return new_string;

} /* string trim_left(const string& str, const char *char_list) */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

extern "C" uint64_t string_to_uint64(const char* val, bool* valid)
{
    // Determine the radix
    int    requested_radix = 10; // Default to base 10
    string value(val);

    if ((val[0] == '0') && (val[1] == 'x')) {
        requested_radix = 16;
        value.erase(0, 2);
    } // if ((val[0]=='0') && (val[1]=='x'))

    if ((val[0] == '0') && (val[1] == 'o')) {
        requested_radix = 8;
        value.erase(0, 2);
    } // if ((val[0]=='0') && (val[1]=='o'))

    if ((val[0] == '0') && (val[1] == 'b')) {
        requested_radix = 2;
        value.erase(0, 2);
    } // if ((val[0]=='0') && (val[1]=='b'))

    return (_strnum(value, requested_radix, *valid));

} // uint64_t string_to_uint64(const char *val)

#pragma clang diagnostic pop