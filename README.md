# mpd-sacd
A version of the mpd music player daemon that includes support for SACDs and DVD-A discs

This project is a modified version of the code found here: https://sourceforge.net/projects/mpd.sacddecoder.p/files/

My modifications to this project are made primarily with macOS in mind.

The original mpd:

See https://www.musicpd.org/
See https://github.com/MusicPlayerDaemon/MPD

Config: See https://computingforgeeks.com/install-configure-mpd-ncmpcpp-macos/

## To build or rebuild mpd-sacd

Install dependencies via Homebrew:

```sh
brew install mpd
brew install meson
brew install ninja
```

```sh
cd mpd-sacd
rm -rf output
meson . output/release --buildtype=debugoptimized -Db_ndebug=true
meson configure output/release -Dsysconfdir='etc/mpd-sacd'
```

MPD will then look for mpd.conf in /usr/local/etc/mpd-sacd/

```sh
ninja -C output/release
```

The executable mpd can then be found in output/release/

To install:

```sh
ninja -C output/release install
```

- To launch mpd as a daemon: $ mpd
- To kill the running instance of mpd: $ mpd --kill
- To update the mpd database: $ mpc update
- To launch the rudimentary player: $ ncmpcpp

Original documentation about compiling from source:
https://mpd.readthedocs.io/en/stable/user.html#compiling-from-source
