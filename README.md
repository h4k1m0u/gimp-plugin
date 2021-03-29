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
- [Plugin development tutorial in C][3].
- Search for gimp plugins on [Github][4].
- Source code for [plugins included in gimp][5].

[1]: https://developer.gimp.org/api/2.0/
[2]: https://developer.gimp.org/api/2.0/libgimp/libgimp-index.html
[3]: https://developer.gimp.org/plug-ins.html
[4]: https://github.com/search?l=C&q=gimp+plugin&type=Repositories
[5]: https://gitlab.gnome.org/GNOME/gimp/-/tree/gimp-2-10/plug-ins


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
Image processing operations represented by graph nodes. See [its documentation][gegl-api].

[gegl-api]: http://getfr.org/pub/dragonfly-release/usr-local-share/doc/gegl/api.html
