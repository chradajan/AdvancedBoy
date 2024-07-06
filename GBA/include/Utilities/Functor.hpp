#pragma once

/// @brief Primary template of MemberFunctor.
template<typename>
class MemberFunctor;

/// @brief Functor to create a callback into a class member function.
/// @tparam ReturnT Return type of callback function.
/// @tparam ObjT Class type that contains callback function.
/// @tparam ...TArgs Args to pass into callback.
template<typename ReturnT, typename ObjT, typename... TArgs>
class MemberFunctor<ReturnT (ObjT::*)(TArgs...)>
{
    using PtrToMemberFunctionType = ReturnT (ObjT::*)(TArgs...);
    using ThisType = MemberFunctor<ReturnT (ObjT::*)(TArgs...)>;

public:
    MemberFunctor(PtrToMemberFunctionType memberFunctionPtr, ObjT& objectRef);
    MemberFunctor() = delete;

    ReturnT operator()(TArgs... params);

private:
    PtrToMemberFunctionType memberFunctionPtr_;
    ObjT* objectPtr_;
};

#include <GBA/include/Utilities/Functor.tpp>
