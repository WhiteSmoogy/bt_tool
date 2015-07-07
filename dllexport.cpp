#include <Windows.h>
#include <Dbghelp.h>
#include <cstdio>


int main() {
	LoadLibrary(L"cpprest140_2_6.dll");
	auto module = GetModuleHandle(L"cpprest140_2_6.dll");
	ULONG ulSize;
	auto importdesc = (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(
		module, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &ulSize);

	auto iter = importdesc;
	while (iter->Name) {
		auto pszModuleName = (PSTR)((PBYTE)module + iter->Name);
		std::printf(pszModuleName);
		std::printf("\n");

		if (iter->Characteristics == 0)
			std::printf("\tnull import descriptor\n");
		else {
			auto pThunk =  (PIMAGE_THUNK_DATA)((PBYTE)module + iter->OriginalFirstThunk);
			for (;pThunk->u1.Function;++pThunk) {
				if (pThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG32){
					std::printf("\t%d\n",pThunk->u1.Ordinal & 0x0000FFFF);
					continue;
				}
				auto pName = (PIMAGE_IMPORT_BY_NAME)((PBYTE)module + pThunk->u1.AddressOfData);
				
				std::printf("\t%s\n", pName->Name);
			}
			//std::printf("\t%s", pName->Name);
		}

		++iter;
	}

	return 0;
}