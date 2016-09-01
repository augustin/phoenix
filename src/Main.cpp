/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include <clocale>

#include "Phoenix.h"

using std::vector;
using std::string;

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "C"); // Force C locale
	vector<string> arguments(argv, argv + argc);

	return PhoenixMain(arguments);
}
