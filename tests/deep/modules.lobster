import std

import color
import match
import date_time

// A relative import of a nested directory that has its own local (relative)
// import, which resolves regardless of where the importing project lives.
import .relimport.inner
// A file whose only statement is a (here duplicate) import: it compiling at
// all is the test.
import .relimport.only_import

// Pure-Lobster stdlib modules not covered elsewhere: color, match and the
// date_time enum. (quaternion and mat2x2 are covered in overloads.lobster.)

def modules_tests():
    suite("modules")

    // Relative imports, see import above.
    test_eq(relimport_inner(), 42)
    test_eq(relimport_inner2(), 41)

    // color: a 4-float struct, so all float struct math applies.
    test_eq(color_red + color_blue, color { 1.0, 0.0, 1.0, 2.0 })
    test_eq(color_white * 0.5, color { 0.5, 0.5, 0.5, 0.5 })
    test_eq(color_black.alpha, 1.0)
    test_eq(color_clear, color { 0.0, 0.0, 0.0, 0.0 })
    // The luminance weights sum to 1, so white has luminance 1, black 0.
    test_approx(luminance(color_white), 1.0)
    test_approx(luminance(color_black), 0.0)
    test_true(luminance(color_red) < luminance(color_green))
    // srgb conversions are inverses (on positive colors).
    test_approx(to_srgb(from_srgb(color_grey)).red, color_grey.red)
    test_approx(from_srgb(color_white).green, 1.0)
    test_eq(from_srgb(color_black).blue, 0.0)
    // lerp between colors.
    test_approx(lerp(color_black, color_white, 0.5).red, 0.5)

    // match: pattern matching over vectors (here: unicode values of a string).
    let mr = match_with(string_to_unicode("AABCDE")):
        match_any(): match('A')
        let sub = matched():
            match_any(): match('B') or match_sequence(string_to_unicode("CD"))
        test_eq(unicode_to_string(sub), "BCD")
        match('E') and match_end()
    test_true(mr)
    // A failing match, and match_not.
    let mf = match_with(string_to_unicode("AB")):
        match('B')
    test_true(not mf)
    let mn = match_with(string_to_unicode("AB")):
        match_not('B') and match_not('A') and match_end()
    test_true(mn)
    // match_sequence backtracks on failure, so a later alternative can run.
    let mseq = match_with([ 1, 2, 3 ]):
        match_sequence([ 1, 9 ]) or match_sequence([ 1, 2 ])
        match(3) and match_end()
    test_true(mseq)
    // Over a plain int vector.
    let mv = match_with([ 10, 20 ]):
        match(10) and match(20) and match_end()
    test_true(mv)

    // date_time module: enum of indices into the date_time() builtin vector.
    test_eq(dt_epoch, 0)
    test_eq(dt_sec, 8)
    let dtv = date_time()
    test_eq(dtv.length, 9)
    test_true(dtv[dt_hour] >= 0 and dtv[dt_hour] < 24)
    test_true(dtv[dt_weekday] >= 0 and dtv[dt_weekday] < 7)
    test_true(dtv[dt_month] >= 0 and dtv[dt_month] < 12)

modules_tests()
