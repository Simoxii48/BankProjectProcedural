#pragma once

#include <iostream>
#include <iomanip>

#undef max

using namespace std;

namespace validation
{
	int readValidateNumber(int from, int to, string message)
	{
		int number = 0;
		cout << "\n" << right << setw(5) << " " << message;

		while (!(cin >> number) || number < from || number > to)
		{
			cout << endl;
			cout << right << setw(5) << " " << "Invalid (Range / Datatype) please enter a Number from " << from << " To " << to << " : ";
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
		}

		return number;
	}
}