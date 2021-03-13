#ifndef KV_SUPPORT_INTRINSICS_H
#define KV_SUPPORT_INTRINSICS_H

#define LIKELY(expr)    __builtin_expect(!!(expr), 1)
#define UNLIKELY(expr)  __builtin_expect(!!(expr), 0)

#define UNREACHABLE()   __builtin_unreachable()

#endif // KV_SUPPORT_INTRINSICS_H
