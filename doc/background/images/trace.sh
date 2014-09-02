#! /bin/bash

file=$1
outputfile=$2

autotrace \
	--background-color A6A6A6 \
	--color-count 6 \
	--despeckle-level 19 \
	--output-file $outputfile \
	--output-format eps \
	$file


# Usage: autotrace [options] <input_name>.
# Options:<input_name> should be a supported image.
#   You can use `--' or `-' to start an option.
#   You can use any unambiguous abbreviation for an option name.
#   You can separate option names and values with `=' or ` '.
# background-color <hexadezimal>: the color of the background that
#   should be ignored, for example FFFFFF;
#   default is no background color.
# centerline: trace a character's centerline, rather than its outline.
# color-count <unsigned>: number of colors a color bitmap is reduced to,
#   it does not work on grayscale, allowed are 1..256;
#   default is 0, that means not color reduction is done.
# corner-always-threshold <angle-in-degrees>: if the angle at a pixel is
#   less than this, it is considered a corner, even if it is within
#   `corner-surround' pixels of another corner; default is 60.
# corner-surround <unsigned>: number of pixels on either side of a
#   point to consider when determining if that point is a corner;
#   default is 4.
# corner-threshold <angle-in-degrees>: if a pixel, its predecessor(s),
#   and its successor(s) meet at an angle smaller than this, it's a
#   corner; default is 100.
# despeckle-level <unsigned>: 0..20; default is no despeckling.
# despeckle-tightness <real>: 0.0..8.0; default is 2.0.
# dpi <unsigned>: The dots per inch value in the input image, affects scaling
#   of mif output image
# error-threshold <real>: subdivide fitted curves that are off by
#   more pixels than this; default is 2.0.
# filter-iterations <unsigned>: smooth the curve this many times
#   before fitting; default is 4.
# input-format:  PNG, TGA, PBM, PNM, PGM, PPM or BMP.
# help: print this message.
# line-reversion-threshold <real>: if a spline is closer to a straight
#   line than this, weighted by the square of the curve length, keep it a
#   straight line even if it is a list with curves; default is .01.
# line-threshold <real>: if the spline is not more than this far away
#   from the straight line defined by its endpoints,
#   then output a straight line; default is 1.
# list-output-formats: print a list of support output formats to stderr.
# list-input-formats:  print a list of support input formats to stderr.
# log: write detailed progress reports to <input_name>.log.
# output-file <filename>: write to <filename>
# output-format <format>: use format <format> for the output file
#   eps, ai, p2e, sk, svg, fig, emf, mif, er, dxf, epd, pdf, cgm or dr2d can be used.
# preserve-width: whether to preserve line width prior to thinning.
# remove-adjacent-corners: remove corners that are adjacent.
# tangent-surround <unsigned>: number of points on either side of a
#   point to consider when computing the tangent at that point; default is 3.
# report-progress: report tracing status in real time.
# debug-arch: print the type of cpu.
# debug-bitmap: dump loaded bitmap to <input_name>.bitmap.
# version: print the version number of this program.
# width-weight-factor <real>: weight factor for fitting the linewidth.

# You can get the source code of autotrace from
# http://autotrace.sourceforge.net