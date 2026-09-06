/*
DATA-DERIVED SYNTHETIC COMPILER BENCHMARK -- capture of 2026-09-06

Run: bin/lobster.exe tests/bench/benchmark.lobster
Optional program arguments: -- rounds_per_trial trials seed (default: 3000 7 1).
Use --runtime-asserts, and keep backend, arguments and machine fixed when comparing
compiler changes. Unset LOBSTER_BENCH_PROFILE for timings. Compilation, one warmup
round, assertions and output are outside the timed loops. State is rebuilt every
round, including allocations; initialization is deliberately part of this workload.
The minimum and median are seconds per entire trial, not per round. Increase the
round count for smaller effects; do not silently recalibrate the amount of work.

WHY THIS EXISTS
tests/speedtest.lobster is a useful collection of algorithms, but its manually
chosen repetitions can make a compiler change look disproportionately good or bad.
This independent workload was constructed from dynamic feature frequencies in the
actual Lobster sources. It contains common code shapes, not copies of application
algorithms. Its source and repetitions should remain fixed during optimization work.
The existing speedtest and duplicate test aggregators do not contribute extra weight.

SOURCE MATERIAL AND CAPTURE
Read representative game, prototype and test code plus the language reference,
memory-management implementation, compiler/codegen, vec, std and dictionary modules.
The inventory contains 457 .lobster files, including imported modules and explicit
exclusions. All 192 eligible non-Matvoxels entrypoints were attempted: 167 completed,
25 failed, none timed out or remained unattempted. Matvoxels completed separately.
The successful captures used for the three group distributions are:

  Group          Successful / attempted   Raw events          Final group share
  Matvoxels                  1 / 1           970,605,069             50%
  Other projects            77 / 99        9,638,770,935             30%
  Tests and samples         90 / 93        1,194,036,266             20%

Non-Matvoxels: --non-interactive-test 3 --background --runtime-asserts
--no-crash-dialog. The optional frame count was added without changing the old
one-frame default. Nongraphical programs run to completion. Library modules are
represented through entrypoints; intentionally failing compiler fixtures, duplicate
runners, source generators and applications needing external input/account state
have recorded exclusions. The complete manifest, exclusions and hashes are in
corpus.json. Failures, including unavailable custom native APIs, existing compiler
errors/crashes and missing runtime prerequisites, are listed below and archived.

Matvoxels: custom Release executable, --runtime-asserts --background, loading
savegames/SurvivalDemo.4668734651508286811.worldsave through --cpuprofile with
--cpuprofile-frames=30 --cpuprofile-warmup=0. Counters span loading plus exactly
30 actual game updates, with no loading frames mistaken for game updates. The CPU
report has 29 wall-time deltas because its first update establishes the time boundary.
The benchmark exit path saves nothing; the requested save's SHA256 was verified
unchanged: 64b33a29c53fc4900515f836a573c350af6ad21c13be3070d2d22761a0ef7298.
The reproducible warmup-option patch for the nested repository is included here.

Base revisions: Lobster 1ffdd3f1de631bb2e13b901bff3032ff4e573e71;
projects 5bc7432729dc3e9f25cfe1832bba531d4ba97c6c;
Matvoxels dddeb930bc5a645f1b30bd02f0826bc8ad065c00.
The build includes the counter instrumentation and a small correctness fix: do not
inline a terminal void function into a call requiring a value. Matvoxels otherwise
crashed while compiling a throw in a value-producing switch. A focused exception
regression was added; the final root deep suite passed 2062 checks with no failures.
A debug/inlining-minimized game run helped diagnose the issue but is NOT in the mix.

WHAT THE COUNTERS MEAN
codegen.h emits RtBenchCount calls only when LOBSTER_BENCH_PROFILE names a TSV file.
These observe the typed tree AFTER Lobster optimization; constant-folded-away work
does not count. Keys separate family, operation/builtin name, operand/result types,
and context (value, effect, condition; local/global/field/index assignment). Class
names are erased, while scalar types, bool/enums, numeric struct widths, reference
structs, nested vectors and nullable types remain distinct. For example intx3 is an
inline three-int struct, not a heap vector; vector<intx3> is an array of those structs.

If outcomes and loop body iterations are explicit; while-condition events include
the final failed test. Inlined iteration is visible through its body, loop and element
events. Native builtin calls include codegen intrinsics and argument/result signatures.
RC counters count emitted retain/release ATTEMPTS, including nullable checks, not
all reference operations hidden inside runtime helpers or object destruction.
Events overlap: operand reads, the operation and ownership work can all be counted.
They are frequencies of executed source/codegen shapes, NOT CPU-time percentages,
machine instructions, exclusive cost categories, or a measurement of GPU/native work.
They describe expression entry (not guaranteed completion if an argument throws).
Repeated worker/nested VM rows are summed. Exact loop, typed operator, builtin,
short-circuit, nested-VM and worker counts were verified with TCC and MIR2, including
identical instrumented/uninstrumented results. Disabled codegen emits no counter calls.

WEIGHTING AND DESIGN
For program p, let T_p be the sum of all its counters and c_p(k) category k's count.
Within group g, use w_p = log(1 + T_p) / sum(q in g, log(1 + T_q)).
The target is sum(g, group_share_g * sum(p in g, w_p * c_p(k) / T_p)).
Thus a billion-event startup does not dwarf a thousand-event prototype, while longer
runs still receive somewhat greater weight. This is a choice of sampling policy, not
a claim that logarithmic weighting is uniquely correct. Failed/partial profiles get
no weight; the 50/30/20 group shares are preserved. In this capture the largest single
program weights within projects/tests-samples are only about 2.33%/2.80%.

There are 2893 target keys. The 18 keys at >=1% account for 50.44% of event mass;
the next 118 keys at 0.1-1% account for 35.34%; 334 at 0.01-0.1% add 12.12%.
The remaining 2423 keys together account for 2.10%. These orders of magnitude guided
the following broad workload choices; the 50/30/20 shares are source-corpus weights,
not literal timed sections or repetition ratios in this program:

  voxel_mix: global and layered class access, int3 coordinate/component operations,
    compact intx1 tagged cells, vectors of structs/classes, enum flags, early returns,
    getters/setters, integer shifts/indexing, float3 conversion and float4 colors.
    This large section follows Matvoxels' observed loading/data-access emphasis.
  entity_mix: nullable links, inline position/velocity structs inside heap objects,
    field and indexed mutation, float/scalar/vector arithmetic, short-circuit guards.
  grid_mix: nested [[class]] grids and int2 indexing, scalar/struct comparisons,
    switches, borrowed references, map/filter/fold and temporary neighbor links.
  numeric_mix: scalar floating math, float2/3/4 operations, vector/scalar and
    vector/vector arithmetic, array elements, dynamic component indexing, min/max,
    clamp, dot/cross/normalize, sqrt/sin/cos and integer-vector conversions.
  dispatch_mix: heterogeneous subclasses, virtual methods, super, type switches,
    runtime is-tests, explicitly typed function values and indirect calls.
  string_mix: short formatting/HUD-like strings, nullable text and string comparisons,
    arrays of reference-containing structs, class fields, dictionaries, tokenization,
    binary writes/reads with multiple results, appends, replacement and content hashes.
  value_mix: generic scalar/struct/string combinations, multiple returns, nested
    callbacks with nonlocal exits, and while loops. A deliberately small tail.
  collection_mix: sparse nullable vectors, map/filter_map/copy, flat integer buffers,
    push/remove, seeded random builtins, implicit int-to-float and string switches.

The first draft overemphasized floating arithmetic and missed the int3/intx1/global
object access visible in Matvoxels; the voxel section corrected that. Counter review
then prompted more sparse container/copy operations and short string/class-field work.
The FINAL default run (3000 rounds x 7 trials, seed 1, plus warmup) executed
21,666,727,077 events across 349 keys. It covers keys accounting for 85.58% of target
event mass. This is coverage of labels, not 85.58% accuracy or a performance score.
Family total-variation distance is 0.1056; exact-key distance is 0.3431. No broad
family differs by an order of magnitude. Arithmetic remains about 1.84x the target;
unary/conversion forms about 0.42x; builtin calls about 0.57x. These discrepancies
are retained explicitly instead of distorting ordinary code to fit decimal places.
Detailed target/benchmark frequencies, per-program totals and weights follow below.

OBSERVABILITY, VALIDATION AND LIMITS
Runtime command-line input reaches every kernel. Integer and float results both
escape into output. Final-state reductions consume array/object updates, including
the last iteration's stores; temporary grid links are cleared after their use to
avoid reference cycles. Integer sums and FNV hashes of string CONTENTS are used;
pointer hashes, wall-clock values and external inputs never enter checksums.
RNG state is seeded each round. Toleranced float goldens allow harmless rounding.

Seed 1 warmup/one-round golden: integer 87209904, float 55757.103997806626.
Default 3000-round golden: integer 292510068371, float 167842704.0174107.
Both are asserted, and every trial is checked against the first. TCC, MIR2, and
MSVC v145 /O2 /GL /LTCG C++ agree exactly for 1, 8 and 3000 rounds. Native default
seven-trial execution also passes. Benchmark profiling produces the same checksums.
The profile and validation source preceded only this explanatory comment; their
executable-body hash is recorded in raw/benchmark.manifest.json and validation.json.

This is a stable headless CPU/compiler workload. Native graphics, file/network waits,
audio, external resources and their internals are not emulated. Their real call-site
counters remain in the target (not silently deleted); caller-side data shapes are
represented, but absent native calls explain part of the residual mismatch. One save
and three early prototype frames do not cover all play, menus, editing or rare paths.
Some games are still mostly in menus; random/time-dependent startup can vary. Class
shape normalization and a compact synthetic working set do not preserve all cache,
allocation-size, control-flow correlation or instruction-footprint behavior. These
are explicit limits of this capture, not reasons to treat the old speedtest as truth.
Check actual application performance as well when evaluating compiler changes.

REPRODUCTION AND AUDIT FILES (relative to this directory)
README.md gives build/capture/analysis commands. collect_profiles.py inventories,
runs and archives the corpus; collect_matvoxels.py verifies the save and game-frame
capture; analyze_profiles.py reconstructs the weighted mixture and comparison;
verify_profile.py checks the instrumentation. corpus-profiles.zip includes the
captured manifest, every attempted program's status, raw TSVs and complete logs.
raw/matvoxels.* and raw/benchmark.* preserve the other captures. profile-summary.json
contains all exact categories and weights; validation.json records backend checks.
Do not include this benchmark itself when recollecting the real-program target.

DYNAMIC FEATURE MIXTURE (percent of overlapping counter events)
Family          Matvoxels   Projects   Tests/samples   Target   Benchmark
leaf               33.041     34.903          35.042   34.000    33.116
assign              9.493     14.738          14.986   12.165    12.750
field              18.104      4.817           2.902   11.078    13.057
op                  9.194      9.191          10.558    9.466    17.370
rc                  6.913      4.997           6.182    6.192     4.830
builtin             3.251      8.817           7.863    5.843     3.311
control             6.171      3.986           4.561    5.194     3.095
node                2.673      5.828           4.374    3.960     3.467
construct           1.953      3.837           4.409    3.010     2.087
loop                2.028      3.700           2.815    2.687     2.281
unary               2.126      1.372           2.170    1.909     0.801
index               1.934      1.993           1.479    1.861     1.685
branch              1.399      1.351           1.090    1.323     1.413
call                1.718      0.469           1.568    1.313     0.738

ORDER-OF-MAGNITUDE BUCKETS
Target frequency     Distinct keys    Combined target mass
at least 1%                     18                50.4418%
0.1% to 1%                     118                35.3350%
0.01% to 0.1%                  334                12.1197%
0.001% to 0.01%                527                 1.8770%
below 0.001%                  1896                 0.2266%

LARGEST EXACT TYPED / CONTEXT CATEGORIES
Target %    Benchmark %  Counter
  7.1976         6.8148  leaf|read|class|local
  5.9152         7.7614  leaf|read|int|local
  5.6875         8.1543  leaf|integer literal|int|value
  3.9993         2.8840  rc|decrement|Ref|attempt
  3.8622         5.0342  assign|define|int|effect
  3.2553         2.6231  leaf|floating point literal|float|value
  2.5564         2.5071  field|read|intx3->int|value
  2.5452         1.4035  control|if_then|void|effect
  2.1942         0.8835  field|read|class->class|value
  2.1926         1.9457  rc|increment|Ref|attempt
  1.7035         1.6247  leaf|read|class|global
  1.6117         1.5954  node|for loop element|int|value
  1.5117         1.3342  assign|define|class|effect
  1.4796         0.5403  loop|for_iteration|int|effect
  1.3530         0.7382  control|return|void|effect
  1.2017         3.7476  op|+|int,int->int|value
  1.1593         1.5826  op|*|int,int->int|value
  1.0156         0.8202  leaf|read|float|local
  0.9255         1.0749  node|for loop counter|int|value
  0.8490         1.9647  field|read|class->int|value
  0.8367         0.5994  construct|object_constructor|intx3|value
  0.8300         1.3322  leaf|read|intx3|local
  0.7583         0.1117  leaf|read|intx2|local
  0.6929         0.1024  assign|define|floatx4|effect
  0.6685         0.8329  assign|define|intx3|effect
  0.6208         0.3769  leaf|read|floatx2|local
  0.5842         0.6134  field|read|class->intx3|value
  0.5533         0.1117  assign|define|intx2|effect
  0.5487         0.1861  assign|define|floatx2|effect
  0.5372         0.1477  leaf|string literal|string|value
  0.5316         0.3071  construct|object_constructor|intx2|value
  0.5187         0.7306  branch|if_then_taken|void|effect
  0.5079         0.8112  leaf|read|vector<int>|local
  0.4981         0.8119  leaf|integer literal|enum|value
  0.4873         0.0894  node|def|function|value
  0.4821         0.4831  control|if_else|void|effect
  0.4615         0.3727  loop|for_iteration|vector<class>|effect
  0.4615         0.3727  node|for loop element|class|value
  0.4602         0.1170  assign|define|function|effect
  0.4585         0.2982  assign|define|float|effect
  0.4442         0.2978  field|read|class->float|value
  0.4396         0.0000  leaf|read|enum|local
  0.4207         0.8623  op|*|float,float->float|value
  0.4174         0.1675  field|read|intx2->int|value
  0.4118         1.3353  field|read|intx1->int|value
  0.4117         0.3069  branch|if_else_false|void|effect
  0.4069         0.0932  op|-|int,int->int|value
  0.4026         0.1001  control|for|void|effect
  0.4025         0.0062  unary|tofloat|int->float|value
  0.4017         0.2636  construct|object_constructor|floatx2|value
  0.4005         0.2357  op|==|int,int->bool|condition
  0.3976         0.1789  leaf|read|string|local
  0.3926         0.3759  branch|if_else_true|void|effect
  0.3843         0.2419  op|and|bool,bool->bool|condition
  0.3674         1.4254  field|read|class->vector<class>|value
  0.3605         0.1845  leaf|read|floatx3|local
  0.3361         0.1482  field|read|class->vector<int>|value
  0.3331         0.2295  field|read|class->vector<intx1>|value
  0.3078         0.1376  leaf|read|vector<class>|local
  0.3042         1.1105  op|&|int,int->int|value
  0.2977         0.0279  unary|is|class->bool|condition
  0.2969         0.8119  op|&|enum,enum->enum|condition
  0.2889         0.0558  field|read|class->string|value
  0.2867         0.0000  leaf|read|int|global
  0.2855         0.0651  builtin|rnd|int->int|value
  0.2766         0.1228  construct|object_constructor|class|value
  0.2748         0.1261  builtin|push|vector<class>,class->void|effect
  0.2728         0.2109  index|read|vector<intx1>,int->intx1|value
  0.2679         0.0031  construct|object_constructor|floatx4|value
  0.2671         0.3319  assign|define|floatx3|effect

SUCCESSFUL PROGRAMS (raw events; within-group log weight)
Program                                                                       Events   Weight %
matvoxels: 1 complete programs; overall share 50%
  projects/matvoxels/main.lobster                                              970,605,069   100.0000
projects: 77 complete programs; overall share 30%
  projects/_others/bbtest.lobster                                            1,142,488,983     2.2153
  projects/_others/chinese_remainder.lobster                                         1,744     0.7929
  projects/_others/negamax/tic4.lobster                                          6,762,343     1.6704
  projects/_others/robinhood.lobster                                                   195     0.5606
  projects/autoterrass.lobster                                                 669,061,939     2.1585
  projects/autoterrass_tear.lobster                                            904,429,371     2.1905
  projects/blendgrid/blendgrid.lobster                                               8,043     0.9552
  projects/blocklandscapepathfindtest.lobster                                   16,590,506     1.7658
  projects/blockydraw.lobster                                                          421     0.6421
  projects/blur.lobster                                                             88,238     1.2096
  projects/bulletheaven.lobster                                                      3,482     0.8663
  projects/cdrl.lobster                                                            130,148     1.2508
  projects/circular_fisheye_stretch/circular_fisheye_stretch.lobster                   468     0.6533
  projects/circular_fisheye_stretch/equirec2circular.lobster                           460     0.6515
  projects/closest_whole_number_ratio.lobster                                        2,987     0.8500
  projects/construction/construction.lobster                                    15,596,202     1.7592
  projects/driving.lobster                                                         241,590     1.3165
  projects/extradof/extradof.lobster                                                   381     0.6315
  projects/fluidcity.lobster                                                         5,595     0.9166
  projects/goose/goose.lobster                                                       2,964     0.8492
  projects/gradient_descent.lobster                                          3,400,650,256     2.3311
  projects/gridtraffic/gridtraffic.lobster                                       1,429,152     1.5054
  projects/imgratio.lobster                                                        148,296     1.2647
  projects/linestacking.lobster                                                 23,350,879     1.8021
  projects/ludumdare30.lobster                                                   8,625,627     1.6963
  projects/match3flick.lobster                                                       5,998     0.9240
  projects/mirroring/mirrorings.lobster                                             10,689     0.9854
  projects/name_generator.lobster                                                1,685,233     1.5229
  projects/nicepixels/nicepixels.lobster                                               401     0.6369
  projects/offsetcraft.lobster                                                 246,057,496     2.0522
  projects/offsetcraft_surfnet.lobster                                       1,721,852,726     2.2589
  projects/packing.lobster                                                         115,874     1.2385
  projects/premade_td.lobster                                                    1,129,151     1.4803
  projects/procgencraft.lobster                                                      3,824     0.8762
  projects/procgenupgrade.lobster                                                    3,972     0.8802
  projects/projecteuler.lobster                                                          3     0.1472
  projects/puzzlepixel.lobster                                                  11,511,086     1.7269
  projects/puzzlewindow/puzzlewindow.lobster                                   115,919,732     1.9723
  projects/qixelsample.lobster                                                         495     0.6592
  projects/quadabstract/quadabstract.lobster                                   448,838,001     2.1161
  projects/quadtree.lobster                                                        915,342     1.4580
  projects/quadtree_texture_shader.lobster                                             298     0.6055
  projects/raytracing/raytrace_compute.lobster                                       1,281     0.7601
  projects/raytracing/raytrace_compute_scenetexture_fatstack.lobster                13,136     1.0073
  projects/raytracing/raytrace_compute_sceneubo_thinstack.lobster                   13,187     1.0077
  projects/raytracing/raytrace_compute_sceneubo_thinstack_cubemap.lobster           13,160     1.0075
  projects/raytracing/raytrace_compute_sceneubo_thinstack_points.lobster       135,279,477     1.9887
  projects/raytracing/raytrace_compute_sceneubo_thinstack_points_min.lobster     135,279,452     1.9887
  projects/raytracing/raytrace_compute_sceneubo_thinstack_temporal.lobster          13,419     1.0095
  projects/raytracing/raytrace_compute_sceneuniforms_fatstack.lobster               13,117     1.0071
  projects/raytracing/raytrace_compute_sceneuniforms_thinstack.lobster              13,117     1.0071
  projects/raytracing/raytrace_ps.lobster                                            1,196     0.7528
  projects/regularmining.lobster                                                    43,722     1.1350
  projects/rndgrid/rndgrid.lobster                                               1,631,879     1.5194
  projects/rndgrid/rndgrid2.lobster                                             32,259,565     1.8364
  projects/rndgrid/rndgrid3.lobster                                            395,876,920     2.1027
  projects/rndgrid/rndgrid3_path.lobster                                        48,361,815     1.8794
  projects/rotsample.lobster                                                           516     0.6636
  projects/shooter.lobster                                                         126,479     1.2478
  projects/sidestrat/sidestrat.lobster                                             247,193     1.3190
  projects/smoothhex.lobster                                                    13,002,196     1.7399
  projects/squarify/squarify.lobster                                                   533     0.6671
  projects/tapdefense/tapdefense.lobster                                           334,052     1.3510
  projects/taprace.lobster                                                          51,399     1.1522
  projects/trafficjam.lobster                                                       92,938     1.2151
  projects/voxel3d/voxel3dtest.lobster                                           4,477,022     1.6266
  projects/voxel3d/voxel3dtest2.lobster                                             81,815     1.2015
  projects/voxel3d/voxel3dtest3.lobster                                          4,240,121     1.6209
  projects/voxel3d/voxel3dtest3_simplex_random_mirror.lobster                      375,598     1.3634
  projects/voxel3d/voxel3dtest4.lobster                                         23,704,195     1.8037
  projects/voxel3d/voxel3dtest4_extra_detail_test.lobster                        6,137,561     1.6602
  projects/voxel3d/voxel3dtest4_transparency_test.lobster                        6,463,243     1.6656
  projects/vrvoxelpaint/vrvoxelpaint.lobster                                        44,489     1.1368
  projects/water.lobster                                                        92,895,360     1.9487
  projects/water_td.lobster                                                          1,223     0.7552
  projects/watershooter.lobster                                                      5,723     0.9190
  projects/zombietele.lobster                                                        4,245     0.8873
tests_samples: 90 complete programs; overall share 20%
  samples/binary.lobster                                                             5,214     1.1595
  samples/bspgraphic.lobster                                                       255,101     1.6865
  samples/cgtest.lobster                                                           360,896     1.7335
  samples/collision.lobster                                                        335,561     1.7236
  samples/cube.lobster                                                               4,432     1.1375
  samples/custom_shader_metaballs.lobster                                              288     0.7676
  samples/custom_shader_sobel.lobster                                               24,331     1.3682
  samples/demos/custom_shader_pt/custom_shader_pt.lobster                              658     0.8793
  samples/demos/dirbrowse.lobster                                                   21,728     1.3528
  samples/demos/langtons.lobster                                                    99,608     1.5591
  samples/demos/simple_parser_eval.lobster                                           2,078     1.0349
  samples/demos/smallpt.lobster                                                 61,531,426     2.4296
  samples/dragtest.lobster                                                             467     0.8329
  samples/dungeongen.lobster                                                     1,849,383     1.9548
  samples/dynamic_dispatch.lobster                                                      28     0.4562
  samples/eulerspiral.lobster                                                          786     0.9033
  samples/hypocycloid.lobster                                                      670,442     1.8174
  samples/image.lobster                                                                392     0.8093
  samples/imguitest.lobster                                                          3,104     1.0893
  samples/iqmtest/iqmtest.lobster                                                    1,738     1.0107
  samples/knights_tour.lobster                                                      63,862     1.4989
  samples/lazy_py_triples.lobster                                               16,019,740     2.2473
  samples/lobstercraft.lobster                                                  73,256,040     2.4532
  samples/mazepattern.lobster                                                      150,385     1.6149
  samples/pendulum.lobster                                                           6,471     1.1888
  samples/physics_boxes.lobster                                                        426     0.8205
  samples/physics_water.lobster                                                        648     0.8772
  samples/platform/t2s.lobster                                                          34     0.4816
  samples/pythtree.lobster                                                       1,431,702     1.9202
  samples/reach.lobster                                                              2,704     1.0706
  samples/rosettacode/fizzbuzz.lobster                                               3,518     1.1062
  samples/rosettacode/hello_world_graphical.lobster                                     84     0.6018
  samples/rosettacode/hello_world_test.lobster                                           4     0.2180
  samples/rosettacode/quicksort.lobster                                              1,242     0.9652
  samples/rosettacode/quine.lobster                                                     17     0.3915
  samples/shooter_tutorial/tut1.lobster                                                235     0.7402
  samples/shooter_tutorial/tut2.lobster                                                296     0.7713
  samples/shooter_tutorial/tut3.lobster                                                469     0.8335
  samples/shooter_tutorial/tut4.lobster                                                964     0.9309
  samples/shooter_tutorial/tut5.lobster                                              1,476     0.9886
  samples/shooter_tutorial/tut6.lobster                                                436     0.8236
  samples/shooter_tutorial/tut_obj.lobster                                             392     0.8093
  samples/shooter_tutorial/tut_sound.lobster                                           418     0.8179
  samples/sierpinski.lobster                                                       233,092     1.6743
  samples/textinput.lobster                                                            334     0.7876
  samples/threads.lobster                                                      977,393,037     2.8042
  samples/tixy.lobster                                                             370,321     1.7370
  samples/tixy3D.lobster                                                           597,920     1.8019
  samples/vonkoch.lobster                                                          362,781     1.7342
  samples/vrtest.lobster                                                           136,732     1.6020
  samples/wave_function_collapse.lobster                                               431     0.8221
  samples/wavtest.lobster                                                              356     0.7962
  samples/will_it_shuffle.lobster                                               43,079,160     2.3813
  tests/astartest.lobster                                                          170,673     1.6320
  tests/builtintest.lobster                                                            114     0.6428
  tests/ccodetest.lobster                                                              176     0.7012
  tests/deep/test.lobster                                                           55,975     1.4810
  tests/floodtest.lobster                                                          157,227     1.6209
  tests/freevars.lobster                                                               158     0.6867
  tests/goaptest.lobster                                                            51,879     1.4707
  tests/gradienttest.lobster                                                     3,422,080     2.0382
  tests/knightstest.lobster                                                         63,809     1.4988
  tests/lifetimetest.lobster                                                            67     0.5716
  tests/matrixtest.lobster                                                             682     0.8841
  tests/misc/control.lobster                                                            44     0.5157
  tests/misc/dd_with_no_root_freevars.lobster                                          271     0.7594
  tests/misc/default_arg_overlap.lobster                                                22     0.4248
  tests/misc/frame.lobster                                                             478     0.8361
  tests/misc/function_def_arg.lobster                                                    8     0.2977
  tests/misc/keep_dispatch.lobster                                                      60     0.5569
  tests/misc/multi_ret_if.lobster                                                       42     0.5095
  tests/misc/namespace_error1.lobster                                                    3     0.1878
  tests/misc/nil_overload.lobster                                                       10     0.3248
  tests/misc/nil_vtable_entry_for_unused_subclass.lobster                              217     0.7294
  tests/misc/overload_selection_on_given_types.lobster                                  28     0.4562
  tests/misc/ref_structs_must_be_forced_to_keep.lobster                                 60     0.5569
  tests/misc/specialized_field_defaults.lobster                                         21     0.4187
  tests/misc/var_remains.lobster                                                         7     0.2817
  tests/misc/void_constructor.lobster                                                    6     0.2636
  tests/mischtest.lobster                                                              219     0.7307
  tests/misctest.lobster                                                            12,907     1.2823
  tests/operators.lobster                                                              595     0.8657
  tests/parsertest.lobster                                                           2,045     1.0327
  tests/smallpttest.lobster                                                      5,531,770     2.1033
  tests/springstest.lobster                                                      3,838,547     2.0538
  tests/stringtest.lobster                                                             167     0.6941
  tests/structtest.lobster                                                           1,730     1.0101
  tests/typeswitch.lobster                                                             668     0.8813
  tests/typetest.lobster                                                             1,478     0.9888
  tests/watertest.lobster                                                        2,438,635     1.9923

FAILED ATTEMPTS (excluded from mixture; raw partial counters and full logs retained)
  projects/blueprintrts/blueprintrts.lobster: failed; process crash, exit 3
  projects/boundedvoxels/boundedvoxels.lobster: failed; boundedvoxels.lobster(446): error: unknown field/function reference `oc_load`
  projects/camspacemesh/camspacemesh.lobster: failed; camspacemesh.lobster(15): error: unknown field/function reference `create_camspace_mesh`
  projects/cityclicker.lobster: failed; process crash, exit 3
  projects/cityclicker2.lobster: failed; process crash, exit 3
  projects/compressedvoxels/compressedvoxels.lobster: failed; compressedvoxels.lobster(481): error: unknown field/function reference `cv_init`
  projects/inlinevoxels/inlinevoxels.lobster: failed; inlinevoxels.lobster(455): error: unknown field/function reference `oc_load`
  projects/matchrpg/matchrpg.lobster: failed; process crash, exit 3
  projects/prefabvoxels/kitbashvoxelsexperiment.lobster: failed; kitbashvoxelsexperiment.lobster(981): error: unknown field/function reference `aabb_tree`
  projects/prefabvoxels/prefabvoxels.lobster: failed; prefabvoxels.lobster(1002): error: unknown field/function reference `oc_load`
  projects/raytracing/raytrace_compute_sceneubo_thinnerstack.lobster: failed; fatal error: linking failed for shader: raytrace
  projects/raytracing/raytrace_cpu.lobster: failed; raytrace_cpu.lobster(21): error: unknown field/function reference `raytrace_cpu`
  projects/resclicker.lobster: failed; process crash, exit 3
  projects/rpgclicker.lobster: failed; process crash, exit 3
  projects/rtsclicker.lobster: failed; process crash, exit 3
  projects/scrollrpg/scrollrpgfow.lobster: failed; libtcc JIT error: scrollrpgfow.lobster:
  projects/settlement/settlement.lobster: failed; libtcc JIT error: settlement.lobster:
  projects/shrinkrpg/shrinkrpg.lobster: failed; process crash, exit 3
  projects/stockclicker.lobster: failed; process crash, exit 3
  projects/stretchsdf/stretchsdf.lobster: failed; stretchsdf.lobster(15): error: unknown field/function reference `create_sdf_mesh`
  projects/totslike/totslike.lobster: failed; libtcc JIT error: totslike.lobster:
  projects/trafficsim/trafficsim.lobster: failed; process crash, exit 3
  samples/guitest.lobster: failed; libtcc JIT error: guitest.lobster:
  samples/iconfont.lobster: failed; VM error (iconfont.lobster): gl.set_font_size: no current font set with gl.set_font_name
  samples/mgtest.lobster: failed; process crash, exit 3

Unattempted eligible entrypoints: 0
Explicit exclusions and all source hashes: corpus.json; raw counters and logs: corpus-profiles.zip.

Comparison diagnostics (descriptive, not optimization targets):
  Exact-category total variation distance: 0.3431
  Target event mass with a nonzero exact benchmark category: 85.58%
*/

import std
import vec
import dictionary

enum_flags BenchFlags:
    bench_solid = 1
    bench_visible
    bench_active

struct BenchVoxel:
    bits:int

class BenchPalette:
    colors:[float4]
    mask:int
    name:string

class BenchChunk:
    origin:int3
    voxels:[BenchVoxel]
    flags:BenchFlags
    palette:BenchPalette

class BenchWorld:
    chunks:[BenchChunk]
    palette:BenchPalette
    generation:int

let bench_palette = BenchPalette { [float4 { 0.25, 0.5, 0.75, 1.0 }, float4_1], 63, "terrain" }
var bench_world = BenchWorld { [], bench_palette, 0 }

// Globals and layered class access are a large part of the measured voxel game.
// A one-int struct represents compact tagged cells; this is intentionally a
// collection of access/update shapes, not a voxel mesher or traversal algorithm.
def voxel_at(p:int3):
    let chunk = bench_world.chunks[(p.z / 4) % bench_world.chunks.length]
    let local = (p - chunk.origin + 32) % 8
    let index = (local.x + local.y * 8 + local.z * 64) & chunk.palette.mask
    if not (chunk.flags & bench_active): return BenchVoxel { 0 }
    return chunk.voxels[index]

def voxel_write(p:int3, value:BenchVoxel):
    let chunk = bench_world.chunks[(p.z / 4) % bench_world.chunks.length]
    let local = (p - chunk.origin + 32) % 8
    let index = (local.x + local.y * 8 + local.z * 64) & chunk.palette.mask
    if chunk.flags & bench_solid:
        chunk.voxels[index] = value
    else:
        chunk.voxels[index] = BenchVoxel { value.bits / 2 }

def voxel_mix(seed:int):
    bench_world = BenchWorld {
        map(8) i:
            BenchChunk { int3 { i % 2, (i / 2) % 2, i / 4 } * 4,
                         map(64): BenchVoxel { (_ + i + seed) % 31 },
                         BenchFlags(1 + i % 7), bench_palette }
        , bench_palette, seed
    }
    var checksum = 0
    var measure = 0.0
    for(4) pass:
        for(512) i:
            let p = int3 { i & 7, (i >> 3) & 7, (i >> 6) + pass * 4 }
            let center = voxel_at(p)
            let neighbor = voxel_at(p + int3 { 1, 0, 0 })
            let below = voxel_at(p - int3 { 0, 0, 1 })
            if center.bits == neighbor.bits:
                checksum += center.bits
            if center.bits != below.bits:
                voxel_write(p, BenchVoxel { (center.bits + below.bits + pass) & 31 })
                checksum += p.x + p.y * 8 + p.z * 64
            let chunk = bench_world.chunks[i % 8]
            if chunk.flags & bench_visible:
                let color = chunk.palette.colors[center.bits % 2]
                let pos = float(p) * 0.125
                measure += color.x * pos.x + color.y * pos.y + color.z * pos.z + color.w
            checksum += neighbor.bits + bench_world.generation
    for(bench_world.chunks) chunk:
        for(chunk.voxels) voxel: checksum += voxel.bits
    return checksum, measure

struct BenchPose:
    position:float3
    velocity:float3

class BenchEntity:
    pose:BenchPose
    health:float
    flags:int
    target:BenchEntity? = nil
    inventory:[int]

// Mutable object graphs, nullable links, inline structs in heap objects, scalar
// comparisons, vector/scalar arithmetic, borrowed array iteration and field writes.
def entity_mix(seed:int):
    let leader = BenchEntity { BenchPose { float3 { 3.0, 4.0, 5.0 }, float3_0 }, 64.0, seed, [1] }
    let entities = map(384) i:
        BenchEntity {
            BenchPose { float3 { float(i % 17), float(i % 13), float(seed % 11) },
                        float3 { 0.25, -0.5, 0.125 } },
            float((i + seed) % 64 + 1), (i + seed) % 16,
            target: if i % 3: leader else: nil,
            inventory: [i % 7, i % 11, seed % 13]
        }
    var checksum = seed
    var measure = 0.0
    for(3) frame:
        for(entities) e, i:
            let p = e.pose.position
            var delta = float3 { 0.5, 0.25, -0.125 }
            if e.target and e.health > 8.0:
                delta = (e.target.pose.position - p) * 0.03125
                e.flags = (e.flags + frame + 1) & 15
                checksum += e.target.flags
            else:
                e.health += 0.25
                checksum -= e.flags
            e.pose.velocity = e.pose.velocity * 0.75 + delta
            e.pose.position += e.pose.velocity * 0.125
            e.health = min(96.0, max(0.0, e.health - 0.125 * float(e.flags)))
            if e.flags & 1 and e.pose.position.z >= 0.0:
                e.inventory[i % 3] += frame
                checksum += e.inventory[(i + 1) % 3]
            else:
                e.pose.position.z += 0.5
                checksum += i
            measure += e.pose.position.x + e.pose.position.y * 0.5 + e.health
    for(entities) e:
        checksum += sum(e.inventory)
    return checksum, measure

class BenchCell:
    kind:int
    amount:int
    direction:int2
    neighbor:BenchCell? = nil

// Nested vectors indexed by both scalars and int2; integer struct comparisons,
// switches, compound assignments, and collection callbacks which capture state.
def grid_mix(seed:int):
    let grid = map(24) y:
        map(24) x: BenchCell { (x + y + seed) % 4, (x * 3 + y + seed) % 32, int2 { x, y } }
    var checksum = 0
    for(2) pass:
        for(grid) row, y:
            for(row) cell, x:
                let at = (cell.direction + int2 { 1, pass }) % 24
                let other = grid[at]
                cell.amount += other.kind + pass
                switch cell.kind:
                    case 0: cell.amount = (cell.amount * 3 + seed) % 127
                    case 1: cell.amount -= other.amount / 4
                    case 2: cell.direction = (cell.direction + int2 { 1, 2 }) % 24
                    default: cell.amount += x - y
                if cell.direction != int2 { x, y }:
                    checksum += cell.direction.x - cell.direction.y
                if other != cell and other.amount > cell.amount:
                    cell.neighbor = other
                else:
                    cell.neighbor = nil
                checksum += if cell.neighbor: cell.neighbor.amount else: cell.amount
        let chosen = filter(grid[(seed + pass) % 24]): _.amount > pass
        let values = map(chosen): _.amount + seed
        checksum += fold(values, 0): _a + _b
    // Every mutated field remains observable; break links before releasing grid.
    for(grid) row:
        for(row) c:
            checksum += c.amount + c.direction.x + c.direction.y
            c.neighbor = nil
    return checksum, 0.0

// Scalar floating point math remains common even in the graphics-heavy sources.
// Keep 2/3/4-wide math as well, with variables on both sides and nonconstant indices.
def numeric_mix(seed:int):
    let positions = map(384) i: float2 { float((i + seed) % 31) * 0.125, float(i % 23) * 0.25 }
    let weights = map(384) i: float(i % 7 + 1) * 0.0625
    var checksum = seed
    var measure = 0.0
    for(2) pass:
        for(positions) p, i:
            let w = weights[i]
            var q = p * w + float2 { 0.25, 0.5 }
            q -= p / (w + 1.0)
            positions[i] = clamp(q, float2 { -8.0, -8.0 }, float2 { 8.0, 8.0 })
            let d = dot(q, p)
            let distance = sqrt(abs(d) + 1.0)
            let scale = min(2.0, max(0.25, distance / (w + 1.0)))
            weights[i] = (w + scale * 0.03125) * 0.875
            if d < w or q.x > q.y:
                measure += distance + q.x * q.y
                checksum += i + pass
            else:
                measure -= distance * 0.125
                checksum -= pass
            measure += sin(w) + cos(q.y) + weights[i]
    for(positions) p, i:
        measure += p.x + p.y + weights[i]
    for(32) i:
        let v = float3 { weights[i], positions[i].x, positions[i].y }
        let n = normalize(v + float3_1)
        let c = cross(v, n)
        let color = float4 { n.x, n.y, n.z, 1.0 } * 0.75 + 0.125
        measure += c.x + c.y + c.z + color[i % 4]
        let coord = int3 { i, seed % 17, i % 5 } * 2 + 1
        checksum += coord[i % 3] + manhattan(coord % 11)
    return checksum, measure

class BenchRule:
    bias:int
    def apply(value:int): return value + bias
class BenchScale : BenchRule
    factor:int
    def apply(value:int): return value * factor - bias
class BenchOffset : BenchRule
    offset:int
    def apply(value:int): return (super apply(value)) + offset

def bench_action(value:int) -> int

def dispatch_mix(seed:int):
    def make_rule(i:int) -> BenchRule:
        return switch i % 3:
            case 0: BenchRule { seed % 7 }
            case 1: BenchScale { seed % 5, i % 5 + 1 }
            default: BenchOffset { i % 7, seed % 11 }
    let rules = map(96): make_rule(_)
    let actions:[bench_action] = [fn x: x + 3, fn x: x * 2, fn x: x - 1]
    var checksum = 0
    for(3) pass:
        for(rules) rule, i:
            let n = rule.apply(i + pass)
            let action = actions[(i + seed) % actions.length]
            checksum += action(n)
            switch rule:
                case BenchScale: checksum += rule.factor
                case BenchOffset: checksum -= rule.offset
                default: checksum += rule.bias
            if rule is BenchScale: checksum += 1
    return checksum, 0.0

struct BenchLabel:
    text:string
    values:[int]
    tag:int

class BenchMessage:
    title:string
    detail:string?
    payload:BenchLabel

// Short strings and binary save/network-like records; real builtin calls plus
// generic dictionaries, mutable reference vectors and structs containing refs.
def string_mix(seed:int):
    let names = ["stone", "wood", "metal", "water", "light", "grass", "sand", "brick"]
    let counts = dictionary<string, int>(67)
    let records = map(192) i:
        BenchLabel { names[(i + seed) % names.length] + ":" + string(i % 7), [i, seed % 17], i }
    var checksum = 0
    var buffer = ""
    var offset = 0
    for(records) record, i:
        let parts = tokenize(record.text, ":", "")
        counts.add(parts[0], record.values[0] + 1)
        checksum += string_to_int(parts[1]) + parts[0].length
        let label = substring(record.text, 0, min(3, record.text.length))
        checksum += label[0] + label[label.length - 1]
        buffer, offset = buffer.write_int32_le(offset, record.tag + seed)
        if i % 3 == 0:
            records[i] = BenchLabel { record.text + "!", append(record.values, [i + 1]), record.tag + 1 }
        else:
            records[i].values.push(i)
    for(names) name:
        checksum += counts.get(name, 0)
    for(records) record, i:
        checksum += buffer.read_int32_le(i * 4) + record.tag + record.text.length
        checksum += sum(record.values)
    let joined = concat_string(map(records): _.text, ",")
    checksum += joined.length + (hash(joined) & 65535)
    let messages = map(records) record:
        BenchMessage { record.text, if record.tag & 1: "enabled" else: nil, record }
    for(messages) message:
        let line = "item: " + message.title + " status=" + (message.detail or "none") +
                   " value=" + string(message.payload.tag)
        checksum += hash(line) & 65535
        if message.detail and message.detail != "disabled":
            checksum += message.payload.values[0]
        else:
            checksum -= message.title.length
    return checksum, 0.0

// Small tail of generic value/reference permutations, multiple returns and
// nonlocal callback returns; these should not dominate the common game shapes.
def bench_pair(a, b): return a + b, a

def find_match(values:[int], wanted:int):
    for(values) n:
        do():
            if n == wanted: return n + 1 from find_match
    return -1

def value_mix(seed:int):
    var checksum = 0
    var measure = 0.0
    let values = map(32): (_ + seed) % 23
    for(16) i:
        let a, b = bench_pair(values[i], values[i + 1])
        let p, q = bench_pair(float2 { float(a), 0.5 }, float2 { 0.25, float(b) })
        let text, old = bench_pair(string(a), string(b))
        checksum += a + b + text.length + old.length + find_match(values, i)
        measure += p.x + p.y + q.x + q.y
        var n = i % 5
        while n > 0:
            checksum += values[n]
            n--
    return checksum, measure

def collection_mix(seed:int):
    rnd_seed(seed)
    let sparse = map(64) i:
        if i % 3: map(16): rnd(31) else: nil
    var checksum = 0
    var measure = 0.0
    for(4) pass:
        let selected = filter_map(sparse) row:
            if row: copy(row) else: nil
        let flat = []::int
        for(selected) row:
            for(row) n: flat.push(n)
        let changed = map(flat): _ + pass
        checksum += sum(changed)
        let replacement = map(16): changed[(_ + seed) % changed.length]
        sparse[(seed + pass) % sparse.length] = replacement
        for(replacement) value:
            measure += value * 0.125
        let label = if pass & 1: "active" else: "idle"
        switch label:
            case "active": checksum += flat.remove(flat.length - 1)
            case "idle": checksum -= flat[0]
            default: checksum += label.length
        checksum += flat.length
    for(sparse) row:
        if row: checksum += sum(row)
    return checksum, measure

def benchmark_round(seed:int):
    let v, vf = voxel_mix(seed)
    let a, af = entity_mix(seed)
    let b, bf = grid_mix(seed)
    let c, cf = numeric_mix(seed)
    let d, df = dispatch_mix(seed)
    let e, ef = string_mix(seed)
    let f, ff = value_mix(seed)
    let g, gf = collection_mix(seed)
    return v + a * 3 + b * 5 + c * 7 + d * 11 + e * 13 + f * 17 + g * 19, vf + af + bf + cf + df + ef + ff + gf

let arguments = command_line_arguments()
let iterations = if arguments.length > 0: string_to_int(arguments[0]) else: 3000
let trials = if arguments.length > 1: string_to_int(arguments[1]) else: 7
let seed = if arguments.length > 2: string_to_int(arguments[2]) else: 1
assert iterations > 0 and iterations <= 100000
assert trials > 0 and trials <= 100
assert seed >= 0 and seed <= 1000000

// Runtime input reaches every kernel. Each trial reconstructs identical state;
// output and assertions consume both the integer and floating point results.
let warm_i, warm_f = benchmark_round(seed)
if seed == 1:
    assert warm_i == 87209904
    assert abs(warm_f - 55757.103997806626) < 0.000001
var expected_i = 0
var expected_f = 0.0
let times = []
for(trials) trial:
    var result_i = 0
    var result_f = 0.0
    let start = seconds_elapsed()
    for(iterations) iteration:
        let ri, rf = benchmark_round(seed + iteration)
        result_i += ri
        result_f += rf
    times.push(seconds_elapsed() - start)
    if seed == 1 and iterations == 3000:
        assert result_i == 292510068371
        assert abs(result_f - 167842704.0174107) < 0.01
    if trial == 0:
        expected_i = result_i
        expected_f = result_f
    else:
        assert result_i == expected_i
        assert abs(result_f - expected_f) <= 0.0000001 * max(1.0, abs(expected_f))
let sorted_times = times.qsort(): _a < _b
print "benchmark checksum: {expected_i}, {expected_f}; warmup: {warm_i}, {warm_f}"
print "{iterations} rounds, {trials} trials: min {sorted_times[0]} s, median {sorted_times[trials / 2]} s"
