#ifndef __BTOOL_TYPES_H
#define __BTOOL_TYPES_H

typedef unsigned int		uint;
typedef unsigned short		ushort;

typedef unsigned char		u8;
typedef unsigned short		u16;
typedef unsigned long		u32;
typedef unsigned long long	u64;
typedef unsigned char		u128[16];

typedef signed char		i8;
typedef signed short		i16;
typedef signed long		i32;
typedef signed long long	i64;
typedef signed char		i128[16];

#define MIN(a,b)	( a < b ? a : b )

#define __PACKED __attribute__((packed))

#define bit(idx)			( 1 << idx )
#define bit_set(mask, width, idx)	( mask |= bit(idx & width) )
#define bit_clear(mask, width, idx)	( mask &= ~bit(idx & width) )
#define bit_test(mask, width, idx)	(bool)( mask & bit(idx & width) )

#endif//__BTOOL_TYPES_H
