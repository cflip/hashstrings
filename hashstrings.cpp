#include <algorithm>
#include <cmath>
#include <cstring>
#include <execution>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>

#define MAX_GENERATED_STRING_LENGTH 8

// Toggles whether generated strings are printed as they are generated. It looks
// cool and lets you see the progress, but slows down the process a lot.
#define PRINT_GENERATED_STRINGS 0

// This is the minimum length for generating strings where we start computing
// across separate threads instead of in parallel.
#define MULTITHREADING_THRESHOLD 6

// This function uses the same algorithm as Java's String.hashCode method. This
// implementation was stolen from OpenJDK.
int compute_hash_code(const std::string& str)
{
	int result = 0;
	for (int i = 0; i < str.length(); i++)
		result = 31 * result + str[i];
	return result;
}

// This function checks if the hash matches for both the original string and the
// same string with the first letter capitalized.
bool compare_with_case_insensitivity(std::string& str, int hash_to_find)
{
	int hash_code = compute_hash_code(str);
	if (hash_code == hash_to_find) {
		std::cout << str << std::endl;
		return true;
	}

	// Early exit if the first character is already uppercase.
	if (std::isupper(str[0]))
		return false;

	str[0] = std::toupper(str[0]);
	hash_code = compute_hash_code(str);
	if (hash_code == hash_to_find) {
		std::cout << str << std::endl;
		return true;
	}
	return false;
}

bool find_from_dictionary(int hash_to_find)
{
	bool found = false;

	std::ifstream in_file("/usr/share/dict/words");
	if (!in_file.is_open()) {
		std::cerr << "Failed to read dictionary file" << std::endl;
		return false;
	}

	std::string line;
	while (std::getline(in_file, line)) {
		found |= compare_with_case_insensitivity(line, hash_to_find);
	}

	in_file.close();
	return found;
}

bool generate_all_strings(const std::string& prefix, int hash_to_find, int k)
{
	bool found = false;
	if (k == 0) {
#if PRINT_GENERATED_STRINGS
		std::cout << prefix << "\t\r" << std::flush;
#endif

		// Make a copy so the comparison function can safely modify the
		// capitalization of the string.
		std::string string_to_test = prefix;
		return compare_with_case_insensitivity(string_to_test, hash_to_find);
	}

	if (k >= MULTITHREADING_THRESHOLD) {
		// Create a vector containing the alphabet so I can use std::for_each
		std::vector<char> alphabet;
		for (char c = 'a'; c <= 'z'; c++) {
			alphabet.push_back(c);
		}

		std::for_each(std::execution::par, alphabet.begin(), alphabet.end(), [&](char c) {
			std::string new_prefix = prefix + c;
			found |= generate_all_strings(new_prefix, hash_to_find, k - 1);
		});
	} else {
		for (char c = 'a'; c <= 'z'; c++) {
			std::string new_prefix = prefix + c;
			found |= generate_all_strings(new_prefix, hash_to_find, k - 1);
		}
	}

	return found;
}

// Generate every possible string using the lowercase alphabet with sizes up to
// max_length and test each and every one to see if it matches the wanted value.
bool find_from_generated_strings(int hash_to_find, int max_length)
{
	bool found = false;

	std::string str;
	for (int length = 1; length <= max_length; length++) {
		std::cout << "Checking strings of length " << length << '\n';
		found |= generate_all_strings(str, hash_to_find, length);
	}

	return found;
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		std::cerr << "Please specify a hash code to find strings for.\n";
		return 1;
	}

	if (strncmp(argv[1], "compute", 7) == 0) {
		if (argc < 3) {
			std::cerr << "Please specify a string to compute a hash for.\n";
			return 1;
		}
		std::cout << compute_hash_code(argv[2]) << std::endl;
		return 0;
	}

	int hash_to_find = std::atoi(argv[1]);
	if (hash_to_find == 0) {
		std::cerr << "The hash code you specified was invalid.\n";
		return 1;
	}

	bool found = false;

	std::cout << "Searching dictionary... " << std::endl;
	found = find_from_dictionary(hash_to_find);
	if (!found)
		std::cout << "Nothing found." << std::endl;

	std::cout << "Generating names up to " << MAX_GENERATED_STRING_LENGTH << " chars..." << std::endl;
	found = find_from_generated_strings(hash_to_find, MAX_GENERATED_STRING_LENGTH);
	if (!found)
		std::cout << "Nothing found." << std::endl;

	return 0;
}
