#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <armadillo>
#include <arrayfire.h>
#include <map>
#include <set>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <cmath>
#include <cctype>
#include <iomanip>
#include <bitset>
#include <queue>
#include <boost/function.hpp>
#include <iterator>
#include "Storage.hpp"

using namespace std;
//using namespace arma;

inline 
double sign(const double& x)
{
	if (x==0)
		return 0;
	else
		return x>0?+1:-1;
}