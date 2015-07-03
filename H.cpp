#include<cstring>
#include<cassert>
#include<cstdint>
#include<type_traits>
#include<algorithm>

template<typename scalar, std::size_t N>
auto arrlen(scalar(&)[N]) {
	return N;
}

class String {
public:
	using Char = char;

	enum : std::size_t {
		kNpos = std::size_t(-1)
	};


private:
	union xStorage {
		struct {
			Char achData[(4 * sizeof(void *)) / sizeof(Char) - 2];
			Char chNull;
			std::make_unsigned_t<Char> uchLength;
		} vSmall;

		struct {
			Char *pchBegin;
			std::size_t uLength;
			std::size_t uCapacity;
		} vLarge;
	} x_vStorage;

	class Observer {
	private:
		const Char *x_pchBegin;
		const Char *x_pchEnd;
	private:
		static std::size_t xTranslateOffset(std::ptrdiff_t nOffset, std::size_t uLength) noexcept {
			auto uRet = (std::size_t)nOffset;
			if (nOffset < 0) {
				uRet += uLength + 1;
			}
			assert(uRet <= uLength);// L"索引越界。"
			return uRet;
		}

		template<typename CharT>
		static const CharT *StrEndOf(const CharT *pszBegin) noexcept {
			assert(pszBegin);

			auto pchEnd = pszBegin;
			while (*pchEnd != CharT()) {
				++pchEnd;
			}
			return pchEnd;
		}

		template<typename CharT, typename IteratorT>
		static std::size_t StrChrRep(IteratorT itBegin, std::common_type_t<IteratorT> itEnd,
			CharT chToFind, std::size_t uRepCount) noexcept
		{
			assert(uRepCount != 0);
			assert((std::size_t)(itEnd - itBegin) >= uRepCount);

			const auto itSearchEnd = itEnd - (std::ptrdiff_t)(uRepCount - 1);

			std::size_t uFound = kNpos;

			auto itCur = itBegin;
			do {
				const auto itPartBegin = std::find_if(itCur, itSearchEnd,
					[chToFind](CharT ch) noexcept { return ch == chToFind; });
				if (itPartBegin == itSearchEnd) {
					break;
				}
				const auto itPartEnd = itPartBegin + (std::ptrdiff_t)uRepCount;
				itCur = std::find_if(itPartBegin, itPartEnd,
					[chToFind](CharT ch) noexcept { return ch != chToFind; });
				if (itCur == itPartEnd) {
					uFound = (std::size_t)(itPartBegin - itBegin);
					break;
				}
				++itCur;
			} while (itCur < itSearchEnd);

			return uFound;
		}


		template<typename IteratorT, typename ToFindIteratorT>
		static std::size_t StrStr(IteratorT itBegin, IteratorT itEnd,
			ToFindIteratorT itToFindBegin, ToFindIteratorT itToFindEnd) noexcept
		{
			assert(itToFindEnd >= itToFindBegin);
			assert(itEnd - itBegin >= itToFindEnd - itToFindBegin);

			const auto uToFindLen = (std::size_t)(itToFindEnd - itToFindBegin);
			const auto itSearchEnd = itEnd - (std::ptrdiff_t)(uToFindLen - 1);

			std::size_t *puKmpTable;

			std::size_t auSmallTable[256];
			if (uToFindLen <= arrlen(auSmallTable)) {
				puKmpTable = auSmallTable;
			}
			else {
				puKmpTable = new(std::nothrow) std::size_t[uToFindLen];
				if (!puKmpTable) {
					// 内存不足，使用暴力搜索方法。
					for (auto itCur = itBegin; itCur != itSearchEnd; ++itCur) {
						if (std::equal(itToFindBegin, itToFindEnd, itCur)) {
							return (std::size_t)(itCur - itBegin);
						}
					}
					return kNpos;
				}
			}

			std::size_t uFound = kNpos;

			puKmpTable[0] = 0;
			puKmpTable[1] = 0;

			std::size_t uPos = 2;
			std::size_t uCand = 0;
			while (uPos < uToFindLen) {
				if (itToFindBegin[(std::ptrdiff_t)(uPos - 1)] == itToFindBegin[(std::ptrdiff_t)uCand]) {
					puKmpTable[uPos++] = ++uCand;
				}
				else if (uCand != 0) {
					uCand = puKmpTable[uCand];
				}
				else {
					puKmpTable[uPos++] = 0;
				}
			}

			auto itCur = itBegin;
			std::size_t uToSkip = 0;
			do {
				const auto vResult = std::mismatch(
					itToFindBegin + (std::ptrdiff_t)uToSkip, itToFindEnd, itCur + (std::ptrdiff_t)uToSkip);
				if (vResult.first == itToFindEnd) {
					uFound = (std::size_t)(itCur - itBegin);
					break;
				}
				auto uDelta = (std::size_t)(vResult.first - itToFindBegin);
				uToSkip = puKmpTable[uDelta];
				uDelta -= uToSkip;
				uDelta += (std::size_t)(*vResult.second != *itToFindBegin);
				itCur += (std::ptrdiff_t)uDelta;
			} while (itCur < itSearchEnd);

			if (puKmpTable != auSmallTable) {
				delete[] puKmpTable;
			}
			return uFound;
		}

	public:
		const Char *GetBegin() const noexcept {
			return x_pchBegin;

		}
		const Char *GetEnd() const noexcept {
			return x_pchEnd;

		}
		std::size_t GetSize() const noexcept {
			return (std::size_t)(x_pchEnd - x_pchBegin);
		}

		constexpr Observer(const Char *pchBegin, std::size_t uLen) noexcept
			: x_pchBegin(pchBegin), x_pchEnd(pchBegin + uLen)
		{
		}

		constexpr Observer(const Char *pchBegin, const Char *pchEnd) noexcept
			: x_pchBegin(pchBegin), x_pchEnd(pchEnd)
		{
		}

		explicit Observer(const Char *pszBegin) noexcept
			: Observer(pszBegin, StrEndOf(pszBegin))
		{
		}

		Observer Slice(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd) const noexcept {
			const auto uLength = GetSize();
			return Observer(x_pchBegin + xTranslateOffset(nBegin, uLength), x_pchBegin + xTranslateOffset(nEnd, uLength));
		}

		std::size_t Find(const Observer &obsToFind, std::ptrdiff_t nBegin = 0) const noexcept {
			const auto uLength = GetSize();
			const auto uRealBegin = xTranslateOffset(nBegin, uLength);
			const auto uLenToFind = obsToFind.GetSize();
			if (uLenToFind == 0) {
				return uRealBegin;
			}
			if (uLength < uLenToFind) {
				return kNpos;
			}
			if (uRealBegin + uLenToFind > uLength) {
				return kNpos;
			}
			const auto uPos = StrStr(GetBegin() + uRealBegin, GetEnd(), obsToFind.GetBegin(), obsToFind.GetEnd());
			if (uPos == kNpos) {
				return kNpos;
			}
			return uPos + uRealBegin;
		}

		std::size_t Find(Char chToFind, std::ptrdiff_t nBegin = 0) const noexcept {
			return FindRep(chToFind, 1, nBegin);
		}


		std::size_t FindRep(Char chToFind, std::size_t uRepCount,std::ptrdiff_t nBegin = 0) const noexcept {
			const auto uLength = GetSize();
			const auto uRealBegin = xTranslateOffset(nBegin, uLength);
			if (uRepCount == 0) {
				return uRealBegin;
			}
			if (uLength < uRepCount) {
				return kNpos;
			}
			if (uRealBegin < uRepCount) {
				return kNpos;
			}
			const auto uPos = StrChrRep(GetBegin() + uRealBegin, GetEnd(), chToFind, uRepCount);
			if (uPos == kNpos) {
				return kNpos;
			}
			return uPos + uRealBegin;

		}
		bool DoesOverlapWith(const Observer &rhs) const noexcept {
			return std::less<void>()(x_pchBegin, rhs.x_pchEnd) && std::less<void>()(rhs.x_pchBegin, x_pchEnd);
		}

	};
public:
	String() noexcept {
		x_vStorage.vSmall.chNull = Char();
		x_vStorage.vSmall.uchLength = 0;
	}

	explicit String(const Char *pszBegin)
		: String()
	{
		Append(pszBegin);
	}

	String &operator=(const String &rhs) {
		Assign(rhs);
		return *this;
	}
	String &operator=(String &&rhs) noexcept {
		Assign(std::move(rhs));
		return *this;
	}

	~String() noexcept {
		if (x_vStorage.vSmall.chNull != Char()) {
			delete[] x_vStorage.vLarge.pchBegin;
		}
#ifndef NDEBUG
		std::memset(&x_vStorage, 0xCC, sizeof(x_vStorage));
#endif
	}

private:
	Char *xChopAndSplice(std::size_t uRemovedBegin, std::size_t uRemovedEnd,
		std::size_t uFirstOffset, std::size_t uThirdOffset)
	{
		const auto pchOldBuffer = GetBegin();
		const auto uOldLength = GetSize();
		auto pchNewBuffer = pchOldBuffer;
		const auto uNewLength = uThirdOffset + (uOldLength - uRemovedEnd);
		auto uSizeToAlloc = uNewLength + 1;

		assert(uRemovedBegin <= uOldLength);
		assert(uRemovedEnd <= uOldLength);
		assert(uRemovedBegin <= uRemovedEnd);
		assert(uFirstOffset + uRemovedBegin <= uThirdOffset);

		if (GetCapacity() < uNewLength) {
			uSizeToAlloc += (uSizeToAlloc >> 1);
			uSizeToAlloc = (uSizeToAlloc + 0x0F) & (std::size_t) - 0x10;
			if (uSizeToAlloc < uNewLength + 1) {
				uSizeToAlloc = uNewLength + 1;
			}
			pchNewBuffer = new Char[uSizeToAlloc];
		}

		if ((pchNewBuffer + uFirstOffset != pchOldBuffer) && (uRemovedBegin != 0)) {
			std::memmove(pchNewBuffer + uFirstOffset, pchOldBuffer, uRemovedBegin * sizeof(Char));
		}
		if ((pchNewBuffer + uThirdOffset != pchOldBuffer + uRemovedEnd) && (uOldLength != uRemovedEnd)) {
			std::memmove(pchNewBuffer + uThirdOffset, pchOldBuffer + uRemovedEnd, (uOldLength - uRemovedEnd) * sizeof(Char));
		}

		if (pchNewBuffer != pchOldBuffer) {
			if (x_vStorage.vSmall.chNull == Char()) {
				++x_vStorage.vSmall.chNull;
			}
			else {
				delete[] pchOldBuffer;
			}

			x_vStorage.vLarge.pchBegin = pchNewBuffer;
			x_vStorage.vLarge.uLength = uOldLength;
			x_vStorage.vLarge.uCapacity = uSizeToAlloc;
		}

		return pchNewBuffer + uFirstOffset + uRemovedBegin;
	}

	void xSetSize(std::size_t uNewSize) noexcept {
		assert(uNewSize <= GetCapacity());

		if (x_vStorage.vSmall.chNull == Char()) {
			x_vStorage.vSmall.uchLength =static_cast<unsigned char>(uNewSize);
		}
		else {
			x_vStorage.vLarge.uLength = uNewSize;
		}
	}
public:
	const Char *GetBegin() const noexcept {
		if (x_vStorage.vSmall.chNull == Char()) {
			return x_vStorage.vSmall.achData;
		}
		else {
			return x_vStorage.vLarge.pchBegin;
		}
	}
	Char *GetBegin() noexcept {
		if (x_vStorage.vSmall.chNull == Char()) {
			return x_vStorage.vSmall.achData;
		}
		else {
			return x_vStorage.vLarge.pchBegin;
		}
	}

	const Char *GetEnd() const noexcept {
		if (x_vStorage.vSmall.chNull == Char()) {
			return x_vStorage.vSmall.achData + x_vStorage.vSmall.uchLength;
		}
		else {
			return x_vStorage.vLarge.pchBegin + x_vStorage.vLarge.uLength;
		}
	}
	Char *GetEnd() noexcept {
		if (x_vStorage.vSmall.chNull == Char()) {
			return x_vStorage.vSmall.achData + x_vStorage.vSmall.uchLength;
		}
		else {
			return x_vStorage.vLarge.pchBegin + x_vStorage.vLarge.uLength;
		}
	}

	const Char *GetStr() const noexcept {
		const_cast<Char &>(GetEnd()[0]) = Char();
		return GetBegin();
	}
	Char *GetStr() noexcept {
		GetEnd()[0] = Char();
		return GetBegin();
	}

	void Assign(const Char* pszBegin) {
		Assign(Observer(pszBegin));
	}

	void Assign(const String &rhs) {
		if (&rhs != this) {
			Assign(rhs.GetObserver());
		}
	}
	void Assign(String &&rhs) noexcept {
		assert(this != &rhs);
		Swap(rhs);
	}

	void Assign(const Observer &rhs) {
		Resize(rhs.GetSize());
		std::copy(rhs.GetBegin(), rhs.GetEnd(), GetStr());
	}

	void Truncate(std::size_t uCount = 1) noexcept {
		const auto uOldSize = GetSize();
		assert(uOldSize >= uCount);// "删除的字符数太多。"
		xSetSize(uOldSize - uCount);
	}

	void Resize(std::size_t uNewSize) {
		const std::size_t uOldSize = GetSize();
		if (uNewSize > uOldSize) {
			Reserve(uNewSize);
			xSetSize(uNewSize);
		}
		else if (uNewSize < uOldSize) {
			Truncate(uOldSize - uNewSize);
		}
	}

	std::size_t GetSize() const noexcept {
		if (x_vStorage.vSmall.chNull == Char()) {
			return x_vStorage.vSmall.uchLength;
		}
		else {
			return x_vStorage.vLarge.uLength;
		}
	}

	std::size_t GetCapacity() const noexcept {
		if (x_vStorage.vSmall.chNull == Char()) {
			return arrlen(x_vStorage.vSmall.achData);
		}
		else {
			return x_vStorage.vLarge.uCapacity - 1;
		}
	}
	void Reserve(std::size_t uNewCapacity) {
		if (uNewCapacity > GetCapacity()) {
			const auto uOldLength = GetSize();
			xChopAndSplice(uOldLength, uOldLength, 0, uNewCapacity);
		}
	}

	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const Char* pszRep) {
		Replace(nBegin, nEnd, Observer(pszRep));
	}

	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const Observer& obsRep) {
		const auto obsCurrent(GetObserver());
		const auto uOldLength = GetSize();
		const auto obsRemoved(obsCurrent.Slice(nBegin, nEnd));
		const auto uRemovedBegin = (std::size_t)(obsRemoved.GetBegin() - GetBegin());
		const auto uRemovedEnd = (std::size_t)(obsRemoved.GetEnd() - GetBegin());

		if (obsCurrent.DoesOverlapWith(obsRep)) {
			String strTemp;
			strTemp.Resize(uRemovedBegin + obsRep.GetSize() + (uOldLength - uRemovedEnd));
			auto pchWrite = strTemp.GetStr();
			pchWrite = std::copy(obsCurrent.GetBegin(), obsCurrent.GetBegin() + uRemovedBegin, pchWrite);
			pchWrite = std::copy(obsRep.GetBegin(), obsRep.GetEnd(), pchWrite);
			pchWrite = std::copy(obsCurrent.GetBegin() + uRemovedEnd, obsCurrent.GetEnd(), pchWrite);
			Swap(strTemp);
		}
		else {
			const auto pchWrite = xChopAndSplice(uRemovedBegin, uRemovedEnd, 0, uRemovedBegin + obsRep.GetSize());
			std::copy_n(obsRep.GetBegin(), obsRep.GetSize(), pchWrite);
			xSetSize(uRemovedBegin + obsRep.GetSize() + (uOldLength - uRemovedEnd));
		}
	}

	void Swap(String &rhs) noexcept {
		xStorage vStorage;
		std::memcpy(&vStorage, &x_vStorage, sizeof(vStorage));
		std::memcpy(&x_vStorage, &rhs.x_vStorage, sizeof(vStorage));
		std::memcpy(&rhs.x_vStorage, &vStorage, sizeof(vStorage));
	}

	std::size_t Find(const Char* pszToFind, std::ptrdiff_t nBegin = 0) const noexcept {
		return GetObserver().Find(Observer(pszToFind), nBegin);
	}

	std::size_t Find(Char chToFind, std::ptrdiff_t nBegin = 0) const noexcept {
		return GetObserver().Find(chToFind, nBegin);
	}

	Observer GetObserver() const noexcept {
		if (x_vStorage.vSmall.chNull == Char()) {
			return Observer(x_vStorage.vSmall.achData, x_vStorage.vSmall.uchLength);
		}
		else {
			return Observer(x_vStorage.vLarge.pchBegin, x_vStorage.vLarge.uLength);
		}
	}

	void Append(const Char *pszBegin) {
		Append(Observer(pszBegin));
	}
	void Append(const Char *pchBegin, const Char *pchEnd) {
		Append(Observer(pchBegin, pchEnd));
	}
	void Append(const Observer &rhs) {
		Replace(-1, -1, rhs);
	}

	void Insert(std::size_t npos, const Char* pszIns) {
		Insert(npos, Observer(pszIns));
	}

	void Insert(std::size_t npos, const Observer& obsIns) {
		Replace(npos, npos, obsIns);
	}

	void Clear() noexcept {
		xSetSize(0);
	}
};

#include <iostream>

int main(int argc, char* argv[]) {
	int code = 0;
	String s;
	do {
		std::cout << "menu:-------\n";
		std::cout << "\t\t1.\t生成字符串\n";
		std::cout << "\t\t2.\t显   示\n";
		std::cout << "\t\t3.\t求字符串长度\n";
		std::cout << "\t\t4.\t字符串插入\n";
		std::cout << "\t\t5.\t字符串替换\n";
		std::cout << "\t\t6.\t字符串删除\n";
		std::cout << "\t\t7.\t字符串查找(0表示未找到)\n";
		std::cout << "\t\t8.\t字符串的连接\n";
		std::cout << "\t\t0.\t返回\n";
		std::cin >> code;
		switch (code)
		{
		case 1:
			s = String("ABCDBCEFGHTH");
			break;
		case 2:
			printf("String: %s\n",s.GetStr());
			break;
		case 3:
			printf("Length: %u\n",s.GetSize());
			break;
		case 4:
			s.Insert(3, "XYZ");
			break;
		case 5:
		{
			std::size_t index;
			while( (index = s.Find("BC")) != String::kNpos ){
				s.Replace(s.Find("BC"), s.Find("BC")+2,"LMN");
			}
		}
			break;
		case 6:
			s.Clear();
			break;
		case 7:
			printf("Index: %u\n",s.Find("BC")+1);
			break;
		case 8:
			s.Append("12345678");
			break;
		default:
			printf("String: %s\n", s.GetStr());
			break;
		}
	} while (code);
	
	return 0;
}