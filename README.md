# Prerequisites
```console
$ sudo apt install libgimp2.0-dev
```


# Installation
## Folders
- **Gimp's plugins:** /usr/lib/gimp/2.0/plug-ins
- **User's pluging:** ~/.config/GIMP/2.10/plug-ins/

## From a single source file
```console
$ gimptool-2.0 --install main.c
```

## From a cmake project
```console
$ mkdir build && cd build
$ cmake .. && make
$ gimptool-2.0 --install-bin <executable>
```


# Uninstallation
```console
$ gimptool-2.0 --uninstall-bin <executable>
```

# Resources
- [LibGimp manual][1].
- [LibGimp reference][2].
- [Updated reference for Gimp 2.10.16][gimp-2.10.16-api].
- [Plugin development tutorial in C][3].
- Search for gimp plugins on [Github][4].
- Source code for [plugins included in gimp][gimp-plugins], particularly `<gimp-repo>/plug-ins/file-bmp` for reading/writing BMP images.

[1]: https://developer.gimp.org/api/2.0/
[2]: https://developer.gimp.org/api/2.0/libgimp/libgimp-index.html
[3]: https://developer.gimp.org/plug-ins.html
[4]: https://github.com/search?l=C&q=gimp+plugin&type=Repositories
[gimp-plugins]: https://gitlab.gnome.org/GNOME/gimp/-/tree/gimp-2-10/plug-ins
[gimp-2.10.16-api]: https://www.manpagez.com/html/libgimp/libgimp-2.10.16/


# Debugging
1. Build Gimp from [source][gimp-source], babl and GEGL may need to be built from source if on Ubuntu:
```console
$ ./configure --disable-python --enable-debug
$ make
# make install
```

2. Following [this link][gimp-plugin-debug]:

```console
$ export GIMP_PLUGIN_DEBUG=<plugin-binary-name>
$ gimp &
```

3. Once the plugin is run, its pid should appear on the terminal gimp was launched from. All that's left to do is to attach `gdb` to that process:

```console
$ cgdb ~/.config/GIMP/2.10/plug-ins/box_blur/box_blur
(gdb) b <function-name>
(gdb) handle all nostop # to ignore signals interruptions, or simply `continue` multiple times to skip them
(gdb) attach <pid>
```

[gimp-source]: https://www.gimp.org/source/#gimp-source-code
[gimp-plugin-debug]: https://gitlab.gnome.org/GNOME/gimp/-/blob/master/devel-docs/debug-plug-ins.txt

## Troubleshooting
On Ubuntu, gdb is unlikely to bind to a process unless called as root. To circumvent this limitation:

```console
$ echo 0 | sudo tee /proc/sys/kernel/yama/ptrace_scope
```


# GTK spinoffs
## GTK
- Gimp 2.10 is compatible with Gtk 2.
- [Gtk2 official tutorial][gtk-tutorial].

[gtk-tutorial]: https://developer.gnome.org/gtk-tutorial/stable/book1.html

## Glib
- Implementations of data structures in C (chained lists, hash maps...).
- [Glib tutorial][6].
- [Glib reference manual][7].

[6]: https://developer.ibm.com/tutorials/l-glib/
[7]: https://developer.gnome.org/glib/stable/

## GIO
File input/output using streams in C (similar to what they have in java).

## GObject
OOP in C supporting properties, methods, and signals.

## GEGL
Image processing operations represented by graph nodes.
See [its documentation][gegl-api]. To build GEGL from [gegl-source][its source], `meson` and `ninja` need to be used:

```console
$ pip install meson
$ meson build && cd build
$ ninja
$ sudo ninja install
```

[gegl-api]: http://getfr.org/pub/dragonfly-release/usr-local-share/doc/gegl/api.html
[gegl-source]: https://download.gimp.org/pub/gegl/

## BABL
Conversion between pixel formats encoding and color spaces. See [its website][babl-api] for general concepts.
Babl can be built from [babl-source][its source] with meson/ninja similarly to gegl.

[babl-api]: https://gegl.org/babl/
[babl-source]: https://download.gimp.org/pub/babl/
