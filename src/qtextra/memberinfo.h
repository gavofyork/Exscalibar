#pragma once
#include <cstring>

#include <typeinfo>
#include <limits>
#include <cxxabi.h>

struct MemberDefinition
{
	const char* name;
	const char* type;
	unsigned long offset;
	unsigned long size;
	template<typename T> static inline bool calculateIsPod(T&) { return std::numeric_limits<T>::is_specialized; }
	bool isPod;
};

template<class T> inline char const* typeOfHelper(T&)
{
	char buf[1024];
	size_t size = 1024;
	int status;
	abi::__cxa_demangle(typeid(T).name(), buf, &size, &status);
	char* ret = new char[size + 1];
	memcpy(ret, buf, size);
	// TODO: FIX LEAK.
	return ret;
}

#define offsetOf(C, M) ((unsigned long)&(((C*)1)->M) - 1)
#define typeOf(C, M) typeOfHelper(((C*)1)->M)

#define __DO_MEMBER(PT, M) \
protected: \
	struct __MEMBER_INFO_ ## M \
	{ \
		typedef PT Next; \
		typedef __THIS_CLASS Class; \
		static const int memberCount = 1 + Next::memberCount; \
									   static inline void fillMemberInfo(MemberDefinition* _m) { Next::fillMemberInfo(--_m); _m->name = #M; _m->type = typeOf(Class, M); _m->offset = offsetOf(Class, M); _m->size = sizeof(Class::M); _m->isPod = MemberDefinition::calculateIsPod(((Class*)1)->M); } \
		static inline bool doCompare(Class const* _a, Class const* _b) { return _a->M == _b->M; } \
		static inline void doCopy(Class* _d, Class const* _s) { _d->M = _s->M; } \
		static inline bool compare(Class const* _a, Class const* _b) { return Next::compare(_a, _b) && doCompare(_a, _b); } \
		static inline void copy(Class* _d, Class const* _s) { Next::copy(_d, _s); doCopy(_d, _s); } \
	}
#define __DO_FINAL_MEMBER(PT) \
protected: \
	typedef PT __FIRST_MEMBER; \
public: \
	inline static const MemberDefinition* memberInfo() { static MemberDefinition s_mi[memberCount]; static bool ft = true; if (ft) __FIRST_MEMBER::fillMemberInfo(s_mi+memberCount); ft = false; return s_mi; } \
	inline bool operator==(__THIS_CLASS const& _cmp) const { return &_cmp && __FIRST_MEMBER::compare(this, &_cmp);; } \
	inline bool operator!=(__THIS_CLASS const& _cmp) const { return !operator==(_cmp); } \
	static const int memberCount = __FIRST_MEMBER::memberCount

#define TT_BASE_CLASS(NAME) protected: typedef NAME __THIS_CLASS; typedef NAME __BASE_CLASS; typedef FirstMemberDeclaration<__THIS_CLASS> __INITIAL_MEMBER;
#define TT_INHERITED_CLASS(NAME, SUPER) protected: typedef NAME __THIS_CLASS; typedef SUPER __SUPER_CLASS; typedef SUPER::__BASE_CLASS __BASE_CLASS; typedef SUPER::__FIRST_MEMBER __INITIAL_MEMBER;

#define TT_FIRST_MEMBER(M) __DO_MEMBER(__INITIAL_MEMBER, M)
#define TT_NEXT_MEMBER(P, M) __DO_MEMBER(__MEMBER_INFO_ ## P, M)
#define TT_FINISHED_MEMBERS(P) __DO_FINAL_MEMBER(__MEMBER_INFO_ ## P)
#define TT_NO_MEMBERS __DO_FINAL_MEMBER(__INITIAL_MEMBER)

#define TT_1_MEMBER(M1) TT_FIRST_MEMBER(M1); TT_FINISHED_MEMBERS(M1)
#define TT_2_MEMBERS(M1, M2) TT_FIRST_MEMBER(M1); TT_NEXT_MEMBER(M1, M2); TT_FINISHED_MEMBERS(M2)
#define TT_3_MEMBERS(M1, M2, M3) TT_FIRST_MEMBER(M1); TT_NEXT_MEMBER(M1, M2); TT_NEXT_MEMBER(M2, M3); TT_FINISHED_MEMBERS(M3)
#define TT_4_MEMBERS(M1, M2, M3, M4) TT_FIRST_MEMBER(M1); TT_NEXT_MEMBER(M1, M2); TT_NEXT_MEMBER(M2, M3); TT_NEXT_MEMBER(M3, M4); TT_FINISHED_MEMBERS(M4)
#define TT_5_MEMBERS(M1, M2, M3, M4, M5) TT_FIRST_MEMBER(M1); TT_NEXT_MEMBER(M1, M2); TT_NEXT_MEMBER(M2, M3); TT_NEXT_MEMBER(M3, M4); TT_NEXT_MEMBER(M4, M5); TT_FINISHED_MEMBERS(M5)
#define TT_6_MEMBERS(M1, M2, M3, M4, M5, M6) TT_FIRST_MEMBER(M1); TT_NEXT_MEMBER(M1, M2); TT_NEXT_MEMBER(M2, M3); TT_NEXT_MEMBER(M3, M4); TT_NEXT_MEMBER(M4, M5); TT_NEXT_MEMBER(M5, M6); TT_FINISHED_MEMBERS(M6)
#define TT_7_MEMBERS(M1, M2, M3, M4, M5, M6, M7) TT_FIRST_MEMBER(M1); TT_NEXT_MEMBER(M1, M2); TT_NEXT_MEMBER(M2, M3); TT_NEXT_MEMBER(M3, M4); TT_NEXT_MEMBER(M4, M5); TT_NEXT_MEMBER(M5, M6); TT_NEXT_MEMBER(M6, M7); TT_FINISHED_MEMBERS(M7)
#define TT_8_MEMBERS(M1, M2, M3, M4, M5, M6, M7, M8) TT_FIRST_MEMBER(M1); TT_NEXT_MEMBER(M1, M2); TT_NEXT_MEMBER(M2, M3); TT_NEXT_MEMBER(M3, M4); TT_NEXT_MEMBER(M4, M5); TT_NEXT_MEMBER(M5, M6); TT_NEXT_MEMBER(M6, M7); TT_NEXT_MEMBER(M7, M8); TT_FINISHED_MEMBERS(M8)

template<class T>
struct FirstMemberDeclaration
{
	typedef T Class;
	static const int memberCount = 0;
	static inline void fillMemberInfo(MemberDefinition*) {}
	static inline bool compare(T const*, T const*) { return true; }
	static inline void copy(T const*, T const*) {}
};
