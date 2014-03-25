/****************************************************************************
 *	 FileName	 : tca_ckc.c
 *	 Description : 
 ****************************************************************************
*
 *	 TCC Version 1.0
 *	 Copyright (c) Telechips, Inc.
 *	 ALL RIGHTS RESERVED
*
 ****************************************************************************/


#if defined(__KERNEL__)
#   include <asm/io.h> 
#   include <mach/tca_ckc.h>
#   include <mach/TCC92x_Structures.h>
#   include <mach/TCC92x_Physical.h>
#   include <mach/globals.h>
#	include <linux/mm.h>	// for PAGE_ALIGN
#	include <linux/kernel.h>
#	include <linux/module.h>
#	ifndef VOLATILE
#	define VOLATILE
#	endif
#else
#   include "tca_ckc.h"
#endif
#include <linux/delay.h>
#include <asm/mach-types.h>

typedef struct {
	unsigned 			uFpll;
	unsigned char		P;
	unsigned short		M;
	unsigned char		S;
} sfPLL;

extern unsigned int g_org_tcc_clk; /* backup cpu clock value */


#define PLLFREQ(P, M, S)		(( 120000 * (M) / (P) ) >> (S)) // 100Hz Unit..
#define FPLL_t(P, M, S) 	PLLFREQ(P,M,S), P, M, S
// PLL table for XIN=12MHz
 // P,	 M,  S
sfPLL	pIO_CKC_PLL0[]	=
{
	 {FPLL_t(3 ,202 ,3)}	// 101 MHz
	,{FPLL_t(2 ,176 ,3)}	// 132 MHz
	,{FPLL_t(3 ,280 ,3)}	// 140 MHz
	,{FPLL_t(2 ,192 ,3)}	// 144 MHz
	,{FPLL_t(3 ,292 ,3)}	// 146 MHz
	,{FPLL_t(3 ,344 ,3)}	// 172 MHz
	,{FPLL_t(2 ,240 ,3)}	// 180 MHz
	,{FPLL_t(3 ,368 ,3)}	// 184 MHz
	,{FPLL_t(3 ,380 ,3)}	// 190 MHz
	,{FPLL_t(2 ,256 ,3)}	// 192 MHz
	,{FPLL_t(3 ,202 ,2)}	// 202 MHz
	,{FPLL_t(2 ,136 ,2)}	// 204 MHz
	,{FPLL_t(3 ,208 ,2)}	// 208 MHz
	,{FPLL_t(2 ,140 ,2)}	// 210 MHz
	,{FPLL_t(2 ,140 ,2)}	// 210 MHz
	,{FPLL_t(3 ,214 ,2)}	// 214 MHz
	,{FPLL_t(2 ,144 ,2)}	// 216 MHz
	,{FPLL_t(2 ,148 ,2)}	// 222 MHz
	,{FPLL_t(3 ,226 ,2)}	// 226 MHz
	,{FPLL_t(2 ,152 ,2)}	// 228 MHz
	,{FPLL_t(3 ,230 ,2)}	// 230 MHz
	,{FPLL_t(2 ,156 ,2)}	// 234 MHz
	,{FPLL_t(3 ,236 ,2)}	// 236 MHz
	,{FPLL_t(3 ,244 ,2)}	// 244 MHz
	,{FPLL_t(2 ,164 ,2)}	// 246 MHz
	,{FPLL_t(3 ,248 ,2)}	// 248 MHz
	,{FPLL_t(2 ,168 ,2)}	// 252 MHz
	,{FPLL_t(3 ,254 ,2)}	// 254 MHz
	,{FPLL_t(2 ,172 ,2)}	// 258 MHz
	,{FPLL_t(3 ,260 ,2)}	// 260 MHz
	,{FPLL_t(3 ,262 ,2)}	// 262 MHz
	,{FPLL_t(2 ,176 ,2)}	// 264 MHz
	,{FPLL_t(3 ,266 ,2)}	// 266 MHz
	,{FPLL_t(2 ,184 ,2)}	// 276 MHz
	,{FPLL_t(2 ,188 ,2)}	// 282 MHz
	,{FPLL_t(3 ,290 ,2)}	// 290 MHz
	,{FPLL_t(3 ,292 ,2)}	// 292 MHz
	,{FPLL_t(2 ,196 ,2)}	// 294 MHz
	,{FPLL_t(3 ,304 ,2)}	// 304 MHz
	,{FPLL_t(3 ,308 ,2)}	// 308 MHz
	,{FPLL_t(2 ,208 ,2)}	// 312 MHz
	,{FPLL_t(3 ,316 ,2)}	// 316 MHz
	,{FPLL_t(3 ,320 ,2)}	// 320 MHz
	,{FPLL_t(2 ,216 ,2)}	// 324 MHz
	,{FPLL_t(3 ,328 ,2)}	// 328 MHz
	,{FPLL_t(2 ,220 ,2)}	// 330 MHz
	,{FPLL_t(3 ,332 ,2)}	// 332 MHz
	,{FPLL_t(2 ,224 ,2)}	// 336 MHz
	,{FPLL_t(3 ,340 ,2)}	// 340 MHz
	,{FPLL_t(2 ,232 ,2)}	// 348 MHz
	,{FPLL_t(3 ,352 ,2)}	// 352 MHz
	,{FPLL_t(3 ,356 ,2)}	// 356 MHz
	,{FPLL_t(2 ,240 ,2)}	// 360 MHz
	,{FPLL_t(3 ,362 ,2)}	// 362 MHz
	,{FPLL_t(3 ,364 ,2)}	// 364 MHz
	,{FPLL_t(3 ,368 ,2)}	// 368 MHz
	,{FPLL_t(2 ,246 ,2)}	// 369 MHz
	,{FPLL_t(3 ,370 ,2)}	// 370 MHz
	,{FPLL_t(2 ,248 ,2)}	// 372 MHz
	,{FPLL_t(3 ,380 ,2)}	// 380 MHz
	,{FPLL_t(3 ,380 ,2)}	// 380 MHz
	,{FPLL_t(2 ,254 ,2)}	// 381 MHz
	,{FPLL_t(2 ,256 ,2)}	// 384 MHz
	,{FPLL_t(3 ,388 ,2)}	// 388 MHz
	,{FPLL_t(2 ,260 ,2)}	// 390 MHz
	,{FPLL_t(3 ,392 ,2)}	// 392 MHz
	,{FPLL_t(2 ,262 ,2)}	// 393 MHz
	,{FPLL_t(2 ,264 ,2)}	// 396 MHz
	,{FPLL_t(2 ,266 ,2)}	// 399 MHz
	,{FPLL_t(3 ,400 ,2)}	// 400 MHz
	,{FPLL_t(3 ,202 ,1)}	// 404 MHz
	,{FPLL_t(5 ,338 ,1)}	// 405.6 MHz
	,{FPLL_t(3 ,203 ,1)}	// 406 MHz
	,{FPLL_t(2 ,136 ,1)}	// 408 MHz
	,{FPLL_t(3 ,205 ,1)}	// 410 MHz
	,{FPLL_t(3 ,206 ,1)}	// 412 MHz
	,{FPLL_t(3 ,208 ,1)}	// 416 MHz
	,{FPLL_t(5 ,348 ,1)}	// 417.6 MHz
	,{FPLL_t(2 ,140 ,1)}	// 420 MHz
	,{FPLL_t(3 ,212 ,1)}	// 424 MHz
	,{FPLL_t(3 ,214 ,1)}	// 428 MHz
	,{FPLL_t(2 ,143 ,1)}	// 429 MHz
	,{FPLL_t(3 ,215 ,1)}	// 430 MHz
	,{FPLL_t(2 ,144 ,1)}	// 432 MHz
	,{FPLL_t(3 ,218 ,1)}	// 436 MHz
	,{FPLL_t(3 ,220 ,1)}	// 440 MHz
	,{FPLL_t(3 ,221 ,1)}	// 442 MHz
	,{FPLL_t(2 ,148 ,1)}	// 444 MHz
	,{FPLL_t(3 ,224 ,1)}	// 448 MHz
	,{FPLL_t(2 ,150 ,1)}	// 450 MHz
	,{FPLL_t(3 ,226 ,1)}	// 452 MHz
	,{FPLL_t(3 ,227 ,1)}	// 454 MHz
	,{FPLL_t(2 ,152 ,1)}	// 456 MHz
	,{FPLL_t(3 ,230 ,1)}	// 460 MHz
	,{FPLL_t(5 ,386 ,1)}	// 463.2 MHz
	,{FPLL_t(3 ,232 ,1)}	// 464 MHz
	,{FPLL_t(3 ,233 ,1)}	// 466 MHz
	,{FPLL_t(2 ,156 ,1)}	// 468 MHz
	,{FPLL_t(3 ,235 ,1)}	// 470 MHz
	,{FPLL_t(3 ,236 ,1)}	// 472 MHz
	,{FPLL_t(2 ,158 ,1)}	// 474 MHz
	,{FPLL_t(3 ,238 ,1)}	// 476 MHz
	,{FPLL_t(2 ,160 ,1)}	// 480 MHz
	,{FPLL_t(3 ,242 ,1)}	// 484 MHz
	,{FPLL_t(2 ,162 ,1)}	// 486 MHz
	,{FPLL_t(3 ,244 ,1)}	// 488 MHz
	,{FPLL_t(3 ,245 ,1)}	// 490 MHz
	,{FPLL_t(2 ,164 ,1)}	// 492 MHz
	,{FPLL_t(3 ,248 ,1)}	// 496 MHz
	,{FPLL_t(5 ,414 ,1)}	// 496.8 MHz
	,{FPLL_t(2 ,166 ,1)}	// 498 MHz
	,{FPLL_t(3 ,250 ,1)}	// 500 MHz
	,{FPLL_t(2 ,168 ,1)}	// 504 MHz
	,{FPLL_t(3 ,254 ,1)}	// 508 MHz
	,{FPLL_t(3 ,256 ,1)}	// 512 MHz
	,{FPLL_t(2 ,172 ,1)}	// 516 MHz
	,{FPLL_t(3 ,260 ,1)}	// 520 MHz
	,{FPLL_t(3 ,262 ,1)}	// 524 MHz
	,{FPLL_t(2 ,176 ,1)}	// 528 MHz
	,{FPLL_t(3 ,266 ,1)}	// 532 MHz
	,{FPLL_t(3 ,268 ,1)}	// 536 MHz
	,{FPLL_t(2 ,180 ,1)}	// 540 MHz
	,{FPLL_t(3 ,272 ,1)}	// 544 MHz
	,{FPLL_t(3 ,274 ,1)}	// 548 MHz
	,{FPLL_t(2 ,184 ,1)}	// 552 MHz
	,{FPLL_t(3 ,278 ,1)}	// 556 MHz
	,{FPLL_t(3 ,280 ,1)}	// 560 MHz
	,{FPLL_t(2 ,188 ,1)}	// 564 MHz
	,{FPLL_t(3 ,284 ,1)}	// 568 MHz
	,{FPLL_t(3 ,286 ,1)}	// 572 MHz
	,{FPLL_t(2 ,192 ,1)}	// 576 MHz
	,{FPLL_t(3 ,290 ,1)}	// 580 MHz
	,{FPLL_t(3 ,292 ,1)}	// 584 MHz
	,{FPLL_t(2 ,196 ,1)}	// 588 MHz
	,{FPLL_t(3 ,296 ,1)}	// 592 MHz
	,{FPLL_t(3 ,298 ,1)}	// 596 MHz
	,{FPLL_t(2 ,200 ,1)}	// 600 MHz
	,{FPLL_t(3 ,304 ,1)}	// 608 MHz
	,{FPLL_t(3 ,308 ,1)}	// 616 MHz
	,{FPLL_t(2 ,208 ,1)}	// 624 MHz
	,{FPLL_t(3 ,316 ,1)}	// 632 MHz
	,{FPLL_t(3 ,320 ,1)}	// 640 MHz
	,{FPLL_t(2 ,216 ,1)}	// 648 MHz
	,{FPLL_t(3 ,328 ,1)}	// 656 MHz
	,{FPLL_t(2 ,220 ,1)}	// 660 MHz
	,{FPLL_t(3 ,332 ,1)}	// 664 MHz
	,{FPLL_t(2 ,224 ,1)}	// 672 MHz
	,{FPLL_t(3 ,340 ,1)}	// 680 MHz
	,{FPLL_t(2 ,232 ,1)}	// 696 MHz
	,{FPLL_t(3 ,280 ,4)}	// 70 MHz
	,{FPLL_t(3 ,352 ,1)}	// 704 MHz
	,{FPLL_t(3 ,356 ,1)}	// 712 MHz
	,{FPLL_t(2 ,192 ,4)}	// 72 MHz
	,{FPLL_t(2 ,240 ,1)}	// 720 MHz
	,{FPLL_t(3 ,362 ,1)}	// 724 MHz
	,{FPLL_t(3 ,364 ,1)}	// 728 MHz
	,{FPLL_t(2 ,246 ,1)}	// 738 MHz
	,{FPLL_t(3 ,370 ,1)}	// 740 MHz
	,{FPLL_t(2 ,248 ,1)}	// 744 MHz
	,{FPLL_t(3 ,380 ,1)}	// 760 MHz
	,{FPLL_t(2 ,254 ,1)}	// 762 MHz
	,{FPLL_t(2 ,256 ,1)}	// 768 MHz
	,{FPLL_t(3 ,388 ,1)}	// 776 MHz
	,{FPLL_t(2 ,260 ,1)}	// 780 MHz
	,{FPLL_t(3 ,392 ,1)}	// 784 MHz
	,{FPLL_t(2 ,262 ,1)}	// 786 MHz
	,{FPLL_t(2 ,264 ,1)}	// 792 MHz
	,{FPLL_t(2 ,266 ,1)}	// 798 MHz
	,{FPLL_t(3 ,400 ,1)}	// 800 MHz
	,{FPLL_t(3 ,202 ,0)}	// 808 MHz
	,{FPLL_t(5 ,338 ,0)}	// 811.2 MHz
	,{FPLL_t(3 ,203 ,0)}	// 812 MHz
	,{FPLL_t(2 ,136 ,0)}	// 816 MHz
	,{FPLL_t(3 ,205 ,0)}	// 820 MHz
	,{FPLL_t(3 ,206 ,0)}	// 824 MHz
	,{FPLL_t(3 ,208 ,0)}	// 832 MHz
	,{FPLL_t(5 ,348 ,0)}	// 835.2 MHz
	,{FPLL_t(2 ,140 ,0)}	// 840 MHz
	,{FPLL_t(3 ,212 ,0)}	// 848 MHz
	,{FPLL_t(3 ,214 ,0)}	// 856 MHz
	,{FPLL_t(2 ,143 ,0)}	// 858 MHz
	,{FPLL_t(3 ,344 ,4)}	// 86 MHz
	,{FPLL_t(3 ,215 ,0)}	// 860 MHz
	,{FPLL_t(2 ,144 ,0)}	// 864 MHz
	,{FPLL_t(3 ,218 ,0)}	// 872 MHz
	,{FPLL_t(3 ,220 ,0)}	// 880 MHz
	,{FPLL_t(3 ,221 ,0)}	// 884 MHz
	,{FPLL_t(2 ,148 ,0)}	// 888 MHz
	,{FPLL_t(3 ,224 ,0)}	// 896 MHz
	,{FPLL_t(2 ,150 ,0)}	// 900 MHz
	,{FPLL_t(3 ,226 ,0)}	// 904 MHz
	,{FPLL_t(3 ,227 ,0)}	// 908 MHz
	,{FPLL_t(2 ,152 ,0)}	// 912 MHz
	,{FPLL_t(3 ,230 ,0)}	// 920 MHz
	,{FPLL_t(5 ,386 ,0)}	// 926.4 MHz
	,{FPLL_t(3 ,232 ,0)}	// 928 MHz
	,{FPLL_t(3 ,233 ,0)}	// 932 MHz
	,{FPLL_t(2 ,156 ,0)}	// 936 MHz
	,{FPLL_t(3 ,235 ,0)}	// 940 MHz
	,{FPLL_t(3 ,236 ,0)}	// 944 MHz
	,{FPLL_t(2 ,158 ,0)}	// 948 MHz
	,{FPLL_t(3 ,238 ,0)}	// 952 MHz
	,{FPLL_t(2 ,160 ,0)}	// 960 MHz
	,{FPLL_t(3 ,242 ,0)}	// 968 MHz
	,{FPLL_t(2 ,162 ,0)}	// 972 MHz
	,{FPLL_t(3 ,244 ,0)}	// 976 MHz
	,{FPLL_t(3 ,245 ,0)}	// 980 MHz
	,{FPLL_t(2 ,164 ,0)}	// 984 MHz
	,{FPLL_t(3 ,248 ,0)}	// 992 MHz
	,{FPLL_t(5 ,414 ,0)}	// 993.6 MHz
	,{FPLL_t(2 ,166 ,0)}	// 996 MHz
	,{FPLL_t(3 ,250 ,0)}	// 1000 MHz
	,{FPLL_t(2 ,168 ,0)}	// 1008 MHz
	,{FPLL_t(3 ,254 ,0)}	// 1016 MHz
	,{FPLL_t(3 ,256 ,0)}	// 1024 MHz
	,{FPLL_t(2 ,172 ,0)}	// 1032 MHz
	,{FPLL_t(3 ,260 ,0)}	// 1040 MHz
	,{FPLL_t(3 ,262 ,0)}	// 1048 MHz
	,{FPLL_t(2 ,176 ,0)}	// 1056 MHz
	,{FPLL_t(3 ,266 ,0)}	// 1064 MHz
	,{FPLL_t(3 ,268 ,0)}	// 1072 MHz
	,{FPLL_t(2 ,180 ,0)}	// 1080 MHz
	,{FPLL_t(3 ,272 ,0)}	// 1088 MHz
	,{FPLL_t(3 ,274 ,0)}	// 1096 MHz
	,{FPLL_t(2 ,184 ,0)}	// 1104 MHz
	,{FPLL_t(3 ,278 ,0)}	// 1112 MHz
	,{FPLL_t(3 ,280 ,0)}	// 1120 MHz
	,{FPLL_t(3 ,280 ,0)}	// 1120 MHz
	,{FPLL_t(2 ,188 ,0)}	// 1128 MHz
	,{FPLL_t(3 ,284 ,0)}	// 1136 MHz
	,{FPLL_t(3 ,286 ,0)}	// 1144 MHz
	,{FPLL_t(2 ,192 ,0)}	// 1152 MHz
	,{FPLL_t(3 ,290 ,0)}	// 1160 MHz
	,{FPLL_t(3 ,292 ,0)}	// 1168 MHz
	,{FPLL_t(2 ,196 ,0)}	// 1176 MHz
	,{FPLL_t(3 ,296 ,0)}	// 1184 MHz
	,{FPLL_t(3, 298, 0)}	// 1192 MHz
	,{FPLL_t(2, 200, 0)}	// 1200 MHz
};

#define NUM_PLL0 				(sizeof(pIO_CKC_PLL0)/sizeof(sfPLL))

sfPLL	pIO_CKC_PLL1[]	=
{
	 {FPLL_t(2,  85, 5)}	// 15.9375 MHz
	,{FPLL_t(1 , 33 ,4)}	// 24.75 MHz
	,{FPLL_t(3 ,140 ,3)}	// 70 MHz
	,{FPLL_t(1 , 48 ,3)}	// 72 MHz
	,{FPLL_t(3 , 86 ,2)}	// 86 MHz
	,{FPLL_t(3 ,101 ,2)}	// 101 MHz
	,{FPLL_t(1 , 44 ,2)}	// 132 MHz
	,{FPLL_t(3 ,146 ,2)}	// 146 MHz
	,{FPLL_t(1 , 30 ,1)}	// 180 MHz
	,{FPLL_t(3 , 92 ,1)}	// 184 MHz
	,{FPLL_t(3 , 95 ,1)}	// 190 MHz
	,{FPLL_t(1 , 32 ,1)}	// 192 MHz
	,{FPLL_t(1 , 34 ,1)}	// 204 MHz
	,{FPLL_t(3 ,104 ,1)}	// 208 MHz
	,{FPLL_t(1 , 35 ,1)}	// 210 MHz
	,{FPLL_t(3 ,107 ,1)}	// 214 MHz
	,{FPLL_t(1 , 36 ,1)}	// 216 MHz
	,{FPLL_t(1 , 37 ,1)}	// 222 MHz
	,{FPLL_t(3 ,113 ,1)}	// 226 MHz
	,{FPLL_t(1 , 38 ,1)}	// 228 MHz
	,{FPLL_t(3 ,115 ,1)}	// 230 MHz
	,{FPLL_t(1 , 39 ,1)}	// 234 MHz
	,{FPLL_t(3 ,118 ,1)}	// 236 MHz
	,{FPLL_t(3 ,122 ,1)}	// 244 MHz
	,{FPLL_t(1 , 41 ,1)}	// 246 MHz
	,{FPLL_t(3 ,124 ,1)}	// 248 MHz
	,{FPLL_t(1 , 42 ,1)}	// 252 MHz
	,{FPLL_t(3 ,127 ,1)}	// 254 MHz
	,{FPLL_t(1 , 43 ,1)}	// 258 MHz
	,{FPLL_t(3 ,130 ,1)}	// 260 MHz
	,{FPLL_t(3 ,131 ,1)}	// 262 MHz
	,{FPLL_t(1 , 44 ,1)}	// 264 MHz
	,{FPLL_t(3 ,133 ,1)}	// 266 MHz
	,{FPLL_t(1 , 46 ,1)}	// 276 MHz
	,{FPLL_t(1 , 47 ,1)}	// 282 MHz
	,{FPLL_t(3 ,145 ,1)}	// 290 MHz
	,{FPLL_t(1 , 49 ,1)}	// 294 MHz
	,{FPLL_t(3 , 76 ,0)}	// 304 MHz
	,{FPLL_t(1 , 26 ,0)}	// 312 MHz
	,{FPLL_t(3 , 79 ,0)}	// 316 MHz
	,{FPLL_t(3 , 80 ,0)}	// 320 MHz
	,{FPLL_t(1 , 27 ,0)}	// 324 MHz
	,{FPLL_t(3 , 82 ,0)}	// 328 MHz
	,{FPLL_t(2 , 55 ,0)}	// 330 MHz
	,{FPLL_t(3 , 83 ,0)}	// 332 MHz
	,{FPLL_t(1 , 28 ,0)}	// 336 MHz
	,{FPLL_t(3 , 85 ,0)}	// 340 MHz
	,{FPLL_t(1 , 29 ,0)}	// 348 MHz
	,{FPLL_t(3 , 88 ,0)}	// 352 MHz
	,{FPLL_t(3 , 89 ,0)}	// 356 MHz
	,{FPLL_t(1 , 30 ,0)}	// 360 MHz
	,{FPLL_t(6 ,181 ,0)}	// 362 MHz
	,{FPLL_t(3 , 91 ,0)}	// 364 MHz
	,{FPLL_t(4 ,123 ,0)}	// 369 MHz
	,{FPLL_t(6 ,185 ,0)}	// 370 MHz
	,{FPLL_t(1 , 31 ,0)}	// 372 MHz
	,{FPLL_t(3 , 95 ,0)}	// 380 MHz
	,{FPLL_t(4 ,127 ,0)}	// 381 MHz
	,{FPLL_t(1 , 32 ,0)}	// 384 MHz
	,{FPLL_t(3 , 97 ,0)}	// 388 MHz
	,{FPLL_t(2 , 65 ,0)}	// 390 MHz
	,{FPLL_t(3 , 98 ,0)}	// 392 MHz
	,{FPLL_t(4 ,131 ,0)}	// 393 MHz
	,{FPLL_t(1 , 33 ,0)}	// 396 MHz
	,{FPLL_t(4 ,133 ,0)}	// 399 MHz
	,{FPLL_t(3 ,100 ,0)}	// 400 MHz
	,{FPLL_t(3 ,101 ,0)}	// 404 MHz
	,{FPLL_t(5 ,169 ,0)}	// 405.6 MHz
	,{FPLL_t(6 ,203 ,0)}	// 406 MHz
	,{FPLL_t(1 , 34 ,0)}	// 408 MHz
	,{FPLL_t(6 ,205 ,0)}	// 410 MHz
	,{FPLL_t(3 ,103 ,0)}	// 412 MHz
	,{FPLL_t(3 ,104 ,0)}	// 416 MHz
	,{FPLL_t(5 ,174 ,0)}	// 417.6 MHz
	,{FPLL_t(1 , 35 ,0)}	// 420 MHz
	,{FPLL_t(3 ,106 ,0)}	// 424 MHz
	,{FPLL_t(3 ,107 ,0)}	// 428 MHz
	,{FPLL_t(4 ,143 ,0)}	// 429 MHz
	,{FPLL_t(6 ,215 ,0)}	// 430 MHz
	,{FPLL_t(1 , 36 ,0)}	// 432 MHz
	,{FPLL_t(3 ,109 ,0)}	// 436 MHz
	,{FPLL_t(3 ,110 ,0)}	// 440 MHz
	,{FPLL_t(6 ,221 ,0)}	// 442 MHz
	,{FPLL_t(1 , 37 ,0)}	// 444 MHz
	,{FPLL_t(3 ,112 ,0)}	// 448 MHz
	,{FPLL_t(2 , 75 ,0)}	// 450 MHz
	,{FPLL_t(3 ,113 ,0)}	// 452 MHz
	,{FPLL_t(6 ,227 ,0)}	// 454 MHz
	,{FPLL_t(1 , 38 ,0)}	// 456 MHz
	,{FPLL_t(3 ,115 ,0)}	// 460 MHz
	,{FPLL_t(3 ,116 ,0)}	// 464 MHz
	,{FPLL_t(6 ,233 ,0)}	// 466 MHz
	,{FPLL_t(1 , 39 ,0)}	// 468 MHz
	,{FPLL_t(6 ,235 ,0)}	// 470 MHz
	,{FPLL_t(3 ,118 ,0)}	// 472 MHz
	,{FPLL_t(2 , 79 ,0)}	// 474 MHz
	,{FPLL_t(3 ,119 ,0)}	// 476 MHz
	,{FPLL_t(1 , 40 ,0)}	// 480 MHz
	,{FPLL_t(3 ,121 ,0)}	// 484 MHz
	,{FPLL_t(2 , 81 ,0)}	// 486 MHz
	,{FPLL_t(3 ,122 ,0)}	// 488 MHz
	,{FPLL_t(6 ,245 ,0)}	// 490 MHz
	,{FPLL_t(1 , 41 ,0)}	// 492 MHz
	,{FPLL_t(3 ,124 ,0)}	// 496 MHz
	,{FPLL_t(5 ,207 ,0)}	// 496.8 MHz
	,{FPLL_t(2 , 83 ,0)}	// 498 MHz
	,{FPLL_t(3 ,125 ,0)}	// 500 MHz
	,{FPLL_t(1 , 42 ,0)}	// 504 MHz
	,{FPLL_t(3 ,127 ,0)}	// 508 MHz
	,{FPLL_t(3 ,128 ,0)}	// 512 MHz
	,{FPLL_t(1 , 43 ,0)}	// 516 MHz
	,{FPLL_t(3 ,130 ,0)}	// 520 MHz
	,{FPLL_t(3 ,131 ,0)}	// 524 MHz
	,{FPLL_t(1 , 44 ,0)}	// 528 MHz
	,{FPLL_t(3 ,133 ,0)}	// 532 MHz
	,{FPLL_t(3 ,134 ,0)}	// 536 MHz
	,{FPLL_t(1 , 45 ,0)}	// 540 MHz
	,{FPLL_t(3 ,136 ,0)}	// 544 MHz
	,{FPLL_t(3 ,137 ,0)}	// 548 MHz
	,{FPLL_t(1 , 46 ,0)}	// 552 MHz
	,{FPLL_t(3 ,139 ,0)}	// 556 MHz
	,{FPLL_t(3 ,140 ,0)}	// 560 MHz
	,{FPLL_t(1 , 47 ,0)}	// 564 MHz
	,{FPLL_t(3 ,142 ,0)}	// 568 MHz
	,{FPLL_t(3 ,143 ,0)}	// 572 MHz
	,{FPLL_t(1 , 48 ,0)}	// 576 MHz
	,{FPLL_t(3 ,145 ,0)}	// 580 MHz
	,{FPLL_t(3 ,146 ,0)}	// 584 MHz
	,{FPLL_t(1 , 49 ,0)}	// 588 MHz
	,{FPLL_t(3 ,148 ,0)}	// 592 MHz
	,{FPLL_t(3 ,149 ,0)}	// 596 MHz
	,{FPLL_t(1 , 50 ,0)}	// 600 MHz
	,{FPLL_t(2 ,110 ,0)}	// 660 MHz
	,{0, 0, 0, 0}		// Variable Frequency initialized with 0
};
#define NUM_PLL1 				(sizeof(pIO_CKC_PLL1)/sizeof(sfPLL))

#define tca_wait()				{ volatile int i; for (i=0; i<0x2000; i++); }

#if defined(_LINUX_)
    #define iomap_p2v(x)            (x)
#else
//	#define iomap_p2v(x)			(x & ~0x40000000) //0xF0400000 -> 0xB04 
	#define iomap_p2v(x)			(OALPAtoVA(x,FALSE)) //0xF0400000 -> 0xB04 
#endif

/****************************************************************************************
* Global Variable
* ***************************************************************************************/
PCKC	pCKC ;
PPMU	pPMU ; 
PIOBUSCFG pIOBUSCFG;

unsigned		uIO_CKC_Fpll[MAX_PLL_CH];		// Current PLL Frequency, 100Hz unit
unsigned		uIO_CKC_Fsys[HwCKC_SYS_MAX];	// Current System Clock Frequency, 100Hz unit
unsigned		uIO_CKC_Fclk[HwCKC_SYS_MAX];	// Current Generated Clock Frequency, 100Hz unit


/****************************************************************************************
* FUNCTION :void tca_ckc_init(void)
* DESCRIPTION :
* ***************************************************************************************/
VOLATILE void tca_ckc_init(void)
{
	pCKC = (PCKC)(iomap_p2v((unsigned int)&HwCLK_BASE)); //0xF0400000
	pPMU = (PPMU)(iomap_p2v((unsigned int)&HwPMU_BASE)); //0xF0404000
	pIOBUSCFG = (PIOBUSCFG)(iomap_p2v((unsigned int)&HwIOBUSCFG_BASE)); //0xF05F5000
	
}
/****************************************************************************************
* FUNCTION :unsigned int tca_ckc_getpll(unsigned int ch)
* DESCRIPTION :
* ***************************************************************************************/
VOLATILE unsigned int tca_ckc_getpll(unsigned int ch)
{
	volatile unsigned	tPLL;
	volatile unsigned	tPLLCFG;
	unsigned	iP=0, iM=0, iS=0;

	switch(ch)
	{
		case DIRECTPLL0:
			tPLLCFG = pCKC->PLL0CFG;
			break;
		case DIRECTPLL1:
			tPLLCFG = pCKC->PLL1CFG;
			break;
		case DIRECTPLL2:
			tPLLCFG = pCKC->PLL2CFG;
			break;
		case DIRECTPLL3:
			tPLLCFG = pCKC->PLL3CFG;
			break;
	}

	//Fpll Clock
	iS	= (tPLLCFG & 0x7000000) >> 24;
	iM	= (tPLLCFG & 0xFFF00) >> 8;
	iP	= (tPLLCFG & 0x0003F) >> 0;

	tPLL= (((120000 * iM )/ iP) >> (iS));
	
	return tPLL;

}

/****************************************************************************************
* FUNCTION :unsigned int tca_ckc_getcpu(void)
* DESCRIPTION : 
* ***************************************************************************************/
VOLATILE unsigned int tca_ckc_getcpu(void)
{
	unsigned int lcpu = 0;
	unsigned int lconfig = 0;
	unsigned int lcnt = 0;
	unsigned int li = 0;
	unsigned int lclksource = 0;
	
	lconfig = ((pCKC->CLK0CTRL & (Hw20-Hw4))>>4);

	for(li = 0; li < 16; li++)
	{
		if((lconfig & Hw0) == 1)
			lcnt++;
		lconfig = (lconfig >> 1);
	}

	switch(pCKC->CLK0CTRL & (Hw3-Hw0)) // Check CPU Source
	{
		case PCDIRECTPLL0 :
			lclksource =  tca_ckc_getpll(0);
			break;
		case PCDIRECTPLL1 :
			lclksource =  tca_ckc_getpll(1);
			break;
		case PCDIRECTPLL2 :
			lclksource =  tca_ckc_getpll(2);
			break;
		case PCDIRECTPLL3 :
			lclksource =  tca_ckc_getpll(3);
			break;
		case PCDIRECTXIN :
			lclksource =  120000;
			break;
		case PCHDMI :
			lclksource =  270000;
			break;
		case PCSATA :
			lclksource =  250000;		
			break;
		case PCUSBPHY:
			lclksource =  480000;		
			break;
		default : 
			lclksource =  tca_ckc_getpll(1);
			break;
	}
	
	if(pCKC->CLK0CTRL & Hw20) // Dynamic Mode
	{
		lcnt = pCKC->CLK0CTRL & (Hw8-Hw4);
		lcnt = lcnt>>4;

		if (lcnt < 1)
			lcnt = 1;

		lcpu = (lclksource / lcnt);
	}
	else
		lcpu = (lclksource * lcnt)/16;

	return lcpu;
}

/****************************************************************************************
* FUNCTION :unsigned int tca_ckc_getbus(void)
* DESCRIPTION : 
* ***************************************************************************************/
VOLATILE unsigned int tca_ckc_getbus(void)
{
	unsigned int lbus = 0;
	unsigned int lconfig = 0;
	unsigned int lclksource = 0;
	
	lconfig = ((pCKC->CLK2CTRL & (Hw8-Hw4))>>4);

	switch(pCKC->CLK2CTRL & (Hw3-Hw0)) // Check CPU Source
	{
		case PCDIRECTPLL0 :
			lclksource =  tca_ckc_getpll(0);
			break;
		case PCDIRECTPLL1 :
			lclksource =  tca_ckc_getpll(1);
			break;
		case PCDIRECTPLL2 :
			lclksource =  tca_ckc_getpll(2);
			break;
		case PCDIRECTPLL3 :
			lclksource =  tca_ckc_getpll(3);
			break;
		case PCDIRECTXIN :
			lclksource =  120000;
			break;
		case PCHDMI :
			lclksource =  270000;
			break;
		case PCSATA :
			lclksource =  250000;		
			break;
		case PCUSBPHY:
			lclksource =  480000;		
			break;
		default : 
			lclksource =  tca_ckc_getpll(1);
			break;
	}

	lbus = lclksource /(lconfig+1);

	return lbus;
}

/****************************************************************************************
* FUNCTION :static unsigned int tca_ckc_setclkctrlx(unsigned int isenable,unsigned int md,unsigned int config,unsigned int sel)
* DESCRIPTION : not ctrl 0 and ctrl 2 (CPU, BUS CTRL)
* ***************************************************************************************/
VOLATILE static unsigned int tca_ckc_gclkctrlx(unsigned int isenable,unsigned int md,unsigned int config,unsigned int sel)
{
	unsigned int retVal = 0;

	retVal = ((isenable?1:0)<<21)|(md<<20)|(config<<4)|(sel<<0);
	
	return retVal;
}

/****************************************************************************************
* FUNCTION :static unsigned int tca_ckc_clkctrly(unsigned int isenable,unsigned int md,unsigned int config,unsigned int sel)
* DESCRIPTION : ctrl 0 and ctrl 2 (CPU and BUS CTRL)
*				config is divider (md = 0)
* ***************************************************************************************/
VOLATILE static unsigned int tca_ckc_gclkctrly(unsigned int isenable,unsigned int md,unsigned int config,unsigned int sel, unsigned int ch)
{
	unsigned int retVal = 0;
//	md = 0; // Normal Mode

	if(ch == CLKCTRL0)
	{
		switch(config)
		{
			case CLKDIV0:
					config = 0xFFFF; // 1111111111111111b 16/16
				break;
			case CLKDIV2:
					config = 0xAAAA; // 1010101010101010b 8/16
				break;
			case CLKDIV3:
					config = 0x9249; // 1001001001001001b 6/16
				break;
			case CLKDIV4:
					config = 0x8888; // 1000100010001000b 4/16
				break;
			case CLKDIVNONCHANGE:
					config = 0xFFFF; // 1111111111111111b
				break;
			default:
					config = 0xFFFF; // 1111111111111111b
				break;
		}
	}

	if(config == CLKDIVNONCHANGE)
	{
		if(ch == 0) // Fcpu
			retVal = (pCKC->CLK0CTRL & (Hw20-Hw4));
		else		// Fmem_bus 
			retVal = (pCKC->CLK2CTRL & (Hw20-Hw4));
			
		retVal |= ((isenable?1:0)<<21)|(md<<20)|(sel<<0);

	}
	else
		retVal = ((isenable?1:0)<<21)|(md<<20)|(config<<4)|(sel<<0);
	
	return retVal;
}

/****************************************************************************************
* FUNCTION :void tca_ckc_setfbusctrl(unsigned int clkname,unsigned int isenable,unsigned int freq, unsigned int sor)
* DESCRIPTION :
* ***************************************************************************************/
VOLATILE void tca_ckc_setfbusctrl(unsigned int clkname,unsigned int isenable,unsigned int md,unsigned int freq, unsigned int sor)
{
	volatile unsigned	*pCLKCTRL;
	unsigned int clkdiv = 0;
	unsigned int clksource = 0;
	unsigned int lconfig = 0;
	
	pCLKCTRL =(volatile unsigned	*)((&pCKC->CLK0CTRL)+clkname); 

	switch(sor)
	{
		case DIRECTPLL0 :
			clksource =  tca_ckc_getpll(0);
			break;
		case DIRECTPLL1 :
			clksource =  tca_ckc_getpll(1);
			break;
		case DIRECTPLL2 :
			clksource =  tca_ckc_getpll(2);
			break;
		case DIRECTPLL3 :
			clksource =  tca_ckc_getpll(3);
			break;
		case DIRECTXIN:
			clksource =  120000;
			break;
		default : 
			clksource =  tca_ckc_getpll(1);
			break;
	}

	if (freq != 0)
	{
		clkdiv	= (clksource + (freq>>1)) / freq ;	// should be even number of division factor
		clkdiv -= 1;
	}
	else
		clkdiv	= 1;

	if(clkdiv == CLKDIV0) // The config value should not be "ZERO" = 1/(config+1)
		clkdiv = 1;

	if (machine_is_tcc9200s()) {
#if defined(CONFIG_TCC_CLK_600_330)
		if (clkdiv < 1)
			clkdiv = 1;
#else
		if (clkname == CLKCTRL1)	// ddi clock
		{
			clksource =  tca_ckc_getpll(3);
			clkdiv = 1;
		}
#endif
	}

	if(md == DYNAMIC_MD && !(clkname == CLKCTRL0 || clkname == CLKCTRL2))
	{
		/*
			CONFIG[3:0] 	: Curretn Divisor(Read-only)
			CONFIG[7:4] 	: Max. Divisor
			CONFIG[11:8]	: Min. Divisor
			CONFIG[15:12]	: Update Cycle Period
		*/
			lconfig = (clkdiv<<8); //Min. Divisor
			clkdiv = 10; //Max. Divisor
			lconfig |= ((clkdiv<<4)| 0xF000);	// Min. Divisor = Max. Divisor/2, Update Cycle Period = F
			
			clkdiv = lconfig;
	}

	if(clkname == CLKCTRL0 || clkname == CLKCTRL2)
	{
		*pCLKCTRL = tca_ckc_gclkctrly(isenable,md,clkdiv,sor,clkname);
	}
	else
	{
		if(isenable == DISABLE)
			*pCLKCTRL &= ~Hw21;
		else if (isenable == NOCHANGE)
			*pCLKCTRL = (*pCLKCTRL&(1<<21))|(md<<20)|(clkdiv<<4)|(sor<<0);
		else
			*pCLKCTRL = tca_ckc_gclkctrlx(isenable,md,clkdiv,sor);
	}
	
}

/****************************************************************************************
* FUNCTION :void tca_ckc_getfbusctrl(unsigned int clkname)
* DESCRIPTION :
* ***************************************************************************************/
VOLATILE int tca_ckc_getfbusctrl(unsigned int clkname)
{
	volatile unsigned	*pCLKCTRL;
	unsigned int lcheck = 0;
	unsigned int lmd = 0;
	unsigned int lconfig = 0;
	unsigned int lsel = 0;
	unsigned int clksource = 0; 

	pCLKCTRL =(volatile unsigned	*)((&pCKC->CLK0CTRL)+clkname); 

	lcheck = ((*pCLKCTRL >> 21) & Hw0);
	lmd = ((*pCLKCTRL >> 20) & Hw0);
	lconfig = ((*pCLKCTRL >> 4) & 0xF);
	lsel = ((*pCLKCTRL) & 0x7);
	
	if(!lcheck || (clkname == CLKCTRL0 || clkname == CLKCTRL2))
		return -1;

	if(lmd == 0)
	{
		switch(lsel)
		{
			case DIRECTPLL0 :
				clksource =  tca_ckc_getpll(0);
				break;
			case DIRECTPLL1 :
				clksource =  tca_ckc_getpll(1);
				break;
			case DIRECTPLL2 :
				clksource =  tca_ckc_getpll(2);
				break;
			case DIRECTPLL3 :
				clksource =  tca_ckc_getpll(3);
				break;
			case DIRECTXIN:
				clksource =  120000;
				break;
			default : 
				clksource =  tca_ckc_getpll(1);
				break;
		}

	}
	else
		return -1;
			
	return (clksource / (lconfig+1));
}

/****************************************************************************************
* FUNCTION :static unsigned int tca_ckc_setpllxcfg(unsigned int isEnable, int P, int M, int S)
* DESCRIPTION :
* ***************************************************************************************/
VOLATILE static unsigned int tca_ckc_gpllxcfg(unsigned int isenable, unsigned int p, unsigned int m, unsigned int s)
{
	unsigned int retVal = Hw31;//Disable
	
	if(isenable > 0)
	{
		retVal = (s<<24)|(m<<8)|(p<<0);
		retVal |= Hw31;	//Enable
	}

	return retVal;
}

/****************************************************************************************
* FUNCTION :static void tca_ckc_pll(unsigned int p, unsigned int m, unsigned int s,unsigned int ch)
* DESCRIPTION :
* ***************************************************************************************/
VOLATILE static void tca_ckc_pll(unsigned int p, unsigned int m, unsigned int s,unsigned int ch)
{
	volatile unsigned	*pPLLCFG;

	pPLLCFG =(volatile unsigned *)((&pCKC->PLL0CFG)+ch);	

	if(ch == 0) // PLL0 is System Clock Source
	{
		// Change System Clock Souce --> XIN (12Mhz)
	//	pCKC->CLK0CTRL = tca_ckc_gclkctrly(ENABLE,NORMAL_MD,CLKDIVNONCHANGE,DIRECTXIN,0);
		pCKC->CLK7CTRL = tca_ckc_gclkctrly(ENABLE, NORMAL_MD, CLKDIVNONCHANGE, DIRECTXIN, CLKCTRL7);
		pCKC->CLK0CTRL = tca_ckc_gclkctrly(ENABLE, NORMAL_MD, CLKDIVNONCHANGE, DIRECTPLL1, CLKCTRL0);
		tca_wait(); 
		tca_wait();
		tca_wait();
	}

	
	//Disable PLL
	*pPLLCFG &= ~Hw31;
	//Set PMS
	*pPLLCFG = tca_ckc_gpllxcfg(ENABLE,p,m,s);
	//Enable PLL
	*pPLLCFG |= Hw31;
	tca_wait(); 
	//Restore System Clock Source
	if(ch == 0)
	{
	//	pCKC->CLK2CTRL = tca_ckc_gclkctrly(ENABLE,NORMAL_MD,CLKDIVNONCHANGE,DIRECTPLL0,2);
		pCKC->CLK7CTRL = tca_ckc_gclkctrly(ENABLE, NORMAL_MD, CLKDIV4, DIRECTPLL0, CLKCTRL7);
		pCKC->CLK0CTRL = tca_ckc_gclkctrly(ENABLE, NORMAL_MD, CLKDIVNONCHANGE, DIRECTPLL0, CLKCTRL0);
	}
	
}


/****************************************************************************************
* FUNCTION :void tca_ckc_validpll(unsigned int * pvalidpll)
* DESCRIPTION :
* ***************************************************************************************/
VOLATILE void tca_ckc_validpll(unsigned int * pvalidpll)
{
	unsigned int uCnt;
	sfPLL		*pPLL;

	pPLL	= &pIO_CKC_PLL0[0];
	for (uCnt = 0; uCnt < NUM_PLL0; uCnt ++, pPLL ++)
	{
		*pvalidpll = pPLL->uFpll ;		
		pvalidpll++;
	}
};

/****************************************************************************************
* FUNCTION :int tca_ckc_setpll(unsigned int pll, unsigned int ch)
* DESCRIPTION :
* ***************************************************************************************/
VOLATILE int tca_ckc_setpll(unsigned int pll, unsigned int ch)
{
	unsigned	uCnt;
	int 		retVal = -1;
	unsigned int num_pll;
	
	sfPLL		*pPLL;

	if(pll != 0 )
	{
		if (ch == 0) {
			pPLL = &pIO_CKC_PLL0[0];
			num_pll = NUM_PLL0;
		} else {
			pPLL = &pIO_CKC_PLL1[0];
			num_pll = NUM_PLL1;
		}
		for (uCnt = 0; uCnt < num_pll; uCnt ++, pPLL ++) {
			if (pPLL->uFpll == pll)
				break;
		}
		
		if (uCnt < num_pll)
		{
			tca_ckc_pll(pPLL->P,pPLL->M ,pPLL->S,ch);
			retVal = 0;
			return 1;
		}
	}

	return -1;
}

/****************************************************************************************
* FUNCTION :void tca_ckc_setcpu(unsigned int n)
* DESCRIPTION :  n is n/16 
* example : CPU == PLL : n=16 - CPU == PLL/2 : n=8
* ***************************************************************************************/
VOLATILE void tca_ckc_setcpu(unsigned int n)
{
	 unsigned int lckc0ctrl;	
	 unsigned int lindex[] = {0x0,0x8000,0x8008,0x8808,0x8888,0xA888,0xA8A8,0xAAA8,0xAAAA,
							0xECCC,0xEECC,0xEEEC,0xEEEE,0xFEEE,0xFFEE,0xFFFE,0xFFFF};


	lckc0ctrl = pCKC->CLK0CTRL;
	lckc0ctrl &= ~(Hw20-Hw4);
	lckc0ctrl |= (lindex[n] << 4);

	pCKC->CLK0CTRL = lckc0ctrl;
	
	g_org_tcc_clk = tca_ckc_getclkctrl0();
}

/****************************************************************************************
* FUNCTION :void tca_ckc_setpmupwroff( unsigned int periname , unsigned int isenable)
* DESCRIPTION : PMU Block :  Power Off Register
* PMU_VIDEODAC
* PMU_HDMIPHY	  
* PMU_LVDSPHY	  
* PMU_USBNANOPHY   
* PMU_SATAPHY 
* PMU_MEMORYBUS	
* PMU_VIDEOBUS
* PMU_DDIBUS
* PMU_GRAPHICBUS
* PMU_IOBUS
* ***************************************************************************************/
VOLATILE void tca_ckc_setpmupwroff( unsigned int periname , unsigned int isenable)
{
	if(isenable)
		pPMU->PWROFF &= ~(1<<periname);
	else
		pPMU->PWROFF |= (1<<periname);	
}
/****************************************************************************************
* FUNCTION :void tca_ckc_getpmupwroff( unsigned int pmuoffname)
* DESCRIPTION : 
* ***************************************************************************************/
VOLATILE int tca_ckc_getpmupwroff( unsigned int pmuoffname)
{
	unsigned int retVal = 0;
	unsigned int lpoweroff = 0;
	lpoweroff = pPMU->PWROFF;

	switch(pmuoffname)
	{
		case PMU_VIDEODAC:
				retVal =  (lpoweroff & Hw0);;	
			break;
		case PMU_HDMIPHY:
				retVal =  (lpoweroff & Hw1);;	
			break;
		case PMU_LVDSPHY:
				retVal =  (lpoweroff & Hw2);;	
			break;
		case PMU_USBNANOPHY:
				retVal =  (lpoweroff & Hw3);;	
			break;
		case PMU_SATAPHY:
				retVal =  (lpoweroff & Hw4);;	
			break;
		case PMU_MEMORYBUS:
				retVal =  (lpoweroff & Hw5);;	
			break;
		case PMU_VIDEOBUS:
				retVal =  (lpoweroff & Hw6);;	
			break;
		case PMU_DDIBUS:
				retVal =  (lpoweroff & Hw7);;	
			break;
		case PMU_GRAPHICBUS:
				retVal =  (lpoweroff & Hw8);;	
			break;
		case PMU_IOBUS:
				retVal =  (lpoweroff & Hw9);;	
			break;
		default:
			break;
	}

	return retVal;
}

/****************************************************************************************
* FUNCTION :static unsigned int tca_ckc_setpckxxx(unsigned int isenable, unsigned int sel, unsigned int div)
* DESCRIPTION : 
* ***************************************************************************************/
VOLATILE static unsigned int tca_ckc_gpckxxx(unsigned int isenable, unsigned int sel, unsigned int div)
{
    unsigned int retVal;

    retVal = ((isenable?1:0)<<28)|(sel<<24)|(div<<0);
	return retVal;
}

/****************************************************************************************
* FUNCTION :static unsigned int tca_ckc_setpckyyy(unsigned int isenable, unsigned int sel, unsigned int div)
* DESCRIPTION : md (1: divider Mode, 0:DCO Mode)
* ***************************************************************************************/
VOLATILE static unsigned int tca_ckc_gpckyyy(unsigned int isenable, unsigned int md, unsigned int sel, unsigned int div)
{
	unsigned int retVal;

    retVal = (md<<31)|((isenable?1:0)<<28)|(sel<<24)|(div<<0);
	return retVal;
}

/****************************************************************************************
* FUNCTION :void tca_ckc_setperi(unsigned int periname,unsigned int isenable, unsigned int freq, unsigned int sor)
* DESCRIPTION : 
* ***************************************************************************************/
VOLATILE void tca_ckc_setperi(unsigned int periname,unsigned int isenable, unsigned int freq, unsigned int sor)
{
	unsigned uPll;
	unsigned int clkdiv = 0;
	unsigned int lclksource = 0;
	unsigned int clkmode = 1;
	
	volatile unsigned	*pPERI;
	pPERI =(volatile unsigned	*)((&pCKC->PCLK_TCX)+periname); 

	switch(sor)
	{
		case PCDIRECTPLL0 :
			lclksource =  tca_ckc_getpll(0);
			break;
		case PCDIRECTPLL1 :
			lclksource =  tca_ckc_getpll(1);
			break;
		case PCDIRECTPLL2 :
			lclksource =  tca_ckc_getpll(2);
			break;
		case PCDIRECTPLL3 :
			lclksource =  tca_ckc_getpll(3);
			break;
		case PCDIRECTXIN :
			lclksource =  120000;
			break;
		case PCHDMI :
			lclksource =  270000;
			break;
		case PCSATA :
			lclksource =  250000;		
			break;
		case PCUSBPHY:
			lclksource =  480000;		
			break;
		default : 
			lclksource =  tca_ckc_getpll(1);
			break;
	}
	
	if (freq != 0)
	{
		clkdiv	= (lclksource + (freq>>1)) / freq ; // should be even number of division factor
		clkdiv -= 1;
	}
	else
		clkdiv	= 0;
	
	if(periname == PERI_ADC || periname == PERI_SPDIF ||periname == PERI_AUD || periname == PERI_DAI)
	{
		if(periname == PERI_DAI || periname == PERI_SPDIF)
		{
			clkmode = 0;	// DCO Mode
			if (periname == PERI_SPDIF)
				freq /= 2;

			if(freq >= 261243) {
				clkdiv = (freq *8192);
				uPll = lclksource;
				clkdiv = clkdiv/uPll;
				clkdiv <<= 3;
				clkdiv = clkdiv + 1;
			}
			else if(freq >= 131071)
			{
				clkdiv = (freq *16384);
				uPll = lclksource;
				clkdiv = clkdiv/uPll;
				clkdiv <<= 2;
				clkdiv = clkdiv + 1;
			}
			else
			{
				clkdiv = (freq *32768);
				uPll = lclksource;
				clkdiv = clkdiv/uPll;
				clkdiv <<= 1;
				clkdiv = clkdiv + 1;
			}
			if (periname == PERI_SPDIF)
				clkdiv *= 2;
		}
		
		*pPERI = tca_ckc_gpckyyy(isenable,clkmode,sor,clkdiv);
	}
	else
	{
		*pPERI = tca_ckc_gpckxxx(isenable,sor,clkdiv);

	}
}

/****************************************************************************************
* FUNCTION : static int tca_ckc_gperi(unsigned int lclksrc, unsigned int ldiv,unsigned int lmd)
* DESCRIPTION :
* ***************************************************************************************/
VOLATILE static int tca_ckc_gperi(unsigned int lclksrc, unsigned int ldiv,unsigned int lmd)
{
	if(lmd == 1)
	{
		if(lclksrc == PCDIRECTXIN)
			return 120000/(ldiv+1);
		else if(lclksrc == PCDIRECTPLL0){
			return (tca_ckc_getpll(0)/(ldiv+1));
		}
		else if(lclksrc == PCDIRECTPLL1){
			return (tca_ckc_getpll(1)/(ldiv+1));
		}
		else if(lclksrc == PCDIRECTPLL2){
			return (tca_ckc_getpll(2)/(ldiv+1));
		}
		else if(lclksrc == PCDIRECTPLL3){
			return (tca_ckc_getpll(3)/(ldiv+1));
		}
		else if(lclksrc == PCHDMI){
			return (270000/(ldiv+1));
		}
		else if(lclksrc == PCSATA){
			return (250000/(ldiv+1));
		}
		else if(lclksrc == PCUSBPHY){
			return (480000/(ldiv+1));
		}
		else
			return -1; // Not Support Others

	}
	else
		return -1; // TO DO
}

/****************************************************************************************
* FUNCTION : int tca_ckc_getperi(unsigned int periname)
* DESCRIPTION :
* ***************************************************************************************/
VOLATILE int tca_ckc_getperi(unsigned int periname)
{
	unsigned int lreg = 0;
	unsigned int lmd = 1; // DIVIDER mode
	unsigned int lclksrc = 0;
	unsigned int ldiv = 0;
	
	lreg =*(volatile unsigned	*)((&pCKC->PCLK_TCX)+periname); 
	lclksrc = (lreg&0xF000000)>>24;
	
	if(periname == PERI_ADC || periname == PERI_SPDIF ||periname == PERI_AUD || periname == PERI_DAI)
	{
		lmd = (lreg&0x80000000);
		ldiv = (lreg & 0xFFFF);
		return tca_ckc_gperi(lclksrc, ldiv,lmd);
	}
	else
	{
		ldiv = (lreg & 0xFFF);
		return tca_ckc_gperi(lclksrc, ldiv,1);
	}
}

/****************************************************************************************
* FUNCTION :void tca_ckc_setswresetprd(unsigned int prd)
* DESCRIPTION : 
* ***************************************************************************************/
VOLATILE void tca_ckc_setswresetprd(unsigned int prd)
{
	pCKC->SWRESETPRD = prd<<0;
}

/****************************************************************************************
* FUNCTION :void tca_ckc_set_iobus_swreset(unsigned int sel, unsigned int mode)
* DESCRIPTION : 
* ***************************************************************************************/
VOLATILE unsigned int tca_ckc_set_iobus_swreset(unsigned int sel, unsigned int mode)
{
	unsigned int lindex[] = {Hw0,Hw1,Hw2,Hw3,Hw4,Hw5,Hw6,Hw7,Hw8,Hw9,Hw10,Hw11,Hw12,Hw13,Hw14,Hw15
							,Hw16,Hw17,Hw18,Hw19,Hw20,Hw21,Hw22,Hw23,Hw24,Hw25,Hw26,Hw27,Hw28,Hw29,Hw30,Hw31};

	unsigned int lrb_min;
	unsigned int lrb_max;
	unsigned int lrb_seperate;

	lrb_min = RB_USB11H;
	lrb_max = RB_ALLPERIPERALS;
	lrb_seperate = RB_ADMACONTROLLER;

	if(sel <  lrb_min || sel >= lrb_max)
	{
		return 0;
	}

	if(sel >  lrb_seperate)
	{
		sel -=	(lrb_seperate+1);
		
		if(mode)
			pIOBUSCFG->HRSTEN1 |= lindex[sel];
		else
			pIOBUSCFG->HRSTEN1 &= ~lindex[sel];
	}
	else
	{
		if(mode)
			pIOBUSCFG->HRSTEN0 |= lindex[sel];
		else
			pIOBUSCFG->HRSTEN0 &= ~lindex[sel];
	}

	return 1;
}

/****************************************************************************************
* FUNCTION :void tca_ckc_setswreset(unsigned int lfbusname, unsigned int mode)
* DESCRIPTION : 
* ***************************************************************************************/

VOLATILE void  tca_ckc_setswreset(unsigned int lfbusname, unsigned int mode)
{
	unsigned int hIndex[] = {Hw0,Hw1,Hw2,Hw3,Hw4,Hw5,Hw6,Hw7};

	if(mode)
		pCKC->SWRESET |= hIndex[lfbusname];
	else
		pCKC->SWRESET &= ~(hIndex[lfbusname]);
}
/****************************************************************************************
* FUNCTION :  int tca_ckc_setiobus(unsigned int sel, unsigned int mode)
* DESCRIPTION : 
* ***************************************************************************************/
VOLATILE void tca_ckc_setiobus(unsigned int sel, unsigned int mode)
{
	unsigned int lindex[] = {Hw0,Hw1,Hw2,Hw3,Hw4,Hw5,Hw6,Hw7,Hw8,Hw9,Hw10,Hw11,Hw12,Hw13,Hw14,Hw15
							,Hw16,Hw17,Hw18,Hw19,Hw20,Hw21,Hw22,Hw23,Hw24,Hw25,Hw26,Hw27,Hw28,Hw29,Hw30,Hw31};

	unsigned int lrb_min;
	unsigned int lrb_max;
	unsigned int lrb_seperate;

	lrb_min = RB_USB11H;
	lrb_max = RB_ALLPERIPERALS;
	lrb_seperate = RB_ADMACONTROLLER;

	if(sel <  lrb_min || sel >=  lrb_max)
		return;

	if(sel >  lrb_seperate)
	{
		sel -=	(lrb_seperate+1);
		
		if(mode)
			pIOBUSCFG->HCLKEN1 |= lindex[sel];
		else
			pIOBUSCFG->HCLKEN1 &= ~lindex[sel];
	}
	else
	{
		if(mode)
			pIOBUSCFG->HCLKEN0 |= lindex[sel];
		else
			pIOBUSCFG->HCLKEN0 &= ~lindex[sel];
	}
}

/****************************************************************************************
* FUNCTION :  int tca_ckc_getiobus(unsigned int sel)
* DESCRIPTION : 
* ***************************************************************************************/
VOLATILE int tca_ckc_getiobus(unsigned int sel)
{
	unsigned int lindex[] = {Hw0,Hw1,Hw2,Hw3,Hw4,Hw5,Hw6,Hw7,Hw8,Hw9,Hw10,Hw11,Hw12,Hw13,Hw14,Hw15
							,Hw16,Hw17,Hw18,Hw19,Hw20,Hw21,Hw22,Hw23,Hw24,Hw25,Hw26,Hw27,Hw28,Hw29,Hw30,Hw31};
	unsigned int lrb_min;
	unsigned int lrb_max;
	unsigned int lrb_seperate;
	int lretVal = 0;
	
	lrb_min = RB_USB11H;
	lrb_max = RB_ALLPERIPERALS;
	lrb_seperate = RB_ADMACONTROLLER;

	
	if(sel <  lrb_min || sel >=  lrb_max)
	{
		return -1;
	}
		
	if(sel >  lrb_seperate)
	{
		sel -=	(lrb_seperate+1);

		lretVal = (pIOBUSCFG->HCLKEN1  & lindex[sel]) ;

	}
	else
	{
		lretVal = (pIOBUSCFG->HCLKEN0  & lindex[sel]) ;
	}

	if(lretVal != 0)
		lretVal = 1; // Enable
		
	return lretVal;
}

VOLATILE void tca_ckc_setioswreset(unsigned int sel, unsigned int mode)
{
	unsigned int lindex[] = {Hw0,Hw1,Hw2,Hw3,Hw4,Hw5,Hw6,Hw7,Hw8,Hw9,Hw10,Hw11,Hw12,Hw13,Hw14,Hw15
							,Hw16,Hw17,Hw18,Hw19,Hw20,Hw21,Hw22,Hw23,Hw24,Hw25,Hw26,Hw27,Hw28,Hw29,Hw30,Hw31};

	unsigned int lrb_min;
	unsigned int lrb_max;
	unsigned int lrb_seperate;

	lrb_min = RB_USB11H;
	lrb_max = RB_ALLPERIPERALS;
	lrb_seperate = RB_ADMACONTROLLER;

	if(sel <  lrb_min || sel >=  lrb_max)
		return;

	if(sel >  lrb_seperate)
	{
		sel -=	(lrb_seperate+1);
		
		if(mode)
			pIOBUSCFG->HRSTEN1 &= ~lindex[sel];
		else
			pIOBUSCFG->HRSTEN1 |= lindex[sel];
	}
	else
	{
		if(mode)
			pIOBUSCFG->HRSTEN0 &= ~lindex[sel];
		else
			pIOBUSCFG->HRSTEN0 |= lindex[sel];
	}
}

/****************************************************************************************
* FUNCTION :  int tca_ckc_setsmui2c(unsigned int freq)
* DESCRIPTION : unit : 100Hz
* ***************************************************************************************/
VOLATILE void tca_ckc_setsmui2c(unsigned int freq)
{
	PSMUI2CICLK lSMUICLK;
	unsigned int lclkctrl7=0;
	unsigned int lsel=0;
	unsigned int lclksource=0;
	unsigned int lclkdiv=0;
	
	lSMUICLK = (PSMUI2CICLK)(iomap_p2v((unsigned int)&HwSMU_I2CICLK_BASE)); //0xF0400000
	lclkctrl7 = (unsigned int)pCKC->CLK7CTRL;

	lsel = (lclkctrl7 & 7);
	
	if(((lclkctrl7 >>20) & Hw0) == 0 ) // Normal Mode
	{
		switch(lsel)
		{
			case DIRECTPLL0:
				lclksource = tca_ckc_getpll(0);
				break;
			case DIRECTPLL1:
				lclksource = tca_ckc_getpll(1);
				break;
			case DIRECTPLL2:
				lclksource = tca_ckc_getpll(2);
				break;
			case DIRECTPLL3:
				lclksource = tca_ckc_getpll(3);
				break;
			default :
				lclksource = tca_ckc_getpll(1);
				break;

		}
	}
	
	if (freq != 0)
	{
		lclkdiv	= (lclksource + (freq>>1)) / freq ; // should be even number of division factor
		lSMUICLK->ICLK = (Hw31|lclkdiv);
	}
	else
	{
		lclkdiv	= 0;
		lSMUICLK->ICLK = 0;
	}


}
/****************************************************************************************
* FUNCTION :  int tca_ckc_getsmui2c(void)
* DESCRIPTION : unit : 100Hz
* ***************************************************************************************/
VOLATILE int tca_ckc_getsmui2c(void)
{
	PSMUI2CICLK lSMUICLK;
	unsigned int lclkctrl7;
	unsigned int lsel;
	unsigned int lclksource;
	unsigned int lclkdiv;

	lSMUICLK = (PSMUI2CICLK)(iomap_p2v((unsigned int)&HwSMU_I2CICLK_BASE)); //0xF0400000
	lclkctrl7 = (unsigned int)pCKC->CLK7CTRL;

	lsel = (lclkctrl7 & 7);

	if(((lclkctrl7 >>20) & Hw0) == 0 ) // Normal Mode
	{
		switch(lsel)
		{
			case DIRECTPLL0:
				lclksource = tca_ckc_getpll(0);
				break;
			case DIRECTPLL1:
				lclksource = tca_ckc_getpll(1);
				break;
			case DIRECTPLL2:
				lclksource = tca_ckc_getpll(2);
				break;
			case DIRECTPLL3:
				lclksource = tca_ckc_getpll(3);
				break;
			default :
				lclksource = tca_ckc_getpll(1);
				break;

		}
		lclkdiv = (lclkctrl7 & 0xFFFF);
		
		if (lclkdiv != 0)
		{
			return (lclksource / lclkdiv) ; 
		}
		else
			return -1;
	}
	else
		return -1;

}

/****************************************************************************************
* FUNCTION : void tca_ckc_setddipwdn(unsigned int lpwdn , unsigned int lmode)
* DESCRIPTION : Power Down Register of DDI_CONFIG 
* ***************************************************************************************/
VOLATILE void tca_ckc_setddipwdn(unsigned int lpwdn , unsigned int lmode)
{
	PDDICONFIG lDDIPWDN;
	unsigned int lindex[] = {Hw0,Hw1,Hw2,Hw3,Hw4,Hw5,Hw6,Hw7,Hw8};

	if(!(pCKC->CLK1CTRL & Hw21))
		return;

	lDDIPWDN = (PDDICONFIG)(iomap_p2v((unsigned int)&HwDDI_CONFIG_BASE)); //0xF0400000

	if(lmode)  // Normal
		lDDIPWDN->PWDN &= ~lindex[lpwdn];
	else // Power Down
		lDDIPWDN->PWDN |= lindex[lpwdn];
	
	pr_debug("DDI PWDN(0x%x) \n", lDDIPWDN->PWDN);
}
/****************************************************************************************
* FUNCTION : int tca_ckc_getddipwdn(unsigned int lpwdn)
* DESCRIPTION : Power Down Register of DDI_CONFIG 
* ***************************************************************************************/
int tca_ckc_getddipwdn(unsigned int lpwdn)
{
	PDDICONFIG lDDIPWDN;
	unsigned int lindex[] = {Hw0,Hw1,Hw2,Hw3,Hw4,Hw5,Hw6,Hw7,Hw8};

	if(!(pCKC->CLK1CTRL & Hw21))
		return 1;

	lDDIPWDN = (PDDICONFIG)(iomap_p2v((unsigned int)&HwDDI_CONFIG_BASE)); //0xF0400000

	return (lDDIPWDN->PWDN &  lindex[lpwdn]);
}

VOLATILE void tca_ckc_setddiswreset(unsigned int lpwdn , unsigned int lmode)
{
	PDDICONFIG lDDIPWDN;
	unsigned int lindex[] = {Hw0,Hw1,Hw2,Hw3,Hw4,Hw5,Hw6,Hw7,Hw8};

	if(!(pCKC->CLK1CTRL & Hw21))
		return;

	lDDIPWDN = (PDDICONFIG)(iomap_p2v((unsigned int)&HwDDI_CONFIG_BASE)); //0xF0400000

	if(lmode)
		lDDIPWDN->SWRESET |= lindex[lpwdn];
	else
		lDDIPWDN->SWRESET &= ~lindex[lpwdn];
}

/****************************************************************************************
* FUNCTION : void tca_ckc_setgrppwdn(unsigned int lpwdn , unsigned int lmode)
* DESCRIPTION : Power Down Register of GRP_CONFIG 
* ***************************************************************************************/
VOLATILE void tca_ckc_setgrppwdn(unsigned int lpwdn , unsigned int lmode)
{
	PGPUGRPBUSCONFIG lGRPPWDN;
	unsigned int lindex[] = {Hw0,Hw1};

	if(!(pCKC->CLK3CTRL & Hw21))
		return;

	lGRPPWDN = (PGPUGRPBUSCONFIG)(iomap_p2v((unsigned int)&HwGRPBUS_BASE));

	if(lmode)  // Normal
		lGRPPWDN->GRPBUS_PWRDOWN &= ~lindex[lpwdn];
	else // Power Down
		lGRPPWDN->GRPBUS_PWRDOWN |= lindex[lpwdn];
	
	pr_debug("GRP PWDN(0x%x) \n", lGRPPWDN->GRPBUS_PWRDOWN);
}
/****************************************************************************************
* FUNCTION : int tca_ckc_getgrppwdn(unsigned int lpwdn)
* DESCRIPTION : Power Down Register of GRP_CONFIG 
* ***************************************************************************************/
int tca_ckc_getgrppwdn(unsigned int lpwdn)
{
	PGPUGRPBUSCONFIG lGRPPWDN;
	unsigned int lindex[] = {Hw0,Hw1};

	if(!(pCKC->CLK3CTRL & Hw21))
		return 1;

	lGRPPWDN = (PGPUGRPBUSCONFIG)(iomap_p2v((unsigned int)&HwGRPBUS_BASE));

	return (lGRPPWDN->GRPBUS_PWRDOWN &  lindex[lpwdn]);
}

VOLATILE void tca_ckc_setgrpswreset(unsigned int lpwdn , unsigned int lmode)
{
	PGPUGRPBUSCONFIG lGRPPWDN;
	unsigned int lindex[] = {Hw0,Hw1};

	if(!(pCKC->CLK3CTRL & Hw21))
		return;

	lGRPPWDN = (PGPUGRPBUSCONFIG)(iomap_p2v((unsigned int)&HwGRPBUS_BASE));

	if(lmode)
		lGRPPWDN->GRPBUS_SWRESET |= lindex[lpwdn];
	else
		lGRPPWDN->GRPBUS_SWRESET &= ~lindex[lpwdn];
}

/****************************************************************************************
* FUNCTION : void tca_ckc_setvbuspwdn(unsigned int lpwdn , unsigned int lmode)
* DESCRIPTION : Power Down Register of VIDEO_BUS 
* ***************************************************************************************/
VOLATILE void tca_ckc_setvbuspwdn(unsigned int lpwdn , unsigned int lmode)
{
	PVIDEOCODEC VBUSPWDN;
	unsigned int lindex[] = {Hw0,Hw1,Hw2,Hw3};

	if(!(pCKC->CLK5CTRL & Hw21))
		return;

	VBUSPWDN = (PVIDEOCODEC)(iomap_p2v((unsigned int)&HwVIDEOBUS_BASE));

	if(lmode)  // Normal
		VBUSPWDN->PWDN &= ~lindex[lpwdn];
	else // Power Down
		VBUSPWDN->PWDN |= lindex[lpwdn];

	pr_debug("VBUS PWDN(0x%x) \n", VBUSPWDN->PWDN);
}
/****************************************************************************************
* FUNCTION :void tca_ckc_getclkctrl0(void)
* DESCRIPTION :
* ***************************************************************************************/
VOLATILE unsigned int tca_ckc_getclkctrl0(void)
{
	if (machine_is_tcc9200s()) {
		return 0;
	} else {
		return pCKC->CLK0CTRL;
	}
}

/****************************************************************************************
* FUNCTION : int tca_ckc_getvbuspwdn(unsigned int lpwdn)
* DESCRIPTION : Power Down Register of VIDEO_BUS 
* ***************************************************************************************/
int tca_ckc_getvbuspwdn(unsigned int lpwdn)
{

	PVIDEOCODEC VBUSPWDN;
	unsigned int lindex[] = {Hw0,Hw1,Hw2,Hw3};

	if(!(pCKC->CLK5CTRL & Hw21))
		return 1;

	VBUSPWDN = (PVIDEOCODEC)(iomap_p2v((unsigned int)&HwVIDEOBUS_BASE));

	return (VBUSPWDN->PWDN &  lindex[lpwdn]);
	
}

VOLATILE void tca_ckc_setvbusswreset(unsigned int lpwdn , unsigned int lmode)
{
	PVIDEOCODEC VBUSPWDN;
	unsigned int lindex[] = {Hw0,Hw1,Hw2,Hw3};

	if(!(pCKC->CLK5CTRL & Hw21))
		return;

	VBUSPWDN = (PVIDEOCODEC)(iomap_p2v((unsigned int)&HwVIDEOBUS_BASE));

	if(lmode)
		VBUSPWDN->SWRESET |= lindex[lpwdn];
	else
		VBUSPWDN->SWRESET &= ~lindex[lpwdn];
}

void tca_ckc_enable(int clk, int enable)
{
	volatile unsigned *pCLKCTRL;

	pCLKCTRL =(volatile unsigned    *)((&pCKC->CLK0CTRL)+clk);
	if (enable)
		*pCLKCTRL |= HwCLKCTRL_EN;
	else
		*pCLKCTRL &= ~HwCLKCTRL_EN;
}

void tca_ckc_pclk_enable(int pclk, int enable)
{
	volatile unsigned *pPERI;
	pPERI =(volatile unsigned *)((&pCKC->PCLK_TCX)+pclk);
        if (enable)
                *pPERI |= Hw28;
        else
                *pPERI &= ~Hw28;
}

#if defined(CONFIG_DRAM_DDR2)
int tcc_ddr2_set_clock(unsigned int freq);
#elif defined(CONFIG_DRAM_MDDR)
int tcc_mddr_set_clock(unsigned int freq);
#endif

int tca_ckc_set_mem_freq(unsigned int freq)
{
#if defined(CONFIG_DRAM_DDR2)
	return tcc_ddr2_set_clock(freq);
#elif defined(CONFIG_DRAM_MDDR)
	return tcc_mddr_set_clock(freq);
#endif
}

/****************************************************************************************
* EXPORT_SYMBOL clock functions for Linux
* ***************************************************************************************/
#if defined(_LINUX_)
EXPORT_SYMBOL(tca_ckc_init);
EXPORT_SYMBOL(tca_ckc_getpll);
EXPORT_SYMBOL(tca_ckc_getcpu);
EXPORT_SYMBOL(tca_ckc_getbus);
//EXPORT_SYMBOL(tca_ckc_gclkctrlx);
//EXPORT_SYMBOL(tca_ckc_gclkctrly);
EXPORT_SYMBOL(tca_ckc_setfbusctrl);
EXPORT_SYMBOL(tca_ckc_getfbusctrl);
//EXPORT_SYMBOL(tca_ckc_gpllxcfg);
//EXPORT_SYMBOL(tca_ckc_pll);
EXPORT_SYMBOL(tca_ckc_validpll);
EXPORT_SYMBOL(tca_ckc_setpll);
EXPORT_SYMBOL(tca_ckc_setpmupwroff);
EXPORT_SYMBOL(tca_ckc_getpmupwroff);
//EXPORT_SYMBOL(tca_ckc_gpckxxx);
//EXPORT_SYMBOL(tca_ckc_gpckyyy);
EXPORT_SYMBOL(tca_ckc_setperi);
//EXPORT_SYMBOL(tca_ckc_gperi);
EXPORT_SYMBOL(tca_ckc_getperi);
EXPORT_SYMBOL(tca_ckc_setswresetprd);
EXPORT_SYMBOL(tca_ckc_set_iobus_swreset);
EXPORT_SYMBOL(tca_ckc_setswreset);
EXPORT_SYMBOL(tca_ckc_setiobus);
EXPORT_SYMBOL(tca_ckc_getiobus);
EXPORT_SYMBOL(tca_ckc_setsmui2c);
EXPORT_SYMBOL(tca_ckc_getsmui2c);
EXPORT_SYMBOL(tca_ckc_setddipwdn);
EXPORT_SYMBOL(tca_ckc_getddipwdn);
EXPORT_SYMBOL(tca_ckc_enable);
EXPORT_SYMBOL(tca_ckc_pclk_enable);
EXPORT_SYMBOL(tca_ckc_set_mem_freq);
#endif

/* end of file */
