# mpd-sacd
A version of the mpd music player daemon that includes support for SACDs and DVD-A discs

## To build or rebuild mpd-sacd

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
