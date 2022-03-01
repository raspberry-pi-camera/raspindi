// C++ code adapted from https://www.geeksforgeeks.org/convert-given-float-value-to-equivalent-fraction/

#include <bits/stdc++.h>
using namespace std;

// Function to convert the floating
// values into fraction

struct fraction_t
{
    int num;
    int den;
};

fraction_t findFraction(float f)
{
    char chr[10];
    sprintf(chr, "%.2f", f);
    
    string s(chr);
	// Initialize variables
	string be_deci = "",
		af_deci = "",
		reccu = "";

	bool x = true, y = false,
		z = false;

	// Traverse the floating string
	for (int i = 0; i < s.size(); ++i) {

		// Check if decimal part exist
		if (s[i] == '.') {
			x = false;
			y = true;
			continue;
		}

		// Retrieve decimal part
		if (x)
			be_deci += s[i];

		if (y)
			af_deci += s[i];
	}

	// Convert string to integer
	int num_be_deci = stoi(be_deci);
    int num_af_deci = stoi(af_deci);

	// Initialize numerator & denominator
	int numr = num_be_deci
				* pow(10, af_deci.size())
			+ num_af_deci;

	int deno = pow(10, af_deci.size());

    int gd = __gcd(numr, deno);

    // Print the result
    fraction_t res;

    res.num = numr / gd;
    res.den = deno / gd;
    return res;
}