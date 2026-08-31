#!/usr/bin/env python3
"""
Regenerate droidx_page.h from DroidX.html.

The page is stored GZIPPED in flash and served with
Content-Encoding: gzip. Run this after every edit to DroidX.html -
droidx_page.h is generated and must never be edited by hand.

    python make_page.py

Works in either layout, so one copy of this script serves both:
  * all four files in one folder   (the published repository)
  * DroidX/ beside Casio-ESP32-DroidX/   (the development tree)
"""
import glob, gzip, io, os, sys

HERE = os.path.dirname(os.path.abspath(__file__))

# The sketch tells us which layout this is. If a .ino sits beside this
# script, everything lives in one folder (the published repository).
# Otherwise this is the development tree, with DroidX/ beside the
# sketch folder.
if glob.glob(os.path.join(HERE, "*.ino")):
    SRC = os.path.join(HERE, "DroidX.html")
    DST = os.path.join(HERE, "droidx_page.h")
else:
    SRC = os.path.join(HERE, "DroidX.html")
    DST = os.path.join(HERE, "..", "Casio-ESP32-DroidX", "droidx_page.h")

if not os.path.exists(SRC):
    sys.exit("DroidX.html not found beside this script")

raw = open(SRC, "rb").read()

# mtime=0 so the same page always produces the same bytes: a rebuild
# that changes nothing should show as no change.
buf = io.BytesIO()
with gzip.GzipFile(fileobj=buf, mode="wb", compresslevel=9, mtime=0) as gz:
    gz.write(raw)
comp = buf.getvalue()

rows = []
for i in range(0, len(comp), 16):
    rows.append("  " + " ".join("0x%02x," % b for b in comp[i:i + 16]))

out = """/*
 ===================================================================
  DROIDX PAGE - served GZIPPED from PROGMEM by Casio-ESP32-DroidX.ino
  (C) Michael Fenton, MRSNZ, 2026. CC BY-NC-SA 4.0.

  *** GENERATED FILE. DO NOT EDIT. ***
  Edit DroidX.html, then run make_page.py.

  %d bytes of HTML compress to %d, which is why it is stored this
  way: the page is the largest single object in the firmware and the
  saving is worth more than every diagnostic string in the sketch put
  together.

  SELF-CONTAINED BY REQUIREMENT. Served from the ESP32's own access
  point, where there is no internet, so it fetches nothing: no fonts,
  no CDN, no icon set, no block-editor library.
 ===================================================================
*/
#pragma once
#include <pgmspace.h>

const uint8_t DROIDX_PAGE_GZ[] PROGMEM = {
%s
};
const size_t DROIDX_PAGE_GZ_LEN = sizeof DROIDX_PAGE_GZ;
""" % (len(raw), len(comp), "\n".join(rows))

open(DST, "w", encoding="utf-8", newline="\n").write(out)
print("%s\n  %d bytes HTML -> %d bytes gzip (%.1f%%)"
      % (os.path.normpath(DST), len(raw), len(comp), 100.0 * len(comp) / len(raw)))
