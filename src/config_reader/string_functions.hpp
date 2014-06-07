/** String Functions */
#pragma once

#include <string>

namespace string_fun {

/// Trim white space ' ' + '\t' off both ends of a string
void trim(std::string& string);

/** Dequotes a string
 *
 * @param input A string surrounded with '"'s.
 * @param output The string to write the decoded string out from
 * @returns A pointer to the character after the last quote or input.end()
 *
 * Things it'll translate
 *  * \\ => \
 *  * \" => "
 *  * \x => \x (where 'x' is any other character)
 *  
 */
std::string::const_iterator dequoteString(const std::string& input, std::string& output);

}
