#pragma once

#include <time.h>

#include <string>
#include <vector>

// Watch the alignment (or use pragma pack 1)
class CTest
{
public:
	CTest()
	{
		srand((unsigned)time(NULL));

		// Just fill the buffer
		m_pDummy = new char[40];
		memset(m_pDummy, 'A', 39);
		m_pDummy[39] = {0};		
	}

	~CTest()
	{
		delete[] m_pDummy;
	}

	void DoSomething()
	{
		// Called in the middle
		for (int i = 0; i < 400; i++)
			m_cVec.push_back(rand() % 30);		//Grow the vector...

		for (int j = 0; j < rand() % 150 + 40; j++)
		{
			char cAlpha[2] = {0};
			cAlpha[0] = rand() % 25 + 65; 		// Capital letters
			m_cStr1.append(cAlpha);		  	// ...and grow the string
		}
	}

private:
	char* m_pDummy = NULL;				  	// Will be accessing these three private members
	std::vector<int> m_cVec;
	std::string m_cStr1 = "string1";

public:
	unsigned long long m_nReserved = 500; 			// Just a reference member variable that is public

private:
	std::string m_cStr2 = "string2";      			// And also this one
};