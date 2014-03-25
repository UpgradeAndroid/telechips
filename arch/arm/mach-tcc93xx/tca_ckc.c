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

#include <mach/bsp.h>
#include <asm/io.h> 
#include <linux/mm.h>	// for PAGE_ALIGN
#include <linux/kernel.h>
#include <linux/module.h>

#ifndef VOLATILE
#define VOLATILE
#endif

typedef struct {
	unsigned			uFpll;
	unsigned			P;
	unsigned			M;
	unsigned			S;
	unsigned			VSEL;
} sfPLL;

#define PLLFREQ(P, M, S)		(( 120000 * (M) )  / (P) ) >> (S) // 100Hz Unit..
#define FPLL_t(P, M, S, VSEL) 	PLLFREQ(P,M,S), P, M, S, VSEL
// PLL table for XIN=12MHz
// P, M, S, VSEL
sfPLL	pIO_CKC_PLL012[]	=
{
	 {FPLL_t(3, 280, 4, 0)}		// 70
	,{FPLL_t(3, 288, 4, 0)}		// 72
	,{FPLL_t(3, 344, 4, 0)}		// 86
	,{FPLL_t(3, 404, 4, 1)}		// 101
	,{FPLL_t(3, 264, 3, 0)}		// 132
	,{FPLL_t(3, 292, 3, 0)}		// 146
	,{FPLL_t(3, 360, 3, 1)}		// 180
	,{FPLL_t(3, 368, 3, 1)}		// 184
	,{FPLL_t(3, 380, 3, 1)}		// 190
	,{FPLL_t(3, 384, 3, 1)}		// 192
	,{FPLL_t(3, 408, 3, 1)}		// 204
	,{FPLL_t(3, 416, 3, 1)}		// 208
	,{FPLL_t(3, 420, 3, 1)}		// 210
	,{FPLL_t(3, 426, 3, 1)}		// 213
	,{FPLL_t(3, 428, 3, 1)}		// 214
	,{FPLL_t(3, 432, 3, 1)}		// 216
	,{FPLL_t(3, 444, 3, 1)}		// 222
	,{FPLL_t(3, 452, 3, 1)}		// 226
	,{FPLL_t(3, 456, 3, 1)}		// 228
	,{FPLL_t(3, 460, 3, 1)}		// 230
	,{FPLL_t(3, 468, 3, 1)}		// 234
	,{FPLL_t(3, 472, 3, 1)}		// 236
	,{FPLL_t(3, 488, 3, 1)}		// 244
	,{FPLL_t(3, 492, 3, 1)}		// 246
	,{FPLL_t(3, 496, 3, 1)}		// 248
	,{FPLL_t(3, 252, 2, 0)}		// 252
	,{FPLL_t(3, 260, 2, 0)}		// 260
	,{FPLL_t(3, 262, 2, 0)}		// 262
	,{FPLL_t(3, 264, 2, 0)}		// 264
	,{FPLL_t(3, 266, 2, 0)}		// 266
	,{FPLL_t(3, 276, 2, 0)}		// 276
	,{FPLL_t(3, 282, 2, 0)}		// 282
	,{FPLL_t(3, 288, 2, 0)}		// 288
	,{FPLL_t(3, 290, 2, 0)}		// 290
	,{FPLL_t(3, 294, 2, 0)}		// 294
	,{FPLL_t(3, 304, 2, 0)}		// 304
	,{FPLL_t(3, 308, 2, 0)}		// 308
	,{FPLL_t(3, 312, 2, 0)}		// 312
	,{FPLL_t(3, 316, 2, 0)}		// 316
	,{FPLL_t(3, 320, 2, 0)}		// 320
	,{FPLL_t(3, 324, 2, 0)}		// 324
	,{FPLL_t(3, 312, 2, 0)}		// 312
	,{FPLL_t(3, 316, 2, 0)}		// 316
	,{FPLL_t(3, 320, 2, 0)}		// 320
	,{FPLL_t(3, 324, 2, 0)}		// 324
	,{FPLL_t(3, 328, 2, 0)}		// 328
	,{FPLL_t(3, 330, 2, 0)}		// 330
	,{FPLL_t(3, 332, 2, 0)}		// 332
	,{FPLL_t(3, 336, 2, 0)}		// 336
	,{FPLL_t(3, 340, 2, 0)}		// 340
	,{FPLL_t(3, 348, 2, 0)}		// 348
	,{FPLL_t(3, 352, 2, 1)}		// 352
	,{FPLL_t(3, 356, 2, 1)}		// 356
	,{FPLL_t(3, 360, 2, 1)}		// 360
	,{FPLL_t(3, 362, 2, 1)}		// 362
	,{FPLL_t(3, 364, 2, 1)}		// 364
	,{FPLL_t(3, 369, 2, 1)}		// 369
	,{FPLL_t(3, 370, 2, 1)}		// 370
	,{FPLL_t(3, 372, 2, 1)}		// 372
	,{FPLL_t(3, 380, 2, 1)}		// 380
	,{FPLL_t(3, 381, 2, 1)}		// 381
	,{FPLL_t(3, 384, 2, 1)}		// 384
	,{FPLL_t(3, 388, 2, 1)}		// 388
	,{FPLL_t(3, 390, 2, 1)}		// 390
	,{FPLL_t(3, 392, 2, 1)}		// 392
	,{FPLL_t(3, 393, 2, 1)}		// 393
	,{FPLL_t(3, 396, 2, 1)}		// 396
	,{FPLL_t(3, 399, 2, 1)}		// 399
	,{FPLL_t(3, 400, 2, 1)}		// 400
	,{FPLL_t(3, 404, 2, 1)}		// 404
	,{FPLL_t(4, 541, 2, 1)}		// 405.75
	,{FPLL_t(3, 406, 2, 1)}		// 406
	,{FPLL_t(3, 408, 2, 1)}		// 408
	,{FPLL_t(3, 410, 2, 1)}		// 410
	,{FPLL_t(3, 412, 2, 1)}		// 412
	,{FPLL_t(3, 416, 2, 1)}		// 416
	,{FPLL_t(4, 557, 2, 1)}		// 417.75
	,{FPLL_t(3, 420, 2, 1)}		// 420
	,{FPLL_t(3, 424, 2, 1)}		// 424
	,{FPLL_t(3, 428, 2, 1)}		// 428
	,{FPLL_t(3, 429, 2, 1)}		// 429
	,{FPLL_t(3, 430, 2, 1)}		// 430
	,{FPLL_t(3, 432, 2, 1)}		// 432
	,{FPLL_t(3, 436, 2, 1)}		// 436
	,{FPLL_t(3, 440, 2, 1)}		// 440
	,{FPLL_t(3, 442, 2, 1)}		// 442
	,{FPLL_t(3, 444, 2, 1)}		// 444
	,{FPLL_t(3, 448, 2, 1)}		// 448
	,{FPLL_t(3, 450, 2, 1)}		// 450
	,{FPLL_t(3, 452, 2, 1)}		// 452
	,{FPLL_t(3, 454, 2, 1)}		// 454
	,{FPLL_t(3, 456, 2, 1)}		// 456
	,{FPLL_t(3, 460, 2, 1)}		// 460
	,{FPLL_t(3, 463, 2, 1)}		// 463
	,{FPLL_t(3, 464, 2, 1)}		// 464
	,{FPLL_t(3, 466, 2, 1)}		// 466
	,{FPLL_t(3, 468, 2, 1)}		// 468
	,{FPLL_t(3, 470, 2, 1)}		// 470
	,{FPLL_t(3, 472, 2, 1)}		// 472
	,{FPLL_t(3, 474, 2, 1)}		// 474
	,{FPLL_t(3, 476, 2, 1)}		// 476
	,{FPLL_t(3, 480, 2, 1)}		// 480
	,{FPLL_t(3, 484, 2, 1)}		// 484
	,{FPLL_t(3, 486, 2, 1)}		// 486
	,{FPLL_t(3, 488, 2, 1)}		// 488
	,{FPLL_t(3, 490, 2, 1)}		// 490
	,{FPLL_t(3, 492, 2, 1)}		// 492
	,{FPLL_t(3, 496, 2, 1)}		// 496
	,{FPLL_t(3, 497, 2, 1)}		// 497
	,{FPLL_t(3, 498, 2, 1)}		// 498
	,{FPLL_t(3, 500, 2, 1)}		// 500
	,{FPLL_t(3, 252, 1, 0)}		// 504
	,{FPLL_t(3, 254, 1, 0)}		// 508
	,{FPLL_t(3, 256, 1, 0)}		// 512
	,{FPLL_t(3, 258, 1, 0)}		// 516
	,{FPLL_t(3, 260, 1, 0)}		// 520
	,{FPLL_t(3, 262, 1, 0)}		// 524
	,{FPLL_t(3, 264, 1, 0)}		// 528
	,{FPLL_t(3, 266, 1, 0)}		// 532
	,{FPLL_t(3, 268, 1, 0)}		// 536
	,{FPLL_t(3, 270, 1, 0)}		// 540
	,{FPLL_t(3, 272, 1, 0)}		// 544
	,{FPLL_t(3, 274, 1, 0)}		// 548
	,{FPLL_t(3, 276, 1, 0)}		// 552
	,{FPLL_t(3, 278, 1, 0)}		// 556
	,{FPLL_t(3, 280, 1, 0)}		// 560
	,{FPLL_t(3, 282, 1, 0)}		// 564
	,{FPLL_t(3, 284, 1, 0)}		// 568
	,{FPLL_t(3, 286, 1, 0)}		// 572
	,{FPLL_t(3, 288, 1, 0)}		// 576
	,{FPLL_t(3, 290, 1, 0)}		// 580
	,{FPLL_t(3, 292, 1, 0)}		// 584
	,{FPLL_t(3, 294, 1, 0)}		// 588
	,{FPLL_t(3, 296, 1, 0)}		// 592
	,{FPLL_t(3, 298, 1, 0)}		// 596
	,{FPLL_t(3, 300, 1, 0)}		// 600
	,{FPLL_t(3, 302, 1, 0)}		// 604
	,{FPLL_t(3, 304, 1, 0)}		// 608
	,{FPLL_t(3, 306, 1, 0)}		// 612
	,{FPLL_t(3, 308, 1, 0)}		// 616
	,{FPLL_t(3, 310, 1, 0)}		// 620
	,{FPLL_t(3, 312, 1, 0)}		// 624
	,{FPLL_t(3, 314, 1, 0)}		// 628
	,{FPLL_t(3, 316, 1, 0)}		// 632
	,{FPLL_t(3, 318, 1, 0)}		// 636
	,{FPLL_t(4, 426, 1, 0)}		// 639
	,{FPLL_t(3, 320, 1, 0)}		// 640
	,{FPLL_t(3, 322, 1, 0)}		// 644
	,{FPLL_t(3, 324, 1, 0)}		// 648
	,{FPLL_t(3, 326, 1, 0)}		// 652
	,{FPLL_t(3, 328, 1, 0)}		// 656
	,{FPLL_t(3, 330, 1, 0)}		// 660
	,{FPLL_t(3, 332, 1, 0)}		// 664
	,{FPLL_t(3, 334, 1, 0)}		// 668
	,{FPLL_t(3, 336, 1, 0)}		// 672
	,{FPLL_t(3, 338, 1, 0)}		// 676
	,{FPLL_t(3, 340, 1, 0)}		// 680
	,{FPLL_t(3, 342, 1, 0)}		// 684
	,{FPLL_t(3, 344, 1, 0)}		// 688
	,{FPLL_t(3, 346, 1, 0)}		// 692
	,{FPLL_t(3, 348, 1, 0)}		// 696
	,{FPLL_t(3, 350, 1, 0)}		// 700
	,{FPLL_t(3, 352, 1, 1)}		// 704
	,{FPLL_t(3, 354, 1, 1)}		// 708
	,{FPLL_t(3, 356, 1, 1)}		// 712
	,{FPLL_t(3, 358, 1, 1)}		// 716
	,{FPLL_t(3, 360, 1, 1)}		// 720
	,{FPLL_t(3, 362, 1, 1)}		// 724
	,{FPLL_t(3, 364, 1, 1)}		// 728
	,{FPLL_t(3, 366, 1, 1)}		// 732
	,{FPLL_t(3, 368, 1, 1)}		// 736
	,{FPLL_t(3, 370, 1, 1)}		// 740
	,{FPLL_t(3, 372, 1, 1)}		// 744
	,{FPLL_t(3, 374, 1, 1)}		// 748
	,{FPLL_t(3, 376, 1, 1)}		// 752
	,{FPLL_t(3, 378, 1, 1)}		// 756
	,{FPLL_t(3, 380, 1, 1)}		// 760
	,{FPLL_t(3, 382, 1, 1)}		// 764
	,{FPLL_t(3, 384, 1, 1)}		// 768
	,{FPLL_t(3, 386, 1, 1)}		// 772
	,{FPLL_t(3, 388, 1, 1)}		// 776
	,{FPLL_t(3, 390, 1, 1)}		// 780
	,{FPLL_t(3, 392, 1, 1)}		// 784
	,{FPLL_t(3, 394, 1, 1)}		// 788
	,{FPLL_t(3, 396, 1, 1)}		// 792
	,{FPLL_t(3, 398, 1, 1)}		// 796
	,{FPLL_t(3, 400, 1, 1)}		// 800
	,{FPLL_t(3, 402, 1, 1)}		// 804
	,{FPLL_t(3, 404, 1, 1)}		// 808
	,{FPLL_t(3, 406, 1, 1)}		// 812
	,{FPLL_t(3, 408, 1, 1)}		// 816
	,{FPLL_t(3, 410, 1, 1)}		// 820
	,{FPLL_t(3, 412, 1, 1)}		// 824
	,{FPLL_t(3, 414, 1, 1)}		// 828
	,{FPLL_t(3, 416, 1, 1)}		// 832
	,{FPLL_t(3, 418, 1, 1)}		// 836
	,{FPLL_t(3, 420, 1, 1)}		// 840
	,{FPLL_t(3, 422, 1, 1)}		// 844
	,{FPLL_t(3, 424, 1, 1)}		// 848
	,{FPLL_t(3, 426, 1, 1)}		// 852
	,{FPLL_t(3, 428, 1, 1)}		// 856
	,{FPLL_t(3, 430, 1, 1)}		// 860
	,{FPLL_t(3, 432, 1, 1)}		// 864
	,{FPLL_t(3, 434, 1, 1)}		// 868
	,{FPLL_t(3, 436, 1, 1)}		// 872
	,{FPLL_t(3, 438, 1, 1)}		// 876
	,{FPLL_t(3, 440, 1, 1)}		// 880
	,{FPLL_t(3, 442, 1, 1)}		// 884
	,{FPLL_t(3, 444, 1, 1)}		// 888
	,{FPLL_t(3, 446, 1, 1)}		// 892
	,{FPLL_t(3, 448, 1, 1)}		// 896
	,{FPLL_t(3, 450, 1, 1)}		// 900
	,{FPLL_t(3, 454, 1, 1)}		// 908
	,{FPLL_t(3, 458, 1, 1)}		// 916
	,{FPLL_t(3, 462, 1, 1)}		// 924
	,{FPLL_t(3, 466, 1, 1)}		// 932
	,{FPLL_t(3, 470, 1, 1)}		// 940
	,{FPLL_t(3, 474, 1, 1)}		// 948
	,{FPLL_t(3, 478, 1, 1)}		// 956
	,{FPLL_t(3, 482, 1, 1)}		// 964
	,{FPLL_t(3, 486, 1, 1)}		// 972
	,{FPLL_t(3, 490, 1, 1)}		// 980
	,{FPLL_t(3, 494, 1, 1)}		// 988
	,{FPLL_t(3, 498, 1, 1)}		// 996
	,{FPLL_t(3, 251, 0, 0)}		// 1004
	,{FPLL_t(3, 253, 0, 0)}		// 1012
	,{FPLL_t(3, 255, 0, 0)}		// 1020
	,{FPLL_t(3, 257, 0, 0)}		// 1028
	,{FPLL_t(3, 259, 0, 0)}		// 1036
	,{FPLL_t(3, 261, 0, 0)}		// 1044
	,{FPLL_t(3, 263, 0, 0)}		// 1052
	,{FPLL_t(3, 265, 0, 0)}		// 1060
	,{FPLL_t(3, 267, 0, 0)}		// 1068
	,{FPLL_t(3, 269, 0, 0)}		// 1076
	,{FPLL_t(3, 271, 0, 0)}		// 1084
	,{FPLL_t(3, 273, 0, 0)}		// 1092
	,{FPLL_t(3, 275, 0, 0)}		// 1100
	,{FPLL_t(3, 277, 0, 0)}		// 1108
	,{FPLL_t(3, 279, 0, 0)}		// 1116
	,{FPLL_t(3, 281, 0, 0)}		// 1124
	,{FPLL_t(3, 283, 0, 0)}		// 1132
	,{FPLL_t(3, 285, 0, 0)}		// 1140
	,{FPLL_t(3, 287, 0, 0)}		// 1148
	,{FPLL_t(3, 289, 0, 0)}		// 1156
	,{FPLL_t(3, 291, 0, 0)}		// 1164
	,{FPLL_t(3, 293, 0, 0)}		// 1172
	,{FPLL_t(3, 295, 0, 0)}		// 1180
	,{FPLL_t(3, 297, 0, 0)}		// 1188
	,{FPLL_t(3, 299, 0, 0)}		// 1196
	,{FPLL_t(3, 300, 0, 0)}		// 1200
	,{FPLL_t(3, 301, 0, 0)}		// 1204
	,{FPLL_t(3, 303, 0, 0)}		// 1212
	,{FPLL_t(3, 305, 0, 0)}		// 1220
	,{FPLL_t(3, 307, 0, 0)}		// 1228
	,{FPLL_t(3, 309, 0, 0)}		// 1236
	,{FPLL_t(3, 311, 0, 0)}		// 1244
	,{FPLL_t(3, 313, 0, 0)}		// 1252
	,{FPLL_t(3, 315, 0, 0)}		// 1260
	,{FPLL_t(3, 317, 0, 0)}		// 1268
	,{FPLL_t(3, 319, 0, 0)}		// 1276
	,{FPLL_t(3, 321, 0, 0)}		// 1284
	,{FPLL_t(3, 323, 0, 0)}		// 1292
	,{FPLL_t(3, 325, 0, 0)}		// 1300
	,{FPLL_t(3, 327, 0, 0)}		// 1308
	,{FPLL_t(3, 329, 0, 0)}		// 1316
	,{FPLL_t(3, 331, 0, 0)}		// 1324
	,{FPLL_t(3, 333, 0, 0)}		// 1332
	,{FPLL_t(3, 335, 0, 0)}		// 1340
	,{FPLL_t(3, 337, 0, 0)}		// 1348
	,{FPLL_t(3, 339, 0, 0)}		// 1356
	,{FPLL_t(3, 341, 0, 0)}		// 1364
	,{FPLL_t(3, 343, 0, 0)}		// 1372
	,{FPLL_t(3, 345, 0, 0)}		// 1380
	,{FPLL_t(3, 347, 0, 0)}		// 1388
	,{FPLL_t(3, 349, 0, 0)}		// 1396
	,{FPLL_t(3, 351, 0, 1)}		// 1404
	,{FPLL_t(3, 353, 0, 1)}		// 1412
	,{FPLL_t(3, 355, 0, 1)}		// 1420
	,{FPLL_t(3, 357, 0, 1)}		// 1428
	,{FPLL_t(3, 359, 0, 1)}		// 1436
	,{FPLL_t(3, 361, 0, 1)}		// 1444
	,{FPLL_t(3, 363, 0, 1)}		// 1452
	,{FPLL_t(3, 365, 0, 1)}		// 1460
	,{FPLL_t(3, 367, 0, 1)}		// 1468
	,{FPLL_t(3, 369, 0, 1)}		// 1476
	,{FPLL_t(3, 371, 0, 1)}		// 1484
	,{FPLL_t(3, 373, 0, 1)}		// 1492
	,{FPLL_t(3, 375, 0, 1)}		// 1500
	,{FPLL_t(3, 377, 0, 1)}		// 1508
	,{FPLL_t(3, 379, 0, 1)}		// 1516
	,{FPLL_t(3, 381, 0, 1)}		// 1524
	,{FPLL_t(3, 383, 0, 1)}		// 1532
	,{FPLL_t(3, 385, 0, 1)}		// 1540
	,{FPLL_t(3, 387, 0, 1)}		// 1548
	,{FPLL_t(3, 389, 0, 1)}		// 1556
	,{FPLL_t(3, 391, 0, 1)}		// 1564
	,{FPLL_t(3, 393, 0, 1)}		// 1572
	,{FPLL_t(3, 395, 0, 1)}		// 1580
	,{FPLL_t(3, 397, 0, 1)}		// 1588
	,{FPLL_t(3, 399, 0, 1)}		// 1596
	,{FPLL_t(3, 401, 0, 1)}		// 1604
	,{FPLL_t(3, 403, 0, 1)}		// 1612
	,{FPLL_t(3, 405, 0, 1)}		// 1620
	,{FPLL_t(3, 407, 0, 1)}		// 1628
	,{FPLL_t(3, 409, 0, 1)}		// 1636
	,{FPLL_t(3, 411, 0, 1)}		// 1644
	,{FPLL_t(3, 413, 0, 1)}		// 1652
	,{FPLL_t(3, 415, 0, 1)}		// 1660
	,{FPLL_t(3, 417, 0, 1)}		// 1668
	,{FPLL_t(3, 419, 0, 1)}		// 1676
	,{FPLL_t(3, 421, 0, 1)}		// 1684
	,{FPLL_t(3, 423, 0, 1)}		// 1692
	,{FPLL_t(3, 425, 0, 1)}		// 1700
	,{FPLL_t(3, 427, 0, 1)}		// 1708
	,{FPLL_t(3, 429, 0, 1)}		// 1716
	,{FPLL_t(3, 431, 0, 1)}		// 1724
	,{FPLL_t(3, 433, 0, 1)}		// 1732
	,{FPLL_t(3, 435, 0, 1)}		// 1740
	,{FPLL_t(3, 437, 0, 1)}		// 1748
	,{FPLL_t(3, 439, 0, 1)}		// 1756
	,{FPLL_t(3, 441, 0, 1)}		// 1764
	,{FPLL_t(3, 443, 0, 1)}		// 1772
	,{FPLL_t(3, 445, 0, 1)}		// 1780
	,{FPLL_t(3, 447, 0, 1)}		// 1788
	,{FPLL_t(3, 449, 0, 1)}		// 1796
	,{FPLL_t(3, 451, 0, 1)}		// 1804
	,{FPLL_t(3, 453, 0, 1)}		// 1812
	,{FPLL_t(3, 455, 0, 1)}		// 1820
	,{FPLL_t(3, 457, 0, 1)}		// 1828
	,{FPLL_t(3, 459, 0, 1)}		// 1836
	,{FPLL_t(3, 461, 0, 1)}		// 1844
	,{FPLL_t(3, 463, 0, 1)}		// 1852
	,{FPLL_t(3, 465, 0, 1)}		// 1860
	,{FPLL_t(3, 467, 0, 1)}		// 1868
	,{FPLL_t(3, 469, 0, 1)}		// 1876
	,{FPLL_t(3, 471, 0, 1)}		// 1884
	,{FPLL_t(3, 473, 0, 1)}		// 1892
	,{FPLL_t(3, 475, 0, 1)}		// 1900
	,{FPLL_t(3, 477, 0, 1)}		// 1908
	,{FPLL_t(3, 479, 0, 1)}		// 1916
	,{FPLL_t(3, 481, 0, 1)}		// 1924
	,{FPLL_t(3, 483, 0, 1)}		// 1932
	,{FPLL_t(3, 485, 0, 1)}		// 1940
	,{FPLL_t(3, 487, 0, 1)}		// 1948
	,{FPLL_t(3, 489, 0, 1)}		// 1956
	,{FPLL_t(3, 491, 0, 1)}		// 1964
	,{FPLL_t(3, 493, 0, 1)}		// 1972
	,{FPLL_t(3, 495, 0, 1)}		// 1980
	,{FPLL_t(3, 497, 0, 1)}		// 1988
	,{FPLL_t(3, 499, 0, 1)}		// 1996
	,{FPLL_t(3, 500, 0, 1)}		// 2000
};
sfPLL	pIO_CKC_PLL345[]	=
{
	 {FPLL_t(3, 128, 5, 1)}		// 16
	,{FPLL_t(3,  99, 4, 0)}		// 24.75
	,{FPLL_t(3, 140, 3, 1)}		// 70
	,{FPLL_t(3, 144, 3, 1)}		// 72
	,{FPLL_t(3,  86, 2, 0)}		// 86
	,{FPLL_t(3, 101, 2, 0)}		// 101
	,{FPLL_t(3, 132, 2, 1)}		// 132
	,{FPLL_t(3, 146, 2, 1)}		// 146
	,{FPLL_t(3,  90, 1, 0)}		// 180
	,{FPLL_t(3,  92, 1, 0)}		// 184
	,{FPLL_t(3,  95, 1, 0)}		// 190
	,{FPLL_t(3,  96, 1, 0)}		// 192
	,{FPLL_t(3, 102, 1, 0)}		// 204
	,{FPLL_t(3, 104, 1, 0)}		// 208
	,{FPLL_t(3, 105, 1, 0)}		// 210
	,{FPLL_t(4, 142, 1, 0)}		// 213
	,{FPLL_t(3, 107, 1, 0)}		// 214
	,{FPLL_t(3, 108, 1, 0)}		// 216
	,{FPLL_t(3, 111, 1, 0)}		// 222
	,{FPLL_t(3, 113, 1, 0)}		// 226
	,{FPLL_t(3, 114, 1, 0)}		// 228
	,{FPLL_t(3, 115, 1, 0)}		// 230
	,{FPLL_t(3, 117, 1, 1)}		// 234
	,{FPLL_t(3, 118, 1, 1)}		// 236
	,{FPLL_t(3, 122, 1, 1)}		// 244
	,{FPLL_t(3, 123, 1, 1)}		// 246
	,{FPLL_t(3, 124, 1, 1)}		// 248
	,{FPLL_t(3, 126, 1, 1)}		// 252
	,{FPLL_t(3, 130, 1, 1)}		// 260
	,{FPLL_t(3, 131, 1, 1)}		// 262
	,{FPLL_t(3, 132, 1, 1)}		// 264
	,{FPLL_t(3, 133, 1, 1)}		// 266
	,{FPLL_t(3, 138, 1, 1)}		// 276
	,{FPLL_t(3, 141, 1, 1)}		// 282
	,{FPLL_t(3, 144, 1, 1)}		// 288
	,{FPLL_t(3, 145, 1, 1)}		// 290
	,{FPLL_t(3, 147, 1, 1)}		// 294
	,{FPLL_t(3, 152, 1, 1)}		// 304
	,{FPLL_t(3, 154, 1, 1)}		// 308
	,{FPLL_t(3, 156, 1, 1)}		// 312
	,{FPLL_t(3, 158, 1, 1)}		// 316
	,{FPLL_t(3, 160, 1, 1)}		// 320
	,{FPLL_t(3, 162, 1, 1)}		// 324
	,{FPLL_t(3, 156, 1, 1)}		// 312
	,{FPLL_t(3, 158, 1, 1)}		// 316
	,{FPLL_t(3, 160, 1, 1)}		// 320
	,{FPLL_t(3, 162, 1, 1)}		// 324
	,{FPLL_t(3, 164, 1, 1)}		// 328
	,{FPLL_t(3, 165, 1, 1)}		// 330
	,{FPLL_t(3,  83, 0, 0)}		// 332
	,{FPLL_t(3,  84, 0, 0)}		// 336
	,{FPLL_t(3,  85, 0, 0)}		// 340
	,{FPLL_t(3,  87, 0, 0)}		// 348
	,{FPLL_t(3,  88, 0, 0)}		// 352
	,{FPLL_t(3,  89, 0, 0)}		// 356
	,{FPLL_t(3,  90, 0, 0)}		// 360
	,{FPLL_t(6, 181, 0, 0)}		// 362
	,{FPLL_t(3,  91, 0, 0)}		// 364
	,{FPLL_t(4, 123, 0, 0)}		// 369
	,{FPLL_t(6, 185, 0, 0)}		// 370
	,{FPLL_t(3,  93, 0, 0)}		// 372
	,{FPLL_t(3,  95, 0, 0)}		// 380
	,{FPLL_t(4, 127, 0, 0)}		// 381
	,{FPLL_t(3,  96, 0, 0)}		// 384
	,{FPLL_t(3,  97, 0, 0)}		// 388
	,{FPLL_t(4, 130, 0, 0)}		// 390
	,{FPLL_t(3,  98, 0, 0)}		// 392
	,{FPLL_t(4, 131, 0, 0)}		// 393
	,{FPLL_t(3,  99, 0, 0)}		// 396
	,{FPLL_t(4, 133, 0, 0)}		// 399
	,{FPLL_t(3, 100, 0, 0)}		// 400
	,{FPLL_t(3, 101, 0, 0)}		// 404
	,{FPLL_t(5, 169, 0, 0)}		// 405.6
	,{FPLL_t(6, 203, 0, 0)}		// 406
	,{FPLL_t(3, 102, 0, 0)}		// 408
	,{FPLL_t(6, 205, 0, 0)}		// 410
	,{FPLL_t(3, 103, 0, 0)}		// 412
	,{FPLL_t(3, 104, 0, 0)}		// 416
	,{FPLL_t(5, 174, 0, 0)}		// 417.6
	,{FPLL_t(3, 105, 0, 0)}		// 420
	,{FPLL_t(3, 106, 0, 0)}		// 424
	,{FPLL_t(3, 107, 0, 0)}		// 428
	,{FPLL_t(4, 143, 0, 0)}		// 429
	,{FPLL_t(6, 215, 0, 0)}		// 430
	,{FPLL_t(3, 108, 0, 0)}		// 432
	,{FPLL_t(3, 109, 0, 0)}		// 436
	,{FPLL_t(3, 110, 0, 0)}		// 440
	,{FPLL_t(6, 221, 0, 0)}		// 442
	,{FPLL_t(3, 111, 0, 0)}		// 444
	,{FPLL_t(3, 112, 0, 0)}		// 448
	,{FPLL_t(4, 150, 0, 0)}		// 450
	,{FPLL_t(3, 113, 0, 0)}		// 452
	,{FPLL_t(6, 227, 0, 0)}		// 454
	,{FPLL_t(3, 114, 0, 0)}		// 456
	,{FPLL_t(3, 115, 0, 0)}		// 460
	,{FPLL_t(5, 193, 0, 1)}		// 463.2
	,{FPLL_t(3, 116, 0, 1)}		// 464
	,{FPLL_t(6, 233, 0, 1)}		// 466
	,{FPLL_t(3, 117, 0, 1)}		// 468
	,{FPLL_t(6, 235, 0, 1)}		// 470
	,{FPLL_t(3, 118, 0, 1)}		// 472
	,{FPLL_t(4, 158, 0, 1)}		// 474
	,{FPLL_t(3, 119, 0, 1)}		// 476
	,{FPLL_t(3, 120, 0, 1)}		// 480
	,{FPLL_t(3, 121, 0, 1)}		// 484
	,{FPLL_t(4, 162, 0, 1)}		// 486
	,{FPLL_t(3, 122, 0, 1)}		// 488
	,{FPLL_t(6, 245, 0, 1)}		// 490
	,{FPLL_t(3, 123, 0, 1)}		// 492
	,{FPLL_t(3, 124, 0, 1)}		// 496
	,{FPLL_t(5, 207, 0, 1)}		// 496.8
	,{FPLL_t(4, 166, 0, 1)}		// 498
	,{FPLL_t(3, 125, 0, 1)}		// 500
	,{FPLL_t(3, 126, 0, 1)}		// 504
	,{FPLL_t(3, 127, 0, 1)}		// 508
	,{FPLL_t(3, 128, 0, 1)}		// 512
	,{FPLL_t(3, 129, 0, 1)}		// 516
	,{FPLL_t(3, 130, 0, 1)}		// 520
	,{FPLL_t(3, 131, 0, 1)}		// 524
	,{FPLL_t(3, 132, 0, 1)}		// 528
	,{FPLL_t(3, 133, 0, 1)}		// 532
	,{FPLL_t(3, 134, 0, 1)}		// 536
	,{FPLL_t(3, 135, 0, 1)}		// 540
	,{FPLL_t(3, 136, 0, 1)}		// 544
	,{FPLL_t(3, 137, 0, 1)}		// 548
	,{FPLL_t(3, 138, 0, 1)}		// 552
	,{FPLL_t(3, 139, 0, 1)}		// 556
	,{FPLL_t(3, 140, 0, 1)}		// 560
	,{FPLL_t(3, 141, 0, 1)}		// 564
	,{FPLL_t(3, 142, 0, 1)}		// 568
	,{FPLL_t(3, 143, 0, 1)}		// 572
	,{FPLL_t(3, 144, 0, 1)}		// 576
	,{FPLL_t(3, 145, 0, 1)}		// 580
	,{FPLL_t(3, 146, 0, 1)}		// 584
	,{FPLL_t(3, 147, 0, 1)}		// 588
	,{FPLL_t(3, 148, 0, 1)}		// 592
	,{FPLL_t(3, 149, 0, 1)}		// 596
	,{FPLL_t(3, 150, 0, 1)}		// 600
	,{FPLL_t(3, 151, 0, 1)}		// 604
	,{FPLL_t(3, 152, 0, 1)}		// 608
	,{FPLL_t(3, 153, 0, 1)}		// 612
	,{FPLL_t(3, 154, 0, 1)}		// 616
	,{FPLL_t(3, 155, 0, 1)}		// 620
	,{FPLL_t(3, 156, 0, 1)}		// 624
	,{FPLL_t(3, 157, 0, 1)}		// 628
	,{FPLL_t(3, 158, 0, 1)}		// 632
	,{FPLL_t(3, 159, 0, 1)}		// 636
	,{FPLL_t(4, 213, 0, 1)}		// 639
	,{FPLL_t(3, 160, 0, 1)}		// 640
	,{FPLL_t(3, 161, 0, 1)}		// 644
	,{FPLL_t(3, 162, 0, 1)}		// 648
	,{FPLL_t(3, 163, 0, 1)}		// 652
	,{FPLL_t(3, 164, 0, 1)}		// 656
	,{FPLL_t(3, 165, 0, 1)}		// 660
};


#define NUM_PLL012 				(sizeof(pIO_CKC_PLL012)/sizeof(sfPLL))
#define NUM_PLL345 				(sizeof(pIO_CKC_PLL345)/sizeof(sfPLL))


#define tca_wait()				{ volatile int i; for (i=0; i<0x2000; i++); }

#if defined(_LINUX_)
    #define iomap_p2v(x)            io_p2v(x)
#else
	#define iomap_p2v(x)			(OALPAtoVA(x,FALSE))
#endif

/****************************************************************************************
* Global Variable
* ***************************************************************************************/
PCKC	pCKC ;
PPMU	pPMU ; 
PIOBUSCFG pIOBUSCFG;


/****************************************************************************************
* FUNCTION :void tca_ckc_init(void)
* DESCRIPTION :
* ***************************************************************************************/
VOLATILE void tca_ckc_init(void)
{
	pCKC = (PCKC)(iomap_p2v((unsigned int)&HwCLK_BASE)); 
	pPMU = (PPMU)(iomap_p2v((unsigned int)&HwPMU_BASE)); 
	pIOBUSCFG = (PIOBUSCFG)(iomap_p2v((unsigned int)&HwIOBUSCFG_BASE));
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
		case 4:
			tPLLCFG = pCKC->PLL4CFG;
			break;
		case 5:
			tPLLCFG = pCKC->PLL5CFG;
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
* FUNCTION :unsigned int tca_ckc_getdividpll(unsigned int ch)
* DESCRIPTION :
* ***************************************************************************************/
volatile unsigned int tca_ckc_getdividpll(unsigned int ch)
{
	volatile unsigned int tDIVPLL;
	volatile unsigned int tPLL = tca_ckc_getpll(ch);
	unsigned int uiPDIV;
		
	switch(ch)
	{
		case 0:
			uiPDIV = (pCKC->CLKDIVC & (Hw30-Hw24))>>24;
			break;
		case 1:
			uiPDIV = (pCKC->CLKDIVC & (Hw22-Hw16))>>16;
			break;
		case 2:
			uiPDIV = (pCKC->CLKDIVC & (Hw14-Hw8))>>8;
			break;
		case 3:
			uiPDIV = (pCKC->CLKDIVC & (Hw6-Hw0));
			break;
		case 4:
			uiPDIV = (pCKC->CLKDIVC2 & (Hw30-Hw24))>>24;
			break;
		case 5:
			uiPDIV = (pCKC->CLKDIVC2 & (Hw22-Hw16))>>16;
			break;
	}

	//Fdivpll Clock
	tDIVPLL = (unsigned int)tPLL/(uiPDIV+1);
	
	return tDIVPLL;
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
		case DIRECTPLL0 :
			lclksource =  tca_ckc_getpll(0);
			break;
		case DIRECTPLL1 :
			lclksource =  tca_ckc_getpll(1);
			break;
		case DIRECTPLL2 :
			lclksource =  tca_ckc_getpll(2);
			break;
		case DIRECTPLL3 :
			lclksource =  tca_ckc_getpll(3);
			break;
		case DIRECTXIN:
			lclksource =  120000;
			break;
		case DIVIDPLL0:
			lclksource = tca_ckc_getdividpll(0);
			break;
		case DIVIDPLL1:
			lclksource = tca_ckc_getdividpll(1);
			break;			
		case DIRECTXTIN:
			lclksource =  120000;
			break;
		case DIRECTPLL4:
			lclksource =  tca_ckc_getpll(4);
			break;
		case DIRECTPLL5:
			lclksource =  tca_ckc_getpll(5);
			break;
		case DIVIDPLL2:
			lclksource = tca_ckc_getdividpll(2);
			break;
		case DIVIDPLL3:
			lclksource = tca_ckc_getdividpll(3);
			break;
		case DIVIDPLL4:
			lclksource = tca_ckc_getdividpll(4);
			break;
		case DIVIDPLL5:
			lclksource = tca_ckc_getdividpll(5);
			break;
		/*
		case DIVIDXIN:
			break;
		case DIVIDXTIN:
			break;
		*/
		default : 
			lclksource =  tca_ckc_getpll(1);
			break;
	}
	
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
	//Check synchronous clock mode
	if(!(pCKC->MBUSCTRL & Hw0))
	{
		//synchronous clock mode
		lconfig = ((pCKC->MBUSCTRL & (Hw20-Hw8))>>8);
		lclksource = tca_ckc_getcpu();
		lbus = lclksource /(lconfig+1);
		return lbus;		
	}
	
	lconfig = ((pCKC->CLK2CTRL & (Hw8-Hw4))>>4);

	switch(pCKC->CLK2CTRL & (Hw4-Hw0)) // Check Memory Bus Source
	{
		case DIRECTPLL0 :
			lclksource =  tca_ckc_getpll(0);
			break;
		case DIRECTPLL1 :
			lclksource =  tca_ckc_getpll(1);
			break;
		case DIRECTPLL2 :
			lclksource =  tca_ckc_getpll(2);
			break;
		case DIRECTPLL3 :
			lclksource =  tca_ckc_getpll(3);
			break;
		case DIRECTXIN:
			lclksource =  120000;
			break;
		case DIVIDPLL0:
			lclksource = tca_ckc_getdividpll(0);
			break;
		case DIVIDPLL1:
			lclksource = tca_ckc_getdividpll(1);
			break;			
		case DIRECTXTIN:
			lclksource =  120000;
			break;
#if defined(TCC9300)
		case DIRECTPLL4:
			lclksource =  tca_ckc_getpll(4);
			break;
		case DIRECTPLL5:
			lclksource =  tca_ckc_getpll(5);
			break;
		case DIVIDPLL2:
			lclksource = tca_ckc_getdividpll(2);
			break;
		case DIVIDPLL3:
			lclksource = tca_ckc_getdividpll(3);
			break;
		case DIVIDPLL4:
			lclksource = tca_ckc_getdividpll(4);
			break;
		case DIVIDPLL5:
			lclksource = tca_ckc_getdividpll(5);
			break;
		/*
		case DIVIDXIN:
			break;
		case DIVIDXTIN:
			break;
		*/
#endif
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
VOLATILE static unsigned int tca_ckc_gclkctrlx(unsigned int isenable,unsigned int config,unsigned int sel)
{
	unsigned int retVal = 0;
	retVal = ((isenable?1:0)<<21)|(config<<4)|(sel<<0);

	
	return retVal;
}

/****************************************************************************************
* FUNCTION :static unsigned int tca_ckc_clkctrly(unsigned int isenable,unsigned int md,unsigned int config,unsigned int sel)
* DESCRIPTION : ctrl 0 and ctrl 2 (CPU and BUS CTRL)
*				config is divider (md = 0)
* ***************************************************************************************/
VOLATILE static unsigned int tca_ckc_gclkctrly(unsigned int isenable,unsigned int config,unsigned int sel, unsigned int ch)
{
	unsigned int retVal = 0;

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
		//cpu
		retVal = (pCKC->CLK0CTRL & (Hw20-Hw4));
			
		retVal |= ((isenable?1:0)<<21)|(sel<<0);

	}
	else
		retVal = ((isenable?1:0)<<21)|(config<<4)|(sel<<0);
	
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
		case DIRECTPLL4:
			clksource =  tca_ckc_getpll(4);
			break;
		case DIRECTPLL5:
			clksource =  tca_ckc_getpll(5);
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


	if(clkname == CLKCTRL0 || clkname == CLKCTRL2)
	{
		*pCLKCTRL = tca_ckc_gclkctrly(isenable,clkdiv,sor,clkname);
	}
	else
	{
		if(isenable == DISABLE)
			*pCLKCTRL &= ~Hw21;
		else if (isenable == NOCHANGE)
			*pCLKCTRL = (*pCLKCTRL&(1<<21))|(clkdiv<<4)|(sor<<0);
		else
			*pCLKCTRL = tca_ckc_gclkctrlx(isenable,clkdiv,sor);
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
	lsel = ((*pCLKCTRL) & 0xF);
	
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
			case DIRECTPLL4:
				clksource =  tca_ckc_getpll(4);
				break;
			case DIRECTPLL5:
				clksource =  tca_ckc_getpll(5);
				break;
			default : 
				clksource =  tca_ckc_getpll(1);
				break;
		}

	}
	else
		return 0;
			
	return (clksource / (lconfig+1));
}

/****************************************************************************************
* FUNCTION :static unsigned int tca_ckc_setpllxcfg(unsigned int isEnable, int P, int M, int S)
* DESCRIPTION :
* ***************************************************************************************/
VOLATILE static unsigned int tca_ckc_gpllxcfg(unsigned int isenable, unsigned int p, unsigned int m, unsigned int s, unsigned int vsel)
{
	unsigned int retVal = Hw31;//Disable
	
	if(isenable > 0)
	{
		retVal = (s<<24)|(m<<8)|(p<<0);
		retVal |= (vsel<<30);
		retVal |= Hw31;	//Enable
	}

	return retVal;
}

/****************************************************************************************
* FUNCTION :static void tca_ckc_pll(unsigned int p, unsigned int m, unsigned int s,unsigned int ch)
* DESCRIPTION :
* ***************************************************************************************/
VOLATILE static void tca_ckc_pll(unsigned int p, unsigned int m, unsigned int s, unsigned int vsel, unsigned int ch)
{
	volatile unsigned	*pPLLCFG;

	pPLLCFG =(volatile unsigned *)((&pCKC->PLL0CFG)+ch);	

	if(ch == 0) // PLL0 is System Clock Source
	{
		// Change System Clock Souce --> XIN (12Mhz)
		pCKC->CLK7CTRL = tca_ckc_gclkctrly(ENABLE,CLKDIV2,DIRECTXIN,CLKCTRL7);
		pCKC->CLK0CTRL = tca_ckc_gclkctrly(ENABLE,CLKDIVNONCHANGE,DIRECTPLL2,CLKCTRL0);
		tca_wait(); 
	}
	
	//Disable PLL
	*pPLLCFG &= ~Hw31;
	//Set PMS
	*pPLLCFG = tca_ckc_gpllxcfg(ENABLE,p,m,s, vsel);
	//Enable PLL
	*pPLLCFG |= Hw31;
	tca_wait(); 
	//Restore System Clock Source
	if(ch == 0)
	{
		pCKC->CLK0CTRL = tca_ckc_gclkctrly(ENABLE,CLKDIVNONCHANGE,DIRECTPLL0,CLKCTRL0);
		pCKC->CLK7CTRL = tca_ckc_gclkctrly(ENABLE,CLKDIV4,DIRECTPLL0,7);
	}
	
}


/****************************************************************************************
* FUNCTION :void tca_ckc_validpll(unsigned int * pvalidpll)
* DESCRIPTION :
* ***************************************************************************************/
VOLATILE void tca_ckc_validpll(unsigned int * pvalidpll012, unsigned int * pvalidpll345)
{
	unsigned int uCnt;
	sfPLL		*pPLL012, *pPLL345;

	pPLL012	= &pIO_CKC_PLL012[0];
	for (uCnt = 0; uCnt < NUM_PLL012; uCnt ++, pPLL012 ++)
	{
		*pvalidpll012 = pPLL012->uFpll ;	

		pvalidpll012++;
	}

	pPLL345	= &pIO_CKC_PLL345[0];
	for (uCnt = 0; uCnt < NUM_PLL345; uCnt ++, pPLL345 ++)
	{
		*pvalidpll345 = pPLL345->uFpll ;	

		pvalidpll345++;
	}
}

/****************************************************************************************
* FUNCTION :int tca_ckc_setpll(unsigned int pll, unsigned int ch)
* DESCRIPTION :
* ***************************************************************************************/
VOLATILE int tca_ckc_setpll(unsigned int pll, unsigned int ch)
{
	unsigned	uCnt;
	int 		retVal = -1;
	

	sfPLL		*pPLL012, *pPLL345;
	if(pll != 0)
	{
		if(ch <= 2)
		{
			pPLL012 = &pIO_CKC_PLL012[0];
			for (uCnt = 0; uCnt < NUM_PLL012; uCnt ++, pPLL012++)
				if (pPLL012->uFpll == pll)
					break;
			
			if (uCnt < NUM_PLL012)
			{
				tca_ckc_pll(pPLL012->P,pPLL012->M ,pPLL012->S, pPLL012->VSEL, ch);
				retVal = 0;
				return 1;
			}
		}
		else
		{
			pPLL345 = &pIO_CKC_PLL345[0];
			for (uCnt = 0; uCnt < NUM_PLL345; uCnt ++, pPLL345++)
				if (pPLL345->uFpll >= pll)	// SangWon, change "==" to ">="
					break;
			
			if (uCnt < NUM_PLL345)
			{
				tca_ckc_pll(pPLL345->P,pPLL345->M ,pPLL345->S, pPLL345->VSEL, ch);
				retVal = 0;
				return 1;
			}
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
}
/****************************************************************************************
* FUNCTION :void tca_ckc_setcpu(unsigned int n)
* DESCRIPTION :  n is n/16 
* example : CPU == PLL : n=16 - CPU == PLL/2 : n=8
* ***************************************************************************************/
VOLATILE void tca_ckc_setcpuXIN(unsigned int n)
{
	 unsigned int lckc0ctrl;	
	 unsigned int lindex[] = {0x0,0x8000,0x8008,0x8808,0x8888,0xA888,0xA8A8,0xAAA8,0xAAAA,
							0xECCC,0xEECC,0xEEEC,0xEEEE,0xFEEE,0xFFEE,0xFFFE,0xFFFF};


	lckc0ctrl = pCKC->CLK0CTRL;
	lckc0ctrl &= ~(Hw20-Hw0);
	lckc0ctrl |= (lindex[n] << 4)|Hw2;

	pCKC->CLK0CTRL = lckc0ctrl;
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
	unsigned int retVal = 0;
	retVal = Hw0<<periname;

	if(isenable)
		pPMU->PWROFF |= (retVal);
	else
		pPMU->PWROFF &= ~(retVal);	
}

/****************************************************************************************
* FUNCTION :void tca_ckc_getpmupwroff( unsigned int pmuoffname)
* DESCRIPTION : 
* ***************************************************************************************/
VOLATILE int tca_ckc_getpmupwroff( unsigned int pmuoffname)
{
	unsigned int retVal = 0;
	retVal =  (pPMU->PWROFF >> pmuoffname)  & Hw0;	

	return retVal;
}

/****************************************************************************************
* FUNCTION :static unsigned int tca_ckc_setpckxxx(unsigned int isenable, unsigned int sel, unsigned int div)
* DESCRIPTION : 
* ***************************************************************************************/
VOLATILE static unsigned int tca_ckc_gpckxxx(unsigned int isenable, unsigned int sel, unsigned int div)
{
	return (((isenable?1:0)<<29)|(sel<<24)|(div<<0));
}

/****************************************************************************************
* FUNCTION :static unsigned int tca_ckc_setpckyyy(unsigned int isenable, unsigned int sel, unsigned int div)
* DESCRIPTION : md (1: divider Mode, 0:DCO Mode)
* ***************************************************************************************/
VOLATILE static unsigned int tca_ckc_gpckyyy(unsigned int isenable, unsigned int md, unsigned int sel, unsigned int div)
{
	return ((md<<31)|((isenable?1:0)<<29)|(sel<<24)|(div<<0));
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
		case PCDIVIDPLL0:
			lclksource =  tca_ckc_getdividpll(0);
			break;
		case PCDIVIDPLL1:
			lclksource =  tca_ckc_getdividpll(1);
			break;
		case PCDIVIDPLL2:
			lclksource =  tca_ckc_getdividpll(2);
			break;
		case PCDIVIDPLL3:
			lclksource =  tca_ckc_getdividpll(3);
			break;
		/*
		case PCDIRECTXTIN:
			break;
		case PCEXITERNAL:
			break;
		case PCDIVIDXIN_HDMITMDS:
			break;
		case PCDIVIDXTIN_HDMIPCLK:
			break;
		*/
		case PCHDMI :
			lclksource =  270000;
			break;
		case PCSATA :
			lclksource =  250000;		
			break;
		case PCUSBPHY:
			lclksource =  480000;		
			break;
		/*
		case PCDIVIDXIN:
			break;
		case PCDIVIDXTIN:
			break;
		*/
		case PCDIRECTPLL4:
			lclksource =  tca_ckc_getpll(4);
			break;
		case PCDIRECTPLL5:
			lclksource =  tca_ckc_getpll(5);
			break;
		case PCDIVIDPLL4:
			lclksource =  tca_ckc_getdividpll(4);
			break;
		case PCDIVIDPLL5:
			lclksource =  tca_ckc_getdividpll(5);
			break;
		case PCUSB1PHY:
			lclksource =  480000;
			break;
		/*
		case PCMIPI_PLL:
			break;
		*/
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
	//TCC9300 SPDIF does not support DCO mode
	if(periname == PERI_ADC ||periname == PERI_AUD || periname == PERI_AUD1)
	{
		if(periname == PERI_AUD || periname == PERI_AUD1)
		{
			clkmode = 0;	// DCO Mode

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
		else if(lclksrc == PCDIRECTPLL4){
			return (tca_ckc_getpll(4)/(ldiv+1));
		}
		else if(lclksrc == PCDIRECTPLL5){
			return (tca_ckc_getpll(5)/(ldiv+1));
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
	lclksrc = (lreg&0x1F000000)>>24;
	
	if(periname == PERI_ADC || periname == PERI_AUD || periname == PERI_AUD1)
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

	lrb_min = RB_PREFETCHWRITEBUFFER;
	lrb_max = RB_ALLPERIPERALS;
	lrb_seperate = RB_GPSBCONTROLLER0;

	if(sel <  lrb_min || sel >= lrb_max)
	{
		return 0;
	}
	if(sel >  lrb_seperate)
	{
		sel -=	(lrb_seperate+1);
		
		if(mode)
			pIOBUSCFG->SWRESET1 &= ~lindex[sel];		
		else
			pIOBUSCFG->SWRESET1 |= lindex[sel];
	}
	else
	{
		if(mode)
			pIOBUSCFG->SWRESET0 &= ~lindex[sel];
		else
			pIOBUSCFG->SWRESET0 |= lindex[sel];
	}
	return 1;
}

VOLATILE void tca_ckc_setioswreset(unsigned int sel, unsigned int mode)
{
	unsigned int lindex[] = {Hw0,Hw1,Hw2,Hw3,Hw4,Hw5,Hw6,Hw7,Hw8,Hw9,Hw10,Hw11,Hw12,Hw13,Hw14,Hw15
							,Hw16,Hw17,Hw18,Hw19,Hw20,Hw21,Hw22,Hw23,Hw24,Hw25,Hw26,Hw27,Hw28,Hw29,Hw30,Hw31};

	unsigned int lrb_min;
	unsigned int lrb_max;
	unsigned int lrb_seperate;

	lrb_min = RB_PREFETCHWRITEBUFFER;
	lrb_max = RB_ALLPERIPERALS;
	lrb_seperate = RB_GPSBCONTROLLER0;

	if(sel <  lrb_min || sel >= lrb_max)
	{
		return 0;
	}
	if(sel >  lrb_seperate)
	{
		sel -=	(lrb_seperate+1);
		
		if(mode)
			pIOBUSCFG->SWRESET1 |= lindex[sel];		
		else
			pIOBUSCFG->SWRESET1 &= ~lindex[sel];
	}
	else
	{
		if(mode)
			pIOBUSCFG->SWRESET0 |= lindex[sel];
		else
			pIOBUSCFG->SWRESET0 &= ~lindex[sel];
	}
	return 1;
}

/****************************************************************************************
* FUNCTION :void tca_ckc_setswreset(unsigned int lfbusname, unsigned int mode)
* DESCRIPTION : 
* ***************************************************************************************/

VOLATILE void  tca_ckc_setswreset(unsigned int lfbusname, unsigned int mode)
{
	unsigned int hIndex[] = {Hw0,Hw1,Hw2,Hw3,Hw4,Hw5,Hw6,Hw7,Hw8,Hw9,Hw10};

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

	lrb_min = RB_PREFETCHWRITEBUFFER;
	lrb_max = RB_ALLPERIPERALS;
	lrb_seperate = RB_GPSBCONTROLLER0;

	if(sel <  lrb_min || sel >=  lrb_max)
	{
		return -1;
	}

	if(sel >  lrb_seperate)
	{
		sel -=	(lrb_seperate+1);
		
		if(mode)
			pIOBUSCFG->HCLKMASK1 &= ~lindex[sel];		
		else
			pIOBUSCFG->HCLKMASK1 |= lindex[sel];
	}
	else
	{
		if(mode)
			pIOBUSCFG->HCLKMASK0 &= ~lindex[sel];
		else
			pIOBUSCFG->HCLKMASK0 |= lindex[sel];
	}

	return 1;
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
	
	lrb_min = RB_PREFETCHWRITEBUFFER;
	lrb_max = RB_ALLPERIPERALS;
	lrb_seperate = RB_GPSBCONTROLLER0;
	
	if(sel <  lrb_min || sel >=  lrb_max)
	{
		return -1;
	}
	if(sel >  lrb_seperate)
	{
		sel -=	(lrb_seperate+1);
		
		lretVal = (pIOBUSCFG->HCLKMASK1  & lindex[sel]) ;
	}
	else
	{
		lretVal = (pIOBUSCFG->HCLKMASK0  & lindex[sel]) ;
	}

	if(lretVal == 0)
		lretVal = 1;
	else
		lretVal = 0;
		
	return lretVal;
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

	lsel = (lclkctrl7 & (Hw4-Hw0));
	
	switch(lsel)
	{
		case DIRECTPLL0 :
			lclksource =  tca_ckc_getpll(0);
			break;
		case DIRECTPLL1 :
			lclksource =  tca_ckc_getpll(1);
			break;
		case DIRECTPLL2 :
			lclksource =  tca_ckc_getpll(2);
			break;
		case DIRECTPLL3 :
			lclksource =  tca_ckc_getpll(3);
			break;
		case DIRECTXIN:
			lclksource =  120000;
			break;
		case DIVIDPLL0:
			lclksource = tca_ckc_getdividpll(0);
			break;
		case DIVIDPLL1:
			lclksource = tca_ckc_getdividpll(1);
			break;			
		case DIRECTXTIN:
			lclksource =  120000;
			break;
		case DIRECTPLL4:
			lclksource =  tca_ckc_getpll(4);
			break;
		case DIRECTPLL5:
			lclksource =  tca_ckc_getpll(5);
			break;
		case DIVIDPLL2:
			lclksource = tca_ckc_getdividpll(2);
			break;
		case DIVIDPLL3:
			lclksource = tca_ckc_getdividpll(3);
			break;
		case DIVIDPLL4:
			lclksource = tca_ckc_getdividpll(4);
			break;
		case DIVIDPLL5:
			lclksource = tca_ckc_getdividpll(5);
			break;
		/*
		case DIVIDXIN:
			break;
		case DIVIDXTIN:
			break;
		*/
		default :
			lclksource = tca_ckc_getpll(1);
			break;
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

	lsel = (lclkctrl7 & 0xF);

	switch(lsel)
	{
		case DIRECTPLL0 :
			lclksource =  tca_ckc_getpll(0);
			break;
		case DIRECTPLL1 :
			lclksource =  tca_ckc_getpll(1);
			break;
		case DIRECTPLL2 :
			lclksource =  tca_ckc_getpll(2);
			break;
		case DIRECTPLL3 :
			lclksource =  tca_ckc_getpll(3);
			break;
		case DIRECTXIN:
			lclksource =  120000;
			break;
		case DIVIDPLL0:
			lclksource = tca_ckc_getdividpll(0);
			break;
		case DIVIDPLL1:
			lclksource = tca_ckc_getdividpll(1);
			break;			
		case DIRECTXTIN:
			lclksource =  120000;
			break;
		case DIRECTPLL4:
			lclksource =  tca_ckc_getpll(4);
			break;
		case DIRECTPLL5:
			lclksource =  tca_ckc_getpll(5);
			break;
		case DIVIDPLL2:
			lclksource = tca_ckc_getdividpll(2);
			break;
		case DIVIDPLL3:
			lclksource = tca_ckc_getdividpll(3);
			break;
		case DIVIDPLL4:
			lclksource = tca_ckc_getdividpll(4);
			break;
		case DIVIDPLL5:
			lclksource = tca_ckc_getdividpll(5);
			break;
		/*
		case DIVIDXIN:
			break;
		case DIVIDXTIN:
			break;
		*/
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

/****************************************************************************************
* FUNCTION : void tca_ckc_setgrppwdn(unsigned int lmode)
* DESCRIPTION : Power Down Register of GRPBUS 
* ***************************************************************************************/
VOLATILE void tca_ckc_setgrppwdn(unsigned int lpwdn , unsigned int lmode)
{
	PGRPBUSCONFIG lGRPPWDN;
	unsigned int lindex[] = {Hw0};

	lGRPPWDN = (PGRPBUSCONFIG)(iomap_p2v((unsigned int)&HwGRPBUSCONFIG_BASE));

	if (lmode)
		lGRPPWDN->GRPBUS_PWRDOWN &= ~lindex[lpwdn];
	else
		lGRPPWDN->GRPBUS_PWRDOWN |= lindex[lpwdn];
}
/****************************************************************************************
* FUNCTION : int tca_ckc_getgrppwdn(void)
* DESCRIPTION : Power Down Register of GRPBUS 
* ***************************************************************************************/
int tca_ckc_getgrppwdn(void)
{
	PGRPBUSCONFIG lGRPPWDN;

	lGRPPWDN = (PGRPBUSCONFIG)(iomap_p2v((unsigned int)&HwGRPBUSCONFIG_BASE));

	return (int)(lGRPPWDN->GRPBUS_PWRDOWN);
}

VOLATILE void tca_ckc_setgrpswreset(unsigned int lpwdn , unsigned int lmode)
{
	PGRPBUSCONFIG lGRPPWDN;
	unsigned int lindex[] = {Hw0};

	lGRPPWDN = (PGRPBUSCONFIG)(iomap_p2v((unsigned int)&HwGRPBUSCONFIG_BASE));

	if (lmode)
		lGRPPWDN->GRPBUS_SWRESET |= lindex[lpwdn];
	else
		lGRPPWDN->GRPBUS_SWRESET &= ~lindex[lpwdn];
}


/****************************************************************************************
* FUNCTION : void tca_ckc_setddipwdn(unsigned int lpwdn , unsigned int lmode)
* DESCRIPTION : Power Down Register of DDI_CONFIG 
* ***************************************************************************************/
VOLATILE void tca_ckc_setddipwdn(unsigned int lpwdn , unsigned int lmode)
{
	PDDICONFIG lDDIPWDN;
	unsigned int lindex[] = {Hw0,Hw1,Hw2,Hw3,Hw4,Hw5,Hw6,Hw7,Hw8,Hw9};	

	lDDIPWDN = (PDDICONFIG)(iomap_p2v((unsigned int)&HwDDI_CONFIG_BASE)); //0xF0400000

	if(lmode)  // Normal
		lDDIPWDN->PWDN &= ~lindex[lpwdn];
	else // Power Down
		lDDIPWDN->PWDN |= lindex[lpwdn];

}
/****************************************************************************************
* FUNCTION : int tca_ckc_getddipwdn(unsigned int lpwdn)
* DESCRIPTION : Power Down Register of DDI_CONFIG 
* ***************************************************************************************/
int tca_ckc_getddipwdn(unsigned int lpwdn)
{
	PDDICONFIG lDDIPWDN;
	unsigned int lindex[] = {Hw0,Hw1,Hw2,Hw3,Hw4,Hw5,Hw6,Hw7,Hw8,Hw9};

	lDDIPWDN = (PDDICONFIG)(iomap_p2v((unsigned int)&HwDDI_CONFIG_BASE)); //0xF0400000

	return (lDDIPWDN->PWDN &  lindex[lpwdn]);
}

VOLATILE void tca_ckc_setddiswreset(unsigned int lpwdn , unsigned int lmode)
{
	PDDICONFIG lDDIPWDN;
	unsigned int lindex[] = {Hw0,Hw1,Hw2,Hw3,Hw4,Hw5,Hw6,Hw7,Hw8,Hw9};	

	lDDIPWDN = (PDDICONFIG)(iomap_p2v((unsigned int)&HwDDI_CONFIG_BASE)); //0xF0400000

	if(lmode)  // reset
		lDDIPWDN->SWRESET |= lindex[lpwdn];
	else // normal
		lDDIPWDN->SWRESET &= ~lindex[lpwdn];

}

/****************************************************************************************
* FUNCTION : void tca_ckc_setvideobuscfg(unsigned int lpwdn , unsigned int lmode)
* DESCRIPTION : Power Down Register of Video Bus Config 
* ***************************************************************************************/
VOLATILE void tca_ckc_setvideobuscfgpwdn(unsigned int lpwdn , unsigned int lmode)
{
	PVIDEOBUSCFG pVIDEOBUSCFG;
	unsigned int lindex[] = {Hw0,Hw1,Hw2,Hw3};

	pVIDEOBUSCFG = (PVIDEOBUSCFG)(iomap_p2v((unsigned int)&HwVIDEOBUSCFG_BASE));

	if(lmode)  // Normal
		pVIDEOBUSCFG->PWDN &= ~lindex[lpwdn];
	else // Power Down
		pVIDEOBUSCFG->PWDN |= lindex[lpwdn];
	
}

/****************************************************************************************
* FUNCTION : int tca_ckc_getddipwdn(unsigned int lpwdn)
* DESCRIPTION : Power Down Register of Video Bus Config  
* ***************************************************************************************/
int tca_ckc_getvideobuscfgpwdn(unsigned int lpwdn)
{
	PVIDEOBUSCFG pVIDEOBUSCFG;
	unsigned int lindex[] = {Hw0,Hw1,Hw2,Hw3};

	pVIDEOBUSCFG = (PVIDEOBUSCFG)(iomap_p2v((unsigned int)&HwVIDEOBUSCFG_BASE));

	return (pVIDEOBUSCFG->PWDN &  lindex[lpwdn]);
}

/****************************************************************************************
* FUNCTION : void tca_ckc_setvideobuscfg(unsigned int lpwdn , unsigned int lmode)
* DESCRIPTION : Power Down Register of Video Bus Config 
* ***************************************************************************************/
VOLATILE void tca_ckc_setvideobuscfgswreset(unsigned int lpwdn , unsigned int lmode)
{
	PVIDEOBUSCFG pVIDEOBUSCFG;
	unsigned int lindex[] = {Hw0,Hw1,Hw2,Hw3};

	pVIDEOBUSCFG = (PVIDEOBUSCFG)(iomap_p2v((unsigned int)&HwVIDEOBUSCFG_BASE));

	if(lmode)
		pVIDEOBUSCFG->SWRESET |= lindex[lpwdn];
	else
		pVIDEOBUSCFG->SWRESET &= ~lindex[lpwdn];
	
}

/****************************************************************************************
* FUNCTION :void tca_ckc_getclkctrl0(void)
* DESCRIPTION :
* ***************************************************************************************/
VOLATILE unsigned int tca_ckc_getclkctrl0(void)
{
    return pCKC->CLK0CTRL;
}

VOLATILE void tca_ckc_sethsiobus(unsigned int sel, unsigned int mode)
{
	unsigned int hIndex[] = {Hw0,Hw1,Hw2,Hw3,Hw4,Hw5,Hw6,Hw7};
	PHSIOBUSCFG pHSIOBUSCFG = (PHSIOBUSCFG)(iomap_p2v((unsigned int)&HwHSIOBUSCFG_BASE));

	if(mode)
		pHSIOBUSCFG->HCLKMASK0 &= ~hIndex[sel];
	else
		pHSIOBUSCFG->HCLKMASK0 |= hIndex[sel];
}

VOLATILE int tca_ckc_gethsiobus(unsigned int sel)
{
	unsigned int hIndex[] = {Hw0,Hw1,Hw2,Hw3,Hw4,Hw5,Hw6,Hw7};
	PHSIOBUSCFG pHSIOBUSCFG = (PHSIOBUSCFG)(iomap_p2v((unsigned int)&HwHSIOBUSCFG_BASE));
	int iRetVal;

	iRetVal = pHSIOBUSCFG->HCLKMASK0 & hIndex[sel];

	if(iRetVal == 0)
		iRetVal = 1;
	else
		iRetVal = 0;

	return iRetVal;
}

VOLATILE void tca_ckc_sethsiobusswreset(unsigned int sel, unsigned int mode)
{
	unsigned int hIndex[] = {Hw0,Hw1,Hw2,Hw3,Hw4,Hw5,Hw6,Hw7};
	PHSIOBUSCFG pHSIOBUSCFG = (PHSIOBUSCFG)(iomap_p2v((unsigned int)&HwHSIOBUSCFG_BASE));

	if(mode)
		pHSIOBUSCFG->SWRESET0 |= hIndex[sel];
	else
		pHSIOBUSCFG->SWRESET0 &= ~hIndex[sel];	
}

//This function can be used in only kernel mode. 
VOLATILE int tca_ckc_setcommonhsiobus(unsigned int hsiocontroller, unsigned int enable)
{
	static unsigned int cur_status = 0;

	if(enable == ENABLE)
	{
		if(cur_status == 0)
		{
			//tca_ckc_setpll(5000000, 4);
			//tca_ckc_setfbusctrl(CLKCTRL8, ENABLE, 0, 2500000, DIRECTPLL4);

		#if defined(CONFIG_MACH_TCC9300ST)
			tca_ckc_setpll(5940000, 2); /* For THS8200 */
			tca_ckc_setfbusctrl( CLKCTRL8,  ENABLE, 0, 1980000, DIRECTPLL2);	/*FBUS_HSIO     198 MHz */
		#else
			tca_ckc_setpll(6480000, 2);
			tca_ckc_setfbusctrl( CLKCTRL8,  ENABLE, 0, 2160000, DIRECTPLL2);	/*FBUS_HSIO     240 MHz */
		#endif
		}
		cur_status |= hsiocontroller;
	}
	else
	{
		cur_status &= ~(hsiocontroller);
		if(cur_status == 0)
		{
		#if defined(CONFIG_MACH_TCC9300ST)
			tca_ckc_setpll(5940000, 2);  /* For THS8200 */
			tca_ckc_setfbusctrl( CLKCTRL8,  ENABLE, 0, 1980000, DIRECTPLL2);	/*FBUS_HSIO     198 MHz */			
		#else
			tca_ckc_setpll(6480000, 2);
			tca_ckc_setfbusctrl( CLKCTRL8,  ENABLE, 0, 2160000, DIRECTPLL2);	/*FBUS_HSIO     240 MHz */			
		#endif
			
			//tca_ckc_setfbusctrl(CLKCTRL8, DISABLE, 0, 2500000, DIRECTPLL4);
			//tca_ckc_setpll(0, 4);
		}
	}

	return cur_status;
}

VOLATILE void tca_ckc_setcambuspwdn(unsigned int sel, unsigned int mode)
{
	PCAMBUSCFG pCAMBUSCFG = (PCAMBUSCFG)(iomap_p2v((unsigned int)&HwCAMBUSCFG_BASE));

	if(mode)
		pCAMBUSCFG->PowerDownMode &= ~(0x1<<sel);
	else
		pCAMBUSCFG->PowerDownMode |= (0x1<<sel);
}

VOLATILE int tca_ckc_getcambuspwdn(unsigned int sel)
{
	PCAMBUSCFG pCAMBUSCFG = (PCAMBUSCFG)(iomap_p2v((unsigned int)&HwCAMBUSCFG_BASE));

	return ((pCAMBUSCFG->PowerDownMode & (0x1<<sel)) ? 0 : 1);
}

VOLATILE void tca_ckc_setcambusswreset(unsigned int sel, unsigned int mode)
{
	PCAMBUSCFG pCAMBUSCFG = (PCAMBUSCFG)(iomap_p2v((unsigned int)&HwCAMBUSCFG_BASE));

	if(mode)
		pCAMBUSCFG->SoftResetRegister |= (0x1<<sel);
	else
		pCAMBUSCFG->SoftResetRegister &= ~(0x1<<sel);
}

VOLATILE void tca_ckc_setddisubbus(unsigned int sel, unsigned int mode)
{
	PMIPIDSBUSCFG pMIPICFG = (PMIPIDSBUSCFG)(iomap_p2v((unsigned int)&HwMIPIDSBUSCFG_BASE));

	if(mode)
		pMIPICFG->HCLKMASK &= ~(0x1<<sel);
	else
		pMIPICFG->HCLKMASK |= (0x1<<sel);
}

VOLATILE int tca_ckc_getddisubbus(unsigned int sel)
{
	PMIPIDSBUSCFG pMIPICFG = (PMIPIDSBUSCFG)(iomap_p2v((unsigned int)&HwMIPIDSBUSCFG_BASE));

	return ((pMIPICFG->HCLKMASK & (0x1<<sel)) ? 0 : 1);
}

VOLATILE void tca_ckc_setddisubbusswreset(unsigned int sel, unsigned int mode)
{
	PMIPIDSBUSCFG pMIPICFG = (PMIPIDSBUSCFG)(iomap_p2v((unsigned int)&HwMIPIDSBUSCFG_BASE));

	if(mode)
		pMIPICFG->SWRST |= (0x1<<sel);
	else
		pMIPICFG->SWRST &= ~(0x1<<sel);
}

VOLATILE void tca_ckc_enable(int clk, int enable)
{
	volatile unsigned *pCLKCTRL;

	pCLKCTRL =(volatile unsigned    *)((&pCKC->CLK0CTRL)+clk);
	if (enable)
		*pCLKCTRL |= Hw21;
	else
		*pCLKCTRL &= ~Hw21;
}

VOLATILE void tca_ckc_pclk_enable(int pclk, int enable)
{
	volatile unsigned *pPERI;
	pPERI =(volatile unsigned *)((&pCKC->PCLK_TCX)+pclk);
        if (enable)
                *pPERI |= Hw29;
        else
                *pPERI &= ~Hw29;
}

int tca_ckc_set_mem_freq(unsigned int freq)
{
#if defined(CONFIG_DRAM_DDR3)
	extern void tcc_ddr_set_clock(unsigned int freq);
	tcc_ddr_set_clock(freq);
#elif defined(CONFIG_DRAM_DDR2)
	extern void tcc_ddr_set_clock(unsigned int freq);
	tcc_ddr_set_clock(freq);
#elif defined(CONFIG_DRAM_MDDR)
	extern int tcc_mddr_set_clock(unsigned int freq);
	return tcc_mddr_set_clock(freq);
#endif
	return 0;
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
EXPORT_SYMBOL(tca_ckc_set_iobus_swreset);
EXPORT_SYMBOL(tca_ckc_setswreset);
EXPORT_SYMBOL(tca_ckc_setiobus);
EXPORT_SYMBOL(tca_ckc_getiobus);
EXPORT_SYMBOL(tca_ckc_setsmui2c);
EXPORT_SYMBOL(tca_ckc_getsmui2c);
EXPORT_SYMBOL(tca_ckc_setgrppwdn);
EXPORT_SYMBOL(tca_ckc_getgrppwdn);
EXPORT_SYMBOL(tca_ckc_setddipwdn);
EXPORT_SYMBOL(tca_ckc_getddipwdn);
EXPORT_SYMBOL(tca_ckc_setvideobuscfgpwdn);
EXPORT_SYMBOL(tca_ckc_getvideobuscfgpwdn);
EXPORT_SYMBOL(tca_ckc_setvideobuscfgswreset);
EXPORT_SYMBOL(tca_ckc_sethsiobus);
EXPORT_SYMBOL(tca_ckc_gethsiobus);
EXPORT_SYMBOL(tca_ckc_sethsiobusswreset);
EXPORT_SYMBOL(tca_ckc_setcambuspwdn);
EXPORT_SYMBOL(tca_ckc_getcambuspwdn);
EXPORT_SYMBOL(tca_ckc_setcambusswreset);
EXPORT_SYMBOL(tca_ckc_setddisubbus);
EXPORT_SYMBOL(tca_ckc_getddisubbus);
EXPORT_SYMBOL(tca_ckc_setddisubbusswreset);
#endif

/* end of file */
