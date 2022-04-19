# mpd-sacd
A version of the mpd music player daemon that includes support for SACDs and DVD-A discs

This project is a fork of [this project](https://sourceforge.net/projects/mpd.sacddecoder.p/files/), which adds support for Super Audio CDs and DVD-Audio discs to [mpd](https://www.musicpd.org/).

My modifications to this project are made primarily with macOS in mind.

The original mpd can be found on GitHub [here](https://github.com/MusicPlayerDaemon/MPD).

[Here](https://computingforgeeks.com/install-configure-mpd-ncmpcpp-macos/) is a useful guide to setting up mpd and the ncmpcpp player on macOS.

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

[Here](https://mpd.readthedocs.io/en/stable/user.html#compiling-from-source) is the original documentation about compiling from the source.
