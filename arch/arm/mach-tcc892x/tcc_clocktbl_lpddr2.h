/*
 * arch/arm/mach-tcc892x/tcc_clocktbl_lpddr2.h
 *
 * TCC892x cpufreq driver
 *
 * Copyright (C) 2011 Telechips, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

static struct tcc_freq_table_t gtClockLimitTable[] = {
	/*  CPU /   DDI /   MEM /   GPU /    IO /  VBUS /  VCOD /   SMU /  HSIO */
//	{ 225000,      0, 125000,      0, 116000,      0,      0,  50000,      0 },	// Core 1.00V
//	{ 325000,      0, 150000,      0, 136000,      0,      0, 100000,      0 },	// Core 1.05V
//	{ 425000,      0, 200000,      0, 156000,      0,      0, 100000,      0 },	// Core 1.10V
//	{ 525000,      0, 250000,      0, 176000,      0,      0, 100000,      0 },	// Core 1.15V
	{ 625000,      0, 300000,      0, 196000,      0,      0, 100000,      0 },	// Core 1.20V
//	{ 800000,      0, 360000,      0, 220000,      0,      0, 100000,      0 },	// Core 1.20V
//	{ 996000,      0, 400000,      0, 220000,      0,      0, 100000,      0 },	// Core 1.20V
};

//sync with gtJpegClockLimitTable
const struct tcc_freq_table_t gtCameraClockLimitTable[] =
{
	{      0, 312000, 300000,      0,      0,      0,      0,      0,      0 },	// Core 1.20V
	{      0, 312000, 300000,      0,      0,      0,      0,      0,      0 },	// Core 1.20V
	{      0, 312000, 300000,      0,      0,      0,      0,      0,      0 },	// Core 1.20V
};

const struct tcc_freq_table_t gtISPCameraClockLimitTable[] =
{
	{      0, 312000, 300000,      0,      0,      0,      0,      0,      0 },	// Core 1.20V
	{      0, 312000, 300000,      0,      0,      0,      0,      0,      0 },	// Core 1.20V
	{      0, 312000, 300000,      0,      0,      0,      0,      0,      0 },	// Core 1.20V
};

const struct tcc_freq_table_t gtVpuNormalClockLimitTable[] =
{
	{      0, 312000, 300000,      0,      0, 277000, 277000,      0,      0 },	// Core 1.20V
	{      0, 312000, 300000,      0,      0, 277000, 277000,      0,      0 },	// Core 1.20V
	{      0, 312000, 300000,      0,      0, 277000, 277000,      0,      0 },	// Core 1.20V
};

const struct tcc_freq_table_t gtJpegClockLimitTable[]= {
	{      0, 312000, 300000,      0,      0, 277000, 277000,      0,      0 },	// Core 1.20V
	{      0, 312000, 300000,      0,      0, 277000, 277000,      0,      0 },	// Core 1.20V
	{      0, 312000, 300000,      0,      0, 277000, 277000,      0,      0 },	// Core 1.20V
};

const struct tcc_freq_table_t gtJpegMaxClockLimitTable = {
	       0, 312000, 300000,      0,      0, 277000, 277000,      0,      0	// Core 1.20V
};

const struct tcc_freq_table_t gtHdmiClockLimitTable = {
	       0, 312000, 300000,      0, 196000,      0,      0,      0,      0	// Core 1.20V
};

const struct tcc_freq_table_t gtMaliClockLimitTable = {
	       0,      0, 300000, 370000,      0,      0,      0,      0,      0	// Core 1.20V
};

const struct tcc_freq_table_t stFBClockLimitTable = {
	       0, 110000, 125000,      0,  98000,      0,      0,      0,      0	// Core 1.00V
};

const struct tcc_freq_table_t gtOverlayClockLimitTable = {
	       0,      0,      0,      0, 196000,      0,      0,      0,      0	// Core 1.20V
};


const struct tcc_freq_table_t stPowerResumeClockLimitTable = {
	       0,      0, 300000,      0, 196000,      0,      0,      0,      0	// Core 1.20V
};

const struct tcc_freq_table_t gtTvClockLimitTable = {
	       0, 312000, 300000,      0,      0,      0,      0,      0,      0	// Core 1.20V
};

const struct tcc_freq_table_t gtEthernetClockLimitTable = {
	       0, 312000, 300000,      0, 196000,      0,      0,      0,      0	// Core 1.20V
};

const struct tcc_freq_table_t gtBtClockLimitTable = {
	       0,      0, 300000,      0, 196000,      0,      0,      0,      0	// Core 1.32V
};

const struct tcc_freq_table_t gtUSBClockLimitTable[] = {
	{      0,      0,      0,      0,  98000,      0,      0,      0,      0 }, // Core 1.00V	// Idle
	{      0,      0, 300000,      0, 196000,      0,      0,      0,      0 }, // Core 1.20V	// Actived
};

const struct tcc_freq_table_t gtAppClockLimitTable = {
	  600000,      0, 300000,      0,      0,      0,      0,      0,      0	// Core 1.32V
};

const struct tcc_freq_table_t gtRemoconClockLimitTable = {
	       0,      0,      0,      0, 196000,      0,      0,      0,      0	// Core 1.20V
};

const struct tcc_freq_table_t gtHSIOClockLimitTable = {
	       0,      0, 300000,      0,      0,      0,      0,      0, 250000	// Core 1.25V
};

const struct tcc_freq_table_t gtHSIONormalClockLimitTable = {
	       0,      0,      0,      0,      0,      0,      0,      0, 125000	// Core 1.00V
};

const struct tcc_freq_table_t gtVoipClockLimitTable[] = {
	{      0,      0, 300000,      0, 196000,      0,      0,      0,      0 },	// Core 1.10V
	{      0,      0, 300000,      0, 196000,      0,      0,      0,      0 },	// Core 1.32V
};

EXPORT_SYMBOL(gtCameraClockLimitTable);
EXPORT_SYMBOL(gtISPCameraClockLimitTable);
EXPORT_SYMBOL(gtVpuNormalClockLimitTable);
EXPORT_SYMBOL(gtJpegClockLimitTable);
EXPORT_SYMBOL(gtJpegMaxClockLimitTable);
EXPORT_SYMBOL(gtHdmiClockLimitTable);
EXPORT_SYMBOL(gtMaliClockLimitTable);
EXPORT_SYMBOL(stFBClockLimitTable);
EXPORT_SYMBOL(gtTvClockLimitTable);
EXPORT_SYMBOL(gtEthernetClockLimitTable);
EXPORT_SYMBOL(gtBtClockLimitTable);
EXPORT_SYMBOL(gtAppClockLimitTable);
EXPORT_SYMBOL(gtUSBClockLimitTable);
EXPORT_SYMBOL(gtRemoconClockLimitTable);
EXPORT_SYMBOL(gtHSIOClockLimitTable);
EXPORT_SYMBOL(gtHSIONormalClockLimitTable);
EXPORT_SYMBOL(gtVoipClockLimitTable);
