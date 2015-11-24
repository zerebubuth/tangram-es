# Giflantro

Giflantro is an attempt to write something to offscreen render maps to raster formats. This has a large number of potential applications, e.g:

* Giflantro could render raster tiles as a fall-back for clients not supporting WebGL. This would be about 17% of the worldwide audience according to [CanIUse](http://caniuse.com/#search=webgl), but that does include all but the current version of IE.
* Giflantro could render raster images to embed in pages for use trialling or comparing changes to styles or datasources. This might be useful as a CI step.
* Giflantro could render hi-res raster images to use for printing. It's possible that WebGL could do this without any external tool, but that seems like something that's very new and probably hasn't stabilised yet.
* Giflantro could be used to render animated GIFs of animated styles. Because hilarity.

## How do I build it?

You should be able to check out this branch and run:

```
mkdir build
cd build
cmake -DPLATFORM_TARGET=linuxosmesa ..
```

From the top level of the source tree. Remember that there are submodules, so you'll need to have run `git submodule update --init --recursive` before trying to build!

This produces a `build/bin/tangram` executable which will currently renders an 800x600 animated GIF of the style's default viewport (so make sure it has one!).

## How does it work?

Giflantro uses [OSMesa](http://www.mesa3d.org/osmesa.html), the Mesa off-screen rendering library, to render without needing a windowing system. This means it can run headless on a server, or from the command line without interfering with the window system.
