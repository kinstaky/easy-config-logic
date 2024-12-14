/*
 * 这个文件是用来避免编译出来的东西依赖于 GLIBCXX_3.4.22。
 *
 * 参考
 * https://bugzilla.mozilla.org/show_bug.cgi?id=1389422
 * https://hg.mozilla.org/mozreview/gecko/rev/c407d0863f71b0da8479f63b3b0fc8462522912f
 * https://searchfox.org/mozilla-central/source/build/unix/stdc++compat/stdc++compat.cpp
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sstream>
/* Expose the definitions for the old ABI, allowing us to call its functions */
#define _GLIBCXX_THREAD_ABI_COMPAT 1
#include <thread>

namespace std {
// /* The old ABI has a thread::_M_start_thread(shared_ptr<_Impl_base>),
//  * while the new has thread::_M_start_thread(unique_ptr<_State>, void(*)()).
//  * There is an intermediate ABI at version 3.4.21, with
//  * thread::_M_start_thread(shared_ptr<_Impl_base>, void(*)()).
//  * The void(*)() parameter is only there to keep a reference to pthread_create
//  * on the caller side, and is unused in the implementation
//  * We're creating an entry point for the new and intermediate ABIs, and make
//  * them call the old ABI. This avoids the GLIBCXX_3.4.21 symbol version. */
// __attribute__((weak)) void thread::_M_start_thread(shared_ptr<_Impl_base> impl,
//                                                    void (*)()) {
//   _M_start_thread(std::move(impl));
// }

/* We need a _Impl_base-derived class wrapping a _State to call the old ABI
 * from what we got by diverting the new API. This avoids the GLIBCXX_3.4.22
 * symbol version. */
struct StateWrapper : public thread::_Impl_base {
  unique_ptr<thread::_State> mState;

  StateWrapper(unique_ptr<thread::_State> aState) : mState(std::move(aState)) {}

  void _M_run() override { mState->_M_run(); }
};

/* This avoids the GLIBCXX_3.4.22 symbol version. */
__attribute__((weak)) void thread::_M_start_thread(unique_ptr<_State> aState,
                                                   void (*)()) {
  auto impl = std::make_shared<StateWrapper>(std::move(aState));
  _M_start_thread(std::move(impl));
}

/* For some reason this is a symbol exported by new versions of libstdc++,
 * even though the destructor is default there too. This avoids the
 * GLIBCXX_3.4.22 symbol version. */
__attribute__((weak)) thread::_State::~_State() = default;

#if _GLIBCXX_RELEASE >= 9
// Ideally we'd define
//    bool _Sp_make_shared_tag::_S_eq(const type_info& ti) noexcept
// but we wouldn't be able to change its visibility because of the existing
// definition in C++ headers. We do need to change its visibility because we
// don't want it to be shadowing the one provided by libstdc++ itself, because
// it doesn't support RTTI. Not supporting RTTI doesn't matter for Firefox
// itself because it's built with RTTI disabled.
// So we define via the mangled symbol.
// This avoids the GLIBCXX_3.4.26 symbol version.
extern "C" __attribute__((visibility("hidden"))) bool
_ZNSt19_Sp_make_shared_tag5_S_eqERKSt9type_info(const type_info*) noexcept {
  return false;
}
#endif

}  // namespace std

namespace std {
/* Instantiate this template to avoid GLIBCXX_3.4.23 symbol versions
 * depending on optimization level */
template basic_string<char, char_traits<char>, allocator<char>>::basic_string(
    const basic_string&, size_t, const allocator<char>&);

#if _GLIBCXX_RELEASE >= 9
// This avoids the GLIBCXX_3.4.26 symbol version.
template basic_stringstream<char, char_traits<char>,
                            allocator<char>>::basic_stringstream();

template basic_ostringstream<char, char_traits<char>,
                             allocator<char>>::basic_ostringstream();
#endif

#if _GLIBCXX_RELEASE >= 11
// This avoids the GLIBCXX_3.4.29 symbol version.
template void basic_string<char, char_traits<char>, allocator<char>>::reserve();

template void
basic_string<wchar_t, char_traits<wchar_t>, allocator<wchar_t>>::reserve();
#endif

}  // namespace std