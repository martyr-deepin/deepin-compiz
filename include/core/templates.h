#ifndef _COMPIZ_TEMPLATES_H
#define _COMPIZ_TEMPLATES_H

#ifdef __GXX_EXPERIMENTAL_CXX0X__
// Compiz core is built for C++98. If you're not then get your own templates...
#define COMPIZ_EXTERN_STD(_c)
#else
#define COMPIZ_EXTERN_STD(_c)  extern template class std::_c;
#endif

// Non-std templates are unaffected by C++0x
#define COMPIZ_EXTERN(_c)  extern template class _c;

#endif
