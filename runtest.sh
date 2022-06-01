#!/bin/sh

# quick and dirty test file

die() {
	echo "\e[31m[ERROR]\e[0m" $1
	exit 1
}

info_test() {
	echo ""
	echo "\e[32m[INFO]\e[0m" $1
	echo "---------------------------------------"
}

if [ ! -x "chunkinfo" ]; then
	die "chunkinfo doesn't exist, please compile it first"
fi

pngsuite_dir="pngsuite"

exec_test() {
	if ( ./chunkinfo $1 >/dev/null ); then
		echo "  \e[32m[OK]\e[0m " $1
	else
		echo "  \e[31m[FAIL]\e[0m " $1
	fi
}

test_basic() {
	files="basn0g01.png basn0g02.png basn0g04.png basn0g08.png basn0g16.png basn2c08.png basn2c16.png basn3p01.png basn3p02.png basn3p04.png basn3p08.png basn4a08.png basn4a16.png basn6a08.png basn6a16.png"
	info_test "Test basic formats"
	for f in $files; do
		exec_test "$pngsuite_dir/$f"
	done
}

test_interlace() {
	files="basi0g01.png basi0g02.png basi0g04.png basi0g08.png basi0g16.png basi2c08.png basi2c16.png basi3p01.png basi3p02.png basi3p04.png basi3p08.png basi4a08.png basi4a16.png basi6a08.png basi6a16.png"
	info_test "Test interlacing"
	for f in $files; do
		exec_test "$pngsuite_dir/$f"
	done
}

test_odd_sizes() {
	files="s01i3p01.png s01n3p01.png s02i3p01.png s02n3p01.png s03i3p01.png s03n3p01.png s04i3p01.png s04n3p01.png s05i3p02.png s05n3p02.png s06i3p02.png s06n3p02.png s07i3p02.png s07n3p02.png s08i3p02.png s08n3p02.png s09i3p02.png s09n3p02.png s32i3p04.png s32n3p04.png s33i3p04.png s33n3p04.png s34i3p04.png s34n3p04.png s35i3p04.png s35n3p04.png s36i3p04.png s36n3p04.png s37i3p04.png s37n3p04.png s38i3p04.png s38n3p04.png s39i3p04.png s39n3p04.png s40i3p04.png s40n3p04.png"
	info_test "Test odd sizes"
	for f in $files; do
		exec_test "$pngsuite_dir/$f"
	done
}

test_bg_colors() {
	files="bgai4a08.png bgai4a16.png bgan6a08.png bgan6a16.png bgbn4a08.png bggn4a16.png bgwn6a08.png bgyn6a16.png"
	info_test "Test background colors"
	for f in $files; do
		exec_test "$pngsuite_dir/$f"
	done
}

test_transparency() {
	files="tbbn0g04.png tbbn2c16.png tbbn3p08.png tbgn2c16.png tbgn3p08.png tbrn2c08.png tbwn0g16.png tbwn3p08.png tbyn3p08.png tm3n3p02.png tp0n0g08.png tp0n2c08.png tp0n3p08.png tp1n3p08.png"
	info_test "Test transparency"
	for f in $files; do
		exec_test "$pngsuite_dir/$f"
	done
}

test_gamma() {
	files="g03n0g16.png g03n2c08.png g03n3p04.png g04n0g16.png g04n2c08.png g04n3p04.png g05n0g16.png g05n2c08.png g05n3p04.png g07n0g16.png g07n2c08.png g07n3p04.png g10n0g16.png g10n2c08.png g10n3p04.png g25n0g16.png g25n2c08.png g25n3p04.png"
	info_test "Test gamma values"
	for f in $files; do
		exec_test "$pngsuite_dir/$f"
	done
}

test_filter() {
	files="f00n0g08.png f00n2c08.png f01n0g08.png f01n2c08.png f02n0g08.png f02n2c08.png f03n0g08.png f03n2c08.png f04n0g08.png f04n2c08.png f99n0g04.png"
	info_test "Test image filtering"
	for f in $files; do
		exec_test "$pngsuite_dir/$f"
	done
}

test_pallete() {
	files="pp0n2c16.png pp0n6a08.png ps1n0g08.png ps1n2c16.png ps2n0g08.png ps2n2c16.png"
	info_test "Test image pallete"
	for f in $files; do
		exec_test "$pngsuite_dir/$f"
	done
}

test_zlib() {
	files="z00n2c08.png z03n2c08.png z06n2c08.png z09n2c08.png"
	info_test "Test zlib compression"
	for f in $files; do
		exec_test "$pngsuite_dir/$f"
	done
}

test_corrupt() {
	files="xc1n0g08.png xc9n2c08.png xcrn0g04.png xcsn0g01.png xd0n2c08.png xd3n2c08.png xd9n2c08.png xhdn0g08.png xlfn0g04.png xs1n0g01.png xs4n0g01.png xs2n0g01.png xs7n0g01.png"
	info_test "Test corrupted files, must FAIL"
	for f in $files; do
		exec_test "$pngsuite_dir/$f"
	done
}

test_all() {
	test_basic
	test_interlace
	test_odd_sizes
	test_bg_colors
	test_transparency
	test_gamma
	test_filter
	test_pallete
	test_zlib
	test_corrupt
}

test_all
