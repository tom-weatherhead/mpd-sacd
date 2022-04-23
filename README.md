# mpd-sacd

[![watchers][watchers-badge-image]][watchers-url]
[![stars][stars-badge-image]][stars-url]
[![issues][issues-badge-image]][issues-url]
[![forks][forks-badge-image]][forks-url]
[![contributors][contributors-badge-image]][contributors-url]
[![branches][branches-badge-image]][branches-url]
[![commits][commits-badge-image]][commits-url]
[![last commit][last-commit-badge-image]][last-commit-url]

This project is a fork of [this project](https://sourceforge.net/projects/mpd.sacddecoder.p/files/), which adds support for Super Audio CDs and DVD-Audio discs to the official [mpd music player daemon](https://www.musicpd.org/).

My modifications to this project are made primarily with macOS in mind.

The official mpd source code can be found on GitHub [here](https://github.com/MusicPlayerDaemon/MPD).

And [here](https://computingforgeeks.com/install-configure-mpd-ncmpcpp-macos/) is a useful guide to setting up mpd and the ncmpcpp player on macOS.

## To build or rebuild mpd-sacd

Install dependencies via Homebrew:

```sh
brew install mpd
brew install meson
brew install ninja
```

Configure:

```sh
cd mpd-sacd
rm -rf output
meson . output/release --buildtype=debugoptimized -Db_ndebug=true
meson configure output/release -Dsysconfdir='etc/mpd-sacd'
```

MPD will then look for mpd.conf in /usr/local/etc/mpd-sacd/

Build:

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

## License
[GPL-2.0](https://opensource.org/licenses/gpl-2.0.php)

[watchers-badge-image]: https://badgen.net/github/watchers/tom-weatherhead/mpd-sacd
[watchers-url]: https://github.com/tom-weatherhead/mpd-sacd/watchers
[stars-badge-image]: https://badgen.net/github/stars/tom-weatherhead/mpd-sacd
[stars-url]: https://github.com/tom-weatherhead/mpd-sacd/stargazers
[issues-badge-image]: https://badgen.net/github/issues/tom-weatherhead/mpd-sacd
[issues-url]: https://github.com/tom-weatherhead/mpd-sacd/issues
[forks-badge-image]: https://badgen.net/github/forks/tom-weatherhead/mpd-sacd
[forks-url]: https://github.com/tom-weatherhead/mpd-sacd/network/members
[contributors-badge-image]: https://badgen.net/github/contributors/tom-weatherhead/mpd-sacd
[contributors-url]: https://github.com/tom-weatherhead/mpd-sacd/graphs/contributors
[branches-badge-image]: https://badgen.net/github/branches/tom-weatherhead/mpd-sacd
[branches-url]: https://github.com/tom-weatherhead/mpd-sacd/branches
[commits-badge-image]: https://badgen.net/github/commits/tom-weatherhead/mpd-sacd
[commits-url]: https://github.com/tom-weatherhead/mpd-sacd/commits/master
[last-commit-badge-image]: https://badgen.net/github/last-commit/tom-weatherhead/mpd-sacd
[last-commit-url]: https://github.com/tom-weatherhead/mpd-sacd
