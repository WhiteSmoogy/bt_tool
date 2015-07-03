#include <string>
class MyString
{
public:
	MyString();
	~MyString();

	MyString(char* str);

	int len() const;

	void insert(char* dst, char* src);

	void replace(int pos, char* src);

	int find(char * src);

	void cat(char* src);
private:
	char* _str;
	char* _end;
};



#include <type_traits>
#include <cstring>
#include <cstddef>
#include <cstdint>


	class String {
	public:
		using Observer = String;

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
		String(const Char *pchBegin, const Char *pchEnd)
			: String()
		{
			Append(pchBegin, pchEnd);
		}
		String(const Char *pchBegin, std::size_t uLen)
			: String()
		{
			Append(pchBegin, uLen);
		}
		
		String(const String &rhs)
			: String()
		{
			Append(rhs);
		}
		String(String &&rhs) noexcept
			: String()
		{
			Swap(rhs);
		}
		
		String &operator=(Char ch) noexcept {
			Assign(ch);
			return *this;
		}
		String &operator=(const Char *pszBegin) {
			Assign(pszBegin);
			return *this;
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
			const auto uOldLength = GetLength();
			auto pchNewBuffer = pchOldBuffer;
			const auto uNewLength = uThirdOffset + (uOldLength - uRemovedEnd);
			auto uSizeToAlloc = uNewLength + 1;

			ASSERT(uRemovedBegin <= uOldLength);
			ASSERT(uRemovedEnd <= uOldLength);
			ASSERT(uRemovedBegin <= uRemovedEnd);
			ASSERT(uFirstOffset + uRemovedBegin <= uThirdOffset);

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
			ASSERT(uNewSize <= GetCapacity());

			if (x_vStorage.vSmall.chNull == Char()) {
				x_vStorage.vSmall.uchLength = uNewSize;
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

		const Char *GetData() const noexcept {
			return GetBegin();
		}
		Char *GetData() noexcept {
			return GetBegin();
		}
		std::size_t GetSize() const noexcept {
			if (x_vStorage.vSmall.chNull == Char()) {
				return x_vStorage.vSmall.uchLength;
			}
			else {
				return x_vStorage.vLarge.uLength;
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
		std::size_t GetLength() const noexcept {
			return GetSize();
		}

		Observer GetObserver() const noexcept {
			if (x_vStorage.vSmall.chNull == Char()) {
				return Observer(x_vStorage.vSmall.achData, x_vStorage.vSmall.uchLength);
			}
			else {
				return Observer(x_vStorage.vLarge.pchBegin, x_vStorage.vLarge.uLength);
			}
		}

		std::size_t GetCapacity() const noexcept {
			if (x_vStorage.vSmall.chNull == Char()) {
				return COUNT_OF(x_vStorage.vSmall.achData);
			}
			else {
				return x_vStorage.vLarge.uCapacity - 1;
			}
		}
		void Reserve(std::size_t uNewCapacity) {
			if (uNewCapacity > GetCapacity()) {
				const auto uOldLength = GetLength();
				xChopAndSplice(uOldLength, uOldLength, 0, uNewCapacity);
			}
		}
		void ReserveMore(std::size_t uDeltaCapacity) {
			const auto uOldLength = GetLength();
			xChopAndSplice(uOldLength, uOldLength, 0, uOldLength + uDeltaCapacity);
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
		Char *ResizeFront(std::size_t uDeltaSize) {
			const auto uOldSize = GetSize();
			xChopAndSplice(uOldSize, uOldSize, uDeltaSize, uOldSize + uDeltaSize);
			xSetSize(uOldSize + uDeltaSize);
			return GetData();
		}
		Char *ResizeMore(std::size_t uDeltaSize) {
			const auto uOldSize = GetSize();
			xChopAndSplice(uOldSize, uOldSize, 0, uOldSize + uDeltaSize);
			xSetSize(uOldSize + uDeltaSize);
			return GetData() + uOldSize;
		}
		void Shrink() noexcept {
			const auto uSzLen = Observer(GetStr()).GetLength();
			ASSERT(uSzLen <= GetSize());
			xSetSize(uSzLen);
		}

		bool IsEmpty() const noexcept {
			return GetBegin() == GetEnd();
		}
		void Clear() noexcept {
			xSetSize(0);
		}

		void Swap(String &rhs) noexcept {
			xStorage vStorage;
			std::memcpy(&vStorage, &x_vStorage, sizeof(vStorage));
			std::memcpy(&x_vStorage, &rhs.x_vStorage, sizeof(vStorage));
			std::memcpy(&rhs.x_vStorage, &vStorage, sizeof(vStorage));
		}

		int Compare(const Observer &rhs) const noexcept {
			return GetObserver().Compare(rhs);
		}
		int Compare(const String &rhs) const noexcept {
			return GetObserver().Compare(rhs.GetObserver());
		}

		void Assign(Char ch, std::size_t uCount = 1) {
			Resize(uCount);
			FillN(GetStr(), uCount, ch);
		}
		void Assign(const Char *pszBegin) {
			Assign(Observer(pszBegin));
		}
		void Assign(const Char *pchBegin, const Char *pchEnd) {
			Assign(Observer(pchBegin, pchEnd));
		}
		void Assign(const Char *pchBegin, std::size_t uCount) {
			Assign(Observer(pchBegin, uCount));
		}
		void Assign(const Observer &rhs) {
			Resize(rhs.GetSize());
			Copy(GetStr(), rhs.GetBegin(), rhs.GetEnd());
		}
		void Assign(std::initializer_list<Char> rhs) {
			Assign(Observer(rhs));
		}
		template<StringType OTHER_kTypeT>
		void Assign(const StringObserver<OTHER_kTypeT> &rhs) {
			Clear();
			Append(rhs);
		}
		template<StringType OTHER_kTypeT>
		void Assign(const String<OTHER_kTypeT> &rhs) {
			Assign(StringObserver<OTHER_kTypeT>(rhs));
		}
		void Assign(const String &rhs) {
			if (&rhs != this) {
				Assign(Observer(rhs));
			}
		}
		void Assign(String &&rhs) noexcept {
			ASSERT(this != &rhs);
			Swap(rhs);
		}

		void Append(Char ch, std::size_t uCount = 1) {
			FillN(ResizeMore(uCount), uCount, ch);
		}
		void Append(const Char *pszBegin) {
			Append(Observer(pszBegin));
		}
		void Append(const Char *pchBegin, const Char *pchEnd) {
			Append(Observer(pchBegin, pchEnd));
		}
		void Append(const Char *pchBegin, std::size_t uCount) {
			Append(Observer(pchBegin, uCount));
		}
		void Append(const Observer &rhs) {
			Replace(-1, -1, rhs);
		}
		void Append(std::initializer_list<Char> rhs) {
			Append(Observer(rhs));
		}
		void Append(const String &rhs) {
			Append(Observer(rhs));
		}
		void Append(String &&rhs) {
			const Observer obsToAppend(rhs);
			const auto uSizeTotal = GetSize() + obsToAppend.GetSize();
			if ((GetCapacity() >= uSizeTotal) || (rhs.GetCapacity() < uSizeTotal)) {
				Append(obsToAppend);
			}
			else {
				rhs.Unshift(obsToAppend);
				Swap(rhs);
			}
		}
		template<StringType OTHER_kTypeT>
		void Append(const StringObserver<OTHER_kTypeT> &rhs) {
			UnifiedString ucsTempStorage;
			Deunify(*this, GetSize(), String<OTHER_kTypeT>::Unify(ucsTempStorage, rhs));
		}
		template<StringType OTHER_kTypeT>
		void Append(const String<OTHER_kTypeT> &rhs) {
			Append(rhs.GetObserver());
		}
		void Truncate(std::size_t uCount = 1) noexcept {
			const auto uOldSize = GetSize();
			ASSERT_MSG(uOldSize >= uCount, L"ɾ�����ַ���̫�ࡣ");
			xSetSize(uOldSize - uCount);
		}

		void Push(Char ch) {
			Append(ch, 1);
		}
		void Pop() noexcept {
			Truncate(1);
		}

		void UncheckedPush(Char ch) noexcept {
			ASSERT_MSG(GetLength() < GetCapacity(), L"����������");

			if (x_vStorage.vSmall.chNull == Char()) {
				x_vStorage.vSmall.achData[x_vStorage.vSmall.uchLength] = ch;
				++x_vStorage.vSmall.uchLength;
			}
			else {
				x_vStorage.vLarge.pchBegin[x_vStorage.vLarge.uLength] = ch;
				++x_vStorage.vLarge.uLength;
			}
		}
		void UncheckedPop() noexcept {
			ASSERT_MSG(GetLength() != 0, L"�����ѿա�");

			if (x_vStorage.vSmall.chNull == Char()) {
				--x_vStorage.vSmall.uchLength;
			}
			else {
				--x_vStorage.vLarge.uLength;
			}
		}

		void Unshift(Char ch, std::size_t uCount = 1) {
			FillN(ResizeFront(uCount), uCount, ch);
		}
		void Unshift(const Char *pszBegin) {
			Unshift(Observer(pszBegin));
		}
		void Unshift(const Char *pchBegin, const Char *pchEnd) {
			Unshift(Observer(pchBegin, pchEnd));
		}
		void Unshift(const Char *pchBegin, std::size_t uCount) {
			Unshift(Observer(pchBegin, uCount));
		}
		void Unshift(const Observer &obs) {
			Replace(0, 0, obs);
		}
		void Unshift(std::initializer_list<Char> rhs) {
			Unshift(Observer(rhs));
		}
		void Unshift(const String &rhs) {
			Unshift(Observer(rhs));
		}
		void Unshift(String &&rhs) {
			const Observer obsToAppend(rhs);
			const auto uSizeTotal = GetSize() + obsToAppend.GetSize();
			if ((GetCapacity() >= uSizeTotal) || (rhs.GetCapacity() < uSizeTotal)) {
				Unshift(obsToAppend);
			}
			else {
				rhs.Append(obsToAppend);
				Swap(rhs);
			}
		}
		template<StringType OTHER_kTypeT>
		void Unshift(const StringObserver<OTHER_kTypeT> &rhs) {
			UnifiedString ucsTempStorage;
			Deunify(*this, 0, String<OTHER_kTypeT>::Unify(ucsTempStorage, rhs));
		}
		template<StringType OTHER_kTypeT>
		void Unshift(const String<OTHER_kTypeT> &rhs) {
			Unshift(rhs.GetObserver());
		}
		void Shift(std::size_t uCount = 1) noexcept {
			const auto uOldSize = GetSize();
			ASSERT_MSG(uOldSize >= uCount, L"ɾ�����ַ���̫�ࡣ");
			const auto pchWrite = GetBegin();
			CopyN(pchWrite, pchWrite + uCount, uOldSize - uCount);
			xSetSize(uOldSize - uCount);
		}

		Observer Slice(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd = -1) const noexcept {
			return GetObserver().Slice(nBegin, nEnd);
		}
		String SliceStr(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd = -1) const {
			return String(Slice(nBegin, nEnd));
		}

		void Reverse() noexcept {
			auto pchBegin = GetBegin();
			auto pchEnd = GetEnd();
			if (pchBegin != pchEnd) {
				--pchEnd;
				while (pchBegin < pchEnd) {
					std::iter_swap(pchBegin++, pchEnd--);
				}
			}
		}

		std::size_t Find(const Observer &obsToFind, std::ptrdiff_t nBegin = 0) const noexcept {
			return GetObserver().Find(obsToFind, nBegin);
		}
		std::size_t FindBackward(const Observer &obsToFind, std::ptrdiff_t nEnd = -1) const noexcept {
			return GetObserver().FindBackward(obsToFind, nEnd);
		}
		std::size_t FindRep(Char chToFind, std::size_t uRepCount, std::ptrdiff_t nBegin = 0) const noexcept {
			return GetObserver().FindRep(chToFind, uRepCount, nBegin);
		}
		std::size_t FindRepBackward(Char chToFind, std::size_t uRepCount, std::ptrdiff_t nEnd = -1) const noexcept {
			return GetObserver().FindRepBackward(chToFind, uRepCount, nEnd);
		}
		std::size_t Find(Char chToFind, std::ptrdiff_t nBegin = 0) const noexcept {
			return GetObserver().Find(chToFind, nBegin);
		}
		std::size_t FindBackward(Char chToFind, std::ptrdiff_t nEnd = -1) const noexcept {
			return GetObserver().FindBackward(chToFind, nEnd);
		}

		void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, Char chRep, std::size_t uCount = 1) {
			const auto obsCurrent(GetObserver());
			const auto uOldLength = obsCurrent.GetLength();

			const auto obsRemoved(obsCurrent.Slice(nBegin, nEnd));
			const auto uRemovedBegin = (std::size_t)(obsRemoved.GetBegin() - obsCurrent.GetBegin());
			const auto uRemovedEnd = (std::size_t)(obsRemoved.GetEnd() - obsCurrent.GetBegin());

			const auto pchWrite = xChopAndSplice(uRemovedBegin, uRemovedEnd, 0, uRemovedBegin + uCount);
			FillN(pchWrite, uCount, chRep);
			xSetSize(uRemovedBegin + uCount + (uOldLength - uRemovedEnd));
		}
		void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const Char *pchRepBegin) {
			Replace(nBegin, nEnd, Observer(pchRepBegin));
		}
		void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const Char *pchRepBegin, const Char *pchRepEnd) {
			Replace(nBegin, nEnd, Observer(pchRepBegin, pchRepEnd));
		}
		void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const Char *pchRepBegin, std::size_t uLen) {
			Replace(nBegin, nEnd, Observer(pchRepBegin, uLen));
		}
		void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const Observer &obsRep) {
			const auto obsCurrent(GetObserver());
			const auto uOldLength = obsCurrent.GetLength();

			const auto obsRemoved(obsCurrent.Slice(nBegin, nEnd));
			const auto uRemovedBegin = (std::size_t)(obsRemoved.GetBegin() - obsCurrent.GetBegin());
			const auto uRemovedEnd = (std::size_t)(obsRemoved.GetEnd() - obsCurrent.GetBegin());

			if (obsCurrent.DoesOverlapWith(obsRep)) {
				String strTemp;
				strTemp.Resize(uRemovedBegin + obsRep.GetSize() + (uOldLength - uRemovedEnd));
				auto pchWrite = strTemp.GetStr();
				pchWrite = Copy(pchWrite, obsCurrent.GetBegin(), obsCurrent.GetBegin() + uRemovedBegin);
				pchWrite = Copy(pchWrite, obsRep.GetBegin(), obsRep.GetEnd());
				pchWrite = Copy(pchWrite, obsCurrent.GetBegin() + uRemovedEnd, obsCurrent.GetEnd());
				Swap(strTemp);
			}
			else {
				const auto pchWrite = xChopAndSplice(uRemovedBegin, uRemovedEnd, 0, uRemovedBegin + obsRep.GetSize());
				CopyN(pchWrite, obsRep.GetBegin(), obsRep.GetSize());
				xSetSize(uRemovedBegin + obsRep.GetSize() + (uOldLength - uRemovedEnd));
			}
		}
		template<StringType OTHER_kTypeT>
		void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const StringObserver<OTHER_kTypeT> &obsRep) {
			// �����쳣��ȫ��֤��
			const auto obsCurrent(GetObserver());
			const auto uOldLength = obsCurrent.GetLength();

			const auto obsRemoved(obsCurrent.Slice(nBegin, nEnd));
			const auto uRemovedBegin = (std::size_t)(obsRemoved.GetBegin() - obsCurrent.GetBegin());
			const auto uRemovedEnd = (std::size_t)(obsRemoved.GetEnd() - obsCurrent.GetBegin());

			const auto pchWrite = xChopAndSplice(uRemovedBegin, uRemovedEnd, 0, uRemovedBegin);
			xSetSize(uRemovedBegin + (uOldLength - uRemovedEnd));
			UnifiedString ucsTempStorage;
			Deunify(*this, uRemovedBegin, String<OTHER_kTypeT>::Unify(ucsTempStorage, obsRep));
		}
		template<StringType OTHER_kTypeT>
		void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const String<OTHER_kTypeT> &strRep) {
			Replace(nBegin, nEnd, strRep.GetObserver());
		}

	public:
		operator Observer() const noexcept {
			return GetObserver();
		}

		explicit operator bool() const noexcept {
			return !IsEmpty();
		}
		explicit operator const Char *() const noexcept {
			return GetStr();
		}
		explicit operator Char *() noexcept {
			return GetStr();
		}
		const Char &operator[](std::size_t uIndex) const noexcept {
			ASSERT_MSG(uIndex <= GetLength(), L"����Խ�硣");
			return GetBegin()[uIndex];
		}
		Char &operator[](std::size_t uIndex) noexcept {
			ASSERT_MSG(uIndex <= GetLength(), L"����Խ�硣");
			return GetBegin()[uIndex];
		}

	public:
		using value_type = Char;

		// std::back_insert_iterator
		template<typename ParamT>
		void push_back(ParamT &&vParam) {
			Push(std::forward<ParamT>(vParam));
		}
		// std::front_insert_iterator
		template<typename ParamT>
		void push_front(ParamT &&vParam) {
			Unshift(std::forward<ParamT>(vParam));
		}
	};

	template<StringType kTypeT, StringType OTHER_kTypeT>
	String<kTypeT> &operator+=(String<kTypeT> &lhs, const String<OTHER_kTypeT> &rhs) {
		lhs.Append(rhs);
		return lhs;
	}
	template<StringType kTypeT, StringType OTHER_kTypeT>
	String<kTypeT> &operator+=(String<kTypeT> &lhs, const StringObserver<OTHER_kTypeT> &rhs) {
		lhs.Append(rhs);
		return lhs;
	}
	template<StringType kTypeT>
	String<kTypeT> &operator+=(String<kTypeT> &lhs, typename String<kTypeT>::Char rhs) {
		lhs.Append(rhs);
		return lhs;
	}
	template<StringType kTypeT, StringType OTHER_kTypeT>
	String<kTypeT> &&operator+=(String<kTypeT> &&lhs, const String<OTHER_kTypeT> &rhs) {
		lhs.Append(rhs);
		return std::move(lhs);
	}
	template<StringType kTypeT, StringType OTHER_kTypeT>
	String<kTypeT> &&operator+=(String<kTypeT> &&lhs, const StringObserver<OTHER_kTypeT> &rhs) {
		lhs.Append(rhs);
		return std::move(lhs);
	}
	template<StringType kTypeT>
	String<kTypeT> &&operator+=(String<kTypeT> &&lhs, typename String<kTypeT>::Char rhs) {
		lhs.Append(rhs);
		return std::move(lhs);
	}
	template<StringType kTypeT>
	String<kTypeT> &&operator+=(String<kTypeT> &&lhs, String<kTypeT> &&rhs) {
		lhs.Append(std::move(rhs));
		return std::move(lhs);
	}

	template<StringType kTypeT, StringType OTHER_kTypeT>
	String<kTypeT> operator+(const String<kTypeT> &lhs, const String<OTHER_kTypeT> &rhs) {
		String<kTypeT> strRet(lhs);
		strRet += rhs;
		return strRet;
	}
	template<StringType kTypeT, StringType OTHER_kTypeT>
	String<kTypeT> operator+(const String<kTypeT> &lhs, const StringObserver<OTHER_kTypeT> &rhs) {
		String<kTypeT> strRet(lhs);
		strRet += rhs;
		return strRet;
	}
	template<StringType kTypeT>
	String<kTypeT> operator+(const String<kTypeT> &lhs, typename String<kTypeT>::Char rhs) {
		String<kTypeT> strRet(lhs);
		strRet += rhs;
		return strRet;
	}
	template<StringType kTypeT, StringType OTHER_kTypeT>
	String<kTypeT> &&operator+(String<kTypeT> &&lhs, const String<OTHER_kTypeT> &rhs) {
		return std::move(lhs += rhs);
	}
	template<StringType kTypeT, StringType OTHER_kTypeT>
	String<kTypeT> &&operator+(String<kTypeT> &&lhs, const StringObserver<OTHER_kTypeT> &rhs) {
		return std::move(lhs += rhs);
	}
	template<StringType kTypeT>
	String<kTypeT> &&operator+(String<kTypeT> &&lhs, typename String<kTypeT>::Char rhs) {
		return std::move(lhs += rhs);
	}
	template<StringType kTypeT>
	String<kTypeT> &&operator+(String<kTypeT> &&lhs, String<kTypeT> &&rhs) {
		return std::move(lhs += std::move(rhs));
	}

	template<StringType kTypeT>
	bool operator==(const String<kTypeT> &lhs, const String<kTypeT> &rhs) noexcept {
		return lhs.GetObserver() == rhs.GetObserver();
	}
	template<StringType kTypeT>
	bool operator==(const String<kTypeT> &lhs, const StringObserver<kTypeT> &rhs) noexcept {
		return lhs.GetObserver() == rhs;
	}
	template<StringType kTypeT>
	bool operator==(const StringObserver<kTypeT> &lhs, const String<kTypeT> &rhs) noexcept {
		return lhs == rhs.GetObserver();
	}

	template<StringType kTypeT>
	bool operator!=(const String<kTypeT> &lhs, const String<kTypeT> &rhs) noexcept {
		return lhs.GetObserver() != rhs.GetObserver();
	}
	template<StringType kTypeT>
	bool operator!=(const String<kTypeT> &lhs, const StringObserver<kTypeT> &rhs) noexcept {
		return lhs.GetObserver() != rhs;
	}
	template<StringType kTypeT>
	bool operator!=(const StringObserver<kTypeT> &lhs, const String<kTypeT> &rhs) noexcept {
		return lhs != rhs.GetObserver();
	}

	template<StringType kTypeT>
	bool operator<(const String<kTypeT> &lhs, const String<kTypeT> &rhs) noexcept {
		return lhs.GetObserver() < rhs.GetObserver();
	}
	template<StringType kTypeT>
	bool operator<(const String<kTypeT> &lhs, const StringObserver<kTypeT> &rhs) noexcept {
		return lhs.GetObserver() < rhs;
	}
	template<StringType kTypeT>
	bool operator<(const StringObserver<kTypeT> &lhs, const String<kTypeT> &rhs) noexcept {
		return lhs < rhs.GetObserver();
	}

	template<StringType kTypeT>
	bool operator>(const String<kTypeT> &lhs, const String<kTypeT> &rhs) noexcept {
		return lhs.GetObserver() > rhs.GetObserver();
	}
	template<StringType   kTypeT>
	bool operator>(const String<kTypeT> &lhs, const StringObserver<kTypeT> &rhs) noexcept {
		return lhs.GetObserver() > rhs;
	}
	template<StringType kTypeT>
	bool operator>(const StringObserver<kTypeT> &lhs, const String<kTypeT> &rhs) noexcept {
		return lhs > rhs.GetObserver();
	}

	template<StringType kTypeT>
	bool operator<=(const String<kTypeT> &lhs, const String<kTypeT> &rhs) noexcept {
		return lhs.GetObserver() <= rhs.GetObserver();
	}
	template<StringType kTypeT>
	bool operator<=(const String<kTypeT> &lhs, const StringObserver<kTypeT> &rhs) noexcept {
		return lhs.GetObserver() <= rhs;
	}
	template<StringType kTypeT>
	bool operator<=(const StringObserver<kTypeT> &lhs, const String<kTypeT> &rhs) noexcept {
		return lhs <= rhs.GetObserver();
	}

	template<StringType kTypeT>
	bool operator>=(const String<kTypeT> &lhs, const String<kTypeT> &rhs) noexcept {
		return lhs.GetObserver() >= rhs.GetObserver();
	}
	template<StringType kTypeT>
	bool operator>=(const String<kTypeT> &lhs, const StringObserver<kTypeT> &rhs) noexcept {
		return lhs.GetObserver() >= rhs;
	}
	template<StringType kTypeT>
	bool operator>=(const StringObserver<kTypeT> &lhs, const String<kTypeT> &rhs) noexcept {
		return lhs >= rhs.GetObserver();
	}

	template<StringType kTypeT>
	void swap(String<kTypeT> &lhs, String<kTypeT> &rhs) noexcept {
		lhs.Swap(rhs);
	}

	template<StringType kTypeT>
	auto begin(const String<kTypeT> &lhs) noexcept {
		return lhs.GetBegin();
	}
	template<StringType kTypeT>
	auto begin(String<kTypeT> &lhs) noexcept {
		return lhs.GetBegin();
	}
	template<StringType kTypeT>
	auto cbegin(const String<kTypeT> &lhs) noexcept {
		return lhs.GetBegin();
	}
	template<StringType kTypeT>
	auto end(const String<kTypeT> &lhs) noexcept {
		return lhs.GetEnd();
	}
	template<StringType kTypeT>
	auto end(String<kTypeT> &lhs) noexcept {
		return lhs.GetEnd();
	}
	template<StringType kTypeT>
	auto cend(const String<kTypeT> &lhs) noexcept {
		return lhs.GetEnd();
	}





