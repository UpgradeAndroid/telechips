#!/bin/sh
CONFIGS=$1
if [ "$CONFIGS" = "" ]; then
	CONFIGS="m805_880x tcc880x tcc8920st_hdb892s tcc8925st_donglehs tcc8925st_isdbt_module m805_892x  tcc8920st  tcc8925st_dongle tcc8925st_hdb892f tcc892x tcc88xx_ua"
fi

if [ "$ARCH" = "" -o "$CROSS_COMPILE" = "" ]; then
	echo "ARCH or CROSS_COMPILE not setup!"
	exit 1
fi

make -j8 mrproper

for cfg in $CONFIGS; do
	if ! make ${cfg}_defconfig; then
		echo error configuring for $cfg!
		exit 1;
	fi
	time make -j8 2>&1 | tee $cfg.log
done

echo ==== Warnings overview:
grep -c 'warning:' *.log
echo ==== Section mismatch overview:
grep 'section mismatch' *.log
