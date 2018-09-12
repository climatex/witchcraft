// Can be compiled under Visual Studio or similar
#include "private.h"

// Get class private member by offset. Syntax: return type, instance, offset in bytes.
#define get_private_ofs(typ,inst,ofs) (   (typ)( (unsigned char*)(&inst)+(ofs) )   )

// Get class private member by a known (public) reference right before it. Syntax: return type, instance, reference. 
#define get_private_ref(typ,inst,ref) (   (typ)( (unsigned char*)(&inst.ref)+sizeof(inst.ref) )   )

int main(int argc, char* argv[])
{	
	// Instantiate the class
	CTest cInstance;

	// Get pointer to CTest::m_pDummy. Right at the beginning (offset 0)
	auto pDummy1 = get_private_ofs(char**, cInstance, 0);

	// Get pointer to CTest::m_cVec. Start from the beginning, but skip the char* m_pDummy pointer.
	auto pVec = get_private_ofs(std::vector<int>*, cInstance, sizeof(char*));

	// Grow the vector and append random data to CTest::m_cStr1
	cInstance.DoSomething();

	// Get pointer to CTest::m_cStr1. Now use a known public reference (m_nReserved) and go up in RAM (-sizeof).
	auto pStr1 = get_private_ofs(std::string*, cInstance, offsetof(CTest, m_nReserved)-sizeof(std::string));

	// Get pointer to CTest::m_cStr2. Use a known public reference (nReserved) and go below (+sizeof).
	auto pStr2 = get_private_ref(std::string*, cInstance, m_nReserved);

	return 0;
}


