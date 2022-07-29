#!/bin/sh

# April 16, 2022

# To build or rebuild mpd-sacd:

rm -rf output
meson . output/release --buildtype=debugoptimized -Db_ndebug=true
meson configure output/release -Dsysconfdir='etc/mpd-sacd'
# MPD will then look for mpd.conf in /usr/local/etc/mpd-sacd/
ninja -C output/release
# The executable mpd can then be found in output/release/

# - To launch mpd as a daemon: $ mpd
# - To kill the running instance of mpd: $ mpd --kill
# - To update the mpd database: $ mpc update
# - To launch the rudimentary player: $ ncmpcpp

