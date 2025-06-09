#include "lobster/stdafx.h"
#include "lobster/vmdata.h"
#include "lobster/vmops.h"
#include "lobster/compiler.h"

typedef lobster::Value Value;
typedef lobster::StackPtr StackPtr;
typedef lobster::VM &VMRef;
typedef lobster::fun_base_t fun_base_t;

#if LOBSTER_ENGINE
    // FIXME: This makes SDL not modular, but without it it will miss the SDLMain indirection.
    #include "lobster/sdlincludes.h"
    #include "lobster/sdlinterface.h"
    extern "C" void GLFrame(StackPtr sp, VMRef vm);
#endif

extern const int funinfo_table[];


static void fun_10000001(VMRef vm, StackPtr psp) {
    Value regs[1];
    (void)regs;
    U_ABORT(vm, regs + 0);
}
static void fun_0(VMRef, StackPtr);
static void fun_3(VMRef, StackPtr);
static void fun_4(VMRef, StackPtr);
static void fun_5(VMRef, StackPtr);
static void fun_6(VMRef, StackPtr);
static void fun_616(VMRef, StackPtr);
static void fun_615(VMRef, StackPtr);
static void fun_614(VMRef, StackPtr);
static void fun_613(VMRef, StackPtr);
static void fun_612(VMRef, StackPtr);
static void fun_611(VMRef, StackPtr);
static void fun_604(VMRef, StackPtr);
static void fun_603(VMRef, StackPtr);
static void fun_602(VMRef, StackPtr);
static void fun_601(VMRef, StackPtr);
static void fun_593(VMRef, StackPtr);
static void fun_592(VMRef, StackPtr);
static void fun_587(VMRef, StackPtr);
static void fun_586(VMRef, StackPtr);
static void fun_585(VMRef, StackPtr);
static void fun_584(VMRef, StackPtr);
static void fun_582(VMRef, StackPtr);
static void fun_581(VMRef, StackPtr);
static void fun_579(VMRef, StackPtr);
static void fun_576(VMRef, StackPtr);
static void fun_573(VMRef, StackPtr);
static void fun_566(VMRef, StackPtr);
static void fun_565(VMRef, StackPtr);
static void fun_564(VMRef, StackPtr);
static void fun_563(VMRef, StackPtr);
static void fun_562(VMRef, StackPtr);
static void fun_558(VMRef, StackPtr);
static void fun_543(VMRef, StackPtr);
static void fun_542(VMRef, StackPtr);
static void fun_532(VMRef, StackPtr);
static void fun_505(VMRef, StackPtr);
static void fun_502(VMRef, StackPtr);
static void fun_492(VMRef, StackPtr);
static void fun_485(VMRef, StackPtr);
static void fun_482(VMRef, StackPtr);
static void fun_471(VMRef, StackPtr);
static void fun_7(VMRef, StackPtr);
static void fun_567(VMRef, StackPtr);
static void fun_10(VMRef, StackPtr);
static void fun_595(VMRef, StackPtr);
static void fun_590(VMRef, StackPtr);
static void fun_13(VMRef, StackPtr);
static void fun_600(VMRef, StackPtr);
static void fun_599(VMRef, StackPtr);
static void fun_568(VMRef, StackPtr);
static void fun_15(VMRef, StackPtr);
static void fun_572(VMRef, StackPtr);
static void fun_571(VMRef, StackPtr);
static void fun_570(VMRef, StackPtr);
static void fun_559(VMRef, StackPtr);
static void fun_481(VMRef, StackPtr);
static void fun_17(VMRef, StackPtr);
static void fun_598(VMRef, StackPtr);
static void fun_19(VMRef, StackPtr);
static void fun_597(VMRef, StackPtr);
static void fun_23(VMRef, StackPtr);
static void fun_26(VMRef, StackPtr);
static void fun_472(VMRef, StackPtr);
static void fun_29(VMRef, StackPtr);
static void fun_30(VMRef, StackPtr);
static void fun_594(VMRef, StackPtr);
static void fun_588(VMRef, StackPtr);
static void fun_48(VMRef, StackPtr);
static void fun_596(VMRef, StackPtr);
static void fun_591(VMRef, StackPtr);
static void fun_49(VMRef, StackPtr);
static void fun_50(VMRef, StackPtr);
static void fun_51(VMRef, StackPtr);
static void fun_52(VMRef, StackPtr);
static void fun_476(VMRef, StackPtr);
static void fun_70(VMRef, StackPtr);
static void fun_71(VMRef, StackPtr);
static void fun_72(VMRef, StackPtr);
static void fun_79(VMRef, StackPtr);
static void fun_89(VMRef, StackPtr);
static void fun_95(VMRef, StackPtr);
static void fun_98(VMRef, StackPtr);
static void fun_574(VMRef, StackPtr);
static void fun_100(VMRef, StackPtr);
static void fun_577(VMRef, StackPtr);
static void fun_101(VMRef, StackPtr);
static void fun_580(VMRef, StackPtr);
static void fun_102(VMRef, StackPtr);
static void fun_110(VMRef, StackPtr);
static void fun_111(VMRef, StackPtr);
static void fun_112(VMRef, StackPtr);
static void fun_113(VMRef, StackPtr);
static void fun_114(VMRef, StackPtr);
static void fun_115(VMRef, StackPtr);
static void fun_116(VMRef, StackPtr);
static void fun_117(VMRef, StackPtr);
static void fun_118(VMRef, StackPtr);
static void fun_119(VMRef, StackPtr);
static void fun_120(VMRef, StackPtr);
static void fun_121(VMRef, StackPtr);
static void fun_122(VMRef, StackPtr);
static void fun_123(VMRef, StackPtr);
static void fun_124(VMRef, StackPtr);
static void fun_125(VMRef, StackPtr);
static void fun_126(VMRef, StackPtr);
static void fun_127(VMRef, StackPtr);
static void fun_477(VMRef, StackPtr);
static void fun_128(VMRef, StackPtr);
static void fun_478(VMRef, StackPtr);
static void fun_129(VMRef, StackPtr);
static void fun_479(VMRef, StackPtr);
static void fun_130(VMRef, StackPtr);
static void fun_134(VMRef, StackPtr);
static void fun_537(VMRef, StackPtr);
static void fun_535(VMRef, StackPtr);
static void fun_507(VMRef, StackPtr);
static void fun_497(VMRef, StackPtr);
static void fun_495(VMRef, StackPtr);
static void fun_490(VMRef, StackPtr);
static void fun_488(VMRef, StackPtr);
static void fun_483(VMRef, StackPtr);
static void fun_138(VMRef, StackPtr);
static void fun_534(VMRef, StackPtr);
static void fun_494(VMRef, StackPtr);
static void fun_487(VMRef, StackPtr);
static void fun_139(VMRef, StackPtr);
static void fun_144(VMRef, StackPtr);
static void fun_536(VMRef, StackPtr);
static void fun_496(VMRef, StackPtr);
static void fun_489(VMRef, StackPtr);
static void fun_145(VMRef, StackPtr);
static void fun_531(VMRef, StackPtr);
static void fun_504(VMRef, StackPtr);
static void fun_491(VMRef, StackPtr);
static void fun_484(VMRef, StackPtr);
static void fun_151(VMRef, StackPtr);
static void fun_153(VMRef, StackPtr);
static void fun_156(VMRef, StackPtr);
static void fun_159(VMRef, StackPtr);
static void fun_160(VMRef, StackPtr);
static void fun_162(VMRef, StackPtr);
static void fun_163(VMRef, StackPtr);
static void fun_164(VMRef, StackPtr);
static void fun_165(VMRef, StackPtr);
static void fun_166(VMRef, StackPtr);
static void fun_167(VMRef, StackPtr);
static void fun_480(VMRef, StackPtr);
static void fun_168(VMRef, StackPtr);
static void fun_169(VMRef, StackPtr);
static void fun_170(VMRef, StackPtr);
static void fun_473(VMRef, StackPtr);
static void fun_171(VMRef, StackPtr);
static void fun_172(VMRef, StackPtr);
static void fun_173(VMRef, StackPtr);
static void fun_174(VMRef, StackPtr);
static void fun_175(VMRef, StackPtr);
static void fun_474(VMRef, StackPtr);
static void fun_176(VMRef, StackPtr);
static void fun_475(VMRef, StackPtr);
static void fun_177(VMRef, StackPtr);
static void fun_178(VMRef, StackPtr);
static void fun_179(VMRef, StackPtr);
static void fun_180(VMRef, StackPtr);
static void fun_181(VMRef, StackPtr);
static void fun_182(VMRef, StackPtr);
static void fun_183(VMRef, StackPtr);
static void fun_184(VMRef, StackPtr);
static void fun_185(VMRef, StackPtr);
static void fun_186(VMRef, StackPtr);
static void fun_187(VMRef, StackPtr);
static void fun_188(VMRef, StackPtr);
static void fun_190(VMRef, StackPtr);
static void fun_193(VMRef, StackPtr);
static void fun_194(VMRef, StackPtr);
static void fun_195(VMRef, StackPtr);
static void fun_196(VMRef, StackPtr);
static void fun_197(VMRef, StackPtr);
static void fun_198(VMRef, StackPtr);
static void fun_199(VMRef, StackPtr);
static void fun_200(VMRef, StackPtr);
static void fun_201(VMRef, StackPtr);
static void fun_202(VMRef, StackPtr);
static void fun_203(VMRef, StackPtr);
static void fun_511(VMRef, StackPtr);
static void fun_509(VMRef, StackPtr);
static void fun_205(VMRef, StackPtr);
static void fun_206(VMRef, StackPtr);
static void fun_208(VMRef, StackPtr);
static void fun_209(VMRef, StackPtr);
static void fun_514(VMRef, StackPtr);
static void fun_210(VMRef, StackPtr);
static void fun_211(VMRef, StackPtr);
static void fun_516(VMRef, StackPtr);
static void fun_212(VMRef, StackPtr);
static void fun_213(VMRef, StackPtr);
static void fun_214(VMRef, StackPtr);
static void fun_215(VMRef, StackPtr);
static void fun_218(VMRef, StackPtr);
static void fun_230(VMRef, StackPtr);
static void fun_219(VMRef, StackPtr);
static void fun_225(VMRef, StackPtr);
static void fun_231(VMRef, StackPtr);
static void fun_519(VMRef, StackPtr);
static void fun_220(VMRef, StackPtr);
static void fun_520(VMRef, StackPtr);
static void fun_226(VMRef, StackPtr);
static void fun_221(VMRef, StackPtr);
static void fun_521(VMRef, StackPtr);
static void fun_227(VMRef, StackPtr);
static void fun_222(VMRef, StackPtr);
static void fun_228(VMRef, StackPtr);
static void fun_223(VMRef, StackPtr);
static void fun_229(VMRef, StackPtr);
static void fun_224(VMRef, StackPtr);
static void fun_523(VMRef, StackPtr);
static void fun_232(VMRef, StackPtr);
static void fun_234(VMRef, StackPtr);
static void fun_235(VMRef, StackPtr);
static void fun_236(VMRef, StackPtr);
static void fun_237(VMRef, StackPtr);
static void fun_238(VMRef, StackPtr);
static void fun_526(VMRef, StackPtr);
static void fun_239(VMRef, StackPtr);
static void fun_527(VMRef, StackPtr);
static void fun_240(VMRef, StackPtr);
static void fun_241(VMRef, StackPtr);
static void fun_528(VMRef, StackPtr);
static void fun_242(VMRef, StackPtr);
static void fun_244(VMRef, StackPtr);
static void fun_245(VMRef, StackPtr);
static void fun_246(VMRef, StackPtr);
static void fun_250(VMRef, StackPtr);
static void fun_251(VMRef, StackPtr);
static void fun_252(VMRef, StackPtr);
static void fun_253(VMRef, StackPtr);
static void fun_254(VMRef, StackPtr);
static void fun_257(VMRef, StackPtr);
static void fun_258(VMRef, StackPtr);
static void fun_259(VMRef, StackPtr);
static void fun_260(VMRef, StackPtr);
static void fun_261(VMRef, StackPtr);
static void fun_560(VMRef, StackPtr);
static void fun_549(VMRef, StackPtr);
static void fun_263(VMRef, StackPtr);
static void fun_561(VMRef, StackPtr);
static void fun_556(VMRef, StackPtr);
static void fun_264(VMRef, StackPtr);
static void fun_545(VMRef, StackPtr);
static void fun_265(VMRef, StackPtr);
static void fun_550(VMRef, StackPtr);
static void fun_266(VMRef, StackPtr);
static void fun_552(VMRef, StackPtr);
static void fun_267(VMRef, StackPtr);
static void fun_555(VMRef, StackPtr);
static void fun_268(VMRef, StackPtr);
static void fun_557(VMRef, StackPtr);
static void fun_269(VMRef, StackPtr);
static void fun_270(VMRef, StackPtr);
static void fun_544(VMRef, StackPtr);
static void fun_271(VMRef, StackPtr);
static void fun_554(VMRef, StackPtr);
static void fun_272(VMRef, StackPtr);
static void fun_273(VMRef, StackPtr);
static void fun_274(VMRef, StackPtr);
static void fun_277(VMRef, StackPtr);
static void fun_278(VMRef, StackPtr);
static void fun_279(VMRef, StackPtr);
static void fun_280(VMRef, StackPtr);
static void fun_281(VMRef, StackPtr);
static void fun_282(VMRef, StackPtr);
static void fun_283(VMRef, StackPtr);
static void fun_284(VMRef, StackPtr);
static void fun_285(VMRef, StackPtr);
static void fun_286(VMRef, StackPtr);
static void fun_287(VMRef, StackPtr);
static void fun_288(VMRef, StackPtr);
static void fun_289(VMRef, StackPtr);
static void fun_290(VMRef, StackPtr);
static void fun_292(VMRef, StackPtr);
static void fun_294(VMRef, StackPtr);
static void fun_296(VMRef, StackPtr);
static void fun_298(VMRef, StackPtr);
static void fun_300(VMRef, StackPtr);
static void fun_291(VMRef, StackPtr);
static void fun_293(VMRef, StackPtr);
static void fun_295(VMRef, StackPtr);
static void fun_297(VMRef, StackPtr);
static void fun_299(VMRef, StackPtr);
static void fun_301(VMRef, StackPtr);
static void fun_302(VMRef, StackPtr);
static void fun_303(VMRef, StackPtr);
static void fun_304(VMRef, StackPtr);
static void fun_305(VMRef, StackPtr);
static void fun_306(VMRef, StackPtr);
static void fun_308(VMRef, StackPtr);
static void fun_309(VMRef, StackPtr);
static void fun_310(VMRef, StackPtr);
static void fun_311(VMRef, StackPtr);
static void fun_312(VMRef, StackPtr);
static void fun_313(VMRef, StackPtr);
static void fun_314(VMRef, StackPtr);
static void fun_315(VMRef, StackPtr);
static void fun_318(VMRef, StackPtr);
static void fun_316(VMRef, StackPtr);
static void fun_320(VMRef, StackPtr);
static void fun_317(VMRef, StackPtr);
static void fun_323(VMRef, StackPtr);
static void fun_319(VMRef, StackPtr);
static void fun_321(VMRef, StackPtr);
static void fun_322(VMRef, StackPtr);
static void fun_324(VMRef, StackPtr);
static void fun_325(VMRef, StackPtr);
static void fun_326(VMRef, StackPtr);
static void fun_327(VMRef, StackPtr);
static void fun_328(VMRef, StackPtr);
static void fun_329(VMRef, StackPtr);
static void fun_330(VMRef, StackPtr);
static void fun_569(VMRef, StackPtr);
static void fun_331(VMRef, StackPtr);
static void fun_332(VMRef, StackPtr);
static void fun_333(VMRef, StackPtr);
static void fun_334(VMRef, StackPtr);
static void fun_335(VMRef, StackPtr);
static void fun_336(VMRef, StackPtr);
static void fun_337(VMRef, StackPtr);
static void fun_339(VMRef, StackPtr);
static void fun_340(VMRef, StackPtr);
static void fun_341(VMRef, StackPtr);
static void fun_583(VMRef, StackPtr);
static void fun_342(VMRef, StackPtr);
static void fun_343(VMRef, StackPtr);
static void fun_344(VMRef, StackPtr);
static void fun_345(VMRef, StackPtr);
static void fun_346(VMRef, StackPtr);
static void fun_347(VMRef, StackPtr);
static void fun_348(VMRef, StackPtr);
static void fun_349(VMRef, StackPtr);
static void fun_350(VMRef, StackPtr);
static void fun_351(VMRef, StackPtr);
static void fun_355(VMRef, StackPtr);
static void fun_356(VMRef, StackPtr);
static void fun_358(VMRef, StackPtr);
static void fun_359(VMRef, StackPtr);
static void fun_361(VMRef, StackPtr);
static void fun_362(VMRef, StackPtr);
static void fun_363(VMRef, StackPtr);
static void fun_364(VMRef, StackPtr);
static void fun_365(VMRef, StackPtr);
static void fun_366(VMRef, StackPtr);
static void fun_370(VMRef, StackPtr);
static void fun_371(VMRef, StackPtr);
static void fun_372(VMRef, StackPtr);
static void fun_373(VMRef, StackPtr);
static void fun_375(VMRef, StackPtr);
static void fun_376(VMRef, StackPtr);
static void fun_377(VMRef, StackPtr);
static void fun_378(VMRef, StackPtr);
static void fun_379(VMRef, StackPtr);
static void fun_380(VMRef, StackPtr);
static void fun_381(VMRef, StackPtr);
static void fun_382(VMRef, StackPtr);
static void fun_383(VMRef, StackPtr);
static void fun_384(VMRef, StackPtr);
static void fun_385(VMRef, StackPtr);
static void fun_396(VMRef, StackPtr);
static void fun_428(VMRef, StackPtr);
static void fun_446(VMRef, StackPtr);
static void fun_447(VMRef, StackPtr);
static void fun_386(VMRef, StackPtr);
static void fun_609(VMRef, StackPtr);
static void fun_421(VMRef, StackPtr);
static void fun_394(VMRef, StackPtr);
static void fun_426(VMRef, StackPtr);
static void fun_395(VMRef, StackPtr);
static void fun_427(VMRef, StackPtr);
static void fun_397(VMRef, StackPtr);
static void fun_429(VMRef, StackPtr);
static void fun_404(VMRef, StackPtr);
static void fun_608(VMRef, StackPtr);
static void fun_414(VMRef, StackPtr);
static void fun_436(VMRef, StackPtr);
static void fun_415(VMRef, StackPtr);
static void fun_422(VMRef, StackPtr);
static void fun_423(VMRef, StackPtr);
static void fun_424(VMRef, StackPtr);
static void fun_425(VMRef, StackPtr);
static void fun_445(VMRef, StackPtr);
static void fun_448(VMRef, StackPtr);
static void fun_449(VMRef, StackPtr);
static void fun_450(VMRef, StackPtr);
static void fun_451(VMRef, StackPtr);
static void fun_452(VMRef, StackPtr);
static void fun_453(VMRef, StackPtr);
static void fun_454(VMRef, StackPtr);
static void fun_455(VMRef, StackPtr);
static void fun_456(VMRef, StackPtr);
static void fun_457(VMRef, StackPtr);
static void fun_458(VMRef, StackPtr);
static void fun_459(VMRef, StackPtr);
static void fun_460(VMRef, StackPtr);

// __top_level_expression
static void fun_0(VMRef vm, StackPtr psp) {
    Value regs[24];
    Value locals[204];
    BackupVar(vm, 0);
    BackupVar(vm, 74);
    BackupVar(vm, 75);
    BackupVar(vm, 76);
    BackupVar(vm, 77);
    *(locals + 0) = Value(0, lobster::V_NIL);
    *(locals + 1) = Value(0, lobster::V_NIL);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    *(locals + 5) = Value(0, lobster::V_NIL);
    BackupVar(vm, 84);
    BackupVar(vm, 85);
    BackupVar(vm, 86);
    BackupVar(vm, 87);
    BackupVar(vm, 88);
    BackupVar(vm, 89);
    BackupVar(vm, 90);
    BackupVar(vm, 91);
    BackupVar(vm, 92);
    BackupVar(vm, 93);
    BackupVar(vm, 94);
    BackupVar(vm, 95);
    *(locals + 6) = Value(0, lobster::V_NIL);
    *(locals + 7) = Value(0, lobster::V_NIL);
    *(locals + 8) = Value(0, lobster::V_NIL);
    *(locals + 9) = Value(0, lobster::V_NIL);
    *(locals + 10) = Value(0, lobster::V_NIL);
    *(locals + 11) = Value(0, lobster::V_NIL);
    *(locals + 12) = Value(0, lobster::V_NIL);
    *(locals + 13) = Value(0, lobster::V_NIL);
    *(locals + 14) = Value(0, lobster::V_NIL);
    *(locals + 15) = Value(0, lobster::V_NIL);
    *(locals + 16) = Value(0, lobster::V_NIL);
    *(locals + 17) = Value(0, lobster::V_NIL);
    *(locals + 18) = Value(0, lobster::V_NIL);
    *(locals + 19) = Value(0, lobster::V_NIL);
    *(locals + 20) = Value(0, lobster::V_NIL);
    *(locals + 21) = Value(0, lobster::V_NIL);
    *(locals + 22) = Value(0, lobster::V_NIL);
    *(locals + 23) = Value(0, lobster::V_NIL);
    *(locals + 24) = Value(0, lobster::V_NIL);
    *(locals + 25) = Value(0, lobster::V_NIL);
    *(locals + 26) = Value(0, lobster::V_NIL);
    *(locals + 27) = Value(0, lobster::V_NIL);
    *(locals + 28) = Value(0, lobster::V_NIL);
    *(locals + 29) = Value(0, lobster::V_NIL);
    *(locals + 30) = Value(0, lobster::V_NIL);
    *(locals + 31) = Value(0, lobster::V_NIL);
    *(locals + 32) = Value(0, lobster::V_NIL);
    BackupVar(vm, 123);
    BackupVar(vm, 124);
    BackupVar(vm, 125);
    BackupVar(vm, 126);
    BackupVar(vm, 127);
    BackupVar(vm, 128);
    BackupVar(vm, 129);
    BackupVar(vm, 130);
    BackupVar(vm, 131);
    BackupVar(vm, 132);
    BackupVar(vm, 133);
    BackupVar(vm, 134);
    BackupVar(vm, 135);
    BackupVar(vm, 136);
    *(locals + 33) = Value(0, lobster::V_NIL);
    *(locals + 34) = Value(0, lobster::V_NIL);
    *(locals + 35) = Value(0, lobster::V_NIL);
    *(locals + 36) = Value(0, lobster::V_NIL);
    *(locals + 37) = Value(0, lobster::V_NIL);
    *(locals + 38) = Value(0, lobster::V_NIL);
    *(locals + 39) = Value(0, lobster::V_NIL);
    *(locals + 40) = Value(0, lobster::V_NIL);
    *(locals + 41) = Value(0, lobster::V_NIL);
    *(locals + 42) = Value(0, lobster::V_NIL);
    *(locals + 43) = Value(0, lobster::V_NIL);
    *(locals + 44) = Value(0, lobster::V_NIL);
    *(locals + 45) = Value(0, lobster::V_NIL);
    *(locals + 46) = Value(0, lobster::V_NIL);
    *(locals + 47) = Value(0, lobster::V_NIL);
    *(locals + 48) = Value(0, lobster::V_NIL);
    *(locals + 49) = Value(0, lobster::V_NIL);
    *(locals + 50) = Value(0, lobster::V_NIL);
    *(locals + 51) = Value(0, lobster::V_NIL);
    *(locals + 52) = Value(0, lobster::V_NIL);
    *(locals + 53) = Value(0, lobster::V_NIL);
    *(locals + 54) = Value(0, lobster::V_NIL);
    *(locals + 55) = Value(0, lobster::V_NIL);
    *(locals + 56) = Value(0, lobster::V_NIL);
    *(locals + 57) = Value(0, lobster::V_NIL);
    *(locals + 58) = Value(0, lobster::V_NIL);
    *(locals + 59) = Value(0, lobster::V_NIL);
    *(locals + 60) = Value(0, lobster::V_NIL);
    *(locals + 61) = Value(0, lobster::V_NIL);
    *(locals + 62) = Value(0, lobster::V_NIL);
    *(locals + 63) = Value(0, lobster::V_NIL);
    *(locals + 64) = Value(0, lobster::V_NIL);
    *(locals + 65) = Value(0, lobster::V_NIL);
    BackupVar(vm, 170);
    BackupVar(vm, 171);
    *(locals + 66) = Value(0, lobster::V_NIL);
    *(locals + 67) = Value(0, lobster::V_NIL);
    *(locals + 68) = Value(0, lobster::V_NIL);
    *(locals + 69) = Value(0, lobster::V_NIL);
    *(locals + 70) = Value(0, lobster::V_NIL);
    *(locals + 71) = Value(0, lobster::V_NIL);
    *(locals + 72) = Value(0, lobster::V_NIL);
    *(locals + 73) = Value(0, lobster::V_NIL);
    *(locals + 74) = Value(0, lobster::V_NIL);
    *(locals + 75) = Value(0, lobster::V_NIL);
    *(locals + 76) = Value(0, lobster::V_NIL);
    *(locals + 77) = Value(0, lobster::V_NIL);
    *(locals + 78) = Value(0, lobster::V_NIL);
    *(locals + 79) = Value(0, lobster::V_NIL);
    *(locals + 80) = Value(0, lobster::V_NIL);
    *(locals + 81) = Value(0, lobster::V_NIL);
    *(locals + 82) = Value(0, lobster::V_NIL);
    *(locals + 83) = Value(0, lobster::V_NIL);
    *(locals + 84) = Value(0, lobster::V_NIL);
    *(locals + 85) = Value(0, lobster::V_NIL);
    *(locals + 86) = Value(0, lobster::V_NIL);
    *(locals + 87) = Value(0, lobster::V_NIL);
    *(locals + 88) = Value(0, lobster::V_NIL);
    *(locals + 89) = Value(0, lobster::V_NIL);
    *(locals + 90) = Value(0, lobster::V_NIL);
    *(locals + 91) = Value(0, lobster::V_NIL);
    *(locals + 92) = Value(0, lobster::V_NIL);
    *(locals + 93) = Value(0, lobster::V_NIL);
    *(locals + 94) = Value(0, lobster::V_NIL);
    *(locals + 95) = Value(0, lobster::V_NIL);
    *(locals + 96) = Value(0, lobster::V_NIL);
    *(locals + 97) = Value(0, lobster::V_NIL);
    *(locals + 98) = Value(0, lobster::V_NIL);
    *(locals + 99) = Value(0, lobster::V_NIL);
    *(locals + 100) = Value(0, lobster::V_NIL);
    *(locals + 101) = Value(0, lobster::V_NIL);
    *(locals + 102) = Value(0, lobster::V_NIL);
    *(locals + 103) = Value(0, lobster::V_NIL);
    *(locals + 104) = Value(0, lobster::V_NIL);
    *(locals + 105) = Value(0, lobster::V_NIL);
    *(locals + 106) = Value(0, lobster::V_NIL);
    *(locals + 107) = Value(0, lobster::V_NIL);
    *(locals + 108) = Value(0, lobster::V_NIL);
    *(locals + 109) = Value(0, lobster::V_NIL);
    *(locals + 110) = Value(0, lobster::V_NIL);
    *(locals + 111) = Value(0, lobster::V_NIL);
    *(locals + 112) = Value(0, lobster::V_NIL);
    *(locals + 113) = Value(0, lobster::V_NIL);
    *(locals + 114) = Value(0, lobster::V_NIL);
    *(locals + 115) = Value(0, lobster::V_NIL);
    *(locals + 116) = Value(0, lobster::V_NIL);
    *(locals + 117) = Value(0, lobster::V_NIL);
    *(locals + 118) = Value(0, lobster::V_NIL);
    *(locals + 119) = Value(0, lobster::V_NIL);
    *(locals + 120) = Value(0, lobster::V_NIL);
    *(locals + 121) = Value(0, lobster::V_NIL);
    *(locals + 122) = Value(0, lobster::V_NIL);
    *(locals + 123) = Value(0, lobster::V_NIL);
    *(locals + 124) = Value(0, lobster::V_NIL);
    *(locals + 125) = Value(0, lobster::V_NIL);
    *(locals + 126) = Value(0, lobster::V_NIL);
    *(locals + 127) = Value(0, lobster::V_NIL);
    *(locals + 128) = Value(0, lobster::V_NIL);
    *(locals + 129) = Value(0, lobster::V_NIL);
    *(locals + 130) = Value(0, lobster::V_NIL);
    *(locals + 131) = Value(0, lobster::V_NIL);
    *(locals + 132) = Value(0, lobster::V_NIL);
    *(locals + 133) = Value(0, lobster::V_NIL);
    *(locals + 134) = Value(0, lobster::V_NIL);
    *(locals + 135) = Value(0, lobster::V_NIL);
    *(locals + 136) = Value(0, lobster::V_NIL);
    *(locals + 137) = Value(0, lobster::V_NIL);
    *(locals + 138) = Value(0, lobster::V_NIL);
    *(locals + 139) = Value(0, lobster::V_NIL);
    *(locals + 140) = Value(0, lobster::V_NIL);
    *(locals + 141) = Value(0, lobster::V_NIL);
    *(locals + 142) = Value(0, lobster::V_NIL);
    *(locals + 143) = Value(0, lobster::V_NIL);
    *(locals + 144) = Value(0, lobster::V_NIL);
    *(locals + 145) = Value(0, lobster::V_NIL);
    *(locals + 146) = Value(0, lobster::V_NIL);
    *(locals + 147) = Value(0, lobster::V_NIL);
    *(locals + 148) = Value(0, lobster::V_NIL);
    *(locals + 149) = Value(0, lobster::V_NIL);
    *(locals + 150) = Value(0, lobster::V_NIL);
    *(locals + 151) = Value(0, lobster::V_NIL);
    *(locals + 152) = Value(0, lobster::V_NIL);
    *(locals + 153) = Value(0, lobster::V_NIL);
    *(locals + 154) = Value(0, lobster::V_NIL);
    *(locals + 155) = Value(0, lobster::V_NIL);
    *(locals + 156) = Value(0, lobster::V_NIL);
    *(locals + 157) = Value(0, lobster::V_NIL);
    *(locals + 158) = Value(0, lobster::V_NIL);
    *(locals + 159) = Value(0, lobster::V_NIL);
    *(locals + 160) = Value(0, lobster::V_NIL);
    *(locals + 161) = Value(0, lobster::V_NIL);
    *(locals + 162) = Value(0, lobster::V_NIL);
    *(locals + 163) = Value(0, lobster::V_NIL);
    *(locals + 164) = Value(0, lobster::V_NIL);
    *(locals + 165) = Value(0, lobster::V_NIL);
    *(locals + 166) = Value(0, lobster::V_NIL);
    *(locals + 167) = Value(0, lobster::V_NIL);
    *(locals + 168) = Value(0, lobster::V_NIL);
    *(locals + 169) = Value(0, lobster::V_NIL);
    *(locals + 170) = Value(0, lobster::V_NIL);
    *(locals + 171) = Value(0, lobster::V_NIL);
    *(locals + 172) = Value(0, lobster::V_NIL);
    *(locals + 173) = Value(0, lobster::V_NIL);
    *(locals + 174) = Value(0, lobster::V_NIL);
    *(locals + 175) = Value(0, lobster::V_NIL);
    *(locals + 176) = Value(0, lobster::V_NIL);
    *(locals + 177) = Value(0, lobster::V_NIL);
    *(locals + 178) = Value(0, lobster::V_NIL);
    *(locals + 179) = Value(0, lobster::V_NIL);
    *(locals + 180) = Value(0, lobster::V_NIL);
    *(locals + 181) = Value(0, lobster::V_NIL);
    *(locals + 182) = Value(0, lobster::V_NIL);
    *(locals + 183) = Value(0, lobster::V_NIL);
    *(locals + 184) = Value(0, lobster::V_NIL);
    *(locals + 185) = Value(0, lobster::V_NIL);
    *(locals + 186) = Value(0, lobster::V_NIL);
    *(locals + 187) = Value(0, lobster::V_NIL);
    *(locals + 188) = Value(0, lobster::V_NIL);
    *(locals + 189) = Value(0, lobster::V_NIL);
    *(locals + 190) = Value(0, lobster::V_NIL);
    *(locals + 191) = Value(0, lobster::V_NIL);
    *(locals + 192) = Value(0, lobster::V_NIL);
    *(locals + 193) = Value(0, lobster::V_NIL);
    *(locals + 194) = Value(0, lobster::V_NIL);
    *(locals + 195) = Value(0, lobster::V_NIL);
    *(locals + 196) = Value(0, lobster::V_NIL);
    *(locals + 197) = Value(0, lobster::V_NIL);
    *(locals + 198) = Value(0, lobster::V_NIL);
    *(locals + 199) = Value(0, lobster::V_NIL);
    *(locals + 200) = Value(0, lobster::V_NIL);
    *(locals + 201) = Value(0, lobster::V_NIL);
    *(locals + 202) = Value(0, lobster::V_NIL);
    *(locals + 203) = Value(0, lobster::V_NIL);
    BackupVar(vm, 1150);
    BackupVar(vm, 1151);
    BackupVar(vm, 1152);
    BackupVar(vm, 1153);
    PushFunId(vm, funinfo_table + 0, locals);
    vm.last = {8, 1};
    vm.last = {12, 1};
    vm.last = {15, 1};
    vm.last = {20, 1};
    vm.last = {21, 1};
    vm.last = {23, 1};
    vm.last = {24, 1};
    vm.last = {26, 1};
    vm.last = {27, 1};
    vm.last = {31, 1};
    vm.last = {4, 2};
    vm.last = {5, 2};
    vm.last = {8, 2};
    U_PUSHFUN(vm, regs + 0, 0, fun_3);
    U_LVAL_VARF(vm, regs + 1, 0); // current_test_runner
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {10, 2};
    vm.last = {14, 2};
    U_PUSHSTR(vm, regs + 0, 0); // "example"
    U_PUSHFUN(vm, regs + 1, 0, fun_5);
    fun_4(vm, regs + 2); // call: run_test
    vm.last = {7, 0};
    U_PUSHFUN(vm, regs + 0, 0, fun_6);
    U_LVAL_VARF(vm, regs + 1, 0); // current_test_runner
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {3, 5};
    vm.last = {9, 5};
    vm.last = {11, 5};
    vm.last = {21, 5};
    vm.last = {29, 5};
    vm.last = {36, 5};
    vm.last = {46, 5};
    vm.last = {56, 5};
    vm.last = {62, 5};
    vm.last = {66, 5};
    vm.last = {71, 5};
    vm.last = {76, 5};
    vm.last = {82, 5};
    vm.last = {85, 5};
    vm.last = {92, 5};
    vm.last = {99, 5};
    vm.last = {106, 5};
    vm.last = {114, 5};
    vm.last = {127, 5};
    vm.last = {139, 5};
    vm.last = {142, 5};
    vm.last = {146, 5};
    vm.last = {148, 5};
    vm.last = {153, 5};
    vm.last = {158, 5};
    vm.last = {164, 5};
    vm.last = {171, 5};
    vm.last = {176, 5};
    vm.last = {180, 5};
    vm.last = {186, 5};
    vm.last = {188, 5};
    vm.last = {190, 5};
    vm.last = {193, 5};
    vm.last = {202, 5};
    vm.last = {222, 5};
    vm.last = {230, 5};
    vm.last = {237, 5};
    vm.last = {245, 5};
    vm.last = {249, 5};
    vm.last = {253, 5};
    vm.last = {255, 5};
    vm.last = {258, 5};
    vm.last = {262, 5};
    vm.last = {264, 5};
    vm.last = {269, 5};
    vm.last = {276, 5};
    vm.last = {281, 5};
    vm.last = {284, 5};
    vm.last = {290, 5};
    vm.last = {7, 4};
    *(regs + 0) = Value(0.000000);
    *(regs + 1) = Value(0.000000);
    U_LVAL_VARF(vm, regs + 2, 74); // float2_0+0
    U_LV_WRITEV(vm, regs + 2, 2);
    vm.last = {8, 4};
    *(regs + 0) = Value(1.000000);
    *(regs + 1) = Value(1.000000);
    U_LVAL_VARF(vm, regs + 2, 76); // float2_1+0
    U_LV_WRITEV(vm, regs + 2, 2);
    vm.last = {9, 4};
    *(regs + 0) = Value(0.500000);
    *(regs + 1) = Value(0.500000);
    vm.temp_lval = locals + 0; // float2_h+0
    U_LV_WRITEV(vm, regs + 2, 2);
    vm.last = {10, 4};
    *(regs + 0) = Value(1.000000);
    *(regs + 1) = Value(0.000000);
    vm.temp_lval = locals + 2; // float2_x+0
    U_LV_WRITEV(vm, regs + 2, 2);
    vm.last = {11, 4};
    *(regs + 0) = Value(0.000000);
    *(regs + 1) = Value(1.000000);
    vm.temp_lval = locals + 4; // float2_y+0
    U_LV_WRITEV(vm, regs + 2, 2);
    vm.last = {13, 4};
    *(regs + 0) = Value(0.000000);
    *(regs + 1) = Value(0.000000);
    *(regs + 2) = Value(0.000000);
    U_LVAL_VARF(vm, regs + 3, 84); // float3_0+0
    U_LV_WRITEV(vm, regs + 3, 3);
    vm.last = {14, 4};
    *(regs + 0) = Value(1.000000);
    *(regs + 1) = Value(1.000000);
    *(regs + 2) = Value(1.000000);
    U_LVAL_VARF(vm, regs + 3, 87); // float3_1+0
    U_LV_WRITEV(vm, regs + 3, 3);
    vm.last = {15, 4};
    *(regs + 0) = Value(1.000000);
    *(regs + 1) = Value(0.000000);
    *(regs + 2) = Value(0.000000);
    U_LVAL_VARF(vm, regs + 3, 90); // float3_x+0
    U_LV_WRITEV(vm, regs + 3, 3);
    vm.last = {16, 4};
    *(regs + 0) = Value(0.000000);
    *(regs + 1) = Value(1.000000);
    *(regs + 2) = Value(0.000000);
    U_LVAL_VARF(vm, regs + 3, 93); // float3_y+0
    U_LV_WRITEV(vm, regs + 3, 3);
    vm.last = {17, 4};
    *(regs + 0) = Value(0.000000);
    *(regs + 1) = Value(0.000000);
    *(regs + 2) = Value(1.000000);
    vm.temp_lval = locals + 6; // float3_z+0
    U_LV_WRITEV(vm, regs + 3, 3);
    vm.last = {19, 4};
    *(regs + 0) = Value(0.000000);
    *(regs + 1) = Value(0.000000);
    *(regs + 2) = Value(0.000000);
    *(regs + 3) = Value(0.000000);
    vm.temp_lval = locals + 9; // float4_0+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {20, 4};
    *(regs + 0) = Value(1.000000);
    *(regs + 1) = Value(1.000000);
    *(regs + 2) = Value(1.000000);
    *(regs + 3) = Value(1.000000);
    vm.temp_lval = locals + 13; // float4_1+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {21, 4};
    *(regs + 0) = Value(1.000000);
    *(regs + 1) = Value(0.000000);
    *(regs + 2) = Value(0.000000);
    *(regs + 3) = Value(0.000000);
    vm.temp_lval = locals + 17; // float4_x+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {22, 4};
    *(regs + 0) = Value(0.000000);
    *(regs + 1) = Value(1.000000);
    *(regs + 2) = Value(0.000000);
    *(regs + 3) = Value(0.000000);
    vm.temp_lval = locals + 21; // float4_y+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {23, 4};
    *(regs + 0) = Value(0.000000);
    *(regs + 1) = Value(0.000000);
    *(regs + 2) = Value(1.000000);
    *(regs + 3) = Value(0.000000);
    vm.temp_lval = locals + 25; // float4_z+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {24, 4};
    *(regs + 0) = Value(0.000000);
    *(regs + 1) = Value(0.000000);
    *(regs + 2) = Value(0.000000);
    *(regs + 3) = Value(1.000000);
    vm.temp_lval = locals + 29; // float4_w+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {28, 4};
    *(regs + 0) = Value(0);
    *(regs + 1) = Value(0);
    U_LVAL_VARF(vm, regs + 2, 123); // int2_0+0
    U_LV_WRITEV(vm, regs + 2, 2);
    vm.last = {29, 4};
    *(regs + 0) = Value(1);
    *(regs + 1) = Value(1);
    U_LVAL_VARF(vm, regs + 2, 125); // int2_1+0
    U_LV_WRITEV(vm, regs + 2, 2);
    vm.last = {30, 4};
    *(regs + 0) = Value(1);
    *(regs + 1) = Value(0);
    U_LVAL_VARF(vm, regs + 2, 127); // int2_x+0
    U_LV_WRITEV(vm, regs + 2, 2);
    vm.last = {31, 4};
    *(regs + 0) = Value(0);
    *(regs + 1) = Value(1);
    U_LVAL_VARF(vm, regs + 2, 129); // int2_y+0
    U_LV_WRITEV(vm, regs + 2, 2);
    vm.last = {33, 4};
    *(regs + 0) = Value(0);
    *(regs + 1) = Value(0);
    *(regs + 2) = Value(0);
    U_LVAL_VARF(vm, regs + 3, 131); // int3_0+0
    U_LV_WRITEV(vm, regs + 3, 3);
    vm.last = {34, 4};
    *(regs + 0) = Value(1);
    *(regs + 1) = Value(1);
    *(regs + 2) = Value(1);
    U_LVAL_VARF(vm, regs + 3, 134); // int3_1+0
    U_LV_WRITEV(vm, regs + 3, 3);
    vm.last = {35, 4};
    *(regs + 0) = Value(1);
    *(regs + 1) = Value(0);
    *(regs + 2) = Value(0);
    vm.temp_lval = locals + 33; // int3_x+0
    U_LV_WRITEV(vm, regs + 3, 3);
    vm.last = {36, 4};
    *(regs + 0) = Value(0);
    *(regs + 1) = Value(1);
    *(regs + 2) = Value(0);
    vm.temp_lval = locals + 36; // int3_y+0
    U_LV_WRITEV(vm, regs + 3, 3);
    vm.last = {37, 4};
    *(regs + 0) = Value(0);
    *(regs + 1) = Value(0);
    *(regs + 2) = Value(1);
    vm.temp_lval = locals + 39; // int3_z+0
    U_LV_WRITEV(vm, regs + 3, 3);
    vm.last = {39, 4};
    *(regs + 0) = Value(0);
    *(regs + 1) = Value(0);
    *(regs + 2) = Value(0);
    *(regs + 3) = Value(0);
    vm.temp_lval = locals + 42; // int4_0+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {40, 4};
    *(regs + 0) = Value(1);
    *(regs + 1) = Value(1);
    *(regs + 2) = Value(1);
    *(regs + 3) = Value(1);
    vm.temp_lval = locals + 46; // int4_1+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {41, 4};
    *(regs + 0) = Value(1);
    *(regs + 1) = Value(0);
    *(regs + 2) = Value(0);
    *(regs + 3) = Value(0);
    vm.temp_lval = locals + 50; // int4_x+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {42, 4};
    *(regs + 0) = Value(0);
    *(regs + 1) = Value(1);
    *(regs + 2) = Value(0);
    *(regs + 3) = Value(0);
    vm.temp_lval = locals + 54; // int4_y+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {43, 4};
    *(regs + 0) = Value(0);
    *(regs + 1) = Value(0);
    *(regs + 2) = Value(1);
    *(regs + 3) = Value(0);
    vm.temp_lval = locals + 58; // int4_z+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {44, 4};
    *(regs + 0) = Value(0);
    *(regs + 1) = Value(0);
    *(regs + 2) = Value(0);
    *(regs + 3) = Value(1);
    vm.temp_lval = locals + 62; // int4_w+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {46, 4};
    *(regs + 0) = Value(0);
    *(regs + 1) = Value(-1);
    *(regs + 2) = Value(1);
    *(regs + 3) = Value(0);
    *(regs + 4) = Value(0);
    *(regs + 5) = Value(1);
    *(regs + 6) = Value(-1);
    *(regs + 7) = Value(0);
    U_NEWVEC(vm, regs + 8, 37, 4);
    U_LVAL_VARF(vm, regs + 1, 170); // cardinal_directions
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {47, 4};
    *(regs + 0) = Value(-1);
    *(regs + 1) = Value(-1);
    *(regs + 2) = Value(1);
    *(regs + 3) = Value(1);
    *(regs + 4) = Value(1);
    *(regs + 5) = Value(-1);
    *(regs + 6) = Value(-1);
    *(regs + 7) = Value(1);
    U_NEWVEC(vm, regs + 8, 37, 4);
    U_LVAL_VARF(vm, regs + 1, 171); // diagonal_directions
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {48, 4};
    *(regs + 0) = Value(0);
    *(regs + 1) = Value(0);
    *(regs + 2) = Value(1);
    *(regs + 3) = Value(0);
    *(regs + 4) = Value(1);
    *(regs + 5) = Value(1);
    *(regs + 6) = Value(0);
    *(regs + 7) = Value(1);
    U_NEWVEC(vm, regs + 8, 37, 4);
    vm.temp_lval = locals + 66; // positive_directions
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {50, 4};
    *(regs + 0) = Value(1);
    *(regs + 1) = Value(1);
    *(regs + 2) = Value(1);
    *(regs + 3) = Value(1);
    *(regs + 4) = Value(1);
    *(regs + 5) = Value(-1);
    *(regs + 6) = Value(1);
    *(regs + 7) = Value(-1);
    *(regs + 8) = Value(1);
    *(regs + 9) = Value(1);
    *(regs + 10) = Value(-1);
    *(regs + 11) = Value(-1);
    *(regs + 12) = Value(-1);
    *(regs + 13) = Value(1);
    *(regs + 14) = Value(1);
    *(regs + 15) = Value(-1);
    *(regs + 16) = Value(1);
    *(regs + 17) = Value(-1);
    *(regs + 18) = Value(-1);
    *(regs + 19) = Value(-1);
    *(regs + 20) = Value(1);
    *(regs + 21) = Value(-1);
    *(regs + 22) = Value(-1);
    *(regs + 23) = Value(-1);
    U_NEWVEC(vm, regs + 24, 240, 8);
    vm.temp_lval = locals + 67; // octant_directions
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {62, 4};
    vm.last = {63, 4};
    vm.last = {64, 4};
    vm.last = {65, 4};
    vm.last = {66, 4};
    vm.last = {68, 4};
    vm.last = {71, 4};
    vm.last = {72, 4};
    vm.last = {73, 4};
    vm.last = {74, 4};
    vm.last = {75, 4};
    vm.last = {76, 4};
    vm.last = {79, 4};
    vm.last = {80, 4};
    vm.last = {81, 4};
    vm.last = {83, 4};
    vm.last = {84, 4};
    vm.last = {85, 4};
    vm.last = {88, 4};
    vm.last = {89, 4};
    vm.last = {90, 4};
    vm.last = {92, 4};
    vm.last = {93, 4};
    vm.last = {94, 4};
    vm.last = {96, 4};
    vm.last = {97, 4};
    vm.last = {99, 4};
    vm.last = {100, 4};
    vm.last = {101, 4};
    vm.last = {102, 4};
    vm.last = {104, 4};
    vm.last = {105, 4};
    vm.last = {106, 4};
    vm.last = {108, 4};
    vm.last = {109, 4};
    vm.last = {111, 4};
    vm.last = {115, 4};
    vm.last = {4, 3};
    U_PUSHSTR(vm, regs + 0, 1); // "struct"
    U_PUSHFUN(vm, regs + 1, 0, fun_110);
    fun_4(vm, regs + 2); // call: run_test
    vm.last = {3, 7};
    vm.last = {8, 7};
    vm.last = {16, 7};
    vm.last = {20, 7};
    vm.last = {23, 7};
    vm.last = {27, 7};
    vm.last = {32, 7};
    vm.last = {37, 7};
    vm.last = {16, 8};
    vm.last = {24, 8};
    vm.last = {33, 8};
    vm.last = {40, 8};
    vm.last = {50, 8};
    vm.last = {56, 8};
    vm.last = {65, 8};
    vm.last = {72, 8};
    vm.last = {78, 8};
    vm.last = {88, 8};
    vm.last = {97, 8};
    vm.last = {105, 8};
    vm.last = {113, 8};
    vm.last = {21, 8};
    vm.last = {119, 8};
    vm.last = {11, 6};
    vm.last = {12, 6};
    vm.last = {15, 6};
    vm.last = {13, 6};
    vm.last = {13, 6};
    vm.last = {19, 6};
    U_PUSHSTR(vm, regs + 0, 2); // "misc"
    U_PUSHFUN(vm, regs + 1, 0, fun_156);
    fun_4(vm, regs + 2); // call: run_test
    vm.last = {8, 9};
    U_PUSHSTR(vm, regs + 0, 3); // "types"
    U_PUSHFUN(vm, regs + 1, 0, fun_202);
    fun_4(vm, regs + 2); // call: run_test
    vm.last = {15, 11};
    vm.last = {5, 11};
    vm.last = {22, 11};
    vm.last = {54, 11};
    vm.last = {67, 11};
    vm.last = {89, 11};
    vm.last = {91, 11};
    vm.last = {88, 11};
    vm.last = {88, 11};
    vm.last = {94, 11};
    vm.last = {96, 11};
    vm.last = {4, 10};
    U_PUSHSTR(vm, regs + 0, 4); // "astar"
    U_PUSHFUN(vm, regs + 1, 0, fun_282);
    fun_4(vm, regs + 2); // call: run_test
    vm.last = {4, 12};
    U_PUSHSTR(vm, regs + 0, 5); // "goap"
    U_PUSHFUN(vm, regs + 1, 0, fun_289);
    fun_4(vm, regs + 2); // call: run_test
    vm.last = {4, 13};
    U_PUSHSTR(vm, regs + 0, 6); // "knightstour"
    U_PUSHFUN(vm, regs + 1, 0, fun_305);
    fun_4(vm, regs + 2); // call: run_test
    vm.last = {6, 14};
    U_PUSHSTR(vm, regs + 0, 7); // "parsereval"
    U_PUSHFUN(vm, regs + 1, 0, fun_314);
    fun_4(vm, regs + 2); // call: run_test
    vm.last = {3, 16};
    vm.last = {3, 16};
    vm.last = {13, 16};
    vm.last = {5, 15};
    U_PUSHSTR(vm, regs + 0, 8); // "flood"
    U_PUSHFUN(vm, regs + 1, 0, fun_337);
    fun_4(vm, regs + 2); // call: run_test
    vm.last = {7, 17};
    U_PUSHSTR(vm, regs + 0, 9); // "water"
    U_PUSHFUN(vm, regs + 1, 0, fun_339);
    fun_4(vm, regs + 2); // call: run_test
    vm.last = {5, 18};
    U_PUSHSTR(vm, regs + 0, 10); // "gradient"
    U_PUSHFUN(vm, regs + 1, 0, fun_350);
    fun_4(vm, regs + 2); // call: run_test
    vm.last = {6, 19};
    U_PUSHSTR(vm, regs + 0, 11); // "springs"
    U_PUSHFUN(vm, regs + 1, 0, fun_355);
    fun_4(vm, regs + 2); // call: run_test
    vm.last = {3, 21};
    vm.last = {3, 21};
    vm.last = {9, 21};
    *(regs + 0) = Value(0.000000);
    *(regs + 1) = Value(0.000000);
    *(regs + 2) = Value(0.000000);
    *(regs + 3) = Value(0.000000);
    vm.temp_lval = locals + 68; // color_clear+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {10, 21};
    *(regs + 0) = Value(0.000000);
    *(regs + 1) = Value(0.000000);
    *(regs + 2) = Value(0.000000);
    *(regs + 3) = Value(1.000000);
    vm.temp_lval = locals + 72; // color_black+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {11, 21};
    *(regs + 0) = Value(1.000000);
    *(regs + 1) = Value(1.000000);
    *(regs + 2) = Value(1.000000);
    *(regs + 3) = Value(1.000000);
    vm.temp_lval = locals + 76; // color_white+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {13, 21};
    *(regs + 0) = Value(1.000000);
    *(regs + 1) = Value(0.000000);
    *(regs + 2) = Value(0.000000);
    *(regs + 3) = Value(1.000000);
    vm.temp_lval = locals + 80; // color_red+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {14, 21};
    *(regs + 0) = Value(0.000000);
    *(regs + 1) = Value(1.000000);
    *(regs + 2) = Value(0.000000);
    *(regs + 3) = Value(1.000000);
    vm.temp_lval = locals + 84; // color_green+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {15, 21};
    *(regs + 0) = Value(0.000000);
    *(regs + 1) = Value(0.000000);
    *(regs + 2) = Value(1.000000);
    *(regs + 3) = Value(1.000000);
    vm.temp_lval = locals + 88; // color_blue+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {17, 21};
    *(regs + 0) = Value(1.000000);
    *(regs + 1) = Value(1.000000);
    *(regs + 2) = Value(0.000000);
    *(regs + 3) = Value(1.000000);
    vm.temp_lval = locals + 92; // color_yellow+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {18, 21};
    *(regs + 0) = Value(0.000000);
    *(regs + 1) = Value(1.000000);
    *(regs + 2) = Value(1.000000);
    *(regs + 3) = Value(1.000000);
    vm.temp_lval = locals + 96; // color_cyan+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {19, 21};
    *(regs + 0) = Value(1.000000);
    *(regs + 1) = Value(0.000000);
    *(regs + 2) = Value(1.000000);
    *(regs + 3) = Value(1.000000);
    vm.temp_lval = locals + 100; // color_pink+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {21, 21};
    *(regs + 0) = Value(0.500000);
    *(regs + 1) = Value(0.500000);
    *(regs + 2) = Value(0.500000);
    *(regs + 3) = Value(1.000000);
    vm.temp_lval = locals + 104; // color_grey+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {22, 21};
    *(regs + 0) = Value(0.250000);
    *(regs + 1) = Value(0.250000);
    *(regs + 2) = Value(0.250000);
    *(regs + 3) = Value(1.000000);
    vm.temp_lval = locals + 108; // color_dark_grey+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {23, 21};
    *(regs + 0) = Value(0.750000);
    *(regs + 1) = Value(0.750000);
    *(regs + 2) = Value(0.750000);
    *(regs + 3) = Value(1.000000);
    vm.temp_lval = locals + 112; // color_light_grey+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {24, 21};
    U_PUSHFLT64(vm, regs + 0, -858993459, 1072483532); // 0.9
    U_PUSHFLT64(vm, regs + 1, -858993459, 1072483532); // 0.9
    U_PUSHFLT64(vm, regs + 2, -858993459, 1072483532); // 0.9
    *(regs + 3) = Value(1.000000);
    vm.temp_lval = locals + 116; // color_lightest_grey+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {25, 21};
    U_PUSHFLT64(vm, regs + 0, -1717986918, 1069128089); // 0.1
    U_PUSHFLT64(vm, regs + 1, -1717986918, 1069128089); // 0.1
    U_PUSHFLT64(vm, regs + 2, -1717986918, 1069128089); // 0.1
    *(regs + 3) = Value(1.000000);
    vm.temp_lval = locals + 120; // color_darkest_grey+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {27, 21};
    *(regs + 0) = Value(1.000000);
    *(regs + 1) = Value(0.500000);
    *(regs + 2) = Value(0.500000);
    *(regs + 3) = Value(1.000000);
    vm.temp_lval = locals + 124; // color_light_red+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {28, 21};
    *(regs + 0) = Value(0.500000);
    *(regs + 1) = Value(1.000000);
    *(regs + 2) = Value(0.500000);
    *(regs + 3) = Value(1.000000);
    vm.temp_lval = locals + 128; // color_light_green+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {29, 21};
    *(regs + 0) = Value(0.500000);
    *(regs + 1) = Value(0.500000);
    *(regs + 2) = Value(1.000000);
    *(regs + 3) = Value(1.000000);
    vm.temp_lval = locals + 132; // color_light_blue+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {30, 21};
    *(regs + 0) = Value(1.000000);
    *(regs + 1) = Value(1.000000);
    *(regs + 2) = Value(0.500000);
    *(regs + 3) = Value(1.000000);
    vm.temp_lval = locals + 136; // color_light_yellow+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {31, 21};
    *(regs + 0) = Value(0.500000);
    *(regs + 1) = Value(1.000000);
    *(regs + 2) = Value(1.000000);
    *(regs + 3) = Value(1.000000);
    vm.temp_lval = locals + 140; // color_light_cyan+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {32, 21};
    *(regs + 0) = Value(1.000000);
    *(regs + 1) = Value(0.500000);
    *(regs + 2) = Value(1.000000);
    *(regs + 3) = Value(1.000000);
    vm.temp_lval = locals + 144; // color_light_pink+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {34, 21};
    *(regs + 0) = Value(0.500000);
    *(regs + 1) = Value(0.000000);
    *(regs + 2) = Value(0.000000);
    *(regs + 3) = Value(1.000000);
    vm.temp_lval = locals + 148; // color_dark_red+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {35, 21};
    *(regs + 0) = Value(0.000000);
    *(regs + 1) = Value(0.500000);
    *(regs + 2) = Value(0.000000);
    *(regs + 3) = Value(1.000000);
    vm.temp_lval = locals + 152; // color_dark_green+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {36, 21};
    *(regs + 0) = Value(0.000000);
    *(regs + 1) = Value(0.000000);
    *(regs + 2) = Value(0.500000);
    *(regs + 3) = Value(1.000000);
    vm.temp_lval = locals + 156; // color_dark_blue+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {37, 21};
    *(regs + 0) = Value(0.500000);
    *(regs + 1) = Value(0.500000);
    *(regs + 2) = Value(0.000000);
    *(regs + 3) = Value(1.000000);
    vm.temp_lval = locals + 160; // color_olive+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {38, 21};
    *(regs + 0) = Value(0.000000);
    *(regs + 1) = Value(0.500000);
    *(regs + 2) = Value(0.500000);
    *(regs + 3) = Value(1.000000);
    vm.temp_lval = locals + 164; // color_teal+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {39, 21};
    *(regs + 0) = Value(0.500000);
    *(regs + 1) = Value(0.000000);
    *(regs + 2) = Value(0.500000);
    *(regs + 3) = Value(1.000000);
    vm.temp_lval = locals + 168; // color_purple+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {41, 21};
    *(regs + 0) = Value(1.000000);
    *(regs + 1) = Value(0.500000);
    *(regs + 2) = Value(0.000000);
    *(regs + 3) = Value(1.000000);
    vm.temp_lval = locals + 172; // color_orange+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {42, 21};
    *(regs + 0) = Value(0.500000);
    *(regs + 1) = Value(1.000000);
    *(regs + 2) = Value(0.000000);
    *(regs + 3) = Value(1.000000);
    vm.temp_lval = locals + 176; // color_chartreuse+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {43, 21};
    *(regs + 0) = Value(0.000000);
    *(regs + 1) = Value(1.000000);
    *(regs + 2) = Value(0.500000);
    *(regs + 3) = Value(1.000000);
    vm.temp_lval = locals + 180; // color_springgreen+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {44, 21};
    *(regs + 0) = Value(0.000000);
    *(regs + 1) = Value(0.500000);
    *(regs + 2) = Value(1.000000);
    *(regs + 3) = Value(1.000000);
    vm.temp_lval = locals + 184; // color_dodgerblue+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {45, 21};
    *(regs + 0) = Value(1.000000);
    *(regs + 1) = Value(0.000000);
    *(regs + 2) = Value(0.500000);
    *(regs + 3) = Value(1.000000);
    vm.temp_lval = locals + 188; // color_pink_violet+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {46, 21};
    *(regs + 0) = Value(0.500000);
    *(regs + 1) = Value(0.000000);
    *(regs + 2) = Value(1.000000);
    *(regs + 3) = Value(1.000000);
    vm.temp_lval = locals + 192; // color_blue_violet+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {48, 21};
    vm.last = {49, 21};
    vm.last = {51, 21};
    U_PUSHFLT64(vm, regs + 0, -446676599, 1070801616); // 0.299
    U_PUSHFLT64(vm, regs + 1, 962072674, 1071827124); // 0.587
    U_PUSHFLT64(vm, regs + 2, -1614907703, 1069362970); // 0.114
    *(regs + 3) = Value(0.000000);
    vm.temp_lval = locals + 196; // color_luminance_weights+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {52, 21};
    vm.last = {7, 20};
    U_PUSHSTR(vm, regs + 0, 12); // "smallpt"
    U_PUSHFUN(vm, regs + 1, 0, fun_370);
    fun_4(vm, regs + 2); // call: run_test
    vm.last = {3, 22};
    U_PUSHSTR(vm, regs + 0, 13); // "string"
    U_PUSHFUN(vm, regs + 1, 0, fun_377);
    fun_4(vm, regs + 2); // call: run_test
    vm.last = {8, 23};
    U_PUSHSTR(vm, regs + 0, 14); // "misch"
    U_PUSHFUN(vm, regs + 1, 0, fun_378);
    fun_4(vm, regs + 2); // call: run_test
    vm.last = {6, 24};
    U_PUSHSTR(vm, regs + 0, 15); // "lifetime"
    U_PUSHFUN(vm, regs + 1, 0, fun_379);
    fun_4(vm, regs + 2); // call: run_test
    vm.last = {6, 25};
    U_PUSHSTR(vm, regs + 0, 16); // "builtins"
    U_PUSHFUN(vm, regs + 1, 0, fun_382);
    fun_4(vm, regs + 2); // call: run_test
    vm.last = {9, 27};
    vm.last = {15, 27};
    vm.last = {18, 27};
    vm.last = {21, 27};
    vm.last = {7, 27};
    vm.last = {7, 27};
    vm.last = {24, 27};
    vm.last = {29, 27};
    vm.last = {32, 27};
    vm.last = {50, 27};
    vm.last = {57, 27};
    vm.last = {62, 27};
    vm.last = {67, 27};
    *(regs + 0) = Value(0.000000);
    *(regs + 1) = Value(0.000000);
    *(regs + 2) = Value(0.000000);
    *(regs + 3) = Value(1.000000);
    vm.temp_lval = locals + 200; // quat_unit+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {10, 26};
    vm.last = {11, 26};
    vm.last = {12, 26};
    vm.last = {13, 26};
    vm.last = {14, 26};
    vm.last = {15, 26};
    vm.last = {16, 26};
    vm.last = {17, 26};
    vm.last = {18, 26};
    vm.last = {19, 26};
    vm.last = {21, 26};
    vm.last = {22, 26};
    vm.last = {23, 26};
    vm.last = {24, 26};
    vm.last = {25, 26};
    vm.last = {26, 26};
    vm.last = {27, 26};
    vm.last = {28, 26};
    vm.last = {29, 26};
    vm.last = {30, 26};
    vm.last = {32, 26};
    vm.last = {33, 26};
    vm.last = {34, 26};
    vm.last = {35, 26};
    vm.last = {36, 26};
    vm.last = {37, 26};
    vm.last = {39, 26};
    vm.last = {40, 26};
    vm.last = {41, 26};
    vm.last = {42, 26};
    vm.last = {44, 26};
    vm.last = {46, 26};
    vm.last = {7, 26};
    vm.last = {7, 26};
    vm.last = {54, 26};
    vm.last = {55, 26};
    vm.last = {56, 26};
    vm.last = {57, 26};
    vm.last = {58, 26};
    vm.last = {59, 26};
    vm.last = {60, 26};
    vm.last = {61, 26};
    vm.last = {62, 26};
    vm.last = {63, 26};
    vm.last = {65, 26};
    vm.last = {66, 26};
    vm.last = {67, 26};
    vm.last = {68, 26};
    vm.last = {69, 26};
    vm.last = {70, 26};
    vm.last = {72, 26};
    vm.last = {73, 26};
    vm.last = {75, 26};
    vm.last = {51, 26};
    vm.last = {51, 26};
    vm.last = {78, 26};
    U_PUSHSTR(vm, regs + 0, 17); // "operators"
    U_PUSHFUN(vm, regs + 1, 0, fun_445);
    fun_4(vm, regs + 2); // call: run_test
    vm.last = {9, 29};
    vm.last = {13, 29};
    vm.last = {17, 29};
    vm.last = {20, 29};
    vm.last = {23, 29};
    vm.last = {26, 29};
    vm.last = {7, 29};
    vm.last = {7, 29};
    vm.last = {33, 29};
    *(regs + 0) = Value(1.000000);
    *(regs + 1) = Value(0.000000);
    *(regs + 2) = Value(0.000000);
    *(regs + 3) = Value(1.000000);
    U_LVAL_VARF(vm, regs + 4, 1150); // mat2x2_identity+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {4, 28};
    U_PUSHSTR(vm, regs + 0, 18); // "matrix"
    U_PUSHFUN(vm, regs + 1, 0, fun_452);
    fun_4(vm, regs + 2); // call: run_test
    vm.last = {4, 30};
    U_PUSHSTR(vm, regs + 0, 19); // "typeswitch"
    U_PUSHFUN(vm, regs + 1, 0, fun_454);
    fun_4(vm, regs + 2); // call: run_test
    vm.last = {30, 0};
    DecOwned(vm, 170);
    DecOwned(vm, 171);
    DecVal(vm, locals[66]);
    DecVal(vm, locals[67]);
    goto epilogue;
    epilogue:;
    RestoreBackup(vm, 1153);
    RestoreBackup(vm, 1152);
    RestoreBackup(vm, 1151);
    RestoreBackup(vm, 1150);
    RestoreBackup(vm, 171);
    RestoreBackup(vm, 170);
    RestoreBackup(vm, 136);
    RestoreBackup(vm, 135);
    RestoreBackup(vm, 134);
    RestoreBackup(vm, 133);
    RestoreBackup(vm, 132);
    RestoreBackup(vm, 131);
    RestoreBackup(vm, 130);
    RestoreBackup(vm, 129);
    RestoreBackup(vm, 128);
    RestoreBackup(vm, 127);
    RestoreBackup(vm, 126);
    RestoreBackup(vm, 125);
    RestoreBackup(vm, 124);
    RestoreBackup(vm, 123);
    RestoreBackup(vm, 95);
    RestoreBackup(vm, 94);
    RestoreBackup(vm, 93);
    RestoreBackup(vm, 92);
    RestoreBackup(vm, 91);
    RestoreBackup(vm, 90);
    RestoreBackup(vm, 89);
    RestoreBackup(vm, 88);
    RestoreBackup(vm, 87);
    RestoreBackup(vm, 86);
    RestoreBackup(vm, 85);
    RestoreBackup(vm, 84);
    RestoreBackup(vm, 77);
    RestoreBackup(vm, 76);
    RestoreBackup(vm, 75);
    RestoreBackup(vm, 74);
    RestoreBackup(vm, 0);
    PopFunId(vm);
}

// function3
static void fun_3(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 244, locals);
    vm.last = {8, 2};
    *(regs + 0) = *(locals + 1); // t
    U_CALLV(vm, regs + 1); vm.next_call_target(vm, regs + 0);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// run_test
static void fun_4(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 249, locals);
    vm.last = {11, 2};
    *(regs + 0) = *(locals + 0); // name
    *(regs + 1) = *(locals + 1); // t
    U_PUSHVARF(vm, regs + 2, 0); // current_test_runner
    U_CALLV(vm, regs + 3); vm.next_call_target(vm, regs + 2);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function5
static void fun_5(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value keepvar[1];
    PushFunId(vm, funinfo_table + 254, 0);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {15, 2};
    U_PUSHSTR(vm, regs + 0, 20); // "a"
    U_PUSHSTR(vm, regs + 1, 21); // "b"
    U_SADD(vm, regs + 2);
    *(keepvar + 0) = *(regs + 0);
    U_PUSHSTR(vm, regs + 1, 22); // "ab"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 15, 2, 23);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// function6
static void fun_6(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value keepvar[1];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 257, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {6, 0};
    U_PUSHSTR(vm, regs + 0, 24); // "testing: "
    *(regs + 1) = *(locals + 0); // name
    U_SADD(vm, regs + 2);
    *(keepvar + 0) = *(regs + 0);
    U_BCALLRET1(vm, regs + 1, 0, 0); // print
    vm.last = {7, 0};
    *(regs + 0) = *(locals + 1); // t
    U_CALLV(vm, regs + 1); vm.next_call_target(vm, regs + 0);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_616(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 262, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {4, 5};
    U_NEWVEC(vm, regs + 0, 11, 0);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {5, 5};
    U_VFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 3; // x
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {5, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 4; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {6, 5};
    *(regs + 2) = *(locals + 2); // r
    *(regs + 3) = *(locals + 3); // x
    fun_460(vm, regs + 4); // call: function403
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {7, 5};
    *(regs + 0) = *(locals + 2); // r
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_615(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 270, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {4, 5};
    U_NEWVEC(vm, regs + 0, 11, 0);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {5, 5};
    U_VFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 3; // x
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {5, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 4; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {6, 5};
    *(regs + 2) = *(locals + 2); // r
    *(regs + 3) = *(locals + 3); // x
    fun_459(vm, regs + 4); // call: function402
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {7, 5};
    *(regs + 0) = *(locals + 2); // r
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_614(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 278, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {4, 5};
    U_NEWVEC(vm, regs + 0, 7, 0);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {5, 5};
    U_VFORELEMREF(vm, regs + 2);
    vm.temp_lval = locals + 3; // x
    U_LV_WRITEREF(vm, regs + 3);
    vm.last = {5, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 4; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {6, 5};
    *(regs + 2) = *(locals + 2); // r
    *(regs + 3) = *(locals + 3); // x
    fun_458(vm, regs + 4); // call: function401
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {7, 5};
    *(regs + 0) = *(locals + 2); // r
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_613(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 286, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {4, 5};
    U_NEWVEC(vm, regs + 0, 7, 0);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {5, 5};
    U_VFORELEMREF(vm, regs + 2);
    vm.temp_lval = locals + 3; // x
    U_LV_WRITEREF(vm, regs + 3);
    vm.last = {5, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 4; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {6, 5};
    *(regs + 2) = *(locals + 2); // r
    *(regs + 3) = *(locals + 3); // x
    fun_457(vm, regs + 4); // call: function400
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {7, 5};
    *(regs + 0) = *(locals + 2); // r
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_612(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 294, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {4, 5};
    U_NEWVEC(vm, regs + 0, 7, 0);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {5, 5};
    U_VFORELEMREF(vm, regs + 2);
    vm.temp_lval = locals + 3; // x
    U_LV_WRITEREF(vm, regs + 3);
    vm.last = {5, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 4; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {6, 5};
    *(regs + 2) = *(locals + 2); // r
    *(regs + 3) = *(locals + 3); // x
    fun_456(vm, regs + 4); // call: function399
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {7, 5};
    *(regs + 0) = *(locals + 2); // r
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_611(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 302, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {4, 5};
    U_NEWVEC(vm, regs + 0, 7, 0);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {5, 5};
    U_VFORELEMREF(vm, regs + 2);
    vm.temp_lval = locals + 3; // x
    U_LV_WRITEREF(vm, regs + 3);
    vm.last = {5, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 4; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {6, 5};
    *(regs + 2) = *(locals + 2); // r
    *(regs + 3) = *(locals + 3); // x
    (regs + 3)->LTINCRTNIL();
    fun_455(vm, regs + 4); // call: function398
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {7, 5};
    *(regs + 0) = *(locals + 2); // r
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_604(VMRef vm, StackPtr psp) {
    Value regs[8];
    Value keepvar[1];
    Value locals[7];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    *(locals + 5) = Value(0, lobster::V_NIL);
    *(locals + 6) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 310, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {4, 5};
    U_NEWVEC(vm, regs + 0, 1256, 0);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {5, 5};
    U_VFORELEM2S(vm, regs + 2);
    vm.temp_lval = locals + 3; // x+0
    U_LV_WRITEV(vm, regs + 5, 3);
    vm.last = {5, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 6; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {6, 5};
    *(regs + 2) = *(locals + 2); // r
    *(regs + 3) = *(locals + 3); // x.0
    *(regs + 4) = *(locals + 4); // x.1
    *(regs + 5) = *(locals + 5); // x.2
    fun_376(vm, regs + 6); // call: function343
    *(regs + 7) = Value(4);
    U_BCALLRETV(vm, regs + 8, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {7, 5};
    *(regs + 0) = *(locals + 2); // r
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_603(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 320, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {4, 5};
    U_NEWVEC(vm, regs + 0, 1258, 0);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {5, 5};
    U_VFORELEMREF(vm, regs + 2);
    vm.temp_lval = locals + 3; // x
    U_LV_WRITEREF(vm, regs + 3);
    vm.last = {5, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 4; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {6, 5};
    *(regs + 2) = *(locals + 2); // r
    *(regs + 3) = *(locals + 3); // x
    fun_375(vm, regs + 4); // call: function342
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {7, 5};
    *(regs + 0) = *(locals + 2); // r
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_602(VMRef vm, StackPtr psp) {
    Value regs[7];
    Value keepvar[1];
    Value locals[5];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 328, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {4, 5};
    U_NEWVEC(vm, regs + 0, 1252, 0);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_IFOR(vm, regs + 2)) goto block1;
    vm.last = {5, 5};
    U_IFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 3; // x
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {5, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 4; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {6, 5};
    *(regs + 2) = *(locals + 2); // r
    vm.last = {119, 20};
    U_PUSHVARVF(vm, regs + 3, 84, 3); // float3_0+0
    *(regs + 6) = Value(3);
    U_BCALLRETV(vm, regs + 7, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {7, 5};
    *(regs + 0) = *(locals + 2); // r
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_601(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 336, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {4, 5};
    U_NEWVEC(vm, regs + 0, 1254, 0);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_IFOR(vm, regs + 2)) goto block1;
    vm.last = {5, 5};
    U_IFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 3; // x
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {5, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 4; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {6, 5};
    *(regs + 2) = *(locals + 2); // r
    fun_373(vm, regs + 3); // call: function340
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {7, 5};
    *(regs + 0) = *(locals + 2); // r
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_593(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 344, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {4, 5};
    U_NEWVEC(vm, regs + 0, 13, 0);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {5, 5};
    U_VFORELEMREF(vm, regs + 2);
    vm.temp_lval = locals + 3; // x
    U_LV_WRITEREF(vm, regs + 3);
    vm.last = {5, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 4; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {6, 5};
    *(regs + 2) = *(locals + 2); // r
    *(regs + 3) = *(locals + 3); // x
    *(regs + 4) = *(locals + 4); // i
    fun_361(vm, regs + 5); // call: function328
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {7, 5};
    *(regs + 0) = *(locals + 2); // r
    DecVal(vm, locals[0]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_592(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[6];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    *(locals + 5) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 352, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {4, 5};
    U_NEWVEC(vm, regs + 0, 13, 0);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {5, 5};
    U_VFORELEM2S(vm, regs + 2);
    vm.temp_lval = locals + 3; // x+0
    U_LV_WRITEV(vm, regs + 4, 2);
    vm.last = {5, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 5; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {6, 5};
    *(regs + 2) = *(locals + 2); // r
    vm.last = {55, 19};
    U_NEWVEC(vm, regs + 3, 7, 0);
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {7, 5};
    *(regs + 0) = *(locals + 2); // r
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_587(VMRef vm, StackPtr psp) {
    Value regs[6];
    Value keepvar[1];
    Value locals[6];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    *(locals + 5) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 361, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {4, 5};
    U_NEWVEC(vm, regs + 0, 1148, 0);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {5, 5};
    U_VFORELEM2S(vm, regs + 2);
    vm.temp_lval = locals + 3; // x+0
    U_LV_WRITEV(vm, regs + 4, 2);
    vm.last = {5, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 5; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {6, 5};
    *(regs + 2) = *(locals + 2); // r
    vm.last = {23, 19};
    U_PUSHVARVF(vm, regs + 3, 74, 2); // float2_0+0
    *(regs + 5) = Value(2);
    U_BCALLRETV(vm, regs + 6, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {7, 5};
    *(regs + 0) = *(locals + 2); // r
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_586(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 370, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {4, 5};
    U_NEWVEC(vm, regs + 0, 9, 0);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_IFOR(vm, regs + 2)) goto block1;
    vm.last = {5, 5};
    U_IFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 3; // x
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {5, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 4; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {6, 5};
    *(regs + 2) = *(locals + 2); // r
    vm.last = {13, 18};
    *(regs + 3) = Value(0.000000);
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {7, 5};
    *(regs + 0) = *(locals + 2); // r
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_585(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 378, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {4, 5};
    U_NEWVEC(vm, regs + 0, 9, 0);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_IFOR(vm, regs + 2)) goto block1;
    vm.last = {5, 5};
    U_IFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 3; // x
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {5, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 4; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {6, 5};
    *(regs + 2) = *(locals + 2); // r
    vm.last = {34, 18};
    *(regs + 3) = Value(0.000000);
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {7, 5};
    *(regs + 0) = *(locals + 2); // r
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_584(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 386, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {4, 5};
    U_NEWVEC(vm, regs + 0, 9, 0);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_IFOR(vm, regs + 2)) goto block1;
    vm.last = {5, 5};
    U_IFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 3; // x
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {5, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 4; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {6, 5};
    *(regs + 2) = *(locals + 2); // r
    vm.last = {33, 18};
    *(regs + 3) = Value(0.000000);
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {7, 5};
    *(regs + 0) = *(locals + 2); // r
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_582(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 394, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {4, 5};
    U_NEWVEC(vm, regs + 0, 1138, 0);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_IFOR(vm, regs + 2)) goto block1;
    vm.last = {5, 5};
    U_IFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 3; // x
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {5, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 4; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {6, 5};
    *(regs + 2) = *(locals + 2); // r
    *(regs + 3) = *(locals + 3); // x
    fun_341(vm, regs + 4); // call: function308
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {7, 5};
    *(regs + 0) = *(locals + 2); // r
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_581(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 402, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {4, 5};
    U_NEWVEC(vm, regs + 0, 1140, 0);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_IFOR(vm, regs + 2)) goto block1;
    vm.last = {5, 5};
    U_IFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 3; // x
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {5, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 4; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {6, 5};
    *(regs + 2) = *(locals + 2); // r
    *(regs + 3) = *(locals + 3); // x
    fun_340(vm, regs + 4); // call: function307
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {7, 5};
    *(regs + 0) = *(locals + 2); // r
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_579(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 410, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {4, 5};
    U_NEWVEC(vm, regs + 0, 1119, 0);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_IFOR(vm, regs + 2)) goto block1;
    vm.last = {5, 5};
    U_IFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 3; // x
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {5, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 4; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {6, 5};
    *(regs + 2) = *(locals + 2); // r
    *(regs + 3) = *(locals + 3); // x
    fun_580(vm, regs + 4); // call: function99
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {7, 5};
    *(regs + 0) = *(locals + 2); // r
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_576(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 418, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {4, 5};
    U_NEWVEC(vm, regs + 0, 1121, 0);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_IFOR(vm, regs + 2)) goto block1;
    vm.last = {5, 5};
    U_IFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 3; // x
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {5, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 4; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {6, 5};
    *(regs + 2) = *(locals + 2); // r
    *(regs + 3) = *(locals + 3); // x
    fun_577(vm, regs + 4); // call: function98
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {7, 5};
    *(regs + 0) = *(locals + 2); // r
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_573(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 426, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {4, 5};
    U_NEWVEC(vm, regs + 0, 11, 0);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {5, 5};
    U_VFORELEMREF(vm, regs + 2);
    vm.temp_lval = locals + 3; // x
    U_LV_WRITEREF(vm, regs + 3);
    vm.last = {5, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 4; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {6, 5};
    *(regs + 2) = *(locals + 2); // r
    *(regs + 3) = *(locals + 3); // x
    fun_324(vm, regs + 4); // call: function291
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {7, 5};
    *(regs + 0) = *(locals + 2); // r
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_566(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[6];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    *(locals + 5) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 434, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {4, 5};
    U_NEWVEC(vm, regs + 0, 7, 0);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {5, 5};
    U_VFORELEM2S(vm, regs + 2);
    vm.temp_lval = locals + 3; // x+0
    U_LV_WRITEV(vm, regs + 4, 2);
    vm.last = {5, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 5; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {6, 5};
    *(regs + 2) = *(locals + 2); // r
    *(regs + 3) = *(locals + 3); // x.0
    *(regs + 4) = *(locals + 4); // x.1
    fun_311(vm, regs + 5); // call: function281
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {7, 5};
    *(regs + 0) = *(locals + 2); // r
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_565(VMRef vm, StackPtr psp) {
    Value regs[6];
    Value keepvar[1];
    Value locals[6];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    *(locals + 5) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 443, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {4, 5};
    U_NEWVEC(vm, regs + 0, 37, 0);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {5, 5};
    U_VFORELEM2S(vm, regs + 2);
    vm.temp_lval = locals + 3; // x+0
    U_LV_WRITEV(vm, regs + 4, 2);
    vm.last = {5, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 5; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {6, 5};
    *(regs + 2) = *(locals + 2); // r
    *(regs + 3) = *(locals + 3); // x.0
    *(regs + 4) = *(locals + 4); // x.1
    fun_308(vm, regs + 5); // call: function278
    *(regs + 5) = Value(2);
    U_BCALLRETV(vm, regs + 6, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {7, 5};
    *(regs + 0) = *(locals + 2); // r
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_564(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 452, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {4, 5};
    U_NEWVEC(vm, regs + 0, 7, 0);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_IFOR(vm, regs + 2)) goto block1;
    vm.last = {5, 5};
    U_IFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 3; // x
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {5, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 4; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {6, 5};
    *(regs + 2) = *(locals + 2); // r
    *(regs + 3) = *(locals + 3); // x
    fun_102(vm, regs + 4); // call: function99
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {7, 5};
    *(regs + 0) = *(locals + 2); // r
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_563(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 460, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {4, 5};
    U_NEWVEC(vm, regs + 0, 13, 0);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_IFOR(vm, regs + 2)) goto block1;
    vm.last = {5, 5};
    U_IFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 3; // x
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {5, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 4; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {6, 5};
    *(regs + 2) = *(locals + 2); // r
    *(regs + 3) = *(locals + 3); // x
    fun_101(vm, regs + 4); // call: function98
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {7, 5};
    *(regs + 0) = *(locals + 2); // r
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_562(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 468, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {4, 5};
    U_NEWVEC(vm, regs + 0, 11, 0);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {5, 5};
    U_VFORELEMREF(vm, regs + 2);
    vm.temp_lval = locals + 3; // x
    U_LV_WRITEREF(vm, regs + 3);
    vm.last = {5, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 4; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {6, 5};
    *(regs + 2) = *(locals + 2); // r
    *(regs + 3) = *(locals + 3); // x
    fun_304(vm, regs + 4); // call: function274
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {7, 5};
    *(regs + 0) = *(locals + 2); // r
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_558(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 476, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {4, 5};
    U_NEWVEC(vm, regs + 0, 11, 0);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {5, 5};
    U_VFORELEMREF(vm, regs + 2);
    vm.temp_lval = locals + 3; // x
    U_LV_WRITEREF(vm, regs + 3);
    vm.last = {5, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 4; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {6, 5};
    *(regs + 2) = *(locals + 2); // r
    *(regs + 3) = *(locals + 3); // x
    fun_287(vm, regs + 4); // call: function267
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {7, 5};
    *(regs + 0) = *(locals + 2); // r
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_543(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 484, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {4, 5};
    U_NEWVEC(vm, regs + 0, 1005, 0);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_IFOR(vm, regs + 2)) goto block1;
    vm.last = {5, 5};
    U_IFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 3; // x
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {5, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 4; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {6, 5};
    *(regs + 2) = *(locals + 2); // r
    *(regs + 3) = *(locals + 3); // x
    fun_284(vm, regs + 4); // call: function264
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {7, 5};
    *(regs + 0) = *(locals + 2); // r
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_542(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 492, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {4, 5};
    U_NEWVEC(vm, regs + 0, 1027, 0);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_IFOR(vm, regs + 2)) goto block1;
    vm.last = {5, 5};
    U_IFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 3; // x
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {5, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 4; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {6, 5};
    *(regs + 2) = *(locals + 2); // r
    *(regs + 3) = *(locals + 3); // x
    fun_283(vm, regs + 4); // call: function263
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {7, 5};
    *(regs + 0) = *(locals + 2); // r
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_532(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 500, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {4, 5};
    U_NEWVEC(vm, regs + 0, 540, 0);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_IFOR(vm, regs + 2)) goto block1;
    vm.last = {5, 5};
    U_IFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 3; // x
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {5, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 4; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {6, 5};
    *(regs + 2) = *(locals + 2); // r
    vm.last = {120, 8};
    U_PUSHNIL(vm, regs + 3);
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {7, 5};
    *(regs + 0) = *(locals + 2); // r
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_505(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 508, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {4, 5};
    U_NEWVEC(vm, regs + 0, 542, 0);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_IFOR(vm, regs + 2)) goto block1;
    vm.last = {5, 5};
    U_IFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 3; // x
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {5, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 4; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {6, 5};
    *(regs + 2) = *(locals + 2); // r
    vm.last = {120, 8};
    U_PUSHNIL(vm, regs + 3);
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {7, 5};
    *(regs + 0) = *(locals + 2); // r
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_502(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 516, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {4, 5};
    U_NEWVEC(vm, regs + 0, 7, 0);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_IFOR(vm, regs + 2)) goto block1;
    vm.last = {5, 5};
    U_IFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 3; // x
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {5, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 4; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {6, 5};
    *(regs + 2) = *(locals + 2); // r
    *(regs + 3) = *(locals + 3); // x
    fun_200(vm, regs + 4); // call: function192
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {7, 5};
    *(regs + 0) = *(locals + 2); // r
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_492(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 524, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {4, 5};
    U_NEWVEC(vm, regs + 0, 697, 0);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_IFOR(vm, regs + 2)) goto block1;
    vm.last = {5, 5};
    U_IFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 3; // x
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {5, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 4; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {6, 5};
    *(regs + 2) = *(locals + 2); // r
    vm.last = {120, 8};
    U_PUSHNIL(vm, regs + 3);
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {7, 5};
    *(regs + 0) = *(locals + 2); // r
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_485(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 532, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {4, 5};
    U_NEWVEC(vm, regs + 0, 657, 0);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_IFOR(vm, regs + 2)) goto block1;
    vm.last = {5, 5};
    U_IFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 3; // x
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {5, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 4; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {6, 5};
    *(regs + 2) = *(locals + 2); // r
    vm.last = {120, 8};
    U_PUSHNIL(vm, regs + 3);
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {7, 5};
    *(regs + 0) = *(locals + 2); // r
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_482(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 540, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {4, 5};
    U_NEWVEC(vm, regs + 0, 486, 0);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_IFOR(vm, regs + 2)) goto block1;
    vm.last = {5, 5};
    U_IFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 3; // x
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {5, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 4; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {6, 5};
    *(regs + 2) = *(locals + 2); // r
    vm.last = {120, 8};
    U_PUSHNIL(vm, regs + 3);
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {7, 5};
    *(regs + 0) = *(locals + 2); // r
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_471(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 548, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {4, 5};
    U_NEWVEC(vm, regs + 0, 7, 0);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {5, 5};
    U_VFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 3; // x
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {5, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 4; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {6, 5};
    *(regs + 2) = *(locals + 2); // r
    *(regs + 3) = *(locals + 3); // x
    fun_167(vm, regs + 4); // call: function160
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {7, 5};
    *(regs + 0) = *(locals + 2); // r
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_7(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 556, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {4, 5};
    U_NEWVEC(vm, regs + 0, 11, 0);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_IFOR(vm, regs + 2)) goto block1;
    vm.last = {5, 5};
    U_IFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 3; // x
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {5, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 4; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {6, 5};
    *(regs + 2) = *(locals + 2); // r
    *(regs + 3) = *(locals + 3); // x
    fun_162(vm, regs + 4); // call: function155
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {7, 5};
    *(regs + 0) = *(locals + 2); // r
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// filter
static void fun_567(VMRef vm, StackPtr psp) {
    Value regs[6];
    Value locals[6];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    *(locals + 5) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 564, locals);
    vm.last = {12, 5};
    U_NEWVEC(vm, regs + 0, 37, 0);
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {13, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {13, 5};
    U_VFORELEM2S(vm, regs + 2);
    vm.temp_lval = locals + 3; // x+0
    U_LV_WRITEV(vm, regs + 4, 2);
    vm.last = {13, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 5; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {14, 5};
    *(regs + 2) = *(locals + 3); // x.0
    *(regs + 3) = *(locals + 4); // x.1
    fun_312(vm, regs + 4); // call: function282
    if (!(regs + 2)->True()) goto block2;
    vm.last = {15, 5};
    *(regs + 2) = *(locals + 2); // r
    *(regs + 3) = *(locals + 3); // x.0
    *(regs + 4) = *(locals + 4); // x.1
    *(regs + 5) = Value(2);
    U_BCALLRETV(vm, regs + 6, 15, 1); // push
    U_POP(vm, regs + 3);
    block2:;
    goto block0;
    block1:;
    vm.last = {16, 5};
    *(regs + 0) = *(locals + 2); // r
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// filter
static void fun_10(VMRef vm, StackPtr psp) {
    Value regs[6];
    Value locals[6];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    *(locals + 5) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 573, locals);
    vm.last = {12, 5};
    U_NEWVEC(vm, regs + 0, 37, 0);
    vm.temp_lval = locals + 2; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {13, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {13, 5};
    U_VFORELEM2S(vm, regs + 2);
    vm.temp_lval = locals + 3; // x+0
    U_LV_WRITEV(vm, regs + 4, 2);
    vm.last = {13, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 5; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {14, 5};
    *(regs + 2) = *(locals + 3); // x.0
    *(regs + 3) = *(locals + 4); // x.1
    fun_310(vm, regs + 4); // call: function280
    if (!(regs + 2)->True()) goto block2;
    vm.last = {15, 5};
    *(regs + 2) = *(locals + 2); // r
    *(regs + 3) = *(locals + 3); // x.0
    *(regs + 4) = *(locals + 4); // x.1
    *(regs + 5) = Value(2);
    U_BCALLRETV(vm, regs + 6, 15, 1); // push
    U_POP(vm, regs + 3);
    block2:;
    goto block0;
    block1:;
    vm.last = {16, 5};
    *(regs + 0) = *(locals + 2); // r
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// partition
static void fun_595(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value locals[6];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    *(locals + 5) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 582, locals);
    vm.last = {37, 5};
    U_NEWVEC(vm, regs + 0, 7, 0);
    vm.temp_lval = locals + 2; // t
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {38, 5};
    U_NEWVEC(vm, regs + 0, 7, 0);
    vm.temp_lval = locals + 3; // f
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {39, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {39, 5};
    U_VFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 4; // x
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {39, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 5; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {40, 5};
    *(regs + 2) = *(locals + 4); // x
    fun_596(vm, regs + 3); // call: function49
    if (!(regs + 2)->True()) goto block2;
    vm.last = {41, 5};
    *(regs + 2) = *(locals + 2); // t
    *(regs + 3) = *(locals + 4); // x
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block3;
    block2:;
    vm.last = {43, 5};
    *(regs + 2) = *(locals + 3); // f
    *(regs + 3) = *(locals + 4); // x
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    block3:;
    goto block0;
    block1:;
    vm.last = {44, 5};
    *(regs + 0) = *(locals + 2); // t
    *(regs + 1) = *(locals + 3); // f
    (regs + 1)->LTINCRTNIL();
    (regs + 0)->LTINCRTNIL();
    DecVal(vm, locals[0]);
    DecVal(vm, locals[2]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// partition
static void fun_590(VMRef vm, StackPtr psp) {
    Value regs[7];
    Value locals[8];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    *(locals + 5) = Value(0, lobster::V_NIL);
    *(locals + 6) = Value(0, lobster::V_NIL);
    *(locals + 7) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 591, locals);
    vm.last = {37, 5};
    U_NEWVEC(vm, regs + 0, 1165, 0);
    vm.temp_lval = locals + 2; // t
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {38, 5};
    U_NEWVEC(vm, regs + 0, 1165, 0);
    vm.temp_lval = locals + 3; // f
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {39, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {39, 5};
    U_VFORELEM2S(vm, regs + 2);
    vm.temp_lval = locals + 4; // x+0
    U_LV_WRITEV(vm, regs + 5, 3);
    vm.last = {39, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 7; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {40, 5};
    *(regs + 2) = *(locals + 4); // x.0
    *(regs + 3) = *(locals + 5); // x.1
    *(regs + 4) = *(locals + 6); // x.2
    fun_591(vm, regs + 5); // call: function49
    if (!(regs + 2)->True()) goto block2;
    vm.last = {41, 5};
    *(regs + 2) = *(locals + 2); // t
    *(regs + 3) = *(locals + 4); // x.0
    *(regs + 4) = *(locals + 5); // x.1
    *(regs + 5) = *(locals + 6); // x.2
    *(regs + 6) = Value(3);
    U_BCALLRETV(vm, regs + 7, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block3;
    block2:;
    vm.last = {43, 5};
    *(regs + 2) = *(locals + 3); // f
    *(regs + 3) = *(locals + 4); // x.0
    *(regs + 4) = *(locals + 5); // x.1
    *(regs + 5) = *(locals + 6); // x.2
    *(regs + 6) = Value(3);
    U_BCALLRETV(vm, regs + 7, 15, 1); // push
    U_POP(vm, regs + 3);
    block3:;
    goto block0;
    block1:;
    vm.last = {44, 5};
    *(regs + 0) = *(locals + 2); // t
    *(regs + 1) = *(locals + 3); // f
    (regs + 1)->LTINCRTNIL();
    (regs + 0)->LTINCRTNIL();
    DecVal(vm, locals[0]);
    DecVal(vm, locals[2]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// partition
static void fun_13(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value locals[6];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    *(locals + 5) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 602, locals);
    vm.last = {37, 5};
    U_NEWVEC(vm, regs + 0, 7, 0);
    vm.temp_lval = locals + 2; // t
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {38, 5};
    U_NEWVEC(vm, regs + 0, 7, 0);
    vm.temp_lval = locals + 3; // f
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {39, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {39, 5};
    U_VFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 4; // x
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {39, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 5; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {40, 5};
    *(regs + 2) = *(locals + 4); // x
    fun_49(vm, regs + 3); // call: function49
    if (!(regs + 2)->True()) goto block2;
    vm.last = {41, 5};
    *(regs + 2) = *(locals + 2); // t
    *(regs + 3) = *(locals + 4); // x
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block3;
    block2:;
    vm.last = {43, 5};
    *(regs + 2) = *(locals + 3); // f
    *(regs + 3) = *(locals + 4); // x
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    block3:;
    goto block0;
    block1:;
    vm.last = {44, 5};
    *(regs + 0) = *(locals + 2); // t
    *(regs + 1) = *(locals + 3); // f
    (regs + 1)->LTINCRTNIL();
    (regs + 0)->LTINCRTNIL();
    DecVal(vm, locals[0]);
    DecVal(vm, locals[2]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// exists
static void fun_600(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[4];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 611, locals);
    vm.last = {57, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {57, 5};
    U_VFORELEMREF(vm, regs + 2);
    vm.temp_lval = locals + 2; // x
    U_LV_WRITEREF(vm, regs + 3);
    vm.last = {57, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 3; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {58, 5};
    *(regs + 2) = *(locals + 2); // x
    fun_366(vm, regs + 3); // call: function333
    if (!(regs + 2)->True()) goto block2;
    vm.last = {59, 5};
    U_POP(vm, regs + 2);
    U_POP(vm, regs + 1);
    *(regs + 0) = Value(1);
    DecVal(vm, locals[2]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    block2:;
    goto block0;
    block1:;
    vm.last = {60, 5};
    *(regs + 0) = Value(0);
    DecVal(vm, locals[2]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// exists
static void fun_599(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[4];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 618, locals);
    vm.last = {57, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {57, 5};
    U_VFORELEMREF(vm, regs + 2);
    vm.temp_lval = locals + 2; // x
    U_LV_WRITEREF(vm, regs + 3);
    vm.last = {57, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 3; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {58, 5};
    *(regs + 2) = *(locals + 2); // x
    fun_365(vm, regs + 3); // call: function332
    if (!(regs + 2)->True()) goto block2;
    vm.last = {59, 5};
    U_POP(vm, regs + 2);
    U_POP(vm, regs + 1);
    *(regs + 0) = Value(1);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[2]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    block2:;
    goto block0;
    block1:;
    vm.last = {60, 5};
    *(regs + 0) = Value(0);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[2]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// exists
static void fun_568(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[4];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 625, locals);
    vm.last = {57, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {57, 5};
    U_VFORELEMREF(vm, regs + 2);
    vm.temp_lval = locals + 2; // x
    U_LV_WRITEREF(vm, regs + 3);
    vm.last = {57, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 3; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {58, 5};
    *(regs + 2) = *(locals + 2); // x
    fun_333(vm, regs + 3); // call: function300
    if (!(regs + 2)->True()) goto block2;
    vm.last = {59, 5};
    U_POP(vm, regs + 2);
    U_POP(vm, regs + 1);
    *(regs + 0) = Value(1);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[2]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    block2:;
    goto block0;
    block1:;
    vm.last = {60, 5};
    *(regs + 0) = Value(0);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[2]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// exists
static void fun_15(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[4];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 632, locals);
    vm.last = {57, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_SFOR(vm, regs + 2)) goto block1;
    vm.last = {57, 5};
    U_SFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 2; // x
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {57, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 3; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {58, 5};
    *(regs + 2) = *(locals + 2); // x
    fun_329(vm, regs + 3); // call: function296
    if (!(regs + 2)->True()) goto block2;
    vm.last = {59, 5};
    U_POP(vm, regs + 2);
    U_POP(vm, regs + 1);
    *(regs + 0) = Value(1);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    block2:;
    goto block0;
    block1:;
    vm.last = {60, 5};
    *(regs + 0) = Value(0);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// fold
static void fun_572(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[4];
    *(locals + 0) = *(psp - 3);
    *(locals + 1) = *(psp - 2);
    *(locals + 2) = *(psp - 1);
    *(locals + 3) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 639, locals);
    vm.last = {67, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {68, 5};
    U_VFORELEMREF(vm, regs + 2);
    vm.temp_lval = locals + 3; // _
    U_LV_WRITEREF(vm, regs + 3);
    vm.last = {68, 5};
    *(regs + 2) = *(locals + 1); // acc
    *(regs + 3) = *(locals + 3); // _
    fun_322(vm, regs + 4); // call: function290
    vm.temp_lval = locals + 1; // acc
    *(vm.temp_lval) = *(regs + 2);
    goto block0;
    block1:;
    vm.last = {69, 5};
    *(regs + 0) = *(locals + 1); // acc
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// fold
static void fun_571(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[4];
    *(locals + 0) = *(psp - 3);
    *(locals + 1) = *(psp - 2);
    *(locals + 2) = *(psp - 1);
    *(locals + 3) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 646, locals);
    vm.last = {67, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {68, 5};
    U_VFORELEMREF(vm, regs + 2);
    vm.temp_lval = locals + 3; // _
    U_LV_WRITEREF(vm, regs + 3);
    vm.last = {68, 5};
    *(regs + 2) = *(locals + 1); // acc
    *(regs + 3) = *(locals + 3); // _
    fun_321(vm, regs + 4); // call: function289
    vm.temp_lval = locals + 1; // acc
    *(vm.temp_lval) = *(regs + 2);
    goto block0;
    block1:;
    vm.last = {69, 5};
    *(regs + 0) = *(locals + 1); // acc
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// fold
static void fun_570(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[4];
    *(locals + 0) = *(psp - 3);
    *(locals + 1) = *(psp - 2);
    *(locals + 2) = *(psp - 1);
    *(locals + 3) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 653, locals);
    vm.last = {67, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {68, 5};
    U_VFORELEMREF(vm, regs + 2);
    vm.temp_lval = locals + 3; // _
    U_LV_WRITEREF(vm, regs + 3);
    vm.last = {68, 5};
    *(regs + 2) = *(locals + 1); // acc
    *(regs + 3) = *(locals + 3); // _
    fun_319(vm, regs + 4); // call: function288
    vm.temp_lval = locals + 1; // acc
    U_LV_WRITEREF(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {69, 5};
    *(regs + 0) = *(locals + 1); // acc
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// fold
static void fun_559(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[4];
    *(locals + 0) = *(psp - 3);
    *(locals + 1) = *(psp - 2);
    *(locals + 2) = *(psp - 1);
    *(locals + 3) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 660, locals);
    vm.last = {67, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {68, 5};
    U_VFORELEMREF(vm, regs + 2);
    vm.temp_lval = locals + 3; // _
    U_LV_WRITEREF(vm, regs + 3);
    vm.last = {68, 5};
    *(regs + 2) = *(locals + 1); // acc
    *(regs + 3) = *(locals + 3); // _
    fun_288(vm, regs + 4); // call: function268
    vm.temp_lval = locals + 1; // acc
    U_LV_WRITEREF(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {69, 5};
    *(regs + 0) = *(locals + 1); // acc
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// fold
static void fun_481(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[4];
    *(locals + 0) = *(psp - 3);
    *(locals + 1) = *(psp - 2);
    *(locals + 2) = *(psp - 1);
    *(locals + 3) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 667, locals);
    vm.last = {67, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {68, 5};
    U_VFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 3; // _
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {68, 5};
    *(regs + 2) = *(locals + 1); // acc
    *(regs + 3) = *(locals + 3); // _
    fun_183(vm, regs + 4); // call: function176
    vm.temp_lval = locals + 1; // acc
    *(vm.temp_lval) = *(regs + 2);
    goto block0;
    block1:;
    vm.last = {69, 5};
    Pop(psp);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// fold
static void fun_17(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[4];
    *(locals + 0) = *(psp - 3);
    *(locals + 1) = *(psp - 2);
    *(locals + 2) = *(psp - 1);
    *(locals + 3) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 674, locals);
    vm.last = {67, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {68, 5};
    U_VFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 3; // _
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {68, 5};
    *(regs + 2) = *(locals + 1); // acc
    *(regs + 3) = *(locals + 3); // _
    fun_30(vm, regs + 4); // call: function30
    vm.temp_lval = locals + 1; // acc
    *(vm.temp_lval) = *(regs + 2);
    goto block0;
    block1:;
    vm.last = {69, 5};
    *(regs + 0) = *(locals + 1); // acc
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// reduce
static void fun_598(VMRef vm, StackPtr psp) {
    Value regs[6];
    Value locals[4];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 681, locals);
    vm.last = {77, 5};
    *(regs + 0) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 1, 13, 1); // length
    U_ASSERT(vm, regs + 1, 77, 5, 25);
    vm.last = {78, 5};
    *(regs + 0) = *(locals + 0); // xs
    *(regs + 1) = Value(0);
    U_VPUSHIDXI(vm, regs + 2);
    vm.temp_lval = locals + 2; // acc
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {79, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    *(regs + 2) = Value(1);
    U_ISUB(vm, regs + 3);
    block0:;
    if (!U_IFOR(vm, regs + 2)) goto block1;
    vm.last = {79, 5};
    U_IFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 3; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {79, 5};
    *(regs + 2) = *(locals + 2); // acc
    *(regs + 3) = *(locals + 0); // xs
    *(regs + 4) = *(locals + 3); // i
    *(regs + 5) = Value(1);
    U_IADD(vm, regs + 6);
    U_VPUSHIDXI(vm, regs + 5);
    fun_364(vm, regs + 4); // call: function331
    vm.temp_lval = locals + 2; // acc
    *(vm.temp_lval) = *(regs + 2);
    goto block0;
    block1:;
    vm.last = {80, 5};
    *(regs + 0) = *(locals + 2); // acc
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// reduce
static void fun_19(VMRef vm, StackPtr psp) {
    Value regs[6];
    Value locals[4];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 688, locals);
    vm.last = {77, 5};
    *(regs + 0) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 1, 13, 1); // length
    U_ASSERT(vm, regs + 1, 77, 5, 26);
    vm.last = {78, 5};
    *(regs + 0) = *(locals + 0); // xs
    *(regs + 1) = Value(0);
    U_VPUSHIDXI(vm, regs + 2);
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // acc
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {79, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    *(regs + 2) = Value(1);
    U_ISUB(vm, regs + 3);
    block0:;
    if (!U_IFOR(vm, regs + 2)) goto block1;
    vm.last = {79, 5};
    U_IFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 3; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {79, 5};
    *(regs + 2) = *(locals + 2); // acc
    *(regs + 3) = *(locals + 0); // xs
    *(regs + 4) = *(locals + 3); // i
    *(regs + 5) = Value(1);
    U_IADD(vm, regs + 6);
    U_VPUSHIDXI(vm, regs + 5);
    fun_325(vm, regs + 4); // call: function292
    vm.temp_lval = locals + 2; // acc
    U_LV_WRITEREF(vm, regs + 3);
    goto block0;
    block1:;
    vm.last = {80, 5};
    *(regs + 0) = *(locals + 2); // acc
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// find
static void fun_597(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[4];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 695, locals);
    vm.last = {93, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {93, 5};
    U_VFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 2; // x
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {93, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 3; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {94, 5};
    *(regs + 2) = *(locals + 2); // x
    fun_363(vm, regs + 3); // call: function330
    if (!(regs + 2)->True()) goto block2;
    vm.last = {95, 5};
    U_POP(vm, regs + 2);
    U_POP(vm, regs + 1);
    *(regs + 0) = *(locals + 3); // i
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    block2:;
    goto block0;
    block1:;
    vm.last = {96, 5};
    *(regs + 0) = Value(-1);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// find
static void fun_23(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[4];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 702, locals);
    vm.last = {93, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {93, 5};
    U_VFORELEMREF(vm, regs + 2);
    vm.temp_lval = locals + 2; // x
    U_LV_WRITEREF(vm, regs + 3);
    vm.last = {93, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 3; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {94, 5};
    *(regs + 2) = *(locals + 2); // x
    fun_280(vm, regs + 3); // call: function260
    if (!(regs + 2)->True()) goto block2;
    vm.last = {95, 5};
    U_POP(vm, regs + 2);
    U_POP(vm, regs + 1);
    *(regs + 0) = *(locals + 3); // i
    DecVal(vm, locals[2]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    block2:;
    goto block0;
    block1:;
    vm.last = {96, 5};
    *(regs + 0) = Value(-1);
    DecVal(vm, locals[2]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// find_best
static void fun_26(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[8];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    *(locals + 5) = Value(0, lobster::V_NIL);
    *(locals + 6) = Value(0, lobster::V_NIL);
    *(locals + 7) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 709, locals);
    vm.last = {115, 5};
    *(regs + 0) = Value(0);
    vm.temp_lval = locals + 2; // init
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {116, 5};
    *(regs + 0) = *(locals + 2); // init
    vm.temp_lval = locals + 3; // best
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {117, 5};
    *(regs + 0) = Value(-1);
    vm.temp_lval = locals + 4; // i
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {118, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {118, 5};
    U_VFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 5; // x
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {118, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 6; // j
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {119, 5};
    *(regs + 2) = *(locals + 5); // x
    fun_313(vm, regs + 3); // call: function283
    vm.temp_lval = locals + 7; // v
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {120, 5};
    *(regs + 2) = *(locals + 4); // i
    *(regs + 3) = Value(0);
    U_ILT(vm, regs + 4);
    if ((regs + 2)->True()) goto block2;
    U_POP(vm, regs + 3);
    *(regs + 2) = *(locals + 7); // v
    *(regs + 3) = *(locals + 3); // best
    U_IGT(vm, regs + 4);
    block2:;
    if (!(regs + 2)->True()) goto block3;
    vm.last = {121, 5};
    *(regs + 2) = *(locals + 6); // j
    vm.temp_lval = locals + 4; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {122, 5};
    *(regs + 2) = *(locals + 7); // v
    vm.temp_lval = locals + 3; // best
    *(vm.temp_lval) = *(regs + 2);
    block3:;
    goto block0;
    block1:;
    vm.last = {123, 5};
    *(regs + 0) = *(locals + 4); // i
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// sum
static void fun_472(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    *(locals + 0) = *(psp - 1);
    *(locals + 1) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 720, locals);
    vm.last = {140, 5};
    *(regs + 0) = Value(0);
    vm.temp_lval = locals + 1; // init
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {141, 5};
    *(regs + 0) = *(locals + 0); // xs
    *(regs + 1) = *(locals + 1); // init
    U_PUSHFUN(vm, regs + 2, 0, fun_30);
    fun_17(vm, regs + 3); // call: fold
    DecVal(vm, locals[0]);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// sum
static void fun_29(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    *(locals + 0) = *(psp - 1);
    *(locals + 1) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 725, locals);
    vm.last = {140, 5};
    *(regs + 0) = Value(0);
    vm.temp_lval = locals + 1; // init
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {141, 5};
    *(regs + 0) = *(locals + 0); // xs
    *(regs + 1) = *(locals + 1); // init
    U_PUSHFUN(vm, regs + 2, 0, fun_30);
    fun_17(vm, regs + 3); // call: fold
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function30
static void fun_30(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 730, locals);
    vm.last = {141, 5};
    *(regs + 0) = *(locals + 0); // _x
    *(regs + 1) = *(locals + 1); // _y
    U_IADD(vm, regs + 2);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// qsort
static void fun_594(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value keepvar[4];
    Value locals[4];
    *(locals + 0) = *(psp - 2);
    SwapVars(vm, 1549, psp, 1);
    BackupVar(vm, 1550);
    *(locals + 1) = Value(0, lobster::V_NIL);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 735, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    *(keepvar + 1) = Value(0, lobster::V_NIL);
    *(keepvar + 2) = Value(0, lobster::V_NIL);
    *(keepvar + 3) = Value(0, lobster::V_NIL);
    vm.last = {194, 5};
    *(regs + 0) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 1, 13, 1); // length
    *(regs + 1) = Value(1);
    U_IGT(vm, regs + 2);
    if (!(regs + 0)->True()) goto block0;
    vm.last = {196, 5};
    *(regs + 0) = *(locals + 0); // xs
    *(regs + 1) = Value(0);
    U_VPUSHIDXI(vm, regs + 2);
    U_LVAL_VARF(vm, regs + 1, 1550); // pivot
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {197, 5};
    *(regs + 0) = *(locals + 0); // xs
    *(regs + 1) = Value(1);
    *(regs + 2) = Value(-1);
    U_BCALLRET3(vm, regs + 3, 31, 1); // slice
    vm.temp_lval = locals + 1; // tail
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {198, 5};
    *(regs + 0) = *(locals + 1); // tail
    (regs + 0)->LTINCRTNIL();
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_595(vm, regs + 2); // call: partition
    vm.temp_lval = locals + 3; // f2
    U_LV_WRITEREF(vm, regs + 2);
    vm.temp_lval = locals + 2; // f1
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {200, 5};
    *(regs + 0) = *(locals + 2); // f1
    (regs + 0)->LTINCRTNIL();
    U_PUSHVARF(vm, regs + 1, 1549); // lt
    fun_594(vm, regs + 2); // call: qsort
    *(keepvar + 0) = *(regs + 0);
    U_PUSHVARF(vm, regs + 1, 1550); // pivot
    U_NEWVEC(vm, regs + 2, 7, 1);
    *(keepvar + 1) = *(regs + 1);
    U_BCALLRET2(vm, regs + 2, 8, 1); // append
    *(keepvar + 2) = *(regs + 0);
    *(regs + 1) = *(locals + 3); // f2
    (regs + 1)->LTINCRTNIL();
    U_PUSHVARF(vm, regs + 2, 1549); // lt
    fun_594(vm, regs + 3); // call: qsort
    *(keepvar + 3) = *(regs + 1);
    U_BCALLRET2(vm, regs + 2, 8, 1); // append
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[2]);
    DecVal(vm, locals[3]);
    psp = PopArg(vm, 1549, psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    goto block1;
    block0:;
    vm.last = {195, 5};
    *(regs + 0) = *(locals + 0); // xs
    (regs + 0)->LTINCRTNIL();
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[2]);
    DecVal(vm, locals[3]);
    psp = PopArg(vm, 1549, psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    block1:;
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[2]);
    DecVal(vm, locals[3]);
    psp = PopArg(vm, 1549, psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    RestoreBackup(vm, 1550);
    DecVal(vm, keepvar[0]);
    DecVal(vm, keepvar[1]);
    DecVal(vm, keepvar[2]);
    DecVal(vm, keepvar[3]);
    PopFunId(vm);
}

// qsort
static void fun_588(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value keepvar[4];
    Value locals[4];
    *(locals + 0) = *(psp - 2);
    SwapVars(vm, 1519, psp, 1);
    BackupVar(vm, 1520);
    BackupVar(vm, 1521);
    BackupVar(vm, 1522);
    *(locals + 1) = Value(0, lobster::V_NIL);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 744, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    *(keepvar + 1) = Value(0, lobster::V_NIL);
    *(keepvar + 2) = Value(0, lobster::V_NIL);
    *(keepvar + 3) = Value(0, lobster::V_NIL);
    vm.last = {194, 5};
    *(regs + 0) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 1, 13, 1); // length
    *(regs + 1) = Value(1);
    U_IGT(vm, regs + 2);
    if (!(regs + 0)->True()) goto block0;
    vm.last = {196, 5};
    *(regs + 0) = *(locals + 0); // xs
    *(regs + 1) = Value(0);
    U_VPUSHIDXI2V(vm, regs + 2);
    U_LVAL_VARF(vm, regs + 3, 1520); // pivot+0
    U_LV_WRITEV(vm, regs + 3, 3);
    vm.last = {197, 5};
    *(regs + 0) = *(locals + 0); // xs
    *(regs + 1) = Value(1);
    *(regs + 2) = Value(-1);
    U_BCALLRET3(vm, regs + 3, 31, 1); // slice
    vm.temp_lval = locals + 1; // tail
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {198, 5};
    *(regs + 0) = *(locals + 1); // tail
    (regs + 0)->LTINCRTNIL();
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_590(vm, regs + 2); // call: partition
    vm.temp_lval = locals + 3; // f2
    U_LV_WRITEREF(vm, regs + 2);
    vm.temp_lval = locals + 2; // f1
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {200, 5};
    *(regs + 0) = *(locals + 2); // f1
    (regs + 0)->LTINCRTNIL();
    U_PUSHVARF(vm, regs + 1, 1519); // lt
    fun_588(vm, regs + 2); // call: qsort
    *(keepvar + 0) = *(regs + 0);
    U_PUSHVARVF(vm, regs + 1, 1520, 3); // pivot+0
    U_NEWVEC(vm, regs + 4, 1165, 1);
    *(keepvar + 1) = *(regs + 1);
    U_BCALLRET2(vm, regs + 2, 8, 1); // append
    *(keepvar + 2) = *(regs + 0);
    *(regs + 1) = *(locals + 3); // f2
    (regs + 1)->LTINCRTNIL();
    U_PUSHVARF(vm, regs + 2, 1519); // lt
    fun_588(vm, regs + 3); // call: qsort
    *(keepvar + 3) = *(regs + 1);
    U_BCALLRET2(vm, regs + 2, 8, 1); // append
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[2]);
    DecVal(vm, locals[3]);
    psp = PopArg(vm, 1519, psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    goto block1;
    block0:;
    vm.last = {195, 5};
    *(regs + 0) = *(locals + 0); // xs
    (regs + 0)->LTINCRTNIL();
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[2]);
    DecVal(vm, locals[3]);
    psp = PopArg(vm, 1519, psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    block1:;
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[2]);
    DecVal(vm, locals[3]);
    psp = PopArg(vm, 1519, psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    RestoreBackup(vm, 1522);
    RestoreBackup(vm, 1521);
    RestoreBackup(vm, 1520);
    DecVal(vm, keepvar[0]);
    DecVal(vm, keepvar[1]);
    DecVal(vm, keepvar[2]);
    DecVal(vm, keepvar[3]);
    PopFunId(vm);
}

// qsort
static void fun_48(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value keepvar[4];
    Value locals[4];
    *(locals + 0) = *(psp - 2);
    SwapVars(vm, 53, psp, 1);
    BackupVar(vm, 54);
    *(locals + 1) = Value(0, lobster::V_NIL);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 755, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    *(keepvar + 1) = Value(0, lobster::V_NIL);
    *(keepvar + 2) = Value(0, lobster::V_NIL);
    *(keepvar + 3) = Value(0, lobster::V_NIL);
    vm.last = {194, 5};
    *(regs + 0) = *(locals + 0); // xs
    U_BCALLRET1(vm, regs + 1, 13, 1); // length
    *(regs + 1) = Value(1);
    U_IGT(vm, regs + 2);
    if (!(regs + 0)->True()) goto block0;
    vm.last = {196, 5};
    *(regs + 0) = *(locals + 0); // xs
    *(regs + 1) = Value(0);
    U_VPUSHIDXI(vm, regs + 2);
    U_LVAL_VARF(vm, regs + 1, 54); // pivot
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {197, 5};
    *(regs + 0) = *(locals + 0); // xs
    *(regs + 1) = Value(1);
    *(regs + 2) = Value(-1);
    U_BCALLRET3(vm, regs + 3, 31, 1); // slice
    vm.temp_lval = locals + 1; // tail
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {198, 5};
    *(regs + 0) = *(locals + 1); // tail
    (regs + 0)->LTINCRTNIL();
    U_PUSHFUN(vm, regs + 1, 0, fun_49);
    fun_13(vm, regs + 2); // call: partition
    vm.temp_lval = locals + 3; // f2
    U_LV_WRITEREF(vm, regs + 2);
    vm.temp_lval = locals + 2; // f1
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {200, 5};
    *(regs + 0) = *(locals + 2); // f1
    (regs + 0)->LTINCRTNIL();
    U_PUSHVARF(vm, regs + 1, 53); // lt
    fun_48(vm, regs + 2); // call: qsort
    *(keepvar + 0) = *(regs + 0);
    U_PUSHVARF(vm, regs + 1, 54); // pivot
    U_NEWVEC(vm, regs + 2, 7, 1);
    *(keepvar + 1) = *(regs + 1);
    U_BCALLRET2(vm, regs + 2, 8, 1); // append
    *(keepvar + 2) = *(regs + 0);
    *(regs + 1) = *(locals + 3); // f2
    (regs + 1)->LTINCRTNIL();
    U_PUSHVARF(vm, regs + 2, 53); // lt
    fun_48(vm, regs + 3); // call: qsort
    *(keepvar + 3) = *(regs + 1);
    U_BCALLRET2(vm, regs + 2, 8, 1); // append
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[2]);
    DecVal(vm, locals[3]);
    psp = PopArg(vm, 53, psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    goto block1;
    block0:;
    vm.last = {195, 5};
    *(regs + 0) = *(locals + 0); // xs
    (regs + 0)->LTINCRTNIL();
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[2]);
    DecVal(vm, locals[3]);
    psp = PopArg(vm, 53, psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    block1:;
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[2]);
    DecVal(vm, locals[3]);
    psp = PopArg(vm, 53, psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    RestoreBackup(vm, 54);
    DecVal(vm, keepvar[0]);
    DecVal(vm, keepvar[1]);
    DecVal(vm, keepvar[2]);
    DecVal(vm, keepvar[3]);
    PopFunId(vm);
}

// function49
static void fun_596(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 764, locals);
    vm.last = {198, 5};
    *(regs + 0) = *(locals + 0); // _
    U_PUSHVARF(vm, regs + 1, 1550); // pivot
    fun_362(vm, regs + 2); // call: function329
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function49
static void fun_591(VMRef vm, StackPtr psp) {
    Value regs[6];
    Value locals[3];
    *(locals + 0) = *(psp - 3);
    *(locals + 1) = *(psp - 2);
    *(locals + 2) = *(psp - 1);
    PushFunId(vm, funinfo_table + 768, locals);
    vm.last = {198, 5};
    *(regs + 0) = *(locals + 0); // _.0
    *(regs + 1) = *(locals + 1); // _.1
    *(regs + 2) = *(locals + 2); // _.2
    U_PUSHVARVF(vm, regs + 3, 1520, 3); // pivot+0
    fun_359(vm, regs + 6); // call: function326
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function49
static void fun_49(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 774, locals);
    vm.last = {198, 5};
    *(regs + 0) = *(locals + 0); // _
    U_PUSHVARF(vm, regs + 1, 54); // pivot
    fun_164(vm, regs + 2); // call: function157
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// qsort_in_place
static void fun_50(VMRef vm, StackPtr psp) {
    Value regs[2];
    SwapVars(vm, 59, psp, 2);
    SwapVars(vm, 60, psp, 1);
    PushFunId(vm, funinfo_table + 778, 0);
    vm.last = {203, 5};
    vm.last = {220, 5};
    *(regs + 0) = Value(0);
    U_PUSHVARF(vm, regs + 1, 59); // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    fun_51(vm, regs + 2); // call: rec
    psp = PopArg(vm, 60, psp);
    psp = PopArg(vm, 59, psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// rec
static void fun_51(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[7];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    *(locals + 5) = Value(0, lobster::V_NIL);
    *(locals + 6) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 783, locals);
    vm.last = {204, 5};
    *(regs + 0) = *(locals + 1); // e
    *(regs + 1) = *(locals + 0); // s
    U_ISUB(vm, regs + 2);
    vm.temp_lval = locals + 2; // l
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {205, 5};
    *(regs + 0) = *(locals + 2); // l
    *(regs + 1) = Value(1);
    U_IGT(vm, regs + 2);
    if (!(regs + 0)->True()) goto block0;
    vm.last = {206, 5};
    U_PUSHVARF(vm, regs + 0, 59); // xs
    *(regs + 1) = *(locals + 0); // s
    U_VPUSHIDXI(vm, regs + 2);
    vm.temp_lval = locals + 3; // pivot
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {207, 5};
    *(regs + 0) = *(locals + 0); // s
    *(regs + 1) = Value(1);
    U_IADD(vm, regs + 2);
    vm.temp_lval = locals + 4; // sp
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {208, 5};
    *(regs + 0) = *(locals + 1); // e
    vm.temp_lval = locals + 5; // ep
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {209, 5};
    block1:;
    *(regs + 0) = *(locals + 4); // sp
    *(regs + 1) = *(locals + 5); // ep
    U_ILT(vm, regs + 2);
    if (!(regs + 0)->True()) goto block2;
    vm.last = {210, 5};
    U_PUSHVARF(vm, regs + 0, 59); // xs
    *(regs + 1) = *(locals + 4); // sp
    U_VPUSHIDXI(vm, regs + 2);
    vm.temp_lval = locals + 6; // c
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {211, 5};
    *(regs + 0) = *(locals + 6); // c
    *(regs + 1) = *(locals + 3); // pivot
    fun_166(vm, regs + 2); // call: function159
    if (!(regs + 0)->True()) goto block3;
    vm.last = {212, 5};
    U_PUSHVARF(vm, regs + 0, 59); // xs
    *(regs + 1) = *(locals + 4); // sp
    U_VPUSHIDXI(vm, regs + 2);
    U_PUSHVARF(vm, regs + 1, 59); // xs
    *(regs + 2) = *(locals + 4); // sp
    *(regs + 3) = Value(1);
    U_ISUB(vm, regs + 4);
    U_LVAL_IDXVI(vm, regs + 3, 0);
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {213, 5};
    vm.temp_lval = locals + 4; // sp
    U_LV_IPP(vm, regs + 0);
    goto block4;
    block3:;
    vm.last = {215, 5};
    U_PUSHVARF(vm, regs + 0, 59); // xs
    vm.temp_lval = locals + 5; // ep
    U_LV_IMM(vm, regs + 1);
    U_LV_DUP(vm, regs + 1);
    U_VPUSHIDXI(vm, regs + 2);
    U_PUSHVARF(vm, regs + 1, 59); // xs
    *(regs + 2) = *(locals + 4); // sp
    U_LVAL_IDXVI(vm, regs + 3, 0);
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {216, 5};
    *(regs + 0) = *(locals + 6); // c
    U_PUSHVARF(vm, regs + 1, 59); // xs
    *(regs + 2) = *(locals + 5); // ep
    U_LVAL_IDXVI(vm, regs + 3, 0);
    *(vm.temp_lval) = *(regs + 0);
    block4:;
    goto block1;
    block2:;
    vm.last = {217, 5};
    *(regs + 0) = *(locals + 3); // pivot
    U_PUSHVARF(vm, regs + 1, 59); // xs
    vm.temp_lval = locals + 4; // sp
    U_LV_IMM(vm, regs + 2);
    U_LV_DUP(vm, regs + 2);
    U_LVAL_IDXVI(vm, regs + 3, 0);
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {218, 5};
    *(regs + 0) = *(locals + 0); // s
    *(regs + 1) = *(locals + 4); // sp
    fun_51(vm, regs + 2); // call: rec
    vm.last = {219, 5};
    *(regs + 0) = *(locals + 5); // ep
    *(regs + 1) = *(locals + 1); // e
    fun_51(vm, regs + 2); // call: rec
    block0:;
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// insertion_sort
static void fun_52(VMRef vm, StackPtr psp) {
    Value regs[6];
    Value locals[5];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 793, locals);
    vm.last = {223, 5};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // xs
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {223, 5};
    U_VFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 2; // key
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {223, 5};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 3; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {224, 5};
    *(regs + 2) = *(locals + 3); // i
    if (!(regs + 2)->True()) goto block2;
    vm.last = {225, 5};
    *(regs + 2) = *(locals + 3); // i
    vm.temp_lval = locals + 4; // j
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {226, 5};
    block3:;
    *(regs + 2) = *(locals + 4); // j
    *(regs + 3) = Value(0);
    U_IGT(vm, regs + 4);
    if (!(regs + 2)->True()) goto block4;
    U_POP(vm, regs + 3);
    *(regs + 2) = *(locals + 2); // key
    *(regs + 3) = *(locals + 0); // xs
    *(regs + 4) = *(locals + 4); // j
    *(regs + 5) = Value(1);
    U_ISUB(vm, regs + 6);
    U_VPUSHIDXI(vm, regs + 5);
    fun_165(vm, regs + 4); // call: function158
    block4:;
    if (!(regs + 2)->True()) goto block5;
    vm.last = {227, 5};
    *(regs + 2) = *(locals + 0); // xs
    *(regs + 3) = *(locals + 4); // j
    *(regs + 4) = Value(1);
    U_ISUB(vm, regs + 5);
    U_VPUSHIDXI(vm, regs + 4);
    *(regs + 3) = *(locals + 0); // xs
    vm.temp_lval = locals + 4; // j
    U_LV_DUP(vm, regs + 4);
    U_LV_IMM(vm, regs + 5);
    U_LVAL_IDXVI(vm, regs + 5, 0);
    *(vm.temp_lval) = *(regs + 2);
    goto block3;
    block5:;
    vm.last = {228, 5};
    *(regs + 2) = *(locals + 2); // key
    *(regs + 3) = *(locals + 0); // xs
    *(regs + 4) = *(locals + 4); // j
    U_LVAL_IDXVI(vm, regs + 5, 0);
    *(vm.temp_lval) = *(regs + 2);
    block2:;
    goto block0;
    block1:;
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// do
static void fun_476(VMRef vm, StackPtr psp) {
    Value regs[1];
    (void)regs;
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 801, locals);
    vm.last = {253, 5};
    fun_178(vm, regs + 0); // call: function171
    if (vm.ret_unwind_to == 171) goto block0;
    Pop(psp);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block0:;
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// xz
static void fun_70(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[4];
    *(locals + 0) = *(psp - 4);
    *(locals + 1) = *(psp - 3);
    *(locals + 2) = *(psp - 2);
    *(locals + 3) = *(psp - 1);
    PushFunId(vm, funinfo_table + 805, locals);
    vm.last = {64, 4};
    *(regs + 0) = *(locals + 0); // v+0
    *(regs + 1) = *(locals + 2); // v+2
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// yw
static void fun_71(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[4];
    *(locals + 0) = *(psp - 4);
    *(locals + 1) = *(psp - 3);
    *(locals + 2) = *(psp - 2);
    *(locals + 3) = *(psp - 1);
    PushFunId(vm, funinfo_table + 812, locals);
    vm.last = {65, 4};
    *(regs + 0) = *(locals + 1); // v+1
    *(regs + 1) = *(locals + 3); // v+3
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// zw
static void fun_72(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[4];
    *(locals + 0) = *(psp - 4);
    *(locals + 1) = *(psp - 3);
    *(locals + 2) = *(psp - 2);
    *(locals + 3) = *(psp - 1);
    PushFunId(vm, funinfo_table + 819, locals);
    vm.last = {66, 4};
    *(regs + 0) = *(locals + 2); // v+2
    *(regs + 1) = *(locals + 3); // v+3
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// vec4
static void fun_79(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[4];
    *(locals + 0) = *(psp - 4);
    *(locals + 1) = *(psp - 3);
    *(locals + 2) = *(psp - 2);
    *(locals + 3) = *(psp - 1);
    PushFunId(vm, funinfo_table + 826, locals);
    vm.last = {76, 4};
    *(regs + 0) = *(locals + 0); // v+0
    *(regs + 1) = *(locals + 1); // v+1
    *(regs + 2) = *(locals + 2); // v+2
    *(regs + 3) = *(locals + 3); // w
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    *(psp++) = *(regs + 2);
    *(psp++) = *(regs + 3);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// rnd_float2
static void fun_89(VMRef vm, StackPtr psp) {
    Value regs[2];
    PushFunId(vm, funinfo_table + 833, 0);
    vm.last = {92, 4};
    U_BCALLRET0(vm, regs + 0, 95, 1); // rnd_float
    U_BCALLRET0(vm, regs + 1, 95, 1); // rnd_float
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// rnd_int2
static void fun_95(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 836, locals);
    vm.last = {100, 4};
    *(regs + 0) = *(locals + 0); // n+0
    U_BCALLRET1(vm, regs + 1, 93, 1); // rnd
    *(regs + 1) = *(locals + 1); // n+1
    U_BCALLRET1(vm, regs + 2, 93, 1); // rnd
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// forxy
static void fun_98(VMRef vm, StackPtr psp) {
    Value regs[6];
    Value locals[5];
    *(locals + 0) = *(psp - 3);
    *(locals + 1) = *(psp - 2);
    *(locals + 2) = *(psp - 1);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 841, locals);
    vm.last = {104, 4};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 1); // v+1
    block0:;
    if (!U_IFOR(vm, regs + 2)) goto block1;
    vm.last = {104, 4};
    U_IFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 3; // y
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {104, 4};
    *(regs + 2) = Value(-1);
    *(regs + 3) = *(locals + 0); // v+0
    block2:;
    if (!U_IFOR(vm, regs + 4)) goto block3;
    vm.last = {104, 4};
    U_IFORELEM(vm, regs + 4);
    vm.temp_lval = locals + 4; // x
    *(vm.temp_lval) = *(regs + 4);
    vm.last = {104, 4};
    *(regs + 4) = *(locals + 4); // x
    *(regs + 5) = *(locals + 3); // y
    fun_358(vm, regs + 6); // call: function325
    goto block2;
    block3:;
    goto block0;
    block1:;
    Pop(psp);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// mapxy
static void fun_574(VMRef vm, StackPtr psp) {
    Value regs[2];
    SwapVars(vm, 1469, psp, 3);
    SwapVars(vm, 1470, psp, 2);
    SwapVars(vm, 1471, psp, 1);
    PushFunId(vm, funinfo_table + 849, 0);
    vm.last = {106, 4};
    U_PUSHVARF(vm, regs + 0, 1470); // v+1
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_576(vm, regs + 2); // call: map
    psp = PopArg(vm, 1471, psp);
    psp = PopArg(vm, 1470, psp);
    psp = PopArg(vm, 1469, psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// mapxy
static void fun_100(VMRef vm, StackPtr psp) {
    Value regs[2];
    SwapVars(vm, 197, psp, 3);
    SwapVars(vm, 198, psp, 2);
    SwapVars(vm, 199, psp, 1);
    PushFunId(vm, funinfo_table + 855, 0);
    vm.last = {106, 4};
    U_PUSHVARF(vm, regs + 0, 198); // v+1
    U_PUSHFUN(vm, regs + 1, 0, fun_101);
    fun_563(vm, regs + 2); // call: map
    psp = PopArg(vm, 199, psp);
    psp = PopArg(vm, 198, psp);
    psp = PopArg(vm, 197, psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function98
static void fun_577(VMRef vm, StackPtr psp) {
    Value regs[2];
    SwapVars(vm, 1477, psp, 1);
    PushFunId(vm, funinfo_table + 861, 0);
    vm.last = {106, 4};
    U_PUSHVARF(vm, regs + 0, 1469); // v+0
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_579(vm, regs + 2); // call: map
    psp = PopArg(vm, 1477, psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function98
static void fun_101(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 865, locals);
    vm.last = {106, 4};
    U_PUSHVARF(vm, regs + 0, 197); // v+0
    U_PUSHFUN(vm, regs + 1, 0, fun_102);
    fun_564(vm, regs + 2); // call: map
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function99
static void fun_580(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 869, locals);
    vm.last = {106, 4};
    *(regs + 0) = *(locals + 0); // x
    U_PUSHVARF(vm, regs + 1, 1477); // y
    fun_335(vm, regs + 2); // call: function302
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function99
static void fun_102(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 873, locals);
    vm.last = {106, 4};
    vm.last = {10, 13};
    *(regs + 0) = Value(-1);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function107
static void fun_110(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[10];
    *(locals + 0) = Value(0, lobster::V_NIL);
    *(locals + 1) = Value(0, lobster::V_NIL);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    *(locals + 5) = Value(0, lobster::V_NIL);
    *(locals + 6) = Value(0, lobster::V_NIL);
    *(locals + 7) = Value(0, lobster::V_NIL);
    *(locals + 8) = Value(0, lobster::V_NIL);
    *(locals + 9) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 877, locals);
    vm.last = {7, 3};
    vm.last = {7, 3};
    U_PUSHFUN(vm, regs + 0, 0, fun_111);
    vm.temp_lval = locals + 0; // f
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {253, 5};
    fun_111(vm, regs + 0); // call: function108
    vm.last = {67, 3};
    vm.last = {67, 3};
    U_PUSHFUN(vm, regs + 0, 0, fun_112);
    vm.temp_lval = locals + 1; // f
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {253, 5};
    fun_112(vm, regs + 0); // call: function109
    vm.last = {97, 3};
    vm.last = {97, 3};
    U_PUSHFUN(vm, regs + 0, 0, fun_113);
    vm.temp_lval = locals + 2; // f
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {253, 5};
    fun_113(vm, regs + 0); // call: function110
    vm.last = {108, 3};
    vm.last = {108, 3};
    U_PUSHFUN(vm, regs + 0, 0, fun_114);
    vm.temp_lval = locals + 3; // f
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {253, 5};
    fun_114(vm, regs + 0); // call: function111
    vm.last = {117, 3};
    vm.last = {117, 3};
    U_PUSHFUN(vm, regs + 0, 0, fun_117);
    vm.temp_lval = locals + 4; // f
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {253, 5};
    fun_117(vm, regs + 0); // call: function114
    vm.last = {128, 3};
    vm.last = {128, 3};
    U_PUSHFUN(vm, regs + 0, 0, fun_120);
    vm.temp_lval = locals + 5; // f
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {253, 5};
    fun_120(vm, regs + 0); // call: function116
    vm.last = {151, 3};
    vm.last = {151, 3};
    U_PUSHFUN(vm, regs + 0, 0, fun_123);
    vm.temp_lval = locals + 6; // f
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {253, 5};
    fun_123(vm, regs + 0); // call: function119
    vm.last = {162, 3};
    vm.last = {162, 3};
    U_PUSHFUN(vm, regs + 0, 0, fun_125);
    vm.temp_lval = locals + 7; // f
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {253, 5};
    fun_125(vm, regs + 0); // call: function121
    vm.last = {183, 3};
    vm.last = {183, 3};
    U_PUSHFUN(vm, regs + 0, 0, fun_126);
    vm.temp_lval = locals + 8; // f
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {253, 5};
    fun_126(vm, regs + 0); // call: function122
    vm.last = {272, 3};
    vm.last = {272, 3};
    U_PUSHFUN(vm, regs + 0, 0, fun_127);
    vm.temp_lval = locals + 9; // f
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {253, 5};
    fun_127(vm, regs + 0); // call: function123
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function108
static void fun_111(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value locals[14];
    *(locals + 0) = Value(0, lobster::V_NIL);
    *(locals + 1) = Value(0, lobster::V_NIL);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    *(locals + 5) = Value(0, lobster::V_NIL);
    *(locals + 6) = Value(0, lobster::V_NIL);
    *(locals + 7) = Value(0, lobster::V_NIL);
    *(locals + 8) = Value(0, lobster::V_NIL);
    *(locals + 9) = Value(0, lobster::V_NIL);
    *(locals + 10) = Value(0, lobster::V_NIL);
    *(locals + 11) = Value(0, lobster::V_NIL);
    *(locals + 12) = Value(0, lobster::V_NIL);
    *(locals + 13) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 890, locals);
    vm.last = {8, 3};
    vm.last = {8, 3};
    vm.last = {11, 3};
    vm.last = {11, 3};
    vm.last = {14, 3};
    vm.last = {14, 3};
    vm.last = {17, 3};
    *(regs + 0) = Value(3);
    *(regs + 1) = Value(4);
    *(regs + 2) = Value(1);
    *(regs + 3) = Value(1);
    U_IVVADD(vm, regs + 4, 2);
    *(regs + 2) = Value(1);
    *(regs + 3) = Value(1);
    U_IVVSUB(vm, regs + 4, 2);
    vm.temp_lval = locals + 0; // a+0
    U_LV_WRITEV(vm, regs + 2, 2);
    vm.last = {18, 3};
    *(regs + 0) = Value(5);
    *(regs + 1) = Value(6);
    *(regs + 2) = *(locals + 0); // a+0
    *(regs + 3) = *(locals + 1); // a+1
    U_IADD(vm, regs + 4);
    vm.temp_lval = locals + 2; // b+0
    U_LV_WRITEV(vm, regs + 3, 3);
    vm.last = {19, 3};
    *(regs + 0) = *(locals + 2); // b+0
    *(regs + 1) = *(locals + 3); // b+1
    U_IADD(vm, regs + 2);
    *(regs + 1) = *(locals + 4); // b+2
    U_IADD(vm, regs + 2);
    *(regs + 1) = Value(18);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 19, 3, 27);
    vm.last = {20, 3};
    *(regs + 0) = *(locals + 0); // a.0
    *(regs + 1) = *(locals + 1); // a.1
    *(regs + 2) = *(locals + 0); // a.0
    *(regs + 3) = *(locals + 1); // a.1
    U_IVVADD(vm, regs + 4, 2);
    vm.temp_lval = locals + 5; // aa+0
    U_LV_WRITEV(vm, regs + 2, 2);
    vm.last = {21, 3};
    *(regs + 0) = *(locals + 5); // aa+0
    *(regs + 1) = *(locals + 6); // aa+1
    U_IADD(vm, regs + 2);
    *(regs + 1) = Value(14);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 21, 3, 28);
    vm.last = {22, 3};
    *(regs + 0) = *(locals + 5); // aa.0
    *(regs + 1) = *(locals + 6); // aa.1
    *(regs + 2) = Value(1);
    U_IVSADD(vm, regs + 3, 2);
    *(regs + 2) = Value(7);
    *(regs + 3) = Value(9);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 22, 3, 29);
    vm.last = {23, 3};
    *(regs + 0) = *(locals + 0); // a.0
    *(regs + 1) = *(locals + 1); // a.1
    vm.temp_lval = locals + 0; // a+0
    U_LV_IVVADD(vm, regs + 2, 2);
    vm.last = {24, 3};
    *(regs + 0) = *(locals + 0); // a.0
    *(regs + 1) = *(locals + 1); // a.1
    *(regs + 2) = Value(6);
    *(regs + 3) = Value(8);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 24, 3, 30);
    vm.last = {25, 3};
    *(regs + 0) = Value(1);
    vm.temp_lval = locals + 5; // aa+0
    U_LV_IVSADD(vm, regs + 1, 2);
    vm.last = {26, 3};
    *(regs + 0) = *(locals + 5); // aa.0
    *(regs + 1) = *(locals + 6); // aa.1
    *(regs + 2) = Value(7);
    *(regs + 3) = Value(9);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 26, 3, 31);
    vm.last = {27, 3};
    *(regs + 0) = Value(10);
    *(regs + 1) = Value(11);
    *(regs + 2) = Value(12);
    U_NEWOBJECT(vm, regs + 3, 291); // C
    vm.temp_lval = locals + 7; // c
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {28, 3};
    *(regs + 0) = *(locals + 7); // c
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = *(locals + 7); // c
    U_PUSHFLD(vm, regs + 2, 1);
    U_IADD(vm, regs + 2);
    *(regs + 1) = *(locals + 7); // c
    U_PUSHFLD(vm, regs + 2, 2);
    U_IADD(vm, regs + 2);
    *(regs + 1) = Value(33);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 28, 3, 32);
    vm.last = {29, 3};
    *(regs + 0) = Value(1);
    *(regs + 1) = *(locals + 7); // c
    U_LVAL_FLD(vm, regs + 2, 0);
    U_LV_IVSADD(vm, regs + 1, 2);
    vm.last = {30, 3};
    *(regs + 0) = *(locals + 7); // c
    U_PUSHFLD2V(vm, regs + 1, 0, 2);
    *(regs + 2) = Value(11);
    *(regs + 3) = Value(12);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 30, 3, 33);
    vm.last = {31, 3};
    *(regs + 0) = Value(1);
    *(regs + 1) = Value(2);
    *(regs + 2) = *(locals + 7); // c
    U_LVAL_FLD(vm, regs + 3, 0);
    U_LV_IVVADD(vm, regs + 2, 2);
    vm.last = {32, 3};
    *(regs + 0) = *(locals + 7); // c
    U_PUSHFLD2V(vm, regs + 1, 0, 2);
    *(regs + 2) = Value(12);
    *(regs + 3) = Value(14);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 32, 3, 34);
    vm.last = {33, 3};
    *(regs + 0) = Value(3);
    *(regs + 1) = Value(4);
    *(regs + 2) = *(locals + 7); // c
    U_LVAL_FLD(vm, regs + 3, 0);
    U_LV_WRITEV(vm, regs + 2, 2);
    vm.last = {34, 3};
    *(regs + 0) = *(locals + 7); // c
    U_PUSHFLD2V(vm, regs + 1, 0, 2);
    *(regs + 2) = Value(3);
    *(regs + 3) = Value(4);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 34, 3, 35);
    vm.last = {35, 3};
    *(regs + 0) = Value(1.000000);
    *(regs + 1) = Value(2.000000);
    *(regs + 2) = Value(2);
    U_BCALLRETV(vm, regs + 3, 62, 1); // int
    *(regs + 2) = Value(1);
    *(regs + 3) = Value(2);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 35, 3, 36);
    vm.last = {36, 3};
    *(regs + 0) = Value(1);
    *(regs + 1) = Value(2);
    *(regs + 2) = Value(2);
    U_BCALLRETV(vm, regs + 3, 124, 1); // min
    *(regs + 1) = Value(1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 36, 3, 37);
    vm.last = {37, 3};
    *(regs + 0) = Value(2);
    *(regs + 1) = Value(3);
    *(regs + 2) = Value(2);
    U_BCALLRETV(vm, regs + 3, 89, 1); // manhattan
    *(regs + 1) = Value(5);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 37, 3, 38);
    vm.last = {38, 3};
    U_PUSHVARVF(vm, regs + 0, 74, 2); // float2_0+0
    vm.temp_lval = locals + 8; // nv+0
    U_LV_WRITEV(vm, regs + 2, 2);
    vm.last = {39, 3};
    *(regs + 0) = Value(123.000000);
    *(regs + 1) = Value(456.000000);
    *(regs + 2) = Value(2);
    U_BCALLRETV(vm, regs + 3, 84, 1); // normalize
    vm.temp_lval = locals + 8; // nv+0
    U_LV_WRITEV(vm, regs + 2, 2);
    vm.last = {40, 3};
    *(regs + 0) = *(locals + 8); // nv.0
    *(regs + 1) = *(locals + 9); // nv.1
    *(regs + 2) = Value(2);
    U_BCALLRETV(vm, regs + 3, 86, 1); // magnitude
    *(regs + 1) = Value(1.000000);
    U_FSUB(vm, regs + 2);
    U_BCALLRET1(vm, regs + 1, 113, 1); // abs
    U_PUSHFLT64(vm, regs + 1, -755914244, 1062232653); // 0.001
    U_FLT(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 40, 3, 39);
    vm.last = {41, 3};
    U_PUSHVARVF(vm, regs + 0, 74, 2); // float2_0+0
    *(regs + 2) = Value(2);
    U_BCALLRETV(vm, regs + 3, 84, 1); // normalize
    vm.temp_lval = locals + 8; // nv+0
    U_LV_WRITEV(vm, regs + 2, 2);
    vm.last = {42, 3};
    *(regs + 0) = *(locals + 8); // nv.0
    *(regs + 1) = *(locals + 9); // nv.1
    *(regs + 2) = Value(2);
    U_BCALLRETV(vm, regs + 3, 86, 1); // magnitude
    U_BCALLRET1(vm, regs + 1, 113, 1); // abs
    U_PUSHFLT64(vm, regs + 1, -755914244, 1062232653); // 0.001
    U_FLT(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 42, 3, 40);
    vm.last = {43, 3};
    *(regs + 0) = Value(1);
    *(regs + 1) = Value(2);
    *(regs + 2) = Value(3);
    *(regs + 3) = Value(4);
    U_NEWVEC(vm, regs + 4, 37, 2);
    vm.temp_lval = locals + 10; // v
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {44, 3};
    *(regs + 0) = *(locals + 10); // v
    *(regs + 1) = Value(1);
    U_VPUSHIDXI2V(vm, regs + 2);
    *(regs + 2) = Value(3);
    *(regs + 3) = Value(4);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 44, 3, 41);
    vm.last = {45, 3};
    *(regs + 0) = Value(5);
    *(regs + 1) = Value(6);
    *(regs + 2) = *(locals + 10); // v
    *(regs + 3) = Value(1);
    U_LVAL_IDXVI(vm, regs + 4, 0);
    U_LV_WRITEV(vm, regs + 2, 2);
    vm.last = {46, 3};
    *(regs + 0) = *(locals + 10); // v
    *(regs + 1) = Value(1);
    U_VPUSHIDXI2V(vm, regs + 2);
    *(regs + 2) = Value(5);
    *(regs + 3) = Value(6);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 46, 3, 42);
    vm.last = {47, 3};
    *(regs + 0) = *(locals + 10); // v
    *(regs + 1) = Value(1);
    U_VPUSHIDXIS2V(vm, regs + 2, 1, 1);
    *(regs + 1) = Value(6);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 47, 3, 43);
    vm.last = {48, 3};
    *(regs + 0) = Value(1);
    *(regs + 1) = Value(1);
    *(regs + 2) = *(locals + 10); // v
    *(regs + 3) = Value(1);
    U_LVAL_IDXVI(vm, regs + 4, 0);
    U_LV_IVVADD(vm, regs + 2, 2);
    vm.last = {49, 3};
    *(regs + 0) = *(locals + 10); // v
    *(regs + 1) = Value(1);
    U_VPUSHIDXI2V(vm, regs + 2);
    *(regs + 2) = Value(6);
    *(regs + 3) = Value(7);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 49, 3, 44);
    vm.last = {50, 3};
    *(regs + 0) = Value(1);
    *(regs + 1) = *(locals + 10); // v
    *(regs + 2) = Value(1);
    U_LVAL_IDXVI(vm, regs + 3, 0);
    U_LV_IVSADD(vm, regs + 1, 2);
    vm.last = {51, 3};
    *(regs + 0) = *(locals + 10); // v
    *(regs + 1) = Value(1);
    U_VPUSHIDXI2V(vm, regs + 2);
    *(regs + 2) = Value(7);
    *(regs + 3) = Value(8);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 51, 3, 45);
    vm.last = {52, 3};
    *(regs + 0) = Value(123);
    U_NEWVEC(vm, regs + 1, 7, 1);
    U_NEWVEC(vm, regs + 1, 13, 1);
    U_NEWVEC(vm, regs + 1, 306, 1);
    vm.temp_lval = locals + 11; // tri
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {53, 3};
    *(regs + 0) = *(locals + 11); // tri
    *(regs + 1) = Value(0);
    *(regs + 2) = Value(0);
    *(regs + 3) = Value(0);
    U_VPUSHIDXV(vm, regs + 4, 3);
    *(regs + 1) = Value(123);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 53, 3, 46);
    vm.last = {54, 3};
    *(regs + 0) = Value(124);
    *(regs + 1) = *(locals + 11); // tri
    *(regs + 2) = Value(0);
    *(regs + 3) = Value(0);
    *(regs + 4) = Value(0);
    U_LVAL_IDXVV(vm, regs + 5, 0, 3);
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {55, 3};
    *(regs + 0) = *(locals + 11); // tri
    *(regs + 1) = Value(0);
    *(regs + 2) = Value(0);
    *(regs + 3) = Value(0);
    U_VPUSHIDXV(vm, regs + 4, 3);
    *(regs + 1) = Value(124);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 55, 3, 47);
    vm.last = {56, 3};
    *(regs + 0) = Value(10);
    *(regs + 1) = Value(11);
    U_NEWVEC(vm, regs + 2, 37, 1);
    U_NEWVEC(vm, regs + 1, 308, 1);
    U_NEWVEC(vm, regs + 1, 310, 1);
    vm.temp_lval = locals + 12; // triv
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {57, 3};
    *(regs + 0) = *(locals + 12); // triv
    *(regs + 1) = Value(0);
    *(regs + 2) = Value(0);
    *(regs + 3) = Value(0);
    U_VPUSHIDXV(vm, regs + 4, 3);
    *(regs + 2) = Value(10);
    *(regs + 3) = Value(11);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 57, 3, 48);
    vm.last = {58, 3};
    *(regs + 0) = Value(4);
    *(regs + 1) = *(locals + 12); // triv
    *(regs + 2) = Value(0);
    *(regs + 3) = Value(0);
    *(regs + 4) = Value(0);
    U_LVAL_IDXVV(vm, regs + 5, 0, 3);
    U_LV_IVSADD(vm, regs + 1, 2);
    vm.last = {59, 3};
    *(regs + 0) = *(locals + 12); // triv
    *(regs + 1) = Value(0);
    *(regs + 2) = Value(0);
    *(regs + 3) = Value(0);
    U_VPUSHIDXV(vm, regs + 4, 3);
    *(regs + 2) = Value(14);
    *(regs + 3) = Value(15);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 59, 3, 49);
    vm.last = {60, 3};
    *(regs + 0) = Value(5);
    *(regs + 1) = Value(6);
    *(regs + 2) = Value(7);
    U_NEWVEC(vm, regs + 3, 312, 1);
    U_NEWVEC(vm, regs + 1, 314, 1);
    U_NEWVEC(vm, regs + 1, 316, 1);
    vm.temp_lval = locals + 13; // trivv
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {61, 3};
    *(regs + 0) = *(locals + 13); // trivv
    *(regs + 1) = Value(0);
    *(regs + 2) = Value(0);
    *(regs + 3) = Value(0);
    U_VPUSHIDXVS(vm, regs + 4, 3, 2, 0);
    *(regs + 2) = Value(5);
    *(regs + 3) = Value(6);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 61, 3, 50);
    vm.last = {62, 3};
    *(regs + 0) = *(locals + 13); // trivv
    *(regs + 1) = Value(0);
    *(regs + 2) = Value(0);
    *(regs + 3) = Value(0);
    U_VPUSHIDXVS(vm, regs + 4, 3, 1, 1);
    *(regs + 1) = Value(6);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 62, 3, 51);
    vm.last = {63, 3};
    *(regs + 0) = *(locals + 13); // trivv
    *(regs + 1) = Value(0);
    *(regs + 2) = Value(0);
    U_VPUSHIDXV(vm, regs + 3, 2);
    *(regs + 1) = Value(0);
    U_VPUSHIDXIS2V(vm, regs + 2, 1, 1);
    *(regs + 1) = Value(6);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 63, 3, 52);
    vm.last = {64, 3};
    *(regs + 0) = Value(5);
    *(regs + 1) = Value(6);
    *(regs + 2) = Value(7);
    *(regs + 3) = Value(1);
    U_NPUSHIDXI(vm, regs + 4, 3);
    *(regs + 1) = Value(6);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 64, 3, 53);
    DecVal(vm, locals[7]);
    DecVal(vm, locals[10]);
    DecVal(vm, locals[11]);
    DecVal(vm, locals[12]);
    DecVal(vm, locals[13]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function109
static void fun_112(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value locals[11];
    *(locals + 0) = Value(0, lobster::V_NIL);
    *(locals + 1) = Value(0, lobster::V_NIL);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    *(locals + 5) = Value(0, lobster::V_NIL);
    *(locals + 6) = Value(0, lobster::V_NIL);
    *(locals + 7) = Value(0, lobster::V_NIL);
    *(locals + 8) = Value(0, lobster::V_NIL);
    *(locals + 9) = Value(0, lobster::V_NIL);
    *(locals + 10) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 907, locals);
    vm.last = {69, 3};
    U_PUSHVARVF(vm, regs + 0, 123, 2); // int2_0+0
    vm.temp_lval = locals + 0; // a+0
    U_LV_WRITEV(vm, regs + 2, 2);
    vm.last = {70, 3};
    *(regs + 0) = *(locals + 0); // a.0
    *(regs + 1) = *(locals + 1); // a.1
    vm.temp_lval = locals + 2; // b+0
    U_LV_WRITEV(vm, regs + 2, 2);
    vm.last = {71, 3};
    *(regs + 0) = Value(1);
    vm.temp_lval = locals + 1; // a+1
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {72, 3};
    *(regs + 0) = *(locals + 0); // a.0
    *(regs + 1) = *(locals + 1); // a.1
    U_PUSHVARVF(vm, regs + 2, 129, 2); // int2_y+0
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 72, 3, 54);
    vm.last = {73, 3};
    *(regs + 0) = *(locals + 2); // b.0
    *(regs + 1) = *(locals + 3); // b.1
    U_PUSHVARVF(vm, regs + 2, 123, 2); // int2_0+0
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 73, 3, 55);
    vm.last = {75, 3};
    vm.last = {75, 3};
    vm.last = {78, 3};
    U_PUSHVARVF(vm, regs + 0, 134, 3); // int3_1+0
    U_PUSHVARVF(vm, regs + 3, 123, 2); // int2_0+0
    U_NEWOBJECT(vm, regs + 5, 318); // C
    vm.temp_lval = locals + 4; // c
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {79, 3};
    *(regs + 0) = Value(1);
    *(regs + 1) = *(locals + 4); // c
    U_LVAL_FLD(vm, regs + 2, 4);
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {80, 3};
    *(regs + 0) = *(locals + 4); // c
    U_PUSHFLD2V(vm, regs + 1, 3, 2);
    U_PUSHVARVF(vm, regs + 2, 129, 2); // int2_y+0
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 80, 3, 56);
    vm.last = {82, 3};
    vm.last = {82, 3};
    vm.last = {85, 3};
    U_PUSHVARVF(vm, regs + 0, 134, 3); // int3_1+0
    U_PUSHVARVF(vm, regs + 3, 123, 2); // int2_0+0
    vm.temp_lval = locals + 5; // s+0
    U_LV_WRITEREFV(vm, regs + 5, 5, 0);
    vm.last = {86, 3};
    *(regs + 0) = Value(1);
    vm.temp_lval = locals + 9; // s+4
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {87, 3};
    *(regs + 0) = *(locals + 8); // s.0
    *(regs + 1) = *(locals + 9); // s.1
    U_PUSHVARVF(vm, regs + 2, 129, 2); // int2_y+0
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 87, 3, 57);
    vm.last = {89, 3};
    U_PUSHVARVF(vm, regs + 0, 127, 2); // int2_x+0
    vm.temp_lval = locals + 8; // s+3
    U_LV_WRITEV(vm, regs + 2, 2);
    vm.last = {90, 3};
    *(regs + 0) = *(locals + 8); // s.0
    *(regs + 1) = *(locals + 9); // s.1
    U_PUSHVARVF(vm, regs + 2, 127, 2); // int2_x+0
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 90, 3, 58);
    vm.last = {92, 3};
    U_PUSHVARVF(vm, regs + 0, 123, 2); // int2_0+0
    U_NEWVEC(vm, regs + 2, 37, 1);
    vm.temp_lval = locals + 10; // v
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {93, 3};
    *(regs + 0) = Value(1);
    *(regs + 1) = *(locals + 10); // v
    *(regs + 2) = Value(0);
    U_LVAL_IDXVI(vm, regs + 3, 1);
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {94, 3};
    *(regs + 0) = *(locals + 10); // v
    *(regs + 1) = Value(0);
    U_VPUSHIDXI2V(vm, regs + 2);
    U_PUSHVARVF(vm, regs + 2, 129, 2); // int2_y+0
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 94, 3, 59);
    DecVal(vm, locals[4]);
    DecVal(vm, locals[10]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function110
static void fun_113(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[3];
    Value locals[4];
    *(locals + 0) = Value(0, lobster::V_NIL);
    *(locals + 1) = Value(0, lobster::V_NIL);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 921, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    *(keepvar + 1) = Value(0, lobster::V_NIL);
    *(keepvar + 2) = Value(0, lobster::V_NIL);
    vm.last = {98, 3};
    *(regs + 0) = Value(1);
    *(regs + 1) = Value(1);
    *(regs + 2) = Value(2);
    U_BCALLRETV(vm, regs + 3, 89, 1); // manhattan
    if (!(regs + 0)->True()) goto block0;
    vm.last = {98, 3};
    *(regs + 0) = Value(1);
    *(regs + 1) = Value(2);
    goto block1;
    block0:;
    vm.last = {98, 3};
    *(regs + 0) = Value(3);
    *(regs + 1) = Value(4);
    block1:;
    U_PUSHFLDV(vm, regs + 2, 1, 2);
    *(regs + 1) = Value(2);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 98, 3, 60);
    vm.last = {99, 3};
    vm.last = {99, 3};
    *(regs + 0) = Value(1);
    *(regs + 1) = Value(2);
    U_PUSHFLDV(vm, regs + 2, 1, 2);
    *(regs + 1) = Value(2);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 99, 3, 61);
    vm.last = {100, 3};
    *(regs + 0) = Value(1);
    *(regs + 1) = Value(2);
    *(regs + 2) = Value(1);
    *(regs + 3) = Value(1);
    *(regs + 4) = Value(2);
    U_BCALLRETV(vm, regs + 5, 89, 1); // manhattan
    U_DUP(vm, regs + 3);
    *(regs + 4) = Value(2);
    U_IEQ(vm, regs + 5);
    if (!(regs + 3)->True()) goto block2;
    U_POP(vm, regs + 3);
    vm.last = {101, 3};
    *(regs + 2) = Value(1);
    *(regs + 3) = Value(2);
    goto block3;
    block2:;
    U_POP(vm, regs + 3);
    vm.last = {102, 3};
    *(regs + 2) = Value(3);
    *(regs + 3) = Value(4);
    block3:;
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 102, 3, 62);
    vm.last = {103, 3};
    *(regs + 0) = Value(-1);
    *(regs + 1) = Value(1);
    *(regs + 2) = Value(2);
    *(regs + 3) = Value(0);
    *(regs + 4) = Value(3);
    U_NEWVEC(vm, regs + 5, 37, 2);
    *(keepvar + 0) = *(regs + 1);
    block4:;
    if (!U_VFOR(vm, regs + 2)) goto block5;
    vm.last = {103, 3};
    U_VFORELEM2S(vm, regs + 2);
    vm.temp_lval = locals + 0; // st+0
    U_LV_WRITEV(vm, regs + 4, 2);
    vm.last = {103, 3};
    *(regs + 2) = *(locals + 0); // st+0
    *(regs + 3) = *(locals + 1); // st+1
    U_IADD(vm, regs + 4);
    *(regs + 3) = Value(3);
    U_IEQ(vm, regs + 4);
    U_ASSERT(vm, regs + 3, 103, 3, 63);
    goto block4;
    block5:;
    vm.last = {104, 3};
    vm.last = {105, 3};
    *(regs + 0) = Value(-1);
    U_PUSHSTR(vm, regs + 1, 64); // "a"
    (regs + 1)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 2, 65); // "bb"
    (regs + 2)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 3, 66); // "ab"
    (regs + 3)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 4, 67); // "b"
    (regs + 4)->LTINCRTNIL();
    U_NEWVEC(vm, regs + 5, 1695, 2);
    *(keepvar + 1) = *(regs + 1);
    block6:;
    if (!U_VFOR(vm, regs + 2)) goto block7;
    vm.last = {105, 3};
    U_VFORELEMREF2S(vm, regs + 2, 3);
    vm.temp_lval = locals + 2; // st+0
    U_LV_WRITEREFV(vm, regs + 4, 2, 3);
    vm.last = {105, 3};
    *(regs + 2) = *(locals + 2); // st+0
    *(regs + 3) = *(locals + 3); // st+1
    U_SADD(vm, regs + 4);
    DecVal(vm, keepvar[2]);
    *(keepvar + 2) = *(regs + 2);
    U_PUSHSTR(vm, regs + 3, 68); // "abb"
    U_SEQ(vm, regs + 4);
    U_ASSERT(vm, regs + 3, 105, 3, 69);
    goto block6;
    block7:;
    DecVal(vm, locals[2]);
    DecVal(vm, locals[3]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    DecVal(vm, keepvar[1]);
    DecVal(vm, keepvar[2]);
    PopFunId(vm);
}

// function111
static void fun_114(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[4];
    *(locals + 0) = Value(0, lobster::V_NIL);
    *(locals + 1) = Value(0, lobster::V_NIL);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 928, locals);
    vm.last = {109, 3};
    vm.last = {111, 3};
    vm.last = {112, 3};
    *(regs + 0) = Value(1);
    *(regs + 1) = Value(2);
    *(regs + 2) = Value(3);
    *(regs + 3) = Value(4);
    fun_115(vm, regs + 4); // call: f
    *(regs + 1) = Value(10);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 112, 3, 70);
    vm.last = {113, 3};
    *(regs + 0) = Value(7);
    fun_116(vm, regs + 1); // call: g
    vm.temp_lval = locals + 3; // mr3
    *(vm.temp_lval) = *(regs + 3);
    vm.temp_lval = locals + 1; // mr2+0
    U_LV_WRITEV(vm, regs + 3, 2);
    vm.temp_lval = locals + 0; // mr1
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {114, 3};
    *(regs + 0) = *(locals + 1); // mr2.0
    *(regs + 1) = *(locals + 2); // mr2.1
    *(regs + 2) = Value(7);
    *(regs + 3) = Value(7);
    U_STEQ(vm, regs + 4, 2);
    if (!(regs + 0)->True()) goto block0;
    U_POP(vm, regs + 1);
    *(regs + 0) = *(locals + 0); // mr1
    *(regs + 1) = Value(1);
    U_IEQ(vm, regs + 2);
    block0:;
    if (!(regs + 0)->True()) goto block1;
    U_POP(vm, regs + 1);
    *(regs + 0) = *(locals + 3); // mr3
    *(regs + 1) = Value(1);
    U_IEQ(vm, regs + 2);
    block1:;
    U_ASSERT(vm, regs + 1, 114, 3, 71);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// f
static void fun_115(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[4];
    *(locals + 0) = *(psp - 4);
    *(locals + 1) = *(psp - 3);
    *(locals + 2) = *(psp - 2);
    *(locals + 3) = *(psp - 1);
    PushFunId(vm, funinfo_table + 935, locals);
    vm.last = {110, 3};
    *(regs + 0) = *(locals + 0); // a
    *(regs + 1) = *(locals + 1); // b+0
    U_IADD(vm, regs + 2);
    *(regs + 1) = *(locals + 2); // b+1
    U_IADD(vm, regs + 2);
    *(regs + 1) = *(locals + 3); // c
    U_IADD(vm, regs + 2);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// g
static void fun_116(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 942, locals);
    vm.last = {111, 3};
    *(regs + 0) = Value(1);
    *(regs + 1) = *(locals + 0); // a
    *(regs + 2) = *(locals + 0); // a
    *(regs + 3) = Value(1);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    *(psp++) = *(regs + 2);
    *(psp++) = *(regs + 3);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function114
static void fun_117(VMRef vm, StackPtr psp) {
    Value regs[3];
    PushFunId(vm, funinfo_table + 946, 0);
    vm.last = {118, 3};
    vm.last = {118, 3};
    vm.last = {120, 3};
    vm.last = {120, 3};
    vm.last = {121, 3};
    vm.last = {122, 3};
    vm.last = {123, 3};
    *(regs + 0) = Value(1);
    U_NEWOBJECT(vm, regs + 1, 372); // mm1
    *(regs + 1) = Value(4);
    *(regs + 2) = Value(5);
    U_DDCALL(vm, regs + 3, 0, 2); vm.next_call_target(vm, regs + 3);
    *(regs + 1) = Value(10);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 123, 3, 72);
    vm.last = {124, 3};
    *(regs + 0) = Value(0);
    U_NEWOBJECT(vm, regs + 1, 381); // mm2
    *(regs + 1) = Value(4);
    *(regs + 2) = Value(5);
    fun_119(vm, regs + 3); // call: mm
    *(regs + 1) = Value(9);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 124, 3, 73);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// mm
static void fun_118(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[3];
    *(locals + 0) = *(psp - 3);
    *(locals + 1) = *(psp - 2);
    *(locals + 2) = *(psp - 1);
    PushFunId(vm, funinfo_table + 949, locals);
    vm.last = {121, 3};
    *(regs + 0) = *(locals + 0); // a
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = *(locals + 1); // b+0
    U_IADD(vm, regs + 2);
    *(regs + 1) = *(locals + 2); // b+1
    U_IADD(vm, regs + 2);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// mm
static void fun_119(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[3];
    *(locals + 0) = *(psp - 3);
    *(locals + 1) = *(psp - 2);
    *(locals + 2) = *(psp - 1);
    PushFunId(vm, funinfo_table + 955, locals);
    vm.last = {122, 3};
    *(regs + 0) = *(locals + 0); // a
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = *(locals + 1); // b+0
    U_IADD(vm, regs + 2);
    *(regs + 1) = *(locals + 2); // b+1
    U_IADD(vm, regs + 2);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function116
static void fun_120(VMRef vm, StackPtr psp) {
    Value regs[6];
    Value locals[12];
    *(locals + 0) = Value(0, lobster::V_NIL);
    *(locals + 1) = Value(0, lobster::V_NIL);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    *(locals + 5) = Value(0, lobster::V_NIL);
    *(locals + 6) = Value(0, lobster::V_NIL);
    *(locals + 7) = Value(0, lobster::V_NIL);
    *(locals + 8) = Value(0, lobster::V_NIL);
    *(locals + 9) = Value(0, lobster::V_NIL);
    *(locals + 10) = Value(0, lobster::V_NIL);
    *(locals + 11) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 961, locals);
    vm.last = {129, 3};
    vm.last = {129, 3};
    vm.last = {132, 3};
    vm.last = {132, 3};
    vm.last = {135, 3};
    U_PUSHSTR(vm, regs + 0, 74); // "hello"
    (regs + 0)->LTINCRTNIL();
    *(regs + 1) = Value(123);
    U_NEWVEC(vm, regs + 2, 7, 1);
    vm.temp_lval = locals + 0; // r+0
    U_LV_WRITEREFV(vm, regs + 2, 2, 3);
    vm.last = {136, 3};
    vm.last = {137, 3};
    *(regs + 0) = *(locals + 0); // r.0
    *(regs + 1) = *(locals + 1); // r.1
    (regs + 0)->LTINCRTNIL();
    (regs + 1)->LTINCRTNIL();
    fun_121(vm, regs + 2); // call: f
    vm.temp_lval = locals + 2; // fr+0
    U_LV_WRITEREFV(vm, regs + 2, 2, 3);
    vm.last = {138, 3};
    *(regs + 0) = *(locals + 2); // fr+0
    U_PUSHSTR(vm, regs + 1, 75); // "hellohello"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 138, 3, 76);
    vm.last = {139, 3};
    U_PUSHSTR(vm, regs + 0, 77); // "one"
    (regs + 0)->LTINCRTNIL();
    *(regs + 1) = Value(1);
    U_NEWVEC(vm, regs + 2, 7, 1);
    U_PUSHSTR(vm, regs + 2, 78); // "two"
    (regs + 2)->LTINCRTNIL();
    *(regs + 3) = Value(2);
    U_NEWVEC(vm, regs + 4, 7, 1);
    U_NEWVEC(vm, regs + 4, 402, 2);
    vm.temp_lval = locals + 4; // rs
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {140, 3};
    *(regs + 0) = *(locals + 4); // rs
    *(regs + 1) = Value(1);
    U_VPUSHIDXI2V(vm, regs + 2);
    (regs + 0)->LTINCRTNIL();
    (regs + 1)->LTINCRTNIL();
    fun_121(vm, regs + 2); // call: f
    vm.temp_lval = locals + 2; // fr+0
    U_LV_WRITEREFV(vm, regs + 2, 2, 3);
    vm.last = {141, 3};
    *(regs + 0) = *(locals + 2); // fr+0
    U_PUSHSTR(vm, regs + 1, 79); // "twotwo"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 141, 3, 80);
    vm.last = {142, 3};
    U_PUSHSTR(vm, regs + 0, 81); // "A"
    (regs + 0)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 1, 82); // "B"
    (regs + 1)->LTINCRTNIL();
    *(regs + 2) = Value(64);
    U_NEWVEC(vm, regs + 3, 7, 1);
    vm.temp_lval = locals + 5; // rn+0
    U_LV_WRITEREFV(vm, regs + 3, 3, 7);
    vm.last = {143, 3};
    vm.last = {144, 3};
    *(regs + 0) = *(locals + 5); // rn.0
    *(regs + 1) = *(locals + 6); // rn.1
    *(regs + 2) = *(locals + 7); // rn.2
    (regs + 0)->LTINCRTNIL();
    (regs + 1)->LTINCRTNIL();
    (regs + 2)->LTINCRTNIL();
    fun_122(vm, regs + 3); // call: fun
    vm.temp_lval = locals + 8; // frn+0
    U_LV_WRITEREFV(vm, regs + 3, 3, 7);
    vm.last = {145, 3};
    *(regs + 0) = *(locals + 8); // frn+0
    U_PUSHSTR(vm, regs + 1, 83); // "AA"
    U_SEQ(vm, regs + 2);
    if (!(regs + 0)->True()) goto block0;
    U_POP(vm, regs + 1);
    *(regs + 0) = *(locals + 10); // frn+2
    *(regs + 1) = Value(1);
    U_VPUSHIDXI(vm, regs + 2);
    *(regs + 1) = Value(64);
    U_IEQ(vm, regs + 2);
    block0:;
    U_ASSERT(vm, regs + 1, 145, 3, 84);
    vm.last = {146, 3};
    U_PUSHSTR(vm, regs + 0, 85); // "AA"
    (regs + 0)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 1, 86); // "one"
    (regs + 1)->LTINCRTNIL();
    *(regs + 2) = Value(1);
    U_NEWVEC(vm, regs + 3, 7, 1);
    U_PUSHSTR(vm, regs + 3, 87); // "BB"
    (regs + 3)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 4, 88); // "two"
    (regs + 4)->LTINCRTNIL();
    *(regs + 5) = Value(2);
    U_NEWVEC(vm, regs + 6, 7, 1);
    U_NEWVEC(vm, regs + 6, 419, 2);
    vm.temp_lval = locals + 11; // rns
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {147, 3};
    *(regs + 0) = *(locals + 11); // rns
    *(regs + 1) = Value(1);
    U_VPUSHIDXI2V(vm, regs + 2);
    (regs + 0)->LTINCRTNIL();
    (regs + 1)->LTINCRTNIL();
    (regs + 2)->LTINCRTNIL();
    fun_122(vm, regs + 3); // call: fun
    vm.temp_lval = locals + 8; // frn+0
    U_LV_WRITEREFV(vm, regs + 3, 3, 7);
    vm.last = {148, 3};
    *(regs + 0) = *(locals + 8); // frn+0
    U_PUSHSTR(vm, regs + 1, 89); // "BBBB"
    U_SEQ(vm, regs + 2);
    if (!(regs + 0)->True()) goto block1;
    U_POP(vm, regs + 1);
    *(regs + 0) = *(locals + 10); // frn+2
    *(regs + 1) = Value(1);
    U_VPUSHIDXI(vm, regs + 2);
    *(regs + 1) = Value(2);
    U_IEQ(vm, regs + 2);
    block1:;
    U_ASSERT(vm, regs + 1, 148, 3, 90);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[2]);
    DecVal(vm, locals[3]);
    DecVal(vm, locals[4]);
    DecVal(vm, locals[5]);
    DecVal(vm, locals[6]);
    DecVal(vm, locals[7]);
    DecVal(vm, locals[8]);
    DecVal(vm, locals[9]);
    DecVal(vm, locals[10]);
    DecVal(vm, locals[11]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// f
static void fun_121(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 976, locals);
    vm.last = {136, 3};
    *(regs + 0) = *(locals + 0); // x+0
    *(regs + 1) = *(locals + 0); // x+0
    U_SADD(vm, regs + 2);
    *(regs + 1) = *(locals + 1); // x+1
    *(regs + 2) = *(locals + 1); // x+1
    U_BCALLRET2(vm, regs + 3, 8, 1); // append
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// fun
static void fun_122(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[3];
    *(locals + 0) = *(psp - 3);
    *(locals + 1) = *(psp - 2);
    *(locals + 2) = *(psp - 1);
    PushFunId(vm, funinfo_table + 981, locals);
    vm.last = {143, 3};
    *(regs + 0) = *(locals + 0); // x+0
    *(regs + 1) = *(locals + 0); // x+0
    U_SADD(vm, regs + 2);
    *(regs + 1) = *(locals + 1); // x+1
    *(regs + 2) = *(locals + 1); // x+1
    U_SADD(vm, regs + 3);
    *(regs + 2) = *(locals + 2); // x+2
    *(regs + 3) = *(locals + 2); // x+2
    U_BCALLRET2(vm, regs + 4, 8, 1); // append
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[2]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    *(psp++) = *(regs + 2);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function119
static void fun_123(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value keepvar[1];
    Value locals[2];
    *(locals + 0) = Value(0, lobster::V_NIL);
    *(locals + 1) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 987, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {152, 3};
    vm.last = {152, 3};
    vm.last = {155, 3};
    U_PUSHSTR(vm, regs + 0, 91); // "hi"
    (regs + 0)->LTINCRTNIL();
    *(regs + 1) = Value(1);
    vm.temp_lval = locals + 0; // rs+0
    U_LV_WRITEREFV(vm, regs + 2, 2, 1);
    vm.last = {156, 3};
    vm.last = {157, 3};
    *(regs + 0) = *(locals + 0); // rs.0
    *(regs + 1) = *(locals + 1); // rs.1
    (regs + 0)->LTINCRTNIL();
    fun_124(vm, regs + 2); // call: frs
    vm.temp_lval = locals + 0; // rs+0
    U_LV_WRITEREFV(vm, regs + 2, 2, 1);
    vm.last = {158, 3};
    *(regs + 0) = *(locals + 0); // rs+0
    U_PUSHSTR(vm, regs + 1, 92); // "hihi"
    U_SEQ(vm, regs + 2);
    if (!(regs + 0)->True()) goto block0;
    U_POP(vm, regs + 1);
    *(regs + 0) = *(locals + 1); // rs+1
    *(regs + 1) = Value(2);
    U_IEQ(vm, regs + 2);
    block0:;
    U_ASSERT(vm, regs + 1, 158, 3, 93);
    vm.last = {159, 3};
    *(regs + 0) = *(locals + 0); // rs.0
    *(regs + 1) = *(locals + 1); // rs.1
    U_ST2S(vm, regs + 2, 421); // RS
    *(keepvar + 0) = *(regs + 0);
    U_PUSHSTR(vm, regs + 1, 94); // "RS{\"hihi\", 2}"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 159, 3, 95);
    DecVal(vm, locals[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// frs
static void fun_124(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 992, locals);
    vm.last = {156, 3};
    *(regs + 0) = *(locals + 0); // x+0
    *(regs + 1) = *(locals + 0); // x+0
    U_SADD(vm, regs + 2);
    *(regs + 1) = *(locals + 1); // x+1
    *(regs + 2) = *(locals + 1); // x+1
    U_IADD(vm, regs + 3);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function121
static void fun_125(VMRef vm, StackPtr psp) {
    Value regs[18];
    Value locals[10];
    *(locals + 0) = Value(0, lobster::V_NIL);
    *(locals + 1) = Value(0, lobster::V_NIL);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    *(locals + 5) = Value(0, lobster::V_NIL);
    *(locals + 6) = Value(0, lobster::V_NIL);
    *(locals + 7) = Value(0, lobster::V_NIL);
    *(locals + 8) = Value(0, lobster::V_NIL);
    *(locals + 9) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 997, locals);
    vm.last = {163, 3};
    vm.last = {163, 3};
    vm.last = {165, 3};
    *(regs + 0) = Value(1);
    *(regs + 1) = Value(1);
    U_IVVADD(vm, regs + 2, 1);
    *(regs + 1) = Value(2);
    U_STEQ(vm, regs + 2, 1);
    U_ASSERT(vm, regs + 1, 165, 3, 96);
    vm.last = {166, 3};
    *(regs + 0) = Value(2);
    *(regs + 1) = Value(1);
    *(regs + 2) = Value(1);
    *(regs + 3) = Value(1);
    *(regs + 4) = Value(-1);
    *(regs + 5) = Value(1);
    U_BCALLRETV(vm, regs + 6, 130, 1); // max
    *(regs + 3) = Value(1);
    U_BCALLRETV(vm, regs + 4, 122, 1); // min
    *(regs + 1) = Value(1);
    U_STEQ(vm, regs + 2, 1);
    U_ASSERT(vm, regs + 1, 166, 3, 97);
    vm.last = {167, 3};
    *(regs + 0) = Value(2);
    *(regs + 1) = Value(1);
    U_BCALLRETV(vm, regs + 2, 89, 1); // manhattan
    *(regs + 1) = Value(2);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 167, 3, 98);
    vm.last = {169, 3};
    vm.last = {169, 3};
    vm.last = {175, 3};
    *(regs + 0) = Value(1.000000);
    *(regs + 1) = Value(2.000000);
    *(regs + 2) = Value(3.000000);
    *(regs + 3) = Value(4.000000);
    *(regs + 4) = Value(5.000000);
    vm.temp_lval = locals + 0; // v5test+0
    U_LV_WRITEV(vm, regs + 5, 5);
    vm.last = {176, 3};
    *(regs + 0) = *(locals + 0); // v5test.0
    *(regs + 1) = *(locals + 1); // v5test.1
    *(regs + 2) = *(locals + 2); // v5test.2
    *(regs + 3) = *(locals + 3); // v5test.3
    *(regs + 4) = *(locals + 4); // v5test.4
    *(regs + 5) = *(locals + 0); // v5test.0
    *(regs + 6) = *(locals + 1); // v5test.1
    *(regs + 7) = *(locals + 2); // v5test.2
    *(regs + 8) = *(locals + 3); // v5test.3
    *(regs + 9) = *(locals + 4); // v5test.4
    U_FVVADD(vm, regs + 10, 5);
    vm.temp_lval = locals + 5; // v5test2+0
    U_LV_WRITEV(vm, regs + 5, 5);
    vm.last = {177, 3};
    *(regs + 0) = *(locals + 5); // v5test2.0
    *(regs + 1) = *(locals + 6); // v5test2.1
    *(regs + 2) = *(locals + 7); // v5test2.2
    *(regs + 3) = *(locals + 8); // v5test2.3
    *(regs + 4) = *(locals + 9); // v5test2.4
    *(regs + 5) = *(locals + 0); // v5test.0
    *(regs + 6) = *(locals + 1); // v5test.1
    *(regs + 7) = *(locals + 2); // v5test.2
    *(regs + 8) = *(locals + 3); // v5test.3
    *(regs + 9) = *(locals + 4); // v5test.4
    *(regs + 10) = Value(2.000000);
    U_FVSMUL(vm, regs + 11, 5);
    U_STEQ(vm, regs + 10, 5);
    U_ASSERT(vm, regs + 1, 177, 3, 99);
    vm.last = {178, 3};
    *(regs + 0) = *(locals + 5); // v5test2.0
    *(regs + 1) = *(locals + 6); // v5test2.1
    *(regs + 2) = *(locals + 7); // v5test2.2
    *(regs + 3) = *(locals + 8); // v5test2.3
    *(regs + 4) = *(locals + 9); // v5test2.4
    *(regs + 5) = Value(5);
    U_BCALLRETV(vm, regs + 6, 84, 1); // normalize
    *(regs + 5) = Value(5);
    U_BCALLRETV(vm, regs + 6, 86, 1); // magnitude
    *(regs + 1) = Value(1.000000);
    U_FEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 178, 3, 100);
    vm.last = {179, 3};
    *(regs + 0) = *(locals + 0); // v5test.0
    *(regs + 1) = *(locals + 1); // v5test.1
    *(regs + 2) = *(locals + 2); // v5test.2
    *(regs + 3) = *(locals + 3); // v5test.3
    *(regs + 4) = *(locals + 4); // v5test.4
    *(regs + 5) = Value(5);
    *(regs + 6) = *(locals + 0); // v5test.0
    *(regs + 7) = *(locals + 1); // v5test.1
    *(regs + 8) = *(locals + 2); // v5test.2
    *(regs + 9) = *(locals + 3); // v5test.3
    *(regs + 10) = *(locals + 4); // v5test.4
    *(regs + 11) = Value(5);
    *(regs + 12) = *(locals + 5); // v5test2.0
    *(regs + 13) = *(locals + 6); // v5test2.1
    *(regs + 14) = *(locals + 7); // v5test2.2
    *(regs + 15) = *(locals + 8); // v5test2.3
    *(regs + 16) = *(locals + 9); // v5test2.4
    *(regs + 17) = Value(5);
    U_BCALLRETV(vm, regs + 18, 131, 1); // max
    *(regs + 11) = Value(5);
    U_BCALLRETV(vm, regs + 12, 123, 1); // min
    *(regs + 5) = *(locals + 0); // v5test.0
    *(regs + 6) = *(locals + 1); // v5test.1
    *(regs + 7) = *(locals + 2); // v5test.2
    *(regs + 8) = *(locals + 3); // v5test.3
    *(regs + 9) = *(locals + 4); // v5test.4
    U_STEQ(vm, regs + 10, 5);
    U_ASSERT(vm, regs + 1, 179, 3, 101);
    vm.last = {180, 3};
    *(regs + 0) = *(locals + 0); // v5test.0
    *(regs + 1) = *(locals + 1); // v5test.1
    *(regs + 2) = *(locals + 2); // v5test.2
    *(regs + 3) = *(locals + 3); // v5test.3
    *(regs + 4) = *(locals + 4); // v5test.4
    *(regs + 5) = Value(5);
    *(regs + 6) = *(locals + 5); // v5test2.0
    *(regs + 7) = *(locals + 6); // v5test2.1
    *(regs + 8) = *(locals + 7); // v5test2.2
    *(regs + 9) = *(locals + 8); // v5test2.3
    *(regs + 10) = *(locals + 9); // v5test2.4
    *(regs + 11) = Value(5);
    U_BCALLRETV(vm, regs + 12, 85, 1); // dot
    *(regs + 1) = Value(110.000000);
    U_FEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 180, 3, 102);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function122
static void fun_126(VMRef vm, StackPtr psp) {
    Value regs[10];
    PushFunId(vm, funinfo_table + 1010, 0);
    vm.last = {187, 3};
    *(regs + 0) = Value(2);
    *(regs + 1) = Value(3);
    *(regs + 2) = Value(5);
    U_IVSMUL(vm, regs + 3, 2);
    *(regs + 2) = Value(10);
    *(regs + 3) = Value(15);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 187, 3, 103);
    vm.last = {188, 3};
    *(regs + 0) = Value(5);
    *(regs + 1) = Value(2);
    *(regs + 2) = Value(3);
    U_SIVMUL(vm, regs + 3, 2);
    *(regs + 2) = Value(10);
    *(regs + 3) = Value(15);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 188, 3, 104);
    vm.last = {189, 3};
    *(regs + 0) = Value(2);
    *(regs + 1) = Value(3);
    *(regs + 2) = Value(4);
    *(regs + 3) = Value(5);
    U_IVSMUL(vm, regs + 4, 3);
    *(regs + 3) = Value(10);
    *(regs + 4) = Value(15);
    *(regs + 5) = Value(20);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 189, 3, 105);
    vm.last = {190, 3};
    *(regs + 0) = Value(5);
    *(regs + 1) = Value(2);
    *(regs + 2) = Value(3);
    *(regs + 3) = Value(4);
    U_SIVMUL(vm, regs + 4, 3);
    *(regs + 3) = Value(10);
    *(regs + 4) = Value(15);
    *(regs + 5) = Value(20);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 190, 3, 106);
    vm.last = {193, 3};
    U_PUSHFLT64(vm, regs + 0, -858993459, 1073794252); // 2.1
    U_PUSHFLT64(vm, regs + 1, -1717986918, 1074370969); // 3.2
    *(regs + 2) = Value(5.000000);
    U_FVSMUL(vm, regs + 3, 2);
    *(regs + 2) = Value(10.500000);
    *(regs + 3) = Value(16.000000);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 193, 3, 107);
    vm.last = {194, 3};
    *(regs + 0) = Value(5.000000);
    U_PUSHFLT64(vm, regs + 1, -858993459, 1073794252); // 2.1
    U_PUSHFLT64(vm, regs + 2, -1717986918, 1074370969); // 3.2
    U_SFVMUL(vm, regs + 3, 2);
    *(regs + 2) = Value(10.500000);
    *(regs + 3) = Value(16.000000);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 194, 3, 108);
    vm.last = {197, 3};
    vm.last = {197, 3};
    vm.last = {204, 3};
    *(regs + 0) = Value(2);
    *(regs + 1) = Value(3);
    *(regs + 2) = Value(4);
    *(regs + 3) = Value(5);
    *(regs + 4) = Value(6);
    *(regs + 5) = Value(5);
    U_IVSMUL(vm, regs + 6, 5);
    *(regs + 5) = Value(10);
    *(regs + 6) = Value(15);
    *(regs + 7) = Value(20);
    *(regs + 8) = Value(25);
    *(regs + 9) = Value(30);
    U_STEQ(vm, regs + 10, 5);
    U_ASSERT(vm, regs + 1, 204, 3, 109);
    vm.last = {205, 3};
    *(regs + 0) = Value(5);
    *(regs + 1) = Value(2);
    *(regs + 2) = Value(3);
    *(regs + 3) = Value(4);
    *(regs + 4) = Value(5);
    *(regs + 5) = Value(6);
    U_SIVMUL(vm, regs + 6, 5);
    *(regs + 5) = Value(10);
    *(regs + 6) = Value(15);
    *(regs + 7) = Value(20);
    *(regs + 8) = Value(25);
    *(regs + 9) = Value(30);
    U_STEQ(vm, regs + 10, 5);
    U_ASSERT(vm, regs + 1, 205, 3, 110);
    vm.last = {208, 3};
    *(regs + 0) = Value(5);
    *(regs + 1) = Value(2);
    *(regs + 2) = Value(3);
    U_SIVADD(vm, regs + 3, 2);
    *(regs + 2) = Value(7);
    *(regs + 3) = Value(8);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 208, 3, 111);
    vm.last = {209, 3};
    *(regs + 0) = Value(2);
    *(regs + 1) = Value(3);
    *(regs + 2) = Value(5);
    U_IVSADD(vm, regs + 3, 2);
    *(regs + 2) = Value(7);
    *(regs + 3) = Value(8);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 209, 3, 112);
    vm.last = {210, 3};
    *(regs + 0) = Value(2);
    *(regs + 1) = Value(3);
    *(regs + 2) = Value(4);
    *(regs + 3) = Value(5);
    U_IVSADD(vm, regs + 4, 3);
    *(regs + 3) = Value(7);
    *(regs + 4) = Value(8);
    *(regs + 5) = Value(9);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 210, 3, 113);
    vm.last = {211, 3};
    *(regs + 0) = Value(5);
    *(regs + 1) = Value(2);
    *(regs + 2) = Value(3);
    *(regs + 3) = Value(4);
    U_SIVADD(vm, regs + 4, 3);
    *(regs + 3) = Value(7);
    *(regs + 4) = Value(8);
    *(regs + 5) = Value(9);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 211, 3, 114);
    vm.last = {216, 3};
    *(regs + 0) = Value(5);
    *(regs + 1) = Value(2);
    *(regs + 2) = Value(3);
    U_SIVSUB(vm, regs + 3, 2);
    *(regs + 2) = Value(3);
    *(regs + 3) = Value(2);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 216, 3, 115);
    vm.last = {217, 3};
    *(regs + 0) = Value(2);
    *(regs + 1) = Value(3);
    *(regs + 2) = Value(5);
    U_IVSSUB(vm, regs + 3, 2);
    *(regs + 2) = Value(-3);
    *(regs + 3) = Value(-2);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 217, 3, 116);
    vm.last = {218, 3};
    *(regs + 0) = Value(5);
    *(regs + 1) = Value(2);
    *(regs + 2) = Value(3);
    *(regs + 3) = Value(4);
    U_SIVSUB(vm, regs + 4, 3);
    *(regs + 3) = Value(3);
    *(regs + 4) = Value(2);
    *(regs + 5) = Value(1);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 218, 3, 117);
    vm.last = {219, 3};
    *(regs + 0) = Value(2);
    *(regs + 1) = Value(3);
    *(regs + 2) = Value(4);
    *(regs + 3) = Value(5);
    U_IVSSUB(vm, regs + 4, 3);
    *(regs + 3) = Value(-3);
    *(regs + 4) = Value(-2);
    *(regs + 5) = Value(-1);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 219, 3, 118);
    vm.last = {222, 3};
    *(regs + 0) = Value(10);
    *(regs + 1) = Value(5);
    *(regs + 2) = Value(10);
    U_SIVDIV(vm, regs + 3, 2);
    *(regs + 2) = Value(2);
    *(regs + 3) = Value(1);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 222, 3, 119);
    vm.last = {223, 3};
    *(regs + 0) = Value(10);
    *(regs + 1) = Value(20);
    *(regs + 2) = Value(5);
    U_IVSDIV(vm, regs + 3, 2);
    *(regs + 2) = Value(2);
    *(regs + 3) = Value(4);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 223, 3, 120);
    vm.last = {224, 3};
    *(regs + 0) = Value(10);
    *(regs + 1) = Value(5);
    *(regs + 2) = Value(10);
    *(regs + 3) = Value(1);
    U_SIVDIV(vm, regs + 4, 3);
    *(regs + 3) = Value(2);
    *(regs + 4) = Value(1);
    *(regs + 5) = Value(10);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 224, 3, 121);
    vm.last = {225, 3};
    *(regs + 0) = Value(10);
    *(regs + 1) = Value(20);
    *(regs + 2) = Value(25);
    *(regs + 3) = Value(5);
    U_IVSDIV(vm, regs + 4, 3);
    *(regs + 3) = Value(2);
    *(regs + 4) = Value(4);
    *(regs + 5) = Value(5);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 225, 3, 122);
    vm.last = {228, 3};
    *(regs + 0) = Value(10);
    *(regs + 1) = Value(2);
    *(regs + 2) = Value(3);
    U_SIVMOD(vm, regs + 3, 2);
    *(regs + 2) = Value(0);
    *(regs + 3) = Value(1);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 228, 3, 123);
    vm.last = {229, 3};
    *(regs + 0) = Value(10);
    *(regs + 1) = Value(21);
    *(regs + 2) = Value(4);
    U_IVSMOD(vm, regs + 3, 2);
    *(regs + 2) = Value(2);
    *(regs + 3) = Value(1);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 229, 3, 124);
    vm.last = {230, 3};
    *(regs + 0) = Value(10);
    *(regs + 1) = Value(2);
    *(regs + 2) = Value(3);
    *(regs + 3) = Value(4);
    U_SIVMOD(vm, regs + 4, 3);
    *(regs + 3) = Value(0);
    *(regs + 4) = Value(1);
    *(regs + 5) = Value(2);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 230, 3, 125);
    vm.last = {231, 3};
    *(regs + 0) = Value(10);
    *(regs + 1) = Value(21);
    *(regs + 2) = Value(24);
    *(regs + 3) = Value(4);
    U_IVSMOD(vm, regs + 4, 3);
    *(regs + 3) = Value(2);
    *(regs + 4) = Value(1);
    *(regs + 5) = Value(0);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 231, 3, 126);
    vm.last = {236, 3};
    *(regs + 0) = Value(2);
    *(regs + 1) = Value(5);
    *(regs + 2) = Value(5);
    U_IVSLT(vm, regs + 3, 2);
    *(regs + 2) = Value(1);
    *(regs + 3) = Value(0);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 236, 3, 127);
    vm.last = {237, 3};
    *(regs + 0) = Value(5);
    *(regs + 1) = Value(2);
    *(regs + 2) = Value(5);
    U_SIVLT(vm, regs + 3, 2);
    *(regs + 2) = Value(0);
    *(regs + 3) = Value(0);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 237, 3, 128);
    vm.last = {238, 3};
    *(regs + 0) = Value(2);
    *(regs + 1) = Value(6);
    *(regs + 2) = Value(5);
    U_IVSLT(vm, regs + 3, 2);
    *(regs + 2) = Value(1);
    *(regs + 3) = Value(0);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 238, 3, 129);
    vm.last = {239, 3};
    *(regs + 0) = Value(5);
    *(regs + 1) = Value(2);
    *(regs + 2) = Value(6);
    U_SIVLT(vm, regs + 3, 2);
    *(regs + 2) = Value(0);
    *(regs + 3) = Value(1);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 239, 3, 130);
    vm.last = {241, 3};
    *(regs + 0) = Value(2);
    *(regs + 1) = Value(5);
    *(regs + 2) = Value(6);
    *(regs + 3) = Value(5);
    U_IVSLT(vm, regs + 4, 3);
    *(regs + 3) = Value(1);
    *(regs + 4) = Value(0);
    *(regs + 5) = Value(0);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 241, 3, 131);
    vm.last = {242, 3};
    *(regs + 0) = Value(5);
    *(regs + 1) = Value(2);
    *(regs + 2) = Value(5);
    *(regs + 3) = Value(6);
    U_SIVLT(vm, regs + 4, 3);
    *(regs + 3) = Value(0);
    *(regs + 4) = Value(0);
    *(regs + 5) = Value(1);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 242, 3, 132);
    vm.last = {245, 3};
    *(regs + 0) = Value(2);
    *(regs + 1) = Value(5);
    *(regs + 2) = Value(5);
    U_IVSLE(vm, regs + 3, 2);
    *(regs + 2) = Value(1);
    *(regs + 3) = Value(1);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 245, 3, 133);
    vm.last = {246, 3};
    *(regs + 0) = Value(5);
    *(regs + 1) = Value(2);
    *(regs + 2) = Value(5);
    U_SIVLE(vm, regs + 3, 2);
    *(regs + 2) = Value(0);
    *(regs + 3) = Value(1);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 246, 3, 134);
    vm.last = {247, 3};
    *(regs + 0) = Value(2);
    *(regs + 1) = Value(6);
    *(regs + 2) = Value(5);
    U_IVSLE(vm, regs + 3, 2);
    *(regs + 2) = Value(1);
    *(regs + 3) = Value(0);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 247, 3, 135);
    vm.last = {248, 3};
    *(regs + 0) = Value(5);
    *(regs + 1) = Value(2);
    *(regs + 2) = Value(6);
    U_SIVLE(vm, regs + 3, 2);
    *(regs + 2) = Value(0);
    *(regs + 3) = Value(1);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 248, 3, 136);
    vm.last = {250, 3};
    *(regs + 0) = Value(2);
    *(regs + 1) = Value(5);
    *(regs + 2) = Value(6);
    *(regs + 3) = Value(5);
    U_IVSLE(vm, regs + 4, 3);
    *(regs + 3) = Value(1);
    *(regs + 4) = Value(1);
    *(regs + 5) = Value(0);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 250, 3, 137);
    vm.last = {251, 3};
    *(regs + 0) = Value(5);
    *(regs + 1) = Value(2);
    *(regs + 2) = Value(5);
    *(regs + 3) = Value(6);
    U_SIVLE(vm, regs + 4, 3);
    *(regs + 3) = Value(0);
    *(regs + 4) = Value(1);
    *(regs + 5) = Value(1);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 251, 3, 138);
    vm.last = {254, 3};
    *(regs + 0) = Value(2);
    *(regs + 1) = Value(5);
    *(regs + 2) = Value(5);
    U_IVSGT(vm, regs + 3, 2);
    *(regs + 2) = Value(0);
    *(regs + 3) = Value(0);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 254, 3, 139);
    vm.last = {255, 3};
    *(regs + 0) = Value(5);
    *(regs + 1) = Value(2);
    *(regs + 2) = Value(5);
    U_SIVGT(vm, regs + 3, 2);
    *(regs + 2) = Value(1);
    *(regs + 3) = Value(0);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 255, 3, 140);
    vm.last = {256, 3};
    *(regs + 0) = Value(2);
    *(regs + 1) = Value(6);
    *(regs + 2) = Value(5);
    U_IVSGT(vm, regs + 3, 2);
    *(regs + 2) = Value(0);
    *(regs + 3) = Value(1);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 256, 3, 141);
    vm.last = {257, 3};
    *(regs + 0) = Value(5);
    *(regs + 1) = Value(2);
    *(regs + 2) = Value(6);
    U_SIVGT(vm, regs + 3, 2);
    *(regs + 2) = Value(1);
    *(regs + 3) = Value(0);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 257, 3, 142);
    vm.last = {259, 3};
    *(regs + 0) = Value(2);
    *(regs + 1) = Value(5);
    *(regs + 2) = Value(6);
    *(regs + 3) = Value(5);
    U_IVSGT(vm, regs + 4, 3);
    *(regs + 3) = Value(0);
    *(regs + 4) = Value(0);
    *(regs + 5) = Value(1);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 259, 3, 143);
    vm.last = {260, 3};
    *(regs + 0) = Value(5);
    *(regs + 1) = Value(2);
    *(regs + 2) = Value(5);
    *(regs + 3) = Value(6);
    U_SIVGT(vm, regs + 4, 3);
    *(regs + 3) = Value(1);
    *(regs + 4) = Value(0);
    *(regs + 5) = Value(0);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 260, 3, 144);
    vm.last = {263, 3};
    *(regs + 0) = Value(2);
    *(regs + 1) = Value(5);
    *(regs + 2) = Value(5);
    U_IVSGE(vm, regs + 3, 2);
    *(regs + 2) = Value(0);
    *(regs + 3) = Value(1);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 263, 3, 145);
    vm.last = {264, 3};
    *(regs + 0) = Value(5);
    *(regs + 1) = Value(2);
    *(regs + 2) = Value(5);
    U_SIVGE(vm, regs + 3, 2);
    *(regs + 2) = Value(1);
    *(regs + 3) = Value(1);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 264, 3, 146);
    vm.last = {265, 3};
    *(regs + 0) = Value(2);
    *(regs + 1) = Value(6);
    *(regs + 2) = Value(5);
    U_IVSGE(vm, regs + 3, 2);
    *(regs + 2) = Value(0);
    *(regs + 3) = Value(1);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 265, 3, 147);
    vm.last = {266, 3};
    *(regs + 0) = Value(5);
    *(regs + 1) = Value(2);
    *(regs + 2) = Value(6);
    U_SIVGE(vm, regs + 3, 2);
    *(regs + 2) = Value(1);
    *(regs + 3) = Value(0);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 266, 3, 148);
    vm.last = {268, 3};
    *(regs + 0) = Value(2);
    *(regs + 1) = Value(5);
    *(regs + 2) = Value(6);
    *(regs + 3) = Value(5);
    U_IVSGE(vm, regs + 4, 3);
    *(regs + 3) = Value(0);
    *(regs + 4) = Value(1);
    *(regs + 5) = Value(1);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 268, 3, 149);
    vm.last = {269, 3};
    *(regs + 0) = Value(5);
    *(regs + 1) = Value(2);
    *(regs + 2) = Value(5);
    *(regs + 3) = Value(6);
    U_SIVGE(vm, regs + 4, 3);
    *(regs + 3) = Value(1);
    *(regs + 4) = Value(1);
    *(regs + 5) = Value(0);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 269, 3, 150);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function123
static void fun_127(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value keepvar[2];
    PushFunId(vm, funinfo_table + 1013, 0);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    *(keepvar + 1) = Value(0, lobster::V_NIL);
    vm.last = {273, 3};
    *(regs + 0) = Value(1);
    *(regs + 1) = Value(2);
    U_NEWVEC(vm, regs + 2, 37, 1);
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = Value(1);
    *(regs + 2) = Value(2);
    U_NEWVEC(vm, regs + 3, 37, 1);
    *(keepvar + 1) = *(regs + 1);
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 273, 3, 151);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    DecVal(vm, keepvar[1]);
    PopFunId(vm);
}

// try
static void fun_477(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[4];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1016, locals);
    vm.last = {4, 7};
    *(regs + 0) = *(locals + 0); // body
    fun_478(vm, regs + 1); // call: exception_handler
    vm.temp_lval = locals + 3; // stack_trace
    U_LV_WRITEREF(vm, regs + 2);
    vm.temp_lval = locals + 2; // err
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 7};
    *(regs + 0) = *(locals + 2); // err
    if (!(regs + 0)->True()) goto block0;
    vm.last = {6, 7};
    *(regs + 0) = *(locals + 2); // err
    fun_181(vm, regs + 1); // call: function174
    if (vm.ret_unwind_to == 174) goto block1;
    DecVal(vm, locals[2]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block1:;
    block0:;
    vm.ret_slots = 0;
    vm.ret_unwind_to = 124;
    DecVal(vm, locals[2]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// try
static void fun_128(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[5];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1023, locals);
    vm.last = {4, 7};
    *(regs + 0) = *(locals + 0); // body
    fun_129(vm, regs + 1); // call: exception_handler
    vm.temp_lval = locals + 3; // stack_trace
    U_LV_WRITEREF(vm, regs + 2);
    vm.temp_lval = locals + 2; // err
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {5, 7};
    *(regs + 0) = *(locals + 2); // err
    if (!(regs + 0)->True()) goto block0;
    vm.last = {6, 7};
    vm.last = {6, 7};
    *(regs + 0) = *(locals + 2); // err
    vm.temp_lval = locals + 4; // _
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {33, 7};
    block0:;
    DecVal(vm, locals[2]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// exception_handler
static void fun_478(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1031, locals);
    vm.last = {9, 7};
    fun_180(vm, regs + 0); // call: function173
    if (vm.ret_unwind_to == 173) goto block0;
    Pop(psp);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block0:;
    vm.last = {10, 7};
    U_PUSHNIL(vm, regs + 0);
    U_PUSHSTR(vm, regs + 1, 152); // ""
    (regs + 1)->LTINCRTNIL();
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// exception_handler
static void fun_129(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1035, locals);
    vm.last = {9, 7};
    fun_182(vm, regs + 0); // call: function175
    if (vm.ret_unwind_to == 175) goto block0;
    Pop(psp);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block0:;
    vm.last = {10, 7};
    U_PUSHNIL(vm, regs + 0);
    U_PUSHSTR(vm, regs + 1, 153); // ""
    (regs + 1)->LTINCRTNIL();
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// throw
static void fun_479(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1039, locals);
    vm.last = {17, 7};
    *(regs + 0) = *(locals + 0); // v
    U_PUSHSTR(vm, regs + 1, 154); // ""
    (regs + 1)->LTINCRTNIL();
    (regs + 0)->LTINCRTNIL();
    vm.ret_slots = 2;
    vm.ret_unwind_to = 125;
    DecVal(vm, locals[0]);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// throw
static void fun_130(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1043, locals);
    vm.last = {17, 7};
    *(regs + 0) = *(locals + 0); // v
    U_PUSHSTR(vm, regs + 1, 155); // ""
    (regs + 1)->LTINCRTNIL();
    (regs + 0)->LTINCRTNIL();
    vm.ret_slots = 2;
    vm.ret_unwind_to = 125;
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// try
static void fun_134(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1047, locals);
    vm.last = {33, 7};
    *(regs + 0) = *(locals + 0); // body
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_128(vm, regs + 2); // call: try
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// lookup
static void fun_537(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[4];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1051, locals);
    vm.last = {25, 8};
    *(regs + 0) = *(locals + 1); // key
    U_BCALLRET1(vm, regs + 1, 149, 1); // hash
    *(regs + 1) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 2, 0);
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_IMOD(vm, regs + 2);
    vm.temp_lval = locals + 2; // h
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {26, 8};
    *(regs + 0) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = *(locals + 2); // h
    U_VPUSHIDXI(vm, regs + 2);
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 3; // c
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {27, 8};
    block0:;
    *(regs + 0) = *(locals + 3); // c
    if (!(regs + 0)->True()) goto block1;
    vm.last = {28, 8};
    *(regs + 0) = *(locals + 3); // c
    U_PUSHFLD(vm, regs + 1, 1);
    *(regs + 1) = *(locals + 1); // key
    U_SEQ(vm, regs + 2);
    if (!(regs + 0)->True()) goto block2;
    vm.last = {29, 8};
    *(regs + 0) = *(locals + 3); // c
    (regs + 0)->LTINCRTNIL();
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    block2:;
    vm.last = {30, 8};
    *(regs + 0) = *(locals + 3); // c
    U_PUSHFLD(vm, regs + 1, 0);
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 3; // c
    U_LV_WRITEREF(vm, regs + 1);
    goto block0;
    block1:;
    vm.last = {31, 8};
    U_PUSHNIL(vm, regs + 0);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// lookup
static void fun_535(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[4];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1058, locals);
    vm.last = {25, 8};
    *(regs + 0) = *(locals + 1); // key
    U_BCALLRET1(vm, regs + 1, 149, 1); // hash
    *(regs + 1) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 2, 0);
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_IMOD(vm, regs + 2);
    vm.temp_lval = locals + 2; // h
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {26, 8};
    *(regs + 0) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = *(locals + 2); // h
    U_VPUSHIDXI(vm, regs + 2);
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 3; // c
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {27, 8};
    block0:;
    *(regs + 0) = *(locals + 3); // c
    if (!(regs + 0)->True()) goto block1;
    vm.last = {28, 8};
    *(regs + 0) = *(locals + 3); // c
    U_PUSHFLD(vm, regs + 1, 1);
    *(regs + 1) = *(locals + 1); // key
    U_SEQ(vm, regs + 2);
    if (!(regs + 0)->True()) goto block2;
    vm.last = {29, 8};
    *(regs + 0) = *(locals + 3); // c
    *(regs + 1) = *(locals + 2); // h
    (regs + 0)->LTINCRTNIL();
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    goto epilogue;
    block2:;
    vm.last = {30, 8};
    *(regs + 0) = *(locals + 3); // c
    U_PUSHFLD(vm, regs + 1, 0);
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 3; // c
    U_LV_WRITEREF(vm, regs + 1);
    goto block0;
    block1:;
    vm.last = {31, 8};
    U_PUSHNIL(vm, regs + 0);
    *(regs + 1) = *(locals + 2); // h
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// lookup
static void fun_507(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[4];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1065, locals);
    vm.last = {25, 8};
    *(regs + 0) = *(locals + 1); // key
    U_BCALLRET1(vm, regs + 1, 149, 1); // hash
    *(regs + 1) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 2, 0);
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_IMOD(vm, regs + 2);
    vm.temp_lval = locals + 2; // h
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {26, 8};
    *(regs + 0) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = *(locals + 2); // h
    U_VPUSHIDXI(vm, regs + 2);
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 3; // c
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {27, 8};
    block0:;
    *(regs + 0) = *(locals + 3); // c
    if (!(regs + 0)->True()) goto block1;
    vm.last = {28, 8};
    *(regs + 0) = *(locals + 3); // c
    U_PUSHFLD(vm, regs + 1, 1);
    *(regs + 1) = *(locals + 1); // key
    U_SEQ(vm, regs + 2);
    if (!(regs + 0)->True()) goto block2;
    vm.last = {29, 8};
    *(regs + 0) = *(locals + 3); // c
    (regs + 0)->LTINCRTNIL();
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    block2:;
    vm.last = {30, 8};
    *(regs + 0) = *(locals + 3); // c
    U_PUSHFLD(vm, regs + 1, 0);
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 3; // c
    U_LV_WRITEREF(vm, regs + 1);
    goto block0;
    block1:;
    vm.last = {31, 8};
    U_PUSHNIL(vm, regs + 0);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// lookup
static void fun_497(VMRef vm, StackPtr psp) {
    Value regs[6];
    Value locals[6];
    *(locals + 0) = *(psp - 4);
    *(locals + 1) = *(psp - 3);
    *(locals + 2) = *(psp - 2);
    *(locals + 3) = *(psp - 1);
    *(locals + 4) = Value(0, lobster::V_NIL);
    *(locals + 5) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1072, locals);
    vm.last = {25, 8};
    *(regs + 0) = *(locals + 1); // key.0
    *(regs + 1) = *(locals + 2); // key.1
    *(regs + 2) = *(locals + 3); // key.2
    *(regs + 3) = Value(3);
    U_BCALLRETV(vm, regs + 4, 152, 1); // hash
    *(regs + 1) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 2, 0);
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_IMOD(vm, regs + 2);
    vm.temp_lval = locals + 4; // h
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {26, 8};
    *(regs + 0) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = *(locals + 4); // h
    U_VPUSHIDXI(vm, regs + 2);
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 5; // c
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {27, 8};
    block0:;
    *(regs + 0) = *(locals + 5); // c
    if (!(regs + 0)->True()) goto block1;
    vm.last = {28, 8};
    *(regs + 0) = *(locals + 5); // c
    U_PUSHFLD2V(vm, regs + 1, 1, 3);
    *(regs + 3) = *(locals + 1); // key.0
    *(regs + 4) = *(locals + 2); // key.1
    *(regs + 5) = *(locals + 3); // key.2
    U_STEQ(vm, regs + 6, 3);
    if (!(regs + 0)->True()) goto block2;
    vm.last = {29, 8};
    *(regs + 0) = *(locals + 5); // c
    (regs + 0)->LTINCRTNIL();
    DecVal(vm, locals[0]);
    DecVal(vm, locals[5]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    block2:;
    vm.last = {30, 8};
    *(regs + 0) = *(locals + 5); // c
    U_PUSHFLD(vm, regs + 1, 0);
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 5; // c
    U_LV_WRITEREF(vm, regs + 1);
    goto block0;
    block1:;
    vm.last = {31, 8};
    U_PUSHNIL(vm, regs + 0);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[5]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// lookup
static void fun_495(VMRef vm, StackPtr psp) {
    Value regs[6];
    Value locals[6];
    *(locals + 0) = *(psp - 4);
    *(locals + 1) = *(psp - 3);
    *(locals + 2) = *(psp - 2);
    *(locals + 3) = *(psp - 1);
    *(locals + 4) = Value(0, lobster::V_NIL);
    *(locals + 5) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1081, locals);
    vm.last = {25, 8};
    *(regs + 0) = *(locals + 1); // key.0
    *(regs + 1) = *(locals + 2); // key.1
    *(regs + 2) = *(locals + 3); // key.2
    *(regs + 3) = Value(3);
    U_BCALLRETV(vm, regs + 4, 152, 1); // hash
    *(regs + 1) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 2, 0);
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_IMOD(vm, regs + 2);
    vm.temp_lval = locals + 4; // h
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {26, 8};
    *(regs + 0) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = *(locals + 4); // h
    U_VPUSHIDXI(vm, regs + 2);
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 5; // c
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {27, 8};
    block0:;
    *(regs + 0) = *(locals + 5); // c
    if (!(regs + 0)->True()) goto block1;
    vm.last = {28, 8};
    *(regs + 0) = *(locals + 5); // c
    U_PUSHFLD2V(vm, regs + 1, 1, 3);
    *(regs + 3) = *(locals + 1); // key.0
    *(regs + 4) = *(locals + 2); // key.1
    *(regs + 5) = *(locals + 3); // key.2
    U_STEQ(vm, regs + 6, 3);
    if (!(regs + 0)->True()) goto block2;
    vm.last = {29, 8};
    *(regs + 0) = *(locals + 5); // c
    *(regs + 1) = *(locals + 4); // h
    (regs + 0)->LTINCRTNIL();
    DecVal(vm, locals[0]);
    DecVal(vm, locals[5]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    goto epilogue;
    block2:;
    vm.last = {30, 8};
    *(regs + 0) = *(locals + 5); // c
    U_PUSHFLD(vm, regs + 1, 0);
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 5; // c
    U_LV_WRITEREF(vm, regs + 1);
    goto block0;
    block1:;
    vm.last = {31, 8};
    U_PUSHNIL(vm, regs + 0);
    *(regs + 1) = *(locals + 4); // h
    DecVal(vm, locals[0]);
    DecVal(vm, locals[5]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// lookup
static void fun_490(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[4];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1090, locals);
    vm.last = {25, 8};
    *(regs + 0) = *(locals + 1); // key
    U_BCALLRET1(vm, regs + 1, 149, 1); // hash
    *(regs + 1) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 2, 0);
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_IMOD(vm, regs + 2);
    vm.temp_lval = locals + 2; // h
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {26, 8};
    *(regs + 0) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = *(locals + 2); // h
    U_VPUSHIDXI(vm, regs + 2);
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 3; // c
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {27, 8};
    block0:;
    *(regs + 0) = *(locals + 3); // c
    if (!(regs + 0)->True()) goto block1;
    vm.last = {28, 8};
    *(regs + 0) = *(locals + 3); // c
    U_PUSHFLD(vm, regs + 1, 1);
    *(regs + 1) = *(locals + 1); // key
    U_SEQ(vm, regs + 2);
    if (!(regs + 0)->True()) goto block2;
    vm.last = {29, 8};
    *(regs + 0) = *(locals + 3); // c
    (regs + 0)->LTINCRTNIL();
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    block2:;
    vm.last = {30, 8};
    *(regs + 0) = *(locals + 3); // c
    U_PUSHFLD(vm, regs + 1, 0);
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 3; // c
    U_LV_WRITEREF(vm, regs + 1);
    goto block0;
    block1:;
    vm.last = {31, 8};
    U_PUSHNIL(vm, regs + 0);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// lookup
static void fun_488(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[4];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1097, locals);
    vm.last = {25, 8};
    *(regs + 0) = *(locals + 1); // key
    U_BCALLRET1(vm, regs + 1, 149, 1); // hash
    *(regs + 1) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 2, 0);
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_IMOD(vm, regs + 2);
    vm.temp_lval = locals + 2; // h
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {26, 8};
    *(regs + 0) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = *(locals + 2); // h
    U_VPUSHIDXI(vm, regs + 2);
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 3; // c
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {27, 8};
    block0:;
    *(regs + 0) = *(locals + 3); // c
    if (!(regs + 0)->True()) goto block1;
    vm.last = {28, 8};
    *(regs + 0) = *(locals + 3); // c
    U_PUSHFLD(vm, regs + 1, 1);
    *(regs + 1) = *(locals + 1); // key
    U_SEQ(vm, regs + 2);
    if (!(regs + 0)->True()) goto block2;
    vm.last = {29, 8};
    *(regs + 0) = *(locals + 3); // c
    *(regs + 1) = *(locals + 2); // h
    (regs + 0)->LTINCRTNIL();
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    goto epilogue;
    block2:;
    vm.last = {30, 8};
    *(regs + 0) = *(locals + 3); // c
    U_PUSHFLD(vm, regs + 1, 0);
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 3; // c
    U_LV_WRITEREF(vm, regs + 1);
    goto block0;
    block1:;
    vm.last = {31, 8};
    U_PUSHNIL(vm, regs + 0);
    *(regs + 1) = *(locals + 2); // h
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// lookup
static void fun_483(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[4];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1104, locals);
    vm.last = {25, 8};
    *(regs + 0) = *(locals + 1); // key
    U_BCALLRET1(vm, regs + 1, 151, 1); // hash
    *(regs + 1) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 2, 0);
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_IMOD(vm, regs + 2);
    vm.temp_lval = locals + 2; // h
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {26, 8};
    *(regs + 0) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = *(locals + 2); // h
    U_VPUSHIDXI(vm, regs + 2);
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 3; // c
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {27, 8};
    block0:;
    *(regs + 0) = *(locals + 3); // c
    if (!(regs + 0)->True()) goto block1;
    vm.last = {28, 8};
    *(regs + 0) = *(locals + 3); // c
    U_PUSHFLD(vm, regs + 1, 1);
    *(regs + 1) = *(locals + 1); // key
    U_FEQ(vm, regs + 2);
    if (!(regs + 0)->True()) goto block2;
    vm.last = {29, 8};
    *(regs + 0) = *(locals + 3); // c
    *(regs + 1) = *(locals + 2); // h
    (regs + 0)->LTINCRTNIL();
    DecVal(vm, locals[0]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    goto epilogue;
    block2:;
    vm.last = {30, 8};
    *(regs + 0) = *(locals + 3); // c
    U_PUSHFLD(vm, regs + 1, 0);
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 3; // c
    U_LV_WRITEREF(vm, regs + 1);
    goto block0;
    block1:;
    vm.last = {31, 8};
    U_PUSHNIL(vm, regs + 0);
    *(regs + 1) = *(locals + 2); // h
    DecVal(vm, locals[0]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// lookup
static void fun_138(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[4];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1111, locals);
    vm.last = {25, 8};
    *(regs + 0) = *(locals + 1); // key
    U_BCALLRET1(vm, regs + 1, 151, 1); // hash
    *(regs + 1) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 2, 0);
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_IMOD(vm, regs + 2);
    vm.temp_lval = locals + 2; // h
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {26, 8};
    *(regs + 0) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = *(locals + 2); // h
    U_VPUSHIDXI(vm, regs + 2);
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 3; // c
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {27, 8};
    block0:;
    *(regs + 0) = *(locals + 3); // c
    if (!(regs + 0)->True()) goto block1;
    vm.last = {28, 8};
    *(regs + 0) = *(locals + 3); // c
    U_PUSHFLD(vm, regs + 1, 1);
    *(regs + 1) = *(locals + 1); // key
    U_FEQ(vm, regs + 2);
    if (!(regs + 0)->True()) goto block2;
    vm.last = {29, 8};
    *(regs + 0) = *(locals + 3); // c
    (regs + 0)->LTINCRTNIL();
    DecVal(vm, locals[0]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    block2:;
    vm.last = {30, 8};
    *(regs + 0) = *(locals + 3); // c
    U_PUSHFLD(vm, regs + 1, 0);
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 3; // c
    U_LV_WRITEREF(vm, regs + 1);
    goto block0;
    block1:;
    vm.last = {31, 8};
    U_PUSHNIL(vm, regs + 0);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// set
static void fun_534(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[5];
    *(locals + 0) = *(psp - 3);
    *(locals + 1) = *(psp - 2);
    *(locals + 2) = *(psp - 1);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1118, locals);
    vm.last = {34, 8};
    *(regs + 0) = *(locals + 0); // this
    (regs + 0)->LTINCRTNIL();
    *(regs + 1) = *(locals + 1); // key
    (regs + 1)->LTINCRTNIL();
    fun_535(vm, regs + 2); // call: lookup
    vm.temp_lval = locals + 4; // h
    *(vm.temp_lval) = *(regs + 1);
    vm.temp_lval = locals + 3; // c
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {35, 8};
    *(regs + 0) = *(locals + 3); // c
    if (!(regs + 0)->True()) goto block0;
    vm.last = {36, 8};
    *(regs + 0) = *(locals + 2); // value
    *(regs + 1) = *(locals + 3); // c
    U_LVAL_FLD(vm, regs + 2, 2);
    *(vm.temp_lval) = *(regs + 0);
    goto block1;
    block0:;
    vm.last = {38, 8};
    *(regs + 0) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = *(locals + 4); // h
    U_VPUSHIDXI(vm, regs + 2);
    (regs + 0)->LTINCRTNIL();
    *(regs + 1) = *(locals + 1); // key
    (regs + 1)->LTINCRTNIL();
    *(regs + 2) = *(locals + 2); // value
    U_NEWOBJECT(vm, regs + 3, 523); // chain
    *(regs + 1) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 2, 0);
    *(regs + 2) = *(locals + 4); // h
    U_LVAL_IDXVI(vm, regs + 3, 0);
    U_LV_WRITEREF(vm, regs + 1);
    block1:;
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// set
static void fun_494(VMRef vm, StackPtr psp) {
    Value regs[7];
    Value locals[9];
    *(locals + 0) = *(psp - 7);
    *(locals + 1) = *(psp - 6);
    *(locals + 2) = *(psp - 5);
    *(locals + 3) = *(psp - 4);
    *(locals + 4) = *(psp - 3);
    *(locals + 5) = *(psp - 2);
    *(locals + 6) = *(psp - 1);
    *(locals + 7) = Value(0, lobster::V_NIL);
    *(locals + 8) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1126, locals);
    vm.last = {34, 8};
    *(regs + 0) = *(locals + 0); // this
    (regs + 0)->LTINCRTNIL();
    *(regs + 1) = *(locals + 1); // key.0
    *(regs + 2) = *(locals + 2); // key.1
    *(regs + 3) = *(locals + 3); // key.2
    fun_495(vm, regs + 4); // call: lookup
    vm.temp_lval = locals + 8; // h
    *(vm.temp_lval) = *(regs + 1);
    vm.temp_lval = locals + 7; // c
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {35, 8};
    *(regs + 0) = *(locals + 7); // c
    if (!(regs + 0)->True()) goto block0;
    vm.last = {36, 8};
    *(regs + 0) = *(locals + 4); // value.0
    *(regs + 1) = *(locals + 5); // value.1
    *(regs + 2) = *(locals + 6); // value.2
    *(regs + 3) = *(locals + 7); // c
    U_LVAL_FLD(vm, regs + 4, 4);
    U_LV_WRITEV(vm, regs + 3, 3);
    goto block1;
    block0:;
    vm.last = {38, 8};
    *(regs + 0) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = *(locals + 8); // h
    U_VPUSHIDXI(vm, regs + 2);
    (regs + 0)->LTINCRTNIL();
    *(regs + 1) = *(locals + 1); // key.0
    *(regs + 2) = *(locals + 2); // key.1
    *(regs + 3) = *(locals + 3); // key.2
    *(regs + 4) = *(locals + 4); // value.0
    *(regs + 5) = *(locals + 5); // value.1
    *(regs + 6) = *(locals + 6); // value.2
    U_NEWOBJECT(vm, regs + 7, 668); // chain
    *(regs + 1) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 2, 0);
    *(regs + 2) = *(locals + 8); // h
    U_LVAL_IDXVI(vm, regs + 3, 0);
    U_LV_WRITEREF(vm, regs + 1);
    block1:;
    DecVal(vm, locals[7]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// set
static void fun_487(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[5];
    *(locals + 0) = *(psp - 3);
    *(locals + 1) = *(psp - 2);
    *(locals + 2) = *(psp - 1);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1138, locals);
    vm.last = {34, 8};
    *(regs + 0) = *(locals + 0); // this
    (regs + 0)->LTINCRTNIL();
    *(regs + 1) = *(locals + 1); // key
    (regs + 1)->LTINCRTNIL();
    fun_488(vm, regs + 2); // call: lookup
    vm.temp_lval = locals + 4; // h
    *(vm.temp_lval) = *(regs + 1);
    vm.temp_lval = locals + 3; // c
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {35, 8};
    *(regs + 0) = *(locals + 3); // c
    if (!(regs + 0)->True()) goto block0;
    vm.last = {36, 8};
    *(regs + 0) = *(locals + 2); // value
    (regs + 0)->LTINCRTNIL();
    *(regs + 1) = *(locals + 3); // c
    U_LVAL_FLD(vm, regs + 2, 2);
    U_LV_WRITEREF(vm, regs + 1);
    goto block1;
    block0:;
    vm.last = {38, 8};
    *(regs + 0) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = *(locals + 4); // h
    U_VPUSHIDXI(vm, regs + 2);
    (regs + 0)->LTINCRTNIL();
    *(regs + 1) = *(locals + 1); // key
    (regs + 1)->LTINCRTNIL();
    *(regs + 2) = *(locals + 2); // value
    (regs + 2)->LTINCRTNIL();
    U_NEWOBJECT(vm, regs + 3, 640); // chain
    *(regs + 1) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 2, 0);
    *(regs + 2) = *(locals + 4); // h
    U_LVAL_IDXVI(vm, regs + 3, 0);
    U_LV_WRITEREF(vm, regs + 1);
    block1:;
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// set
static void fun_139(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[5];
    *(locals + 0) = *(psp - 3);
    *(locals + 1) = *(psp - 2);
    *(locals + 2) = *(psp - 1);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1146, locals);
    vm.last = {34, 8};
    *(regs + 0) = *(locals + 0); // this
    (regs + 0)->LTINCRTNIL();
    *(regs + 1) = *(locals + 1); // key
    fun_483(vm, regs + 2); // call: lookup
    vm.temp_lval = locals + 4; // h
    *(vm.temp_lval) = *(regs + 1);
    vm.temp_lval = locals + 3; // c
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {35, 8};
    *(regs + 0) = *(locals + 3); // c
    if (!(regs + 0)->True()) goto block0;
    vm.last = {36, 8};
    *(regs + 0) = *(locals + 2); // value
    *(regs + 1) = *(locals + 3); // c
    U_LVAL_FLD(vm, regs + 2, 2);
    *(vm.temp_lval) = *(regs + 0);
    goto block1;
    block0:;
    vm.last = {38, 8};
    *(regs + 0) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = *(locals + 4); // h
    U_VPUSHIDXI(vm, regs + 2);
    (regs + 0)->LTINCRTNIL();
    *(regs + 1) = *(locals + 1); // key
    *(regs + 2) = *(locals + 2); // value
    U_NEWOBJECT(vm, regs + 3, 469); // chain
    *(regs + 1) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 2, 0);
    *(regs + 2) = *(locals + 4); // h
    U_LVAL_IDXVI(vm, regs + 3, 0);
    U_LV_WRITEREF(vm, regs + 1);
    block1:;
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// get
static void fun_144(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[3];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1154, locals);
    vm.last = {66, 8};
    *(regs + 0) = *(locals + 0); // this
    (regs + 0)->LTINCRTNIL();
    *(regs + 1) = *(locals + 1); // key
    (regs + 1)->LTINCRTNIL();
    fun_507(vm, regs + 2); // call: lookup
    vm.temp_lval = locals + 2; // c
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {67, 8};
    *(regs + 0) = *(locals + 2); // c
    if (!(regs + 0)->True()) goto block0;
    U_POP(vm, regs + 1);
    *(regs + 0) = *(locals + 2); // c
    U_PUSHFLD(vm, regs + 1, 2);
    block0:;
    if ((regs + 0)->True()) goto block1;
    U_POP(vm, regs + 1);
    U_PUSHNIL(vm, regs + 0);
    block1:;
    (regs + 0)->LTINCRTNIL();
    DecVal(vm, locals[0]);
    DecVal(vm, locals[2]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// get
static void fun_536(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[4];
    *(locals + 0) = *(psp - 3);
    *(locals + 1) = *(psp - 2);
    *(locals + 2) = *(psp - 1);
    *(locals + 3) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1160, locals);
    vm.last = {73, 8};
    *(regs + 0) = *(locals + 0); // this
    (regs + 0)->LTINCRTNIL();
    *(regs + 1) = *(locals + 1); // key
    (regs + 1)->LTINCRTNIL();
    fun_537(vm, regs + 2); // call: lookup
    vm.temp_lval = locals + 3; // c
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {74, 8};
    *(regs + 0) = *(locals + 3); // c
    if (!(regs + 0)->True()) goto block0;
    vm.last = {74, 8};
    *(regs + 0) = *(locals + 3); // c
    U_PUSHFLD(vm, regs + 1, 2);
    goto block1;
    block0:;
    vm.last = {74, 8};
    *(regs + 0) = *(locals + 2); // default_val
    block1:;
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// get
static void fun_496(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[8];
    *(locals + 0) = *(psp - 7);
    *(locals + 1) = *(psp - 6);
    *(locals + 2) = *(psp - 5);
    *(locals + 3) = *(psp - 4);
    *(locals + 4) = *(psp - 3);
    *(locals + 5) = *(psp - 2);
    *(locals + 6) = *(psp - 1);
    *(locals + 7) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1167, locals);
    vm.last = {73, 8};
    *(regs + 0) = *(locals + 0); // this
    (regs + 0)->LTINCRTNIL();
    *(regs + 1) = *(locals + 1); // key.0
    *(regs + 2) = *(locals + 2); // key.1
    *(regs + 3) = *(locals + 3); // key.2
    fun_497(vm, regs + 4); // call: lookup
    vm.temp_lval = locals + 7; // c
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {74, 8};
    *(regs + 0) = *(locals + 7); // c
    if (!(regs + 0)->True()) goto block0;
    vm.last = {74, 8};
    *(regs + 0) = *(locals + 7); // c
    U_PUSHFLD2V(vm, regs + 1, 4, 3);
    goto block1;
    block0:;
    vm.last = {74, 8};
    *(regs + 0) = *(locals + 4); // default_val.0
    *(regs + 1) = *(locals + 5); // default_val.1
    *(regs + 2) = *(locals + 6); // default_val.2
    block1:;
    DecVal(vm, locals[7]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    *(psp++) = *(regs + 2);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// get
static void fun_489(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[4];
    *(locals + 0) = *(psp - 3);
    *(locals + 1) = *(psp - 2);
    *(locals + 2) = *(psp - 1);
    *(locals + 3) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1178, locals);
    vm.last = {73, 8};
    *(regs + 0) = *(locals + 0); // this
    (regs + 0)->LTINCRTNIL();
    *(regs + 1) = *(locals + 1); // key
    (regs + 1)->LTINCRTNIL();
    fun_490(vm, regs + 2); // call: lookup
    vm.temp_lval = locals + 3; // c
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {74, 8};
    *(regs + 0) = *(locals + 3); // c
    if (!(regs + 0)->True()) goto block0;
    vm.last = {74, 8};
    *(regs + 0) = *(locals + 3); // c
    U_PUSHFLD(vm, regs + 1, 2);
    (regs + 0)->LTINCRTNIL();
    goto block1;
    block0:;
    vm.last = {74, 8};
    *(regs + 0) = *(locals + 2); // default_val
    (regs + 0)->LTINCRTNIL();
    block1:;
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// get
static void fun_145(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[4];
    *(locals + 0) = *(psp - 3);
    *(locals + 1) = *(psp - 2);
    *(locals + 2) = *(psp - 1);
    *(locals + 3) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1185, locals);
    vm.last = {73, 8};
    *(regs + 0) = *(locals + 0); // this
    (regs + 0)->LTINCRTNIL();
    *(regs + 1) = *(locals + 1); // key
    fun_138(vm, regs + 2); // call: lookup
    vm.temp_lval = locals + 3; // c
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {74, 8};
    *(regs + 0) = *(locals + 3); // c
    if (!(regs + 0)->True()) goto block0;
    vm.last = {74, 8};
    *(regs + 0) = *(locals + 3); // c
    U_PUSHFLD(vm, regs + 1, 2);
    goto block1;
    block0:;
    vm.last = {74, 8};
    *(regs + 0) = *(locals + 2); // default_val
    block1:;
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// dictionary
static void fun_531(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1192, locals);
    vm.last = {120, 8};
    *(regs + 0) = *(locals + 0); // size
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_532(vm, regs + 2); // call: map
    U_NEWOBJECT(vm, regs + 1, 514); // dictionary
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// dictionary
static void fun_504(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1196, locals);
    vm.last = {120, 8};
    *(regs + 0) = *(locals + 0); // size
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_505(vm, regs + 2); // call: map
    U_NEWOBJECT(vm, regs + 1, 488); // dictionary
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// dictionary
static void fun_491(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1200, locals);
    vm.last = {120, 8};
    *(regs + 0) = *(locals + 0); // size
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_492(vm, regs + 2); // call: map
    U_NEWOBJECT(vm, regs + 1, 659); // dictionary
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// dictionary
static void fun_484(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1204, locals);
    vm.last = {120, 8};
    *(regs + 0) = *(locals + 0); // size
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_485(vm, regs + 2); // call: map
    U_NEWOBJECT(vm, regs + 1, 631); // dictionary
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// dictionary
static void fun_151(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1208, locals);
    vm.last = {120, 8};
    *(regs + 0) = *(locals + 0); // size
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_482(vm, regs + 2); // call: map
    U_NEWOBJECT(vm, regs + 1, 460); // dictionary
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// test_namespace.f
static void fun_153(VMRef vm, StackPtr psp) {
    Value regs[1];
    PushFunId(vm, funinfo_table + 1212, 0);
    vm.last = {11, 6};
    vm.last = {12, 6};
    *(regs + 0) = Value(1);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function152
static void fun_156(VMRef vm, StackPtr psp) {
    Value regs[12];
    Value keepvar[24];
    Value locals[72];
    *(locals + 0) = Value(0, lobster::V_NIL);
    *(locals + 1) = Value(0, lobster::V_NIL);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    *(locals + 5) = Value(0, lobster::V_NIL);
    *(locals + 6) = Value(0, lobster::V_NIL);
    *(locals + 7) = Value(0, lobster::V_NIL);
    *(locals + 8) = Value(0, lobster::V_NIL);
    *(locals + 9) = Value(0, lobster::V_NIL);
    *(locals + 10) = Value(0, lobster::V_NIL);
    *(locals + 11) = Value(0, lobster::V_NIL);
    *(locals + 12) = Value(0, lobster::V_NIL);
    *(locals + 13) = Value(0, lobster::V_NIL);
    *(locals + 14) = Value(0, lobster::V_NIL);
    *(locals + 15) = Value(0, lobster::V_NIL);
    *(locals + 16) = Value(0, lobster::V_NIL);
    *(locals + 17) = Value(0, lobster::V_NIL);
    *(locals + 18) = Value(0, lobster::V_NIL);
    *(locals + 19) = Value(0, lobster::V_NIL);
    *(locals + 20) = Value(0, lobster::V_NIL);
    *(locals + 21) = Value(0, lobster::V_NIL);
    *(locals + 22) = Value(0, lobster::V_NIL);
    *(locals + 23) = Value(0, lobster::V_NIL);
    *(locals + 24) = Value(0, lobster::V_NIL);
    *(locals + 25) = Value(0, lobster::V_NIL);
    *(locals + 26) = Value(0, lobster::V_NIL);
    *(locals + 27) = Value(0, lobster::V_NIL);
    *(locals + 28) = Value(0, lobster::V_NIL);
    *(locals + 29) = Value(0, lobster::V_NIL);
    *(locals + 30) = Value(0, lobster::V_NIL);
    *(locals + 31) = Value(0, lobster::V_NIL);
    *(locals + 32) = Value(0, lobster::V_NIL);
    *(locals + 33) = Value(0, lobster::V_NIL);
    *(locals + 34) = Value(0, lobster::V_NIL);
    *(locals + 35) = Value(0, lobster::V_NIL);
    *(locals + 36) = Value(0, lobster::V_NIL);
    *(locals + 37) = Value(0, lobster::V_NIL);
    *(locals + 38) = Value(0, lobster::V_NIL);
    *(locals + 39) = Value(0, lobster::V_NIL);
    *(locals + 40) = Value(0, lobster::V_NIL);
    *(locals + 41) = Value(0, lobster::V_NIL);
    *(locals + 42) = Value(0, lobster::V_NIL);
    *(locals + 43) = Value(0, lobster::V_NIL);
    *(locals + 44) = Value(0, lobster::V_NIL);
    *(locals + 45) = Value(0, lobster::V_NIL);
    *(locals + 46) = Value(0, lobster::V_NIL);
    *(locals + 47) = Value(0, lobster::V_NIL);
    *(locals + 48) = Value(0, lobster::V_NIL);
    *(locals + 49) = Value(0, lobster::V_NIL);
    *(locals + 50) = Value(0, lobster::V_NIL);
    *(locals + 51) = Value(0, lobster::V_NIL);
    *(locals + 52) = Value(0, lobster::V_NIL);
    *(locals + 53) = Value(0, lobster::V_NIL);
    *(locals + 54) = Value(0, lobster::V_NIL);
    *(locals + 55) = Value(0, lobster::V_NIL);
    *(locals + 56) = Value(0, lobster::V_NIL);
    *(locals + 57) = Value(0, lobster::V_NIL);
    *(locals + 58) = Value(0, lobster::V_NIL);
    *(locals + 59) = Value(0, lobster::V_NIL);
    *(locals + 60) = Value(0, lobster::V_NIL);
    *(locals + 61) = Value(0, lobster::V_NIL);
    *(locals + 62) = Value(0, lobster::V_NIL);
    *(locals + 63) = Value(0, lobster::V_NIL);
    *(locals + 64) = Value(0, lobster::V_NIL);
    *(locals + 65) = Value(0, lobster::V_NIL);
    *(locals + 66) = Value(0, lobster::V_NIL);
    *(locals + 67) = Value(0, lobster::V_NIL);
    *(locals + 68) = Value(0, lobster::V_NIL);
    *(locals + 69) = Value(0, lobster::V_NIL);
    *(locals + 70) = Value(0, lobster::V_NIL);
    *(locals + 71) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1215, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    *(keepvar + 1) = Value(0, lobster::V_NIL);
    *(keepvar + 2) = Value(0, lobster::V_NIL);
    *(keepvar + 3) = Value(0, lobster::V_NIL);
    *(keepvar + 4) = Value(0, lobster::V_NIL);
    *(keepvar + 5) = Value(0, lobster::V_NIL);
    *(keepvar + 6) = Value(0, lobster::V_NIL);
    *(keepvar + 7) = Value(0, lobster::V_NIL);
    *(keepvar + 8) = Value(0, lobster::V_NIL);
    *(keepvar + 9) = Value(0, lobster::V_NIL);
    *(keepvar + 10) = Value(0, lobster::V_NIL);
    *(keepvar + 11) = Value(0, lobster::V_NIL);
    *(keepvar + 12) = Value(0, lobster::V_NIL);
    *(keepvar + 13) = Value(0, lobster::V_NIL);
    *(keepvar + 14) = Value(0, lobster::V_NIL);
    *(keepvar + 15) = Value(0, lobster::V_NIL);
    *(keepvar + 16) = Value(0, lobster::V_NIL);
    *(keepvar + 17) = Value(0, lobster::V_NIL);
    *(keepvar + 18) = Value(0, lobster::V_NIL);
    *(keepvar + 19) = Value(0, lobster::V_NIL);
    *(keepvar + 20) = Value(0, lobster::V_NIL);
    *(keepvar + 21) = Value(0, lobster::V_NIL);
    *(keepvar + 22) = Value(0, lobster::V_NIL);
    *(keepvar + 23) = Value(0, lobster::V_NIL);
    vm.last = {20, 6};
    vm.last = {27, 6};
    *(regs + 0) = Value(1);
    vm.temp_lval = locals + 0; // vardef
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {28, 6};
    vm.temp_lval = locals + 0; // vardef
    U_LV_IPP(vm, regs + 0);
    vm.last = {29, 6};
    *(regs + 0) = Value(1);
    vm.temp_lval = locals + 1; // constdef
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {32, 6};
    fun_153(vm, regs + 0); // call: test_namespace.f
    *(regs + 1) = Value(1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 32, 6, 156);
    vm.last = {33, 6};
    *(regs + 0) = Value(1);
    U_NEWOBJECT(vm, regs + 1, 544); // test_namespace.g
    *(keepvar + 0) = *(regs + 0);
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = Value(1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 33, 6, 157);
    vm.last = {34, 6};
    vm.last = {34, 6};
    *(regs + 0) = Value(1);
    U_NEWOBJECT(vm, regs + 1, 544); // test_namespace.g
    vm.temp_lval = locals + 59; // this
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {15, 6};
    *(regs + 0) = Value(2);
    *(regs + 1) = Value(2);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 34, 6, 158);
    vm.last = {36, 6};
    vm.last = {36, 6};
    vm.last = {38, 6};
    vm.last = {38, 6};
    vm.last = {42, 6};
    vm.last = {43, 6};
    vm.last = {44, 6};
    vm.last = {45, 6};
    vm.last = {48, 6};
    vm.last = {48, 6};
    *(regs + 0) = Value(1);
    vm.temp_lval = locals + 60; // x
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {48, 6};
    *(regs + 0) = Value(0);
    vm.temp_lval = locals + 61; // y
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {42, 6};
    *(regs + 0) = Value(1);
    *(regs + 1) = Value(1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 48, 6, 159);
    vm.last = {49, 6};
    vm.last = {49, 6};
    U_PUSHSTR(vm, regs + 0, 160); // ""
    vm.temp_lval = locals + 62; // x
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {49, 6};
    *(regs + 0) = Value(0);
    vm.temp_lval = locals + 63; // y
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {43, 6};
    *(regs + 0) = Value(2);
    *(regs + 1) = Value(2);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 49, 6, 161);
    vm.last = {50, 6};
    *(regs + 0) = Value(0);
    *(regs + 1) = Value(0);
    U_NEWOBJECT(vm, regs + 2, 562); // testb
    *(regs + 1) = Value(4);
    fun_160(vm, regs + 2); // call: tf
    *(regs + 1) = Value(4);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 50, 6, 162);
    vm.last = {53, 6};
    vm.last = {54, 6};
    vm.last = {54, 6};
    *(regs + 0) = Value(0);
    *(regs + 1) = Value(0);
    U_NEWOBJECT(vm, regs + 2, 562); // testb
    vm.temp_lval = locals + 64; // a
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {53, 6};
    *(regs + 0) = *(locals + 64); // a
    (regs + 0)->LTINCRTNIL();
    *(regs + 1) = Value(1);
    U_DDCALL(vm, regs + 2, 0, 1); vm.next_call_target(vm, regs + 2);
    *(regs + 1) = Value(4);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 54, 6, 163);
    vm.last = {56, 6};
    vm.last = {56, 6};
    vm.last = {68, 6};
    *(regs + 0) = Value(0);
    *(regs + 1) = Value(1);
    *(regs + 2) = Value(2.000000);
    *(regs + 3) = Value(3.000000);
    *(regs + 4) = Value(4.000000);
    *(regs + 5) = Value(5.000000);
    U_PUSHSTR(vm, regs + 6, 164); // "hello, world!\n\"\'\r\t\\\300"
    (regs + 6)->LTINCRTNIL();
    *(regs + 7) = Value(0);
    *(regs + 8) = Value(-64);
    U_NEWVEC(vm, regs + 9, 7, 2);
    U_PUSHNIL(vm, regs + 8);
    *(regs + 9) = Value(1);
    *(regs + 10) = Value(6);
    *(regs + 11) = Value(7);
    U_NEWOBJECT(vm, regs + 12, 562); // testb
    U_NEWOBJECT(vm, regs + 11, 574); // parsetest
    vm.temp_lval = locals + 2; // direct
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {72, 6};
    *(regs + 0) = Value(574);
    *(regs + 1) = *(locals + 2); // direct
    U_A2S(vm, regs + 2, 574);
    *(keepvar + 1) = *(regs + 1);
    U_BCALLRETV(vm, regs + 2, 254, 1); // parse_data
    vm.temp_lval = locals + 4; // err
    U_LV_WRITEREF(vm, regs + 2);
    vm.temp_lval = locals + 3; // parsed
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {73, 6};
    *(regs + 0) = *(locals + 4); // err
    if (!(regs + 0)->True()) goto block0;
    vm.last = {73, 6};
    *(regs + 0) = *(locals + 4); // err
    U_BCALLRET1(vm, regs + 1, 0, 0); // print
    block0:;
    vm.last = {74, 6};
    *(regs + 0) = *(locals + 3); // parsed
    *(regs + 1) = *(locals + 2); // direct
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 74, 6, 165);
    vm.last = {76, 6};
    *(regs + 0) = Value(574);
    U_PUSHSTR(vm, regs + 1, 166); // "parsetest { true, 1, 1.0, float3 {}, \"\", [], nil, "
    U_BCALLRETV(vm, regs + 2, 254, 1); // parse_data
    vm.temp_lval = locals + 4; // err
    U_LV_WRITEREF(vm, regs + 2);
    vm.temp_lval = locals + 3; // parsed
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {77, 6};
    *(regs + 0) = *(locals + 4); // err
    if (!(regs + 0)->True()) goto block1;
    vm.last = {77, 6};
    *(regs + 0) = *(locals + 4); // err
    U_BCALLRET1(vm, regs + 1, 0, 0); // print
    block1:;
    vm.last = {78, 6};
    *(regs + 0) = Value(1);
    *(regs + 1) = Value(1);
    *(regs + 2) = Value(1.000000);
    *(regs + 3) = Value(0.000000);
    *(regs + 4) = Value(0.000000);
    *(regs + 5) = Value(0.000000);
    U_PUSHSTR(vm, regs + 6, 167); // ""
    (regs + 6)->LTINCRTNIL();
    U_NEWVEC(vm, regs + 7, 7, 0);
    U_PUSHNIL(vm, regs + 8);
    *(regs + 9) = Value(-1);
    *(regs + 10) = Value(6);
    *(regs + 11) = Value(7);
    U_NEWOBJECT(vm, regs + 12, 562); // testb
    U_NEWOBJECT(vm, regs + 11, 574); // parsetest
    vm.temp_lval = locals + 5; // groundv
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {79, 6};
    *(regs + 0) = *(locals + 3); // parsed
    *(regs + 1) = *(locals + 5); // groundv
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 79, 6, 168);
    vm.last = {80, 6};
    *(regs + 0) = *(locals + 3); // parsed
    *(regs + 1) = Value(0);
    *(regs + 2) = Value(0);
    U_BCALLRET3(vm, regs + 3, 255, 1); // flexbuffers_value_to_binary
    vm.temp_lval = locals + 6; // flex
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {81, 6};
    *(regs + 0) = *(locals + 6); // flex
    *(regs + 1) = Value(0);
    U_PUSHSTR(vm, regs + 2, 169); // ""
    U_BCALLRETV(vm, regs + 3, 257, 1); // flexbuffers_binary_to_json
    U_POPREF(vm, regs + 2);
    vm.temp_lval = locals + 7; // json
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {82, 6};
    *(regs + 0) = *(locals + 7); // json
    U_ASSERT(vm, regs + 1, 82, 6, 170);
    vm.last = {84, 6};
    U_PUSHSTR(vm, regs + 0, 171); // "{ a: 1, b: 1.0, d: \"\", e: [  ], g: -1, h: 1, s: { "
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 8; // groundj
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {85, 6};
    *(regs + 0) = *(locals + 7); // json
    *(regs + 1) = *(locals + 8); // groundj
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 85, 6, 172);
    vm.last = {86, 6};
    *(regs + 0) = *(locals + 7); // json
    U_PUSHNIL(vm, regs + 1);
    U_BCALLRET2(vm, regs + 2, 258, 1); // flexbuffers_json_to_binary
    vm.temp_lval = locals + 10; // ferr
    U_LV_WRITEREF(vm, regs + 2);
    vm.temp_lval = locals + 9; // flex2
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {87, 6};
    *(regs + 0) = *(locals + 10); // ferr
    U_LOGNOTREF(vm, regs + 1);
    U_ASSERT(vm, regs + 1, 87, 6, 173);
    vm.last = {88, 6};
    *(regs + 0) = *(locals + 9); // flex2
    *(regs + 1) = Value(0);
    U_PUSHSTR(vm, regs + 2, 174); // ""
    U_BCALLRETV(vm, regs + 3, 257, 1); // flexbuffers_binary_to_json
    U_POPREF(vm, regs + 2);
    vm.temp_lval = locals + 11; // json2
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {89, 6};
    *(regs + 0) = *(locals + 11); // json2
    U_ASSERT(vm, regs + 1, 89, 6, 175);
    vm.last = {90, 6};
    *(regs + 0) = *(locals + 11); // json2
    *(regs + 1) = *(locals + 8); // groundj
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 90, 6, 176);
    vm.last = {91, 6};
    *(regs + 0) = Value(574);
    *(regs + 1) = *(locals + 9); // flex2
    U_BCALLRETV(vm, regs + 2, 256, 1); // flexbuffers_binary_to_value
    vm.temp_lval = locals + 13; // fverr
    U_LV_WRITEREF(vm, regs + 2);
    vm.temp_lval = locals + 12; // fval
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {92, 6};
    *(regs + 0) = *(locals + 13); // fverr
    U_LOGNOTREF(vm, regs + 1);
    U_ASSERT(vm, regs + 1, 92, 6, 177);
    vm.last = {93, 6};
    *(regs + 0) = *(locals + 12); // fval
    *(regs + 1) = *(locals + 5); // groundv
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 93, 6, 178);
    vm.last = {94, 6};
    *(regs + 0) = *(locals + 3); // parsed
    U_BCALLRET1(vm, regs + 1, 259, 1); // lobster_value_to_binary
    vm.temp_lval = locals + 14; // lb
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {95, 6};
    *(regs + 0) = Value(574);
    *(regs + 1) = *(locals + 14); // lb
    U_BCALLRETV(vm, regs + 2, 260, 1); // lobster_binary_to_value
    vm.temp_lval = locals + 16; // lberr
    U_LV_WRITEREF(vm, regs + 2);
    vm.temp_lval = locals + 15; // lbval
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {96, 6};
    *(regs + 0) = *(locals + 16); // lberr
    U_LOGNOTREF(vm, regs + 1);
    U_ASSERT(vm, regs + 1, 96, 6, 179);
    vm.last = {97, 6};
    *(regs + 0) = *(locals + 15); // lbval
    *(regs + 1) = *(locals + 5); // groundv
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 97, 6, 180);
    vm.last = {99, 6};
    *(regs + 0) = Value(544);
    U_PUSHSTR(vm, regs + 1, 181); // "test_namespace.g {}"
    U_BCALLRETV(vm, regs + 2, 254, 1); // parse_data
    U_POPREF(vm, regs + 2);
    *(keepvar + 2) = *(regs + 0);
    U_ASSERT(vm, regs + 1, 99, 6, 182);
    vm.last = {101, 6};
    *(regs + 0) = Value(2);
    U_BCALLRET1(vm, regs + 1, 93, 1); // rnd
    U_POP(vm, regs + 1);
    vm.last = {102, 6};
    *(regs + 0) = Value(1);
    U_ASSERT(vm, regs + 1, 101, 6, 183);
    vm.last = {104, 6};
    *(regs + 0) = Value(12518);
    *(regs + 1) = Value(12540);
    *(regs + 2) = Value(12470);
    *(regs + 3) = Value(12540);
    *(regs + 4) = Value(21029);
    *(regs + 5) = Value(12469);
    *(regs + 6) = Value(12452);
    *(regs + 7) = Value(12488);
    U_NEWVEC(vm, regs + 8, 7, 8);
    vm.temp_lval = locals + 17; // unicodetests
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {105, 6};
    *(regs + 0) = *(locals + 17); // unicodetests
    U_BCALLRET1(vm, regs + 1, 43, 1); // unicode_to_string
    *(keepvar + 3) = *(regs + 0);
    U_BCALLRET1(vm, regs + 1, 44, 1); // string_to_unicode
    U_POP(vm, regs + 2);
    *(keepvar + 4) = *(regs + 0);
    *(regs + 1) = *(locals + 17); // unicodetests
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 105, 6, 184);
    vm.last = {107, 6};
    *(regs + 0) = Value(10);
    U_PUSHFUN(vm, regs + 1, 0, fun_162);
    fun_7(vm, regs + 2); // call: map
    vm.temp_lval = locals + 18; // st
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {115, 6};
    *(regs + 0) = *(locals + 18); // st
    U_PUSHSTR(vm, regs + 1, 185); // "what?"
    (regs + 1)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 2, 186); // "no"
    (regs + 2)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 3, 187); // "yes2"
    (regs + 3)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 4, 188); // "yes3"
    (regs + 4)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 5, 189); // "maybe"
    (regs + 5)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 6, 190); // "maybe"
    (regs + 6)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 7, 191); // "maybe"
    (regs + 7)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 8, 192); // "what?"
    (regs + 8)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 9, 193); // "maybe"
    (regs + 9)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 10, 194); // "what?"
    (regs + 10)->LTINCRTNIL();
    U_NEWVEC(vm, regs + 11, 11, 10);
    *(keepvar + 5) = *(regs + 1);
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 115, 6, 195);
    vm.last = {116, 6};
    *(regs + 0) = *(locals + 18); // st
    *(regs + 1) = Value(2);
    U_VPUSHIDXI(vm, regs + 2);
    U_DUP(vm, regs + 1);
    U_PUSHSTR(vm, regs + 2, 196); // "yes2"
    U_SEQ(vm, regs + 3);
    if (!(regs + 1)->True()) goto block2;
    U_POP(vm, regs + 1);
    vm.last = {117, 6};
    *(regs + 0) = Value(1);
    goto block3;
    block2:;
    U_DUP(vm, regs + 1);
    *(regs + 2) = *(locals + 18); // st
    *(regs + 3) = Value(3);
    U_VPUSHIDXI(vm, regs + 4);
    U_SEQ(vm, regs + 3);
    if (!(regs + 1)->True()) goto block4;
    U_POP(vm, regs + 1);
    vm.last = {118, 6};
    *(regs + 0) = Value(0);
    goto block5;
    block4:;
    U_POP(vm, regs + 1);
    vm.last = {119, 6};
    *(regs + 0) = Value(0);
    block3:;
    block5:;
    U_ASSERT(vm, regs + 1, 116, 6, 197);
    vm.last = {120, 6};
    U_PUSHFLT64(vm, regs + 0, 1374389535, 1074339512); // 3.14
    U_DUP(vm, regs + 1);
    *(regs + 2) = Value(10.000000);
    U_FGE(vm, regs + 3);
    if (!(regs + 1)->True()) goto block6;
    U_DUP(vm, regs + 1);
    *(regs + 2) = Value(20.000000);
    U_FLE(vm, regs + 3);
    if (!(regs + 1)->True()) goto block7;
    U_POP(vm, regs + 1);
    vm.last = {121, 6};
    *(regs + 0) = Value(0);
    goto block8;
    block6:;
    block7:;
    U_POP(vm, regs + 1);
    vm.last = {122, 6};
    *(regs + 0) = Value(1);
    block8:;
    U_ASSERT(vm, regs + 1, 120, 6, 198);
    vm.last = {124, 6};
    vm.last = {124, 6};
    U_PUSHFUN(vm, regs + 0, 0, fun_163);
    vm.temp_lval = locals + 65; // f
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {253, 5};
    fun_163(vm, regs + 0); // call: function156
    vm.last = {130, 6};
    *(regs + 0) = Value(3);
    *(regs + 1) = Value(9);
    *(regs + 2) = Value(5);
    *(regs + 3) = Value(4);
    *(regs + 4) = Value(1);
    *(regs + 5) = Value(3);
    *(regs + 6) = Value(9);
    *(regs + 7) = Value(5);
    *(regs + 8) = Value(4);
    *(regs + 9) = Value(1);
    U_NEWVEC(vm, regs + 10, 7, 10);
    vm.temp_lval = locals + 19; // testvector
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {132, 6};
    *(regs + 0) = *(locals + 19); // testvector
    (regs + 0)->LTINCRTNIL();
    U_PUSHFUN(vm, regs + 1, 0, fun_164);
    fun_48(vm, regs + 2); // call: qsort
    vm.temp_lval = locals + 20; // sorted1
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {133, 6};
    *(regs + 0) = *(locals + 19); // testvector
    U_BCALLRET1(vm, regs + 1, 29, 1); // copy
    vm.temp_lval = locals + 21; // sorted2
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {134, 6};
    *(regs + 0) = *(locals + 21); // sorted2
    (regs + 0)->LTINCRTNIL();
    U_PUSHFUN(vm, regs + 1, 0, fun_165);
    fun_52(vm, regs + 2); // call: insertion_sort
    vm.last = {135, 6};
    *(regs + 0) = *(locals + 19); // testvector
    U_BCALLRET1(vm, regs + 1, 29, 1); // copy
    vm.temp_lval = locals + 22; // sorted3
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {136, 6};
    *(regs + 0) = *(locals + 22); // sorted3
    U_PUSHFUN(vm, regs + 1, 0, fun_166);
    fun_50(vm, regs + 2); // call: qsort_in_place
    vm.last = {138, 6};
    *(regs + 0) = *(locals + 20); // sorted1
    *(regs + 1) = Value(1);
    *(regs + 2) = Value(1);
    *(regs + 3) = Value(3);
    *(regs + 4) = Value(3);
    *(regs + 5) = Value(4);
    *(regs + 6) = Value(4);
    *(regs + 7) = Value(5);
    *(regs + 8) = Value(5);
    *(regs + 9) = Value(9);
    *(regs + 10) = Value(9);
    U_NEWVEC(vm, regs + 11, 7, 10);
    *(keepvar + 6) = *(regs + 1);
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 138, 6, 199);
    vm.last = {139, 6};
    *(regs + 0) = *(locals + 20); // sorted1
    *(regs + 1) = *(locals + 21); // sorted2
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 139, 6, 200);
    vm.last = {140, 6};
    *(regs + 0) = *(locals + 20); // sorted1
    *(regs + 1) = *(locals + 22); // sorted3
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 140, 6, 201);
    vm.last = {142, 6};
    *(regs + 0) = *(locals + 20); // sorted1
    *(regs + 1) = Value(1);
    U_BCALLRET2(vm, regs + 2, 23, 1); // binary_search
    vm.temp_lval = locals + 24; // findex
    *(vm.temp_lval) = *(regs + 1);
    vm.temp_lval = locals + 23; // found
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {143, 6};
    *(regs + 0) = *(locals + 23); // found
    *(regs + 1) = Value(2);
    U_IEQ(vm, regs + 2);
    if (!(regs + 0)->True()) goto block9;
    U_POP(vm, regs + 1);
    *(regs + 0) = *(locals + 24); // findex
    *(regs + 1) = Value(0);
    U_IEQ(vm, regs + 2);
    block9:;
    U_ASSERT(vm, regs + 1, 143, 6, 202);
    vm.last = {144, 6};
    *(regs + 0) = *(locals + 20); // sorted1
    *(regs + 1) = Value(9);
    U_BCALLRET2(vm, regs + 2, 23, 1); // binary_search
    vm.temp_lval = locals + 24; // findex
    *(vm.temp_lval) = *(regs + 1);
    vm.temp_lval = locals + 23; // found
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {145, 6};
    *(regs + 0) = *(locals + 23); // found
    *(regs + 1) = Value(2);
    U_IEQ(vm, regs + 2);
    if (!(regs + 0)->True()) goto block10;
    U_POP(vm, regs + 1);
    *(regs + 0) = *(locals + 24); // findex
    *(regs + 1) = Value(8);
    U_IEQ(vm, regs + 2);
    block10:;
    U_ASSERT(vm, regs + 1, 145, 6, 203);
    vm.last = {146, 6};
    *(regs + 0) = *(locals + 20); // sorted1
    *(regs + 1) = Value(3);
    U_BCALLRET2(vm, regs + 2, 23, 1); // binary_search
    vm.temp_lval = locals + 24; // findex
    *(vm.temp_lval) = *(regs + 1);
    vm.temp_lval = locals + 23; // found
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {147, 6};
    *(regs + 0) = *(locals + 23); // found
    *(regs + 1) = Value(2);
    U_IEQ(vm, regs + 2);
    if (!(regs + 0)->True()) goto block11;
    U_POP(vm, regs + 1);
    *(regs + 0) = *(locals + 24); // findex
    *(regs + 1) = Value(2);
    U_IEQ(vm, regs + 2);
    block11:;
    U_ASSERT(vm, regs + 1, 147, 6, 204);
    vm.last = {149, 6};
    *(regs + 0) = Value(44);
    *(regs + 1) = *(locals + 19); // testvector
    fun_29(vm, regs + 2); // call: sum
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 149, 6, 205);
    vm.last = {150, 6};
    *(regs + 0) = Value(264);
    *(regs + 1) = *(locals + 19); // testvector
    U_PUSHFUN(vm, regs + 2, 0, fun_167);
    fun_471(vm, regs + 3); // call: map
    fun_472(vm, regs + 2); // call: sum
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 150, 6, 206);
    vm.last = {153, 6};
    vm.last = {154, 6};
    *(regs + 0) = Value(7);
    fun_168(vm, regs + 1); // call: factorial
    *(regs + 1) = Value(5040);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 154, 6, 207);
    vm.last = {156, 6};
    U_PUSHFUN(vm, regs + 0, 0, fun_169);
    vm.temp_lval = locals + 25; // applyto4
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {157, 6};
    *(regs + 0) = Value(16);
    U_PUSHFUN(vm, regs + 1, 0, fun_170);
    fun_169(vm, regs + 2); // call: function162
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 157, 6, 208);
    vm.last = {159, 6};
    U_PUSHSTR(vm, regs + 0, 209); // "abc"
    (regs + 0)->LTINCRTNIL();
    *(regs + 1) = Value(3);
    *(regs + 2) = Value(122);
    *(regs + 3) = Value(0);
    U_BCALLRET4(vm, regs + 4, 198, 1); // ensure_size
    *(keepvar + 7) = *(regs + 0);
    U_PUSHSTR(vm, regs + 1, 210); // "abc"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 159, 6, 211);
    vm.last = {160, 6};
    U_PUSHSTR(vm, regs + 0, 212); // "abc"
    (regs + 0)->LTINCRTNIL();
    *(regs + 1) = Value(5);
    *(regs + 2) = Value(122);
    *(regs + 3) = Value(0);
    U_BCALLRET4(vm, regs + 4, 198, 1); // ensure_size
    *(keepvar + 8) = *(regs + 0);
    U_PUSHSTR(vm, regs + 1, 213); // "abczz"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 160, 6, 214);
    vm.last = {161, 6};
    U_PUSHSTR(vm, regs + 0, 215); // "abc"
    (regs + 0)->LTINCRTNIL();
    *(regs + 1) = Value(-5);
    *(regs + 2) = Value(122);
    *(regs + 3) = Value(0);
    U_BCALLRET4(vm, regs + 4, 198, 1); // ensure_size
    *(keepvar + 9) = *(regs + 0);
    U_PUSHSTR(vm, regs + 1, 216); // "zzabc"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 161, 6, 217);
    vm.last = {162, 6};
    U_PUSHSTR(vm, regs + 0, 218); // ""
    (regs + 0)->LTINCRTNIL();
    *(regs + 1) = Value(0);
    *(regs + 2) = Value(1027);
    U_BCALLRET3(vm, regs + 3, 201, 1); // write_int16_le
    vm.temp_lval = locals + 27; // ni
    *(vm.temp_lval) = *(regs + 1);
    vm.temp_lval = locals + 26; // ns
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {163, 6};
    *(regs + 0) = *(locals + 26); // ns
    U_PUSHSTR(vm, regs + 1, 219); // "\003\004\000\000"
    U_SEQ(vm, regs + 2);
    if (!(regs + 0)->True()) goto block12;
    U_POP(vm, regs + 1);
    *(regs + 0) = *(locals + 27); // ni
    *(regs + 1) = Value(2);
    U_IEQ(vm, regs + 2);
    block12:;
    U_ASSERT(vm, regs + 1, 163, 6, 220);
    vm.last = {164, 6};
    *(regs + 0) = *(locals + 26); // ns
    *(regs + 1) = Value(0);
    U_BCALLRET2(vm, regs + 2, 215, 1); // read_int32_le
    U_POP(vm, regs + 2);
    *(regs + 1) = Value(1027);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 164, 6, 221);
    vm.last = {165, 6};
    U_PUSHSTR(vm, regs + 0, 222); // ""
    (regs + 0)->LTINCRTNIL();
    *(regs + 1) = Value(0);
    *(regs + 2) = Value(2050);
    U_BCALLRET3(vm, regs + 3, 207, 1); // write_int16_le_back
    vm.temp_lval = locals + 27; // ni
    *(vm.temp_lval) = *(regs + 1);
    vm.temp_lval = locals + 26; // ns
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {166, 6};
    *(regs + 0) = *(locals + 26); // ns
    U_PUSHSTR(vm, regs + 1, 223); // "\000\000\002\010"
    U_SEQ(vm, regs + 2);
    if (!(regs + 0)->True()) goto block13;
    U_POP(vm, regs + 1);
    *(regs + 0) = *(locals + 27); // ni
    *(regs + 1) = Value(2);
    U_IEQ(vm, regs + 2);
    block13:;
    U_ASSERT(vm, regs + 1, 166, 6, 224);
    vm.last = {167, 6};
    *(regs + 0) = *(locals + 26); // ns
    *(regs + 1) = Value(0);
    U_BCALLRET2(vm, regs + 2, 226, 1); // read_int16_le_back
    U_POP(vm, regs + 2);
    *(regs + 1) = Value(2050);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 167, 6, 225);
    vm.last = {169, 6};
    U_PUSHSTR(vm, regs + 0, 226); // "a"
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 28; // s1
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {170, 6};
    U_PUSHSTR(vm, regs + 0, 227); // "b"
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 29; // s2
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {171, 6};
    *(regs + 0) = *(locals + 28); // s1
    if (!(regs + 0)->True()) goto block14;
    U_POP(vm, regs + 1);
    *(regs + 0) = *(locals + 29); // s2
    block14:;
    *(regs + 1) = *(locals + 29); // s2
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 171, 6, 228);
    vm.last = {172, 6};
    *(regs + 0) = *(locals + 29); // s2
    *(regs + 1) = *(locals + 28); // s1
    (regs + 1)->LTINCRTNIL();
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 29; // s2
    U_LV_WRITEREF(vm, regs + 2);
    vm.temp_lval = locals + 28; // s1
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {174, 6};
    *(regs + 0) = Value(5);
    vm.temp_lval = locals + 30; // aa
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {175, 6};
    *(regs + 0) = Value(2);
    vm.temp_lval = locals + 30; // aa
    U_LV_IMOD(vm, regs + 1);
    vm.last = {176, 6};
    *(regs + 0) = *(locals + 30); // aa
    *(regs + 1) = Value(1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 176, 6, 229);
    vm.last = {177, 6};
    *(regs + 0) = Value(-6.000000);
    *(regs + 1) = Value(0.000000);
    *(regs + 2) = Value(6.000000);
    *(regs + 3) = Value(3);
    U_BCALLRETV(vm, regs + 4, 119, 1); // sign
    *(regs + 3) = Value(-1);
    *(regs + 4) = Value(0);
    *(regs + 5) = Value(1);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 177, 6, 230);
    vm.last = {180, 6};
    *(regs + 0) = Value(1);
    *(regs + 1) = Value(1);
    *(regs + 2) = Value(1);
    *(regs + 3) = Value(1);
    vm.temp_lval = locals + 34; // ee
    *(vm.temp_lval) = *(regs + 3);
    vm.temp_lval = locals + 33; // dd
    *(vm.temp_lval) = *(regs + 2);
    vm.temp_lval = locals + 32; // cc
    *(vm.temp_lval) = *(regs + 1);
    vm.temp_lval = locals + 31; // bb
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {181, 6};
    *(regs + 0) = *(locals + 31); // bb
    *(regs + 1) = Value(1);
    U_IEQ(vm, regs + 2);
    if (!(regs + 0)->True()) goto block15;
    U_POP(vm, regs + 1);
    *(regs + 0) = *(locals + 31); // bb
    *(regs + 1) = *(locals + 32); // cc
    U_IEQ(vm, regs + 2);
    block15:;
    if (!(regs + 0)->True()) goto block16;
    U_POP(vm, regs + 1);
    *(regs + 0) = *(locals + 33); // dd
    *(regs + 1) = *(locals + 34); // ee
    U_IEQ(vm, regs + 2);
    block16:;
    U_ASSERT(vm, regs + 1, 181, 6, 231);
    vm.last = {182, 6};
    *(regs + 0) = Value(2);
    *(regs + 1) = Value(3);
    vm.temp_lval = locals + 36; // gg
    *(vm.temp_lval) = *(regs + 1);
    vm.temp_lval = locals + 35; // ff
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {183, 6};
    *(regs + 0) = *(locals + 35); // ff
    *(regs + 1) = Value(2);
    U_IEQ(vm, regs + 2);
    if (!(regs + 0)->True()) goto block17;
    U_POP(vm, regs + 1);
    *(regs + 0) = *(locals + 36); // gg
    *(regs + 1) = Value(3);
    U_IEQ(vm, regs + 2);
    block17:;
    U_ASSERT(vm, regs + 1, 183, 6, 232);
    vm.last = {184, 6};
    *(regs + 0) = Value(8);
    *(regs + 1) = Value(9);
    vm.temp_lval = locals + 36; // gg
    *(vm.temp_lval) = *(regs + 1);
    vm.temp_lval = locals + 35; // ff
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {185, 6};
    *(regs + 0) = *(locals + 35); // ff
    *(regs + 1) = Value(8);
    U_IEQ(vm, regs + 2);
    if (!(regs + 0)->True()) goto block18;
    U_POP(vm, regs + 1);
    *(regs + 0) = *(locals + 36); // gg
    *(regs + 1) = Value(9);
    U_IEQ(vm, regs + 2);
    block18:;
    U_ASSERT(vm, regs + 1, 185, 6, 233);
    vm.last = {186, 6};
    vm.last = {189, 6};
    fun_171(vm, regs + 0); // call: multiret
    vm.temp_lval = locals + 38; // ii
    *(vm.temp_lval) = *(regs + 1);
    vm.temp_lval = locals + 37; // hh
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {190, 6};
    *(regs + 0) = *(locals + 37); // hh
    *(regs + 1) = Value(6);
    U_IEQ(vm, regs + 2);
    if (!(regs + 0)->True()) goto block19;
    U_POP(vm, regs + 1);
    *(regs + 0) = *(locals + 38); // ii
    *(regs + 1) = Value(7);
    U_IEQ(vm, regs + 2);
    block19:;
    U_ASSERT(vm, regs + 1, 190, 6, 234);
    vm.last = {191, 6};
    fun_473(vm, regs + 0); // call: multiret
    vm.temp_lval = locals + 39; // ll
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {192, 6};
    *(regs + 0) = *(locals + 39); // ll
    *(regs + 1) = Value(6);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 192, 6, 235);
    vm.last = {193, 6};
    vm.last = {194, 6};
    fun_172(vm, regs + 0); // call: multiretpassthru
    vm.temp_lval = locals + 41; // kk
    *(vm.temp_lval) = *(regs + 1);
    vm.temp_lval = locals + 40; // jj
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {195, 6};
    *(regs + 0) = *(locals + 40); // jj
    *(regs + 1) = Value(6);
    U_IEQ(vm, regs + 2);
    if (!(regs + 0)->True()) goto block20;
    U_POP(vm, regs + 1);
    *(regs + 0) = *(locals + 41); // kk
    *(regs + 1) = Value(7);
    U_IEQ(vm, regs + 2);
    block20:;
    U_ASSERT(vm, regs + 1, 195, 6, 236);
    vm.last = {196, 6};
    vm.last = {197, 6};
    fun_173(vm, regs + 0); // call: multirettyped
    vm.temp_lval = locals + 43; // mm
    U_LV_WRITEREF(vm, regs + 2);
    vm.temp_lval = locals + 42; // nn
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {198, 6};
    *(regs + 0) = *(locals + 42); // nn
    *(regs + 1) = Value(1.000000);
    U_FEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 198, 6, 237);
    vm.last = {199, 6};
    *(regs + 0) = *(locals + 43); // mm
    U_PUSHSTR(vm, regs + 1, 238); // "a"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 199, 6, 239);
    vm.last = {203, 6};
    vm.last = {200, 6};
    vm.last = {200, 6};
    vm.last = {205, 6};
    U_PUSHSTR(vm, regs + 0, 240); // ""
    (regs + 0)->LTINCRTNIL();
    *(regs + 1) = Value(0);
    U_NEWOBJECT(vm, regs + 2, 619); // mretfields
    fun_174(vm, regs + 1); // call: write
    vm.last = {208, 6};
    vm.last = {212, 6};
    *(regs + 0) = Value(5);
    fun_175(vm, regs + 1); // call: union_return
    *(keepvar + 10) = *(regs + 0);
    U_ASSERT(vm, regs + 1, 212, 6, 241);
    vm.last = {215, 6};
    vm.last = {218, 6};
    vm.last = {220, 6};
    fun_176(vm, regs + 0); // call: nlr1
    *(regs + 1) = Value(1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 220, 6, 242);
    vm.last = {221, 6};
    fun_474(vm, regs + 0); // call: nlr1
    vm.last = {223, 6};
    U_PUSHFUN(vm, regs + 0, 0, fun_178);
    fun_476(vm, regs + 1); // call: do
    vm.last = {228, 6};
    vm.last = {235, 6};
    U_PUSHFUN(vm, regs + 0, 0, fun_182);
    fun_134(vm, regs + 1); // call: try
    vm.last = {238, 6};
    *(regs + 0) = Value(-1);
    *(regs + 1) = Value(10);
    block21:;
    if (!U_IFOR(vm, regs + 2)) goto block22;
    vm.last = {239, 6};
    *(regs + 2) = Value(7);
    fun_480(vm, regs + 3); // call: factorial
    vm.last = {240, 6};
    *(regs + 2) = *(locals + 19); // testvector
    *(regs + 3) = Value(0);
    U_PUSHFUN(vm, regs + 4, 0, fun_183);
    fun_481(vm, regs + 5); // call: fold
    goto block21;
    block22:;
    vm.last = {242, 6};
    *(regs + 0) = *(locals + 19); // testvector
    *(regs + 1) = Value(1);
    U_BCALLRETV(vm, regs + 2, 22, 0); // truncate
    vm.last = {243, 6};
    *(regs + 0) = *(locals + 19); // testvector
    *(regs + 1) = Value(3);
    U_NEWVEC(vm, regs + 2, 7, 1);
    *(keepvar + 11) = *(regs + 1);
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 243, 6, 243);
    vm.last = {245, 6};
    *(regs + 0) = Value(0);
    *(regs + 1) = Value(1);
    *(regs + 2) = Value(0);
    U_BCALLRET3(vm, regs + 3, 106, 1); // in_range
    U_ASSERT(vm, regs + 1, 245, 6, 244);
    vm.last = {246, 6};
    *(regs + 0) = Value(0);
    *(regs + 1) = Value(1);
    *(regs + 2) = Value(0);
    U_BCALLRET3(vm, regs + 3, 106, 1); // in_range
    U_ASSERT(vm, regs + 1, 246, 6, 245);
    vm.last = {249, 6};
    *(regs + 0) = Value(10);
    *(regs + 1) = Value(20);
    *(regs + 2) = Value(3);
    *(regs + 3) = Value(6);
    U_IVVMOD(vm, regs + 4, 2);
    *(regs + 2) = Value(1);
    *(regs + 3) = Value(2);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 249, 6, 246);
    vm.last = {250, 6};
    *(regs + 0) = Value(10);
    *(regs + 1) = Value(20);
    *(regs + 2) = Value(3);
    U_IVSMOD(vm, regs + 3, 2);
    *(regs + 2) = Value(1);
    *(regs + 3) = Value(2);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 250, 6, 247);
    vm.last = {251, 6};
    *(regs + 0) = Value(4.000000);
    *(regs + 1) = Value(1.500000);
    U_FMOD(vm, regs + 2);
    *(regs + 1) = Value(1.000000);
    U_FSUB(vm, regs + 2);
    U_BCALLRET1(vm, regs + 1, 113, 1); // abs
    U_PUSHFLT64(vm, regs + 1, 1202590843, 1065646817); // 0.01
    U_FLT(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 251, 6, 248);
    vm.last = {252, 6};
    U_PUSHFLT64(vm, regs + 0, 858993459, 1076114227); // 10.1
    U_PUSHFLT64(vm, regs + 1, -1717986918, 1077156249); // 20.1
    *(regs + 2) = Value(3.000000);
    *(regs + 3) = Value(6.000000);
    U_FVVMOD(vm, regs + 4, 2);
    U_PUSHFLT64(vm, regs + 2, -1717986918, 1072798105); // 1.1
    U_PUSHFLT64(vm, regs + 3, -858993459, 1073794252); // 2.1
    U_FVVSUB(vm, regs + 4, 2);
    *(regs + 2) = Value(2);
    U_BCALLRETV(vm, regs + 3, 115, 1); // abs
    *(regs + 2) = Value(2);
    U_BCALLRETV(vm, regs + 3, 133, 1); // max
    U_PUSHFLT64(vm, regs + 1, 1202590843, 1065646817); // 0.01
    U_FLT(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 252, 6, 249);
    vm.last = {253, 6};
    U_PUSHFLT64(vm, regs + 0, 858993459, 1076114227); // 10.1
    U_PUSHFLT64(vm, regs + 1, -1717986918, 1077156249); // 20.1
    *(regs + 2) = Value(3.000000);
    U_FVSMOD(vm, regs + 3, 2);
    U_PUSHFLT64(vm, regs + 2, -1717986918, 1072798105); // 1.1
    U_PUSHFLT64(vm, regs + 3, -858993459, 1073794252); // 2.1
    U_FVVSUB(vm, regs + 4, 2);
    *(regs + 2) = Value(2);
    U_BCALLRETV(vm, regs + 3, 115, 1); // abs
    *(regs + 2) = Value(2);
    U_BCALLRETV(vm, regs + 3, 133, 1); // max
    U_PUSHFLT64(vm, regs + 1, 1202590843, 1065646817); // 0.01
    U_FLT(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 253, 6, 250);
    vm.last = {256, 6};
    U_PUSHSTR(vm, regs + 0, 251); // "\342\272\206\343\213\207\344\254\210"
    U_PUSHSTR(vm, regs + 1, 252); // "\342\272\206\343\213\207\344\254\210"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 256, 6, 253);
    vm.last = {259, 6};
    *(regs + 0) = Value(1);
    vm.temp_lval = locals + 44; // bit
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {260, 6};
    *(regs + 0) = Value(2);
    vm.temp_lval = locals + 44; // bit
    U_LV_BINOR(vm, regs + 1);
    U_LV_DUP(vm, regs + 0);
    *(regs + 1) = Value(3);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 260, 6, 254);
    vm.last = {261, 6};
    *(regs + 0) = Value(-3);
    vm.temp_lval = locals + 44; // bit
    U_LV_BINAND(vm, regs + 1);
    U_LV_DUP(vm, regs + 0);
    *(regs + 1) = Value(1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 261, 6, 255);
    vm.last = {262, 6};
    *(regs + 0) = Value(3);
    vm.temp_lval = locals + 44; // bit
    U_LV_XOR(vm, regs + 1);
    U_LV_DUP(vm, regs + 0);
    *(regs + 1) = Value(2);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 262, 6, 256);
    vm.last = {263, 6};
    *(regs + 0) = Value(1);
    vm.temp_lval = locals + 44; // bit
    U_LV_ASL(vm, regs + 1);
    U_LV_DUP(vm, regs + 0);
    *(regs + 1) = Value(4);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 263, 6, 257);
    vm.last = {264, 6};
    *(regs + 0) = Value(1);
    vm.temp_lval = locals + 44; // bit
    U_LV_ASR(vm, regs + 1);
    U_LV_DUP(vm, regs + 0);
    *(regs + 1) = Value(2);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 264, 6, 258);
    vm.last = {267, 6};
    *(regs + 0) = Value(17);
    fun_151(vm, regs + 1); // call: dictionary
    vm.temp_lval = locals + 45; // dict
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {269, 6};
    *(regs + 0) = *(locals + 45); // dict
    *(regs + 1) = Value(1.000000);
    *(regs + 2) = Value(-1.000000);
    fun_145(vm, regs + 3); // call: get
    *(regs + 1) = Value(-1.000000);
    U_FEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 269, 6, 259);
    vm.last = {270, 6};
    *(regs + 0) = Value(0);
    U_BCALLRET1(vm, regs + 1, 97, 0); // rnd_seed
    vm.last = {271, 6};
    *(regs + 0) = Value(-1);
    *(regs + 1) = Value(32);
    block23:;
    if (!U_IFOR(vm, regs + 2)) goto block24;
    vm.last = {272, 6};
    U_BCALLRET0(vm, regs + 2, 95, 1); // rnd_float
    vm.temp_lval = locals + 46; // key
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {273, 6};
    *(regs + 2) = *(locals + 45); // dict
    *(regs + 3) = *(locals + 46); // key
    *(regs + 4) = *(locals + 46); // key
    fun_139(vm, regs + 5); // call: set
    goto block23;
    block24:;
    vm.last = {274, 6};
    *(regs + 0) = Value(0);
    U_BCALLRET1(vm, regs + 1, 97, 0); // rnd_seed
    vm.last = {275, 6};
    *(regs + 0) = Value(-1);
    *(regs + 1) = Value(32);
    block25:;
    if (!U_IFOR(vm, regs + 2)) goto block26;
    vm.last = {276, 6};
    U_BCALLRET0(vm, regs + 2, 95, 1); // rnd_float
    vm.temp_lval = locals + 47; // key
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {277, 6};
    *(regs + 2) = *(locals + 45); // dict
    *(regs + 3) = *(locals + 47); // key
    *(regs + 4) = Value(-1.000000);
    fun_145(vm, regs + 5); // call: get
    vm.temp_lval = locals + 48; // value
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {278, 6};
    *(regs + 2) = *(locals + 48); // value
    *(regs + 3) = *(locals + 47); // key
    U_FEQ(vm, regs + 4);
    U_ASSERT(vm, regs + 3, 278, 6, 260);
    goto block25;
    block26:;
    vm.last = {280, 6};
    *(regs + 0) = Value(17);
    fun_484(vm, regs + 1); // call: dictionary
    vm.temp_lval = locals + 49; // dict2
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {281, 6};
    *(regs + 0) = *(locals + 49); // dict2
    U_PUSHSTR(vm, regs + 1, 261); // "a"
    U_PUSHSTR(vm, regs + 2, 262); // "b"
    fun_487(vm, regs + 3); // call: set
    vm.last = {282, 6};
    *(regs + 0) = *(locals + 49); // dict2
    U_PUSHSTR(vm, regs + 1, 263); // "a"
    U_PUSHNIL(vm, regs + 2);
    fun_489(vm, regs + 3); // call: get
    vm.temp_lval = locals + 50; // val
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {283, 6};
    *(regs + 0) = *(locals + 50); // val
    U_E2B(vm, regs + 1);
    if (!(regs + 0)->True()) goto block27;
    U_POP(vm, regs + 1);
    *(regs + 0) = *(locals + 50); // val
    U_PUSHSTR(vm, regs + 1, 264); // "b"
    U_SEQ(vm, regs + 2);
    block27:;
    U_ASSERT(vm, regs + 1, 283, 6, 265);
    vm.last = {285, 6};
    *(regs + 0) = Value(17);
    fun_491(vm, regs + 1); // call: dictionary
    vm.temp_lval = locals + 51; // dict3
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {286, 6};
    *(regs + 0) = *(locals + 51); // dict3
    *(regs + 1) = Value(1);
    *(regs + 2) = Value(2);
    *(regs + 3) = Value(3);
    *(regs + 4) = Value(4);
    *(regs + 5) = Value(5);
    *(regs + 6) = Value(6);
    fun_494(vm, regs + 7); // call: set
    vm.last = {287, 6};
    *(regs + 0) = *(locals + 51); // dict3
    *(regs + 1) = Value(1);
    *(regs + 2) = Value(2);
    *(regs + 3) = Value(3);
    U_PUSHVARVF(vm, regs + 4, 131, 3); // int3_0+0
    fun_496(vm, regs + 7); // call: get
    *(regs + 3) = Value(4);
    *(regs + 4) = Value(5);
    *(regs + 5) = Value(6);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 287, 6, 266);
    vm.last = {290, 6};
    *(regs + 0) = Value(0);
    vm.temp_lval = locals + 52; // lc
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {291, 6};
    *(regs + 0) = Value(-1);
    *(regs + 1) = Value(10);
    block28:;
    if (!U_IFOR(vm, regs + 2)) goto block29;
    vm.last = {291, 6};
    U_IFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 53; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {292, 6};
    *(regs + 2) = *(locals + 53); // i
    *(regs + 3) = Value(5);
    U_IEQ(vm, regs + 4);
    if (!(regs + 2)->True()) goto block30;
    vm.last = {292, 6};
    U_POP(vm, regs + 2);
    U_POP(vm, regs + 1);
    goto block31;
    block30:;
    vm.last = {293, 6};
    vm.temp_lval = locals + 52; // lc
    U_LV_IPP(vm, regs + 2);
    goto block28;
    block29:;
    block31:;
    vm.last = {294, 6};
    *(regs + 0) = *(locals + 52); // lc
    *(regs + 1) = Value(5);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 294, 6, 267);
    vm.last = {295, 6};
    block32:;
    *(regs + 0) = *(locals + 52); // lc
    if (!(regs + 0)->True()) goto block33;
    vm.last = {296, 6};
    *(regs + 0) = *(locals + 52); // lc
    *(regs + 1) = Value(1);
    U_IEQ(vm, regs + 2);
    if (!(regs + 0)->True()) goto block34;
    vm.last = {296, 6};
    goto block35;
    block34:;
    vm.last = {297, 6};
    vm.temp_lval = locals + 52; // lc
    U_LV_IMM(vm, regs + 0);
    goto block32;
    block33:;
    block35:;
    vm.last = {298, 6};
    *(regs + 0) = *(locals + 52); // lc
    *(regs + 1) = Value(1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 298, 6, 268);
    vm.last = {301, 6};
    vm.last = {310, 6};
    fun_184(vm, regs + 0); // call: outer
    vm.last = {314, 6};
    vm.last = {324, 6};
    *(regs + 0) = Value(0);
    fun_186(vm, regs + 1); // call: outer_dest2
    *(regs + 2) = Value(1);
    *(regs + 3) = Value(0);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 324, 6, 269);
    vm.last = {325, 6};
    *(regs + 0) = Value(1);
    fun_186(vm, regs + 1); // call: outer_dest2
    *(regs + 2) = Value(4);
    *(regs + 3) = Value(5);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 325, 6, 270);
    vm.last = {326, 6};
    *(regs + 0) = Value(2);
    fun_186(vm, regs + 1); // call: outer_dest2
    *(regs + 2) = Value(3);
    *(regs + 3) = Value(0);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 326, 6, 271);
    vm.last = {329, 6};
    vm.last = {332, 6};
    vm.last = {338, 6};
    *(regs + 0) = Value(1);
    vm.temp_lval = locals + 54; // dav
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {339, 6};
    vm.last = {340, 6};
    vm.last = {341, 6};
    vm.last = {342, 6};
    vm.last = {342, 6};
    *(regs + 0) = Value(1);
    vm.temp_lval = locals + 66; // a
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {341, 6};
    *(regs + 0) = *(locals + 66); // a
    *(regs + 1) = Value(1);
    *(regs + 2) = Value(2);
    *(regs + 3) = *(locals + 54); // dav
    *(regs + 4) = Value(1);
    U_IADD(vm, regs + 5);
    fun_190(vm, regs + 4); // call: da
    U_IADD(vm, regs + 2);
    *(regs + 1) = Value(1);
    *(regs + 2) = Value(2);
    *(regs + 3) = Value(3);
    fun_190(vm, regs + 4); // call: da
    U_IADD(vm, regs + 2);
    *(regs + 1) = Value(12);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 342, 6, 272);
    vm.last = {345, 6};
    U_PUSHSTR(vm, regs + 0, 273); // "abcdabcd"
    U_PUSHSTR(vm, regs + 1, 274); // "BC"
    *(regs + 2) = Value(0);
    U_BCALLRET3(vm, regs + 3, 37, 1); // find_string
    *(regs + 1) = Value(-1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 345, 6, 275);
    vm.last = {346, 6};
    U_PUSHSTR(vm, regs + 0, 276); // "abcdabcd"
    U_PUSHSTR(vm, regs + 1, 277); // "bc"
    *(regs + 2) = Value(0);
    U_BCALLRET3(vm, regs + 3, 37, 1); // find_string
    *(regs + 1) = Value(1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 346, 6, 278);
    vm.last = {347, 6};
    U_PUSHSTR(vm, regs + 0, 279); // "abcdabcd"
    U_PUSHSTR(vm, regs + 1, 280); // "bc"
    *(regs + 2) = Value(4);
    U_BCALLRET3(vm, regs + 3, 37, 1); // find_string
    *(regs + 1) = Value(5);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 347, 6, 281);
    vm.last = {348, 6};
    U_PUSHSTR(vm, regs + 0, 282); // "abcdabcd"
    U_PUSHSTR(vm, regs + 1, 283); // "bc"
    *(regs + 2) = Value(0);
    U_BCALLRET3(vm, regs + 3, 38, 1); // find_string_reverse
    *(regs + 1) = Value(5);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 348, 6, 284);
    vm.last = {349, 6};
    U_PUSHSTR(vm, regs + 0, 285); // "abcdabcd"
    U_PUSHSTR(vm, regs + 1, 286); // "bc"
    *(regs + 2) = Value(4);
    U_BCALLRET3(vm, regs + 3, 38, 1); // find_string_reverse
    *(regs + 1) = Value(1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 349, 6, 287);
    vm.last = {350, 6};
    U_PUSHSTR(vm, regs + 0, 288); // "abcdabcd"
    U_PUSHSTR(vm, regs + 1, 289); // "bc"
    U_PUSHSTR(vm, regs + 2, 290); // "X"
    *(regs + 3) = Value(0);
    U_BCALLRET4(vm, regs + 4, 39, 1); // replace_string
    *(keepvar + 12) = *(regs + 0);
    U_PUSHSTR(vm, regs + 1, 291); // "aXdaXd"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 350, 6, 292);
    vm.last = {351, 6};
    U_PUSHSTR(vm, regs + 0, 293); // "abcdabcd"
    U_PUSHSTR(vm, regs + 1, 294); // "bc"
    U_PUSHSTR(vm, regs + 2, 295); // "X"
    *(regs + 3) = Value(1);
    U_BCALLRET4(vm, regs + 4, 39, 1); // replace_string
    *(keepvar + 13) = *(regs + 0);
    U_PUSHSTR(vm, regs + 1, 296); // "aXdabcd"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 351, 6, 297);
    vm.last = {353, 6};
    U_PUSHSTR(vm, regs + 0, 298); // " A | B C |"
    U_PUSHSTR(vm, regs + 1, 299); // "|"
    U_PUSHSTR(vm, regs + 2, 300); // " "
    *(regs + 3) = Value(1);
    U_BCALLRET4(vm, regs + 4, 42, 1); // tokenize
    *(keepvar + 14) = *(regs + 0);
    U_PUSHSTR(vm, regs + 1, 301); // "A"
    (regs + 1)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 2, 302); // "B C"
    (regs + 2)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 3, 303); // ""
    (regs + 3)->LTINCRTNIL();
    U_NEWVEC(vm, regs + 4, 11, 3);
    *(keepvar + 15) = *(regs + 1);
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 353, 6, 304);
    vm.last = {354, 6};
    U_PUSHSTR(vm, regs + 0, 305); // "; A ; B C;; "
    U_PUSHSTR(vm, regs + 1, 306); // ";"
    U_PUSHSTR(vm, regs + 2, 307); // " "
    *(regs + 3) = Value(0);
    U_BCALLRET4(vm, regs + 4, 42, 1); // tokenize
    *(keepvar + 16) = *(regs + 0);
    U_PUSHSTR(vm, regs + 1, 308); // ""
    (regs + 1)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 2, 309); // "A"
    (regs + 2)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 3, 310); // "B C"
    (regs + 3)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 4, 311); // ""
    (regs + 4)->LTINCRTNIL();
    U_NEWVEC(vm, regs + 5, 11, 4);
    *(keepvar + 17) = *(regs + 1);
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 354, 6, 312);
    vm.last = {355, 6};
    U_PUSHSTR(vm, regs + 0, 313); // ", A , B C,, "
    U_PUSHSTR(vm, regs + 1, 314); // ","
    U_PUSHSTR(vm, regs + 2, 315); // " "
    *(regs + 3) = Value(1);
    U_BCALLRET4(vm, regs + 4, 42, 1); // tokenize
    *(keepvar + 18) = *(regs + 0);
    U_PUSHSTR(vm, regs + 1, 316); // ""
    (regs + 1)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 2, 317); // "A"
    (regs + 2)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 3, 318); // "B C"
    (regs + 3)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 4, 319); // ""
    (regs + 4)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 5, 320); // ""
    (regs + 5)->LTINCRTNIL();
    U_NEWVEC(vm, regs + 6, 11, 5);
    *(keepvar + 19) = *(regs + 1);
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 355, 6, 321);
    vm.last = {358, 6};
    vm.last = {360, 6};
    vm.last = {364, 6};
    fun_193(vm, regs + 0); // call: G
    *(regs + 1) = Value(5);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 364, 6, 322);
    vm.last = {367, 6};
    U_PUSHSTR(vm, regs + 0, 323); // "a"
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 55; // slv
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {368, 6};
    U_PUSHSTR(vm, regs + 0, 324); // "b"
    vm.temp_lval = locals + 55; // slv
    U_LV_SADD(vm, regs + 1);
    U_LV_DUP(vm, regs + 0);
    U_PUSHSTR(vm, regs + 1, 325); // "ab"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 368, 6, 326);
    vm.last = {369, 6};
    *(regs + 0) = Value(1);
    U_NEWVEC(vm, regs + 1, 7, 1);
    vm.temp_lval = locals + 56; // vlv
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {370, 6};
    *(regs + 0) = Value(2);
    U_NEWVEC(vm, regs + 1, 7, 1);
    vm.temp_lval = locals + 56; // vlv
    U_LV_WRITEREF(vm, regs + 1);
    U_LV_DUP(vm, regs + 0);
    *(regs + 1) = Value(2);
    U_NEWVEC(vm, regs + 2, 7, 1);
    *(keepvar + 20) = *(regs + 1);
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 370, 6, 327);
    vm.last = {371, 6};
    vm.last = {371, 6};
    vm.last = {374, 6};
    U_PUSHSTR(vm, regs + 0, 328); // "a"
    (regs + 0)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 1, 329); // "a"
    (regs + 1)->LTINCRTNIL();
    U_NEWVEC(vm, regs + 2, 11, 1);
    vm.temp_lval = locals + 57; // stlv+0
    U_LV_WRITEREFV(vm, regs + 2, 2, 3);
    vm.last = {375, 6};
    U_PUSHSTR(vm, regs + 0, 330); // "b"
    (regs + 0)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 1, 331); // "b"
    (regs + 1)->LTINCRTNIL();
    U_NEWVEC(vm, regs + 2, 11, 1);
    vm.temp_lval = locals + 57; // stlv+0
    U_LV_WRITEREFV(vm, regs + 2, 2, 3);
    U_LV_DUPV(vm, regs + 0, 2);
    U_PUSHFLDV(vm, regs + 2, 1, 2);
    *(regs + 1) = Value(0);
    U_VPUSHIDXI(vm, regs + 2);
    U_PUSHSTR(vm, regs + 1, 332); // "b"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 375, 6, 333);
    vm.last = {378, 6};
    U_PUSHFLT64(vm, regs + 0, 0, 2146435072); // inf
    U_A2S(vm, regs + 1, 2);
    *(keepvar + 21) = *(regs + 0);
    U_PUSHSTR(vm, regs + 1, 334); // "inf"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 378, 6, 335);
    vm.last = {379, 6};
    *(regs + 0) = Value(1.000000);
    *(regs + 1) = Value(0.000000);
    U_FMOD(vm, regs + 2);
    U_A2S(vm, regs + 1, 2);
    *(keepvar + 22) = *(regs + 0);
    U_PUSHSTR(vm, regs + 1, 336); // "nan"
    *(regs + 2) = Value(0);
    U_BCALLRET3(vm, regs + 3, 37, 1); // find_string
    *(regs + 1) = Value(0);
    U_IGE(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 379, 6, 337);
    vm.last = {380, 6};
    U_PUSHVARVF(vm, regs + 0, 76, 2); // float2_1+0
    U_PUSHVARVF(vm, regs + 2, 74, 2); // float2_0+0
    U_FVVDIV(vm, regs + 4, 2);
    U_PUSHFLDV(vm, regs + 2, 1, 2);
    U_A2S(vm, regs + 1, 2);
    *(keepvar + 23) = *(regs + 0);
    U_PUSHSTR(vm, regs + 1, 338); // "inf"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 380, 6, 339);
    vm.last = {382, 6};
    vm.last = {382, 6};
    U_PUSHFUN(vm, regs + 0, 0, fun_195);
    vm.temp_lval = locals + 67; // f
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {253, 5};
    fun_195(vm, regs + 0); // call: function187
    vm.last = {389, 6};
    vm.last = {389, 6};
    U_PUSHFUN(vm, regs + 0, 0, fun_196);
    vm.temp_lval = locals + 68; // f
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {253, 5};
    fun_196(vm, regs + 0); // call: function188
    vm.last = {404, 6};
    vm.last = {404, 6};
    U_PUSHFUN(vm, regs + 0, 0, fun_197);
    vm.temp_lval = locals + 69; // f
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {253, 5};
    fun_197(vm, regs + 0); // call: function189
    vm.last = {413, 6};
    vm.last = {413, 6};
    U_PUSHFUN(vm, regs + 0, 0, fun_199);
    vm.temp_lval = locals + 70; // f
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {253, 5};
    fun_199(vm, regs + 0); // call: function191
    vm.last = {428, 6};
    vm.last = {428, 6};
    U_PUSHFUN(vm, regs + 0, 0, fun_201);
    vm.temp_lval = locals + 71; // f
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {253, 5};
    fun_201(vm, regs + 0); // call: function193
    DecVal(vm, locals[2]);
    DecVal(vm, locals[3]);
    DecVal(vm, locals[4]);
    DecVal(vm, locals[5]);
    DecVal(vm, locals[6]);
    DecVal(vm, locals[7]);
    DecVal(vm, locals[8]);
    DecVal(vm, locals[9]);
    DecVal(vm, locals[10]);
    DecVal(vm, locals[11]);
    DecVal(vm, locals[12]);
    DecVal(vm, locals[13]);
    DecVal(vm, locals[14]);
    DecVal(vm, locals[15]);
    DecVal(vm, locals[16]);
    DecVal(vm, locals[17]);
    DecVal(vm, locals[18]);
    DecVal(vm, locals[19]);
    DecVal(vm, locals[20]);
    DecVal(vm, locals[21]);
    DecVal(vm, locals[22]);
    DecVal(vm, locals[26]);
    DecVal(vm, locals[28]);
    DecVal(vm, locals[29]);
    DecVal(vm, locals[43]);
    DecVal(vm, locals[45]);
    DecVal(vm, locals[49]);
    DecVal(vm, locals[50]);
    DecVal(vm, locals[51]);
    DecVal(vm, locals[55]);
    DecVal(vm, locals[56]);
    DecVal(vm, locals[57]);
    DecVal(vm, locals[58]);
    DecVal(vm, locals[59]);
    DecVal(vm, locals[64]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    DecVal(vm, keepvar[1]);
    DecVal(vm, keepvar[2]);
    DecVal(vm, keepvar[3]);
    DecVal(vm, keepvar[4]);
    DecVal(vm, keepvar[5]);
    DecVal(vm, keepvar[6]);
    DecVal(vm, keepvar[7]);
    DecVal(vm, keepvar[8]);
    DecVal(vm, keepvar[9]);
    DecVal(vm, keepvar[10]);
    DecVal(vm, keepvar[11]);
    DecVal(vm, keepvar[12]);
    DecVal(vm, keepvar[13]);
    DecVal(vm, keepvar[14]);
    DecVal(vm, keepvar[15]);
    DecVal(vm, keepvar[16]);
    DecVal(vm, keepvar[17]);
    DecVal(vm, keepvar[18]);
    DecVal(vm, keepvar[19]);
    DecVal(vm, keepvar[20]);
    DecVal(vm, keepvar[21]);
    DecVal(vm, keepvar[22]);
    DecVal(vm, keepvar[23]);
    PopFunId(vm);
}

// tf
static void fun_159(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1290, locals);
    vm.last = {44, 6};
    *(regs + 0) = Value(3);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// tf
static void fun_160(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1295, locals);
    vm.last = {45, 6};
    *(regs + 0) = Value(4);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function155
static void fun_162(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value keepvar[1];
    Value locals[2];
    *(locals + 0) = *(psp - 1);
    *(locals + 1) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1300, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {107, 6};
    *(regs + 0) = *(locals + 0); // i
    switch (regs[0].ival()) {
    case 1:
    vm.last = {108, 6};
    U_PUSHSTR(vm, regs + 0, 340); // "no"
    (regs + 0)->LTINCRTNIL();
    goto block5;
    case 2:case 3:
    vm.last = {110, 6};
    *(regs + 0) = *(locals + 0); // i
    vm.temp_lval = locals + 1; // x
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {111, 6};
    U_PUSHSTR(vm, regs + 0, 341); // "yes"
    *(regs + 1) = *(locals + 1); // x
    U_A2S(vm, regs + 2, 0);
    *(keepvar + 0) = *(regs + 1);
    U_SADD(vm, regs + 2);
    goto block6;
    case 4:case 5:case 6:case 8:
    vm.last = {112, 6};
    U_PUSHSTR(vm, regs + 0, 342); // "maybe"
    (regs + 0)->LTINCRTNIL();
    goto block7;
    case 7:default:
    vm.last = {113, 6};
    U_PUSHSTR(vm, regs + 0, 343); // "what?"
    (regs + 0)->LTINCRTNIL();
    } // switch
    block5:;
    block6:;
    block7:;
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// function156
static void fun_163(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    *(locals + 0) = Value(0, lobster::V_NIL);
    *(locals + 1) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1305, locals);
    vm.last = {125, 6};
    *(regs + 0) = Value(0);
    vm.temp_lval = locals + 0; // a
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {126, 6};
    vm.temp_lval = locals + 0; // a
    U_LV_DUP(vm, regs + 0);
    U_LV_IPP(vm, regs + 1);
    vm.temp_lval = locals + 0; // a
    U_LV_IPP(vm, regs + 1);
    U_LV_DUP(vm, regs + 1);
    U_IADD(vm, regs + 2);
    vm.temp_lval = locals + 0; // a
    U_LV_DUP(vm, regs + 1);
    U_LV_IMM(vm, regs + 2);
    U_IADD(vm, regs + 2);
    vm.temp_lval = locals + 0; // a
    U_LV_IMM(vm, regs + 1);
    U_LV_DUP(vm, regs + 1);
    U_IADD(vm, regs + 2);
    *(regs + 1) = Value(4);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 126, 6, 344);
    vm.last = {127, 6};
    *(regs + 0) = Value(0);
    U_NEWVEC(vm, regs + 1, 7, 1);
    vm.temp_lval = locals + 1; // b
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {128, 6};
    *(regs + 0) = *(locals + 1); // b
    *(regs + 1) = Value(0);
    U_LVAL_IDXVI(vm, regs + 2, 0);
    U_LV_DUP(vm, regs + 0);
    U_LV_IPP(vm, regs + 1);
    *(regs + 1) = *(locals + 1); // b
    *(regs + 2) = Value(0);
    U_LVAL_IDXVI(vm, regs + 3, 0);
    U_LV_IPP(vm, regs + 1);
    U_LV_DUP(vm, regs + 1);
    U_IADD(vm, regs + 2);
    *(regs + 1) = *(locals + 1); // b
    *(regs + 2) = Value(0);
    U_LVAL_IDXVI(vm, regs + 3, 0);
    U_LV_DUP(vm, regs + 1);
    U_LV_IMM(vm, regs + 2);
    U_IADD(vm, regs + 2);
    *(regs + 1) = *(locals + 1); // b
    *(regs + 2) = Value(0);
    U_LVAL_IDXVI(vm, regs + 3, 0);
    U_LV_IMM(vm, regs + 1);
    U_LV_DUP(vm, regs + 1);
    U_IADD(vm, regs + 2);
    *(regs + 1) = Value(4);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 128, 6, 345);
    DecVal(vm, locals[1]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function157
static void fun_164(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1310, locals);
    vm.last = {132, 6};
    *(regs + 0) = *(locals + 0); // _a
    *(regs + 1) = *(locals + 1); // _b
    U_ILT(vm, regs + 2);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function158
static void fun_165(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1315, locals);
    vm.last = {134, 6};
    *(regs + 0) = *(locals + 0); // _a
    *(regs + 1) = *(locals + 1); // _b
    U_ILT(vm, regs + 2);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function159
static void fun_166(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1320, locals);
    vm.last = {136, 6};
    *(regs + 0) = *(locals + 0); // _a
    *(regs + 1) = *(locals + 1); // _b
    U_ILT(vm, regs + 2);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function160
static void fun_167(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1325, locals);
    vm.last = {150, 6};
    *(regs + 0) = *(locals + 0); // _
    *(regs + 1) = *(locals + 0); // _
    U_IMUL(vm, regs + 2);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// factorial
static void fun_480(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1329, locals);
    vm.last = {153, 6};
    *(regs + 0) = Value(1);
    *(regs + 1) = *(locals + 0); // n
    U_IGT(vm, regs + 2);
    if ((regs + 0)->True()) goto block0;
    *(regs + 0) = *(locals + 0); // n
    *(regs + 1) = Value(1);
    U_ISUB(vm, regs + 2);
    fun_168(vm, regs + 1); // call: factorial
    U_POP(vm, regs + 1);
    block0:;
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// factorial
static void fun_168(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1333, locals);
    vm.last = {153, 6};
    *(regs + 0) = Value(1);
    *(regs + 1) = *(locals + 0); // n
    U_IGT(vm, regs + 2);
    if ((regs + 0)->True()) goto block0;
    U_POP(vm, regs + 1);
    *(regs + 0) = *(locals + 0); // n
    *(regs + 1) = Value(1);
    U_ISUB(vm, regs + 2);
    fun_168(vm, regs + 1); // call: factorial
    *(regs + 1) = *(locals + 0); // n
    U_IMUL(vm, regs + 2);
    block0:;
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function162
static void fun_169(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1337, locals);
    vm.last = {156, 6};
    *(regs + 0) = Value(4);
    fun_170(vm, regs + 1); // call: function163
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function163
static void fun_170(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1341, locals);
    vm.last = {157, 6};
    *(regs + 0) = *(locals + 0); // x
    *(regs + 1) = *(locals + 0); // x
    U_IMUL(vm, regs + 2);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// multiret
static void fun_473(VMRef vm, StackPtr psp) {
    Value regs[1];
    PushFunId(vm, funinfo_table + 1345, 0);
    vm.last = {187, 6};
    *(regs + 0) = Value(0.000000);
    U_BCALLRET1(vm, regs + 1, 69, 1); // sin
    if (!(regs + 0)->True()) goto block0;
    vm.last = {187, 6};
    *(regs + 0) = Value(4);
    *(psp++) = *(regs + 0);
    goto epilogue;
    block0:;
    vm.last = {188, 6};
    *(regs + 0) = Value(6);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// multiret
static void fun_171(VMRef vm, StackPtr psp) {
    Value regs[2];
    PushFunId(vm, funinfo_table + 1348, 0);
    vm.last = {187, 6};
    *(regs + 0) = Value(0.000000);
    U_BCALLRET1(vm, regs + 1, 69, 1); // sin
    if (!(regs + 0)->True()) goto block0;
    vm.last = {187, 6};
    *(regs + 0) = Value(4);
    *(regs + 1) = Value(5);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    goto epilogue;
    block0:;
    vm.last = {188, 6};
    *(regs + 0) = Value(6);
    *(regs + 1) = Value(7);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// multiretpassthru
static void fun_172(VMRef vm, StackPtr psp) {
    Value regs[2];
    PushFunId(vm, funinfo_table + 1351, 0);
    vm.last = {193, 6};
    fun_171(vm, regs + 0); // call: multiret
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// multirettyped
static void fun_173(VMRef vm, StackPtr psp) {
    Value regs[2];
    PushFunId(vm, funinfo_table + 1354, 0);
    vm.last = {196, 6};
    *(regs + 0) = Value(1.000000);
    U_PUSHSTR(vm, regs + 1, 346); // "a"
    (regs + 1)->LTINCRTNIL();
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// write
static void fun_174(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1357, locals);
    vm.last = {204, 6};
    *(regs + 0) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 1, 0);
    (regs + 0)->LTINCRTNIL();
    *(regs + 1) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 2, 1);
    *(regs + 2) = Value(12345);
    U_BCALLRET3(vm, regs + 3, 201, 1); // write_int16_le
    *(regs + 2) = *(locals + 0); // this
    U_LVAL_FLD(vm, regs + 3, 1);
    *(vm.temp_lval) = *(regs + 1);
    *(regs + 1) = *(locals + 0); // this
    U_LVAL_FLD(vm, regs + 2, 0);
    U_LV_WRITEREF(vm, regs + 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// union_return
static void fun_175(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[2];
    *(locals + 0) = *(psp - 1);
    *(locals + 1) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1361, locals);
    vm.last = {209, 6};
    *(regs + 0) = Value(-1);
    *(regs + 1) = Value(10);
    block0:;
    if (!U_IFOR(vm, regs + 2)) goto block1;
    vm.last = {209, 6};
    U_IFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 1; // a
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {210, 6};
    *(regs + 2) = *(locals + 1); // a
    *(regs + 3) = *(locals + 0); // n
    U_IEQ(vm, regs + 4);
    if (!(regs + 2)->True()) goto block2;
    vm.last = {210, 6};
    U_POP(vm, regs + 2);
    U_POP(vm, regs + 1);
    U_PUSHSTR(vm, regs + 0, 347); // "foo"
    (regs + 0)->LTINCRTNIL();
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    block2:;
    goto block0;
    block1:;
    vm.last = {211, 6};
    U_PUSHNIL(vm, regs + 0);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// nlr1
static void fun_474(VMRef vm, StackPtr psp) {
    Value regs[1];
    (void)regs;
    PushFunId(vm, funinfo_table + 1366, 0);
    vm.last = {216, 6};
    fun_475(vm, regs + 0); // call: nlr2
    if (vm.ret_unwind_to == 170) goto block0;
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block0:;
    vm.last = {217, 6};
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// nlr1
static void fun_176(VMRef vm, StackPtr psp) {
    Value regs[1];
    PushFunId(vm, funinfo_table + 1369, 0);
    vm.last = {216, 6};
    fun_177(vm, regs + 0); // call: nlr2
    if (vm.ret_unwind_to == 170) goto block0;
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block0:;
    vm.last = {217, 6};
    *(regs + 0) = Value(2);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// nlr2
static void fun_475(VMRef vm, StackPtr psp) {
    Value regs[1];
    (void)regs;
    PushFunId(vm, funinfo_table + 1372, 0);
    vm.last = {219, 6};
    vm.ret_slots = 0;
    vm.ret_unwind_to = 169;
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// nlr2
static void fun_177(VMRef vm, StackPtr psp) {
    Value regs[1];
    PushFunId(vm, funinfo_table + 1375, 0);
    vm.last = {219, 6};
    *(regs + 0) = Value(1);
    vm.ret_slots = 1;
    vm.ret_unwind_to = 169;
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function171
static void fun_178(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[1];
    *(locals + 0) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1378, locals);
    vm.last = {224, 6};
    *(regs + 0) = Value(-1);
    *(regs + 1) = Value(2);
    block0:;
    if (!U_IFOR(vm, regs + 2)) goto block1;
    vm.last = {224, 6};
    U_IFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 0; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {225, 6};
    *(regs + 2) = *(locals + 0); // i
    if (!(regs + 2)->True()) goto block2;
    vm.last = {226, 6};
    U_POP(vm, regs + 2);
    U_POP(vm, regs + 1);
    vm.ret_slots = 0;
    vm.ret_unwind_to = 57;
    goto epilogue;
    block2:;
    goto block0;
    block1:;
    vm.ret_slots = 0;
    vm.ret_unwind_to = 171;
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// recursive_exceptions
static void fun_179(VMRef vm, StackPtr psp) {
    Value regs[2];
    SwapVars(vm, 374, psp, 1);
    PushFunId(vm, funinfo_table + 1382, 0);
    vm.last = {229, 6};
    U_PUSHFUN(vm, regs + 0, 0, fun_180);
    U_PUSHFUN(vm, regs + 1, 0, fun_181);
    fun_477(vm, regs + 2); // call: try
    if (vm.ret_unwind_to == 124) goto block0;
    psp = PopArg(vm, 374, psp);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block0:;
    vm.ret_slots = 0;
    vm.ret_unwind_to = 172;
    psp = PopArg(vm, 374, psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function173
static void fun_180(VMRef vm, StackPtr psp) {
    Value regs[2];
    PushFunId(vm, funinfo_table + 1386, 0);
    vm.last = {230, 6};
    U_PUSHVARF(vm, regs + 0, 374); // n
    if (!(regs + 0)->True()) goto block0;
    vm.last = {230, 6};
    U_PUSHVARF(vm, regs + 0, 374); // n
    *(regs + 1) = Value(1);
    U_ISUB(vm, regs + 2);
    fun_179(vm, regs + 1); // call: recursive_exceptions
    if (vm.ret_unwind_to == 172) goto block1;
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block1:;
    goto block2;
    block0:;
    vm.last = {231, 6};
    U_PUSHSTR(vm, regs + 0, 348); // ""
    fun_130(vm, regs + 1); // call: throw
    if (vm.ret_unwind_to == 126) goto block3;
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block3:;
    block2:;
    vm.ret_slots = 0;
    vm.ret_unwind_to = 173;
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function174
static void fun_181(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1389, locals);
    vm.last = {233, 6};
    *(regs + 0) = *(locals + 0); // v
    U_BCALLRET1(vm, regs + 1, 12, 1); // length
    U_PUSHVARF(vm, regs + 1, 374); // n
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 233, 6, 349);
    vm.last = {234, 6};
    *(regs + 0) = *(locals + 0); // v
    U_PUSHSTR(vm, regs + 1, 350); // "*"
    U_SADD(vm, regs + 2);
    fun_479(vm, regs + 1); // call: throw
    if (vm.ret_unwind_to == 126) goto block0;
    Pop(psp);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block0:;
    vm.ret_slots = 0;
    vm.ret_unwind_to = 174;
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function175
static void fun_182(VMRef vm, StackPtr psp) {
    Value regs[2];
    PushFunId(vm, funinfo_table + 1393, 0);
    vm.last = {236, 6};
    *(regs + 0) = Value(10);
    fun_179(vm, regs + 1); // call: recursive_exceptions
    if (vm.ret_unwind_to == 172) goto block0;
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block0:;
    vm.ret_slots = 0;
    vm.ret_unwind_to = 175;
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function176
static void fun_183(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1396, locals);
    vm.last = {240, 6};
    *(regs + 0) = *(locals + 0); // _x
    *(regs + 1) = *(locals + 1); // _y
    U_IADD(vm, regs + 2);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// outer
static void fun_184(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    *(locals + 0) = Value(0, lobster::V_NIL);
    *(locals + 1) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1401, locals);
    vm.last = {302, 6};
    vm.last = {305, 6};
    *(regs + 0) = Value(1);
    U_BCALLRET1(vm, regs + 1, 112, 1); // abs
    *(regs + 1) = Value(1);
    U_IEQ(vm, regs + 2);
    if (!(regs + 0)->True()) goto block0;
    vm.last = {305, 6};
    *(regs + 0) = Value(1);
    goto block1;
    block0:;
    vm.last = {305, 6};
    fun_185(vm, regs + 0); // call: error
    if (vm.ret_unwind_to == 178) goto block2;
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block2:;
    block1:;
    vm.temp_lval = locals + 0; // a
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {306, 6};
    *(regs + 0) = *(locals + 0); // a
    U_DUP(vm, regs + 1);
    *(regs + 2) = Value(1);
    U_IEQ(vm, regs + 3);
    if (!(regs + 1)->True()) goto block3;
    U_POP(vm, regs + 1);
    vm.last = {307, 6};
    *(regs + 0) = Value(2);
    goto block4;
    block3:;
    U_POP(vm, regs + 1);
    vm.last = {308, 6};
    fun_185(vm, regs + 0); // call: error
    if (vm.ret_unwind_to == 178) goto block5;
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block5:;
    block4:;
    vm.temp_lval = locals + 1; // b
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {309, 6};
    *(regs + 0) = *(locals + 0); // a
    *(regs + 1) = *(locals + 1); // b
    U_IADD(vm, regs + 2);
    *(regs + 1) = Value(3);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 309, 6, 351);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// error
static void fun_185(VMRef vm, StackPtr psp) {
    Value regs[1];
    PushFunId(vm, funinfo_table + 1406, 0);
    vm.last = {303, 6};
    *(regs + 0) = Value(0);
    U_ASSERT(vm, regs + 1, 303, 6, 352);
    vm.last = {304, 6};
    vm.ret_slots = 0;
    vm.ret_unwind_to = 177;
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// outer_dest2
static void fun_186(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value keepvar[1];
    SwapVars(vm, 390, psp, 1);
    PushFunId(vm, funinfo_table + 1409, 0);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {315, 6};
    vm.last = {323, 6};
    fun_187(vm, regs + 0); // call: outer_dest1
    if (vm.ret_unwind_to == 180) goto block0;
    psp = PopArg(vm, 390, psp);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block0:;
    *(keepvar + 0) = *(regs + 0);
    U_BCALLRET1(vm, regs + 1, 12, 1); // length
    *(regs + 1) = Value(0);
    psp = PopArg(vm, 390, psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// outer_dest1
static void fun_187(VMRef vm, StackPtr psp) {
    Value regs[2];
    PushFunId(vm, funinfo_table + 1413, 0);
    vm.last = {316, 6};
    vm.last = {322, 6};
    fun_188(vm, regs + 0); // call: return_past
    if (vm.ret_unwind_to == 181) goto block0;
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block0:;
    U_A2S(vm, regs + 1, 2);
    vm.ret_slots = 1;
    vm.ret_unwind_to = 180;
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// return_past
static void fun_188(VMRef vm, StackPtr psp) {
    Value regs[3];
    PushFunId(vm, funinfo_table + 1416, 0);
    vm.last = {317, 6};
    U_PUSHVARF(vm, regs + 0, 390); // mode
    U_DUP(vm, regs + 1);
    *(regs + 2) = Value(0);
    U_IEQ(vm, regs + 3);
    if (!(regs + 1)->True()) goto block0;
    U_POP(vm, regs + 1);
    vm.last = {318, 6};
    U_PUSHSTR(vm, regs + 0, 353); // "a"
    (regs + 0)->LTINCRTNIL();
    vm.ret_slots = 1;
    vm.ret_unwind_to = 180;
    *(psp++) = *(regs + 0);
    goto epilogue;
    goto block1;
    block0:;
    U_DUP(vm, regs + 1);
    *(regs + 2) = Value(1);
    U_IEQ(vm, regs + 3);
    if (!(regs + 1)->True()) goto block2;
    U_POP(vm, regs + 1);
    vm.last = {319, 6};
    *(regs + 0) = Value(4);
    *(regs + 1) = Value(5);
    vm.ret_slots = 2;
    vm.ret_unwind_to = 179;
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    goto epilogue;
    goto block3;
    block2:;
    U_POP(vm, regs + 1);
    vm.last = {320, 6};
    U_PUSHFLT64(vm, regs + 0, -858993459, 1074318540); // 3.1
    vm.ret_slots = 1;
    vm.ret_unwind_to = 181;
    *(psp++) = *(regs + 0);
    goto epilogue;
    block1:;
    block3:;
    vm.ret_slots = 0;
    vm.ret_unwind_to = 181;
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// da
static void fun_190(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[3];
    *(locals + 0) = *(psp - 3);
    *(locals + 1) = *(psp - 2);
    *(locals + 2) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1419, locals);
    vm.last = {339, 6};
    *(regs + 0) = *(locals + 0); // a
    *(regs + 1) = *(locals + 1); // b
    U_IADD(vm, regs + 2);
    *(regs + 1) = *(locals + 2); // c
    U_IADD(vm, regs + 2);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// G
static void fun_193(VMRef vm, StackPtr psp) {
    Value regs[2];
    PushFunId(vm, funinfo_table + 1425, 0);
    vm.last = {359, 6};
    fun_194(vm, regs + 0); // call: H
    if (vm.ret_unwind_to == 186) goto block0;
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block0:;
    *(regs + 1) = Value(1);
    U_IADD(vm, regs + 2);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// H
static void fun_194(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value keepvar[1];
    PushFunId(vm, funinfo_table + 1428, 0);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {361, 6};
    U_NEWVEC(vm, regs + 0, 1697, 0);
    *(keepvar + 0) = *(regs + 0);
    if (!(regs + 0)->True()) goto block0;
    vm.last = {362, 6};
    *(regs + 0) = Value(4);
    vm.ret_slots = 1;
    vm.ret_unwind_to = 186;
    *(psp++) = *(regs + 0);
    goto epilogue;
    block0:;
    vm.last = {363, 6};
    *(regs + 0) = Value(3);
    vm.ret_slots = 1;
    vm.ret_unwind_to = 185;
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// function187
static void fun_195(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[1];
    *(locals + 0) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1431, locals);
    vm.last = {383, 6};
    *(regs + 0) = Value(2.000000);
    vm.temp_lval = locals + 0; // divisor
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {385, 6};
    *(regs + 0) = Value(5.000000);
    *(regs + 1) = *(locals + 0); // divisor
    U_FMOD(vm, regs + 2);
    *(regs + 1) = Value(1.000000);
    U_FEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 385, 6, 354);
    vm.last = {386, 6};
    U_PUSHVARVF(vm, regs + 0, 76, 2); // float2_1+0
    *(regs + 2) = Value(5.000000);
    U_FVSMUL(vm, regs + 3, 2);
    *(regs + 2) = *(locals + 0); // divisor
    U_FVSMOD(vm, regs + 3, 2);
    U_PUSHVARVF(vm, regs + 2, 76, 2); // float2_1+0
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 386, 6, 355);
    vm.last = {387, 6};
    U_PUSHVARVF(vm, regs + 0, 125, 2); // int2_1+0
    *(regs + 2) = Value(5);
    U_IVSMUL(vm, regs + 3, 2);
    *(regs + 2) = Value(2);
    U_IVSMOD(vm, regs + 3, 2);
    U_PUSHVARVF(vm, regs + 2, 125, 2); // int2_1+0
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 387, 6, 356);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function188
static void fun_196(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value keepvar[6];
    Value locals[1];
    *(locals + 0) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1435, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    *(keepvar + 1) = Value(0, lobster::V_NIL);
    *(keepvar + 2) = Value(0, lobster::V_NIL);
    *(keepvar + 3) = Value(0, lobster::V_NIL);
    *(keepvar + 4) = Value(0, lobster::V_NIL);
    *(keepvar + 5) = Value(0, lobster::V_NIL);
    vm.last = {391, 6};
    vm.last = {391, 6};
    vm.last = {395, 6};
    U_PUSHVARVF(vm, regs + 0, 76, 2); // float2_1+0
    U_PUSHSTR(vm, regs + 2, 357); // "hello"
    (regs + 2)->LTINCRTNIL();
    *(regs + 3) = Value(2);
    U_NEWVEC(vm, regs + 4, 7, 1);
    U_NEWOBJECT(vm, regs + 4, 711); // Reflect
    vm.temp_lval = locals + 0; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {396, 6};
    *(regs + 0) = *(locals + 0); // r
    U_BCALLRETV(vm, regs + 1, 156, 1); // type_string
    *(keepvar + 0) = *(regs + 0);
    U_PUSHSTR(vm, regs + 1, 358); // "Reflect"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 396, 6, 359);
    vm.last = {397, 6};
    *(regs + 0) = *(locals + 0); // r
    U_BCALLRETV(vm, regs + 1, 158, 1); // type_field_count
    *(regs + 1) = Value(4);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 397, 6, 360);
    vm.last = {398, 6};
    *(regs + 0) = *(locals + 0); // r
    *(regs + 1) = Value(0);
    U_BCALLRETV(vm, regs + 2, 159, 1); // type_field_string
    *(keepvar + 1) = *(regs + 0);
    U_PUSHSTR(vm, regs + 1, 361); // "float"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 398, 6, 362);
    vm.last = {399, 6};
    *(regs + 0) = *(locals + 0); // r
    *(regs + 1) = Value(3);
    U_BCALLRETV(vm, regs + 2, 159, 1); // type_field_string
    *(keepvar + 2) = *(regs + 0);
    U_PUSHSTR(vm, regs + 1, 363); // "[int]"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 399, 6, 364);
    vm.last = {400, 6};
    *(regs + 0) = *(locals + 0); // r
    *(regs + 1) = Value(2);
    U_BCALLRETV(vm, regs + 2, 160, 1); // type_field_name
    *(keepvar + 3) = *(regs + 0);
    U_PUSHSTR(vm, regs + 1, 365); // "s"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 400, 6, 366);
    vm.last = {401, 6};
    *(regs + 0) = *(locals + 0); // r
    *(regs + 1) = Value(0);
    U_BCALLRETV(vm, regs + 2, 161, 1); // type_field_value
    *(keepvar + 4) = *(regs + 0);
    U_PUSHSTR(vm, regs + 1, 367); // "1.0"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 401, 6, 368);
    vm.last = {402, 6};
    *(regs + 0) = *(locals + 0); // r
    U_PUSHFLD(vm, regs + 1, 3);
    U_BCALLRETV(vm, regs + 1, 157, 1); // type_element_string
    *(keepvar + 5) = *(regs + 0);
    U_PUSHSTR(vm, regs + 1, 369); // "int"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 402, 6, 370);
    DecVal(vm, locals[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    DecVal(vm, keepvar[1]);
    DecVal(vm, keepvar[2]);
    DecVal(vm, keepvar[3]);
    DecVal(vm, keepvar[4]);
    DecVal(vm, keepvar[5]);
    PopFunId(vm);
}

// function189
static void fun_197(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    BackupVar(vm, 402);
    *(locals + 0) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1439, locals);
    vm.last = {406, 6};
    *(regs + 0) = Value(1);
    U_LVAL_VARF(vm, regs + 1, 402); // state
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {407, 6};
    U_PUSHFUN(vm, regs + 0, 0, fun_198);
    vm.temp_lval = locals + 0; // fv
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {410, 6};
    *(regs + 0) = *(locals + 0); // fv
    U_BCALLRET1(vm, regs + 1, 154, 0); // call_function_value
    vm.last = {411, 6};
    U_PUSHVARF(vm, regs + 0, 402); // state
    *(regs + 1) = Value(2);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 411, 6, 371);
    goto epilogue;
    epilogue:;
    RestoreBackup(vm, 402);
    PopFunId(vm);
}

// function190
static void fun_198(VMRef vm, StackPtr psp) {
    Value regs[1];
    PushFunId(vm, funinfo_table + 1444, 0);
    vm.last = {409, 6};
    *(regs + 0) = Value(2);
    U_LVAL_VARF(vm, regs + 1, 402); // state
    *(vm.temp_lval) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function191
static void fun_199(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[1];
    *(locals + 0) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1447, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {415, 6};
    *(regs + 0) = Value(4);
    U_PUSHFUN(vm, regs + 1, 0, fun_200);
    fun_502(vm, regs + 2); // call: map
    vm.temp_lval = locals + 0; // gs
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {423, 6};
    *(regs + 0) = *(locals + 0); // gs
    *(regs + 1) = Value(10);
    *(regs + 2) = Value(20);
    *(regs + 3) = Value(30);
    *(regs + 4) = Value(40);
    U_NEWVEC(vm, regs + 5, 7, 4);
    *(keepvar + 0) = *(regs + 1);
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 423, 6, 372);
    vm.last = {424, 6};
    vm.last = {425, 6};
    DecVal(vm, locals[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// function192
static void fun_200(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1451, locals);
    vm.last = {416, 6};
    *(regs + 0) = *(locals + 0); // i
    *(regs + 1) = Value(2);
    U_IGE(vm, regs + 2);
    if (!(regs + 0)->True()) goto block0;
    vm.last = {420, 6};
    *(regs + 0) = *(locals + 0); // i
    *(regs + 1) = Value(3);
    U_IGE(vm, regs + 2);
    if (!(regs + 0)->True()) goto block1;
    vm.last = {422, 6};
    *(regs + 0) = Value(40);
    goto block2;
    block1:;
    vm.last = {421, 6};
    *(regs + 0) = Value(30);
    block2:;
    goto block3;
    block0:;
    vm.last = {417, 6};
    *(regs + 0) = *(locals + 0); // i
    *(regs + 1) = Value(1);
    U_IGE(vm, regs + 2);
    if (!(regs + 0)->True()) goto block4;
    vm.last = {419, 6};
    *(regs + 0) = Value(20);
    goto block5;
    block4:;
    vm.last = {418, 6};
    *(regs + 0) = Value(10);
    block5:;
    block3:;
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function193
static void fun_201(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value keepvar[1];
    PushFunId(vm, funinfo_table + 1455, 0);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {430, 6};
    *(regs + 0) = Value(17);
    fun_504(vm, regs + 1); // call: dictionary
    U_PUSHSTR(vm, regs + 1, 373); // "foo"
    fun_144(vm, regs + 2); // call: get
    *(keepvar + 0) = *(regs + 0);
    U_PUSHNIL(vm, regs + 1);
    U_AEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 430, 6, 374);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// function194
static void fun_202(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[12];
    *(locals + 0) = Value(0, lobster::V_NIL);
    *(locals + 1) = Value(0, lobster::V_NIL);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    *(locals + 5) = Value(0, lobster::V_NIL);
    *(locals + 6) = Value(0, lobster::V_NIL);
    *(locals + 7) = Value(0, lobster::V_NIL);
    *(locals + 8) = Value(0, lobster::V_NIL);
    *(locals + 9) = Value(0, lobster::V_NIL);
    *(locals + 10) = Value(0, lobster::V_NIL);
    *(locals + 11) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1458, locals);
    vm.last = {10, 9};
    vm.last = {10, 9};
    U_PUSHFUN(vm, regs + 0, 0, fun_203);
    vm.temp_lval = locals + 0; // f
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {253, 5};
    fun_203(vm, regs + 0); // call: function195
    vm.last = {18, 9};
    vm.last = {18, 9};
    U_PUSHFUN(vm, regs + 0, 0, fun_209);
    vm.temp_lval = locals + 1; // f
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {253, 5};
    fun_209(vm, regs + 0); // call: function201
    vm.last = {34, 9};
    *(regs + 0) = Value(0);
    *(regs + 1) = Value(0);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 34, 9, 375);
    vm.last = {36, 9};
    vm.last = {36, 9};
    U_PUSHFUN(vm, regs + 0, 0, fun_211);
    vm.temp_lval = locals + 2; // f
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {253, 5};
    fun_211(vm, regs + 0); // call: function203
    vm.last = {50, 9};
    vm.last = {50, 9};
    U_PUSHFUN(vm, regs + 0, 0, fun_214);
    vm.temp_lval = locals + 3; // f
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {253, 5};
    fun_214(vm, regs + 0); // call: function206
    vm.last = {61, 9};
    vm.last = {61, 9};
    U_PUSHFUN(vm, regs + 0, 0, fun_215);
    vm.temp_lval = locals + 4; // f
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {253, 5};
    fun_215(vm, regs + 0); // call: function207
    vm.last = {127, 9};
    vm.last = {127, 9};
    U_PUSHFUN(vm, regs + 0, 0, fun_236);
    vm.temp_lval = locals + 5; // f
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {253, 5};
    fun_236(vm, regs + 0); // call: function217
    vm.last = {139, 9};
    vm.last = {139, 9};
    U_PUSHFUN(vm, regs + 0, 0, fun_238);
    vm.temp_lval = locals + 6; // f
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {253, 5};
    fun_238(vm, regs + 0); // call: function219
    vm.last = {227, 9};
    vm.last = {227, 9};
    U_PUSHFUN(vm, regs + 0, 0, fun_253);
    vm.temp_lval = locals + 7; // f
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {253, 5};
    fun_253(vm, regs + 0); // call: function234
    vm.last = {239, 9};
    vm.last = {239, 9};
    U_PUSHFUN(vm, regs + 0, 0, fun_254);
    vm.temp_lval = locals + 8; // f
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {253, 5};
    fun_254(vm, regs + 0); // call: function235
    vm.last = {245, 9};
    vm.last = {245, 9};
    U_PUSHFUN(vm, regs + 0, 0, fun_257);
    vm.temp_lval = locals + 9; // f
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {253, 5};
    fun_257(vm, regs + 0); // call: function237
    vm.last = {265, 9};
    vm.last = {265, 9};
    U_PUSHFUN(vm, regs + 0, 0, fun_259);
    vm.temp_lval = locals + 10; // f
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {253, 5};
    fun_259(vm, regs + 0); // call: function239
    vm.last = {272, 9};
    vm.last = {272, 9};
    U_PUSHFUN(vm, regs + 0, 0, fun_260);
    vm.temp_lval = locals + 11; // f
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {253, 5};
    fun_260(vm, regs + 0); // call: function240
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function195
static void fun_203(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value keepvar[1];
    PushFunId(vm, funinfo_table + 1473, 0);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {11, 9};
    vm.last = {12, 9};
    vm.last = {15, 9};
    *(regs + 0) = Value(1);
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_205(vm, regs + 2); // call: g
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 15, 9, 376);
    vm.last = {16, 9};
    U_PUSHSTR(vm, regs + 0, 377); // "a"
    U_PUSHFUN(vm, regs + 1, 0, fun_208);
    fun_509(vm, regs + 2); // call: g
    *(keepvar + 0) = *(regs + 1);
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 16, 9, 378);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// h
static void fun_511(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1476, locals);
    vm.last = {11, 9};
    vm.last = {14, 9};
    fun_208(vm, regs + 0); // call: function200
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// g
static void fun_509(VMRef vm, StackPtr psp) {
    Value regs[1];
    SwapVars(vm, 1296, psp, 1);
    PushFunId(vm, funinfo_table + 1480, 0);
    vm.last = {14, 9};
    U_PUSHFUN(vm, regs + 0, 0, fun_10000001);
    fun_511(vm, regs + 1); // call: h
    psp = PopArg(vm, 1296, psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// g
static void fun_205(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    SwapVars(vm, 407, psp, 1);
    *(locals + 0) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1484, locals);
    vm.last = {14, 9};
    vm.last = {14, 9};
    U_PUSHFUN(vm, regs + 0, 0, fun_206);
    vm.temp_lval = locals + 0; // i
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {11, 9};
    fun_206(vm, regs + 0); // call: function198
    psp = PopArg(vm, 407, psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function198
static void fun_206(VMRef vm, StackPtr psp) {
    Value regs[1];
    PushFunId(vm, funinfo_table + 1489, 0);
    vm.last = {14, 9};
    vm.last = {15, 9};
    *(regs + 0) = Value(1);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function200
static void fun_208(VMRef vm, StackPtr psp) {
    Value regs[1];
    PushFunId(vm, funinfo_table + 1492, 0);
    vm.last = {16, 9};
    U_PUSHSTR(vm, regs + 0, 379); // "a"
    (regs + 0)->LTINCRTNIL();
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function201
static void fun_209(VMRef vm, StackPtr psp) {
    Value regs[1];
    PushFunId(vm, funinfo_table + 1495, 0);
    vm.last = {19, 9};
    vm.last = {31, 9};
    *(regs + 0) = Value(1);
    fun_210(vm, regs + 1); // call: compile_time_if
    U_POP(vm, regs + 1);
    *(regs + 0) = Value(1);
    U_ASSERT(vm, regs + 1, 31, 9, 380);
    vm.last = {32, 9};
    U_PUSHSTR(vm, regs + 0, 381); // ""
    fun_514(vm, regs + 1); // call: compile_time_if
    U_POPREF(vm, regs + 1);
    *(regs + 0) = Value(1);
    U_ASSERT(vm, regs + 1, 32, 9, 382);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// compile_time_if
static void fun_514(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1498, locals);
    vm.last = {30, 9};
    vm.last = {30, 9};
    *(regs + 0) = *(locals + 0); // x
    (regs + 0)->LTINCRTNIL();
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// compile_time_if
static void fun_210(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1502, locals);
    vm.last = {30, 9};
    vm.last = {26, 9};
    *(regs + 0) = Value(1);
    *(regs + 1) = *(locals + 0); // x
    U_IDIV(vm, regs + 2);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function203
static void fun_211(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value keepvar[2];
    PushFunId(vm, funinfo_table + 1506, 0);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    *(keepvar + 1) = Value(0, lobster::V_NIL);
    vm.last = {37, 9};
    vm.last = {37, 9};
    vm.last = {39, 9};
    vm.last = {39, 9};
    vm.last = {43, 9};
    vm.last = {46, 9};
    vm.last = {47, 9};
    *(regs + 0) = Value(1);
    fun_212(vm, regs + 1); // call: return_from
    *(keepvar + 0) = *(regs + 0);
    U_ISTYPE(vm, regs + 1, 1569); // testa
    U_ASSERT(vm, regs + 1, 47, 9, 383);
    vm.last = {48, 9};
    U_PUSHSTR(vm, regs + 0, 384); // ""
    fun_516(vm, regs + 1); // call: return_from
    *(keepvar + 1) = *(regs + 0);
    U_ISTYPE(vm, regs + 1, 1569); // testa
    U_ASSERT(vm, regs + 1, 48, 9, 385);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    DecVal(vm, keepvar[1]);
    PopFunId(vm);
}

// return_from
static void fun_516(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1509, locals);
    vm.last = {44, 9};
    fun_213(vm, regs + 0); // call: return_from2
    if (vm.ret_unwind_to == 205) goto block0;
    Pop(psp);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block0:;
    vm.last = {45, 9};
    *(regs + 0) = Value(0);
    *(regs + 1) = Value(0);
    U_NEWOBJECT(vm, regs + 2, 1578); // testb
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// return_from
static void fun_212(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1513, locals);
    vm.last = {44, 9};
    fun_213(vm, regs + 0); // call: return_from2
    if (vm.ret_unwind_to == 205) goto block0;
    Pop(psp);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block0:;
    vm.last = {45, 9};
    *(regs + 0) = Value(0);
    *(regs + 1) = Value(0);
    U_NEWOBJECT(vm, regs + 2, 1578); // testb
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// return_from2
static void fun_213(VMRef vm, StackPtr psp) {
    Value regs[1];
    PushFunId(vm, funinfo_table + 1517, 0);
    vm.last = {46, 9};
    *(regs + 0) = Value(0);
    U_NEWOBJECT(vm, regs + 1, 1569); // testa
    vm.ret_slots = 1;
    vm.ret_unwind_to = 204;
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function206
static void fun_214(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value keepvar[2];
    Value locals[1];
    *(locals + 0) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1520, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    *(keepvar + 1) = Value(0, lobster::V_NIL);
    vm.last = {52, 9};
    *(regs + 0) = Value(1);
    vm.temp_lval = locals + 0; // b
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {53, 9};
    *(regs + 0) = Value(1);
    vm.temp_lval = locals + 0; // b
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {54, 9};
    *(regs + 0) = *(locals + 0); // b
    *(regs + 1) = Value(1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 54, 9, 386);
    vm.last = {55, 9};
    *(regs + 0) = Value(1);
    U_A2S(vm, regs + 1, 146);
    *(keepvar + 0) = *(regs + 0);
    U_PUSHSTR(vm, regs + 1, 387); // "true"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 55, 9, 388);
    vm.last = {56, 9};
    *(regs + 0) = Value(1338);
    U_PUSHSTR(vm, regs + 1, 389); // "[true]"
    U_BCALLRETV(vm, regs + 2, 254, 1); // parse_data
    U_POPREF(vm, regs + 2);
    U_ASSERTR(vm, regs + 1, 56, 9, 390);
    *(keepvar + 1) = *(regs + 0);
    *(regs + 1) = Value(0);
    U_VPUSHIDXI(vm, regs + 2);
    *(regs + 1) = Value(1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 56, 9, 391);
    vm.last = {57, 9};
    *(regs + 0) = Value(1);
    U_DUP(vm, regs + 1);
    *(regs + 2) = Value(0);
    U_IEQ(vm, regs + 3);
    if (!(regs + 1)->True()) goto block0;
    U_POP(vm, regs + 1);
    vm.last = {58, 9};
    *(regs + 0) = Value(0);
    U_ASSERT(vm, regs + 1, 58, 9, 392);
    goto block1;
    block0:;
    U_DUP(vm, regs + 1);
    *(regs + 2) = Value(1);
    U_IEQ(vm, regs + 3);
    if (!(regs + 1)->True()) goto block2;
    U_POP(vm, regs + 1);
    vm.last = {59, 9};
    goto block3;
    block2:;
    U_POP(vm, regs + 1);
    U_ENUM_RANGE_ERR(vm, regs + 0);
    block1:;
    block3:;
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    DecVal(vm, keepvar[1]);
    PopFunId(vm);
}

// function207
static void fun_215(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value keepvar[4];
    Value locals[9];
    *(locals + 0) = Value(0, lobster::V_NIL);
    *(locals + 1) = Value(0, lobster::V_NIL);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    *(locals + 5) = Value(0, lobster::V_NIL);
    *(locals + 6) = Value(0, lobster::V_NIL);
    *(locals + 7) = Value(0, lobster::V_NIL);
    *(locals + 8) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1524, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    *(keepvar + 1) = Value(0, lobster::V_NIL);
    *(keepvar + 2) = Value(0, lobster::V_NIL);
    *(keepvar + 3) = Value(0, lobster::V_NIL);
    vm.last = {65, 9};
    vm.last = {63, 9};
    vm.last = {63, 9};
    vm.last = {68, 9};
    vm.last = {66, 9};
    vm.last = {66, 9};
    vm.last = {71, 9};
    vm.last = {71, 9};
    *(regs + 0) = Value(0);
    vm.temp_lval = locals + 5; // this+0
    U_LV_WRITEV(vm, regs + 1, 1);
    vm.last = {65, 9};
    *(regs + 0) = Value(1);
    *(regs + 1) = Value(1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 71, 9, 393);
    vm.last = {72, 9};
    vm.last = {72, 9};
    *(regs + 0) = Value(0);
    *(regs + 1) = Value(0);
    vm.temp_lval = locals + 6; // this+0
    U_LV_WRITEV(vm, regs + 2, 2);
    vm.last = {68, 9};
    *(regs + 0) = Value(2);
    *(regs + 1) = Value(2);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 72, 9, 394);
    vm.last = {74, 9};
    vm.last = {73, 9};
    vm.last = {73, 9};
    vm.last = {77, 9};
    vm.last = {78, 9};
    vm.last = {79, 9};
    vm.last = {80, 9};
    vm.last = {81, 9};
    vm.last = {82, 9};
    vm.last = {75, 9};
    vm.last = {75, 9};
    vm.last = {85, 9};
    vm.last = {86, 9};
    vm.last = {87, 9};
    vm.last = {88, 9};
    vm.last = {89, 9};
    vm.last = {90, 9};
    vm.last = {83, 9};
    vm.last = {83, 9};
    vm.last = {93, 9};
    vm.last = {91, 9};
    vm.last = {94, 9};
    *(regs + 0) = Value(0);
    *(regs + 1) = Value(0);
    U_NEWOBJECT(vm, regs + 2, 771); // C2
    vm.temp_lval = locals + 0; // x
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {95, 9};
    *(regs + 0) = Value(0);
    U_PUSHSTR(vm, regs + 1, 395); // "hello"
    (regs + 1)->LTINCRTNIL();
    U_NEWOBJECT(vm, regs + 2, 783); // C3
    vm.temp_lval = locals + 1; // x2
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {97, 9};
    *(regs + 0) = *(locals + 0); // x
    (regs + 0)->LTINCRTNIL();
    *(regs + 1) = Value(0);
    U_DDCALL(vm, regs + 2, 0, 1); vm.next_call_target(vm, regs + 2);
    *(regs + 1) = Value(2);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 97, 9, 396);
    vm.last = {99, 9};
    *(regs + 0) = Value(0);
    *(regs + 1) = Value(0);
    U_NEWOBJECT(vm, regs + 2, 771); // C2
    *(regs + 1) = Value(0);
    fun_225(vm, regs + 2); // call: f
    *(regs + 1) = Value(2);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 99, 9, 397);
    vm.last = {101, 9};
    *(regs + 0) = *(locals + 0); // x
    (regs + 0)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 1, 398); // "hi"
    (regs + 1)->LTINCRTNIL();
    U_DDCALL(vm, regs + 2, 1, 1); vm.next_call_target(vm, regs + 2);
    *(keepvar + 0) = *(regs + 0);
    U_PUSHSTR(vm, regs + 1, 399); // "hi2"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 101, 9, 400);
    vm.last = {102, 9};
    *(regs + 0) = *(locals + 0); // x
    (regs + 0)->LTINCRTNIL();
    *(regs + 1) = Value(3);
    U_DDCALL(vm, regs + 2, 2, 1); vm.next_call_target(vm, regs + 2);
    *(regs + 1) = Value(5);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 102, 9, 401);
    vm.last = {103, 9};
    *(regs + 0) = *(locals + 1); // x2
    (regs + 0)->LTINCRTNIL();
    *(regs + 1) = Value(10);
    U_DDCALL(vm, regs + 2, 0, 1); vm.next_call_target(vm, regs + 2);
    *(regs + 1) = Value(15);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 103, 9, 402);
    vm.last = {105, 9};
    *(regs + 0) = *(locals + 0); // x
    (regs + 0)->LTINCRTNIL();
    *(regs + 1) = Value(10);
    U_DDCALL(vm, regs + 2, 3, 1); vm.next_call_target(vm, regs + 2);
    *(regs + 1) = Value(0);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 105, 9, 403);
    vm.last = {107, 9};
    *(regs + 0) = *(locals + 0); // x
    (regs + 0)->LTINCRTNIL();
    U_DDCALL(vm, regs + 1, 4, 0); vm.next_call_target(vm, regs + 1);
    *(regs + 1) = Value(2);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 107, 9, 404);
    vm.last = {109, 9};
    *(regs + 0) = *(locals + 0); // x
    (regs + 0)->LTINCRTNIL();
    U_DDCALL(vm, regs + 1, 5, 0); vm.next_call_target(vm, regs + 1);
    *(regs + 1) = Value(11);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 109, 9, 405);
    vm.last = {110, 9};
    *(regs + 0) = Value(0);
    U_NEWOBJECT(vm, regs + 1, 762); // C1
    vm.temp_lval = locals + 2; // x3
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {112, 9};
    *(regs + 0) = *(locals + 2); // x3
    (regs + 0)->LTINCRTNIL();
    U_DDCALL(vm, regs + 1, 6, 0); vm.next_call_target(vm, regs + 1);
    *(regs + 1) = Value(12);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 112, 9, 406);
    vm.last = {114, 9};
    *(regs + 0) = *(locals + 0); // x
    (regs + 0)->LTINCRTNIL();
    U_DDCALL(vm, regs + 1, 7, 0); vm.next_call_target(vm, regs + 1);
    vm.temp_lval = locals + 4; // mr2
    U_LV_WRITEREF(vm, regs + 2);
    vm.temp_lval = locals + 3; // mr1
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {115, 9};
    *(regs + 0) = *(locals + 3); // mr1
    *(regs + 1) = Value(2);
    U_IEQ(vm, regs + 2);
    if (!(regs + 0)->True()) goto block0;
    U_POP(vm, regs + 1);
    *(regs + 0) = *(locals + 4); // mr2
    U_PUSHSTR(vm, regs + 1, 407); // "b"
    U_SEQ(vm, regs + 2);
    block0:;
    U_ASSERT(vm, regs + 1, 115, 9, 408);
    vm.last = {117, 9};
    vm.last = {119, 9};
    vm.last = {120, 9};
    vm.last = {121, 9};
    vm.last = {122, 9};
    vm.last = {123, 9};
    *(regs + 0) = Value(1);
    U_NEWOBJECT(vm, regs + 1, 795); // D
    fun_232(vm, regs + 1); // call: foo
    vm.last = {123, 9};
    *(regs + 1) = Value(2);
    vm.temp_lval = locals + 8; // i
    *(vm.temp_lval) = *(regs + 1);
    vm.last = {120, 9};
    *(regs + 1) = *(locals + 8); // i
    U_IADD(vm, regs + 2);
    *(regs + 1) = Value(3);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 123, 9, 409);
    vm.last = {124, 9};
    U_PUSHSTR(vm, regs + 0, 410); // "ha"
    (regs + 0)->LTINCRTNIL();
    U_NEWOBJECT(vm, regs + 1, 1402); // D
    fun_523(vm, regs + 1); // call: foo
    *(keepvar + 1) = *(regs + 0);
    U_PUSHSTR(vm, regs + 1, 411); // "llo"
    fun_234(vm, regs + 2); // call: foo
    *(keepvar + 2) = *(regs + 1);
    U_SADD(vm, regs + 2);
    *(keepvar + 3) = *(regs + 0);
    U_PUSHSTR(vm, regs + 1, 412); // "hallo"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 124, 9, 413);
    vm.last = {125, 9};
    *(regs + 0) = Value(1.000000);
    fun_235(vm, regs + 1); // call: foo
    *(regs + 1) = Value(2.000000);
    U_FEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 125, 9, 414);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[2]);
    DecVal(vm, locals[4]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    DecVal(vm, keepvar[1]);
    DecVal(vm, keepvar[2]);
    DecVal(vm, keepvar[3]);
    PopFunId(vm);
}

// ad
static void fun_218(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1536, locals);
    vm.last = {74, 9};
    *(regs + 0) = Value(10);
    DecVal(vm, locals[0]);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// ad
static void fun_230(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1540, locals);
    vm.last = {90, 9};
    *(regs + 0) = Value(11);
    DecVal(vm, locals[0]);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// f
static void fun_219(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1544, locals);
    vm.last = {77, 9};
    *(regs + 0) = Value(1);
    *(regs + 1) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 2, 0);
    U_IADD(vm, regs + 2);
    *(regs + 1) = *(locals + 1); // c
    U_IADD(vm, regs + 2);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// f
static void fun_225(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1549, locals);
    vm.last = {85, 9};
    *(regs + 0) = Value(2);
    *(regs + 1) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 2, 0);
    U_IADD(vm, regs + 2);
    *(regs + 1) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 2, 1);
    U_IADD(vm, regs + 2);
    *(regs + 1) = *(locals + 1); // c
    U_IADD(vm, regs + 2);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// f
static void fun_231(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1554, locals);
    vm.last = {93, 9};
    *(regs + 0) = *(locals + 1); // c
    *(regs + 1) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 2, 1);
    U_BCALLRET1(vm, regs + 2, 12, 1); // length
    U_IADD(vm, regs + 2);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// g
static void fun_519(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1559, locals);
    vm.last = {78, 9};
    *(regs + 0) = *(locals + 1); // c
    *(regs + 1) = Value(1);
    U_IADD(vm, regs + 2);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// g
static void fun_220(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value keepvar[1];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1564, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {78, 9};
    *(regs + 0) = *(locals + 1); // c
    *(regs + 1) = Value(1);
    U_A2S(vm, regs + 2, 0);
    *(keepvar + 0) = *(regs + 1);
    U_SADD(vm, regs + 2);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// g
static void fun_520(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1569, locals);
    vm.last = {86, 9};
    *(regs + 0) = *(locals + 1); // c
    *(regs + 1) = Value(2);
    U_IADD(vm, regs + 2);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// g
static void fun_226(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value keepvar[1];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1574, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {86, 9};
    *(regs + 0) = *(locals + 1); // c
    *(regs + 1) = Value(2);
    U_A2S(vm, regs + 2, 0);
    *(keepvar + 0) = *(regs + 1);
    U_SADD(vm, regs + 2);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// r
static void fun_221(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1579, locals);
    vm.last = {79, 9};
    *(regs + 0) = *(locals + 1); // c
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// r
static void fun_521(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1584, locals);
    vm.last = {87, 9};
    *(regs + 0) = *(locals + 1); // c
    if (!(regs + 0)->True()) goto block0;
    vm.last = {87, 9};
    *(regs + 0) = *(locals + 0); // this
    *(regs + 1) = *(locals + 1); // c
    *(regs + 2) = Value(1);
    U_ISUB(vm, regs + 3);
    fun_521(vm, regs + 2); // call: r
    goto block1;
    block0:;
    vm.last = {87, 9};
    *(regs + 0) = Value(0);
    U_NEWOBJECT(vm, regs + 1, 762); // C1
    *(regs + 1) = *(locals + 1); // c
    U_DDCALL(vm, regs + 2, 3, 1); vm.next_call_target(vm, regs + 2);
    block1:;
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// r
static void fun_227(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1589, locals);
    vm.last = {87, 9};
    *(regs + 0) = *(locals + 1); // c
    if (!(regs + 0)->True()) goto block0;
    vm.last = {87, 9};
    *(regs + 0) = *(locals + 0); // this
    *(regs + 1) = *(locals + 1); // c
    *(regs + 2) = Value(1);
    U_ISUB(vm, regs + 3);
    fun_521(vm, regs + 2); // call: r
    goto block1;
    block0:;
    vm.last = {87, 9};
    *(regs + 0) = Value(0);
    U_NEWOBJECT(vm, regs + 1, 762); // C1
    *(regs + 1) = *(locals + 1); // c
    U_DDCALL(vm, regs + 2, 3, 1); vm.next_call_target(vm, regs + 2);
    block1:;
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// mr
static void fun_222(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1594, locals);
    vm.last = {80, 9};
    *(regs + 0) = Value(1);
    U_PUSHSTR(vm, regs + 1, 415); // "a"
    (regs + 1)->LTINCRTNIL();
    DecVal(vm, locals[0]);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// mr
static void fun_228(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1598, locals);
    vm.last = {88, 9};
    *(regs + 0) = Value(2);
    U_PUSHSTR(vm, regs + 1, 416); // "b"
    (regs + 1)->LTINCRTNIL();
    DecVal(vm, locals[0]);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// sup
static void fun_223(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1602, locals);
    vm.last = {81, 9};
    *(regs + 0) = Value(1);
    DecVal(vm, locals[0]);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// sup
static void fun_229(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 1);
    *(locals + 1) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1606, locals);
    vm.last = {89, 9};
    *(regs + 0) = Value(1);
    vm.last = {89, 9};
    *(regs + 1) = *(locals + 0); // this
    vm.temp_lval = locals + 1; // this
    *(vm.temp_lval) = *(regs + 1);
    vm.last = {81, 9};
    *(regs + 1) = Value(1);
    U_IADD(vm, regs + 2);
    DecVal(vm, locals[0]);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// sni
static void fun_224(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1611, locals);
    vm.last = {82, 9};
    *(regs + 0) = Value(12);
    DecVal(vm, locals[0]);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// foo
static void fun_523(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1615, locals);
    vm.last = {119, 9};
    *(regs + 0) = *(locals + 0); // d
    U_PUSHFLD(vm, regs + 1, 0);
    (regs + 0)->LTINCRTNIL();
    DecVal(vm, locals[0]);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// foo
static void fun_232(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1619, locals);
    vm.last = {119, 9};
    *(regs + 0) = *(locals + 0); // d
    U_PUSHFLD(vm, regs + 1, 0);
    DecVal(vm, locals[0]);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// foo
static void fun_234(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1623, locals);
    vm.last = {121, 9};
    *(regs + 0) = *(locals + 0); // s
    (regs + 0)->LTINCRTNIL();
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// foo
static void fun_235(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1627, locals);
    vm.last = {122, 9};
    *(regs + 0) = *(locals + 0); // t
    *(regs + 1) = Value(1.000000);
    U_FADD(vm, regs + 2);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function217
static void fun_236(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = Value(0, lobster::V_NIL);
    *(locals + 1) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1631, locals);
    vm.last = {129, 9};
    U_PUSHSTR(vm, regs + 0, 417); // ""
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 0; // a
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {130, 9};
    *(regs + 0) = *(locals + 0); // a
    U_PUSHSTR(vm, regs + 1, 418); // ""
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 130, 9, 419);
    vm.last = {131, 9};
    U_PUSHNIL(vm, regs + 0);
    vm.temp_lval = locals + 1; // b
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {132, 9};
    *(regs + 0) = *(locals + 1); // b
    U_PUSHNIL(vm, regs + 1);
    U_AEQ(vm, regs + 2);
    if (!(regs + 0)->True()) goto block0;
    vm.last = {133, 9};
    goto block1;
    block0:;
    vm.last = {135, 9};
    *(regs + 0) = *(locals + 1); // b
    U_PUSHSTR(vm, regs + 1, 420); // ""
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 135, 9, 421);
    block1:;
    vm.last = {136, 9};
    vm.last = {137, 9};
    *(regs + 0) = *(locals + 0); // a
    fun_237(vm, regs + 1); // call: f
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// f
static void fun_237(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1636, locals);
    vm.last = {136, 9};
    *(regs + 0) = *(locals + 0); // x
    U_PUSHSTR(vm, regs + 1, 422); // ""
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 136, 9, 423);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function219
static void fun_238(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[6];
    Value locals[8];
    *(locals + 0) = Value(0, lobster::V_NIL);
    *(locals + 1) = Value(0, lobster::V_NIL);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    *(locals + 5) = Value(0, lobster::V_NIL);
    *(locals + 6) = Value(0, lobster::V_NIL);
    *(locals + 7) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1640, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    *(keepvar + 1) = Value(0, lobster::V_NIL);
    *(keepvar + 2) = Value(0, lobster::V_NIL);
    *(keepvar + 3) = Value(0, lobster::V_NIL);
    *(keepvar + 4) = Value(0, lobster::V_NIL);
    *(keepvar + 5) = Value(0, lobster::V_NIL);
    vm.last = {147, 9};
    vm.last = {149, 9};
    vm.last = {151, 9};
    vm.last = {141, 9};
    vm.last = {154, 9};
    vm.last = {157, 9};
    vm.last = {158, 9};
    *(regs + 0) = Value(1);
    U_NEWVEC(vm, regs + 1, 7, 1);
    U_PUSHNIL(vm, regs + 1);
    U_NEWVEC(vm, regs + 2, 827, 0);
    U_PUSHNIL(vm, regs + 3);
    U_PUSHNIL(vm, regs + 4);
    U_NEWOBJECT(vm, regs + 5, 804); // Ai
    vm.temp_lval = locals + 0; // a
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {159, 9};
    *(regs + 0) = *(locals + 0); // a
    U_PUSHFLD(vm, regs + 1, 2);
    *(regs + 1) = *(locals + 0); // a
    (regs + 1)->LTINCRTNIL();
    *(regs + 2) = Value(1);
    U_BCALLRETV(vm, regs + 3, 15, 1); // push
    U_POP(vm, regs + 1);
    vm.last = {160, 9};
    U_NEWVEC(vm, regs + 0, 827, 0);
    *(regs + 1) = *(locals + 0); // a
    U_LVAL_FLD(vm, regs + 2, 2);
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {161, 9};
    *(regs + 0) = *(locals + 0); // a
    (regs + 0)->LTINCRTNIL();
    *(regs + 1) = *(locals + 0); // a
    U_LVAL_FLD(vm, regs + 2, 3);
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {162, 9};
    U_PUSHNIL(vm, regs + 0);
    *(regs + 1) = *(locals + 0); // a
    U_LVAL_FLD(vm, regs + 2, 3);
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {163, 9};
    *(regs + 0) = *(locals + 0); // a
    *(regs + 1) = Value(2);
    fun_239(vm, regs + 2); // call: f
    vm.last = {164, 9};
    *(regs + 0) = *(locals + 0); // a
    *(regs + 1) = Value(3);
    fun_240(vm, regs + 2); // call: g
    vm.last = {165, 9};
    *(regs + 0) = *(locals + 0); // a
    *(regs + 1) = Value(4);
    fun_242(vm, regs + 2); // call: i
    vm.last = {166, 9};
    *(regs + 0) = *(locals + 0); // a
    fun_241(vm, regs + 1); // call: h
    *(regs + 1) = Value(1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 166, 9, 424);
    vm.last = {168, 9};
    U_PUSHSTR(vm, regs + 0, 425); // "hello"
    (regs + 0)->LTINCRTNIL();
    U_NEWVEC(vm, regs + 1, 11, 1);
    U_PUSHNIL(vm, regs + 1);
    U_NEWVEC(vm, regs + 2, 852, 0);
    U_PUSHNIL(vm, regs + 3);
    U_PUSHNIL(vm, regs + 4);
    U_NEWOBJECT(vm, regs + 5, 829); // A
    vm.temp_lval = locals + 1; // b
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {169, 9};
    *(regs + 0) = *(locals + 1); // b
    U_PUSHFLD(vm, regs + 1, 2);
    *(regs + 1) = *(locals + 1); // b
    (regs + 1)->LTINCRTNIL();
    *(regs + 2) = Value(1);
    U_BCALLRETV(vm, regs + 3, 15, 1); // push
    U_POP(vm, regs + 1);
    vm.last = {170, 9};
    U_NEWVEC(vm, regs + 0, 852, 0);
    *(regs + 1) = *(locals + 1); // b
    U_LVAL_FLD(vm, regs + 2, 2);
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {171, 9};
    *(regs + 0) = *(locals + 1); // b
    (regs + 0)->LTINCRTNIL();
    *(regs + 1) = *(locals + 1); // b
    U_LVAL_FLD(vm, regs + 2, 4);
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {172, 9};
    U_PUSHNIL(vm, regs + 0);
    *(regs + 1) = *(locals + 1); // b
    U_LVAL_FLD(vm, regs + 2, 4);
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {173, 9};
    *(regs + 0) = *(locals + 1); // b
    U_PUSHSTR(vm, regs + 1, 426); // "hi"
    fun_526(vm, regs + 2); // call: f
    vm.last = {174, 9};
    *(regs + 0) = *(locals + 1); // b
    U_PUSHSTR(vm, regs + 1, 427); // "yo"
    fun_527(vm, regs + 2); // call: g
    vm.last = {175, 9};
    *(regs + 0) = *(locals + 1); // b
    U_PUSHSTR(vm, regs + 1, 428); // "sup"
    fun_528(vm, regs + 2); // call: i
    vm.last = {195, 9};
    vm.last = {196, 9};
    vm.last = {196, 9};
    *(regs + 0) = Value(1.000000);
    vm.temp_lval = locals + 4; // x
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {196, 9};
    *(regs + 0) = Value(1.000000);
    vm.temp_lval = locals + 5; // y
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {195, 9};
    *(regs + 0) = Value(1);
    U_ASSERT(vm, regs + 1, 196, 9, 429);
    vm.last = {197, 9};
    vm.last = {197, 9};
    *(regs + 0) = Value(1.000000);
    vm.temp_lval = locals + 6; // x
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {197, 9};
    *(regs + 0) = Value(1.000000);
    vm.temp_lval = locals + 7; // y
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {195, 9};
    *(regs + 0) = Value(1);
    U_ASSERT(vm, regs + 1, 197, 9, 430);
    vm.last = {198, 9};
    vm.last = {199, 9};
    *(regs + 0) = Value(2.000000);
    fun_244(vm, regs + 1); // call: k
    *(regs + 1) = Value(1.000000);
    U_FEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 199, 9, 431);
    vm.last = {200, 9};
    *(regs + 0) = Value(2.000000);
    fun_244(vm, regs + 1); // call: k
    *(regs + 1) = Value(1.000000);
    U_FEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 200, 9, 432);
    vm.last = {201, 9};
    vm.last = {203, 9};
    fun_245(vm, regs + 0); // call: q
    vm.temp_lval = locals + 3; // s
    *(vm.temp_lval) = *(regs + 1);
    vm.temp_lval = locals + 2; // r
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {204, 9};
    vm.last = {205, 9};
    vm.last = {207, 9};
    *(regs + 0) = *(locals + 0); // a
    fun_246(vm, regs + 1); // call: t0
    *(keepvar + 0) = *(regs + 0);
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = Value(0);
    U_VPUSHIDXI(vm, regs + 2);
    *(regs + 1) = Value(1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 207, 9, 433);
    vm.last = {210, 9};
    vm.last = {211, 9};
    vm.last = {210, 9};
    *(regs + 0) = Value(1);
    U_ASSERT(vm, regs + 1, 211, 9, 434);
    vm.last = {212, 9};
    vm.last = {213, 9};
    vm.last = {212, 9};
    U_PUSHNIL(vm, regs + 0);
    *(keepvar + 1) = *(regs + 0);
    U_PUSHNIL(vm, regs + 1);
    U_AEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 213, 9, 435);
    vm.last = {214, 9};
    vm.last = {215, 9};
    vm.last = {214, 9};
    U_NEWVEC(vm, regs + 0, 11, 0);
    *(keepvar + 2) = *(regs + 0);
    U_NEWVEC(vm, regs + 1, 11, 0);
    *(keepvar + 3) = *(regs + 1);
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 215, 9, 436);
    vm.last = {216, 9};
    vm.last = {217, 9};
    fun_250(vm, regs + 0); // call: p
    *(regs + 1) = Value(3);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 217, 9, 437);
    vm.last = {220, 9};
    vm.last = {222, 9};
    *(regs + 0) = *(locals + 0); // a
    fun_251(vm, regs + 1); // call: t1
    *(keepvar + 4) = *(regs + 0);
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = Value(0);
    U_VPUSHIDXI(vm, regs + 2);
    *(regs + 1) = Value(1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 222, 9, 438);
    vm.last = {223, 9};
    vm.last = {225, 9};
    *(regs + 0) = *(locals + 0); // a
    fun_252(vm, regs + 1); // call: t2
    *(keepvar + 5) = *(regs + 0);
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = Value(0);
    U_VPUSHIDXI(vm, regs + 2);
    *(regs + 1) = Value(1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 225, 9, 439);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    DecVal(vm, keepvar[1]);
    DecVal(vm, keepvar[2]);
    DecVal(vm, keepvar[3]);
    DecVal(vm, keepvar[4]);
    DecVal(vm, keepvar[5]);
    PopFunId(vm);
}

// f
static void fun_526(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1651, locals);
    vm.last = {148, 9};
    *(regs + 0) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = *(locals + 1); // t
    (regs + 1)->LTINCRTNIL();
    *(regs + 2) = Value(1);
    U_BCALLRETV(vm, regs + 3, 15, 1); // push
    U_POP(vm, regs + 1);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// f
static void fun_239(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1656, locals);
    vm.last = {148, 9};
    *(regs + 0) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = *(locals + 1); // t
    *(regs + 2) = Value(1);
    U_BCALLRETV(vm, regs + 3, 15, 1); // push
    U_POP(vm, regs + 1);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// g
static void fun_527(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1661, locals);
    vm.last = {150, 9};
    *(regs + 0) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = *(locals + 1); // u
    (regs + 1)->LTINCRTNIL();
    *(regs + 2) = Value(1);
    U_BCALLRETV(vm, regs + 3, 15, 1); // push
    U_POP(vm, regs + 1);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// g
static void fun_240(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1666, locals);
    vm.last = {150, 9};
    *(regs + 0) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = *(locals + 1); // u
    *(regs + 2) = Value(1);
    U_BCALLRETV(vm, regs + 3, 15, 1); // push
    U_POP(vm, regs + 1);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// h
static void fun_241(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[2];
    *(locals + 0) = *(psp - 1);
    *(locals + 1) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1671, locals);
    vm.last = {152, 9};
    *(regs + 0) = Value(1);
    vm.temp_lval = locals + 1; // x
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {153, 9};
    *(regs + 0) = *(locals + 1); // x
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// i
static void fun_528(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1676, locals);
    vm.last = {155, 9};
    *(regs + 0) = *(locals + 0); // a
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = *(locals + 1); // t
    (regs + 1)->LTINCRTNIL();
    *(regs + 2) = Value(1);
    U_BCALLRETV(vm, regs + 3, 15, 1); // push
    U_POP(vm, regs + 1);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// i
static void fun_242(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1681, locals);
    vm.last = {155, 9};
    *(regs + 0) = *(locals + 0); // a
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = *(locals + 1); // t
    *(regs + 2) = Value(1);
    U_BCALLRETV(vm, regs + 3, 15, 1); // push
    U_POP(vm, regs + 1);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// k
static void fun_244(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1686, locals);
    vm.last = {198, 9};
    *(regs + 0) = Value(1.000000);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// q
static void fun_245(VMRef vm, StackPtr psp) {
    Value regs[2];
    PushFunId(vm, funinfo_table + 1690, 0);
    vm.last = {202, 9};
    *(regs + 0) = Value(1.000000);
    *(regs + 1) = Value(1.000000);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// t0
static void fun_246(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1693, locals);
    vm.last = {206, 9};
    *(regs + 0) = *(locals + 0); // aa
    if (!(regs + 0)->True()) goto block0;
    vm.last = {206, 9};
    *(regs + 0) = *(locals + 0); // aa
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = Value(0);
    U_VPUSHIDXI(vm, regs + 2);
    goto block1;
    block0:;
    vm.last = {206, 9};
    *(regs + 0) = Value(2);
    block1:;
    U_NEWVEC(vm, regs + 1, 7, 1);
    U_PUSHNIL(vm, regs + 1);
    U_NEWVEC(vm, regs + 2, 827, 0);
    U_PUSHNIL(vm, regs + 3);
    U_PUSHNIL(vm, regs + 4);
    U_NEWOBJECT(vm, regs + 5, 804); // Ai
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// p
static void fun_250(VMRef vm, StackPtr psp) {
    Value regs[1];
    PushFunId(vm, funinfo_table + 1697, 0);
    vm.last = {216, 9};
    *(regs + 0) = Value(3);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// t1
static void fun_251(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1700, locals);
    vm.last = {221, 9};
    *(regs + 0) = *(locals + 0); // aa
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = Value(0);
    U_VPUSHIDXI(vm, regs + 2);
    U_NEWVEC(vm, regs + 1, 7, 1);
    U_PUSHNIL(vm, regs + 1);
    U_NEWVEC(vm, regs + 2, 827, 0);
    U_PUSHNIL(vm, regs + 3);
    U_PUSHNIL(vm, regs + 4);
    U_NEWOBJECT(vm, regs + 5, 804); // Ai
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// t2
static void fun_252(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1704, locals);
    vm.last = {224, 9};
    *(regs + 0) = *(locals + 0); // aa
    if (!(regs + 0)->True()) goto block0;
    vm.last = {224, 9};
    *(regs + 0) = *(locals + 0); // aa
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = Value(0);
    U_VPUSHIDXI(vm, regs + 2);
    goto block1;
    block0:;
    vm.last = {224, 9};
    *(regs + 0) = Value(2);
    block1:;
    U_NEWVEC(vm, regs + 1, 7, 1);
    U_PUSHNIL(vm, regs + 1);
    U_NEWVEC(vm, regs + 2, 827, 0);
    U_PUSHNIL(vm, regs + 3);
    U_PUSHNIL(vm, regs + 4);
    U_NEWOBJECT(vm, regs + 5, 804); // Ai
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function234
static void fun_253(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[2];
    *(locals + 0) = Value(0, lobster::V_NIL);
    *(locals + 1) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1708, locals);
    vm.last = {229, 9};
    vm.last = {231, 9};
    vm.last = {233, 9};
    *(regs + 0) = Value(5);
    fun_531(vm, regs + 1); // call: dictionary
    U_NEWOBJECT(vm, regs + 1, 854); // set1
    vm.temp_lval = locals + 0; // a1
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {234, 9};
    *(regs + 0) = Value(15);
    fun_531(vm, regs + 1); // call: dictionary
    U_NEWOBJECT(vm, regs + 1, 863); // set2
    vm.temp_lval = locals + 1; // a2
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {235, 9};
    *(regs + 0) = *(locals + 0); // a1
    U_PUSHFLD(vm, regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 440); // "a"
    *(regs + 2) = Value(13);
    fun_534(vm, regs + 3); // call: set
    vm.last = {236, 9};
    *(regs + 0) = *(locals + 1); // a2
    U_PUSHFLD(vm, regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 441); // "a"
    *(regs + 2) = Value(13);
    fun_534(vm, regs + 3); // call: set
    vm.last = {237, 9};
    *(regs + 0) = *(locals + 0); // a1
    U_PUSHFLD(vm, regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 442); // "a"
    *(regs + 2) = Value(0);
    fun_536(vm, regs + 3); // call: get
    *(regs + 1) = *(locals + 1); // a2
    U_PUSHFLD(vm, regs + 2, 0);
    U_PUSHSTR(vm, regs + 2, 443); // "a"
    *(regs + 3) = Value(0);
    fun_536(vm, regs + 4); // call: get
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 237, 9, 444);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function235
static void fun_254(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[1];
    *(locals + 0) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1713, locals);
    vm.last = {241, 9};
    vm.last = {242, 9};
    vm.last = {243, 9};
    vm.last = {243, 9};
    *(regs + 0) = Value(1);
    *(regs + 1) = Value(2);
    *(regs + 2) = Value(3);
    U_NEWVEC(vm, regs + 3, 7, 3);
    vm.temp_lval = locals + 0; // xs
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {242, 9};
    *(regs + 0) = Value(2);
    *(regs + 1) = Value(2);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 243, 9, 445);
    DecVal(vm, locals[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function237
static void fun_257(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[3];
    *(locals + 0) = Value(0, lobster::V_NIL);
    *(locals + 1) = Value(0, lobster::V_NIL);
    *(locals + 2) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1717, locals);
    vm.last = {248, 9};
    vm.last = {248, 9};
    vm.last = {250, 9};
    vm.last = {250, 9};
    vm.last = {252, 9};
    *(regs + 0) = Value(1);
    U_NEWOBJECT(vm, regs + 1, 872); // A
    U_NEWVEC(vm, regs + 1, 881, 1);
    vm.temp_lval = locals + 0; // as
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {253, 9};
    *(regs + 0) = Value(1);
    *(regs + 1) = Value(2);
    U_NEWOBJECT(vm, regs + 2, 883); // B
    U_NEWVEC(vm, regs + 1, 895, 1);
    vm.temp_lval = locals + 1; // bs
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {255, 9};
    *(regs + 0) = *(locals + 0); // as
    *(regs + 1) = *(locals + 1); // bs
    U_BCALLRET2(vm, regs + 2, 8, 1); // append
    U_POPREF(vm, regs + 1);
    vm.last = {256, 9};
    *(regs + 0) = *(locals + 1); // bs
    *(regs + 1) = *(locals + 0); // as
    U_BCALLRET2(vm, regs + 2, 8, 1); // append
    U_POPREF(vm, regs + 1);
    vm.last = {258, 9};
    *(regs + 0) = Value(1);
    *(regs + 1) = Value(2);
    U_NEWOBJECT(vm, regs + 2, 883); // B
    U_NEWVEC(vm, regs + 1, 881, 1);
    vm.temp_lval = locals + 2; // xs
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {260, 9};
    vm.last = {261, 9};
    *(regs + 0) = Value(1);
    *(regs + 1) = Value(2);
    U_NEWOBJECT(vm, regs + 2, 883); // B
    U_NEWVEC(vm, regs + 1, 895, 1);
    fun_258(vm, regs + 1); // call: f
    vm.last = {263, 9};
    *(regs + 0) = *(locals + 0); // as
    *(regs + 1) = Value(0);
    U_VPUSHIDXI(vm, regs + 2);
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = Value(1);
    U_IEQ(vm, regs + 2);
    if (!(regs + 0)->True()) goto block0;
    vm.last = {263, 9};
    *(regs + 0) = Value(1);
    U_NEWOBJECT(vm, regs + 1, 872); // A
    U_NEWVEC(vm, regs + 1, 881, 1);
    goto block1;
    block0:;
    vm.last = {263, 9};
    *(regs + 0) = Value(1);
    *(regs + 1) = Value(2);
    U_NEWOBJECT(vm, regs + 2, 883); // B
    U_NEWVEC(vm, regs + 1, 895, 1);
    block1:;
    fun_258(vm, regs + 1); // call: f
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[2]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// f
static void fun_258(VMRef vm, StackPtr psp) {
    Value regs[1];
    (void)regs;
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1723, locals);
    vm.last = {260, 9};
    DecVal(vm, locals[0]);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function239
static void fun_259(VMRef vm, StackPtr psp) {
    Value regs[1];
    (void)regs;
    PushFunId(vm, funinfo_table + 1727, 0);
    vm.last = {267, 9};
    vm.last = {267, 9};
    vm.last = {270, 9};
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function240
static void fun_260(VMRef vm, StackPtr psp) {
    Value regs[6];
    Value locals[1];
    *(locals + 0) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1730, locals);
    vm.last = {274, 9};
    vm.last = {274, 9};
    vm.last = {279, 9};
    U_PUSHNIL(vm, regs + 0);
    *(regs + 1) = Value(1);
    U_PUSHVARVF(vm, regs + 2, 123, 2); // int2_0+0
    U_NEWOBJECT(vm, regs + 4, 897); // A
    U_PUSHNIL(vm, regs + 1);
    *(regs + 2) = Value(1);
    U_PUSHVARVF(vm, regs + 3, 123, 2); // int2_0+0
    U_NEWOBJECT(vm, regs + 5, 897); // A
    *(regs + 2) = Value(1);
    U_PUSHVARVF(vm, regs + 3, 123, 2); // int2_0+0
    U_NEWOBJECT(vm, regs + 5, 897); // A
    U_PUSHNIL(vm, regs + 2);
    *(regs + 3) = Value(2);
    *(regs + 4) = Value(3);
    *(regs + 5) = Value(4);
    U_NEWOBJECT(vm, regs + 6, 897); // A
    *(regs + 3) = Value(1);
    U_PUSHVARVF(vm, regs + 4, 123, 2); // int2_0+0
    U_NEWOBJECT(vm, regs + 6, 897); // A
    U_NEWVEC(vm, regs + 3, 917, 3);
    vm.temp_lval = locals + 0; // as
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {284, 9};
    *(regs + 0) = *(locals + 0); // as
    *(regs + 1) = Value(2);
    U_VPUSHIDXI(vm, regs + 2);
    U_PUSHFLD(vm, regs + 1, 0);
    U_ASSERTR(vm, regs + 1, 284, 9, 446);
    U_PUSHFLD(vm, regs + 1, 3);
    *(regs + 1) = Value(4);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 284, 9, 447);
    vm.last = {286, 9};
    vm.last = {288, 9};
    U_PUSHNIL(vm, regs + 0);
    *(regs + 1) = Value(1);
    U_PUSHVARVF(vm, regs + 2, 123, 2); // int2_0+0
    U_NEWOBJECT(vm, regs + 4, 897); // A
    fun_261(vm, regs + 1); // call: foo
    vm.last = {289, 9};
    U_PUSHNIL(vm, regs + 0);
    *(regs + 1) = Value(1);
    U_PUSHVARVF(vm, regs + 2, 123, 2); // int2_0+0
    U_NEWOBJECT(vm, regs + 4, 897); // A
    *(regs + 1) = Value(1);
    U_PUSHVARVF(vm, regs + 2, 123, 2); // int2_0+0
    U_NEWOBJECT(vm, regs + 4, 897); // A
    fun_261(vm, regs + 1); // call: foo
    vm.last = {290, 9};
    U_PUSHNIL(vm, regs + 0);
    *(regs + 1) = Value(2);
    U_PUSHVARVF(vm, regs + 2, 123, 2); // int2_0+0
    U_NEWOBJECT(vm, regs + 4, 897); // A
    *(regs + 1) = Value(1);
    U_PUSHVARVF(vm, regs + 2, 123, 2); // int2_0+0
    U_NEWOBJECT(vm, regs + 4, 897); // A
    fun_261(vm, regs + 1); // call: foo
    DecVal(vm, locals[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// foo
static void fun_261(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1734, locals);
    vm.last = {287, 9};
    *(regs + 0) = *(locals + 0); // a
    U_PUSHFLD(vm, regs + 1, 1);
    *(regs + 1) = Value(1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 287, 9, 448);
    DecVal(vm, locals[0]);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// astar_generic
static void fun_560(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[5];
    *(locals + 0) = *(psp - 4);
    *(locals + 1) = *(psp - 3);
    *(locals + 2) = *(psp - 2);
    SwapVars(vm, 1404, psp, 1);
    BackupVar(vm, 1405);
    BackupVar(vm, 1406);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1738, locals);
    vm.last = {23, 11};
    *(regs + 0) = *(locals + 0); // startnode
    (regs + 0)->LTINCRTNIL();
    U_NEWVEC(vm, regs + 1, 148, 1);
    U_LVAL_VARF(vm, regs + 1, 1405); // openlist
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {24, 11};
    *(regs + 0) = *(locals + 0); // startnode
    if ((regs + 0)->True()) goto block0;
    U_POP(vm, regs + 1);
    U_PUSHNIL(vm, regs + 0);
    block0:;
    (regs + 0)->LTINCRTNIL();
    U_LVAL_VARF(vm, regs + 1, 1406); // n
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {25, 11};
    block1:;
    U_PUSHVARF(vm, regs + 0, 1406); // n
    U_E2B(vm, regs + 1);
    if (!(regs + 0)->True()) goto block2;
    U_POP(vm, regs + 1);
    U_PUSHVARF(vm, regs + 0, 1406); // n
    fun_278(vm, regs + 1); // call: function258
    U_LOGNOT(vm, regs + 1);
    block2:;
    if (!(regs + 0)->True()) goto block3;
    vm.last = {26, 11};
    U_PUSHVARF(vm, regs + 0, 1405); // openlist
    U_PUSHVARF(vm, regs + 1, 1406); // n
    U_BCALLRET2(vm, regs + 2, 21, 1); // remove_obj
    U_POP(vm, regs + 1);
    vm.last = {27, 11};
    *(regs + 0) = Value(1);
    U_PUSHVARF(vm, regs + 1, 1406); // n
    U_LVAL_FLD(vm, regs + 2, 7);
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {37, 11};
    U_PUSHVARF(vm, regs + 0, 1406); // n
    (regs + 0)->LTINCRTNIL();
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_279(vm, regs + 2); // call: function259
    vm.last = {38, 11};
    U_PUSHNIL(vm, regs + 0);
    U_LVAL_VARF(vm, regs + 1, 1406); // n
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {39, 11};
    *(regs + 0) = Value(-1);
    U_PUSHVARF(vm, regs + 1, 1405); // openlist
    block4:;
    if (!U_VFOR(vm, regs + 2)) goto block5;
    vm.last = {39, 11};
    U_VFORELEMREF(vm, regs + 2);
    vm.temp_lval = locals + 3; // c
    U_LV_WRITEREF(vm, regs + 3);
    vm.last = {40, 11};
    U_PUSHVARF(vm, regs + 2, 1406); // n
    U_LOGNOTREF(vm, regs + 3);
    if ((regs + 2)->True()) goto block6;
    U_POP(vm, regs + 3);
    *(regs + 2) = *(locals + 3); // c
    U_PUSHFLD(vm, regs + 3, 2);
    U_PUSHVARF(vm, regs + 3, 1406); // n
    U_PUSHFLD(vm, regs + 4, 2);
    U_FLT(vm, regs + 4);
    block6:;
    if ((regs + 2)->True()) goto block7;
    U_POP(vm, regs + 3);
    *(regs + 2) = *(locals + 3); // c
    U_PUSHFLD(vm, regs + 3, 2);
    U_PUSHVARF(vm, regs + 3, 1406); // n
    U_PUSHFLD(vm, regs + 4, 2);
    U_FEQ(vm, regs + 4);
    if (!(regs + 2)->True()) goto block8;
    U_POP(vm, regs + 3);
    *(regs + 2) = *(locals + 3); // c
    U_PUSHFLD(vm, regs + 3, 1);
    U_PUSHVARF(vm, regs + 3, 1406); // n
    U_PUSHFLD(vm, regs + 4, 1);
    U_FLT(vm, regs + 4);
    block8:;
    block7:;
    if (!(regs + 2)->True()) goto block9;
    vm.last = {41, 11};
    *(regs + 2) = *(locals + 3); // c
    (regs + 2)->LTINCRTNIL();
    U_LVAL_VARF(vm, regs + 3, 1406); // n
    U_LV_WRITEREF(vm, regs + 3);
    block9:;
    goto block4;
    block5:;
    goto block1;
    block3:;
    vm.last = {42, 11};
    U_NEWVEC(vm, regs + 0, 148, 0);
    vm.temp_lval = locals + 4; // path
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {43, 11};
    block10:;
    U_PUSHVARF(vm, regs + 0, 1406); // n
    if (!(regs + 0)->True()) goto block11;
    vm.last = {44, 11};
    *(regs + 0) = *(locals + 4); // path
    U_PUSHVARF(vm, regs + 1, 1406); // n
    (regs + 1)->LTINCRTNIL();
    *(regs + 2) = Value(1);
    U_BCALLRETV(vm, regs + 3, 15, 1); // push
    U_POP(vm, regs + 1);
    vm.last = {45, 11};
    U_PUSHVARF(vm, regs + 0, 1406); // n
    U_PUSHFLD(vm, regs + 1, 3);
    (regs + 0)->LTINCRTNIL();
    U_LVAL_VARF(vm, regs + 1, 1406); // n
    U_LV_WRITEREF(vm, regs + 1);
    goto block10;
    block11:;
    vm.last = {46, 11};
    *(regs + 0) = *(locals + 4); // path
    DecVal(vm, locals[0]);
    DecOwned(vm, 1405);
    DecOwned(vm, 1406);
    DecVal(vm, locals[3]);
    psp = PopArg(vm, 1404, psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    RestoreBackup(vm, 1406);
    RestoreBackup(vm, 1405);
    PopFunId(vm);
}

// astar_generic
static void fun_549(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[5];
    *(locals + 0) = *(psp - 4);
    *(locals + 1) = *(psp - 3);
    *(locals + 2) = *(psp - 2);
    SwapVars(vm, 1369, psp, 1);
    BackupVar(vm, 1370);
    BackupVar(vm, 1371);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1749, locals);
    vm.last = {23, 11};
    *(regs + 0) = *(locals + 0); // startnode
    (regs + 0)->LTINCRTNIL();
    U_NEWVEC(vm, regs + 1, 1005, 1);
    U_LVAL_VARF(vm, regs + 1, 1370); // openlist
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {24, 11};
    *(regs + 0) = *(locals + 0); // startnode
    if ((regs + 0)->True()) goto block0;
    U_POP(vm, regs + 1);
    U_PUSHNIL(vm, regs + 0);
    block0:;
    (regs + 0)->LTINCRTNIL();
    U_LVAL_VARF(vm, regs + 1, 1371); // n
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {25, 11};
    block1:;
    U_PUSHVARF(vm, regs + 0, 1371); // n
    U_E2B(vm, regs + 1);
    if (!(regs + 0)->True()) goto block2;
    U_POP(vm, regs + 1);
    U_PUSHVARF(vm, regs + 0, 1371); // n
    fun_550(vm, regs + 1); // call: function246
    U_LOGNOT(vm, regs + 1);
    block2:;
    if (!(regs + 0)->True()) goto block3;
    vm.last = {26, 11};
    U_PUSHVARF(vm, regs + 0, 1370); // openlist
    U_PUSHVARF(vm, regs + 1, 1371); // n
    U_BCALLRET2(vm, regs + 2, 21, 1); // remove_obj
    U_POP(vm, regs + 1);
    vm.last = {27, 11};
    *(regs + 0) = Value(1);
    U_PUSHVARF(vm, regs + 1, 1371); // n
    U_LVAL_FLD(vm, regs + 2, 9);
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {37, 11};
    U_PUSHVARF(vm, regs + 0, 1371); // n
    (regs + 0)->LTINCRTNIL();
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_552(vm, regs + 2); // call: function247
    vm.last = {38, 11};
    U_PUSHNIL(vm, regs + 0);
    U_LVAL_VARF(vm, regs + 1, 1371); // n
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {39, 11};
    *(regs + 0) = Value(-1);
    U_PUSHVARF(vm, regs + 1, 1370); // openlist
    block4:;
    if (!U_VFOR(vm, regs + 2)) goto block5;
    vm.last = {39, 11};
    U_VFORELEMREF(vm, regs + 2);
    vm.temp_lval = locals + 3; // c
    U_LV_WRITEREF(vm, regs + 3);
    vm.last = {40, 11};
    U_PUSHVARF(vm, regs + 2, 1371); // n
    U_LOGNOTREF(vm, regs + 3);
    if ((regs + 2)->True()) goto block6;
    U_POP(vm, regs + 3);
    *(regs + 2) = *(locals + 3); // c
    U_PUSHFLD(vm, regs + 3, 2);
    U_PUSHVARF(vm, regs + 3, 1371); // n
    U_PUSHFLD(vm, regs + 4, 2);
    U_FLT(vm, regs + 4);
    block6:;
    if ((regs + 2)->True()) goto block7;
    U_POP(vm, regs + 3);
    *(regs + 2) = *(locals + 3); // c
    U_PUSHFLD(vm, regs + 3, 2);
    U_PUSHVARF(vm, regs + 3, 1371); // n
    U_PUSHFLD(vm, regs + 4, 2);
    U_FEQ(vm, regs + 4);
    if (!(regs + 2)->True()) goto block8;
    U_POP(vm, regs + 3);
    *(regs + 2) = *(locals + 3); // c
    U_PUSHFLD(vm, regs + 3, 1);
    U_PUSHVARF(vm, regs + 3, 1371); // n
    U_PUSHFLD(vm, regs + 4, 1);
    U_FLT(vm, regs + 4);
    block8:;
    block7:;
    if (!(regs + 2)->True()) goto block9;
    vm.last = {41, 11};
    *(regs + 2) = *(locals + 3); // c
    (regs + 2)->LTINCRTNIL();
    U_LVAL_VARF(vm, regs + 3, 1371); // n
    U_LV_WRITEREF(vm, regs + 3);
    block9:;
    goto block4;
    block5:;
    goto block1;
    block3:;
    vm.last = {42, 11};
    U_NEWVEC(vm, regs + 0, 1005, 0);
    vm.temp_lval = locals + 4; // path
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {43, 11};
    block10:;
    U_PUSHVARF(vm, regs + 0, 1371); // n
    if (!(regs + 0)->True()) goto block11;
    vm.last = {44, 11};
    *(regs + 0) = *(locals + 4); // path
    U_PUSHVARF(vm, regs + 1, 1371); // n
    (regs + 1)->LTINCRTNIL();
    *(regs + 2) = Value(1);
    U_BCALLRETV(vm, regs + 3, 15, 1); // push
    U_POP(vm, regs + 1);
    vm.last = {45, 11};
    U_PUSHVARF(vm, regs + 0, 1371); // n
    U_PUSHFLD(vm, regs + 1, 3);
    (regs + 0)->LTINCRTNIL();
    U_LVAL_VARF(vm, regs + 1, 1371); // n
    U_LV_WRITEREF(vm, regs + 1);
    goto block10;
    block11:;
    vm.last = {46, 11};
    *(regs + 0) = *(locals + 4); // path
    DecVal(vm, locals[0]);
    DecOwned(vm, 1370);
    DecOwned(vm, 1371);
    DecVal(vm, locals[3]);
    psp = PopArg(vm, 1369, psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    RestoreBackup(vm, 1371);
    RestoreBackup(vm, 1370);
    PopFunId(vm);
}

// astar_generic
static void fun_263(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[5];
    *(locals + 0) = *(psp - 4);
    *(locals + 1) = *(psp - 3);
    *(locals + 2) = *(psp - 2);
    SwapVars(vm, 477, psp, 1);
    BackupVar(vm, 478);
    BackupVar(vm, 479);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1760, locals);
    vm.last = {23, 11};
    *(regs + 0) = *(locals + 0); // startnode
    (regs + 0)->LTINCRTNIL();
    U_NEWVEC(vm, regs + 1, 1005, 1);
    U_LVAL_VARF(vm, regs + 1, 478); // openlist
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {24, 11};
    *(regs + 0) = *(locals + 0); // startnode
    if ((regs + 0)->True()) goto block0;
    U_POP(vm, regs + 1);
    U_PUSHNIL(vm, regs + 0);
    block0:;
    (regs + 0)->LTINCRTNIL();
    U_LVAL_VARF(vm, regs + 1, 479); // n
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {25, 11};
    block1:;
    U_PUSHVARF(vm, regs + 0, 479); // n
    U_E2B(vm, regs + 1);
    if (!(regs + 0)->True()) goto block2;
    U_POP(vm, regs + 1);
    U_PUSHVARF(vm, regs + 0, 479); // n
    fun_266(vm, regs + 1); // call: function246
    U_LOGNOT(vm, regs + 1);
    block2:;
    if (!(regs + 0)->True()) goto block3;
    vm.last = {26, 11};
    U_PUSHVARF(vm, regs + 0, 478); // openlist
    U_PUSHVARF(vm, regs + 1, 479); // n
    U_BCALLRET2(vm, regs + 2, 21, 1); // remove_obj
    U_POP(vm, regs + 1);
    vm.last = {27, 11};
    *(regs + 0) = Value(1);
    U_PUSHVARF(vm, regs + 1, 479); // n
    U_LVAL_FLD(vm, regs + 2, 9);
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {37, 11};
    U_PUSHVARF(vm, regs + 0, 479); // n
    (regs + 0)->LTINCRTNIL();
    U_PUSHFUN(vm, regs + 1, 0, fun_264);
    fun_267(vm, regs + 2); // call: function247
    vm.last = {38, 11};
    U_PUSHNIL(vm, regs + 0);
    U_LVAL_VARF(vm, regs + 1, 479); // n
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {39, 11};
    *(regs + 0) = Value(-1);
    U_PUSHVARF(vm, regs + 1, 478); // openlist
    block4:;
    if (!U_VFOR(vm, regs + 2)) goto block5;
    vm.last = {39, 11};
    U_VFORELEMREF(vm, regs + 2);
    vm.temp_lval = locals + 3; // c
    U_LV_WRITEREF(vm, regs + 3);
    vm.last = {40, 11};
    U_PUSHVARF(vm, regs + 2, 479); // n
    U_LOGNOTREF(vm, regs + 3);
    if ((regs + 2)->True()) goto block6;
    U_POP(vm, regs + 3);
    *(regs + 2) = *(locals + 3); // c
    U_PUSHFLD(vm, regs + 3, 2);
    U_PUSHVARF(vm, regs + 3, 479); // n
    U_PUSHFLD(vm, regs + 4, 2);
    U_FLT(vm, regs + 4);
    block6:;
    if ((regs + 2)->True()) goto block7;
    U_POP(vm, regs + 3);
    *(regs + 2) = *(locals + 3); // c
    U_PUSHFLD(vm, regs + 3, 2);
    U_PUSHVARF(vm, regs + 3, 479); // n
    U_PUSHFLD(vm, regs + 4, 2);
    U_FEQ(vm, regs + 4);
    if (!(regs + 2)->True()) goto block8;
    U_POP(vm, regs + 3);
    *(regs + 2) = *(locals + 3); // c
    U_PUSHFLD(vm, regs + 3, 1);
    U_PUSHVARF(vm, regs + 3, 479); // n
    U_PUSHFLD(vm, regs + 4, 1);
    U_FLT(vm, regs + 4);
    block8:;
    block7:;
    if (!(regs + 2)->True()) goto block9;
    vm.last = {41, 11};
    *(regs + 2) = *(locals + 3); // c
    (regs + 2)->LTINCRTNIL();
    U_LVAL_VARF(vm, regs + 3, 479); // n
    U_LV_WRITEREF(vm, regs + 3);
    block9:;
    goto block4;
    block5:;
    goto block1;
    block3:;
    vm.last = {42, 11};
    U_NEWVEC(vm, regs + 0, 1005, 0);
    vm.temp_lval = locals + 4; // path
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {43, 11};
    block10:;
    U_PUSHVARF(vm, regs + 0, 479); // n
    if (!(regs + 0)->True()) goto block11;
    vm.last = {44, 11};
    *(regs + 0) = *(locals + 4); // path
    U_PUSHVARF(vm, regs + 1, 479); // n
    (regs + 1)->LTINCRTNIL();
    *(regs + 2) = Value(1);
    U_BCALLRETV(vm, regs + 3, 15, 1); // push
    U_POP(vm, regs + 1);
    vm.last = {45, 11};
    U_PUSHVARF(vm, regs + 0, 479); // n
    U_PUSHFLD(vm, regs + 1, 3);
    (regs + 0)->LTINCRTNIL();
    U_LVAL_VARF(vm, regs + 1, 479); // n
    U_LV_WRITEREF(vm, regs + 1);
    goto block10;
    block11:;
    vm.last = {46, 11};
    *(regs + 0) = *(locals + 4); // path
    DecVal(vm, locals[0]);
    DecOwned(vm, 478);
    DecOwned(vm, 479);
    DecVal(vm, locals[3]);
    psp = PopArg(vm, 477, psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    RestoreBackup(vm, 479);
    RestoreBackup(vm, 478);
    PopFunId(vm);
}

// function244
static void fun_561(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[4];
    *(locals + 0) = *(psp - 3);
    *(locals + 1) = *(psp - 2);
    *(locals + 2) = *(psp - 1);
    *(locals + 3) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1771, locals);
    vm.last = {29, 11};
    *(regs + 0) = *(locals + 2); // nn
    U_PUSHFLD(vm, regs + 1, 7);
    U_LOGNOT(vm, regs + 1);
    if (!(regs + 0)->True()) goto block0;
    vm.last = {30, 11};
    U_PUSHVARF(vm, regs + 0, 1406); // n
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = *(locals + 1); // cost
    U_I2F(vm, regs + 2);
    U_FADD(vm, regs + 2);
    vm.temp_lval = locals + 3; // G
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {31, 11};
    *(regs + 0) = *(locals + 2); // nn
    U_PUSHFLD(vm, regs + 1, 6);
    U_LOGNOT(vm, regs + 1);
    if (!(regs + 0)->True()) goto block1;
    U_POP(vm, regs + 1);
    U_PUSHVARF(vm, regs + 0, 1405); // openlist
    *(regs + 1) = *(locals + 2); // nn
    (regs + 1)->LTINCRTNIL();
    *(regs + 2) = Value(1);
    U_BCALLRETV(vm, regs + 3, 15, 1); // push
    block1:;
    U_E2B(vm, regs + 1);
    if ((regs + 0)->True()) goto block2;
    U_POP(vm, regs + 1);
    *(regs + 0) = *(locals + 3); // G
    *(regs + 1) = *(locals + 2); // nn
    U_PUSHFLD(vm, regs + 2, 0);
    U_FLT(vm, regs + 2);
    block2:;
    if (!(regs + 0)->True()) goto block3;
    vm.last = {32, 11};
    *(regs + 0) = Value(1);
    *(regs + 1) = *(locals + 2); // nn
    U_LVAL_FLD(vm, regs + 2, 6);
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {33, 11};
    *(regs + 0) = *(locals + 0); // delta
    (regs + 0)->LTINCRTNIL();
    *(regs + 1) = *(locals + 2); // nn
    U_LVAL_FLD(vm, regs + 2, 5);
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {34, 11};
    U_PUSHVARF(vm, regs + 0, 1406); // n
    (regs + 0)->LTINCRTNIL();
    *(regs + 1) = *(locals + 2); // nn
    U_LVAL_FLD(vm, regs + 2, 3);
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {35, 11};
    *(regs + 0) = *(locals + 2); // nn
    U_PUSHFLD(vm, regs + 1, 4);
    fun_281(vm, regs + 1); // call: function261
    U_I2F(vm, regs + 1);
    *(regs + 1) = *(locals + 2); // nn
    U_LVAL_FLD(vm, regs + 2, 1);
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {36, 11};
    *(regs + 0) = *(locals + 3); // G
    *(regs + 1) = *(locals + 2); // nn
    U_LVAL_FLD(vm, regs + 2, 0);
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {37, 11};
    *(regs + 0) = *(locals + 3); // G
    *(regs + 1) = *(locals + 2); // nn
    U_PUSHFLD(vm, regs + 2, 1);
    U_FADD(vm, regs + 2);
    *(regs + 1) = *(locals + 2); // nn
    U_LVAL_FLD(vm, regs + 2, 2);
    *(vm.temp_lval) = *(regs + 0);
    block3:;
    block0:;
    DecVal(vm, locals[0]);
    DecVal(vm, locals[2]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function244
static void fun_556(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[5];
    *(locals + 0) = *(psp - 4);
    *(locals + 1) = *(psp - 3);
    *(locals + 2) = *(psp - 2);
    *(locals + 3) = *(psp - 1);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1778, locals);
    vm.last = {29, 11};
    *(regs + 0) = *(locals + 3); // nn
    U_PUSHFLD(vm, regs + 1, 9);
    U_LOGNOT(vm, regs + 1);
    if (!(regs + 0)->True()) goto block0;
    vm.last = {30, 11};
    U_PUSHVARF(vm, regs + 0, 1371); // n
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = *(locals + 2); // cost
    U_I2F(vm, regs + 2);
    U_FADD(vm, regs + 2);
    vm.temp_lval = locals + 4; // G
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {31, 11};
    *(regs + 0) = *(locals + 3); // nn
    U_PUSHFLD(vm, regs + 1, 8);
    U_LOGNOT(vm, regs + 1);
    if (!(regs + 0)->True()) goto block1;
    U_POP(vm, regs + 1);
    U_PUSHVARF(vm, regs + 0, 1370); // openlist
    *(regs + 1) = *(locals + 3); // nn
    (regs + 1)->LTINCRTNIL();
    *(regs + 2) = Value(1);
    U_BCALLRETV(vm, regs + 3, 15, 1); // push
    block1:;
    U_E2B(vm, regs + 1);
    if ((regs + 0)->True()) goto block2;
    U_POP(vm, regs + 1);
    *(regs + 0) = *(locals + 4); // G
    *(regs + 1) = *(locals + 3); // nn
    U_PUSHFLD(vm, regs + 2, 0);
    U_FLT(vm, regs + 2);
    block2:;
    if (!(regs + 0)->True()) goto block3;
    vm.last = {32, 11};
    *(regs + 0) = Value(1);
    *(regs + 1) = *(locals + 3); // nn
    U_LVAL_FLD(vm, regs + 2, 8);
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {33, 11};
    *(regs + 0) = *(locals + 0); // delta.0
    *(regs + 1) = *(locals + 1); // delta.1
    *(regs + 2) = *(locals + 3); // nn
    U_LVAL_FLD(vm, regs + 3, 6);
    U_LV_WRITEV(vm, regs + 2, 2);
    vm.last = {34, 11};
    U_PUSHVARF(vm, regs + 0, 1371); // n
    (regs + 0)->LTINCRTNIL();
    *(regs + 1) = *(locals + 3); // nn
    U_LVAL_FLD(vm, regs + 2, 3);
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {35, 11};
    *(regs + 0) = *(locals + 3); // nn
    U_PUSHFLD2V(vm, regs + 1, 4, 2);
    fun_557(vm, regs + 2); // call: function249
    U_I2F(vm, regs + 1);
    *(regs + 1) = *(locals + 3); // nn
    U_LVAL_FLD(vm, regs + 2, 1);
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {36, 11};
    *(regs + 0) = *(locals + 4); // G
    *(regs + 1) = *(locals + 3); // nn
    U_LVAL_FLD(vm, regs + 2, 0);
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {37, 11};
    *(regs + 0) = *(locals + 4); // G
    *(regs + 1) = *(locals + 3); // nn
    U_PUSHFLD(vm, regs + 2, 1);
    U_FADD(vm, regs + 2);
    *(regs + 1) = *(locals + 3); // nn
    U_LVAL_FLD(vm, regs + 2, 2);
    *(vm.temp_lval) = *(regs + 0);
    block3:;
    block0:;
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function244
static void fun_264(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[5];
    *(locals + 0) = *(psp - 4);
    *(locals + 1) = *(psp - 3);
    *(locals + 2) = *(psp - 2);
    *(locals + 3) = *(psp - 1);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1786, locals);
    vm.last = {29, 11};
    *(regs + 0) = *(locals + 3); // nn
    U_PUSHFLD(vm, regs + 1, 9);
    U_LOGNOT(vm, regs + 1);
    if (!(regs + 0)->True()) goto block0;
    vm.last = {30, 11};
    U_PUSHVARF(vm, regs + 0, 479); // n
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = *(locals + 2); // cost
    U_I2F(vm, regs + 2);
    U_FADD(vm, regs + 2);
    vm.temp_lval = locals + 4; // G
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {31, 11};
    *(regs + 0) = *(locals + 3); // nn
    U_PUSHFLD(vm, regs + 1, 8);
    U_LOGNOT(vm, regs + 1);
    if (!(regs + 0)->True()) goto block1;
    U_POP(vm, regs + 1);
    U_PUSHVARF(vm, regs + 0, 478); // openlist
    *(regs + 1) = *(locals + 3); // nn
    (regs + 1)->LTINCRTNIL();
    *(regs + 2) = Value(1);
    U_BCALLRETV(vm, regs + 3, 15, 1); // push
    block1:;
    U_E2B(vm, regs + 1);
    if ((regs + 0)->True()) goto block2;
    U_POP(vm, regs + 1);
    *(regs + 0) = *(locals + 4); // G
    *(regs + 1) = *(locals + 3); // nn
    U_PUSHFLD(vm, regs + 2, 0);
    U_FLT(vm, regs + 2);
    block2:;
    if (!(regs + 0)->True()) goto block3;
    vm.last = {32, 11};
    *(regs + 0) = Value(1);
    *(regs + 1) = *(locals + 3); // nn
    U_LVAL_FLD(vm, regs + 2, 8);
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {33, 11};
    *(regs + 0) = *(locals + 0); // delta.0
    *(regs + 1) = *(locals + 1); // delta.1
    *(regs + 2) = *(locals + 3); // nn
    U_LVAL_FLD(vm, regs + 3, 6);
    U_LV_WRITEV(vm, regs + 2, 2);
    vm.last = {34, 11};
    U_PUSHVARF(vm, regs + 0, 479); // n
    (regs + 0)->LTINCRTNIL();
    *(regs + 1) = *(locals + 3); // nn
    U_LVAL_FLD(vm, regs + 2, 3);
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {35, 11};
    *(regs + 0) = *(locals + 3); // nn
    U_PUSHFLD2V(vm, regs + 1, 4, 2);
    fun_269(vm, regs + 2); // call: function249
    *(regs + 1) = *(locals + 3); // nn
    U_LVAL_FLD(vm, regs + 2, 1);
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {36, 11};
    *(regs + 0) = *(locals + 4); // G
    *(regs + 1) = *(locals + 3); // nn
    U_LVAL_FLD(vm, regs + 2, 0);
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {37, 11};
    *(regs + 0) = *(locals + 4); // G
    *(regs + 1) = *(locals + 3); // nn
    U_PUSHFLD(vm, regs + 2, 1);
    U_FADD(vm, regs + 2);
    *(regs + 1) = *(locals + 3); // nn
    U_LVAL_FLD(vm, regs + 2, 2);
    *(vm.temp_lval) = *(regs + 0);
    block3:;
    block0:;
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// astar_graph
static void fun_545(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[1];
    *(locals + 0) = *(psp - 5);
    SwapVars(vm, 1362, psp, 4);
    SwapVars(vm, 1363, psp, 3);
    SwapVars(vm, 1364, psp, 2);
    SwapVars(vm, 1365, psp, 1);
    PushFunId(vm, funinfo_table + 1794, locals);
    vm.last = {63, 11};
    *(regs + 0) = *(locals + 0); // startnode
    (regs + 0)->LTINCRTNIL();
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    U_PUSHFUN(vm, regs + 2, 0, fun_10000001);
    U_PUSHFUN(vm, regs + 3, 0, fun_10000001);
    fun_549(vm, regs + 4); // call: astar_generic
    psp = PopArg(vm, 1365, psp);
    psp = PopArg(vm, 1364, psp);
    psp = PopArg(vm, 1363, psp);
    psp = PopArg(vm, 1362, psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// astar_graph
static void fun_265(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[1];
    *(locals + 0) = *(psp - 5);
    SwapVars(vm, 488, psp, 4);
    SwapVars(vm, 489, psp, 3);
    SwapVars(vm, 490, psp, 2);
    SwapVars(vm, 491, psp, 1);
    PushFunId(vm, funinfo_table + 1802, locals);
    vm.last = {63, 11};
    *(regs + 0) = *(locals + 0); // startnode
    (regs + 0)->LTINCRTNIL();
    U_PUSHFUN(vm, regs + 1, 0, fun_266);
    U_PUSHFUN(vm, regs + 2, 0, fun_267);
    U_PUSHFUN(vm, regs + 3, 0, fun_269);
    fun_263(vm, regs + 4); // call: astar_generic
    psp = PopArg(vm, 491, psp);
    psp = PopArg(vm, 490, psp);
    psp = PopArg(vm, 489, psp);
    psp = PopArg(vm, 488, psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function246
static void fun_550(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1810, locals);
    vm.last = {56, 11};
    *(regs + 0) = *(locals + 0); // n
    U_PUSHVARF(vm, regs + 1, 1362); // endnode
    U_AEQ(vm, regs + 2);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function246
static void fun_266(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1814, locals);
    vm.last = {56, 11};
    *(regs + 0) = *(locals + 0); // n
    U_PUSHVARF(vm, regs + 1, 488); // endnode
    U_AEQ(vm, regs + 2);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function247
static void fun_552(VMRef vm, StackPtr psp) {
    Value regs[2];
    SwapVars(vm, 1375, psp, 2);
    SwapVars(vm, 1376, psp, 1);
    PushFunId(vm, funinfo_table + 1818, 0);
    vm.last = {61, 11};
    U_PUSHVARF(vm, regs + 0, 1375); // n
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_554(vm, regs + 2); // call: function252
    DecOwned(vm, 1375);
    psp = PopArg(vm, 1376, psp);
    psp = PopArg(vm, 1375, psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function247
static void fun_267(VMRef vm, StackPtr psp) {
    Value regs[2];
    SwapVars(vm, 493, psp, 2);
    SwapVars(vm, 494, psp, 1);
    PushFunId(vm, funinfo_table + 1823, 0);
    vm.last = {61, 11};
    U_PUSHVARF(vm, regs + 0, 493); // n
    U_PUSHFUN(vm, regs + 1, 0, fun_268);
    fun_272(vm, regs + 2); // call: function252
    DecOwned(vm, 493);
    psp = PopArg(vm, 494, psp);
    psp = PopArg(vm, 493, psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function248
static void fun_555(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[2];
    *(locals + 0) = *(psp - 1);
    *(locals + 1) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1828, locals);
    vm.last = {59, 11};
    U_PUSHVARF(vm, regs + 0, 1375); // n
    *(regs + 1) = *(locals + 0); // nn
    fun_286(vm, regs + 2); // call: function266
    vm.temp_lval = locals + 1; // cost
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {60, 11};
    *(regs + 0) = *(locals + 1); // cost
    U_I2F(vm, regs + 1);
    *(regs + 1) = Value(0.000000);
    U_FGT(vm, regs + 2);
    if (!(regs + 0)->True()) goto block0;
    vm.last = {61, 11};
    *(regs + 0) = *(locals + 0); // nn
    U_PUSHFLD2V(vm, regs + 1, 4, 2);
    U_PUSHVARF(vm, regs + 2, 1375); // n
    U_PUSHFLD2V(vm, regs + 3, 4, 2);
    U_IVVSUB(vm, regs + 4, 2);
    *(regs + 2) = *(locals + 1); // cost
    *(regs + 3) = *(locals + 0); // nn
    (regs + 3)->LTINCRTNIL();
    fun_556(vm, regs + 4); // call: function244
    block0:;
    DecVal(vm, locals[0]);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function248
static void fun_268(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[2];
    *(locals + 0) = *(psp - 1);
    *(locals + 1) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1833, locals);
    vm.last = {59, 11};
    U_PUSHVARF(vm, regs + 0, 493); // n
    *(regs + 1) = *(locals + 0); // nn
    fun_286(vm, regs + 2); // call: function266
    vm.temp_lval = locals + 1; // cost
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {60, 11};
    *(regs + 0) = *(locals + 1); // cost
    U_I2F(vm, regs + 1);
    *(regs + 1) = Value(0.000000);
    U_FGT(vm, regs + 2);
    if (!(regs + 0)->True()) goto block0;
    vm.last = {61, 11};
    *(regs + 0) = *(locals + 0); // nn
    U_PUSHFLD2V(vm, regs + 1, 4, 2);
    U_PUSHVARF(vm, regs + 2, 493); // n
    U_PUSHFLD2V(vm, regs + 3, 4, 2);
    U_IVVSUB(vm, regs + 4, 2);
    *(regs + 2) = *(locals + 1); // cost
    *(regs + 3) = *(locals + 0); // nn
    (regs + 3)->LTINCRTNIL();
    fun_264(vm, regs + 4); // call: function244
    block0:;
    DecVal(vm, locals[0]);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function249
static void fun_557(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1838, locals);
    vm.last = {63, 11};
    *(regs + 0) = *(locals + 0); // state.0
    *(regs + 1) = *(locals + 1); // state.1
    U_PUSHVARF(vm, regs + 2, 1362); // endnode
    U_PUSHFLD2V(vm, regs + 3, 4, 2);
    U_IVVSUB(vm, regs + 4, 2);
    fun_274(vm, regs + 2); // call: function254
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function249
static void fun_269(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1843, locals);
    vm.last = {63, 11};
    *(regs + 0) = *(locals + 0); // state.0
    *(regs + 1) = *(locals + 1); // state.1
    U_PUSHVARF(vm, regs + 2, 488); // endnode
    U_PUSHFLD2V(vm, regs + 3, 4, 2);
    U_IVVSUB(vm, regs + 4, 2);
    fun_273(vm, regs + 2); // call: function253
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// astar_2dgrid
static void fun_270(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 7);
    SwapVars(vm, 500, psp, 6);
    SwapVars(vm, 501, psp, 5);
    SwapVars(vm, 502, psp, 4);
    SwapVars(vm, 503, psp, 3);
    SwapVars(vm, 504, psp, 2);
    SwapVars(vm, 505, psp, 1);
    BackupVar(vm, 506);
    PushFunId(vm, funinfo_table + 1848, locals);
    vm.last = {68, 11};
    U_PUSHVARF(vm, regs + 0, 170); // cardinal_directions
    (regs + 0)->LTINCRTNIL();
    U_LVAL_VARF(vm, regs + 1, 506); // directions
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {69, 11};
    vm.last = {84, 11};
    *(regs + 0) = *(locals + 0); // isocta
    if (!(regs + 0)->True()) goto block0;
    vm.last = {76, 11};
    U_PUSHVARF(vm, regs + 0, 506); // directions
    U_PUSHVARF(vm, regs + 1, 171); // diagonal_directions
    U_BCALLRET2(vm, regs + 2, 8, 1); // append
    U_LVAL_VARF(vm, regs + 1, 506); // directions
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {77, 11};
    U_PUSHFUN(vm, regs + 0, 0, fun_273);
    fun_271(vm, regs + 1); // call: astar_distance
    goto block1;
    block0:;
    vm.last = {84, 11};
    U_PUSHFUN(vm, regs + 0, 0, fun_274);
    fun_544(vm, regs + 1); // call: astar_distance
    block1:;
    DecOwned(vm, 506);
    psp = PopArg(vm, 505, psp);
    psp = PopArg(vm, 504, psp);
    psp = PopArg(vm, 503, psp);
    psp = PopArg(vm, 502, psp);
    psp = PopArg(vm, 501, psp);
    psp = PopArg(vm, 500, psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    RestoreBackup(vm, 506);
    PopFunId(vm);
}

// astar_distance
static void fun_544(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1859, locals);
    vm.last = {74, 11};
    U_PUSHVARF(vm, regs + 0, 502); // startnode
    U_PUSHVARF(vm, regs + 1, 503); // endnode
    U_PUSHVARF(vm, regs + 2, 505); // costf
    *(regs + 3) = *(locals + 0); // distancef
    U_PUSHFUN(vm, regs + 4, 0, fun_272);
    fun_545(vm, regs + 5); // call: astar_graph
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// astar_distance
static void fun_271(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1863, locals);
    vm.last = {74, 11};
    U_PUSHVARF(vm, regs + 0, 502); // startnode
    U_PUSHVARF(vm, regs + 1, 503); // endnode
    U_PUSHVARF(vm, regs + 2, 505); // costf
    *(regs + 3) = *(locals + 0); // distancef
    U_PUSHFUN(vm, regs + 4, 0, fun_272);
    fun_265(vm, regs + 5); // call: astar_graph
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function252
static void fun_554(VMRef vm, StackPtr psp) {
    Value regs[11];
    Value locals[6];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    *(locals + 5) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1867, locals);
    vm.last = {71, 11};
    *(regs + 0) = Value(-1);
    U_PUSHVARF(vm, regs + 1, 506); // directions
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {71, 11};
    U_VFORELEM2S(vm, regs + 2);
    vm.temp_lval = locals + 2; // delta+0
    U_LV_WRITEV(vm, regs + 4, 2);
    vm.last = {72, 11};
    *(regs + 2) = *(locals + 0); // n
    U_PUSHFLD2V(vm, regs + 3, 4, 2);
    *(regs + 4) = *(locals + 2); // delta.0
    *(regs + 5) = *(locals + 3); // delta.1
    U_IVVADD(vm, regs + 6, 2);
    vm.temp_lval = locals + 4; // np+0
    U_LV_WRITEV(vm, regs + 4, 2);
    vm.last = {73, 11};
    *(regs + 2) = *(locals + 4); // np.0
    *(regs + 3) = *(locals + 5); // np.1
    *(regs + 4) = Value(2);
    U_PUSHVARVF(vm, regs + 5, 500, 2); // gridsize+0
    *(regs + 7) = Value(2);
    *(regs + 8) = Value(0);
    *(regs + 9) = Value(0);
    *(regs + 10) = Value(2);
    U_BCALLRETV(vm, regs + 11, 108, 1); // in_range
    if (!(regs + 2)->True()) goto block2;
    vm.last = {74, 11};
    *(regs + 2) = *(locals + 4); // np.0
    *(regs + 3) = *(locals + 5); // np.1
    fun_285(vm, regs + 4); // call: function265
    fun_555(vm, regs + 3); // call: function248
    block2:;
    goto block0;
    block1:;
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function252
static void fun_272(VMRef vm, StackPtr psp) {
    Value regs[11];
    Value locals[6];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    *(locals + 5) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1876, locals);
    vm.last = {71, 11};
    *(regs + 0) = Value(-1);
    U_PUSHVARF(vm, regs + 1, 506); // directions
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {71, 11};
    U_VFORELEM2S(vm, regs + 2);
    vm.temp_lval = locals + 2; // delta+0
    U_LV_WRITEV(vm, regs + 4, 2);
    vm.last = {72, 11};
    *(regs + 2) = *(locals + 0); // n
    U_PUSHFLD2V(vm, regs + 3, 4, 2);
    *(regs + 4) = *(locals + 2); // delta.0
    *(regs + 5) = *(locals + 3); // delta.1
    U_IVVADD(vm, regs + 6, 2);
    vm.temp_lval = locals + 4; // np+0
    U_LV_WRITEV(vm, regs + 4, 2);
    vm.last = {73, 11};
    *(regs + 2) = *(locals + 4); // np.0
    *(regs + 3) = *(locals + 5); // np.1
    *(regs + 4) = Value(2);
    U_PUSHVARVF(vm, regs + 5, 500, 2); // gridsize+0
    *(regs + 7) = Value(2);
    *(regs + 8) = Value(0);
    *(regs + 9) = Value(0);
    *(regs + 10) = Value(2);
    U_BCALLRETV(vm, regs + 11, 108, 1); // in_range
    if (!(regs + 2)->True()) goto block2;
    vm.last = {74, 11};
    *(regs + 2) = *(locals + 4); // np.0
    *(regs + 3) = *(locals + 5); // np.1
    fun_285(vm, regs + 4); // call: function265
    fun_268(vm, regs + 3); // call: function248
    block2:;
    goto block0;
    block1:;
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function253
static void fun_273(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[6];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    *(locals + 5) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1885, locals);
    vm.last = {78, 11};
    *(regs + 0) = *(locals + 0); // v+0
    U_BCALLRET1(vm, regs + 1, 112, 1); // abs
    vm.temp_lval = locals + 2; // x
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {79, 11};
    *(regs + 0) = *(locals + 1); // v+1
    U_BCALLRET1(vm, regs + 1, 112, 1); // abs
    vm.temp_lval = locals + 3; // y
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {80, 11};
    *(regs + 0) = *(locals + 2); // x
    *(regs + 1) = *(locals + 3); // y
    U_BCALLRET2(vm, regs + 2, 128, 1); // max
    vm.temp_lval = locals + 4; // big
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {81, 11};
    *(regs + 0) = *(locals + 2); // x
    *(regs + 1) = *(locals + 3); // y
    U_BCALLRET2(vm, regs + 2, 120, 1); // min
    vm.temp_lval = locals + 5; // small
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {82, 11};
    *(regs + 0) = Value(2.000000);
    U_BCALLRET1(vm, regs + 1, 56, 1); // sqrt
    *(regs + 1) = *(locals + 5); // small
    U_I2F(vm, regs + 2);
    U_FMUL(vm, regs + 2);
    *(regs + 1) = *(locals + 4); // big
    U_I2F(vm, regs + 2);
    U_FADD(vm, regs + 2);
    *(regs + 1) = *(locals + 5); // small
    U_I2F(vm, regs + 2);
    U_FSUB(vm, regs + 2);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function254
static void fun_274(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1894, locals);
    vm.last = {84, 11};
    *(regs + 0) = *(locals + 0); // _.0
    *(regs + 1) = *(locals + 1); // _.1
    *(regs + 2) = Value(2);
    U_BCALLRETV(vm, regs + 3, 89, 1); // manhattan
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// astar_goap
static void fun_277(VMRef vm, StackPtr psp) {
    Value regs[8];
    Value locals[2];
    SwapVars(vm, 522, psp, 4);
    *(locals + 0) = *(psp - 3);
    SwapVars(vm, 524, psp, 2);
    SwapVars(vm, 525, psp, 1);
    *(locals + 1) = Value(0, lobster::V_NIL);
    BackupVar(vm, 527);
    PushFunId(vm, funinfo_table + 1899, locals);
    vm.last = {97, 11};
    *(regs + 0) = *(locals + 0); // initstate
    fun_302(vm, regs + 1); // call: function272
    vm.temp_lval = locals + 1; // H
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {98, 11};
    *(regs + 0) = Value(0.000000);
    *(regs + 1) = *(locals + 1); // H
    U_I2F(vm, regs + 2);
    *(regs + 2) = *(locals + 1); // H
    U_I2F(vm, regs + 3);
    U_PUSHNIL(vm, regs + 3);
    *(regs + 4) = *(locals + 0); // initstate
    (regs + 4)->LTINCRTNIL();
    U_PUSHNIL(vm, regs + 5);
    *(regs + 6) = Value(0);
    *(regs + 7) = Value(0);
    U_NEWOBJECT(vm, regs + 8, 49); // spec_node
    U_NEWVEC(vm, regs + 1, 148, 1);
    U_LVAL_VARF(vm, regs + 1, 527); // existingnodes
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {111, 11};
    U_PUSHVARF(vm, regs + 0, 527); // existingnodes
    *(regs + 1) = Value(0);
    U_VPUSHIDXI(vm, regs + 2);
    (regs + 0)->LTINCRTNIL();
    U_PUSHFUN(vm, regs + 1, 0, fun_278);
    U_PUSHFUN(vm, regs + 2, 0, fun_279);
    U_PUSHFUN(vm, regs + 3, 0, fun_281);
    fun_560(vm, regs + 4); // call: astar_generic
    DecVal(vm, locals[0]);
    DecOwned(vm, 527);
    psp = PopArg(vm, 525, psp);
    psp = PopArg(vm, 524, psp);
    Pop(psp);
    psp = PopArg(vm, 522, psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    RestoreBackup(vm, 527);
    PopFunId(vm);
}

// function258
static void fun_278(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1908, locals);
    vm.last = {100, 11};
    *(regs + 0) = *(locals + 0); // _
    U_PUSHFLD(vm, regs + 1, 4);
    fun_303(vm, regs + 1); // call: function273
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function259
static void fun_279(VMRef vm, StackPtr psp) {
    Value regs[11];
    Value locals[4];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    *(locals + 2) = Value(0, lobster::V_NIL);
    BackupVar(vm, 532);
    *(locals + 3) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1912, locals);
    vm.last = {102, 11};
    *(regs + 0) = Value(-1);
    U_PUSHVARF(vm, regs + 1, 522); // goapactions
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {102, 11};
    U_VFORELEMREF(vm, regs + 2);
    vm.temp_lval = locals + 2; // act
    U_LV_WRITEREF(vm, regs + 3);
    vm.last = {103, 11};
    *(regs + 2) = *(locals + 2); // act
    (regs + 2)->LTINCRTNIL();
    *(regs + 3) = *(locals + 0); // n
    U_PUSHFLD(vm, regs + 4, 4);
    (regs + 3)->LTINCRTNIL();
    U_DDCALL(vm, regs + 4, 0, 1); vm.next_call_target(vm, regs + 4);
    if (!(regs + 2)->True()) goto block2;
    vm.last = {104, 11};
    *(regs + 2) = *(locals + 0); // n
    U_PUSHFLD(vm, regs + 3, 4);
    U_BCALLRET1(vm, regs + 3, 29, 1); // copy
    U_LVAL_VARF(vm, regs + 3, 532); // nstate
    U_LV_WRITEREF(vm, regs + 3);
    vm.last = {105, 11};
    *(regs + 2) = *(locals + 2); // act
    (regs + 2)->LTINCRTNIL();
    U_PUSHVARF(vm, regs + 3, 532); // nstate
    (regs + 3)->LTINCRTNIL();
    U_DDCALL(vm, regs + 4, 1, 1); vm.next_call_target(vm, regs + 4);
    vm.last = {106, 11};
    U_PUSHVARF(vm, regs + 2, 527); // existingnodes
    U_PUSHFUN(vm, regs + 3, 0, fun_280);
    fun_23(vm, regs + 4); // call: find
    vm.temp_lval = locals + 3; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {107, 11};
    *(regs + 2) = *(locals + 3); // i
    *(regs + 3) = Value(0);
    U_ILT(vm, regs + 4);
    if (!(regs + 2)->True()) goto block3;
    vm.last = {108, 11};
    U_PUSHVARF(vm, regs + 2, 527); // existingnodes
    U_BCALLRET1(vm, regs + 3, 13, 1); // length
    vm.temp_lval = locals + 3; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {109, 11};
    U_PUSHVARF(vm, regs + 2, 527); // existingnodes
    *(regs + 3) = Value(0.000000);
    *(regs + 4) = Value(0.000000);
    *(regs + 5) = Value(0.000000);
    U_PUSHNIL(vm, regs + 6);
    U_PUSHVARF(vm, regs + 7, 532); // nstate
    (regs + 7)->LTINCRTNIL();
    U_PUSHNIL(vm, regs + 8);
    *(regs + 9) = Value(0);
    *(regs + 10) = Value(0);
    U_NEWOBJECT(vm, regs + 11, 49); // spec_node
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    block3:;
    vm.last = {110, 11};
    *(regs + 2) = *(locals + 2); // act
    (regs + 2)->LTINCRTNIL();
    *(regs + 3) = Value(1);
    U_PUSHVARF(vm, regs + 4, 527); // existingnodes
    *(regs + 5) = *(locals + 3); // i
    U_VPUSHIDXI(vm, regs + 6);
    (regs + 4)->LTINCRTNIL();
    fun_561(vm, regs + 5); // call: function244
    block2:;
    goto block0;
    block1:;
    DecVal(vm, locals[0]);
    DecVal(vm, locals[2]);
    DecOwned(vm, 532);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    RestoreBackup(vm, 532);
    PopFunId(vm);
}

// function260
static void fun_280(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1920, locals);
    vm.last = {106, 11};
    *(regs + 0) = *(locals + 0); // _
    U_PUSHFLD(vm, regs + 1, 4);
    U_PUSHVARF(vm, regs + 1, 532); // nstate
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function261
static void fun_281(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1924, locals);
    vm.last = {111, 11};
    *(regs + 0) = *(locals + 0); // _
    fun_302(vm, regs + 1); // call: function272
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function262
static void fun_282(VMRef vm, StackPtr psp) {
    Value regs[12];
    Value locals[4];
    BackupVar(vm, 536);
    *(locals + 0) = Value(0, lobster::V_NIL);
    BackupVar(vm, 538);
    BackupVar(vm, 539);
    BackupVar(vm, 540);
    BackupVar(vm, 541);
    BackupVar(vm, 542);
    BackupVar(vm, 543);
    BackupVar(vm, 544);
    *(locals + 1) = Value(0, lobster::V_NIL);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1928, locals);
    vm.last = {6, 10};
    U_PUSHSTR(vm, regs + 0, 449); // "................................."
    (regs + 0)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 1, 450); // "................S................"
    (regs + 1)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 2, 451); // "........#..............#........."
    (regs + 2)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 3, 452); // "........#..............#/////////"
    (regs + 3)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 4, 453); // "........#..............#........."
    (regs + 4)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 5, 454); // "........#..............#........."
    (regs + 5)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 6, 455); // "........################........."
    (regs + 6)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 7, 456); // "................................."
    (regs + 7)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 8, 457); // "...............///..............."
    (regs + 8)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 9, 458); // ".............../E/..............."
    (regs + 9)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 10, 459); // "...............///..............."
    (regs + 10)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 11, 460); // "................................."
    (regs + 11)->LTINCRTNIL();
    U_NEWVEC(vm, regs + 12, 11, 12);
    U_LVAL_VARF(vm, regs + 1, 536); // initworld
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {21, 10};
    U_PUSHSTR(vm, regs + 0, 461); // "......OXXXXXXXXXXXXXXXXXXXO......"
    (regs + 0)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 1, 462); // ".....OXPPPPPPPPPPXXXXXXXXXXO....."
    (regs + 1)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 2, 463); // ".....OXP#XXXXXXXXXXXXXX#XXXO....."
    (regs + 2)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 3, 464); // ".....OXP#XXXXXXXXXXXXXX#XOO//////"
    (regs + 3)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 4, 465); // ".....OXP#XXXXXXXXXXXXXX#XO......."
    (regs + 4)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 5, 466); // ".....OXP#XXXXXXXXXXXXXX#XO......."
    (regs + 5)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 6, 467); // ".....OXP################XO......."
    (regs + 6)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 7, 468); // ".....OXPPPPPPPPPPXXXXXXXXO......."
    (regs + 7)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 8, 469); // ".....OXXXXXXXXXOPOXXXXXXXO......."
    (regs + 8)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 9, 470); // ".....OXXXXXXXXXOPOXXXXXXXO......."
    (regs + 9)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 10, 471); // "......OXXXXXXXXO//OOOOOOO........"
    (regs + 10)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 11, 472); // ".......OOOOOOOO.................."
    (regs + 11)->LTINCRTNIL();
    U_NEWVEC(vm, regs + 12, 11, 12);
    vm.temp_lval = locals + 0; // expected_result
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {36, 10};
    vm.last = {36, 10};
    vm.last = {40, 10};
    U_PUSHVARF(vm, regs + 0, 536); // initworld
    *(regs + 1) = Value(0);
    U_VPUSHIDXI(vm, regs + 2);
    U_BCALLRET1(vm, regs + 1, 12, 1); // length
    U_PUSHVARF(vm, regs + 1, 536); // initworld
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_LVAL_VARF(vm, regs + 2, 538); // worldsize+0
    U_LV_WRITEV(vm, regs + 2, 2);
    vm.last = {42, 10};
    U_PUSHVARVF(vm, regs + 0, 123, 2); // int2_0+0
    U_LVAL_VARF(vm, regs + 2, 540); // startpos+0
    U_LV_WRITEV(vm, regs + 2, 2);
    vm.last = {43, 10};
    U_PUSHVARVF(vm, regs + 0, 123, 2); // int2_0+0
    U_LVAL_VARF(vm, regs + 2, 542); // endpos+0
    U_LV_WRITEV(vm, regs + 2, 2);
    vm.last = {45, 10};
    U_PUSHVARF(vm, regs + 0, 539); // worldsize+1
    U_PUSHFUN(vm, regs + 1, 0, fun_283);
    fun_542(vm, regs + 2); // call: map
    U_LVAL_VARF(vm, regs + 1, 544); // world
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {51, 10};
    *(regs + 0) = Value(0);
    U_PUSHVARVF(vm, regs + 1, 538, 2); // worldsize+0
    U_PUSHVARF(vm, regs + 3, 544); // world
    U_PUSHVARVF(vm, regs + 4, 540, 2); // startpos+0
    U_VPUSHIDXV(vm, regs + 6, 2);
    U_PUSHVARF(vm, regs + 4, 544); // world
    U_PUSHVARVF(vm, regs + 5, 542, 2); // endpos+0
    U_VPUSHIDXV(vm, regs + 7, 2);
    U_PUSHFUN(vm, regs + 5, 0, fun_285);
    U_PUSHFUN(vm, regs + 6, 0, fun_286);
    fun_270(vm, regs + 7); // call: astar_2dgrid
    vm.temp_lval = locals + 1; // path
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {55, 10};
    *(regs + 0) = *(locals + 1); // path
    U_BCALLRET1(vm, regs + 1, 13, 1); // length
    *(regs + 1) = Value(27);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 55, 10, 473);
    vm.last = {57, 10};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 1); // path
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {57, 10};
    U_VFORELEMREF(vm, regs + 2);
    vm.temp_lval = locals + 2; // n
    U_LV_WRITEREF(vm, regs + 3);
    vm.last = {58, 10};
    *(regs + 2) = Value(1);
    *(regs + 3) = *(locals + 2); // n
    U_LVAL_FLD(vm, regs + 4, 10);
    *(vm.temp_lval) = *(regs + 2);
    goto block0;
    block1:;
    vm.last = {60, 10};
    U_PUSHVARF(vm, regs + 0, 544); // world
    U_PUSHFUN(vm, regs + 1, 0, fun_287);
    fun_558(vm, regs + 2); // call: map
    vm.temp_lval = locals + 3; // astar_result
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {66, 10};
    *(regs + 0) = *(locals + 3); // astar_result
    *(regs + 1) = *(locals + 0); // expected_result
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 66, 10, 474);
    DecOwned(vm, 536);
    DecVal(vm, locals[0]);
    DecOwned(vm, 544);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[2]);
    DecVal(vm, locals[3]);
    goto epilogue;
    epilogue:;
    RestoreBackup(vm, 544);
    RestoreBackup(vm, 543);
    RestoreBackup(vm, 542);
    RestoreBackup(vm, 541);
    RestoreBackup(vm, 540);
    RestoreBackup(vm, 539);
    RestoreBackup(vm, 538);
    RestoreBackup(vm, 536);
    PopFunId(vm);
}

// function263
static void fun_283(VMRef vm, StackPtr psp) {
    Value regs[2];
    SwapVars(vm, 545, psp, 1);
    PushFunId(vm, funinfo_table + 1943, 0);
    vm.last = {45, 10};
    U_PUSHVARF(vm, regs + 0, 538); // worldsize+0
    U_PUSHFUN(vm, regs + 1, 0, fun_284);
    fun_543(vm, regs + 2); // call: map
    psp = PopArg(vm, 545, psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function264
static void fun_284(VMRef vm, StackPtr psp) {
    Value regs[12];
    Value locals[2];
    *(locals + 0) = *(psp - 1);
    *(locals + 1) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1947, locals);
    vm.last = {46, 10};
    U_PUSHVARF(vm, regs + 0, 536); // initworld
    U_PUSHVARF(vm, regs + 1, 545); // y
    U_VPUSHIDXI(vm, regs + 2);
    *(regs + 1) = *(locals + 0); // x
    U_SPUSHIDXI(vm, regs + 2);
    vm.temp_lval = locals + 1; // c
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {47, 10};
    *(regs + 0) = *(locals + 1); // c
    *(regs + 1) = Value(83);
    U_IEQ(vm, regs + 2);
    if (!(regs + 0)->True()) goto block0;
    vm.last = {47, 10};
    *(regs + 0) = *(locals + 0); // x
    U_PUSHVARF(vm, regs + 1, 545); // y
    U_LVAL_VARF(vm, regs + 2, 540); // startpos+0
    U_LV_WRITEV(vm, regs + 2, 2);
    block0:;
    vm.last = {48, 10};
    *(regs + 0) = *(locals + 1); // c
    *(regs + 1) = Value(69);
    U_IEQ(vm, regs + 2);
    if (!(regs + 0)->True()) goto block1;
    vm.last = {48, 10};
    *(regs + 0) = *(locals + 0); // x
    U_PUSHVARF(vm, regs + 1, 545); // y
    U_LVAL_VARF(vm, regs + 2, 542); // endpos+0
    U_LV_WRITEV(vm, regs + 2, 2);
    block1:;
    vm.last = {49, 10};
    *(regs + 0) = Value(0.000000);
    *(regs + 1) = Value(0.000000);
    *(regs + 2) = Value(0.000000);
    U_PUSHNIL(vm, regs + 3);
    *(regs + 4) = *(locals + 0); // x
    U_PUSHVARF(vm, regs + 5, 545); // y
    U_PUSHVARVF(vm, regs + 6, 123, 2); // int2_0+0
    *(regs + 8) = Value(0);
    *(regs + 9) = Value(0);
    *(regs + 10) = Value(0);
    *(regs + 11) = *(locals + 1); // c
    U_NEWOBJECT(vm, regs + 12, 919); // pathingcell
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function265
static void fun_285(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1952, locals);
    vm.last = {52, 10};
    U_PUSHVARF(vm, regs + 0, 544); // world
    *(regs + 1) = *(locals + 0); // _.0
    *(regs + 2) = *(locals + 1); // _.1
    U_VPUSHIDXV(vm, regs + 3, 2);
    (regs + 0)->LTINCRTNIL();
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function266
static void fun_286(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1957, locals);
    vm.last = {53, 10};
    *(regs + 0) = *(locals + 1); // nn
    U_PUSHFLD(vm, regs + 1, 11);
    *(regs + 1) = Value(35);
    U_IEQ(vm, regs + 2);
    if (!(regs + 0)->True()) goto block0;
    U_POP(vm, regs + 1);
    *(regs + 0) = Value(-1);
    block0:;
    if ((regs + 0)->True()) goto block1;
    U_POP(vm, regs + 1);
    *(regs + 0) = *(locals + 1); // nn
    U_PUSHFLD(vm, regs + 1, 11);
    *(regs + 1) = Value(47);
    U_IEQ(vm, regs + 2);
    if (!(regs + 0)->True()) goto block2;
    U_POP(vm, regs + 1);
    *(regs + 0) = Value(5);
    block2:;
    block1:;
    if ((regs + 0)->True()) goto block3;
    U_POP(vm, regs + 1);
    *(regs + 0) = Value(1);
    block3:;
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function267
static void fun_287(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1962, locals);
    vm.last = {60, 10};
    *(regs + 0) = *(locals + 0); // row
    U_PUSHSTR(vm, regs + 1, 475); // ""
    (regs + 1)->LTINCRTNIL();
    U_PUSHFUN(vm, regs + 2, 0, fun_288);
    fun_559(vm, regs + 3); // call: fold
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function268
static void fun_288(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value keepvar[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1966, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    *(keepvar + 1) = Value(0, lobster::V_NIL);
    vm.last = {64, 10};
    *(regs + 0) = *(locals + 0); // accum
    *(regs + 1) = *(locals + 1); // n
    U_PUSHFLD(vm, regs + 2, 10);
    if (!(regs + 1)->True()) goto block0;
    U_POP(vm, regs + 2);
    U_PUSHSTR(vm, regs + 1, 476); // "P"
    block0:;
    if ((regs + 1)->True()) goto block1;
    U_POP(vm, regs + 2);
    *(regs + 1) = *(locals + 1); // n
    U_PUSHFLD(vm, regs + 2, 9);
    if (!(regs + 1)->True()) goto block2;
    U_POP(vm, regs + 2);
    U_PUSHSTR(vm, regs + 1, 477); // "X"
    block2:;
    block1:;
    if ((regs + 1)->True()) goto block3;
    U_POP(vm, regs + 2);
    *(regs + 1) = *(locals + 1); // n
    U_PUSHFLD(vm, regs + 2, 8);
    if (!(regs + 1)->True()) goto block4;
    U_POP(vm, regs + 2);
    U_PUSHSTR(vm, regs + 1, 478); // "O"
    block4:;
    block3:;
    (regs + 1)->LTINCRTNIL();
    if ((regs + 1)->True()) goto block5;
    U_POP(vm, regs + 2);
    *(regs + 1) = *(locals + 1); // n
    U_PUSHFLD(vm, regs + 2, 11);
    U_NEWVEC(vm, regs + 2, 7, 1);
    *(keepvar + 0) = *(regs + 1);
    U_BCALLRET1(vm, regs + 2, 43, 1); // unicode_to_string
    block5:;
    *(keepvar + 1) = *(regs + 1);
    U_SADD(vm, regs + 2);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    DecVal(vm, keepvar[1]);
    PopFunId(vm);
}

// function269
static void fun_289(VMRef vm, StackPtr psp) {
    Value regs[9];
    Value keepvar[2];
    Value locals[2];
    *(locals + 0) = Value(0, lobster::V_NIL);
    *(locals + 1) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 1971, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    *(keepvar + 1) = Value(0, lobster::V_NIL);
    vm.last = {6, 12};
    vm.last = {6, 12};
    vm.last = {16, 12};
    vm.last = {18, 12};
    vm.last = {15, 12};
    vm.last = {15, 12};
    vm.last = {23, 12};
    vm.last = {25, 12};
    vm.last = {22, 12};
    vm.last = {22, 12};
    vm.last = {30, 12};
    vm.last = {32, 12};
    vm.last = {29, 12};
    vm.last = {29, 12};
    vm.last = {37, 12};
    vm.last = {39, 12};
    vm.last = {36, 12};
    vm.last = {36, 12};
    vm.last = {44, 12};
    vm.last = {46, 12};
    vm.last = {43, 12};
    vm.last = {43, 12};
    vm.last = {51, 12};
    vm.last = {53, 12};
    vm.last = {50, 12};
    vm.last = {50, 12};
    vm.last = {60, 12};
    U_NEWOBJECT(vm, regs + 0, 1029); // KillWolf
    U_NEWOBJECT(vm, regs + 1, 1035); // SellSkin
    U_NEWOBJECT(vm, regs + 2, 1041); // BuyPizza
    U_NEWOBJECT(vm, regs + 3, 1047); // BuyFlour
    U_NEWOBJECT(vm, regs + 4, 1053); // BakeBread
    U_NEWOBJECT(vm, regs + 5, 1059); // Eat
    U_NEWVEC(vm, regs + 6, 1019, 6);
    vm.temp_lval = locals + 0; // goapactions
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {69, 12};
    vm.last = {71, 12};
    *(regs + 0) = *(locals + 0); // goapactions
    *(regs + 1) = Value(3);
    *(regs + 2) = Value(0);
    *(regs + 3) = Value(0);
    *(regs + 4) = Value(0);
    *(regs + 5) = Value(0);
    *(regs + 6) = Value(0);
    *(regs + 7) = Value(2);
    U_NEWOBJECT(vm, regs + 8, 111); // resources
    U_PUSHFUN(vm, regs + 2, 0, fun_302);
    U_PUSHFUN(vm, regs + 3, 0, fun_303);
    fun_277(vm, regs + 4); // call: astar_goap
    vm.temp_lval = locals + 1; // goap_path
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {77, 12};
    *(regs + 0) = *(locals + 1); // goap_path
    U_BCALLRET1(vm, regs + 1, 13, 1); // length
    *(regs + 1) = Value(9);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 77, 12, 479);
    vm.last = {78, 12};
    *(regs + 0) = *(locals + 1); // goap_path
    U_BCALLRETV(vm, regs + 1, 16, 1); // pop
    U_POPREF(vm, regs + 1);
    vm.last = {79, 12};
    *(regs + 0) = *(locals + 1); // goap_path
    U_PUSHFUN(vm, regs + 1, 0, fun_304);
    fun_562(vm, regs + 2); // call: map
    *(keepvar + 0) = *(regs + 0);
    U_PUSHSTR(vm, regs + 1, 480); // "Eat"
    (regs + 1)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 2, 481); // "BuyPizza"
    (regs + 2)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 3, 482); // "SellSkin"
    (regs + 3)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 4, 483); // "KillWolf"
    (regs + 4)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 5, 484); // "Eat"
    (regs + 5)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 6, 485); // "BuyPizza"
    (regs + 6)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 7, 486); // "SellSkin"
    (regs + 7)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 8, 487); // "KillWolf"
    (regs + 8)->LTINCRTNIL();
    U_NEWVEC(vm, regs + 9, 11, 8);
    *(keepvar + 1) = *(regs + 1);
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 79, 12, 488);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    DecVal(vm, keepvar[1]);
    PopFunId(vm);
}

// precondition
static void fun_290(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1976, locals);
    vm.last = {17, 12};
    *(regs + 0) = *(locals + 1); // s
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = Value(1);
    U_IGE(vm, regs + 2);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// precondition
static void fun_292(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1981, locals);
    vm.last = {24, 12};
    *(regs + 0) = *(locals + 1); // s
    U_PUSHFLD(vm, regs + 1, 1);
    *(regs + 1) = Value(1);
    U_IGE(vm, regs + 2);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// precondition
static void fun_294(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1986, locals);
    vm.last = {31, 12};
    *(regs + 0) = *(locals + 1); // s
    U_PUSHFLD(vm, regs + 1, 2);
    *(regs + 1) = Value(2);
    U_IGE(vm, regs + 2);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// precondition
static void fun_296(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1991, locals);
    vm.last = {38, 12};
    *(regs + 0) = *(locals + 1); // s
    U_PUSHFLD(vm, regs + 1, 2);
    *(regs + 1) = Value(1);
    U_IGE(vm, regs + 2);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// precondition
static void fun_298(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 1996, locals);
    vm.last = {45, 12};
    *(regs + 0) = *(locals + 1); // s
    U_PUSHFLD(vm, regs + 1, 3);
    *(regs + 1) = Value(1);
    U_IGE(vm, regs + 2);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// precondition
static void fun_300(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2001, locals);
    vm.last = {52, 12};
    *(regs + 0) = *(locals + 1); // s
    U_PUSHFLD(vm, regs + 1, 6);
    *(regs + 1) = Value(0);
    U_IGT(vm, regs + 2);
    if (!(regs + 0)->True()) goto block0;
    U_POP(vm, regs + 1);
    *(regs + 0) = *(locals + 1); // s
    U_PUSHFLD(vm, regs + 1, 4);
    *(regs + 1) = Value(0);
    U_IGT(vm, regs + 2);
    if ((regs + 0)->True()) goto block1;
    U_POP(vm, regs + 1);
    *(regs + 0) = *(locals + 1); // s
    U_PUSHFLD(vm, regs + 1, 5);
    *(regs + 1) = Value(0);
    U_IGT(vm, regs + 2);
    block1:;
    block0:;
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// effect
static void fun_291(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2006, locals);
    vm.last = {19, 12};
    *(regs + 0) = *(locals + 1); // s
    U_LVAL_FLD(vm, regs + 1, 0);
    U_LV_IMM(vm, regs + 0);
    vm.last = {20, 12};
    *(regs + 0) = *(locals + 1); // s
    U_LVAL_FLD(vm, regs + 1, 1);
    U_LV_IPP(vm, regs + 0);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// effect
static void fun_293(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2011, locals);
    vm.last = {26, 12};
    *(regs + 0) = Value(2);
    *(regs + 1) = *(locals + 1); // s
    U_LVAL_FLD(vm, regs + 2, 2);
    U_LV_IADD(vm, regs + 1);
    vm.last = {27, 12};
    *(regs + 0) = *(locals + 1); // s
    U_LVAL_FLD(vm, regs + 1, 1);
    U_LV_IMM(vm, regs + 0);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// effect
static void fun_295(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2016, locals);
    vm.last = {33, 12};
    *(regs + 0) = Value(2);
    *(regs + 1) = *(locals + 1); // s
    U_LVAL_FLD(vm, regs + 2, 2);
    U_LV_ISUB(vm, regs + 1);
    vm.last = {34, 12};
    *(regs + 0) = *(locals + 1); // s
    U_LVAL_FLD(vm, regs + 1, 5);
    U_LV_IPP(vm, regs + 0);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// effect
static void fun_297(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2021, locals);
    vm.last = {40, 12};
    *(regs + 0) = *(locals + 1); // s
    U_LVAL_FLD(vm, regs + 1, 2);
    U_LV_IMM(vm, regs + 0);
    vm.last = {41, 12};
    *(regs + 0) = *(locals + 1); // s
    U_LVAL_FLD(vm, regs + 1, 3);
    U_LV_IPP(vm, regs + 0);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// effect
static void fun_299(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2026, locals);
    vm.last = {47, 12};
    *(regs + 0) = *(locals + 1); // s
    U_LVAL_FLD(vm, regs + 1, 3);
    U_LV_IMM(vm, regs + 0);
    vm.last = {48, 12};
    *(regs + 0) = *(locals + 1); // s
    U_LVAL_FLD(vm, regs + 1, 4);
    U_LV_IPP(vm, regs + 0);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// effect
static void fun_301(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2031, locals);
    vm.last = {54, 12};
    *(regs + 0) = *(locals + 1); // s
    U_PUSHFLD(vm, regs + 1, 4);
    *(regs + 1) = Value(0);
    U_IGT(vm, regs + 2);
    if (!(regs + 0)->True()) goto block0;
    vm.last = {55, 12};
    *(regs + 0) = *(locals + 1); // s
    U_LVAL_FLD(vm, regs + 1, 4);
    U_LV_IMM(vm, regs + 0);
    goto block1;
    block0:;
    vm.last = {57, 12};
    *(regs + 0) = *(locals + 1); // s
    U_LVAL_FLD(vm, regs + 1, 5);
    U_LV_IMM(vm, regs + 0);
    block1:;
    vm.last = {58, 12};
    *(regs + 0) = *(locals + 1); // s
    U_LVAL_FLD(vm, regs + 1, 6);
    U_LV_IMM(vm, regs + 0);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function272
static void fun_302(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2036, locals);
    vm.last = {73, 12};
    *(regs + 0) = *(locals + 0); // state
    U_PUSHFLD(vm, regs + 1, 6);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function273
static void fun_303(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2040, locals);
    vm.last = {75, 12};
    *(regs + 0) = *(locals + 0); // s
    U_PUSHFLD(vm, regs + 1, 6);
    *(regs + 1) = Value(0);
    U_ILE(vm, regs + 2);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function274
static void fun_304(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2044, locals);
    vm.last = {79, 12};
    *(regs + 0) = *(locals + 0); // n
    U_PUSHFLD(vm, regs + 1, 5);
    if (!(regs + 0)->True()) goto block0;
    vm.last = {79, 12};
    *(regs + 0) = *(locals + 0); // n
    U_PUSHFLD(vm, regs + 1, 5);
    U_BCALLRETV(vm, regs + 1, 156, 1); // type_string
    goto block1;
    block0:;
    vm.last = {79, 12};
    U_PUSHSTR(vm, regs + 0, 489); // ""
    (regs + 0)->LTINCRTNIL();
    block1:;
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function275
static void fun_305(VMRef vm, StackPtr psp) {
    Value regs[16];
    Value keepvar[1];
    BackupVar(vm, 587);
    PushFunId(vm, funinfo_table + 2048, 0);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {6, 13};
    *(regs + 0) = Value(1);
    *(regs + 1) = Value(2);
    *(regs + 2) = Value(1);
    *(regs + 3) = Value(-2);
    *(regs + 4) = Value(-1);
    *(regs + 5) = Value(2);
    *(regs + 6) = Value(-1);
    *(regs + 7) = Value(-2);
    *(regs + 8) = Value(2);
    *(regs + 9) = Value(1);
    *(regs + 10) = Value(2);
    *(regs + 11) = Value(-1);
    *(regs + 12) = Value(-2);
    *(regs + 13) = Value(1);
    *(regs + 14) = Value(-2);
    *(regs + 15) = Value(-1);
    U_NEWVEC(vm, regs + 16, 37, 8);
    U_LVAL_VARF(vm, regs + 1, 587); // knight_moves
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {9, 13};
    vm.last = {23, 13};
    *(regs + 0) = Value(0);
    U_BCALLRET1(vm, regs + 1, 97, 0); // rnd_seed
    vm.last = {24, 13};
    *(regs + 0) = Value(8);
    *(regs + 1) = Value(8);
    fun_306(vm, regs + 2); // call: knights_tour
    *(keepvar + 0) = *(regs + 0);
    U_ASSERT(vm, regs + 1, 24, 13, 490);
    DecOwned(vm, 587);
    goto epilogue;
    epilogue:;
    RestoreBackup(vm, 587);
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// knights_tour
static void fun_306(VMRef vm, StackPtr psp) {
    Value regs[6];
    Value locals[3];
    SwapVars(vm, 588, psp, 2);
    SwapVars(vm, 589, psp, 1);
    BackupVar(vm, 590);
    BackupVar(vm, 591);
    BackupVar(vm, 592);
    *(locals + 0) = Value(0, lobster::V_NIL);
    *(locals + 1) = Value(0, lobster::V_NIL);
    *(locals + 2) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 2052, locals);
    vm.last = {10, 13};
    U_PUSHVARVF(vm, regs + 0, 588, 2); // dim+0
    U_PUSHFUN(vm, regs + 2, 0, fun_10000001);
    fun_100(vm, regs + 3); // call: mapxy
    U_LVAL_VARF(vm, regs + 1, 590); // board
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {11, 13};
    U_PUSHVARVF(vm, regs + 0, 588, 2); // dim+0
    fun_95(vm, regs + 2); // call: rnd_int2
    U_LVAL_VARF(vm, regs + 2, 591); // cur+0
    U_LV_WRITEV(vm, regs + 2, 2);
    vm.last = {12, 13};
    *(regs + 0) = Value(0);
    U_PUSHVARF(vm, regs + 1, 590); // board
    U_PUSHVARVF(vm, regs + 2, 591, 2); // cur+0
    U_LVAL_IDXVV(vm, regs + 4, 0, 2);
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {13, 13};
    *(regs + 0) = Value(-1);
    U_PUSHVARF(vm, regs + 1, 588); // dim+0
    U_PUSHVARF(vm, regs + 2, 589); // dim+1
    U_IMUL(vm, regs + 3);
    *(regs + 2) = Value(1);
    U_ISUB(vm, regs + 3);
    block0:;
    if (!U_IFOR(vm, regs + 2)) goto block1;
    vm.last = {13, 13};
    U_IFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 0; // step
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {14, 13};
    U_PUSHVARF(vm, regs + 2, 587); // knight_moves
    U_PUSHFUN(vm, regs + 3, 0, fun_308);
    fun_565(vm, regs + 4); // call: map
    vm.temp_lval = locals + 1; // candidates
    U_LV_WRITEREF(vm, regs + 3);
    vm.last = {15, 13};
    vm.last = {16, 13};
    *(regs + 2) = *(locals + 1); // candidates
    (regs + 2)->LTINCRTNIL();
    U_PUSHFUN(vm, regs + 3, 0, fun_310);
    fun_10(vm, regs + 4); // call: filter
    vm.temp_lval = locals + 1; // candidates
    U_LV_WRITEREF(vm, regs + 3);
    vm.last = {17, 13};
    *(regs + 2) = *(locals + 1); // candidates
    U_BCALLRET1(vm, regs + 3, 13, 1); // length
    U_LOGNOT(vm, regs + 3);
    if (!(regs + 2)->True()) goto block2;
    vm.last = {17, 13};
    U_POP(vm, regs + 2);
    U_POP(vm, regs + 1);
    U_PUSHNIL(vm, regs + 0);
    DecOwned(vm, 590);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[2]);
    psp = PopArg(vm, 589, psp);
    psp = PopArg(vm, 588, psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    block2:;
    vm.last = {18, 13};
    *(regs + 2) = *(locals + 1); // candidates
    (regs + 2)->LTINCRTNIL();
    U_PUSHFUN(vm, regs + 3, 0, fun_311);
    fun_566(vm, regs + 4); // call: map
    vm.temp_lval = locals + 2; // degrees
    U_LV_WRITEREF(vm, regs + 3);
    vm.last = {19, 13};
    *(regs + 2) = *(locals + 1); // candidates
    *(regs + 3) = *(locals + 2); // degrees
    (regs + 3)->LTINCRTNIL();
    U_PUSHFUN(vm, regs + 4, 0, fun_313);
    fun_26(vm, regs + 5); // call: find_best
    U_VPUSHIDXI2V(vm, regs + 4);
    U_LVAL_VARF(vm, regs + 4, 591); // cur+0
    U_LV_WRITEV(vm, regs + 4, 2);
    vm.last = {20, 13};
    *(regs + 2) = *(locals + 0); // step
    *(regs + 3) = Value(1);
    U_IADD(vm, regs + 4);
    U_PUSHVARF(vm, regs + 3, 590); // board
    U_PUSHVARVF(vm, regs + 4, 591, 2); // cur+0
    U_LVAL_IDXVV(vm, regs + 6, 0, 2);
    *(vm.temp_lval) = *(regs + 2);
    goto block0;
    block1:;
    vm.last = {21, 13};
    U_PUSHVARF(vm, regs + 0, 590); // board
    (regs + 0)->LTINCRTNIL();
    DecOwned(vm, 590);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[2]);
    psp = PopArg(vm, 589, psp);
    psp = PopArg(vm, 588, psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    RestoreBackup(vm, 592);
    RestoreBackup(vm, 591);
    RestoreBackup(vm, 590);
    PopFunId(vm);
}

// function278
static void fun_308(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2063, locals);
    vm.last = {14, 13};
    U_PUSHVARVF(vm, regs + 0, 591, 2); // cur+0
    *(regs + 2) = *(locals + 0); // m.0
    *(regs + 3) = *(locals + 1); // m.1
    U_IVVADD(vm, regs + 4, 2);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// empty
static void fun_309(VMRef vm, StackPtr psp) {
    Value regs[9];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2068, locals);
    vm.last = {15, 13};
    *(regs + 0) = *(locals + 0); // v.0
    *(regs + 1) = *(locals + 1); // v.1
    *(regs + 2) = Value(2);
    U_PUSHVARVF(vm, regs + 3, 588, 2); // dim+0
    *(regs + 5) = Value(2);
    *(regs + 6) = Value(0);
    *(regs + 7) = Value(0);
    *(regs + 8) = Value(2);
    U_BCALLRETV(vm, regs + 9, 108, 1); // in_range
    if (!(regs + 0)->True()) goto block0;
    U_POP(vm, regs + 1);
    U_PUSHVARF(vm, regs + 0, 590); // board
    *(regs + 1) = *(locals + 0); // v.0
    *(regs + 2) = *(locals + 1); // v.1
    U_VPUSHIDXV(vm, regs + 3, 2);
    *(regs + 1) = Value(0);
    U_ILT(vm, regs + 2);
    block0:;
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function280
static void fun_310(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2073, locals);
    vm.last = {16, 13};
    *(regs + 0) = *(locals + 0); // c.0
    *(regs + 1) = *(locals + 1); // c.1
    fun_309(vm, regs + 2); // call: empty
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function281
static void fun_311(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value keepvar[1];
    SwapVars(vm, 602, psp, 2);
    SwapVars(vm, 603, psp, 1);
    PushFunId(vm, funinfo_table + 2078, 0);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {18, 13};
    U_PUSHVARF(vm, regs + 0, 587); // knight_moves
    U_PUSHFUN(vm, regs + 1, 0, fun_312);
    fun_567(vm, regs + 2); // call: filter
    *(keepvar + 0) = *(regs + 0);
    U_BCALLRET1(vm, regs + 1, 13, 1); // length
    psp = PopArg(vm, 603, psp);
    psp = PopArg(vm, 602, psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// function282
static void fun_312(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2083, locals);
    vm.last = {18, 13};
    U_PUSHVARVF(vm, regs + 0, 602, 2); // c+0
    *(regs + 2) = *(locals + 0); // m.0
    *(regs + 3) = *(locals + 1); // m.1
    U_IVVADD(vm, regs + 4, 2);
    fun_309(vm, regs + 2); // call: empty
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function283
static void fun_313(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2088, locals);
    vm.last = {19, 13};
    *(regs + 0) = *(locals + 0); // d
    U_IUMINUS(vm, regs + 1);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function284
static void fun_314(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value keepvar[1];
    Value locals[4];
    *(locals + 0) = Value(0, lobster::V_NIL);
    *(locals + 1) = Value(0, lobster::V_NIL);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 2092, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {8, 14};
    vm.last = {8, 14};
    vm.last = {13, 14};
    vm.last = {14, 14};
    vm.last = {15, 14};
    vm.last = {10, 14};
    vm.last = {10, 14};
    vm.last = {21, 14};
    vm.last = {25, 14};
    vm.last = {34, 14};
    vm.last = {17, 14};
    vm.last = {17, 14};
    vm.last = {41, 14};
    vm.last = {116, 14};
    U_PUSHSTR(vm, regs + 0, 491); // "(+\n(* 2 3 4)  // Comment.\n10)\n"
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 0; // test_code
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {122, 14};
    *(regs + 0) = *(locals + 0); // test_code
    fun_326(vm, regs + 1); // call: parse
    vm.temp_lval = locals + 2; // err
    U_LV_WRITEREF(vm, regs + 2);
    vm.temp_lval = locals + 1; // ast
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {123, 14};
    *(regs + 0) = *(locals + 1); // ast
    U_E2B(vm, regs + 1);
    if (!(regs + 0)->True()) goto block0;
    U_POP(vm, regs + 1);
    *(regs + 0) = *(locals + 2); // err
    U_PUSHSTR(vm, regs + 1, 492); // ""
    U_SEQ(vm, regs + 2);
    block0:;
    U_ASSERT(vm, regs + 1, 123, 14, 493);
    vm.last = {124, 14};
    *(regs + 0) = *(locals + 1); // ast
    (regs + 0)->LTINCRTNIL();
    U_DDCALL(vm, regs + 1, 0, 0); vm.next_call_target(vm, regs + 1);
    *(keepvar + 0) = *(regs + 0);
    U_PUSHSTR(vm, regs + 1, 494); // "(+ (* 2 3 4) 10)"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 124, 14, 495);
    vm.last = {125, 14};
    *(regs + 0) = *(locals + 1); // ast
    (regs + 0)->LTINCRTNIL();
    U_DDCALL(vm, regs + 1, 2, 0); vm.next_call_target(vm, regs + 1);
    *(regs + 1) = Value(34);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 125, 14, 496);
    vm.last = {126, 14};
    *(regs + 0) = *(locals + 1); // ast
    (regs + 0)->LTINCRTNIL();
    U_DDCALL(vm, regs + 1, 5, 0); vm.next_call_target(vm, regs + 1);
    vm.temp_lval = locals + 3; // code
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {127, 14};
    *(regs + 0) = *(locals + 3); // code
    U_PUSHSTR(vm, regs + 1, 497); // "(((2*3)*4)+10)"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 127, 14, 498);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[2]);
    DecVal(vm, locals[3]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// pretty
static void fun_315(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2099, locals);
    vm.last = {13, 14};
    *(regs + 0) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 1, 0);
    U_A2S(vm, regs + 1, 0);
    DecVal(vm, locals[0]);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// pretty
static void fun_318(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    *(locals + 0) = *(psp - 1);
    *(locals + 1) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 2103, locals);
    vm.last = {22, 14};
    *(regs + 0) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 1, 1);
    *(regs + 1) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 2, 0);
    (regs + 1)->LTINCRTNIL();
    U_PUSHFUN(vm, regs + 2, 0, fun_319);
    fun_570(vm, regs + 3); // call: fold
    vm.temp_lval = locals + 1; // elems
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {23, 14};
    U_PUSHSTR(vm, regs + 0, 499); // "("
    *(regs + 1) = *(locals + 1); // elems
    U_PUSHSTR(vm, regs + 2, 500); // ")"
    U_SADDN(vm, regs + 3, 3);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// eval
static void fun_316(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2108, locals);
    vm.last = {14, 14};
    *(regs + 0) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 1, 0);
    DecVal(vm, locals[0]);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// eval
static void fun_320(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2112, locals);
    vm.last = {32, 14};
    *(regs + 0) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 1, 0);
    U_DUP(vm, regs + 1);
    U_PUSHSTR(vm, regs + 2, 501); // "+"
    U_SEQ(vm, regs + 3);
    if (!(regs + 1)->True()) goto block0;
    U_POP(vm, regs + 1);
    vm.last = {28, 14};
    *(regs + 0) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 1, 1);
    *(regs + 1) = Value(0);
    U_PUSHFUN(vm, regs + 2, 0, fun_321);
    fun_571(vm, regs + 3); // call: fold
    goto block1;
    block0:;
    U_DUP(vm, regs + 1);
    U_PUSHSTR(vm, regs + 2, 502); // "*"
    U_SEQ(vm, regs + 3);
    if (!(regs + 1)->True()) goto block2;
    U_POP(vm, regs + 1);
    vm.last = {30, 14};
    *(regs + 0) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 1, 1);
    *(regs + 1) = Value(1);
    U_PUSHFUN(vm, regs + 2, 0, fun_322);
    fun_572(vm, regs + 3); // call: fold
    goto block3;
    block2:;
    U_POP(vm, regs + 1);
    vm.last = {32, 14};
    *(regs + 0) = Value(0);
    U_ASSERTR(vm, regs + 1, 32, 14, 503);
    block1:;
    block3:;
    DecVal(vm, locals[0]);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// compile
static void fun_317(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2116, locals);
    vm.last = {15, 14};
    *(regs + 0) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 1, 0);
    U_A2S(vm, regs + 1, 0);
    DecVal(vm, locals[0]);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// compile
static void fun_323(VMRef vm, StackPtr psp) {
    Value regs[3];
    SwapVars(vm, 619, psp, 1);
    PushFunId(vm, funinfo_table + 2120, 0);
    vm.last = {39, 14};
    U_PUSHVARF(vm, regs + 0, 619); // this
    U_PUSHFLD(vm, regs + 1, 0);
    U_DUP(vm, regs + 1);
    U_PUSHSTR(vm, regs + 2, 504); // "+"
    U_SEQ(vm, regs + 3);
    if ((regs + 1)->True()) goto block0;
    U_DUP(vm, regs + 1);
    U_PUSHSTR(vm, regs + 2, 505); // "*"
    U_SEQ(vm, regs + 3);
    if (!(regs + 1)->True()) goto block1;
    block0:;
    U_POP(vm, regs + 1);
    vm.last = {37, 14};
    U_PUSHVARF(vm, regs + 0, 619); // this
    U_PUSHFLD(vm, regs + 1, 1);
    U_PUSHFUN(vm, regs + 1, 0, fun_324);
    fun_573(vm, regs + 2); // call: map
    U_PUSHFUN(vm, regs + 1, 0, fun_325);
    fun_19(vm, regs + 2); // call: reduce
    goto block2;
    block1:;
    U_POP(vm, regs + 1);
    vm.last = {39, 14};
    *(regs + 0) = Value(0);
    U_ASSERTR(vm, regs + 1, 39, 14, 506);
    block2:;
    DecOwned(vm, 619);
    psp = PopArg(vm, 619, psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function288
static void fun_319(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value keepvar[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2124, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    *(keepvar + 1) = Value(0, lobster::V_NIL);
    vm.last = {22, 14};
    *(regs + 0) = *(locals + 0); // _a
    (regs + 0)->LTINCRTNIL();
    *(keepvar + 0) = *(regs + 0);
    U_PUSHSTR(vm, regs + 1, 507); // " "
    *(regs + 2) = *(locals + 1); // _e
    (regs + 2)->LTINCRTNIL();
    U_DDCALL(vm, regs + 3, 1, 0); vm.next_call_target(vm, regs + 3);
    *(keepvar + 1) = *(regs + 2);
    U_SADDN(vm, regs + 3, 3);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    DecVal(vm, keepvar[1]);
    PopFunId(vm);
}

// function289
static void fun_321(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2129, locals);
    vm.last = {28, 14};
    *(regs + 0) = *(locals + 0); // _a
    *(regs + 1) = *(locals + 1); // _e
    (regs + 1)->LTINCRTNIL();
    U_DDCALL(vm, regs + 2, 3, 0); vm.next_call_target(vm, regs + 2);
    U_IADD(vm, regs + 2);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function290
static void fun_322(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2134, locals);
    vm.last = {30, 14};
    *(regs + 0) = *(locals + 0); // _a
    *(regs + 1) = *(locals + 1); // _e
    (regs + 1)->LTINCRTNIL();
    U_DDCALL(vm, regs + 2, 4, 0); vm.next_call_target(vm, regs + 2);
    U_IMUL(vm, regs + 2);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function291
static void fun_324(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2139, locals);
    vm.last = {37, 14};
    *(regs + 0) = *(locals + 0); // _
    (regs + 0)->LTINCRTNIL();
    U_DDCALL(vm, regs + 1, 6, 0); vm.next_call_target(vm, regs + 1);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function292
static void fun_325(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2143, locals);
    vm.last = {37, 14};
    U_PUSHSTR(vm, regs + 0, 508); // "("
    *(regs + 1) = *(locals + 0); // _a
    U_PUSHVARF(vm, regs + 2, 619); // this
    U_PUSHFLD(vm, regs + 3, 0);
    *(regs + 3) = *(locals + 1); // _e
    U_PUSHSTR(vm, regs + 4, 509); // ")"
    U_SADDN(vm, regs + 5, 5);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// parse
static void fun_326(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    SwapVars(vm, 623, psp, 1);
    BackupVar(vm, 624);
    BackupVar(vm, 625);
    BackupVar(vm, 626);
    BackupVar(vm, 627);
    BackupVar(vm, 628);
    *(locals + 0) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 2148, locals);
    vm.last = {42, 14};
    U_PUSHSTR(vm, regs + 0, 510); // ""
    (regs + 0)->LTINCRTNIL();
    U_LVAL_VARF(vm, regs + 1, 624); // token
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {43, 14};
    U_PUSHSTR(vm, regs + 0, 511); // ""
    (regs + 0)->LTINCRTNIL();
    U_LVAL_VARF(vm, regs + 1, 625); // atom
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {44, 14};
    *(regs + 0) = Value(0);
    U_LVAL_VARF(vm, regs + 1, 626); // i
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {45, 14};
    *(regs + 0) = Value(1);
    U_LVAL_VARF(vm, regs + 1, 627); // line
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {46, 14};
    *(regs + 0) = Value(0);
    U_LVAL_VARF(vm, regs + 1, 628); // ival
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {47, 14};
    vm.last = {51, 14};
    vm.last = {61, 14};
    vm.last = {87, 14};
    fun_330(vm, regs + 0); // call: lex_next
    if (vm.ret_unwind_to == 297) goto block0;
    DecOwned(vm, 624);
    DecOwned(vm, 625);
    DecVal(vm, locals[0]);
    psp = PopArg(vm, 623, psp);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block0:;
    vm.last = {88, 14};
    vm.last = {94, 14};
    vm.last = {112, 14};
    fun_332(vm, regs + 0); // call: parse_exp
    if (vm.ret_unwind_to == 299) goto block1;
    DecOwned(vm, 624);
    DecOwned(vm, 625);
    DecVal(vm, locals[0]);
    psp = PopArg(vm, 623, psp);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block1:;
    vm.temp_lval = locals + 0; // root
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {113, 14};
    U_PUSHSTR(vm, regs + 0, 512); // "eof"
    fun_569(vm, regs + 1); // call: expect
    if (vm.ret_unwind_to == 298) goto block2;
    DecOwned(vm, 624);
    DecOwned(vm, 625);
    DecVal(vm, locals[0]);
    psp = PopArg(vm, 623, psp);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block2:;
    vm.last = {114, 14};
    *(regs + 0) = *(locals + 0); // root
    U_PUSHSTR(vm, regs + 1, 513); // ""
    (regs + 1)->LTINCRTNIL();
    (regs + 0)->LTINCRTNIL();
    DecOwned(vm, 624);
    DecOwned(vm, 625);
    DecVal(vm, locals[0]);
    psp = PopArg(vm, 623, psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    goto epilogue;
    epilogue:;
    RestoreBackup(vm, 628);
    RestoreBackup(vm, 627);
    RestoreBackup(vm, 626);
    RestoreBackup(vm, 625);
    RestoreBackup(vm, 624);
    PopFunId(vm);
}

// error
static void fun_327(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2158, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {50, 14};
    U_PUSHNIL(vm, regs + 0);
    U_PUSHSTR(vm, regs + 1, 514); // "error: line "
    U_PUSHVARF(vm, regs + 2, 627); // line
    U_A2S(vm, regs + 3, 0);
    *(keepvar + 0) = *(regs + 2);
    U_PUSHSTR(vm, regs + 3, 515); // ": "
    *(regs + 4) = *(locals + 0); // err
    U_SADDN(vm, regs + 5, 4);
    vm.ret_slots = 2;
    vm.ret_unwind_to = 293;
    DecVal(vm, locals[0]);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// lex_atom
static void fun_328(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value keepvar[1];
    Value locals[2];
    *(locals + 0) = Value(0, lobster::V_NIL);
    *(locals + 1) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 2162, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {52, 14};
    U_LVAL_VARF(vm, regs + 0, 626); // i
    U_LV_IMM(vm, regs + 0);
    vm.last = {53, 14};
    U_PUSHVARF(vm, regs + 0, 626); // i
    vm.temp_lval = locals + 0; // start
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {54, 14};
    block0:;
    U_PUSHVARF(vm, regs + 0, 623); // s
    U_PUSHVARF(vm, regs + 1, 626); // i
    U_SPUSHIDXI(vm, regs + 2);
    *(regs + 1) = Value(32);
    U_IGT(vm, regs + 2);
    if (!(regs + 0)->True()) goto block1;
    U_POP(vm, regs + 1);
    U_PUSHSTR(vm, regs + 0, 516); // "()\177"
    U_PUSHFUN(vm, regs + 1, 0, fun_329);
    fun_15(vm, regs + 2); // call: exists
    U_LOGNOT(vm, regs + 1);
    block1:;
    if (!(regs + 0)->True()) goto block2;
    vm.last = {54, 14};
    U_LVAL_VARF(vm, regs + 0, 626); // i
    U_LV_IPP(vm, regs + 0);
    goto block0;
    block2:;
    vm.last = {55, 14};
    *(regs + 0) = *(locals + 0); // start
    U_PUSHVARF(vm, regs + 1, 626); // i
    U_IEQ(vm, regs + 2);
    if (!(regs + 0)->True()) goto block3;
    vm.last = {56, 14};
    U_PUSHSTR(vm, regs + 0, 517); // "unprintable character: "
    U_PUSHVARF(vm, regs + 1, 623); // s
    U_PUSHVARF(vm, regs + 2, 626); // i
    U_SPUSHIDXI(vm, regs + 3);
    U_A2S(vm, regs + 2, 0);
    *(keepvar + 0) = *(regs + 1);
    U_SADD(vm, regs + 2);
    fun_327(vm, regs + 1); // call: error
    if (vm.ret_unwind_to == 294) goto block4;
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block4:;
    block3:;
    vm.last = {57, 14};
    U_PUSHVARF(vm, regs + 0, 623); // s
    *(regs + 1) = *(locals + 0); // start
    U_PUSHVARF(vm, regs + 2, 626); // i
    *(regs + 3) = *(locals + 0); // start
    U_ISUB(vm, regs + 4);
    U_BCALLRET3(vm, regs + 3, 36, 1); // substring
    U_LVAL_VARF(vm, regs + 1, 625); // atom
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {58, 14};
    *(regs + 0) = Value(0);
    vm.temp_lval = locals + 1; // all
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {59, 14};
    U_PUSHVARF(vm, regs + 0, 625); // atom
    *(regs + 1) = Value(0);
    U_BCALLRET2(vm, regs + 2, 40, 1); // string_to_int
    vm.temp_lval = locals + 1; // all
    *(vm.temp_lval) = *(regs + 1);
    U_LVAL_VARF(vm, regs + 1, 628); // ival
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {60, 14};
    *(regs + 0) = *(locals + 1); // all
    if (!(regs + 0)->True()) goto block5;
    vm.last = {60, 14};
    U_PUSHSTR(vm, regs + 0, 518); // "int"
    goto block6;
    block5:;
    vm.last = {60, 14};
    U_PUSHSTR(vm, regs + 0, 519); // "atom"
    block6:;
    (regs + 0)->LTINCRTNIL();
    U_LVAL_VARF(vm, regs + 1, 624); // token
    U_LV_WRITEREF(vm, regs + 1);
    vm.ret_slots = 0;
    vm.ret_unwind_to = 295;
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// function296
static void fun_329(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2167, locals);
    vm.last = {54, 14};
    *(regs + 0) = *(locals + 0); // _
    U_PUSHVARF(vm, regs + 1, 623); // s
    U_PUSHVARF(vm, regs + 2, 626); // i
    U_SPUSHIDXI(vm, regs + 3);
    U_IEQ(vm, regs + 2);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// lex_next
static void fun_330(VMRef vm, StackPtr psp) {
    Value regs[3];
    PushFunId(vm, funinfo_table + 2171, 0);
    vm.last = {62, 14};
    block0:;
    *(regs + 0) = Value(1);
    if (!(regs + 0)->True()) goto block1;
    vm.last = {63, 14};
    U_PUSHVARF(vm, regs + 0, 623); // s
    U_LVAL_VARF(vm, regs + 1, 626); // i
    U_LV_DUP(vm, regs + 1);
    U_LV_IPP(vm, regs + 2);
    U_SPUSHIDXI(vm, regs + 2);
    U_DUP(vm, regs + 1);
    *(regs + 2) = Value(0);
    U_IEQ(vm, regs + 3);
    if (!(regs + 1)->True()) goto block2;
    U_POP(vm, regs + 1);
    vm.last = {65, 14};
    U_LVAL_VARF(vm, regs + 0, 626); // i
    U_LV_IMM(vm, regs + 0);
    vm.last = {66, 14};
    U_PUSHSTR(vm, regs + 0, 520); // "eof"
    (regs + 0)->LTINCRTNIL();
    U_LVAL_VARF(vm, regs + 1, 624); // token
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {67, 14};
    vm.ret_slots = 0;
    vm.ret_unwind_to = 297;
    goto epilogue;
    goto block3;
    block2:;
    U_DUP(vm, regs + 1);
    *(regs + 2) = Value(10);
    U_IEQ(vm, regs + 3);
    if (!(regs + 1)->True()) goto block4;
    U_POP(vm, regs + 1);
    vm.last = {69, 14};
    U_LVAL_VARF(vm, regs + 0, 627); // line
    U_LV_IPP(vm, regs + 0);
    goto block5;
    block4:;
    U_DUP(vm, regs + 1);
    *(regs + 2) = Value(13);
    U_IEQ(vm, regs + 3);
    if ((regs + 1)->True()) goto block6;
    U_DUP(vm, regs + 1);
    *(regs + 2) = Value(9);
    U_IEQ(vm, regs + 3);
    if ((regs + 1)->True()) goto block7;
    U_DUP(vm, regs + 1);
    *(regs + 2) = Value(32);
    U_IEQ(vm, regs + 3);
    if (!(regs + 1)->True()) goto block8;
    block6:;
    block7:;
    U_POP(vm, regs + 1);
    vm.last = {71, 14};
    goto block9;
    block8:;
    U_DUP(vm, regs + 1);
    *(regs + 2) = Value(40);
    U_IEQ(vm, regs + 3);
    if (!(regs + 1)->True()) goto block10;
    U_POP(vm, regs + 1);
    vm.last = {73, 14};
    U_PUSHSTR(vm, regs + 0, 521); // "("
    (regs + 0)->LTINCRTNIL();
    U_LVAL_VARF(vm, regs + 1, 624); // token
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {74, 14};
    vm.ret_slots = 0;
    vm.ret_unwind_to = 297;
    goto epilogue;
    goto block11;
    block10:;
    U_DUP(vm, regs + 1);
    *(regs + 2) = Value(41);
    U_IEQ(vm, regs + 3);
    if (!(regs + 1)->True()) goto block12;
    U_POP(vm, regs + 1);
    vm.last = {76, 14};
    U_PUSHSTR(vm, regs + 0, 522); // ")"
    (regs + 0)->LTINCRTNIL();
    U_LVAL_VARF(vm, regs + 1, 624); // token
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {77, 14};
    vm.ret_slots = 0;
    vm.ret_unwind_to = 297;
    goto epilogue;
    goto block13;
    block12:;
    U_DUP(vm, regs + 1);
    *(regs + 2) = Value(47);
    U_IEQ(vm, regs + 3);
    if (!(regs + 1)->True()) goto block14;
    U_POP(vm, regs + 1);
    vm.last = {79, 14};
    U_PUSHVARF(vm, regs + 0, 623); // s
    U_PUSHVARF(vm, regs + 1, 626); // i
    U_SPUSHIDXI(vm, regs + 2);
    *(regs + 1) = Value(47);
    U_IEQ(vm, regs + 2);
    if (!(regs + 0)->True()) goto block15;
    vm.last = {80, 14};
    block16:;
    U_PUSHVARF(vm, regs + 0, 623); // s
    U_PUSHVARF(vm, regs + 1, 626); // i
    U_SPUSHIDXI(vm, regs + 2);
    if (!(regs + 0)->True()) goto block17;
    U_POP(vm, regs + 1);
    U_PUSHVARF(vm, regs + 0, 623); // s
    U_PUSHVARF(vm, regs + 1, 626); // i
    U_SPUSHIDXI(vm, regs + 2);
    *(regs + 1) = Value(10);
    U_INE(vm, regs + 2);
    block17:;
    if (!(regs + 0)->True()) goto block18;
    vm.last = {80, 14};
    U_LVAL_VARF(vm, regs + 0, 626); // i
    U_LV_IPP(vm, regs + 0);
    goto block16;
    block18:;
    goto block19;
    block15:;
    vm.last = {82, 14};
    fun_328(vm, regs + 0); // call: lex_atom
    if (vm.ret_unwind_to == 295) goto block20;
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block20:;
    vm.last = {83, 14};
    vm.ret_slots = 0;
    vm.ret_unwind_to = 297;
    goto epilogue;
    block19:;
    goto block21;
    block14:;
    U_POP(vm, regs + 1);
    vm.last = {85, 14};
    fun_328(vm, regs + 0); // call: lex_atom
    if (vm.ret_unwind_to == 295) goto block22;
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block22:;
    vm.last = {86, 14};
    vm.ret_slots = 0;
    vm.ret_unwind_to = 297;
    goto epilogue;
    block3:;
    block5:;
    block9:;
    block11:;
    block13:;
    block21:;
    goto block0;
    block1:;
    vm.ret_slots = 0;
    vm.ret_unwind_to = 297;
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// expect
static void fun_569(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[2];
    *(locals + 0) = *(psp - 1);
    *(locals + 1) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 2174, locals);
    vm.last = {89, 14};
    *(regs + 0) = *(locals + 0); // tok
    U_PUSHVARF(vm, regs + 1, 624); // token
    U_SNE(vm, regs + 2);
    if (!(regs + 0)->True()) goto block0;
    vm.last = {90, 14};
    U_PUSHSTR(vm, regs + 0, 523); // "expected: "
    *(regs + 1) = *(locals + 0); // tok
    U_PUSHSTR(vm, regs + 2, 524); // ", found: "
    U_PUSHVARF(vm, regs + 3, 624); // token
    U_SADDN(vm, regs + 4, 4);
    fun_327(vm, regs + 1); // call: error
    if (vm.ret_unwind_to == 294) goto block1;
    DecVal(vm, locals[1]);
    Pop(psp);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block1:;
    block0:;
    vm.last = {91, 14};
    U_PUSHVARF(vm, regs + 0, 625); // atom
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 1; // a
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {92, 14};
    fun_330(vm, regs + 0); // call: lex_next
    if (vm.ret_unwind_to == 297) goto block2;
    DecVal(vm, locals[1]);
    Pop(psp);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block2:;
    vm.last = {93, 14};
    vm.ret_slots = 0;
    vm.ret_unwind_to = 298;
    DecVal(vm, locals[1]);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// expect
static void fun_331(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[2];
    *(locals + 0) = *(psp - 1);
    *(locals + 1) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 2179, locals);
    vm.last = {89, 14};
    *(regs + 0) = *(locals + 0); // tok
    U_PUSHVARF(vm, regs + 1, 624); // token
    U_SNE(vm, regs + 2);
    if (!(regs + 0)->True()) goto block0;
    vm.last = {90, 14};
    U_PUSHSTR(vm, regs + 0, 525); // "expected: "
    *(regs + 1) = *(locals + 0); // tok
    U_PUSHSTR(vm, regs + 2, 526); // ", found: "
    U_PUSHVARF(vm, regs + 3, 624); // token
    U_SADDN(vm, regs + 4, 4);
    fun_327(vm, regs + 1); // call: error
    if (vm.ret_unwind_to == 294) goto block1;
    DecVal(vm, locals[1]);
    Pop(psp);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block1:;
    block0:;
    vm.last = {91, 14};
    U_PUSHVARF(vm, regs + 0, 625); // atom
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 1; // a
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {92, 14};
    fun_330(vm, regs + 0); // call: lex_next
    if (vm.ret_unwind_to == 297) goto block2;
    DecVal(vm, locals[1]);
    Pop(psp);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block2:;
    vm.last = {93, 14};
    *(regs + 0) = *(locals + 1); // a
    (regs + 0)->LTINCRTNIL();
    vm.ret_slots = 1;
    vm.ret_unwind_to = 298;
    DecVal(vm, locals[1]);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// parse_exp
static void fun_332(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    BackupVar(vm, 635);
    *(locals + 0) = Value(0, lobster::V_NIL);
    *(locals + 1) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 2184, locals);
    vm.last = {95, 14};
    U_PUSHVARF(vm, regs + 0, 624); // token
    U_DUP(vm, regs + 1);
    U_PUSHSTR(vm, regs + 2, 527); // "("
    U_SEQ(vm, regs + 3);
    if (!(regs + 1)->True()) goto block0;
    U_POP(vm, regs + 1);
    vm.last = {97, 14};
    fun_330(vm, regs + 0); // call: lex_next
    if (vm.ret_unwind_to == 297) goto block1;
    DecOwned(vm, 635);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block1:;
    vm.last = {98, 14};
    U_PUSHSTR(vm, regs + 0, 528); // "atom"
    fun_331(vm, regs + 1); // call: expect
    if (vm.ret_unwind_to == 298) goto block2;
    DecOwned(vm, 635);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block2:;
    U_LVAL_VARF(vm, regs + 1, 635); // name
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {99, 14};
    U_PUSHSTR(vm, regs + 0, 529); // "+"
    (regs + 0)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 1, 530); // "*"
    (regs + 1)->LTINCRTNIL();
    U_NEWVEC(vm, regs + 2, 11, 2);
    U_PUSHFUN(vm, regs + 1, 0, fun_333);
    fun_568(vm, regs + 2); // call: exists
    U_LOGNOT(vm, regs + 1);
    if (!(regs + 0)->True()) goto block3;
    vm.last = {100, 14};
    U_PUSHSTR(vm, regs + 0, 531); // "unknown op: "
    U_PUSHVARF(vm, regs + 1, 635); // name
    U_SADD(vm, regs + 2);
    fun_327(vm, regs + 1); // call: error
    if (vm.ret_unwind_to == 294) goto block4;
    DecOwned(vm, 635);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block4:;
    block3:;
    vm.last = {101, 14};
    U_PUSHVARF(vm, regs + 0, 635); // name
    (regs + 0)->LTINCRTNIL();
    U_NEWVEC(vm, regs + 1, 1092, 0);
    U_NEWOBJECT(vm, regs + 2, 1080); // inst
    vm.temp_lval = locals + 0; // n
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {102, 14};
    block5:;
    U_PUSHVARF(vm, regs + 0, 624); // token
    U_PUSHSTR(vm, regs + 1, 532); // ")"
    U_SNE(vm, regs + 2);
    if (!(regs + 0)->True()) goto block6;
    vm.last = {103, 14};
    *(regs + 0) = *(locals + 0); // n
    U_PUSHFLD(vm, regs + 1, 1);
    fun_332(vm, regs + 1); // call: parse_exp
    if (vm.ret_unwind_to == 299) goto block7;
    DecOwned(vm, 635);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 1]); }
    U_POP(vm, regs + 1);
    goto epilogue;
    block7:;
    *(regs + 2) = Value(1);
    U_BCALLRETV(vm, regs + 3, 15, 1); // push
    U_POP(vm, regs + 1);
    goto block5;
    block6:;
    vm.last = {104, 14};
    fun_330(vm, regs + 0); // call: lex_next
    if (vm.ret_unwind_to == 297) goto block8;
    DecOwned(vm, 635);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block8:;
    vm.last = {105, 14};
    *(regs + 0) = *(locals + 0); // n
    (regs + 0)->LTINCRTNIL();
    vm.ret_slots = 1;
    vm.ret_unwind_to = 299;
    DecOwned(vm, 635);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    *(psp++) = *(regs + 0);
    goto epilogue;
    goto block9;
    block0:;
    U_DUP(vm, regs + 1);
    U_PUSHSTR(vm, regs + 2, 533); // "int"
    U_SEQ(vm, regs + 3);
    if (!(regs + 1)->True()) goto block10;
    U_POP(vm, regs + 1);
    vm.last = {107, 14};
    U_PUSHVARF(vm, regs + 0, 628); // ival
    U_NEWOBJECT(vm, regs + 1, 1065); // integer
    vm.temp_lval = locals + 1; // n
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {108, 14};
    fun_330(vm, regs + 0); // call: lex_next
    if (vm.ret_unwind_to == 297) goto block11;
    DecOwned(vm, 635);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block11:;
    vm.last = {109, 14};
    *(regs + 0) = *(locals + 1); // n
    (regs + 0)->LTINCRTNIL();
    vm.ret_slots = 1;
    vm.ret_unwind_to = 299;
    DecOwned(vm, 635);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    *(psp++) = *(regs + 0);
    goto epilogue;
    goto block12;
    block10:;
    U_POP(vm, regs + 1);
    vm.last = {111, 14};
    U_PUSHSTR(vm, regs + 0, 534); // "cannot parse expression starting with: "
    U_PUSHVARF(vm, regs + 1, 624); // token
    U_SADD(vm, regs + 2);
    fun_327(vm, regs + 1); // call: error
    if (vm.ret_unwind_to == 294) goto block13;
    DecOwned(vm, 635);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block13:;
    block9:;
    block12:;
    vm.ret_slots = 0;
    vm.ret_unwind_to = 299;
    DecOwned(vm, 635);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    goto epilogue;
    epilogue:;
    RestoreBackup(vm, 635);
    PopFunId(vm);
}

// function300
static void fun_333(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2190, locals);
    vm.last = {99, 14};
    U_PUSHVARF(vm, regs + 0, 635); // name
    *(regs + 1) = *(locals + 0); // _
    U_SEQ(vm, regs + 2);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// floodfillpath
static void fun_334(VMRef vm, StackPtr psp) {
    Value regs[7];
    Value locals[6];
    SwapVars(vm, 644, psp, 5);
    SwapVars(vm, 645, psp, 4);
    *(locals + 0) = *(psp - 3);
    *(locals + 1) = *(psp - 2);
    SwapVars(vm, 648, psp, 1);
    BackupVar(vm, 649);
    *(locals + 2) = Value(0, lobster::V_NIL);
    BackupVar(vm, 653);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    *(locals + 5) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 2194, locals);
    vm.last = {14, 16};
    U_PUSHVARVF(vm, regs + 0, 644, 2); // sz+0
    U_PUSHFUN(vm, regs + 2, 0, fun_335);
    fun_574(vm, regs + 3); // call: mapxy
    U_LVAL_VARF(vm, regs + 1, 649); // m
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {15, 16};
    U_PUSHVARF(vm, regs + 0, 649); // m
    *(regs + 1) = *(locals + 0); // target.0
    *(regs + 2) = *(locals + 1); // target.1
    U_VPUSHIDXV(vm, regs + 3, 2);
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 2; // end
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {16, 16};
    *(regs + 0) = Value(0);
    *(regs + 1) = *(locals + 2); // end
    U_LVAL_FLD(vm, regs + 2, 0);
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {17, 16};
    *(regs + 0) = *(locals + 2); // end
    (regs + 0)->LTINCRTNIL();
    U_NEWVEC(vm, regs + 1, 1119, 1);
    U_LVAL_VARF(vm, regs + 1, 653); // pathq
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {18, 16};
    vm.last = {28, 16};
    block0:;
    U_PUSHVARF(vm, regs + 0, 653); // pathq
    U_BCALLRET1(vm, regs + 1, 13, 1); // length
    if (!(regs + 0)->True()) goto block1;
    vm.last = {29, 16};
    U_PUSHVARF(vm, regs + 0, 653); // pathq
    U_BCALLRETV(vm, regs + 1, 16, 1); // pop
    vm.temp_lval = locals + 3; // c
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {30, 16};
    *(regs + 0) = Value(-1);
    *(regs + 1) = Value(2);
    block2:;
    if (!U_IFOR(vm, regs + 2)) goto block3;
    vm.last = {30, 16};
    U_IFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 4; // x
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {30, 16};
    *(regs + 2) = Value(-1);
    *(regs + 3) = Value(2);
    block4:;
    if (!U_IFOR(vm, regs + 4)) goto block5;
    vm.last = {30, 16};
    U_IFORELEM(vm, regs + 4);
    vm.temp_lval = locals + 5; // y
    *(vm.temp_lval) = *(regs + 4);
    vm.last = {31, 16};
    *(regs + 4) = Value(1);
    *(regs + 5) = *(locals + 5); // y
    U_ISUB(vm, regs + 6);
    *(regs + 5) = *(locals + 4); // x
    U_ISUB(vm, regs + 6);
    *(regs + 5) = *(locals + 5); // y
    *(regs + 6) = *(locals + 4); // x
    U_ISUB(vm, regs + 7);
    *(regs + 6) = *(locals + 3); // c
    (regs + 6)->LTINCRTNIL();
    fun_336(vm, regs + 7); // call: flood
    goto block4;
    block5:;
    goto block2;
    block3:;
    goto block0;
    block1:;
    vm.last = {32, 16};
    U_PUSHVARF(vm, regs + 0, 649); // m
    DecVal(vm, locals[2]);
    DecOwned(vm, 653);
    DecVal(vm, locals[3]);
    psp = PopArg(vm, 648, psp);
    Pop(psp);
    Pop(psp);
    psp = PopArg(vm, 645, psp);
    psp = PopArg(vm, 644, psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    RestoreBackup(vm, 653);
    RestoreBackup(vm, 649);
    PopFunId(vm);
}

// function302
static void fun_335(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2208, locals);
    vm.last = {14, 16};
    *(regs + 0) = Value(-1);
    U_PUSHVARVF(vm, regs + 1, 123, 2); // int2_0+0
    *(regs + 3) = *(locals + 0); // v.0
    *(regs + 4) = *(locals + 1); // v.1
    U_NEWOBJECT(vm, regs + 5, 1098); // pathmap
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// flood
static void fun_336(VMRef vm, StackPtr psp) {
    Value regs[9];
    Value locals[9];
    *(locals + 0) = *(psp - 3);
    *(locals + 1) = *(psp - 2);
    *(locals + 2) = *(psp - 1);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    *(locals + 5) = Value(0, lobster::V_NIL);
    *(locals + 6) = Value(0, lobster::V_NIL);
    *(locals + 7) = Value(0, lobster::V_NIL);
    *(locals + 8) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 2213, locals);
    vm.last = {19, 16};
    *(regs + 0) = *(locals + 2); // parent
    U_PUSHFLD2V(vm, regs + 1, 3, 2);
    *(regs + 2) = *(locals + 0); // dir.0
    *(regs + 3) = *(locals + 1); // dir.1
    U_IVVADD(vm, regs + 4, 2);
    vm.temp_lval = locals + 3; // pos+0
    U_LV_WRITEV(vm, regs + 2, 2);
    vm.last = {20, 16};
    *(regs + 0) = *(locals + 3); // pos.0
    *(regs + 1) = *(locals + 4); // pos.1
    *(regs + 2) = Value(2);
    U_PUSHVARVF(vm, regs + 3, 644, 2); // sz+0
    *(regs + 5) = Value(2);
    *(regs + 6) = Value(0);
    *(regs + 7) = Value(0);
    *(regs + 8) = Value(2);
    U_BCALLRETV(vm, regs + 9, 108, 1); // in_range
    if (!(regs + 0)->True()) goto block0;
    vm.last = {21, 16};
    U_PUSHVARF(vm, regs + 0, 649); // m
    *(regs + 1) = *(locals + 3); // pos.0
    *(regs + 2) = *(locals + 4); // pos.1
    U_VPUSHIDXV(vm, regs + 3, 2);
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 5; // c
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {22, 16};
    vm.last = {22, 16};
    *(regs + 0) = *(locals + 3); // pos.0
    *(regs + 1) = *(locals + 4); // pos.1
    vm.temp_lval = locals + 7; // v+0
    U_LV_WRITEV(vm, regs + 2, 2);
    vm.last = {6, 15};
    *(regs + 0) = Value(1);
    if (!(regs + 0)->True()) goto block1;
    U_POP(vm, regs + 1);
    *(regs + 0) = *(locals + 5); // c
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = Value(0);
    U_ILT(vm, regs + 2);
    block1:;
    if (!(regs + 0)->True()) goto block2;
    vm.last = {23, 16};
    *(regs + 0) = *(locals + 2); // parent
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = Value(1);
    U_IADD(vm, regs + 2);
    *(regs + 1) = *(locals + 5); // c
    U_LVAL_FLD(vm, regs + 2, 0);
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {24, 16};
    *(regs + 0) = *(locals + 0); // dir.0
    *(regs + 1) = *(locals + 1); // dir.1
    U_IVUMINUS(vm, regs + 2, 2);
    *(regs + 2) = *(locals + 5); // c
    U_LVAL_FLD(vm, regs + 3, 1);
    U_LV_WRITEV(vm, regs + 2, 2);
    vm.last = {25, 16};
    U_PUSHVARF(vm, regs + 0, 653); // pathq
    U_BCALLRET1(vm, regs + 1, 13, 1); // length
    vm.temp_lval = locals + 6; // i
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {26, 16};
    block3:;
    *(regs + 0) = *(locals + 6); // i
    *(regs + 1) = Value(0);
    U_IGT(vm, regs + 2);
    if (!(regs + 0)->True()) goto block4;
    U_POP(vm, regs + 1);
    U_PUSHVARF(vm, regs + 0, 653); // pathq
    *(regs + 1) = *(locals + 6); // i
    *(regs + 2) = Value(1);
    U_ISUB(vm, regs + 3);
    U_VPUSHIDXI(vm, regs + 2);
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = *(locals + 5); // c
    U_PUSHFLD(vm, regs + 2, 0);
    U_ILT(vm, regs + 2);
    block4:;
    if (!(regs + 0)->True()) goto block5;
    vm.last = {26, 16};
    vm.temp_lval = locals + 6; // i
    U_LV_IMM(vm, regs + 0);
    goto block3;
    block5:;
    vm.last = {27, 16};
    U_PUSHVARF(vm, regs + 0, 653); // pathq
    *(regs + 1) = *(locals + 6); // i
    *(regs + 2) = *(locals + 5); // c
    (regs + 2)->LTINCRTNIL();
    *(regs + 3) = Value(1);
    U_BCALLRETV(vm, regs + 4, 18, 1); // insert
    U_POP(vm, regs + 1);
    block2:;
    block0:;
    DecVal(vm, locals[2]);
    DecVal(vm, locals[5]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function304
static void fun_337(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    PushFunId(vm, funinfo_table + 2225, 0);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {6, 15};
    U_PUSHVARVF(vm, regs + 0, 125, 2); // int2_1+0
    *(regs + 2) = Value(20);
    U_IVSMUL(vm, regs + 3, 2);
    U_PUSHVARVF(vm, regs + 2, 125, 2); // int2_1+0
    *(regs + 4) = Value(5);
    U_IVSMUL(vm, regs + 5, 2);
    U_PUSHFUN(vm, regs + 4, 0, fun_10000001);
    fun_334(vm, regs + 5); // call: floodfillpath
    *(keepvar + 0) = *(regs + 0);
    U_BCALLRET1(vm, regs + 1, 13, 1); // length
    *(regs + 1) = Value(20);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 6, 15, 535);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// function306
static void fun_339(VMRef vm, StackPtr psp) {
    Value regs[8];
    Value locals[1];
    BackupVar(vm, 666);
    BackupVar(vm, 667);
    BackupVar(vm, 668);
    BackupVar(vm, 669);
    BackupVar(vm, 670);
    BackupVar(vm, 671);
    BackupVar(vm, 672);
    BackupVar(vm, 673);
    BackupVar(vm, 674);
    BackupVar(vm, 684);
    *(locals + 0) = Value(0, lobster::V_NIL);
    BackupVar(vm, 691);
    BackupVar(vm, 692);
    BackupVar(vm, 693);
    PushFunId(vm, funinfo_table + 2228, locals);
    vm.last = {9, 17};
    *(regs + 0) = Value(0);
    U_BCALLRET1(vm, regs + 1, 97, 0); // rnd_seed
    vm.last = {11, 17};
    vm.last = {11, 17};
    vm.last = {16, 17};
    *(regs + 0) = Value(100);
    U_LVAL_VARF(vm, regs + 1, 666); // lssize
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {17, 17};
    U_PUSHFLT64(vm, regs + 0, -1717986918, 1070176665); // 0.2
    U_LVAL_VARF(vm, regs + 1, 667); // hmargin
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {19, 17};
    U_PUSHVARVF(vm, regs + 0, 123, 2); // int2_0+0
    U_LVAL_VARF(vm, regs + 2, 668); // highest+0
    U_LV_WRITEV(vm, regs + 2, 2);
    vm.last = {20, 17};
    U_PUSHVARVF(vm, regs + 0, 123, 2); // int2_0+0
    U_LVAL_VARF(vm, regs + 2, 670); // lowest+0
    U_LV_WRITEV(vm, regs + 2, 2);
    vm.last = {21, 17};
    *(regs + 0) = Value(-1000.000000);
    U_LVAL_VARF(vm, regs + 1, 672); // highh
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {22, 17};
    *(regs + 0) = Value(1000.000000);
    U_LVAL_VARF(vm, regs + 1, 673); // lowh
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {24, 17};
    U_PUSHVARF(vm, regs + 0, 666); // lssize
    U_PUSHFUN(vm, regs + 1, 0, fun_340);
    fun_581(vm, regs + 2); // call: map
    U_LVAL_VARF(vm, regs + 1, 674); // cells
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {38, 17};
    vm.last = {43, 17};
    vm.last = {47, 17};
    U_PUSHVARVF(vm, regs + 0, 127, 2); // int2_x+0
    U_PUSHVARVF(vm, regs + 2, 129, 2); // int2_y+0
    U_PUSHVARVF(vm, regs + 4, 127, 2); // int2_x+0
    U_IVUMINUS(vm, regs + 6, 2);
    U_PUSHVARVF(vm, regs + 6, 129, 2); // int2_y+0
    U_IVUMINUS(vm, regs + 8, 2);
    U_NEWVEC(vm, regs + 8, 37, 4);
    U_LVAL_VARF(vm, regs + 1, 684); // neighbours
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {49, 17};
    vm.last = {54, 17};
    *(regs + 0) = Value(0.000000);
    vm.temp_lval = locals + 0; // starting_extra_water
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {55, 17};
    *(regs + 0) = Value(4.000000);
    U_LVAL_VARF(vm, regs + 1, 691); // water_per_second
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {56, 17};
    *(regs + 0) = Value(40.000000);
    U_LVAL_VARF(vm, regs + 1, 692); // give_water_divider
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {57, 17};
    U_PUSHFLT64(vm, regs + 0, -1717986918, 1070176665); // 0.2
    U_LVAL_VARF(vm, regs + 1, 693); // minimum_water_accumulation
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {59, 17};
    *(regs + 0) = *(locals + 0); // starting_extra_water
    U_PUSHVARF(vm, regs + 1, 674); // cells
    U_PUSHVARVF(vm, regs + 2, 668, 2); // highest+0
    U_VPUSHIDXV(vm, regs + 4, 2);
    U_LVAL_FLD(vm, regs + 2, 1);
    U_LV_FADD(vm, regs + 1);
    vm.last = {61, 17};
    vm.last = {84, 17};
    *(regs + 0) = Value(-1);
    *(regs + 1) = Value(5);
    block0:;
    if (!U_IFOR(vm, regs + 2)) goto block1;
    vm.last = {84, 17};
    fun_346(vm, regs + 2); // call: sim
    goto block0;
    block1:;
    DecOwned(vm, 674);
    DecOwned(vm, 684);
    goto epilogue;
    epilogue:;
    RestoreBackup(vm, 693);
    RestoreBackup(vm, 692);
    RestoreBackup(vm, 691);
    RestoreBackup(vm, 684);
    RestoreBackup(vm, 674);
    RestoreBackup(vm, 673);
    RestoreBackup(vm, 672);
    RestoreBackup(vm, 671);
    RestoreBackup(vm, 670);
    RestoreBackup(vm, 669);
    RestoreBackup(vm, 668);
    RestoreBackup(vm, 667);
    RestoreBackup(vm, 666);
    PopFunId(vm);
}

// function307
static void fun_340(VMRef vm, StackPtr psp) {
    Value regs[2];
    SwapVars(vm, 675, psp, 1);
    PushFunId(vm, funinfo_table + 2245, 0);
    vm.last = {25, 17};
    U_PUSHVARF(vm, regs + 0, 666); // lssize
    U_PUSHFUN(vm, regs + 1, 0, fun_341);
    fun_582(vm, regs + 2); // call: map
    psp = PopArg(vm, 675, psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function308
static void fun_341(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    *(locals + 0) = *(psp - 1);
    *(locals + 1) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 2249, locals);
    vm.last = {26, 17};
    U_BCALLRET0(vm, regs + 0, 95, 1); // rnd_float
    vm.temp_lval = locals + 1; // h
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {27, 17};
    *(regs + 0) = Value(1.000000);
    vm.temp_lval = locals + 1; // h
    U_LV_FADD(vm, regs + 1);
    vm.last = {28, 17};
    *(regs + 0) = *(locals + 1); // h
    *(regs + 1) = Value(3.000000);
    U_BCALLRET2(vm, regs + 2, 52, 1); // pow
    vm.temp_lval = locals + 1; // h
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {29, 17};
    *(regs + 0) = *(locals + 1); // h
    *(regs + 1) = Value(10.000000);
    U_FMUL(vm, regs + 2);
    vm.temp_lval = locals + 1; // h
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {30, 17};
    *(regs + 0) = *(locals + 1); // h
    U_PUSHVARF(vm, regs + 1, 672); // highh
    U_FGT(vm, regs + 2);
    if (!(regs + 0)->True()) goto block0;
    vm.last = {31, 17};
    *(regs + 0) = *(locals + 1); // h
    U_LVAL_VARF(vm, regs + 1, 672); // highh
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {32, 17};
    *(regs + 0) = *(locals + 0); // x
    U_PUSHVARF(vm, regs + 1, 675); // y
    U_LVAL_VARF(vm, regs + 2, 668); // highest+0
    U_LV_WRITEV(vm, regs + 2, 2);
    block0:;
    vm.last = {33, 17};
    *(regs + 0) = *(locals + 1); // h
    U_PUSHVARF(vm, regs + 1, 673); // lowh
    U_FLT(vm, regs + 2);
    if (!(regs + 0)->True()) goto block1;
    vm.last = {34, 17};
    *(regs + 0) = *(locals + 1); // h
    U_LVAL_VARF(vm, regs + 1, 673); // lowh
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {35, 17};
    *(regs + 0) = *(locals + 0); // x
    U_PUSHVARF(vm, regs + 1, 675); // y
    U_LVAL_VARF(vm, regs + 2, 670); // lowest+0
    U_LV_WRITEV(vm, regs + 2, 2);
    block1:;
    vm.last = {36, 17};
    *(regs + 0) = *(locals + 1); // h
    *(regs + 1) = *(locals + 1); // h
    *(regs + 2) = Value(0.000000);
    U_NEWOBJECT(vm, regs + 3, 1123); // cell
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// loopworld
static void fun_583(VMRef vm, StackPtr psp) {
    Value regs[8];
    Value locals[3];
    *(locals + 0) = *(psp - 1);
    *(locals + 1) = Value(0, lobster::V_NIL);
    *(locals + 2) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 2254, locals);
    vm.last = {39, 17};
    *(regs + 0) = Value(-1);
    U_PUSHVARF(vm, regs + 1, 666); // lssize
    block0:;
    if (!U_IFOR(vm, regs + 2)) goto block1;
    vm.last = {39, 17};
    U_IFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 1; // y
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {40, 17};
    *(regs + 2) = Value(-1);
    U_PUSHVARF(vm, regs + 3, 666); // lssize
    block2:;
    if (!U_IFOR(vm, regs + 4)) goto block3;
    vm.last = {40, 17};
    U_IFORELEM(vm, regs + 4);
    vm.temp_lval = locals + 2; // x
    *(vm.temp_lval) = *(regs + 4);
    vm.last = {41, 17};
    *(regs + 4) = *(locals + 2); // x
    *(regs + 5) = *(locals + 1); // y
    U_PUSHVARF(vm, regs + 6, 674); // cells
    *(regs + 7) = *(locals + 1); // y
    U_VPUSHIDXI(vm, regs + 8);
    *(regs + 7) = *(locals + 2); // x
    U_VPUSHIDXI(vm, regs + 8);
    fun_349(vm, regs + 7); // call: function316
    goto block2;
    block3:;
    goto block0;
    block1:;
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// loopworld
static void fun_342(VMRef vm, StackPtr psp) {
    Value regs[8];
    Value locals[3];
    *(locals + 0) = *(psp - 1);
    *(locals + 1) = Value(0, lobster::V_NIL);
    *(locals + 2) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 2260, locals);
    vm.last = {39, 17};
    *(regs + 0) = Value(-1);
    U_PUSHVARF(vm, regs + 1, 666); // lssize
    block0:;
    if (!U_IFOR(vm, regs + 2)) goto block1;
    vm.last = {39, 17};
    U_IFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 1; // y
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {40, 17};
    *(regs + 2) = Value(-1);
    U_PUSHVARF(vm, regs + 3, 666); // lssize
    block2:;
    if (!U_IFOR(vm, regs + 4)) goto block3;
    vm.last = {40, 17};
    U_IFORELEM(vm, regs + 4);
    vm.temp_lval = locals + 2; // x
    *(vm.temp_lval) = *(regs + 4);
    vm.last = {41, 17};
    *(regs + 4) = *(locals + 2); // x
    *(regs + 5) = *(locals + 1); // y
    U_PUSHVARF(vm, regs + 6, 674); // cells
    *(regs + 7) = *(locals + 1); // y
    U_VPUSHIDXI(vm, regs + 8);
    *(regs + 7) = *(locals + 2); // x
    U_VPUSHIDXI(vm, regs + 8);
    (regs + 6)->LTINCRTNIL();
    fun_347(vm, regs + 7); // call: function314
    goto block2;
    block3:;
    goto block0;
    block1:;
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// ifinworld
static void fun_343(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[3];
    *(locals + 0) = *(psp - 3);
    *(locals + 1) = *(psp - 2);
    *(locals + 2) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2266, locals);
    vm.last = {44, 17};
    *(regs + 0) = *(locals + 0); // pos+0
    *(regs + 1) = Value(0);
    U_IGE(vm, regs + 2);
    if (!(regs + 0)->True()) goto block0;
    U_POP(vm, regs + 1);
    *(regs + 0) = *(locals + 1); // pos+1
    *(regs + 1) = Value(0);
    U_IGE(vm, regs + 2);
    block0:;
    if (!(regs + 0)->True()) goto block1;
    U_POP(vm, regs + 1);
    *(regs + 0) = *(locals + 0); // pos+0
    U_PUSHVARF(vm, regs + 1, 666); // lssize
    U_ILT(vm, regs + 2);
    block1:;
    if (!(regs + 0)->True()) goto block2;
    U_POP(vm, regs + 1);
    *(regs + 0) = *(locals + 1); // pos+1
    U_PUSHVARF(vm, regs + 1, 666); // lssize
    U_ILT(vm, regs + 2);
    block2:;
    if (!(regs + 0)->True()) goto block3;
    vm.last = {45, 17};
    fun_345(vm, regs + 0); // call: function312
    block3:;
    Pop(psp);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// forneighboursinworld
static void fun_344(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value locals[2];
    *(locals + 0) = *(psp - 3);
    *(locals + 1) = *(psp - 2);
    SwapVars(vm, 687, psp, 1);
    BackupVar(vm, 688);
    BackupVar(vm, 689);
    PushFunId(vm, funinfo_table + 2272, locals);
    vm.last = {50, 17};
    *(regs + 0) = Value(-1);
    U_PUSHVARF(vm, regs + 1, 684); // neighbours
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {50, 17};
    U_VFORELEM2S(vm, regs + 2);
    U_LVAL_VARF(vm, regs + 4, 688); // n+0
    U_LV_WRITEV(vm, regs + 4, 2);
    vm.last = {51, 17};
    *(regs + 2) = *(locals + 0); // x
    *(regs + 3) = *(locals + 1); // y
    U_LVAL_VARF(vm, regs + 4, 688); // n+0
    U_LV_IVVADD(vm, regs + 4, 2);
    vm.last = {52, 17};
    U_PUSHVARVF(vm, regs + 2, 688, 2); // n+0
    U_PUSHFUN(vm, regs + 4, 0, fun_345);
    fun_343(vm, regs + 5); // call: ifinworld
    goto block0;
    block1:;
    psp = PopArg(vm, 687, psp);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    RestoreBackup(vm, 689);
    RestoreBackup(vm, 688);
    PopFunId(vm);
}

// function312
static void fun_345(VMRef vm, StackPtr psp) {
    Value regs[2];
    PushFunId(vm, funinfo_table + 2280, 0);
    vm.last = {52, 17};
    U_PUSHVARF(vm, regs + 0, 674); // cells
    U_PUSHVARF(vm, regs + 1, 689); // n+1
    U_VPUSHIDXI(vm, regs + 2);
    U_PUSHVARF(vm, regs + 1, 688); // n+0
    U_VPUSHIDXI(vm, regs + 2);
    fun_348(vm, regs + 1); // call: function315
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// sim
static void fun_346(VMRef vm, StackPtr psp) {
    Value regs[4];
    PushFunId(vm, funinfo_table + 2283, 0);
    vm.last = {62, 17};
    U_PUSHFLT64(vm, regs + 0, 286331153, 1066471697); // 0.016666666666666666
    U_PUSHVARF(vm, regs + 1, 691); // water_per_second
    U_FMUL(vm, regs + 2);
    U_PUSHVARF(vm, regs + 1, 674); // cells
    U_PUSHVARVF(vm, regs + 2, 668, 2); // highest+0
    U_VPUSHIDXV(vm, regs + 4, 2);
    U_LVAL_FLD(vm, regs + 2, 1);
    U_LV_FADD(vm, regs + 1);
    vm.last = {63, 17};
    U_PUSHFUN(vm, regs + 0, 0, fun_347);
    fun_342(vm, regs + 1); // call: loopworld
    vm.last = {79, 17};
    U_PUSHFUN(vm, regs + 0, 0, fun_349);
    fun_583(vm, regs + 1); // call: loopworld
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function314
static void fun_347(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[5];
    *(locals + 0) = *(psp - 3);
    *(locals + 1) = *(psp - 2);
    SwapVars(vm, 696, psp, 1);
    BackupVar(vm, 697);
    BackupVar(vm, 698);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 2286, locals);
    vm.last = {64, 17};
    U_PUSHVARF(vm, regs + 0, 696); // c
    U_PUSHFLD(vm, regs + 1, 1);
    U_PUSHVARF(vm, regs + 1, 696); // c
    U_PUSHFLD(vm, regs + 2, 0);
    U_FSUB(vm, regs + 2);
    U_BCALLRET1(vm, regs + 1, 113, 1); // abs
    U_PUSHVARF(vm, regs + 1, 693); // minimum_water_accumulation
    U_FGT(vm, regs + 2);
    if (!(regs + 0)->True()) goto block0;
    vm.last = {65, 17};
    *(regs + 0) = Value(0.000000);
    U_LVAL_VARF(vm, regs + 1, 697); // totald
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {66, 17};
    U_NEWVEC(vm, regs + 0, 1138, 0);
    U_LVAL_VARF(vm, regs + 1, 698); // lower
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {69, 17};
    *(regs + 0) = *(locals + 0); // x
    *(regs + 1) = *(locals + 1); // y
    U_PUSHFUN(vm, regs + 2, 0, fun_348);
    fun_344(vm, regs + 3); // call: forneighboursinworld
    vm.last = {74, 17};
    U_PUSHVARF(vm, regs + 0, 696); // c
    U_PUSHFLD(vm, regs + 1, 1);
    U_PUSHVARF(vm, regs + 1, 696); // c
    U_PUSHFLD(vm, regs + 2, 0);
    U_FSUB(vm, regs + 2);
    U_PUSHVARF(vm, regs + 1, 692); // give_water_divider
    U_FDIV(vm, regs + 2);
    U_PUSHVARF(vm, regs + 1, 697); // totald
    U_PUSHVARF(vm, regs + 2, 692); // give_water_divider
    U_FDIV(vm, regs + 3);
    U_BCALLRET2(vm, regs + 2, 121, 1); // min
    vm.temp_lval = locals + 2; // w
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {75, 17};
    *(regs + 0) = Value(-1);
    U_PUSHVARF(vm, regs + 1, 698); // lower
    block1:;
    if (!U_VFOR(vm, regs + 2)) goto block2;
    vm.last = {75, 17};
    U_VFORELEMREF(vm, regs + 2);
    vm.temp_lval = locals + 3; // nc
    U_LV_WRITEREF(vm, regs + 3);
    vm.last = {76, 17};
    U_PUSHVARF(vm, regs + 2, 696); // c
    U_PUSHFLD(vm, regs + 3, 1);
    *(regs + 3) = *(locals + 3); // nc
    U_PUSHFLD(vm, regs + 4, 1);
    U_FSUB(vm, regs + 4);
    U_PUSHVARF(vm, regs + 3, 697); // totald
    U_FDIV(vm, regs + 4);
    *(regs + 3) = *(locals + 2); // w
    U_FMUL(vm, regs + 4);
    vm.temp_lval = locals + 4; // give
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {77, 17};
    *(regs + 2) = *(locals + 4); // give
    *(regs + 3) = *(locals + 3); // nc
    U_LVAL_FLD(vm, regs + 4, 2);
    U_LV_FADD(vm, regs + 3);
    vm.last = {78, 17};
    *(regs + 2) = *(locals + 4); // give
    U_PUSHVARF(vm, regs + 3, 696); // c
    U_LVAL_FLD(vm, regs + 4, 2);
    U_LV_FSUB(vm, regs + 3);
    goto block1;
    block2:;
    block0:;
    DecOwned(vm, 696);
    DecOwned(vm, 698);
    DecVal(vm, locals[3]);
    psp = PopArg(vm, 696, psp);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    RestoreBackup(vm, 698);
    RestoreBackup(vm, 697);
    PopFunId(vm);
}

// function315
static void fun_348(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    *(locals + 0) = *(psp - 1);
    *(locals + 1) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 2297, locals);
    vm.last = {70, 17};
    U_PUSHVARF(vm, regs + 0, 696); // c
    U_PUSHFLD(vm, regs + 1, 1);
    *(regs + 1) = *(locals + 0); // nc
    U_PUSHFLD(vm, regs + 2, 1);
    U_FSUB(vm, regs + 2);
    vm.temp_lval = locals + 1; // d
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {71, 17};
    *(regs + 0) = *(locals + 1); // d
    U_PUSHVARF(vm, regs + 1, 667); // hmargin
    U_FGT(vm, regs + 2);
    if (!(regs + 0)->True()) goto block0;
    vm.last = {72, 17};
    *(regs + 0) = *(locals + 1); // d
    U_LVAL_VARF(vm, regs + 1, 697); // totald
    U_LV_FADD(vm, regs + 1);
    vm.last = {73, 17};
    U_PUSHVARF(vm, regs + 0, 698); // lower
    *(regs + 1) = *(locals + 0); // nc
    (regs + 1)->LTINCRTNIL();
    *(regs + 2) = Value(1);
    U_BCALLRETV(vm, regs + 3, 15, 1); // push
    U_POP(vm, regs + 1);
    block0:;
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function316
static void fun_349(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[3];
    *(locals + 0) = *(psp - 3);
    *(locals + 1) = *(psp - 2);
    *(locals + 2) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2302, locals);
    vm.last = {80, 17};
    *(regs + 0) = *(locals + 2); // c
    U_PUSHFLD(vm, regs + 1, 2);
    *(regs + 1) = Value(0.000000);
    U_FNE(vm, regs + 2);
    if (!(regs + 0)->True()) goto block0;
    vm.last = {81, 17};
    *(regs + 0) = *(locals + 2); // c
    U_PUSHFLD(vm, regs + 1, 2);
    *(regs + 1) = *(locals + 2); // c
    U_LVAL_FLD(vm, regs + 2, 1);
    U_LV_FADD(vm, regs + 1);
    vm.last = {82, 17};
    *(regs + 0) = Value(0.000000);
    *(regs + 1) = *(locals + 2); // c
    U_LVAL_FLD(vm, regs + 2, 2);
    *(vm.temp_lval) = *(regs + 0);
    block0:;
    Pop(psp);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function317
static void fun_350(VMRef vm, StackPtr psp) {
    Value regs[6];
    Value keepvar[1];
    Value locals[6];
    BackupVar(vm, 707);
    BackupVar(vm, 708);
    *(locals + 0) = Value(0, lobster::V_NIL);
    *(locals + 1) = Value(0, lobster::V_NIL);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    *(locals + 5) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 2308, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {7, 18};
    *(regs + 0) = Value(0);
    U_BCALLRET1(vm, regs + 1, 97, 0); // rnd_seed
    vm.last = {9, 18};
    *(regs + 0) = Value(100);
    U_LVAL_VARF(vm, regs + 1, 707); // N
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {10, 18};
    *(regs + 0) = Value(2.000000);
    U_PUSHVARF(vm, regs + 1, 707); // N
    U_I2F(vm, regs + 2);
    U_FDIV(vm, regs + 2);
    U_LVAL_VARF(vm, regs + 1, 708); // f
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {12, 18};
    vm.last = {29, 18};
    U_PUSHFLT64(vm, regs + 0, -1717986918, 1069128089); // 0.1
    vm.temp_lval = locals + 0; // sigma
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {30, 18};
    U_PUSHFLT64(vm, regs + 0, -755914244, 1062232653); // 0.001
    vm.temp_lval = locals + 1; // mu
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {31, 18};
    *(regs + 0) = Value(1000);
    vm.temp_lval = locals + 2; // n_epochs
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {33, 18};
    U_PUSHVARF(vm, regs + 0, 707); // N
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_584(vm, regs + 2); // call: map
    vm.temp_lval = locals + 3; // x
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {34, 18};
    U_PUSHVARF(vm, regs + 0, 707); // N
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_585(vm, regs + 2); // call: map
    vm.temp_lval = locals + 4; // d
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {36, 18};
    *(regs + 0) = Value(-1);
    U_PUSHVARF(vm, regs + 1, 707); // N
    block0:;
    if (!U_IFOR(vm, regs + 2)) goto block1;
    vm.last = {36, 18};
    U_IFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 5; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {37, 18};
    U_PUSHVARF(vm, regs + 2, 708); // f
    *(regs + 3) = *(locals + 5); // i
    U_BCALLRET1(vm, regs + 4, 67, 1); // float
    U_FMUL(vm, regs + 4);
    *(regs + 3) = *(locals + 3); // x
    *(regs + 4) = *(locals + 5); // i
    U_LVAL_IDXVI(vm, regs + 5, 0);
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {38, 18};
    *(regs + 2) = Value(3.000000);
    *(regs + 3) = Value(2.000000);
    *(regs + 4) = *(locals + 3); // x
    *(regs + 5) = *(locals + 5); // i
    U_VPUSHIDXI(vm, regs + 6);
    U_FMUL(vm, regs + 5);
    U_FADD(vm, regs + 4);
    *(regs + 3) = *(locals + 0); // sigma
    U_BCALLRET0(vm, regs + 4, 96, 1); // rnd_gaussian
    U_FMUL(vm, regs + 5);
    U_FADD(vm, regs + 4);
    *(regs + 3) = *(locals + 4); // d
    *(regs + 4) = *(locals + 5); // i
    U_LVAL_IDXVI(vm, regs + 5, 0);
    *(vm.temp_lval) = *(regs + 2);
    goto block0;
    block1:;
    vm.last = {40, 18};
    *(regs + 0) = *(locals + 3); // x
    (regs + 0)->LTINCRTNIL();
    *(regs + 1) = *(locals + 4); // d
    (regs + 1)->LTINCRTNIL();
    *(regs + 2) = *(locals + 1); // mu
    *(regs + 3) = *(locals + 2); // n_epochs
    fun_351(vm, regs + 4); // call: gradient_descent
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = Value(0);
    U_VPUSHIDXI(vm, regs + 2);
    U_PUSHFLT64(vm, regs + 1, -1941428633, 1073966704); // 2.428925605728
    U_FSUB(vm, regs + 2);
    U_BCALLRET1(vm, regs + 1, 113, 1); // abs
    U_PUSHFLT64(vm, regs + 1, 1202590843, 1065646817); // 0.01
    U_FLT(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 40, 18, 536);
    DecVal(vm, locals[3]);
    DecVal(vm, locals[4]);
    goto epilogue;
    epilogue:;
    RestoreBackup(vm, 708);
    RestoreBackup(vm, 707);
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// gradient_descent
static void fun_351(VMRef vm, StackPtr psp) {
    Value regs[8];
    Value locals[12];
    *(locals + 0) = *(psp - 4);
    *(locals + 1) = *(psp - 3);
    *(locals + 2) = *(psp - 2);
    *(locals + 3) = *(psp - 1);
    *(locals + 4) = Value(0, lobster::V_NIL);
    *(locals + 5) = Value(0, lobster::V_NIL);
    *(locals + 6) = Value(0, lobster::V_NIL);
    *(locals + 7) = Value(0, lobster::V_NIL);
    *(locals + 8) = Value(0, lobster::V_NIL);
    *(locals + 9) = Value(0, lobster::V_NIL);
    *(locals + 10) = Value(0, lobster::V_NIL);
    *(locals + 11) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 2319, locals);
    vm.last = {13, 18};
    U_PUSHVARF(vm, regs + 0, 707); // N
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_586(vm, regs + 2); // call: map
    vm.temp_lval = locals + 4; // y
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {14, 18};
    *(regs + 0) = Value(0.000000);
    vm.temp_lval = locals + 5; // w0
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {15, 18};
    *(regs + 0) = Value(0.000000);
    vm.temp_lval = locals + 6; // w1
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {16, 18};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 3); // n_epochs
    block0:;
    if (!U_IFOR(vm, regs + 2)) goto block1;
    vm.last = {17, 18};
    *(regs + 2) = Value(0.000000);
    vm.temp_lval = locals + 7; // grad0
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {18, 18};
    *(regs + 2) = Value(0.000000);
    vm.temp_lval = locals + 8; // grad1
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {19, 18};
    *(regs + 2) = Value(-1);
    U_PUSHVARF(vm, regs + 3, 707); // N
    block2:;
    if (!U_IFOR(vm, regs + 4)) goto block3;
    vm.last = {19, 18};
    U_IFORELEM(vm, regs + 4);
    vm.temp_lval = locals + 9; // i
    *(vm.temp_lval) = *(regs + 4);
    vm.last = {20, 18};
    U_PUSHVARF(vm, regs + 4, 708); // f
    *(regs + 5) = *(locals + 1); // d
    *(regs + 6) = *(locals + 9); // i
    U_VPUSHIDXI(vm, regs + 7);
    *(regs + 6) = *(locals + 4); // y
    *(regs + 7) = *(locals + 9); // i
    U_VPUSHIDXI(vm, regs + 8);
    U_FSUB(vm, regs + 7);
    U_FMUL(vm, regs + 6);
    vm.temp_lval = locals + 10; // err
    *(vm.temp_lval) = *(regs + 4);
    vm.last = {21, 18};
    *(regs + 4) = *(locals + 10); // err
    vm.temp_lval = locals + 7; // grad0
    U_LV_FADD(vm, regs + 5);
    vm.last = {22, 18};
    *(regs + 4) = *(locals + 10); // err
    *(regs + 5) = *(locals + 0); // x
    *(regs + 6) = *(locals + 9); // i
    U_VPUSHIDXI(vm, regs + 7);
    U_FMUL(vm, regs + 6);
    vm.temp_lval = locals + 8; // grad1
    U_LV_FADD(vm, regs + 5);
    goto block2;
    block3:;
    vm.last = {23, 18};
    *(regs + 2) = *(locals + 2); // mu
    *(regs + 3) = *(locals + 7); // grad0
    U_FMUL(vm, regs + 4);
    vm.temp_lval = locals + 5; // w0
    U_LV_FADD(vm, regs + 3);
    vm.last = {24, 18};
    *(regs + 2) = *(locals + 2); // mu
    *(regs + 3) = *(locals + 8); // grad1
    U_FMUL(vm, regs + 4);
    vm.temp_lval = locals + 6; // w1
    U_LV_FADD(vm, regs + 3);
    vm.last = {25, 18};
    *(regs + 2) = Value(-1);
    U_PUSHVARF(vm, regs + 3, 707); // N
    block4:;
    if (!U_IFOR(vm, regs + 4)) goto block5;
    vm.last = {25, 18};
    U_IFORELEM(vm, regs + 4);
    vm.temp_lval = locals + 11; // i
    *(vm.temp_lval) = *(regs + 4);
    vm.last = {26, 18};
    *(regs + 4) = *(locals + 5); // w0
    *(regs + 5) = *(locals + 6); // w1
    *(regs + 6) = *(locals + 0); // x
    *(regs + 7) = *(locals + 11); // i
    U_VPUSHIDXI(vm, regs + 8);
    U_FMUL(vm, regs + 7);
    U_FADD(vm, regs + 6);
    *(regs + 5) = *(locals + 4); // y
    *(regs + 6) = *(locals + 11); // i
    U_LVAL_IDXVI(vm, regs + 7, 0);
    *(vm.temp_lval) = *(regs + 4);
    goto block4;
    block5:;
    goto block0;
    block1:;
    vm.last = {27, 18};
    *(regs + 0) = *(locals + 5); // w0
    *(regs + 1) = *(locals + 6); // w1
    U_NEWVEC(vm, regs + 2, 9, 2);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[4]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function322
static void fun_355(VMRef vm, StackPtr psp) {
    Value regs[10];
    Value locals[36];
    BackupVar(vm, 727);
    *(locals + 0) = Value(0, lobster::V_NIL);
    *(locals + 1) = Value(0, lobster::V_NIL);
    BackupVar(vm, 743);
    BackupVar(vm, 744);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    *(locals + 5) = Value(0, lobster::V_NIL);
    *(locals + 6) = Value(0, lobster::V_NIL);
    *(locals + 7) = Value(0, lobster::V_NIL);
    *(locals + 8) = Value(0, lobster::V_NIL);
    *(locals + 9) = Value(0, lobster::V_NIL);
    *(locals + 10) = Value(0, lobster::V_NIL);
    *(locals + 11) = Value(0, lobster::V_NIL);
    *(locals + 12) = Value(0, lobster::V_NIL);
    *(locals + 13) = Value(0, lobster::V_NIL);
    *(locals + 14) = Value(0, lobster::V_NIL);
    *(locals + 15) = Value(0, lobster::V_NIL);
    *(locals + 16) = Value(0, lobster::V_NIL);
    *(locals + 17) = Value(0, lobster::V_NIL);
    BackupVar(vm, 773);
    BackupVar(vm, 774);
    *(locals + 18) = Value(0, lobster::V_NIL);
    *(locals + 19) = Value(0, lobster::V_NIL);
    *(locals + 20) = Value(0, lobster::V_NIL);
    *(locals + 21) = Value(0, lobster::V_NIL);
    *(locals + 22) = Value(0, lobster::V_NIL);
    *(locals + 23) = Value(0, lobster::V_NIL);
    *(locals + 24) = Value(0, lobster::V_NIL);
    *(locals + 25) = Value(0, lobster::V_NIL);
    *(locals + 26) = Value(0, lobster::V_NIL);
    *(locals + 27) = Value(0, lobster::V_NIL);
    *(locals + 28) = Value(0, lobster::V_NIL);
    *(locals + 29) = Value(0, lobster::V_NIL);
    *(locals + 30) = Value(0, lobster::V_NIL);
    *(locals + 31) = Value(0, lobster::V_NIL);
    *(locals + 32) = Value(0, lobster::V_NIL);
    *(locals + 33) = Value(0, lobster::V_NIL);
    *(locals + 34) = Value(0, lobster::V_NIL);
    *(locals + 35) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 2334, locals);
    vm.last = {8, 19};
    *(regs + 0) = Value(0);
    U_BCALLRET1(vm, regs + 1, 97, 0); // rnd_seed
    vm.last = {10, 19};
    vm.last = {10, 19};
    vm.last = {14, 19};
    vm.last = {14, 19};
    vm.last = {18, 19};
    U_NEWVEC(vm, regs + 0, 1148, 0);
    U_LVAL_VARF(vm, regs + 1, 727); // verts
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {19, 19};
    U_NEWVEC(vm, regs + 0, 13, 0);
    vm.temp_lval = locals + 0; // edges
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {20, 19};
    U_NEWVEC(vm, regs + 0, 13, 0);
    vm.temp_lval = locals + 1; // quads
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {22, 19};
    vm.last = {36, 19};
    U_PUSHVARVF(vm, regs + 0, 125, 2); // int2_1+0
    *(regs + 2) = Value(9);
    U_IVSMUL(vm, regs + 3, 2);
    U_LVAL_VARF(vm, regs + 2, 743); // wsize+0
    U_LV_WRITEV(vm, regs + 2, 2);
    vm.last = {37, 19};
    U_NEWVEC(vm, regs + 0, 1148, 0);
    U_LVAL_VARF(vm, regs + 1, 727); // verts
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {38, 19};
    U_NEWVEC(vm, regs + 0, 13, 0);
    vm.temp_lval = locals + 0; // edges
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {39, 19};
    U_PUSHVARVF(vm, regs + 0, 743, 2); // wsize+0
    U_PUSHFUN(vm, regs + 2, 0, fun_358);
    fun_98(vm, regs + 3); // call: forxy
    vm.last = {41, 19};
    block0:;
    fun_356(vm, regs + 0); // call: wiggleverts
    *(regs + 1) = Value(1.000000);
    U_FGT(vm, regs + 2);
    if (!(regs + 0)->True()) goto block1;
    vm.last = {41, 19};
    goto block0;
    block1:;
    vm.last = {42, 19};
    vm.last = {42, 19};
    vm.last = {46, 19};
    U_NEWVEC(vm, regs + 0, 1165, 0);
    vm.temp_lval = locals + 2; // pedges
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {47, 19};
    *(regs + 0) = Value(-1);
    U_PUSHVARF(vm, regs + 1, 727); // verts
    block2:;
    if (!U_VFOR(vm, regs + 2)) goto block3;
    vm.last = {47, 19};
    U_VFORELEM2S(vm, regs + 2);
    vm.temp_lval = locals + 3; // v+0
    U_LV_WRITEV(vm, regs + 4, 2);
    vm.last = {47, 19};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 5; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {48, 19};
    *(regs + 2) = Value(-1);
    U_PUSHVARF(vm, regs + 3, 727); // verts
    block4:;
    if (!U_VFOR(vm, regs + 4)) goto block5;
    vm.last = {48, 19};
    U_VFORELEM2S(vm, regs + 4);
    vm.temp_lval = locals + 6; // w+0
    U_LV_WRITEV(vm, regs + 6, 2);
    vm.last = {48, 19};
    U_FORLOOPI(vm, regs + 4);
    vm.temp_lval = locals + 8; // j
    *(vm.temp_lval) = *(regs + 4);
    vm.last = {49, 19};
    *(regs + 4) = *(locals + 5); // i
    *(regs + 5) = *(locals + 8); // j
    U_ILT(vm, regs + 6);
    if (!(regs + 4)->True()) goto block6;
    vm.last = {50, 19};
    *(regs + 4) = *(locals + 6); // w.0
    *(regs + 5) = *(locals + 7); // w.1
    *(regs + 6) = *(locals + 3); // v.0
    *(regs + 7) = *(locals + 4); // v.1
    U_FVVSUB(vm, regs + 8, 2);
    *(regs + 6) = Value(2);
    U_BCALLRETV(vm, regs + 7, 86, 1); // magnitude
    vm.temp_lval = locals + 9; // dist
    *(vm.temp_lval) = *(regs + 4);
    vm.last = {51, 19};
    *(regs + 4) = *(locals + 9); // dist
    *(regs + 5) = Value(2.000000);
    U_FLT(vm, regs + 6);
    if (!(regs + 4)->True()) goto block7;
    vm.last = {52, 19};
    *(regs + 4) = *(locals + 2); // pedges
    *(regs + 5) = *(locals + 5); // i
    *(regs + 6) = *(locals + 8); // j
    *(regs + 7) = *(locals + 9); // dist
    *(regs + 8) = Value(3);
    U_BCALLRETV(vm, regs + 9, 15, 1); // push
    U_POP(vm, regs + 5);
    block7:;
    block6:;
    goto block4;
    block5:;
    goto block2;
    block3:;
    vm.last = {53, 19};
    *(regs + 0) = *(locals + 2); // pedges
    (regs + 0)->LTINCRTNIL();
    U_PUSHFUN(vm, regs + 1, 0, fun_359);
    fun_588(vm, regs + 2); // call: qsort
    vm.temp_lval = locals + 2; // pedges
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {54, 19};
    *(regs + 0) = *(locals + 2); // pedges
    *(regs + 1) = Value(0);
    U_VPUSHIDXIS2V(vm, regs + 2, 1, 2);
    *(regs + 1) = Value(2.000000);
    U_BCALLRET1(vm, regs + 2, 56, 1); // sqrt
    U_FMUL(vm, regs + 2);
    vm.temp_lval = locals + 10; // edgedistnocross
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {55, 19};
    U_PUSHVARF(vm, regs + 0, 727); // verts
    (regs + 0)->LTINCRTNIL();
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_592(vm, regs + 2); // call: map
    vm.temp_lval = locals + 11; // vnbs
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {56, 19};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 2); // pedges
    block8:;
    if (!U_VFOR(vm, regs + 2)) goto block9;
    vm.last = {56, 19};
    U_VFORELEM2S(vm, regs + 2);
    vm.temp_lval = locals + 12; // pe+0
    U_LV_WRITEV(vm, regs + 5, 3);
    vm.last = {57, 19};
    *(regs + 2) = *(locals + 14); // pe+2
    *(regs + 3) = *(locals + 10); // edgedistnocross
    U_FLT(vm, regs + 4);
    if (!(regs + 2)->True()) goto block10;
    vm.last = {58, 19};
    *(regs + 2) = *(locals + 0); // edges
    *(regs + 3) = *(locals + 12); // pe+0
    *(regs + 4) = *(locals + 13); // pe+1
    U_NEWVEC(vm, regs + 5, 7, 2);
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    vm.last = {59, 19};
    *(regs + 2) = *(locals + 11); // vnbs
    *(regs + 3) = *(locals + 12); // pe+0
    U_VPUSHIDXI(vm, regs + 4);
    *(regs + 3) = *(locals + 13); // pe+1
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    vm.last = {60, 19};
    *(regs + 2) = *(locals + 11); // vnbs
    *(regs + 3) = *(locals + 13); // pe+1
    U_VPUSHIDXI(vm, regs + 4);
    *(regs + 3) = *(locals + 12); // pe+0
    *(regs + 4) = Value(1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    block10:;
    goto block8;
    block9:;
    vm.last = {61, 19};
    U_NEWVEC(vm, regs + 0, 1165, 0);
    vm.temp_lval = locals + 2; // pedges
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {64, 19};
    *(regs + 0) = *(locals + 11); // vnbs
    (regs + 0)->LTINCRTNIL();
    U_PUSHFUN(vm, regs + 1, 0, fun_361);
    fun_593(vm, regs + 2); // call: map
    vm.temp_lval = locals + 11; // vnbs
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {65, 19};
    U_NEWVEC(vm, regs + 0, 13, 0);
    vm.temp_lval = locals + 15; // tris
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {66, 19};
    U_NEWVEC(vm, regs + 0, 13, 0);
    vm.temp_lval = locals + 1; // quads
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {67, 19};
    *(regs + 0) = Value(-1);
    U_PUSHVARF(vm, regs + 1, 727); // verts
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    block11:;
    if (!U_IFOR(vm, regs + 2)) goto block12;
    vm.last = {67, 19};
    U_IFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 16; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {68, 19};
    *(regs + 2) = Value(-1);
    *(regs + 3) = *(locals + 11); // vnbs
    *(regs + 4) = *(locals + 16); // i
    U_VPUSHIDXI(vm, regs + 5);
    block13:;
    if (!U_VFOR(vm, regs + 4)) goto block14;
    vm.last = {68, 19};
    U_VFORELEM(vm, regs + 4);
    vm.temp_lval = locals + 17; // j
    *(vm.temp_lval) = *(regs + 4);
    vm.last = {69, 19};
    *(regs + 4) = *(locals + 16); // i
    *(regs + 5) = *(locals + 17); // j
    U_NEWVEC(vm, regs + 6, 7, 2);
    U_LVAL_VARF(vm, regs + 5, 773); // shape
    U_LV_WRITEREF(vm, regs + 5);
    vm.last = {70, 19};
    *(regs + 4) = *(locals + 16); // i
    U_LVAL_VARF(vm, regs + 5, 774); // p
    *(vm.temp_lval) = *(regs + 4);
    vm.last = {71, 19};
    block15:;
    U_PUSHVARF(vm, regs + 4, 773); // shape
    U_BCALLRETV(vm, regs + 5, 16, 1); // pop
    *(regs + 5) = *(locals + 16); // i
    U_INE(vm, regs + 6);
    if (!(regs + 4)->True()) goto block16;
    vm.last = {72, 19};
    U_PUSHVARF(vm, regs + 4, 773); // shape
    *(regs + 5) = *(locals + 17); // j
    *(regs + 6) = Value(1);
    U_BCALLRETV(vm, regs + 7, 15, 1); // push
    U_POP(vm, regs + 5);
    vm.last = {73, 19};
    *(regs + 4) = *(locals + 11); // vnbs
    *(regs + 5) = *(locals + 17); // j
    U_VPUSHIDXI(vm, regs + 6);
    (regs + 4)->LTINCRTNIL();
    vm.temp_lval = locals + 18; // nbl
    U_LV_WRITEREF(vm, regs + 5);
    vm.last = {74, 19};
    *(regs + 4) = *(locals + 18); // nbl
    U_PUSHFUN(vm, regs + 5, 0, fun_363);
    fun_597(vm, regs + 6); // call: find
    vm.temp_lval = locals + 19; // pidx
    *(vm.temp_lval) = *(regs + 4);
    vm.last = {75, 19};
    *(regs + 4) = *(locals + 19); // pidx
    *(regs + 5) = Value(0);
    U_IGE(vm, regs + 6);
    U_ASSERT(vm, regs + 5, 75, 19, 537);
    vm.last = {76, 19};
    *(regs + 4) = *(locals + 17); // j
    U_LVAL_VARF(vm, regs + 5, 774); // p
    *(vm.temp_lval) = *(regs + 4);
    vm.last = {77, 19};
    *(regs + 4) = *(locals + 18); // nbl
    *(regs + 5) = *(locals + 19); // pidx
    *(regs + 6) = Value(1);
    U_IADD(vm, regs + 7);
    *(regs + 6) = *(locals + 18); // nbl
    U_BCALLRET1(vm, regs + 7, 13, 1); // length
    U_IMOD(vm, regs + 7);
    U_VPUSHIDXI(vm, regs + 6);
    vm.temp_lval = locals + 17; // j
    *(vm.temp_lval) = *(regs + 4);
    vm.last = {78, 19};
    U_PUSHVARF(vm, regs + 4, 773); // shape
    *(regs + 5) = *(locals + 17); // j
    *(regs + 6) = Value(1);
    U_BCALLRETV(vm, regs + 7, 15, 1); // push
    U_POP(vm, regs + 5);
    goto block15;
    block16:;
    vm.last = {79, 19};
    U_PUSHVARF(vm, regs + 4, 773); // shape
    U_BCALLRET1(vm, regs + 5, 13, 1); // length
    *(regs + 5) = Value(4);
    U_ILE(vm, regs + 6);
    if (!(regs + 4)->True()) goto block17;
    vm.last = {80, 19};
    U_PUSHVARF(vm, regs + 4, 773); // shape
    (regs + 4)->LTINCRTNIL();
    U_PUSHFUN(vm, regs + 5, 0, fun_364);
    fun_598(vm, regs + 6); // call: reduce
    vm.temp_lval = locals + 20; // start
    *(vm.temp_lval) = *(regs + 4);
    vm.last = {81, 19};
    block18:;
    U_PUSHVARF(vm, regs + 4, 773); // shape
    *(regs + 5) = Value(0);
    U_VPUSHIDXI(vm, regs + 6);
    *(regs + 5) = *(locals + 20); // start
    U_INE(vm, regs + 6);
    if (!(regs + 4)->True()) goto block19;
    vm.last = {82, 19};
    U_PUSHVARF(vm, regs + 4, 773); // shape
    *(regs + 5) = Value(0);
    U_PUSHVARF(vm, regs + 6, 773); // shape
    U_BCALLRETV(vm, regs + 7, 16, 1); // pop
    *(regs + 7) = Value(1);
    U_BCALLRETV(vm, regs + 8, 18, 1); // insert
    U_POP(vm, regs + 5);
    goto block18;
    block19:;
    vm.last = {83, 19};
    U_PUSHVARF(vm, regs + 4, 773); // shape
    U_BCALLRET1(vm, regs + 5, 13, 1); // length
    *(regs + 5) = Value(4);
    U_IEQ(vm, regs + 6);
    if (!(regs + 4)->True()) goto block20;
    vm.last = {84, 19};
    *(regs + 4) = *(locals + 1); // quads
    (regs + 4)->LTINCRTNIL();
    U_PUSHFUN(vm, regs + 5, 0, fun_365);
    fun_599(vm, regs + 6); // call: exists
    U_LOGNOT(vm, regs + 5);
    if (!(regs + 4)->True()) goto block21;
    vm.last = {85, 19};
    *(regs + 4) = *(locals + 1); // quads
    U_PUSHVARF(vm, regs + 5, 773); // shape
    (regs + 5)->LTINCRTNIL();
    *(regs + 6) = Value(1);
    U_BCALLRETV(vm, regs + 7, 15, 1); // push
    U_POP(vm, regs + 5);
    block21:;
    goto block22;
    block20:;
    vm.last = {86, 19};
    U_PUSHVARF(vm, regs + 4, 773); // shape
    U_BCALLRET1(vm, regs + 5, 13, 1); // length
    *(regs + 5) = Value(3);
    U_IEQ(vm, regs + 6);
    if (!(regs + 4)->True()) goto block23;
    vm.last = {87, 19};
    *(regs + 4) = *(locals + 15); // tris
    U_PUSHFUN(vm, regs + 5, 0, fun_366);
    fun_600(vm, regs + 6); // call: exists
    U_LOGNOT(vm, regs + 5);
    if (!(regs + 4)->True()) goto block24;
    vm.last = {88, 19};
    *(regs + 4) = *(locals + 15); // tris
    U_PUSHVARF(vm, regs + 5, 773); // shape
    (regs + 5)->LTINCRTNIL();
    *(regs + 6) = Value(1);
    U_BCALLRETV(vm, regs + 7, 15, 1); // push
    U_POP(vm, regs + 5);
    block24:;
    block23:;
    block22:;
    block17:;
    goto block13;
    block14:;
    goto block11;
    block12:;
    vm.last = {89, 19};
    block25:;
    *(regs + 0) = *(locals + 15); // tris
    U_BCALLRET1(vm, regs + 1, 13, 1); // length
    if (!(regs + 0)->True()) goto block26;
    vm.last = {90, 19};
    *(regs + 0) = Value(-1);
    vm.temp_lval = locals + 21; // besti
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {91, 19};
    *(regs + 0) = Value(-1);
    vm.temp_lval = locals + 22; // bestqc
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {92, 19};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 15); // tris
    block27:;
    if (!U_VFOR(vm, regs + 2)) goto block28;
    vm.last = {92, 19};
    U_VFORELEMREF(vm, regs + 2);
    vm.temp_lval = locals + 23; // t
    U_LV_WRITEREF(vm, regs + 3);
    vm.last = {92, 19};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 24; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {93, 19};
    *(regs + 2) = Value(0);
    vm.temp_lval = locals + 25; // qc
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {94, 19};
    *(regs + 2) = Value(-1);
    *(regs + 3) = *(locals + 23); // t
    block29:;
    if (!U_VFOR(vm, regs + 4)) goto block30;
    vm.last = {94, 19};
    U_VFORELEM(vm, regs + 4);
    vm.temp_lval = locals + 26; // vi1
    *(vm.temp_lval) = *(regs + 4);
    vm.last = {95, 19};
    *(regs + 4) = Value(-1);
    *(regs + 5) = *(locals + 1); // quads
    block31:;
    if (!U_VFOR(vm, regs + 6)) goto block32;
    vm.last = {95, 19};
    U_VFORELEMREF(vm, regs + 6);
    vm.temp_lval = locals + 27; // q
    U_LV_WRITEREF(vm, regs + 7);
    vm.last = {96, 19};
    *(regs + 6) = Value(-1);
    *(regs + 7) = *(locals + 27); // q
    block33:;
    if (!U_VFOR(vm, regs + 8)) goto block34;
    vm.last = {96, 19};
    U_VFORELEM(vm, regs + 8);
    vm.temp_lval = locals + 28; // vi2
    *(vm.temp_lval) = *(regs + 8);
    vm.last = {97, 19};
    *(regs + 8) = *(locals + 26); // vi1
    *(regs + 9) = *(locals + 28); // vi2
    U_IEQ(vm, regs + 10);
    if (!(regs + 8)->True()) goto block35;
    vm.last = {98, 19};
    vm.temp_lval = locals + 25; // qc
    U_LV_IPP(vm, regs + 8);
    block35:;
    goto block33;
    block34:;
    goto block31;
    block32:;
    goto block29;
    block30:;
    vm.last = {99, 19};
    *(regs + 2) = *(locals + 25); // qc
    *(regs + 3) = *(locals + 22); // bestqc
    U_IGT(vm, regs + 4);
    if (!(regs + 2)->True()) goto block36;
    vm.last = {100, 19};
    *(regs + 2) = *(locals + 25); // qc
    vm.temp_lval = locals + 22; // bestqc
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {101, 19};
    *(regs + 2) = *(locals + 24); // i
    vm.temp_lval = locals + 21; // besti
    *(vm.temp_lval) = *(regs + 2);
    block36:;
    goto block27;
    block28:;
    vm.last = {102, 19};
    *(regs + 0) = *(locals + 15); // tris
    *(regs + 1) = *(locals + 21); // besti
    U_BCALLRETV(vm, regs + 2, 19, 1); // remove
    vm.temp_lval = locals + 29; // t1
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {103, 19};
    *(regs + 0) = Value(0);
    vm.temp_lval = locals + 30; // found
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {104, 19};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 29); // t1
    block37:;
    if (!U_VFOR(vm, regs + 2)) goto block38;
    vm.last = {104, 19};
    U_VFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 31; // vi
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {104, 19};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 32; // vii
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {105, 19};
    *(regs + 2) = *(locals + 30); // found
    U_LOGNOT(vm, regs + 3);
    if (!(regs + 2)->True()) goto block39;
    vm.last = {106, 19};
    *(regs + 2) = *(locals + 29); // t1
    *(regs + 3) = *(locals + 32); // vii
    *(regs + 4) = Value(1);
    U_IADD(vm, regs + 5);
    *(regs + 4) = Value(3);
    U_IMOD(vm, regs + 5);
    U_VPUSHIDXI(vm, regs + 4);
    vm.temp_lval = locals + 33; // ni
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {107, 19};
    *(regs + 2) = *(locals + 29); // t1
    *(regs + 3) = *(locals + 32); // vii
    *(regs + 4) = Value(2);
    U_IADD(vm, regs + 5);
    *(regs + 4) = Value(3);
    U_IMOD(vm, regs + 5);
    U_VPUSHIDXI(vm, regs + 4);
    vm.temp_lval = locals + 34; // li
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {108, 19};
    *(regs + 2) = Value(-1);
    *(regs + 3) = *(locals + 15); // tris
    block40:;
    if (!U_VFOR(vm, regs + 4)) goto block41;
    vm.last = {108, 19};
    U_VFORELEMREF(vm, regs + 4);
    vm.temp_lval = locals + 35; // t
    U_LV_WRITEREF(vm, regs + 5);
    vm.last = {109, 19};
    *(regs + 4) = *(locals + 30); // found
    U_LOGNOT(vm, regs + 5);
    if (!(regs + 4)->True()) goto block42;
    vm.last = {110, 19};
    *(regs + 4) = Value(1);
    vm.temp_lval = locals + 30; // found
    *(vm.temp_lval) = *(regs + 4);
    vm.last = {111, 19};
    *(regs + 4) = *(locals + 35); // t
    *(regs + 5) = Value(0);
    U_VPUSHIDXI(vm, regs + 6);
    *(regs + 5) = *(locals + 33); // ni
    U_IEQ(vm, regs + 6);
    if (!(regs + 4)->True()) goto block43;
    U_POP(vm, regs + 5);
    *(regs + 4) = *(locals + 35); // t
    *(regs + 5) = Value(1);
    U_VPUSHIDXI(vm, regs + 6);
    *(regs + 5) = *(locals + 31); // vi
    U_IEQ(vm, regs + 6);
    block43:;
    if (!(regs + 4)->True()) goto block44;
    vm.last = {112, 19};
    *(regs + 4) = *(locals + 1); // quads
    *(regs + 5) = *(locals + 31); // vi
    *(regs + 6) = *(locals + 35); // t
    *(regs + 7) = Value(2);
    U_VPUSHIDXI(vm, regs + 8);
    *(regs + 7) = *(locals + 33); // ni
    *(regs + 8) = *(locals + 34); // li
    U_NEWVEC(vm, regs + 9, 7, 4);
    *(regs + 6) = Value(1);
    U_BCALLRETV(vm, regs + 7, 15, 1); // push
    U_POP(vm, regs + 5);
    goto block45;
    block44:;
    vm.last = {113, 19};
    *(regs + 4) = *(locals + 35); // t
    *(regs + 5) = Value(1);
    U_VPUSHIDXI(vm, regs + 6);
    *(regs + 5) = *(locals + 33); // ni
    U_IEQ(vm, regs + 6);
    if (!(regs + 4)->True()) goto block46;
    U_POP(vm, regs + 5);
    *(regs + 4) = *(locals + 35); // t
    *(regs + 5) = Value(2);
    U_VPUSHIDXI(vm, regs + 6);
    *(regs + 5) = *(locals + 31); // vi
    U_IEQ(vm, regs + 6);
    block46:;
    if (!(regs + 4)->True()) goto block47;
    vm.last = {114, 19};
    *(regs + 4) = *(locals + 1); // quads
    *(regs + 5) = *(locals + 31); // vi
    *(regs + 6) = *(locals + 35); // t
    *(regs + 7) = Value(0);
    U_VPUSHIDXI(vm, regs + 8);
    *(regs + 7) = *(locals + 33); // ni
    *(regs + 8) = *(locals + 34); // li
    U_NEWVEC(vm, regs + 9, 7, 4);
    *(regs + 6) = Value(1);
    U_BCALLRETV(vm, regs + 7, 15, 1); // push
    U_POP(vm, regs + 5);
    goto block48;
    block47:;
    vm.last = {115, 19};
    *(regs + 4) = *(locals + 35); // t
    *(regs + 5) = Value(2);
    U_VPUSHIDXI(vm, regs + 6);
    *(regs + 5) = *(locals + 33); // ni
    U_IEQ(vm, regs + 6);
    if (!(regs + 4)->True()) goto block49;
    U_POP(vm, regs + 5);
    *(regs + 4) = *(locals + 35); // t
    *(regs + 5) = Value(0);
    U_VPUSHIDXI(vm, regs + 6);
    *(regs + 5) = *(locals + 31); // vi
    U_IEQ(vm, regs + 6);
    block49:;
    if (!(regs + 4)->True()) goto block50;
    vm.last = {116, 19};
    *(regs + 4) = *(locals + 1); // quads
    *(regs + 5) = *(locals + 31); // vi
    *(regs + 6) = *(locals + 35); // t
    *(regs + 7) = Value(1);
    U_VPUSHIDXI(vm, regs + 8);
    *(regs + 7) = *(locals + 33); // ni
    *(regs + 8) = *(locals + 34); // li
    U_NEWVEC(vm, regs + 9, 7, 4);
    *(regs + 6) = Value(1);
    U_BCALLRETV(vm, regs + 7, 15, 1); // push
    U_POP(vm, regs + 5);
    goto block51;
    block50:;
    vm.last = {118, 19};
    *(regs + 4) = Value(0);
    vm.temp_lval = locals + 30; // found
    *(vm.temp_lval) = *(regs + 4);
    block51:;
    block48:;
    block45:;
    vm.last = {119, 19};
    *(regs + 4) = *(locals + 30); // found
    if (!(regs + 4)->True()) goto block52;
    vm.last = {120, 19};
    *(regs + 4) = *(locals + 15); // tris
    *(regs + 5) = *(locals + 35); // t
    U_BCALLRET2(vm, regs + 6, 21, 1); // remove_obj
    U_POP(vm, regs + 5);
    block52:;
    block42:;
    goto block40;
    block41:;
    block39:;
    goto block37;
    block38:;
    goto block25;
    block26:;
    DecOwned(vm, 727);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[2]);
    DecVal(vm, locals[11]);
    DecVal(vm, locals[15]);
    DecOwned(vm, 773);
    DecVal(vm, locals[18]);
    DecVal(vm, locals[23]);
    DecVal(vm, locals[27]);
    DecVal(vm, locals[29]);
    DecVal(vm, locals[35]);
    goto epilogue;
    epilogue:;
    RestoreBackup(vm, 774);
    RestoreBackup(vm, 773);
    RestoreBackup(vm, 744);
    RestoreBackup(vm, 743);
    RestoreBackup(vm, 727);
    PopFunId(vm);
}

// wiggleverts
static void fun_356(VMRef vm, StackPtr psp) {
    Value regs[8];
    Value locals[13];
    *(locals + 0) = Value(0, lobster::V_NIL);
    *(locals + 1) = Value(0, lobster::V_NIL);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    *(locals + 5) = Value(0, lobster::V_NIL);
    *(locals + 6) = Value(0, lobster::V_NIL);
    *(locals + 7) = Value(0, lobster::V_NIL);
    *(locals + 8) = Value(0, lobster::V_NIL);
    *(locals + 9) = Value(0, lobster::V_NIL);
    *(locals + 10) = Value(0, lobster::V_NIL);
    *(locals + 11) = Value(0, lobster::V_NIL);
    *(locals + 12) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 2378, locals);
    vm.last = {23, 19};
    U_PUSHVARF(vm, regs + 0, 727); // verts
    (regs + 0)->LTINCRTNIL();
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_587(vm, regs + 2); // call: map
    vm.temp_lval = locals + 0; // disps
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {24, 19};
    *(regs + 0) = Value(-1);
    U_PUSHVARF(vm, regs + 1, 727); // verts
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {24, 19};
    U_VFORELEM2S(vm, regs + 2);
    vm.temp_lval = locals + 1; // v+0
    U_LV_WRITEV(vm, regs + 4, 2);
    vm.last = {25, 19};
    *(regs + 2) = Value(-1);
    U_PUSHVARF(vm, regs + 3, 727); // verts
    block2:;
    if (!U_VFOR(vm, regs + 4)) goto block3;
    vm.last = {25, 19};
    U_VFORELEM2S(vm, regs + 4);
    vm.temp_lval = locals + 3; // w+0
    U_LV_WRITEV(vm, regs + 6, 2);
    vm.last = {25, 19};
    U_FORLOOPI(vm, regs + 4);
    vm.temp_lval = locals + 5; // i
    *(vm.temp_lval) = *(regs + 4);
    vm.last = {26, 19};
    *(regs + 4) = *(locals + 3); // w.0
    *(regs + 5) = *(locals + 4); // w.1
    *(regs + 6) = *(locals + 1); // v.0
    *(regs + 7) = *(locals + 2); // v.1
    U_FVVSUB(vm, regs + 8, 2);
    vm.temp_lval = locals + 6; // tow+0
    U_LV_WRITEV(vm, regs + 6, 2);
    vm.last = {27, 19};
    *(regs + 4) = *(locals + 6); // tow.0
    *(regs + 5) = *(locals + 7); // tow.1
    *(regs + 6) = Value(2);
    U_BCALLRETV(vm, regs + 7, 86, 1); // magnitude
    vm.temp_lval = locals + 8; // dist
    *(vm.temp_lval) = *(regs + 4);
    vm.last = {28, 19};
    *(regs + 4) = *(locals + 8); // dist
    *(regs + 5) = Value(1.000000);
    U_FLT(vm, regs + 6);
    if (!(regs + 4)->True()) goto block4;
    U_POP(vm, regs + 5);
    *(regs + 4) = *(locals + 1); // v.0
    *(regs + 5) = *(locals + 2); // v.1
    *(regs + 6) = *(locals + 3); // w.0
    *(regs + 7) = *(locals + 4); // w.1
    U_STNE(vm, regs + 8, 2);
    block4:;
    if (!(regs + 4)->True()) goto block5;
    vm.last = {29, 19};
    *(regs + 4) = *(locals + 6); // tow.0
    *(regs + 5) = *(locals + 7); // tow.1
    *(regs + 6) = Value(1.000000);
    *(regs + 7) = *(locals + 8); // dist
    U_FDIV(vm, regs + 8);
    U_FVSMUL(vm, regs + 7, 2);
    *(regs + 6) = Value(1.000000);
    *(regs + 7) = *(locals + 8); // dist
    U_FSUB(vm, regs + 8);
    U_FVSMUL(vm, regs + 7, 2);
    *(regs + 6) = *(locals + 0); // disps
    *(regs + 7) = *(locals + 5); // i
    U_LVAL_IDXVI(vm, regs + 8, 0);
    U_LV_FVVADD(vm, regs + 6, 2);
    block5:;
    goto block2;
    block3:;
    goto block0;
    block1:;
    vm.last = {30, 19};
    *(regs + 0) = Value(0.000000);
    vm.temp_lval = locals + 9; // totaldisp
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {31, 19};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 0); // disps
    block6:;
    if (!U_VFOR(vm, regs + 2)) goto block7;
    vm.last = {31, 19};
    U_VFORELEM2S(vm, regs + 2);
    vm.temp_lval = locals + 10; // d+0
    U_LV_WRITEV(vm, regs + 4, 2);
    vm.last = {31, 19};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 12; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {32, 19};
    *(regs + 2) = *(locals + 10); // d.0
    *(regs + 3) = *(locals + 11); // d.1
    U_PUSHVARF(vm, regs + 4, 727); // verts
    *(regs + 5) = *(locals + 12); // i
    U_LVAL_IDXVI(vm, regs + 6, 0);
    U_LV_FVVADD(vm, regs + 4, 2);
    vm.last = {33, 19};
    *(regs + 2) = *(locals + 10); // d.0
    *(regs + 3) = *(locals + 11); // d.1
    *(regs + 4) = Value(2);
    U_BCALLRETV(vm, regs + 5, 86, 1); // magnitude
    vm.temp_lval = locals + 9; // totaldisp
    U_LV_FADD(vm, regs + 3);
    goto block6;
    block7:;
    vm.last = {34, 19};
    *(regs + 0) = *(locals + 9); // totaldisp
    DecVal(vm, locals[0]);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function325
static void fun_358(VMRef vm, StackPtr psp) {
    Value regs[6];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2394, locals);
    vm.last = {40, 19};
    U_PUSHVARF(vm, regs + 0, 727); // verts
    *(regs + 1) = *(locals + 0); // p.0
    *(regs + 2) = *(locals + 1); // p.1
    *(regs + 3) = Value(2);
    U_BCALLRETV(vm, regs + 4, 68, 1); // float
    fun_89(vm, regs + 3); // call: rnd_float2
    U_FVVADD(vm, regs + 5, 2);
    U_PUSHVARVF(vm, regs + 3, 743, 2); // wsize+0
    *(regs + 5) = Value(2);
    U_BCALLRETV(vm, regs + 6, 68, 1); // float
    *(regs + 5) = Value(2.000000);
    U_FVSDIV(vm, regs + 6, 2);
    U_FVVSUB(vm, regs + 5, 2);
    *(regs + 3) = Value(2);
    U_BCALLRETV(vm, regs + 4, 15, 1); // push
    U_POP(vm, regs + 1);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function326
static void fun_359(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[6];
    *(locals + 0) = *(psp - 6);
    *(locals + 1) = *(psp - 5);
    *(locals + 2) = *(psp - 4);
    *(locals + 3) = *(psp - 3);
    *(locals + 4) = *(psp - 2);
    *(locals + 5) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2399, locals);
    vm.last = {53, 19};
    *(regs + 0) = *(locals + 2); // a+2
    *(regs + 1) = *(locals + 5); // b+2
    U_FLT(vm, regs + 2);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function328
static void fun_361(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 2);
    SwapVars(vm, 767, psp, 1);
    PushFunId(vm, funinfo_table + 2408, locals);
    vm.last = {63, 19};
    *(regs + 0) = *(locals + 0); // nbl
    (regs + 0)->LTINCRTNIL();
    U_PUSHFUN(vm, regs + 1, 0, fun_362);
    fun_594(vm, regs + 2); // call: qsort
    psp = PopArg(vm, 767, psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function329
static void fun_362(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2413, locals);
    vm.last = {64, 19};
    U_PUSHVARF(vm, regs + 0, 727); // verts
    *(regs + 1) = *(locals + 0); // a
    U_VPUSHIDXI2V(vm, regs + 2);
    U_PUSHVARF(vm, regs + 2, 727); // verts
    U_PUSHVARF(vm, regs + 3, 767); // i
    U_VPUSHIDXI2V(vm, regs + 4);
    U_FVVSUB(vm, regs + 4, 2);
    *(regs + 2) = Value(2);
    U_BCALLRETV(vm, regs + 3, 81, 1); // atan2
    U_PUSHVARF(vm, regs + 1, 727); // verts
    *(regs + 2) = *(locals + 1); // b
    U_VPUSHIDXI2V(vm, regs + 3);
    U_PUSHVARF(vm, regs + 3, 727); // verts
    U_PUSHVARF(vm, regs + 4, 767); // i
    U_VPUSHIDXI2V(vm, regs + 5);
    U_FVVSUB(vm, regs + 5, 2);
    *(regs + 3) = Value(2);
    U_BCALLRETV(vm, regs + 4, 81, 1); // atan2
    U_FLT(vm, regs + 2);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function330
static void fun_363(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2418, locals);
    vm.last = {74, 19};
    *(regs + 0) = *(locals + 0); // _
    U_PUSHVARF(vm, regs + 1, 774); // p
    U_IEQ(vm, regs + 2);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function331
static void fun_364(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2422, locals);
    vm.last = {80, 19};
    *(regs + 0) = *(locals + 0); // _a
    *(regs + 1) = *(locals + 1); // _b
    U_BCALLRET2(vm, regs + 2, 120, 1); // min
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function332
static void fun_365(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2427, locals);
    vm.last = {84, 19};
    *(regs + 0) = *(locals + 0); // _
    U_PUSHVARF(vm, regs + 1, 773); // shape
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function333
static void fun_366(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2431, locals);
    vm.last = {87, 19};
    *(regs + 0) = *(locals + 0); // _
    U_PUSHVARF(vm, regs + 1, 773); // shape
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function337
static void fun_370(VMRef vm, StackPtr psp) {
    Value regs[19];
    Value locals[29];
    *(locals + 0) = Value(0, lobster::V_NIL);
    *(locals + 1) = Value(0, lobster::V_NIL);
    BackupVar(vm, 946);
    BackupVar(vm, 1015);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    *(locals + 5) = Value(0, lobster::V_NIL);
    *(locals + 6) = Value(0, lobster::V_NIL);
    *(locals + 7) = Value(0, lobster::V_NIL);
    *(locals + 8) = Value(0, lobster::V_NIL);
    *(locals + 9) = Value(0, lobster::V_NIL);
    *(locals + 10) = Value(0, lobster::V_NIL);
    *(locals + 11) = Value(0, lobster::V_NIL);
    *(locals + 12) = Value(0, lobster::V_NIL);
    *(locals + 13) = Value(0, lobster::V_NIL);
    *(locals + 14) = Value(0, lobster::V_NIL);
    *(locals + 15) = Value(0, lobster::V_NIL);
    BackupVar(vm, 1030);
    *(locals + 16) = Value(0, lobster::V_NIL);
    *(locals + 17) = Value(0, lobster::V_NIL);
    *(locals + 18) = Value(0, lobster::V_NIL);
    *(locals + 19) = Value(0, lobster::V_NIL);
    *(locals + 20) = Value(0, lobster::V_NIL);
    *(locals + 21) = Value(0, lobster::V_NIL);
    *(locals + 22) = Value(0, lobster::V_NIL);
    *(locals + 23) = Value(0, lobster::V_NIL);
    *(locals + 24) = Value(0, lobster::V_NIL);
    *(locals + 25) = Value(0, lobster::V_NIL);
    *(locals + 26) = Value(0, lobster::V_NIL);
    *(locals + 27) = Value(0, lobster::V_NIL);
    *(locals + 28) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 2435, locals);
    vm.last = {9, 20};
    vm.last = {9, 20};
    vm.last = {13, 20};
    vm.last = {18, 20};
    vm.last = {18, 20};
    vm.last = {25, 20};
    vm.last = {41, 20};
    *(regs + 0) = Value(1000.000000);
    vm.temp_lval = locals + 0; // bigrad
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {42, 20};
    *(regs + 0) = Value(100.000000);
    vm.temp_lval = locals + 1; // lrad
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {44, 20};
    *(regs + 0) = *(locals + 1); // lrad
    *(regs + 1) = Value(50.000000);
    *(regs + 2) = *(locals + 1); // lrad
    U_PUSHFLT64(vm, regs + 3, 1717986918, 1079273062); // 81.6
    U_FADD(vm, regs + 4);
    *(regs + 3) = Value(1.000000);
    U_FSUB(vm, regs + 4);
    U_PUSHFLT64(vm, regs + 3, 1717986918, 1079273062); // 81.6
    U_PUSHVARVF(vm, regs + 4, 87, 3); // float3_1+0
    *(regs + 7) = Value(12.000000);
    U_FVSMUL(vm, regs + 8, 3);
    U_PUSHVARVF(vm, regs + 7, 84, 3); // float3_0+0
    *(regs + 10) = Value(0);
    U_NEWOBJECT(vm, regs + 11, 1185); // Sphere
    *(regs + 1) = Value(16.500000);
    *(regs + 2) = Value(73.000000);
    *(regs + 3) = Value(16.500000);
    *(regs + 4) = Value(78.000000);
    U_PUSHVARVF(vm, regs + 5, 84, 3); // float3_0+0
    U_PUSHVARVF(vm, regs + 8, 87, 3); // float3_1+0
    U_PUSHFLT64(vm, regs + 11, -652835029, 1072691150); // 0.999
    U_FVSMUL(vm, regs + 12, 3);
    *(regs + 11) = Value(2);
    U_NEWOBJECT(vm, regs + 12, 1185); // Sphere
    *(regs + 2) = Value(16.500000);
    *(regs + 3) = Value(27.000000);
    *(regs + 4) = Value(16.500000);
    *(regs + 5) = Value(47.000000);
    U_PUSHVARVF(vm, regs + 6, 84, 3); // float3_0+0
    U_PUSHVARVF(vm, regs + 9, 87, 3); // float3_1+0
    U_PUSHFLT64(vm, regs + 12, -652835029, 1072691150); // 0.999
    U_FVSMUL(vm, regs + 13, 3);
    *(regs + 12) = Value(1);
    U_NEWOBJECT(vm, regs + 13, 1185); // Sphere
    *(regs + 3) = *(locals + 0); // bigrad
    *(regs + 4) = Value(50.000000);
    *(regs + 5) = *(locals + 0); // bigrad
    U_FUMINUS(vm, regs + 6);
    U_PUSHFLT64(vm, regs + 6, 1717986918, 1079273062); // 81.6
    U_FADD(vm, regs + 7);
    U_PUSHFLT64(vm, regs + 6, 1717986918, 1079273062); // 81.6
    U_PUSHVARVF(vm, regs + 7, 84, 3); // float3_0+0
    U_PUSHVARVF(vm, regs + 10, 87, 3); // float3_1+0
    *(regs + 13) = Value(0.750000);
    U_FVSMUL(vm, regs + 14, 3);
    *(regs + 13) = Value(0);
    U_NEWOBJECT(vm, regs + 14, 1185); // Sphere
    *(regs + 4) = *(locals + 0); // bigrad
    *(regs + 5) = Value(50.000000);
    *(regs + 6) = *(locals + 0); // bigrad
    U_PUSHFLT64(vm, regs + 7, 1717986918, 1079273062); // 81.6
    U_PUSHVARVF(vm, regs + 8, 84, 3); // float3_0+0
    U_PUSHVARVF(vm, regs + 11, 87, 3); // float3_1+0
    *(regs + 14) = Value(0.750000);
    U_FVSMUL(vm, regs + 15, 3);
    *(regs + 14) = Value(0);
    U_NEWOBJECT(vm, regs + 15, 1185); // Sphere
    *(regs + 5) = *(locals + 0); // bigrad
    *(regs + 6) = Value(50.000000);
    U_PUSHFLT64(vm, regs + 7, 1717986918, 1078224486); // 40.8
    *(regs + 8) = *(locals + 0); // bigrad
    U_FUMINUS(vm, regs + 9);
    *(regs + 9) = Value(170.000000);
    U_FADD(vm, regs + 10);
    U_PUSHVARVF(vm, regs + 9, 84, 3); // float3_0+0
    U_PUSHVARVF(vm, regs + 12, 84, 3); // float3_0+0
    *(regs + 15) = Value(0);
    U_NEWOBJECT(vm, regs + 16, 1185); // Sphere
    *(regs + 6) = *(locals + 0); // bigrad
    *(regs + 7) = Value(50.000000);
    U_PUSHFLT64(vm, regs + 8, 1717986918, 1078224486); // 40.8
    *(regs + 9) = *(locals + 0); // bigrad
    U_PUSHVARVF(vm, regs + 10, 84, 3); // float3_0+0
    U_PUSHVARVF(vm, regs + 13, 87, 3); // float3_1+0
    *(regs + 16) = Value(0.750000);
    U_FVSMUL(vm, regs + 17, 3);
    *(regs + 16) = Value(0);
    U_NEWOBJECT(vm, regs + 17, 1185); // Sphere
    *(regs + 7) = *(locals + 0); // bigrad
    *(regs + 8) = *(locals + 0); // bigrad
    U_FUMINUS(vm, regs + 9);
    *(regs + 9) = Value(99.000000);
    U_FADD(vm, regs + 10);
    U_PUSHFLT64(vm, regs + 9, 1717986918, 1078224486); // 40.8
    U_PUSHFLT64(vm, regs + 10, 1717986918, 1079273062); // 81.6
    U_PUSHVARVF(vm, regs + 11, 84, 3); // float3_0+0
    *(regs + 14) = Value(0.250000);
    *(regs + 15) = Value(0.250000);
    *(regs + 16) = Value(0.750000);
    *(regs + 17) = Value(0);
    U_NEWOBJECT(vm, regs + 18, 1185); // Sphere
    *(regs + 8) = *(locals + 0); // bigrad
    *(regs + 9) = *(locals + 0); // bigrad
    *(regs + 10) = Value(1.000000);
    U_FADD(vm, regs + 11);
    U_PUSHFLT64(vm, regs + 10, 1717986918, 1078224486); // 40.8
    U_PUSHFLT64(vm, regs + 11, 1717986918, 1079273062); // 81.6
    U_PUSHVARVF(vm, regs + 12, 84, 3); // float3_0+0
    *(regs + 15) = Value(0.750000);
    *(regs + 16) = Value(0.250000);
    *(regs + 17) = Value(0.250000);
    *(regs + 18) = Value(0);
    U_NEWOBJECT(vm, regs + 19, 1185); // Sphere
    U_NEWVEC(vm, regs + 9, 1250, 9);
    U_LVAL_VARF(vm, regs + 1, 946); // spheres
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {56, 20};
    vm.last = {112, 20};
    *(regs + 0) = Value(32);
    U_LVAL_VARF(vm, regs + 1, 1015); // w
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {113, 20};
    *(regs + 0) = Value(24);
    vm.temp_lval = locals + 2; // h
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {115, 20};
    *(regs + 0) = Value(50.000000);
    *(regs + 1) = Value(50.000000);
    *(regs + 2) = Value(290.000000);
    *(regs + 3) = Value(0.000000);
    U_PUSHFLT64(vm, regs + 4, 1960154354, -1079652035); // -0.042612
    *(regs + 5) = Value(-1.000000);
    *(regs + 6) = Value(3);
    U_BCALLRETV(vm, regs + 7, 84, 1); // normalize
    vm.temp_lval = locals + 3; // cam+0
    U_LV_WRITEV(vm, regs + 6, 6);
    vm.last = {116, 20};
    U_PUSHVARVF(vm, regs + 0, 90, 3); // float3_x+0
    U_PUSHVARF(vm, regs + 3, 1015); // w
    U_I2F(vm, regs + 4);
    U_PUSHFLT64(vm, regs + 4, -1924145349, 1071672983); // 0.5135
    U_FMUL(vm, regs + 5);
    *(regs + 4) = *(locals + 2); // h
    U_I2F(vm, regs + 5);
    U_FDIV(vm, regs + 5);
    U_FVSMUL(vm, regs + 4, 3);
    vm.temp_lval = locals + 9; // cx+0
    U_LV_WRITEV(vm, regs + 3, 3);
    vm.last = {117, 20};
    *(regs + 0) = *(locals + 9); // cx.0
    *(regs + 1) = *(locals + 10); // cx.1
    *(regs + 2) = *(locals + 11); // cx.2
    *(regs + 3) = Value(3);
    *(regs + 4) = *(locals + 6); // cam.0
    *(regs + 5) = *(locals + 7); // cam.1
    *(regs + 6) = *(locals + 8); // cam.2
    *(regs + 7) = Value(3);
    U_BCALLRETV(vm, regs + 8, 90, 1); // cross
    *(regs + 3) = Value(3);
    U_BCALLRETV(vm, regs + 4, 84, 1); // normalize
    U_PUSHFLT64(vm, regs + 3, -1924145349, 1071672983); // 0.5135
    U_FVSMUL(vm, regs + 4, 3);
    vm.temp_lval = locals + 12; // cy+0
    U_LV_WRITEV(vm, regs + 3, 3);
    vm.last = {119, 20};
    *(regs + 0) = *(locals + 2); // h
    U_PUSHFUN(vm, regs + 1, 0, fun_373);
    fun_601(vm, regs + 2); // call: map
    vm.temp_lval = locals + 15; // c
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {121, 20};
    *(regs + 0) = Value(0);
    U_LVAL_VARF(vm, regs + 1, 1030); // samples
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {123, 20};
    *(regs + 0) = Value(0);
    U_BCALLRET1(vm, regs + 1, 97, 0); // rnd_seed
    vm.last = {125, 20};
    *(regs + 0) = Value(-1);
    *(regs + 1) = *(locals + 2); // h
    block0:;
    if (!U_IFOR(vm, regs + 2)) goto block1;
    vm.last = {125, 20};
    U_IFORELEM(vm, regs + 2);
    vm.temp_lval = locals + 16; // y
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {126, 20};
    *(regs + 2) = Value(-1);
    U_PUSHVARF(vm, regs + 3, 1015); // w
    block2:;
    if (!U_IFOR(vm, regs + 4)) goto block3;
    vm.last = {126, 20};
    U_IFORELEM(vm, regs + 4);
    vm.temp_lval = locals + 17; // x
    *(vm.temp_lval) = *(regs + 4);
    vm.last = {127, 20};
    *(regs + 4) = Value(2.000000);
    U_BCALLRET0(vm, regs + 5, 95, 1); // rnd_float
    U_FMUL(vm, regs + 6);
    vm.temp_lval = locals + 18; // r1
    *(vm.temp_lval) = *(regs + 4);
    vm.last = {128, 20};
    *(regs + 4) = *(locals + 18); // r1
    *(regs + 5) = Value(1.000000);
    U_FLT(vm, regs + 6);
    if (!(regs + 4)->True()) goto block4;
    vm.last = {128, 20};
    *(regs + 4) = *(locals + 18); // r1
    U_BCALLRET1(vm, regs + 5, 56, 1); // sqrt
    *(regs + 5) = Value(1.000000);
    U_FSUB(vm, regs + 6);
    goto block5;
    block4:;
    vm.last = {128, 20};
    *(regs + 4) = Value(1.000000);
    *(regs + 5) = Value(2.000000);
    *(regs + 6) = *(locals + 18); // r1
    U_FSUB(vm, regs + 7);
    U_BCALLRET1(vm, regs + 6, 56, 1); // sqrt
    U_FSUB(vm, regs + 6);
    block5:;
    vm.temp_lval = locals + 19; // dx
    *(vm.temp_lval) = *(regs + 4);
    vm.last = {129, 20};
    *(regs + 4) = Value(2.000000);
    U_BCALLRET0(vm, regs + 5, 95, 1); // rnd_float
    U_FMUL(vm, regs + 6);
    vm.temp_lval = locals + 20; // r2
    *(vm.temp_lval) = *(regs + 4);
    vm.last = {130, 20};
    *(regs + 4) = *(locals + 20); // r2
    *(regs + 5) = Value(1.000000);
    U_FLT(vm, regs + 6);
    if (!(regs + 4)->True()) goto block6;
    vm.last = {130, 20};
    *(regs + 4) = *(locals + 20); // r2
    U_BCALLRET1(vm, regs + 5, 56, 1); // sqrt
    *(regs + 5) = Value(1.000000);
    U_FSUB(vm, regs + 6);
    goto block7;
    block6:;
    vm.last = {130, 20};
    *(regs + 4) = Value(1.000000);
    *(regs + 5) = Value(2.000000);
    *(regs + 6) = *(locals + 20); // r2
    U_FSUB(vm, regs + 7);
    U_BCALLRET1(vm, regs + 6, 56, 1); // sqrt
    U_FSUB(vm, regs + 6);
    block7:;
    vm.temp_lval = locals + 21; // dy
    *(vm.temp_lval) = *(regs + 4);
    vm.last = {131, 20};
    *(regs + 4) = *(locals + 9); // cx.0
    *(regs + 5) = *(locals + 10); // cx.1
    *(regs + 6) = *(locals + 11); // cx.2
    *(regs + 7) = Value(1.000000);
    *(regs + 8) = *(locals + 19); // dx
    U_FADD(vm, regs + 9);
    *(regs + 8) = Value(2.000000);
    U_FDIV(vm, regs + 9);
    *(regs + 8) = *(locals + 17); // x
    U_I2F(vm, regs + 9);
    U_FADD(vm, regs + 9);
    U_PUSHVARF(vm, regs + 8, 1015); // w
    U_I2F(vm, regs + 9);
    U_FDIV(vm, regs + 9);
    *(regs + 8) = Value(0.500000);
    U_FSUB(vm, regs + 9);
    U_FVSMUL(vm, regs + 8, 3);
    *(regs + 7) = *(locals + 12); // cy.0
    *(regs + 8) = *(locals + 13); // cy.1
    *(regs + 9) = *(locals + 14); // cy.2
    *(regs + 10) = Value(1.000000);
    *(regs + 11) = *(locals + 21); // dy
    U_FADD(vm, regs + 12);
    *(regs + 11) = Value(2.000000);
    U_FDIV(vm, regs + 12);
    *(regs + 11) = *(locals + 16); // y
    U_I2F(vm, regs + 12);
    U_FADD(vm, regs + 12);
    *(regs + 11) = *(locals + 2); // h
    U_I2F(vm, regs + 12);
    U_FDIV(vm, regs + 12);
    *(regs + 11) = Value(0.500000);
    U_FSUB(vm, regs + 12);
    U_FVSMUL(vm, regs + 11, 3);
    U_FVVADD(vm, regs + 10, 3);
    *(regs + 7) = *(locals + 6); // cam.0
    *(regs + 8) = *(locals + 7); // cam.1
    *(regs + 9) = *(locals + 8); // cam.2
    U_FVVADD(vm, regs + 10, 3);
    vm.temp_lval = locals + 22; // d+0
    U_LV_WRITEV(vm, regs + 7, 3);
    vm.last = {133, 20};
    *(regs + 4) = *(locals + 22); // d.0
    *(regs + 5) = *(locals + 23); // d.1
    *(regs + 6) = *(locals + 24); // d.2
    *(regs + 7) = Value(3);
    U_BCALLRETV(vm, regs + 8, 84, 1); // normalize
    vm.temp_lval = locals + 22; // d+0
    U_LV_WRITEV(vm, regs + 7, 3);
    vm.last = {134, 20};
    *(regs + 4) = *(locals + 3); // cam.0
    *(regs + 5) = *(locals + 4); // cam.1
    *(regs + 6) = *(locals + 5); // cam.2
    *(regs + 7) = *(locals + 22); // d.0
    *(regs + 8) = *(locals + 23); // d.1
    *(regs + 9) = *(locals + 24); // d.2
    *(regs + 10) = Value(140.000000);
    U_FVSMUL(vm, regs + 11, 3);
    U_FVVADD(vm, regs + 10, 3);
    *(regs + 7) = *(locals + 22); // d.0
    *(regs + 8) = *(locals + 23); // d.1
    *(regs + 9) = *(locals + 24); // d.2
    *(regs + 10) = Value(0);
    fun_372(vm, regs + 11); // call: radiance
    vm.temp_lval = locals + 25; // r+0
    U_LV_WRITEV(vm, regs + 7, 3);
    vm.last = {136, 20};
    *(regs + 4) = *(locals + 25); // r.0
    *(regs + 5) = *(locals + 26); // r.1
    *(regs + 6) = *(locals + 27); // r.2
    *(regs + 7) = *(locals + 15); // c
    *(regs + 8) = *(locals + 2); // h
    *(regs + 9) = *(locals + 16); // y
    U_ISUB(vm, regs + 10);
    *(regs + 9) = Value(1);
    U_ISUB(vm, regs + 10);
    U_VPUSHIDXI(vm, regs + 9);
    *(regs + 8) = *(locals + 17); // x
    U_LVAL_IDXVI(vm, regs + 9, 0);
    U_LV_FVVADD(vm, regs + 7, 3);
    goto block2;
    block3:;
    goto block0;
    block1:;
    vm.last = {137, 20};
    U_LVAL_VARF(vm, regs + 0, 1030); // samples
    U_LV_IPP(vm, regs + 0);
    vm.last = {138, 20};
    *(regs + 0) = *(locals + 15); // c
    U_PUSHFUN(vm, regs + 1, 0, fun_375);
    fun_603(vm, regs + 2); // call: map
    vm.temp_lval = locals + 28; // r
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {139, 20};
    *(regs + 0) = *(locals + 28); // r
    U_BCALLRET1(vm, regs + 1, 13, 1); // length
    *(regs + 1) = *(locals + 2); // h
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 139, 20, 538);
    DecOwned(vm, 946);
    DecVal(vm, locals[15]);
    DecVal(vm, locals[28]);
    goto epilogue;
    epilogue:;
    RestoreBackup(vm, 1030);
    RestoreBackup(vm, 1015);
    RestoreBackup(vm, 946);
    PopFunId(vm);
}

// intersect
static void fun_371(VMRef vm, StackPtr psp) {
    Value regs[9];
    Value locals[14];
    *(locals + 0) = *(psp - 7);
    *(locals + 1) = *(psp - 6);
    *(locals + 2) = *(psp - 5);
    *(locals + 3) = *(psp - 4);
    *(locals + 4) = *(psp - 3);
    *(locals + 5) = *(psp - 2);
    *(locals + 6) = *(psp - 1);
    *(locals + 7) = Value(0, lobster::V_NIL);
    *(locals + 8) = Value(0, lobster::V_NIL);
    *(locals + 9) = Value(0, lobster::V_NIL);
    *(locals + 10) = Value(0, lobster::V_NIL);
    *(locals + 11) = Value(0, lobster::V_NIL);
    *(locals + 12) = Value(0, lobster::V_NIL);
    *(locals + 13) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 2470, locals);
    vm.last = {26, 20};
    *(regs + 0) = *(locals + 0); // sphere
    U_PUSHFLD2V(vm, regs + 1, 1, 3);
    *(regs + 3) = *(locals + 1); // r.0
    *(regs + 4) = *(locals + 2); // r.1
    *(regs + 5) = *(locals + 3); // r.2
    U_FVVSUB(vm, regs + 6, 3);
    vm.temp_lval = locals + 7; // op+0
    U_LV_WRITEV(vm, regs + 3, 3);
    vm.last = {27, 20};
    U_PUSHFLT64(vm, regs + 0, -350469331, 1058682594); // 0.0001
    vm.temp_lval = locals + 10; // eps
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {28, 20};
    *(regs + 0) = *(locals + 7); // op.0
    *(regs + 1) = *(locals + 8); // op.1
    *(regs + 2) = *(locals + 9); // op.2
    *(regs + 3) = Value(3);
    *(regs + 4) = *(locals + 4); // r.0
    *(regs + 5) = *(locals + 5); // r.1
    *(regs + 6) = *(locals + 6); // r.2
    *(regs + 7) = Value(3);
    U_BCALLRETV(vm, regs + 8, 85, 1); // dot
    vm.temp_lval = locals + 11; // b
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {29, 20};
    *(regs + 0) = *(locals + 11); // b
    *(regs + 1) = *(locals + 11); // b
    U_FMUL(vm, regs + 2);
    *(regs + 1) = *(locals + 7); // op.0
    *(regs + 2) = *(locals + 8); // op.1
    *(regs + 3) = *(locals + 9); // op.2
    *(regs + 4) = Value(3);
    *(regs + 5) = *(locals + 7); // op.0
    *(regs + 6) = *(locals + 8); // op.1
    *(regs + 7) = *(locals + 9); // op.2
    *(regs + 8) = Value(3);
    U_BCALLRETV(vm, regs + 9, 85, 1); // dot
    U_FSUB(vm, regs + 2);
    *(regs + 1) = *(locals + 0); // sphere
    U_PUSHFLD(vm, regs + 2, 0);
    *(regs + 2) = *(locals + 0); // sphere
    U_PUSHFLD(vm, regs + 3, 0);
    U_FMUL(vm, regs + 3);
    U_FADD(vm, regs + 2);
    vm.temp_lval = locals + 12; // det
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {30, 20};
    *(regs + 0) = *(locals + 12); // det
    *(regs + 1) = Value(0.000000);
    U_FLT(vm, regs + 2);
    if (!(regs + 0)->True()) goto block0;
    vm.last = {30, 20};
    *(regs + 0) = Value(0.000000);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    block0:;
    vm.last = {31, 20};
    *(regs + 0) = *(locals + 12); // det
    U_BCALLRET1(vm, regs + 1, 56, 1); // sqrt
    vm.temp_lval = locals + 12; // det
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {32, 20};
    *(regs + 0) = *(locals + 11); // b
    *(regs + 1) = *(locals + 12); // det
    U_FSUB(vm, regs + 2);
    vm.temp_lval = locals + 13; // t
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {37, 20};
    *(regs + 0) = *(locals + 13); // t
    *(regs + 1) = *(locals + 10); // eps
    U_FGT(vm, regs + 2);
    if (!(regs + 0)->True()) goto block1;
    vm.last = {34, 20};
    *(regs + 0) = *(locals + 13); // t
    goto block2;
    block1:;
    vm.last = {36, 20};
    *(regs + 0) = *(locals + 11); // b
    *(regs + 1) = *(locals + 12); // det
    U_FADD(vm, regs + 2);
    vm.temp_lval = locals + 13; // t
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {37, 20};
    *(regs + 0) = *(locals + 13); // t
    *(regs + 1) = *(locals + 10); // eps
    U_FGT(vm, regs + 2);
    if (!(regs + 0)->True()) goto block3;
    vm.last = {37, 20};
    *(regs + 0) = *(locals + 13); // t
    goto block4;
    block3:;
    vm.last = {37, 20};
    *(regs + 0) = Value(0.000000);
    block4:;
    block2:;
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// radiance
static void fun_372(VMRef vm, StackPtr psp) {
    Value regs[23];
    Value locals[68];
    *(locals + 0) = *(psp - 7);
    *(locals + 1) = *(psp - 6);
    *(locals + 2) = *(psp - 5);
    *(locals + 3) = *(psp - 4);
    *(locals + 4) = *(psp - 3);
    *(locals + 5) = *(psp - 2);
    *(locals + 6) = *(psp - 1);
    *(locals + 7) = Value(0, lobster::V_NIL);
    *(locals + 8) = Value(0, lobster::V_NIL);
    *(locals + 9) = Value(0, lobster::V_NIL);
    *(locals + 10) = Value(0, lobster::V_NIL);
    *(locals + 11) = Value(0, lobster::V_NIL);
    *(locals + 12) = Value(0, lobster::V_NIL);
    *(locals + 13) = Value(0, lobster::V_NIL);
    *(locals + 14) = Value(0, lobster::V_NIL);
    *(locals + 15) = Value(0, lobster::V_NIL);
    *(locals + 16) = Value(0, lobster::V_NIL);
    *(locals + 17) = Value(0, lobster::V_NIL);
    *(locals + 18) = Value(0, lobster::V_NIL);
    *(locals + 19) = Value(0, lobster::V_NIL);
    *(locals + 20) = Value(0, lobster::V_NIL);
    *(locals + 21) = Value(0, lobster::V_NIL);
    *(locals + 22) = Value(0, lobster::V_NIL);
    *(locals + 23) = Value(0, lobster::V_NIL);
    *(locals + 24) = Value(0, lobster::V_NIL);
    *(locals + 25) = Value(0, lobster::V_NIL);
    *(locals + 26) = Value(0, lobster::V_NIL);
    *(locals + 27) = Value(0, lobster::V_NIL);
    *(locals + 28) = Value(0, lobster::V_NIL);
    *(locals + 29) = Value(0, lobster::V_NIL);
    *(locals + 30) = Value(0, lobster::V_NIL);
    *(locals + 31) = Value(0, lobster::V_NIL);
    *(locals + 32) = Value(0, lobster::V_NIL);
    *(locals + 33) = Value(0, lobster::V_NIL);
    *(locals + 34) = Value(0, lobster::V_NIL);
    *(locals + 35) = Value(0, lobster::V_NIL);
    *(locals + 36) = Value(0, lobster::V_NIL);
    *(locals + 37) = Value(0, lobster::V_NIL);
    *(locals + 38) = Value(0, lobster::V_NIL);
    *(locals + 39) = Value(0, lobster::V_NIL);
    *(locals + 40) = Value(0, lobster::V_NIL);
    *(locals + 41) = Value(0, lobster::V_NIL);
    *(locals + 42) = Value(0, lobster::V_NIL);
    *(locals + 43) = Value(0, lobster::V_NIL);
    *(locals + 44) = Value(0, lobster::V_NIL);
    *(locals + 45) = Value(0, lobster::V_NIL);
    *(locals + 46) = Value(0, lobster::V_NIL);
    *(locals + 47) = Value(0, lobster::V_NIL);
    *(locals + 48) = Value(0, lobster::V_NIL);
    *(locals + 49) = Value(0, lobster::V_NIL);
    *(locals + 50) = Value(0, lobster::V_NIL);
    *(locals + 51) = Value(0, lobster::V_NIL);
    *(locals + 52) = Value(0, lobster::V_NIL);
    *(locals + 53) = Value(0, lobster::V_NIL);
    *(locals + 54) = Value(0, lobster::V_NIL);
    *(locals + 55) = Value(0, lobster::V_NIL);
    *(locals + 56) = Value(0, lobster::V_NIL);
    *(locals + 57) = Value(0, lobster::V_NIL);
    *(locals + 58) = Value(0, lobster::V_NIL);
    *(locals + 59) = Value(0, lobster::V_NIL);
    *(locals + 60) = Value(0, lobster::V_NIL);
    *(locals + 61) = Value(0, lobster::V_NIL);
    *(locals + 62) = Value(0, lobster::V_NIL);
    *(locals + 63) = Value(0, lobster::V_NIL);
    *(locals + 64) = Value(0, lobster::V_NIL);
    *(locals + 65) = Value(0, lobster::V_NIL);
    *(locals + 66) = Value(0, lobster::V_NIL);
    *(locals + 67) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 2487, locals);
    vm.last = {57, 20};
    U_PUSHFLT64(vm, regs + 0, -1577058304, 1114446484); // 1000000000000.0
    vm.temp_lval = locals + 7; // t
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {58, 20};
    *(regs + 0) = Value(-1);
    vm.temp_lval = locals + 8; // id
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {59, 20};
    *(regs + 0) = Value(-1);
    U_PUSHVARF(vm, regs + 1, 946); // spheres
    block0:;
    if (!U_VFOR(vm, regs + 2)) goto block1;
    vm.last = {59, 20};
    U_VFORELEMREF(vm, regs + 2);
    vm.temp_lval = locals + 9; // s
    U_LV_WRITEREF(vm, regs + 3);
    vm.last = {59, 20};
    U_FORLOOPI(vm, regs + 2);
    vm.temp_lval = locals + 10; // i
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {60, 20};
    *(regs + 2) = *(locals + 9); // s
    (regs + 2)->LTINCRTNIL();
    *(regs + 3) = *(locals + 0); // r.0
    *(regs + 4) = *(locals + 1); // r.1
    *(regs + 5) = *(locals + 2); // r.2
    *(regs + 6) = *(locals + 3); // r.3
    *(regs + 7) = *(locals + 4); // r.4
    *(regs + 8) = *(locals + 5); // r.5
    fun_371(vm, regs + 9); // call: intersect
    vm.temp_lval = locals + 11; // d
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {61, 20};
    *(regs + 2) = *(locals + 11); // d
    *(regs + 3) = Value(0.000000);
    U_FNE(vm, regs + 4);
    if (!(regs + 2)->True()) goto block2;
    U_POP(vm, regs + 3);
    *(regs + 2) = *(locals + 11); // d
    *(regs + 3) = *(locals + 7); // t
    U_FLT(vm, regs + 4);
    block2:;
    if (!(regs + 2)->True()) goto block3;
    vm.last = {62, 20};
    *(regs + 2) = *(locals + 11); // d
    vm.temp_lval = locals + 7; // t
    *(vm.temp_lval) = *(regs + 2);
    vm.last = {63, 20};
    *(regs + 2) = *(locals + 10); // i
    vm.temp_lval = locals + 8; // id
    *(vm.temp_lval) = *(regs + 2);
    block3:;
    goto block0;
    block1:;
    vm.last = {64, 20};
    *(regs + 0) = *(locals + 8); // id
    *(regs + 1) = Value(0);
    U_ILT(vm, regs + 2);
    if (!(regs + 0)->True()) goto block4;
    vm.last = {64, 20};
    U_PUSHVARVF(vm, regs + 0, 84, 3); // float3_0+0
    DecVal(vm, locals[9]);
    DecVal(vm, locals[12]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    *(psp++) = *(regs + 2);
    goto epilogue;
    block4:;
    vm.last = {65, 20};
    U_PUSHVARF(vm, regs + 0, 946); // spheres
    *(regs + 1) = *(locals + 8); // id
    U_VPUSHIDXI(vm, regs + 2);
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 12; // obj
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {66, 20};
    *(regs + 0) = *(locals + 0); // r.0
    *(regs + 1) = *(locals + 1); // r.1
    *(regs + 2) = *(locals + 2); // r.2
    *(regs + 3) = *(locals + 3); // r.0
    *(regs + 4) = *(locals + 4); // r.1
    *(regs + 5) = *(locals + 5); // r.2
    *(regs + 6) = *(locals + 7); // t
    U_FVSMUL(vm, regs + 7, 3);
    U_FVVADD(vm, regs + 6, 3);
    vm.temp_lval = locals + 13; // x+0
    U_LV_WRITEV(vm, regs + 3, 3);
    vm.last = {67, 20};
    *(regs + 0) = *(locals + 13); // x.0
    *(regs + 1) = *(locals + 14); // x.1
    *(regs + 2) = *(locals + 15); // x.2
    *(regs + 3) = *(locals + 12); // obj
    U_PUSHFLD2V(vm, regs + 4, 1, 3);
    U_FVVSUB(vm, regs + 6, 3);
    *(regs + 3) = Value(3);
    U_BCALLRETV(vm, regs + 4, 84, 1); // normalize
    vm.temp_lval = locals + 16; // n+0
    U_LV_WRITEV(vm, regs + 3, 3);
    vm.last = {68, 20};
    *(regs + 0) = *(locals + 16); // n.0
    *(regs + 1) = *(locals + 17); // n.1
    *(regs + 2) = *(locals + 18); // n.2
    *(regs + 3) = Value(3);
    *(regs + 4) = *(locals + 3); // r.0
    *(regs + 5) = *(locals + 4); // r.1
    *(regs + 6) = *(locals + 5); // r.2
    *(regs + 7) = Value(3);
    U_BCALLRETV(vm, regs + 8, 85, 1); // dot
    *(regs + 1) = Value(0.000000);
    U_FLT(vm, regs + 2);
    if (!(regs + 0)->True()) goto block5;
    vm.last = {68, 20};
    *(regs + 0) = *(locals + 16); // n.0
    *(regs + 1) = *(locals + 17); // n.1
    *(regs + 2) = *(locals + 18); // n.2
    goto block6;
    block5:;
    vm.last = {68, 20};
    *(regs + 0) = *(locals + 16); // n.0
    *(regs + 1) = *(locals + 17); // n.1
    *(regs + 2) = *(locals + 18); // n.2
    *(regs + 3) = Value(-1.000000);
    U_FVSMUL(vm, regs + 4, 3);
    block6:;
    vm.temp_lval = locals + 19; // nl+0
    U_LV_WRITEV(vm, regs + 3, 3);
    vm.last = {69, 20};
    *(regs + 0) = *(locals + 12); // obj
    U_PUSHFLD2V(vm, regs + 1, 7, 3);
    vm.temp_lval = locals + 22; // f+0
    U_LV_WRITEV(vm, regs + 3, 3);
    vm.last = {70, 20};
    *(regs + 0) = *(locals + 22); // f+0
    *(regs + 1) = *(locals + 23); // f+1
    U_FGT(vm, regs + 2);
    if (!(regs + 0)->True()) goto block7;
    U_POP(vm, regs + 1);
    *(regs + 0) = *(locals + 22); // f+0
    *(regs + 1) = *(locals + 24); // f+2
    U_FGT(vm, regs + 2);
    block7:;
    if (!(regs + 0)->True()) goto block8;
    vm.last = {70, 20};
    *(regs + 0) = *(locals + 22); // f+0
    goto block9;
    block8:;
    vm.last = {70, 20};
    *(regs + 0) = *(locals + 23); // f+1
    *(regs + 1) = *(locals + 24); // f+2
    U_FGT(vm, regs + 2);
    if (!(regs + 0)->True()) goto block10;
    vm.last = {70, 20};
    *(regs + 0) = *(locals + 23); // f+1
    goto block11;
    block10:;
    vm.last = {70, 20};
    *(regs + 0) = *(locals + 24); // f+2
    block11:;
    block9:;
    vm.temp_lval = locals + 25; // p
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {71, 20};
    vm.temp_lval = locals + 6; // depth
    U_LV_IPP(vm, regs + 0);
    U_LV_DUP(vm, regs + 0);
    U_I2F(vm, regs + 1);
    *(regs + 1) = Value(5.000000);
    U_FGT(vm, regs + 2);
    if (!(regs + 0)->True()) goto block12;
    vm.last = {71, 20};
    U_BCALLRET0(vm, regs + 0, 95, 1); // rnd_float
    *(regs + 1) = *(locals + 25); // p
    U_FLT(vm, regs + 2);
    if (!(regs + 0)->True()) goto block13;
    vm.last = {71, 20};
    *(regs + 0) = *(locals + 22); // f.0
    *(regs + 1) = *(locals + 23); // f.1
    *(regs + 2) = *(locals + 24); // f.2
    *(regs + 3) = Value(1.000000);
    *(regs + 4) = *(locals + 25); // p
    U_FDIV(vm, regs + 5);
    U_FVSMUL(vm, regs + 4, 3);
    vm.temp_lval = locals + 22; // f+0
    U_LV_WRITEV(vm, regs + 3, 3);
    goto block14;
    block13:;
    vm.last = {71, 20};
    *(regs + 0) = *(locals + 12); // obj
    U_PUSHFLD2V(vm, regs + 1, 4, 3);
    DecVal(vm, locals[9]);
    DecVal(vm, locals[12]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    *(psp++) = *(regs + 2);
    goto epilogue;
    block14:;
    block12:;
    vm.last = {72, 20};
    *(regs + 0) = *(locals + 12); // obj
    U_PUSHFLD(vm, regs + 1, 10);
    *(regs + 1) = Value(0);
    U_IEQ(vm, regs + 2);
    if (!(regs + 0)->True()) goto block15;
    vm.last = {73, 20};
    *(regs + 0) = Value(360.000000);
    U_BCALLRET0(vm, regs + 1, 95, 1); // rnd_float
    U_FMUL(vm, regs + 2);
    vm.temp_lval = locals + 26; // r1
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {74, 20};
    U_BCALLRET0(vm, regs + 0, 95, 1); // rnd_float
    vm.temp_lval = locals + 27; // r2
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {75, 20};
    *(regs + 0) = *(locals + 27); // r2
    U_BCALLRET1(vm, regs + 1, 56, 1); // sqrt
    vm.temp_lval = locals + 28; // r2s
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {76, 20};
    *(regs + 0) = *(locals + 19); // nl.0
    *(regs + 1) = *(locals + 20); // nl.1
    *(regs + 2) = *(locals + 21); // nl.2
    vm.temp_lval = locals + 29; // w+0
    U_LV_WRITEV(vm, regs + 3, 3);
    vm.last = {77, 20};
    *(regs + 0) = *(locals + 29); // w+0
    U_BCALLRET1(vm, regs + 1, 113, 1); // abs
    U_PUSHFLT64(vm, regs + 1, -1717986918, 1069128089); // 0.1
    U_FGT(vm, regs + 2);
    if (!(regs + 0)->True()) goto block16;
    vm.last = {77, 20};
    U_PUSHVARVF(vm, regs + 0, 93, 3); // float3_y+0
    goto block17;
    block16:;
    vm.last = {77, 20};
    U_PUSHVARVF(vm, regs + 0, 90, 3); // float3_x+0
    block17:;
    *(regs + 3) = Value(3);
    *(regs + 4) = *(locals + 29); // w.0
    *(regs + 5) = *(locals + 30); // w.1
    *(regs + 6) = *(locals + 31); // w.2
    *(regs + 7) = Value(3);
    U_BCALLRETV(vm, regs + 8, 90, 1); // cross
    *(regs + 3) = Value(3);
    U_BCALLRETV(vm, regs + 4, 84, 1); // normalize
    vm.temp_lval = locals + 32; // u+0
    U_LV_WRITEV(vm, regs + 3, 3);
    vm.last = {78, 20};
    *(regs + 0) = *(locals + 29); // w.0
    *(regs + 1) = *(locals + 30); // w.1
    *(regs + 2) = *(locals + 31); // w.2
    *(regs + 3) = Value(3);
    *(regs + 4) = *(locals + 32); // u.0
    *(regs + 5) = *(locals + 33); // u.1
    *(regs + 6) = *(locals + 34); // u.2
    *(regs + 7) = Value(3);
    U_BCALLRETV(vm, regs + 8, 90, 1); // cross
    vm.temp_lval = locals + 35; // v+0
    U_LV_WRITEV(vm, regs + 3, 3);
    vm.last = {79, 20};
    *(regs + 0) = *(locals + 32); // u.0
    *(regs + 1) = *(locals + 33); // u.1
    *(regs + 2) = *(locals + 34); // u.2
    *(regs + 3) = *(locals + 26); // r1
    U_BCALLRET1(vm, regs + 4, 71, 1); // cos
    U_FVSMUL(vm, regs + 4, 3);
    *(regs + 3) = *(locals + 28); // r2s
    U_FVSMUL(vm, regs + 4, 3);
    *(regs + 3) = *(locals + 35); // v.0
    *(regs + 4) = *(locals + 36); // v.1
    *(regs + 5) = *(locals + 37); // v.2
    *(regs + 6) = *(locals + 26); // r1
    U_BCALLRET1(vm, regs + 7, 69, 1); // sin
    U_FVSMUL(vm, regs + 7, 3);
    *(regs + 6) = *(locals + 28); // r2s
    U_FVSMUL(vm, regs + 7, 3);
    U_FVVADD(vm, regs + 6, 3);
    *(regs + 3) = *(locals + 29); // w.0
    *(regs + 4) = *(locals + 30); // w.1
    *(regs + 5) = *(locals + 31); // w.2
    *(regs + 6) = Value(1.000000);
    *(regs + 7) = *(locals + 27); // r2
    U_FSUB(vm, regs + 8);
    U_BCALLRET1(vm, regs + 7, 56, 1); // sqrt
    U_FVSMUL(vm, regs + 7, 3);
    U_FVVADD(vm, regs + 6, 3);
    *(regs + 3) = Value(3);
    U_BCALLRETV(vm, regs + 4, 84, 1); // normalize
    vm.temp_lval = locals + 38; // d+0
    U_LV_WRITEV(vm, regs + 3, 3);
    vm.last = {80, 20};
    *(regs + 0) = *(locals + 12); // obj
    U_PUSHFLD2V(vm, regs + 1, 4, 3);
    *(regs + 3) = *(locals + 22); // f.0
    *(regs + 4) = *(locals + 23); // f.1
    *(regs + 5) = *(locals + 24); // f.2
    *(regs + 6) = *(locals + 13); // x.0
    *(regs + 7) = *(locals + 14); // x.1
    *(regs + 8) = *(locals + 15); // x.2
    *(regs + 9) = *(locals + 38); // d.0
    *(regs + 10) = *(locals + 39); // d.1
    *(regs + 11) = *(locals + 40); // d.2
    *(regs + 12) = *(locals + 6); // depth
    fun_372(vm, regs + 13); // call: radiance
    U_FVVMUL(vm, regs + 9, 3);
    U_FVVADD(vm, regs + 6, 3);
    DecVal(vm, locals[9]);
    DecVal(vm, locals[12]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    *(psp++) = *(regs + 2);
    goto epilogue;
    goto block18;
    block15:;
    vm.last = {81, 20};
    *(regs + 0) = *(locals + 12); // obj
    U_PUSHFLD(vm, regs + 1, 10);
    *(regs + 1) = Value(1);
    U_IEQ(vm, regs + 2);
    if (!(regs + 0)->True()) goto block19;
    vm.last = {82, 20};
    *(regs + 0) = *(locals + 12); // obj
    U_PUSHFLD2V(vm, regs + 1, 4, 3);
    *(regs + 3) = *(locals + 22); // f.0
    *(regs + 4) = *(locals + 23); // f.1
    *(regs + 5) = *(locals + 24); // f.2
    *(regs + 6) = *(locals + 13); // x.0
    *(regs + 7) = *(locals + 14); // x.1
    *(regs + 8) = *(locals + 15); // x.2
    *(regs + 9) = *(locals + 3); // r.0
    *(regs + 10) = *(locals + 4); // r.1
    *(regs + 11) = *(locals + 5); // r.2
    *(regs + 12) = *(locals + 16); // n.0
    *(regs + 13) = *(locals + 17); // n.1
    *(regs + 14) = *(locals + 18); // n.2
    *(regs + 15) = Value(2.000000);
    U_FVSMUL(vm, regs + 16, 3);
    *(regs + 15) = *(locals + 16); // n.0
    *(regs + 16) = *(locals + 17); // n.1
    *(regs + 17) = *(locals + 18); // n.2
    *(regs + 18) = Value(3);
    *(regs + 19) = *(locals + 3); // r.0
    *(regs + 20) = *(locals + 4); // r.1
    *(regs + 21) = *(locals + 5); // r.2
    *(regs + 22) = Value(3);
    U_BCALLRETV(vm, regs + 23, 85, 1); // dot
    U_FVSMUL(vm, regs + 16, 3);
    U_FVVSUB(vm, regs + 15, 3);
    *(regs + 12) = *(locals + 6); // depth
    fun_372(vm, regs + 13); // call: radiance
    U_FVVMUL(vm, regs + 9, 3);
    U_FVVADD(vm, regs + 6, 3);
    DecVal(vm, locals[9]);
    DecVal(vm, locals[12]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    *(psp++) = *(regs + 2);
    goto epilogue;
    block19:;
    block18:;
    vm.last = {83, 20};
    *(regs + 0) = *(locals + 13); // x.0
    *(regs + 1) = *(locals + 14); // x.1
    *(regs + 2) = *(locals + 15); // x.2
    *(regs + 3) = *(locals + 3); // r.0
    *(regs + 4) = *(locals + 4); // r.1
    *(regs + 5) = *(locals + 5); // r.2
    *(regs + 6) = *(locals + 16); // n.0
    *(regs + 7) = *(locals + 17); // n.1
    *(regs + 8) = *(locals + 18); // n.2
    *(regs + 9) = Value(2.000000);
    U_FVSMUL(vm, regs + 10, 3);
    *(regs + 9) = *(locals + 16); // n.0
    *(regs + 10) = *(locals + 17); // n.1
    *(regs + 11) = *(locals + 18); // n.2
    *(regs + 12) = Value(3);
    *(regs + 13) = *(locals + 3); // r.0
    *(regs + 14) = *(locals + 4); // r.1
    *(regs + 15) = *(locals + 5); // r.2
    *(regs + 16) = Value(3);
    U_BCALLRETV(vm, regs + 17, 85, 1); // dot
    U_FVSMUL(vm, regs + 10, 3);
    U_FVVSUB(vm, regs + 9, 3);
    vm.temp_lval = locals + 41; // reflRay+0
    U_LV_WRITEV(vm, regs + 6, 6);
    vm.last = {84, 20};
    *(regs + 0) = *(locals + 16); // n.0
    *(regs + 1) = *(locals + 17); // n.1
    *(regs + 2) = *(locals + 18); // n.2
    *(regs + 3) = Value(3);
    *(regs + 4) = *(locals + 19); // nl.0
    *(regs + 5) = *(locals + 20); // nl.1
    *(regs + 6) = *(locals + 21); // nl.2
    *(regs + 7) = Value(3);
    U_BCALLRETV(vm, regs + 8, 85, 1); // dot
    *(regs + 1) = Value(0.000000);
    U_FGT(vm, regs + 2);
    vm.temp_lval = locals + 47; // into
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {85, 20};
    *(regs + 0) = Value(1.000000);
    vm.temp_lval = locals + 48; // nc
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {86, 20};
    *(regs + 0) = Value(1.500000);
    vm.temp_lval = locals + 49; // nt
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {87, 20};
    *(regs + 0) = *(locals + 47); // into
    if (!(regs + 0)->True()) goto block20;
    vm.last = {87, 20};
    *(regs + 0) = *(locals + 48); // nc
    *(regs + 1) = *(locals + 49); // nt
    U_FDIV(vm, regs + 2);
    goto block21;
    block20:;
    vm.last = {87, 20};
    *(regs + 0) = *(locals + 49); // nt
    *(regs + 1) = *(locals + 48); // nc
    U_FDIV(vm, regs + 2);
    block21:;
    vm.temp_lval = locals + 50; // nnt
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {88, 20};
    *(regs + 0) = *(locals + 3); // r.0
    *(regs + 1) = *(locals + 4); // r.1
    *(regs + 2) = *(locals + 5); // r.2
    *(regs + 3) = Value(3);
    *(regs + 4) = *(locals + 19); // nl.0
    *(regs + 5) = *(locals + 20); // nl.1
    *(regs + 6) = *(locals + 21); // nl.2
    *(regs + 7) = Value(3);
    U_BCALLRETV(vm, regs + 8, 85, 1); // dot
    vm.temp_lval = locals + 51; // ddn
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {89, 20};
    *(regs + 0) = Value(1.000000);
    *(regs + 1) = *(locals + 50); // nnt
    *(regs + 2) = *(locals + 50); // nnt
    U_FMUL(vm, regs + 3);
    *(regs + 2) = Value(1.000000);
    *(regs + 3) = *(locals + 51); // ddn
    *(regs + 4) = *(locals + 51); // ddn
    U_FMUL(vm, regs + 5);
    U_FSUB(vm, regs + 4);
    U_FMUL(vm, regs + 3);
    U_FSUB(vm, regs + 2);
    vm.temp_lval = locals + 52; // cos2t
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {90, 20};
    *(regs + 0) = *(locals + 52); // cos2t
    *(regs + 1) = Value(0.000000);
    U_FLT(vm, regs + 2);
    if (!(regs + 0)->True()) goto block22;
    vm.last = {91, 20};
    *(regs + 0) = *(locals + 12); // obj
    U_PUSHFLD2V(vm, regs + 1, 4, 3);
    *(regs + 3) = *(locals + 22); // f.0
    *(regs + 4) = *(locals + 23); // f.1
    *(regs + 5) = *(locals + 24); // f.2
    *(regs + 6) = *(locals + 41); // reflRay.0
    *(regs + 7) = *(locals + 42); // reflRay.1
    *(regs + 8) = *(locals + 43); // reflRay.2
    *(regs + 9) = *(locals + 44); // reflRay.3
    *(regs + 10) = *(locals + 45); // reflRay.4
    *(regs + 11) = *(locals + 46); // reflRay.5
    *(regs + 12) = *(locals + 6); // depth
    fun_372(vm, regs + 13); // call: radiance
    U_FVVMUL(vm, regs + 9, 3);
    U_FVVADD(vm, regs + 6, 3);
    DecVal(vm, locals[9]);
    DecVal(vm, locals[12]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    *(psp++) = *(regs + 2);
    goto epilogue;
    block22:;
    vm.last = {92, 20};
    *(regs + 0) = *(locals + 3); // r.0
    *(regs + 1) = *(locals + 4); // r.1
    *(regs + 2) = *(locals + 5); // r.2
    *(regs + 3) = *(locals + 50); // nnt
    U_FVSMUL(vm, regs + 4, 3);
    *(regs + 3) = *(locals + 16); // n.0
    *(regs + 4) = *(locals + 17); // n.1
    *(regs + 5) = *(locals + 18); // n.2
    *(regs + 6) = *(locals + 47); // into
    if (!(regs + 6)->True()) goto block23;
    vm.last = {92, 20};
    *(regs + 6) = Value(1);
    goto block24;
    block23:;
    vm.last = {92, 20};
    *(regs + 6) = Value(-1);
    block24:;
    U_I2F(vm, regs + 7);
    *(regs + 7) = *(locals + 51); // ddn
    *(regs + 8) = *(locals + 50); // nnt
    U_FMUL(vm, regs + 9);
    *(regs + 8) = *(locals + 52); // cos2t
    U_BCALLRET1(vm, regs + 9, 56, 1); // sqrt
    U_FADD(vm, regs + 9);
    U_FMUL(vm, regs + 8);
    U_FVSMUL(vm, regs + 7, 3);
    U_FVVSUB(vm, regs + 6, 3);
    *(regs + 3) = Value(3);
    U_BCALLRETV(vm, regs + 4, 84, 1); // normalize
    vm.temp_lval = locals + 53; // tdir+0
    U_LV_WRITEV(vm, regs + 3, 3);
    vm.last = {93, 20};
    *(regs + 0) = *(locals + 49); // nt
    *(regs + 1) = *(locals + 48); // nc
    U_FSUB(vm, regs + 2);
    vm.temp_lval = locals + 56; // a
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {94, 20};
    *(regs + 0) = *(locals + 49); // nt
    *(regs + 1) = *(locals + 48); // nc
    U_FADD(vm, regs + 2);
    vm.temp_lval = locals + 57; // b
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {95, 20};
    *(regs + 0) = *(locals + 56); // a
    *(regs + 1) = *(locals + 56); // a
    U_FMUL(vm, regs + 2);
    *(regs + 1) = *(locals + 57); // b
    *(regs + 2) = *(locals + 57); // b
    U_FMUL(vm, regs + 3);
    U_FDIV(vm, regs + 2);
    vm.temp_lval = locals + 58; // R0
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {96, 20};
    *(regs + 0) = Value(1.000000);
    *(regs + 1) = *(locals + 47); // into
    if (!(regs + 1)->True()) goto block25;
    vm.last = {96, 20};
    *(regs + 1) = *(locals + 51); // ddn
    U_FUMINUS(vm, regs + 2);
    goto block26;
    block25:;
    vm.last = {96, 20};
    *(regs + 1) = *(locals + 53); // tdir.0
    *(regs + 2) = *(locals + 54); // tdir.1
    *(regs + 3) = *(locals + 55); // tdir.2
    *(regs + 4) = Value(3);
    *(regs + 5) = *(locals + 16); // n.0
    *(regs + 6) = *(locals + 17); // n.1
    *(regs + 7) = *(locals + 18); // n.2
    *(regs + 8) = Value(3);
    U_BCALLRETV(vm, regs + 9, 85, 1); // dot
    block26:;
    U_FSUB(vm, regs + 2);
    vm.temp_lval = locals + 59; // c
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {97, 20};
    *(regs + 0) = *(locals + 58); // R0
    *(regs + 1) = Value(1.000000);
    *(regs + 2) = *(locals + 58); // R0
    U_FSUB(vm, regs + 3);
    *(regs + 2) = *(locals + 59); // c
    U_FMUL(vm, regs + 3);
    *(regs + 2) = *(locals + 59); // c
    U_FMUL(vm, regs + 3);
    *(regs + 2) = *(locals + 59); // c
    U_FMUL(vm, regs + 3);
    *(regs + 2) = *(locals + 59); // c
    U_FMUL(vm, regs + 3);
    *(regs + 2) = *(locals + 59); // c
    U_FMUL(vm, regs + 3);
    U_FADD(vm, regs + 2);
    vm.temp_lval = locals + 60; // Re
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {98, 20};
    *(regs + 0) = Value(1.000000);
    *(regs + 1) = *(locals + 60); // Re
    U_FSUB(vm, regs + 2);
    vm.temp_lval = locals + 61; // Tr
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {99, 20};
    *(regs + 0) = Value(0.250000);
    *(regs + 1) = Value(0.500000);
    *(regs + 2) = *(locals + 60); // Re
    U_FMUL(vm, regs + 3);
    U_FADD(vm, regs + 2);
    vm.temp_lval = locals + 62; // P
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {100, 20};
    *(regs + 0) = *(locals + 60); // Re
    *(regs + 1) = *(locals + 62); // P
    U_FDIV(vm, regs + 2);
    vm.temp_lval = locals + 63; // RP
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {101, 20};
    *(regs + 0) = *(locals + 61); // Tr
    *(regs + 1) = Value(1.000000);
    *(regs + 2) = *(locals + 62); // P
    U_FSUB(vm, regs + 3);
    U_FDIV(vm, regs + 2);
    vm.temp_lval = locals + 64; // TP
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {102, 20};
    *(regs + 0) = *(locals + 6); // depth
    *(regs + 1) = Value(2);
    U_IGT(vm, regs + 2);
    if (!(regs + 0)->True()) goto block27;
    vm.last = {104, 20};
    U_BCALLRET0(vm, regs + 0, 95, 1); // rnd_float
    *(regs + 1) = *(locals + 62); // P
    U_FLT(vm, regs + 2);
    if (!(regs + 0)->True()) goto block28;
    vm.last = {105, 20};
    *(regs + 0) = *(locals + 41); // reflRay.0
    *(regs + 1) = *(locals + 42); // reflRay.1
    *(regs + 2) = *(locals + 43); // reflRay.2
    *(regs + 3) = *(locals + 44); // reflRay.3
    *(regs + 4) = *(locals + 45); // reflRay.4
    *(regs + 5) = *(locals + 46); // reflRay.5
    *(regs + 6) = *(locals + 6); // depth
    fun_372(vm, regs + 7); // call: radiance
    *(regs + 3) = *(locals + 63); // RP
    U_FVSMUL(vm, regs + 4, 3);
    goto block29;
    block28:;
    vm.last = {107, 20};
    *(regs + 0) = *(locals + 13); // x.0
    *(regs + 1) = *(locals + 14); // x.1
    *(regs + 2) = *(locals + 15); // x.2
    *(regs + 3) = *(locals + 53); // tdir.0
    *(regs + 4) = *(locals + 54); // tdir.1
    *(regs + 5) = *(locals + 55); // tdir.2
    *(regs + 6) = *(locals + 6); // depth
    fun_372(vm, regs + 7); // call: radiance
    *(regs + 3) = *(locals + 64); // TP
    U_FVSMUL(vm, regs + 4, 3);
    block29:;
    goto block30;
    block27:;
    vm.last = {109, 20};
    *(regs + 0) = *(locals + 41); // reflRay.0
    *(regs + 1) = *(locals + 42); // reflRay.1
    *(regs + 2) = *(locals + 43); // reflRay.2
    *(regs + 3) = *(locals + 44); // reflRay.3
    *(regs + 4) = *(locals + 45); // reflRay.4
    *(regs + 5) = *(locals + 46); // reflRay.5
    *(regs + 6) = *(locals + 6); // depth
    fun_372(vm, regs + 7); // call: radiance
    *(regs + 3) = *(locals + 60); // Re
    U_FVSMUL(vm, regs + 4, 3);
    *(regs + 3) = *(locals + 13); // x.0
    *(regs + 4) = *(locals + 14); // x.1
    *(regs + 5) = *(locals + 15); // x.2
    *(regs + 6) = *(locals + 53); // tdir.0
    *(regs + 7) = *(locals + 54); // tdir.1
    *(regs + 8) = *(locals + 55); // tdir.2
    *(regs + 9) = *(locals + 6); // depth
    fun_372(vm, regs + 10); // call: radiance
    *(regs + 6) = *(locals + 61); // Tr
    U_FVSMUL(vm, regs + 7, 3);
    U_FVVADD(vm, regs + 6, 3);
    block30:;
    vm.temp_lval = locals + 65; // temp+0
    U_LV_WRITEV(vm, regs + 3, 3);
    vm.last = {110, 20};
    *(regs + 0) = *(locals + 12); // obj
    U_PUSHFLD2V(vm, regs + 1, 4, 3);
    *(regs + 3) = *(locals + 22); // f.0
    *(regs + 4) = *(locals + 23); // f.1
    *(regs + 5) = *(locals + 24); // f.2
    *(regs + 6) = *(locals + 65); // temp.0
    *(regs + 7) = *(locals + 66); // temp.1
    *(regs + 8) = *(locals + 67); // temp.2
    U_FVVMUL(vm, regs + 9, 3);
    U_FVVADD(vm, regs + 6, 3);
    DecVal(vm, locals[9]);
    DecVal(vm, locals[12]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    *(psp++) = *(regs + 2);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function340
static void fun_373(VMRef vm, StackPtr psp) {
    Value regs[2];
    PushFunId(vm, funinfo_table + 2558, 0);
    vm.last = {119, 20};
    U_PUSHVARF(vm, regs + 0, 1015); // w
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_602(vm, regs + 2); // call: map
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function342
static void fun_375(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2561, locals);
    vm.last = {138, 20};
    *(regs + 0) = *(locals + 0); // row
    U_PUSHFUN(vm, regs + 1, 0, fun_376);
    fun_604(vm, regs + 2); // call: map
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function343
static void fun_376(VMRef vm, StackPtr psp) {
    Value regs[12];
    Value locals[3];
    *(locals + 0) = *(psp - 3);
    *(locals + 1) = *(psp - 2);
    *(locals + 2) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2565, locals);
    vm.last = {138, 20};
    *(regs + 0) = *(locals + 0); // col.0
    *(regs + 1) = *(locals + 1); // col.1
    *(regs + 2) = *(locals + 2); // col.2
    U_PUSHVARF(vm, regs + 3, 1030); // samples
    U_I2F(vm, regs + 4);
    U_FVSDIV(vm, regs + 4, 3);
    *(regs + 3) = Value(3);
    U_PUSHVARVF(vm, regs + 4, 84, 3); // float3_0+0
    *(regs + 7) = Value(3);
    U_PUSHVARVF(vm, regs + 8, 87, 3); // float3_1+0
    *(regs + 11) = Value(3);
    U_BCALLRETV(vm, regs + 12, 105, 1); // clamp
    *(regs + 3) = Value(3);
    U_PUSHFLT64(vm, regs + 4, -780903145, 1071454021); // 0.45454545454545453
    U_BCALLRETV(vm, regs + 5, 53, 1); // pow
    *(regs + 3) = Value(1.000000);
    fun_79(vm, regs + 4); // call: vec4
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    *(psp++) = *(regs + 2);
    *(psp++) = *(regs + 3);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function344
static void fun_377(VMRef vm, StackPtr psp) {
    Value regs[6];
    Value keepvar[32];
    Value locals[2];
    *(locals + 0) = Value(0, lobster::V_NIL);
    *(locals + 1) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 2571, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    *(keepvar + 1) = Value(0, lobster::V_NIL);
    *(keepvar + 2) = Value(0, lobster::V_NIL);
    *(keepvar + 3) = Value(0, lobster::V_NIL);
    *(keepvar + 4) = Value(0, lobster::V_NIL);
    *(keepvar + 5) = Value(0, lobster::V_NIL);
    *(keepvar + 6) = Value(0, lobster::V_NIL);
    *(keepvar + 7) = Value(0, lobster::V_NIL);
    *(keepvar + 8) = Value(0, lobster::V_NIL);
    *(keepvar + 9) = Value(0, lobster::V_NIL);
    *(keepvar + 10) = Value(0, lobster::V_NIL);
    *(keepvar + 11) = Value(0, lobster::V_NIL);
    *(keepvar + 12) = Value(0, lobster::V_NIL);
    *(keepvar + 13) = Value(0, lobster::V_NIL);
    *(keepvar + 14) = Value(0, lobster::V_NIL);
    *(keepvar + 15) = Value(0, lobster::V_NIL);
    *(keepvar + 16) = Value(0, lobster::V_NIL);
    *(keepvar + 17) = Value(0, lobster::V_NIL);
    *(keepvar + 18) = Value(0, lobster::V_NIL);
    *(keepvar + 19) = Value(0, lobster::V_NIL);
    *(keepvar + 20) = Value(0, lobster::V_NIL);
    *(keepvar + 21) = Value(0, lobster::V_NIL);
    *(keepvar + 22) = Value(0, lobster::V_NIL);
    *(keepvar + 23) = Value(0, lobster::V_NIL);
    *(keepvar + 24) = Value(0, lobster::V_NIL);
    *(keepvar + 25) = Value(0, lobster::V_NIL);
    *(keepvar + 26) = Value(0, lobster::V_NIL);
    *(keepvar + 27) = Value(0, lobster::V_NIL);
    *(keepvar + 28) = Value(0, lobster::V_NIL);
    *(keepvar + 29) = Value(0, lobster::V_NIL);
    *(keepvar + 30) = Value(0, lobster::V_NIL);
    *(keepvar + 31) = Value(0, lobster::V_NIL);
    vm.last = {4, 22};
    *(regs + 0) = Value(42);
    vm.temp_lval = locals + 0; // a
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {5, 22};
    U_PUSHFLT64(vm, regs + 0, 1374389535, 1074339512); // 3.14
    U_NEWVEC(vm, regs + 1, 9, 1);
    vm.temp_lval = locals + 1; // b
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {8, 22};
    U_PUSHSTR(vm, regs + 0, 539); // "a = "
    *(regs + 1) = *(locals + 0); // a
    U_A2S(vm, regs + 2, 0);
    *(keepvar + 0) = *(regs + 1);
    U_PUSHSTR(vm, regs + 2, 540); // "!"
    U_SADDN(vm, regs + 3, 3);
    *(keepvar + 1) = *(regs + 0);
    U_PUSHSTR(vm, regs + 1, 541); // "a = "
    *(regs + 2) = *(locals + 0); // a
    U_A2S(vm, regs + 3, 0);
    *(keepvar + 2) = *(regs + 2);
    U_PUSHSTR(vm, regs + 3, 542); // "!"
    U_SADDN(vm, regs + 4, 3);
    *(keepvar + 3) = *(regs + 1);
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 9, 22, 543);
    vm.last = {10, 22};
    U_PUSHSTR(vm, regs + 0, 544); // "a = "
    *(regs + 1) = *(locals + 0); // a
    U_A2S(vm, regs + 2, 0);
    *(keepvar + 4) = *(regs + 1);
    U_PUSHSTR(vm, regs + 2, 545); // " and b = "
    *(regs + 3) = *(locals + 1); // b
    U_A2S(vm, regs + 4, 9);
    *(keepvar + 5) = *(regs + 3);
    U_PUSHSTR(vm, regs + 4, 546); // "!"
    U_SADDN(vm, regs + 5, 5);
    *(keepvar + 6) = *(regs + 0);
    U_PUSHSTR(vm, regs + 1, 547); // "a = "
    *(regs + 2) = *(locals + 0); // a
    U_A2S(vm, regs + 3, 0);
    *(keepvar + 7) = *(regs + 2);
    U_PUSHSTR(vm, regs + 3, 548); // " and b = "
    *(regs + 4) = *(locals + 1); // b
    U_A2S(vm, regs + 5, 9);
    *(keepvar + 8) = *(regs + 4);
    U_PUSHSTR(vm, regs + 5, 549); // "!"
    U_SADDN(vm, regs + 6, 5);
    *(keepvar + 9) = *(regs + 1);
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 11, 22, 550);
    vm.last = {12, 22};
    *(regs + 0) = Value(3);
    *(regs + 1) = *(locals + 0); // a
    U_IADD(vm, regs + 2);
    U_A2S(vm, regs + 1, 0);
    *(keepvar + 10) = *(regs + 0);
    *(regs + 1) = Value(3);
    *(regs + 2) = *(locals + 0); // a
    U_IADD(vm, regs + 3);
    U_A2S(vm, regs + 2, 0);
    *(keepvar + 11) = *(regs + 1);
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 13, 22, 551);
    vm.last = {14, 22};
    *(regs + 0) = *(locals + 0); // a
    U_A2S(vm, regs + 1, 0);
    *(keepvar + 12) = *(regs + 0);
    *(regs + 1) = *(locals + 1); // b
    U_A2S(vm, regs + 2, 9);
    *(keepvar + 13) = *(regs + 1);
    U_SADD(vm, regs + 2);
    *(keepvar + 14) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // a
    U_A2S(vm, regs + 2, 0);
    *(keepvar + 15) = *(regs + 1);
    *(regs + 2) = *(locals + 1); // b
    U_A2S(vm, regs + 3, 9);
    *(keepvar + 16) = *(regs + 2);
    U_SADD(vm, regs + 3);
    *(keepvar + 17) = *(regs + 1);
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 15, 22, 552);
    vm.last = {16, 22};
    U_PUSHSTR(vm, regs + 0, 553); // "{ "
    *(regs + 1) = *(locals + 0); // a
    U_A2S(vm, regs + 2, 0);
    *(keepvar + 18) = *(regs + 1);
    U_PUSHSTR(vm, regs + 2, 554); // " }"
    U_SADDN(vm, regs + 3, 3);
    *(keepvar + 19) = *(regs + 0);
    U_PUSHSTR(vm, regs + 1, 555); // "{ "
    *(regs + 2) = *(locals + 0); // a
    U_A2S(vm, regs + 3, 0);
    *(keepvar + 20) = *(regs + 2);
    U_PUSHSTR(vm, regs + 3, 556); // " }"
    U_SADDN(vm, regs + 4, 3);
    *(keepvar + 21) = *(regs + 1);
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 17, 22, 557);
    vm.last = {18, 22};
    U_PUSHSTR(vm, regs + 0, 558); // "{ "
    *(regs + 1) = *(locals + 0); // a
    U_A2S(vm, regs + 2, 0);
    *(keepvar + 22) = *(regs + 1);
    U_PUSHSTR(vm, regs + 2, 559); // " }"
    U_SADDN(vm, regs + 3, 3);
    *(keepvar + 23) = *(regs + 0);
    U_PUSHSTR(vm, regs + 1, 560); // "{ "
    *(regs + 2) = *(locals + 0); // a
    U_A2S(vm, regs + 3, 0);
    *(keepvar + 24) = *(regs + 2);
    U_PUSHSTR(vm, regs + 3, 561); // " }"
    U_SADDN(vm, regs + 4, 3);
    *(keepvar + 25) = *(regs + 1);
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 19, 22, 562);
    vm.last = {20, 22};
    U_PUSHSTR(vm, regs + 0, 563); // "{"
    *(regs + 1) = Value(3);
    *(regs + 2) = *(locals + 0); // a
    U_IADD(vm, regs + 3);
    U_A2S(vm, regs + 2, 0);
    *(keepvar + 26) = *(regs + 1);
    U_PUSHSTR(vm, regs + 2, 564); // "}"
    U_SADDN(vm, regs + 3, 3);
    *(keepvar + 27) = *(regs + 0);
    U_PUSHSTR(vm, regs + 1, 565); // "{"
    *(regs + 2) = Value(3);
    *(regs + 3) = *(locals + 0); // a
    U_IADD(vm, regs + 4);
    U_A2S(vm, regs + 3, 0);
    *(keepvar + 28) = *(regs + 2);
    U_PUSHSTR(vm, regs + 3, 566); // "}"
    U_SADDN(vm, regs + 4, 3);
    *(keepvar + 29) = *(regs + 1);
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 21, 22, 567);
    vm.last = {22, 22};
    *(regs + 0) = *(locals + 0); // a
    U_A2S(vm, regs + 1, 0);
    *(keepvar + 30) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // a
    U_A2S(vm, regs + 2, 0);
    *(keepvar + 31) = *(regs + 1);
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 24, 22, 568);
    DecVal(vm, locals[1]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    DecVal(vm, keepvar[1]);
    DecVal(vm, keepvar[2]);
    DecVal(vm, keepvar[3]);
    DecVal(vm, keepvar[4]);
    DecVal(vm, keepvar[5]);
    DecVal(vm, keepvar[6]);
    DecVal(vm, keepvar[7]);
    DecVal(vm, keepvar[8]);
    DecVal(vm, keepvar[9]);
    DecVal(vm, keepvar[10]);
    DecVal(vm, keepvar[11]);
    DecVal(vm, keepvar[12]);
    DecVal(vm, keepvar[13]);
    DecVal(vm, keepvar[14]);
    DecVal(vm, keepvar[15]);
    DecVal(vm, keepvar[16]);
    DecVal(vm, keepvar[17]);
    DecVal(vm, keepvar[18]);
    DecVal(vm, keepvar[19]);
    DecVal(vm, keepvar[20]);
    DecVal(vm, keepvar[21]);
    DecVal(vm, keepvar[22]);
    DecVal(vm, keepvar[23]);
    DecVal(vm, keepvar[24]);
    DecVal(vm, keepvar[25]);
    DecVal(vm, keepvar[26]);
    DecVal(vm, keepvar[27]);
    DecVal(vm, keepvar[28]);
    DecVal(vm, keepvar[29]);
    DecVal(vm, keepvar[30]);
    DecVal(vm, keepvar[31]);
    PopFunId(vm);
}

// function345
static void fun_378(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value keepvar[2];
    Value locals[4];
    *(locals + 0) = Value(0, lobster::V_NIL);
    *(locals + 1) = Value(0, lobster::V_NIL);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 2576, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    *(keepvar + 1) = Value(0, lobster::V_NIL);
    vm.last = {9, 23};
    U_BCALLRET0(vm, regs + 0, 164, 1); // vm_compiled_mode
    U_LOGNOT(vm, regs + 1);
    if (!(regs + 0)->True()) goto block0;
    vm.last = {10, 23};
    U_PUSHSTR(vm, regs + 0, 569); // "return 1 + 2"
    U_NEWVEC(vm, regs + 1, 11, 0);
    *(keepvar + 0) = *(regs + 1);
    U_BCALLRET2(vm, regs + 2, 187, 1); // compile_run_code
    vm.temp_lval = locals + 1; // comperr1
    U_LV_WRITEREF(vm, regs + 2);
    vm.temp_lval = locals + 0; // compres1
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {11, 23};
    *(regs + 0) = *(locals + 1); // comperr1
    if (!(regs + 0)->True()) goto block1;
    vm.last = {12, 23};
    *(regs + 0) = *(locals + 1); // comperr1
    U_BCALLRET1(vm, regs + 1, 0, 0); // print
    block1:;
    vm.last = {13, 23};
    *(regs + 0) = *(locals + 0); // compres1
    U_PUSHSTR(vm, regs + 1, 570); // "3"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 13, 23, 571);
    vm.last = {15, 23};
    U_PUSHSTR(vm, regs + 0, 572); // "plugintest.lobster"
    U_NEWVEC(vm, regs + 1, 11, 0);
    *(keepvar + 1) = *(regs + 1);
    U_BCALLRET2(vm, regs + 2, 188, 1); // compile_run_file
    vm.temp_lval = locals + 3; // comperr2
    U_LV_WRITEREF(vm, regs + 2);
    vm.temp_lval = locals + 2; // compres2
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {16, 23};
    *(regs + 0) = *(locals + 3); // comperr2
    if (!(regs + 0)->True()) goto block2;
    vm.last = {17, 23};
    *(regs + 0) = *(locals + 3); // comperr2
    U_BCALLRET1(vm, regs + 1, 0, 0); // print
    block2:;
    vm.last = {18, 23};
    *(regs + 0) = *(locals + 2); // compres2
    U_PUSHSTR(vm, regs + 1, 573); // "plugintest return value"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 18, 23, 574);
    block0:;
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[2]);
    DecVal(vm, locals[3]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    DecVal(vm, keepvar[1]);
    PopFunId(vm);
}

// function346
static void fun_379(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[2];
    *(locals + 0) = Value(0, lobster::V_NIL);
    *(locals + 1) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 2583, locals);
    vm.last = {8, 24};
    vm.last = {8, 24};
    U_PUSHFUN(vm, regs + 0, 0, fun_380);
    vm.temp_lval = locals + 0; // f
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {253, 5};
    fun_380(vm, regs + 0); // call: function347
    vm.last = {13, 24};
    vm.last = {13, 24};
    U_PUSHFUN(vm, regs + 0, 0, fun_381);
    vm.temp_lval = locals + 1; // f
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {253, 5};
    fun_381(vm, regs + 0); // call: function348
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function347
static void fun_380(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = Value(0, lobster::V_NIL);
    *(locals + 1) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 2588, locals);
    vm.last = {9, 24};
    U_PUSHSTR(vm, regs + 0, 575); // ""
    (regs + 0)->LTINCRTNIL();
    U_NEWVEC(vm, regs + 1, 11, 1);
    vm.temp_lval = locals + 0; // v
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {10, 24};
    *(regs + 0) = *(locals + 0); // v
    *(regs + 1) = Value(0);
    U_VPUSHIDXI(vm, regs + 2);
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 1; // k
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {11, 24};
    *(regs + 0) = *(locals + 0); // v
    *(regs + 1) = Value(0);
    U_VPUSHIDXI(vm, regs + 2);
    (regs + 0)->LTINCRTNIL();
    vm.temp_lval = locals + 1; // k
    U_LV_WRITEREF(vm, regs + 1);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function348
static void fun_381(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = Value(0, lobster::V_NIL);
    *(locals + 1) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 2593, locals);
    vm.last = {14, 24};
    *(regs + 0) = Value(1);
    U_NEWVEC(vm, regs + 1, 7, 1);
    vm.temp_lval = locals + 0; // v
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {15, 24};
    *(regs + 0) = *(locals + 0); // v
    *(regs + 1) = Value(0);
    U_VPUSHIDXI(vm, regs + 2);
    vm.temp_lval = locals + 1; // k
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {16, 24};
    *(regs + 0) = *(locals + 0); // v
    *(regs + 1) = Value(0);
    U_VPUSHIDXI(vm, regs + 2);
    vm.temp_lval = locals + 1; // k
    *(vm.temp_lval) = *(regs + 0);
    DecVal(vm, locals[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function349
static void fun_382(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    *(locals + 0) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 2598, locals);
    vm.last = {7, 25};
    vm.last = {7, 25};
    U_PUSHFUN(vm, regs + 0, 0, fun_383);
    vm.temp_lval = locals + 0; // f
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {253, 5};
    fun_383(vm, regs + 0); // call: function350
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function350
static void fun_383(VMRef vm, StackPtr psp) {
    Value regs[9];
    Value keepvar[1];
    Value locals[2];
    *(locals + 0) = Value(0, lobster::V_NIL);
    *(locals + 1) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 2602, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    vm.last = {8, 25};
    vm.last = {8, 25};
    vm.last = {15, 25};
    U_NEWVEC(vm, regs + 0, 1287, 0);
    U_PUSHSTR(vm, regs + 1, 576); // "a"
    (regs + 1)->LTINCRTNIL();
    *(regs + 2) = Value(1.000000);
    *(regs + 3) = Value(2.000000);
    U_PUSHNIL(vm, regs + 4);
    U_PUSHSTR(vm, regs + 5, 577); // "b"
    (regs + 5)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 6, 578); // "c"
    (regs + 6)->LTINCRTNIL();
    U_NEWVEC(vm, regs + 7, 11, 2);
    *(regs + 6) = Value(1);
    *(regs + 7) = Value(2);
    U_NEWVEC(vm, regs + 8, 7, 2);
    *(regs + 7) = Value(3);
    *(regs + 8) = Value(4);
    U_NEWVEC(vm, regs + 9, 7, 2);
    U_NEWVEC(vm, regs + 8, 13, 2);
    U_NEWOBJECT(vm, regs + 7, 1260); // Nest
    vm.temp_lval = locals + 0; // nested
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {16, 25};
    *(regs + 0) = *(locals + 0); // nested
    *(regs + 1) = Value(10);
    U_BCALLRET2(vm, regs + 2, 30, 1); // deepcopy
    vm.temp_lval = locals + 1; // sub
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {17, 25};
    *(regs + 0) = *(locals + 1); // sub
    (regs + 0)->LTINCRTNIL();
    *(regs + 1) = *(locals + 0); // nested
    U_LVAL_FLD(vm, regs + 2, 4);
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {18, 25};
    *(regs + 0) = *(locals + 0); // nested
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = *(locals + 1); // sub
    (regs + 1)->LTINCRTNIL();
    *(regs + 2) = Value(1);
    U_BCALLRETV(vm, regs + 3, 15, 1); // push
    U_POP(vm, regs + 1);
    vm.last = {19, 25};
    *(regs + 0) = *(locals + 0); // nested
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = *(locals + 1); // sub
    (regs + 1)->LTINCRTNIL();
    *(regs + 2) = Value(1);
    U_BCALLRETV(vm, regs + 3, 15, 1); // push
    U_POP(vm, regs + 1);
    vm.last = {20, 25};
    *(regs + 0) = *(locals + 0); // nested
    *(regs + 1) = Value(10);
    U_BCALLRET2(vm, regs + 2, 30, 1); // deepcopy
    *(keepvar + 0) = *(regs + 0);
    *(regs + 1) = *(locals + 0); // nested
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 20, 25, 579);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// operator*
static void fun_384(VMRef vm, StackPtr psp) {
    Value regs[6];
    Value locals[8];
    *(locals + 0) = *(psp - 8);
    *(locals + 1) = *(psp - 7);
    *(locals + 2) = *(psp - 6);
    *(locals + 3) = *(psp - 5);
    *(locals + 4) = *(psp - 4);
    *(locals + 5) = *(psp - 3);
    *(locals + 6) = *(psp - 2);
    *(locals + 7) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2607, locals);
    vm.last = {13, 27};
    *(regs + 0) = *(locals + 3); // this+3
    *(regs + 1) = *(locals + 4); // o+0
    U_FMUL(vm, regs + 2);
    *(regs + 1) = *(locals + 0); // this+0
    *(regs + 2) = *(locals + 7); // o+3
    U_FMUL(vm, regs + 3);
    U_FADD(vm, regs + 2);
    *(regs + 1) = *(locals + 1); // this+1
    *(regs + 2) = *(locals + 6); // o+2
    U_FMUL(vm, regs + 3);
    U_FADD(vm, regs + 2);
    *(regs + 1) = *(locals + 2); // this+2
    *(regs + 2) = *(locals + 5); // o+1
    U_FMUL(vm, regs + 3);
    U_FSUB(vm, regs + 2);
    *(regs + 1) = *(locals + 3); // this+3
    *(regs + 2) = *(locals + 5); // o+1
    U_FMUL(vm, regs + 3);
    *(regs + 2) = *(locals + 0); // this+0
    *(regs + 3) = *(locals + 6); // o+2
    U_FMUL(vm, regs + 4);
    U_FSUB(vm, regs + 3);
    *(regs + 2) = *(locals + 1); // this+1
    *(regs + 3) = *(locals + 7); // o+3
    U_FMUL(vm, regs + 4);
    U_FADD(vm, regs + 3);
    *(regs + 2) = *(locals + 2); // this+2
    *(regs + 3) = *(locals + 4); // o+0
    U_FMUL(vm, regs + 4);
    U_FADD(vm, regs + 3);
    *(regs + 2) = *(locals + 3); // this+3
    *(regs + 3) = *(locals + 6); // o+2
    U_FMUL(vm, regs + 4);
    *(regs + 3) = *(locals + 0); // this+0
    *(regs + 4) = *(locals + 5); // o+1
    U_FMUL(vm, regs + 5);
    U_FADD(vm, regs + 4);
    *(regs + 3) = *(locals + 1); // this+1
    *(regs + 4) = *(locals + 4); // o+0
    U_FMUL(vm, regs + 5);
    U_FSUB(vm, regs + 4);
    *(regs + 3) = *(locals + 2); // this+2
    *(regs + 4) = *(locals + 7); // o+3
    U_FMUL(vm, regs + 5);
    U_FADD(vm, regs + 4);
    *(regs + 3) = *(locals + 3); // this+3
    *(regs + 4) = *(locals + 7); // o+3
    U_FMUL(vm, regs + 5);
    *(regs + 4) = *(locals + 0); // this+0
    *(regs + 5) = *(locals + 4); // o+0
    U_FMUL(vm, regs + 6);
    U_FSUB(vm, regs + 5);
    *(regs + 4) = *(locals + 1); // this+1
    *(regs + 5) = *(locals + 5); // o+1
    U_FMUL(vm, regs + 6);
    U_FSUB(vm, regs + 5);
    *(regs + 4) = *(locals + 2); // this+2
    *(regs + 5) = *(locals + 6); // o+2
    U_FMUL(vm, regs + 6);
    U_FSUB(vm, regs + 5);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    *(psp++) = *(regs + 2);
    *(psp++) = *(regs + 3);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator*
static void fun_385(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value locals[5];
    *(locals + 0) = *(psp - 5);
    *(locals + 1) = *(psp - 4);
    *(locals + 2) = *(psp - 3);
    *(locals + 3) = *(psp - 2);
    *(locals + 4) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2618, locals);
    vm.last = {16, 27};
    *(regs + 0) = *(locals + 0); // this+0
    *(regs + 1) = *(locals + 4); // f
    U_FMUL(vm, regs + 2);
    *(regs + 1) = *(locals + 1); // this+1
    *(regs + 2) = *(locals + 4); // f
    U_FMUL(vm, regs + 3);
    *(regs + 2) = *(locals + 2); // this+2
    *(regs + 3) = *(locals + 4); // f
    U_FMUL(vm, regs + 4);
    *(regs + 3) = *(locals + 3); // this+3
    *(regs + 4) = *(locals + 4); // f
    U_FMUL(vm, regs + 5);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    *(psp++) = *(regs + 2);
    *(psp++) = *(regs + 3);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator*
static void fun_396(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2626, locals);
    vm.last = {12, 26};
    *(regs + 0) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = *(locals + 1); // o
    U_PUSHFLD(vm, regs + 2, 0);
    U_IMUL(vm, regs + 2);
    U_NEWOBJECT(vm, regs + 1, 1309); // OC
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator*
static void fun_428(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2631, locals);
    vm.last = {56, 26};
    *(regs + 0) = *(locals + 0); // this+0
    *(regs + 1) = *(locals + 1); // o+0
    U_IMUL(vm, regs + 2);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator*
static void fun_446(VMRef vm, StackPtr psp) {
    Value regs[10];
    Value locals[8];
    *(locals + 0) = *(psp - 8);
    *(locals + 1) = *(psp - 7);
    *(locals + 2) = *(psp - 6);
    *(locals + 3) = *(psp - 5);
    *(locals + 4) = *(psp - 4);
    *(locals + 5) = *(psp - 3);
    *(locals + 6) = *(psp - 2);
    *(locals + 7) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2636, locals);
    vm.last = {11, 29};
    *(regs + 0) = *(locals + 4); // o.0
    *(regs + 1) = *(locals + 5); // o.1
    *(regs + 2) = *(locals + 6); // o.2
    *(regs + 3) = *(locals + 7); // o.3
    fun_70(vm, regs + 4); // call: xz
    *(regs + 2) = Value(2);
    *(regs + 3) = *(locals + 0); // this.0
    *(regs + 4) = *(locals + 1); // this.1
    *(regs + 5) = *(locals + 2); // this.2
    *(regs + 6) = *(locals + 3); // this.3
    U_POP(vm, regs + 7);
    U_POP(vm, regs + 6);
    *(regs + 5) = Value(2);
    U_BCALLRETV(vm, regs + 6, 85, 1); // dot
    *(regs + 1) = *(locals + 4); // o.0
    *(regs + 2) = *(locals + 5); // o.1
    *(regs + 3) = *(locals + 6); // o.2
    *(regs + 4) = *(locals + 7); // o.3
    fun_71(vm, regs + 5); // call: yw
    *(regs + 3) = Value(2);
    *(regs + 4) = *(locals + 0); // this.0
    *(regs + 5) = *(locals + 1); // this.1
    *(regs + 6) = *(locals + 2); // this.2
    *(regs + 7) = *(locals + 3); // this.3
    U_POP(vm, regs + 8);
    U_POP(vm, regs + 7);
    *(regs + 6) = Value(2);
    U_BCALLRETV(vm, regs + 7, 85, 1); // dot
    *(regs + 2) = *(locals + 4); // o.0
    *(regs + 3) = *(locals + 5); // o.1
    *(regs + 4) = *(locals + 6); // o.2
    *(regs + 5) = *(locals + 7); // o.3
    fun_70(vm, regs + 6); // call: xz
    *(regs + 4) = Value(2);
    *(regs + 5) = *(locals + 0); // this.0
    *(regs + 6) = *(locals + 1); // this.1
    *(regs + 7) = *(locals + 2); // this.2
    *(regs + 8) = *(locals + 3); // this.3
    fun_72(vm, regs + 9); // call: zw
    *(regs + 7) = Value(2);
    U_BCALLRETV(vm, regs + 8, 85, 1); // dot
    *(regs + 3) = *(locals + 4); // o.0
    *(regs + 4) = *(locals + 5); // o.1
    *(regs + 5) = *(locals + 6); // o.2
    *(regs + 6) = *(locals + 7); // o.3
    fun_71(vm, regs + 7); // call: yw
    *(regs + 5) = Value(2);
    *(regs + 6) = *(locals + 0); // this.0
    *(regs + 7) = *(locals + 1); // this.1
    *(regs + 8) = *(locals + 2); // this.2
    *(regs + 9) = *(locals + 3); // this.3
    fun_72(vm, regs + 10); // call: zw
    *(regs + 8) = Value(2);
    U_BCALLRETV(vm, regs + 9, 85, 1); // dot
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    *(psp++) = *(regs + 2);
    *(psp++) = *(regs + 3);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator*
static void fun_447(VMRef vm, StackPtr psp) {
    Value regs[8];
    Value locals[6];
    *(locals + 0) = *(psp - 6);
    *(locals + 1) = *(psp - 5);
    *(locals + 2) = *(psp - 4);
    *(locals + 3) = *(psp - 3);
    *(locals + 4) = *(psp - 2);
    *(locals + 5) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2647, locals);
    vm.last = {15, 29};
    *(regs + 0) = *(locals + 4); // o.0
    *(regs + 1) = *(locals + 5); // o.1
    *(regs + 2) = Value(2);
    *(regs + 3) = *(locals + 0); // this.0
    *(regs + 4) = *(locals + 1); // this.1
    *(regs + 5) = *(locals + 2); // this.2
    *(regs + 6) = *(locals + 3); // this.3
    U_POP(vm, regs + 7);
    U_POP(vm, regs + 6);
    *(regs + 5) = Value(2);
    U_BCALLRETV(vm, regs + 6, 85, 1); // dot
    *(regs + 1) = *(locals + 4); // o.0
    *(regs + 2) = *(locals + 5); // o.1
    *(regs + 3) = Value(2);
    *(regs + 4) = *(locals + 0); // this.0
    *(regs + 5) = *(locals + 1); // this.1
    *(regs + 6) = *(locals + 2); // this.2
    *(regs + 7) = *(locals + 3); // this.3
    fun_72(vm, regs + 8); // call: zw
    *(regs + 6) = Value(2);
    U_BCALLRETV(vm, regs + 7, 85, 1); // dot
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator-
static void fun_386(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[4];
    *(locals + 0) = *(psp - 4);
    *(locals + 1) = *(psp - 3);
    *(locals + 2) = *(psp - 2);
    *(locals + 3) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2656, locals);
    vm.last = {19, 27};
    *(regs + 0) = *(locals + 0); // this+0
    U_FUMINUS(vm, regs + 1);
    *(regs + 1) = *(locals + 1); // this+1
    U_FUMINUS(vm, regs + 2);
    *(regs + 2) = *(locals + 2); // this+2
    U_FUMINUS(vm, regs + 3);
    *(regs + 3) = *(locals + 3); // this+3
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    *(psp++) = *(regs + 2);
    *(psp++) = *(regs + 3);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator-
static void fun_609(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2663, locals);
    vm.last = {40, 26};
    *(regs + 0) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 1, 0);
    U_IUMINUS(vm, regs + 1);
    U_NEWOBJECT(vm, regs + 1, 1309); // OC
    DecVal(vm, locals[0]);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator-
static void fun_421(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2667, locals);
    vm.last = {40, 26};
    *(regs + 0) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 1, 0);
    U_IUMINUS(vm, regs + 1);
    U_NEWOBJECT(vm, regs + 1, 1309); // OC
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator+
static void fun_394(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2671, locals);
    vm.last = {10, 26};
    *(regs + 0) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = *(locals + 1); // o
    U_PUSHFLD(vm, regs + 2, 0);
    U_IADD(vm, regs + 2);
    U_NEWOBJECT(vm, regs + 1, 1309); // OC
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator+
static void fun_426(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2676, locals);
    vm.last = {54, 26};
    *(regs + 0) = *(locals + 0); // this+0
    *(regs + 1) = *(locals + 1); // o+0
    U_IADD(vm, regs + 2);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator-
static void fun_395(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2681, locals);
    vm.last = {11, 26};
    *(regs + 0) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = *(locals + 1); // o
    U_PUSHFLD(vm, regs + 2, 0);
    U_ISUB(vm, regs + 2);
    U_NEWOBJECT(vm, regs + 1, 1309); // OC
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator-
static void fun_427(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2686, locals);
    vm.last = {55, 26};
    *(regs + 0) = *(locals + 0); // this+0
    *(regs + 1) = *(locals + 1); // o+0
    U_ISUB(vm, regs + 2);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator/
static void fun_397(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2691, locals);
    vm.last = {13, 26};
    *(regs + 0) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = *(locals + 1); // o
    U_PUSHFLD(vm, regs + 2, 0);
    U_IDIV(vm, regs + 2);
    U_NEWOBJECT(vm, regs + 1, 1309); // OC
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator/
static void fun_429(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2696, locals);
    vm.last = {57, 26};
    *(regs + 0) = *(locals + 0); // this+0
    *(regs + 1) = *(locals + 1); // o+0
    U_IDIV(vm, regs + 2);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator+=
static void fun_404(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2701, locals);
    vm.last = {21, 26};
    *(regs + 0) = *(locals + 1); // o
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = *(locals + 0); // this
    U_LVAL_FLD(vm, regs + 2, 0);
    U_LV_IADD(vm, regs + 1);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator==
static void fun_608(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2706, locals);
    vm.last = {32, 26};
    *(regs + 0) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = *(locals + 1); // o
    U_PUSHFLD(vm, regs + 2, 0);
    U_IEQ(vm, regs + 2);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator==
static void fun_414(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2711, locals);
    vm.last = {32, 26};
    *(regs + 0) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = *(locals + 1); // o
    U_PUSHFLD(vm, regs + 2, 0);
    U_IEQ(vm, regs + 2);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator==
static void fun_436(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2716, locals);
    vm.last = {65, 26};
    *(regs + 0) = *(locals + 0); // this+0
    *(regs + 1) = *(locals + 1); // o+0
    U_IEQ(vm, regs + 2);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator!=
static void fun_415(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2721, locals);
    vm.last = {33, 26};
    *(regs + 0) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = *(locals + 1); // o
    U_PUSHFLD(vm, regs + 2, 0);
    U_INE(vm, regs + 2);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator++
static void fun_422(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2726, locals);
    vm.last = {41, 26};
    *(regs + 0) = *(locals + 0); // this
    U_LVAL_FLD(vm, regs + 1, 0);
    U_LV_DUP(vm, regs + 0);
    U_LV_IPP(vm, regs + 1);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator--
static void fun_423(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2730, locals);
    vm.last = {42, 26};
    *(regs + 0) = *(locals + 0); // this
    U_LVAL_FLD(vm, regs + 1, 0);
    U_LV_DUP(vm, regs + 0);
    U_LV_IMM(vm, regs + 1);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator=
static void fun_424(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2734, locals);
    vm.last = {44, 26};
    *(regs + 0) = *(locals + 1); // o
    U_PUSHFLD(vm, regs + 1, 0);
    *(regs + 1) = *(locals + 0); // this
    U_LVAL_FLD(vm, regs + 2, 0);
    *(vm.temp_lval) = *(regs + 0);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator[]
static void fun_425(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    *(locals + 0) = *(psp - 2);
    *(locals + 1) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2739, locals);
    vm.last = {46, 26};
    *(regs + 0) = *(locals + 1); // i
    *(regs + 1) = Value(0);
    U_IEQ(vm, regs + 2);
    if (!(regs + 0)->True()) goto block0;
    vm.last = {46, 26};
    *(regs + 0) = *(locals + 0); // this
    U_PUSHFLD(vm, regs + 1, 0);
    goto block1;
    block0:;
    vm.last = {46, 26};
    *(regs + 0) = Value(0);
    block1:;
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function390
static void fun_445(VMRef vm, StackPtr psp) {
    Value regs[8];
    Value locals[8];
    *(locals + 0) = Value(0, lobster::V_NIL);
    *(locals + 1) = Value(0, lobster::V_NIL);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    *(locals + 5) = Value(0, lobster::V_NIL);
    *(locals + 6) = Value(0, lobster::V_NIL);
    *(locals + 7) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 2744, locals);
    vm.last = {80, 26};
    *(regs + 0) = Value(1);
    U_NEWOBJECT(vm, regs + 1, 1309); // OC
    vm.temp_lval = locals + 0; // c1
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {81, 26};
    *(regs + 0) = Value(2);
    U_NEWOBJECT(vm, regs + 1, 1309); // OC
    vm.temp_lval = locals + 1; // c2
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {82, 26};
    *(regs + 0) = Value(3);
    U_NEWOBJECT(vm, regs + 1, 1309); // OC
    vm.temp_lval = locals + 2; // c3
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {83, 26};
    *(regs + 0) = Value(4);
    U_NEWOBJECT(vm, regs + 1, 1309); // OC
    vm.temp_lval = locals + 3; // c4
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {84, 26};
    *(regs + 0) = *(locals + 0); // c1
    *(regs + 1) = *(locals + 1); // c2
    fun_394(vm, regs + 2); // call: operator+
    *(regs + 1) = *(locals + 0); // c1
    fun_395(vm, regs + 2); // call: operator-
    *(regs + 1) = *(locals + 1); // c2
    fun_396(vm, regs + 2); // call: operator*
    *(regs + 1) = *(locals + 0); // c1
    fun_397(vm, regs + 2); // call: operator/
    *(regs + 1) = *(locals + 3); // c4
    fun_414(vm, regs + 2); // call: operator==
    U_ASSERT(vm, regs + 1, 84, 26, 580);
    vm.last = {85, 26};
    *(regs + 0) = *(locals + 2); // c3
    *(regs + 1) = *(locals + 0); // c1
    fun_404(vm, regs + 2); // call: operator+=
    vm.last = {86, 26};
    *(regs + 0) = *(locals + 2); // c3
    *(regs + 1) = *(locals + 3); // c4
    fun_608(vm, regs + 2); // call: operator==
    U_ASSERT(vm, regs + 1, 86, 26, 581);
    vm.last = {87, 26};
    *(regs + 0) = *(locals + 2); // c3
    fun_423(vm, regs + 1); // call: operator--
    U_POP(vm, regs + 1);
    vm.last = {88, 26};
    *(regs + 0) = *(locals + 3); // c4
    *(regs + 1) = *(locals + 2); // c3
    fun_424(vm, regs + 2); // call: operator=
    vm.last = {89, 26};
    *(regs + 0) = *(locals + 3); // c4
    *(regs + 1) = *(locals + 2); // c3
    fun_608(vm, regs + 2); // call: operator==
    U_ASSERT(vm, regs + 1, 89, 26, 582);
    vm.last = {90, 26};
    *(regs + 0) = *(locals + 3); // c4
    fun_422(vm, regs + 1); // call: operator++
    U_POP(vm, regs + 1);
    vm.last = {91, 26};
    *(regs + 0) = *(locals + 3); // c4
    fun_421(vm, regs + 1); // call: operator-
    fun_609(vm, regs + 1); // call: operator-
    *(regs + 1) = *(locals + 3); // c4
    fun_414(vm, regs + 2); // call: operator==
    U_ASSERT(vm, regs + 1, 91, 26, 583);
    vm.last = {92, 26};
    *(regs + 0) = *(locals + 3); // c4
    *(regs + 1) = *(locals + 3); // c4
    fun_415(vm, regs + 2); // call: operator!=
    *(regs + 1) = Value(0);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 92, 26, 584);
    vm.last = {93, 26};
    *(regs + 0) = *(locals + 3); // c4
    *(regs + 1) = Value(0);
    fun_425(vm, regs + 2); // call: operator[]
    *(regs + 1) = Value(4);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 93, 26, 585);
    vm.last = {96, 26};
    *(regs + 0) = Value(1);
    vm.temp_lval = locals + 4; // s1+0
    U_LV_WRITEV(vm, regs + 1, 1);
    vm.last = {97, 26};
    *(regs + 0) = Value(2);
    vm.temp_lval = locals + 5; // s2+0
    U_LV_WRITEV(vm, regs + 1, 1);
    vm.last = {98, 26};
    *(regs + 0) = Value(3);
    vm.temp_lval = locals + 6; // s3+0
    U_LV_WRITEV(vm, regs + 1, 1);
    vm.last = {99, 26};
    *(regs + 0) = Value(4);
    vm.temp_lval = locals + 7; // s4+0
    U_LV_WRITEV(vm, regs + 1, 1);
    vm.last = {100, 26};
    *(regs + 0) = *(locals + 4); // s1.0
    *(regs + 1) = *(locals + 5); // s2.0
    fun_426(vm, regs + 2); // call: operator+
    *(regs + 1) = *(locals + 4); // s1.0
    fun_427(vm, regs + 2); // call: operator-
    *(regs + 1) = *(locals + 5); // s2.0
    fun_428(vm, regs + 2); // call: operator*
    *(regs + 1) = *(locals + 4); // s1.0
    fun_429(vm, regs + 2); // call: operator/
    *(regs + 1) = *(locals + 7); // s4.0
    fun_436(vm, regs + 2); // call: operator==
    U_ASSERT(vm, regs + 1, 100, 26, 586);
    vm.last = {103, 26};
    *(regs + 0) = Value(1.000000);
    *(regs + 1) = Value(1.000000);
    *(regs + 2) = Value(1.000000);
    *(regs + 3) = Value(1.000000);
    fun_386(vm, regs + 4); // call: operator-
    *(regs + 4) = Value(-1.000000);
    *(regs + 5) = Value(-1.000000);
    *(regs + 6) = Value(-1.000000);
    *(regs + 7) = Value(1.000000);
    U_STEQ(vm, regs + 8, 4);
    U_ASSERT(vm, regs + 1, 103, 26, 587);
    vm.last = {104, 26};
    *(regs + 0) = Value(1.000000);
    *(regs + 1) = Value(1.000000);
    *(regs + 2) = Value(1.000000);
    *(regs + 3) = Value(1.000000);
    *(regs + 4) = Value(2.000000);
    *(regs + 5) = Value(2.000000);
    *(regs + 6) = Value(2.000000);
    *(regs + 7) = Value(1.000000);
    fun_384(vm, regs + 8); // call: operator*
    *(regs + 4) = Value(3.000000);
    *(regs + 5) = Value(3.000000);
    *(regs + 6) = Value(3.000000);
    *(regs + 7) = Value(-5.000000);
    U_STEQ(vm, regs + 8, 4);
    U_ASSERT(vm, regs + 1, 104, 26, 588);
    vm.last = {105, 26};
    *(regs + 0) = Value(1.000000);
    *(regs + 1) = Value(1.000000);
    *(regs + 2) = Value(1.000000);
    *(regs + 3) = Value(1.000000);
    *(regs + 4) = Value(2.000000);
    fun_385(vm, regs + 5); // call: operator*
    *(regs + 4) = Value(2.000000);
    *(regs + 5) = Value(2.000000);
    *(regs + 6) = Value(2.000000);
    *(regs + 7) = Value(2.000000);
    U_STEQ(vm, regs + 8, 4);
    U_ASSERT(vm, regs + 1, 105, 26, 589);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[2]);
    DecVal(vm, locals[3]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// determinant
static void fun_448(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[4];
    *(locals + 0) = *(psp - 4);
    *(locals + 1) = *(psp - 3);
    *(locals + 2) = *(psp - 2);
    *(locals + 3) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2755, locals);
    vm.last = {18, 29};
    *(regs + 0) = *(locals + 0); // this+0
    *(regs + 1) = *(locals + 3); // this+3
    U_FMUL(vm, regs + 2);
    *(regs + 1) = *(locals + 1); // this+1
    *(regs + 2) = *(locals + 2); // this+2
    U_FMUL(vm, regs + 3);
    U_FSUB(vm, regs + 2);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// adjugate
static void fun_449(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[4];
    *(locals + 0) = *(psp - 4);
    *(locals + 1) = *(psp - 3);
    *(locals + 2) = *(psp - 2);
    *(locals + 3) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2762, locals);
    vm.last = {21, 29};
    *(regs + 0) = *(locals + 3); // this+3
    *(regs + 1) = *(locals + 1); // this+1
    U_FUMINUS(vm, regs + 2);
    *(regs + 2) = *(locals + 2); // this+2
    U_FUMINUS(vm, regs + 3);
    *(regs + 3) = *(locals + 0); // this+0
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    *(psp++) = *(regs + 2);
    *(psp++) = *(regs + 3);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// inverse
static void fun_450(VMRef vm, StackPtr psp) {
    Value regs[8];
    Value locals[4];
    *(locals + 0) = *(psp - 4);
    *(locals + 1) = *(psp - 3);
    *(locals + 2) = *(psp - 2);
    *(locals + 3) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2769, locals);
    vm.last = {24, 29};
    *(regs + 0) = *(locals + 0); // this.0
    *(regs + 1) = *(locals + 1); // this.1
    *(regs + 2) = *(locals + 2); // this.2
    *(regs + 3) = *(locals + 3); // this.3
    fun_449(vm, regs + 4); // call: adjugate
    *(regs + 4) = *(locals + 0); // this.0
    *(regs + 5) = *(locals + 1); // this.1
    *(regs + 6) = *(locals + 2); // this.2
    *(regs + 7) = *(locals + 3); // this.3
    fun_448(vm, regs + 8); // call: determinant
    U_FVSDIV(vm, regs + 5, 4);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    *(psp++) = *(regs + 1);
    *(psp++) = *(regs + 2);
    *(psp++) = *(regs + 3);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// trace
static void fun_451(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[4];
    *(locals + 0) = *(psp - 4);
    *(locals + 1) = *(psp - 3);
    *(locals + 2) = *(psp - 2);
    *(locals + 3) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2776, locals);
    vm.last = {27, 29};
    *(regs + 0) = *(locals + 0); // this+0
    *(regs + 1) = *(locals + 3); // this+3
    U_FMUL(vm, regs + 2);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function395
static void fun_452(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    *(locals + 0) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 2783, locals);
    vm.last = {6, 28};
    vm.last = {6, 28};
    U_PUSHFUN(vm, regs + 0, 0, fun_453);
    vm.temp_lval = locals + 0; // f
    *(vm.temp_lval) = *(regs + 0);
    vm.last = {253, 5};
    fun_453(vm, regs + 0); // call: function396
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function396
static void fun_453(VMRef vm, StackPtr psp) {
    Value regs[8];
    Value locals[10];
    *(locals + 0) = Value(0, lobster::V_NIL);
    *(locals + 1) = Value(0, lobster::V_NIL);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    *(locals + 5) = Value(0, lobster::V_NIL);
    *(locals + 6) = Value(0, lobster::V_NIL);
    *(locals + 7) = Value(0, lobster::V_NIL);
    *(locals + 8) = Value(0, lobster::V_NIL);
    *(locals + 9) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 2787, locals);
    vm.last = {7, 28};
    *(regs + 0) = Value(1.000000);
    *(regs + 1) = Value(2.000000);
    *(regs + 2) = Value(3.000000);
    *(regs + 3) = Value(4.000000);
    vm.temp_lval = locals + 0; // a+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {8, 28};
    *(regs + 0) = Value(5.000000);
    *(regs + 1) = Value(6.000000);
    *(regs + 2) = Value(7.000000);
    *(regs + 3) = Value(8.000000);
    vm.temp_lval = locals + 4; // b+0
    U_LV_WRITEV(vm, regs + 4, 4);
    vm.last = {10, 28};
    *(regs + 0) = *(locals + 0); // a.0
    *(regs + 1) = *(locals + 1); // a.1
    *(regs + 2) = *(locals + 2); // a.2
    *(regs + 3) = *(locals + 3); // a.3
    *(regs + 4) = *(locals + 0); // a.0
    *(regs + 5) = *(locals + 1); // a.1
    *(regs + 6) = *(locals + 2); // a.2
    *(regs + 7) = *(locals + 3); // a.3
    U_STEQ(vm, regs + 8, 4);
    U_ASSERT(vm, regs + 1, 10, 28, 590);
    vm.last = {11, 28};
    *(regs + 0) = *(locals + 0); // a.0
    *(regs + 1) = *(locals + 1); // a.1
    *(regs + 2) = *(locals + 2); // a.2
    *(regs + 3) = *(locals + 3); // a.3
    *(regs + 4) = Value(1.000000);
    *(regs + 5) = Value(2.000000);
    *(regs + 6) = Value(3.000000);
    *(regs + 7) = Value(4.000000);
    U_STEQ(vm, regs + 8, 4);
    U_ASSERT(vm, regs + 1, 11, 28, 591);
    vm.last = {12, 28};
    *(regs + 0) = *(locals + 0); // a.0
    *(regs + 1) = *(locals + 1); // a.1
    *(regs + 2) = *(locals + 2); // a.2
    *(regs + 3) = *(locals + 3); // a.3
    *(regs + 4) = *(locals + 4); // b.0
    *(regs + 5) = *(locals + 5); // b.1
    *(regs + 6) = *(locals + 6); // b.2
    *(regs + 7) = *(locals + 7); // b.3
    U_STNE(vm, regs + 8, 4);
    U_ASSERT(vm, regs + 1, 12, 28, 592);
    vm.last = {14, 28};
    *(regs + 0) = Value(5.000000);
    *(regs + 1) = *(locals + 0); // a.0
    *(regs + 2) = *(locals + 1); // a.1
    *(regs + 3) = *(locals + 2); // a.2
    *(regs + 4) = *(locals + 3); // a.3
    U_SFVADD(vm, regs + 5, 4);
    *(regs + 4) = Value(6.000000);
    *(regs + 5) = Value(7.000000);
    *(regs + 6) = Value(8.000000);
    *(regs + 7) = Value(9.000000);
    U_STEQ(vm, regs + 8, 4);
    U_ASSERT(vm, regs + 1, 14, 28, 593);
    vm.last = {15, 28};
    *(regs + 0) = *(locals + 0); // a.0
    *(regs + 1) = *(locals + 1); // a.1
    *(regs + 2) = *(locals + 2); // a.2
    *(regs + 3) = *(locals + 3); // a.3
    *(regs + 4) = Value(5.000000);
    U_FVSADD(vm, regs + 5, 4);
    *(regs + 4) = Value(6.000000);
    *(regs + 5) = Value(7.000000);
    *(regs + 6) = Value(8.000000);
    *(regs + 7) = Value(9.000000);
    U_STEQ(vm, regs + 8, 4);
    U_ASSERT(vm, regs + 1, 15, 28, 594);
    vm.last = {16, 28};
    *(regs + 0) = *(locals + 0); // a.0
    *(regs + 1) = *(locals + 1); // a.1
    *(regs + 2) = *(locals + 2); // a.2
    *(regs + 3) = *(locals + 3); // a.3
    *(regs + 4) = *(locals + 4); // b.0
    *(regs + 5) = *(locals + 5); // b.1
    *(regs + 6) = *(locals + 6); // b.2
    *(regs + 7) = *(locals + 7); // b.3
    U_FVVADD(vm, regs + 8, 4);
    *(regs + 4) = Value(6.000000);
    *(regs + 5) = Value(8.000000);
    *(regs + 6) = Value(10.000000);
    *(regs + 7) = Value(12.000000);
    U_STEQ(vm, regs + 8, 4);
    U_ASSERT(vm, regs + 1, 16, 28, 595);
    vm.last = {18, 28};
    *(regs + 0) = Value(5.000000);
    *(regs + 1) = *(locals + 0); // a.0
    *(regs + 2) = *(locals + 1); // a.1
    *(regs + 3) = *(locals + 2); // a.2
    *(regs + 4) = *(locals + 3); // a.3
    U_SFVSUB(vm, regs + 5, 4);
    *(regs + 4) = Value(4.000000);
    *(regs + 5) = Value(3.000000);
    *(regs + 6) = Value(2.000000);
    *(regs + 7) = Value(1.000000);
    U_STEQ(vm, regs + 8, 4);
    U_ASSERT(vm, regs + 1, 18, 28, 596);
    vm.last = {19, 28};
    *(regs + 0) = *(locals + 0); // a.0
    *(regs + 1) = *(locals + 1); // a.1
    *(regs + 2) = *(locals + 2); // a.2
    *(regs + 3) = *(locals + 3); // a.3
    *(regs + 4) = Value(5.000000);
    U_FVSSUB(vm, regs + 5, 4);
    *(regs + 4) = Value(-4.000000);
    *(regs + 5) = Value(-3.000000);
    *(regs + 6) = Value(-2.000000);
    *(regs + 7) = Value(-1.000000);
    U_STEQ(vm, regs + 8, 4);
    U_ASSERT(vm, regs + 1, 19, 28, 597);
    vm.last = {20, 28};
    *(regs + 0) = *(locals + 4); // b.0
    *(regs + 1) = *(locals + 5); // b.1
    *(regs + 2) = *(locals + 6); // b.2
    *(regs + 3) = *(locals + 7); // b.3
    *(regs + 4) = *(locals + 0); // a.0
    *(regs + 5) = *(locals + 1); // a.1
    *(regs + 6) = *(locals + 2); // a.2
    *(regs + 7) = *(locals + 3); // a.3
    U_FVVSUB(vm, regs + 8, 4);
    *(regs + 4) = Value(4.000000);
    *(regs + 5) = Value(4.000000);
    *(regs + 6) = Value(4.000000);
    *(regs + 7) = Value(4.000000);
    U_STEQ(vm, regs + 8, 4);
    U_ASSERT(vm, regs + 1, 20, 28, 598);
    vm.last = {22, 28};
    *(regs + 0) = Value(5.000000);
    *(regs + 1) = *(locals + 0); // a.0
    *(regs + 2) = *(locals + 1); // a.1
    *(regs + 3) = *(locals + 2); // a.2
    *(regs + 4) = *(locals + 3); // a.3
    U_SFVMUL(vm, regs + 5, 4);
    *(regs + 4) = Value(5.000000);
    *(regs + 5) = Value(10.000000);
    *(regs + 6) = Value(15.000000);
    *(regs + 7) = Value(20.000000);
    U_STEQ(vm, regs + 8, 4);
    U_ASSERT(vm, regs + 1, 22, 28, 599);
    vm.last = {26, 28};
    *(regs + 0) = *(locals + 0); // a.0
    *(regs + 1) = *(locals + 1); // a.1
    *(regs + 2) = *(locals + 2); // a.2
    *(regs + 3) = *(locals + 3); // a.3
    *(regs + 4) = *(locals + 4); // b.0
    *(regs + 5) = *(locals + 5); // b.1
    *(regs + 6) = *(locals + 6); // b.2
    *(regs + 7) = *(locals + 7); // b.3
    fun_446(vm, regs + 8); // call: operator*
    *(regs + 4) = Value(19.000000);
    *(regs + 5) = Value(22.000000);
    *(regs + 6) = Value(43.000000);
    *(regs + 7) = Value(50.000000);
    U_STEQ(vm, regs + 8, 4);
    U_ASSERT(vm, regs + 1, 26, 28, 600);
    vm.last = {27, 28};
    *(regs + 0) = *(locals + 4); // b.0
    *(regs + 1) = *(locals + 5); // b.1
    *(regs + 2) = *(locals + 6); // b.2
    *(regs + 3) = *(locals + 7); // b.3
    *(regs + 4) = *(locals + 0); // a.0
    *(regs + 5) = *(locals + 1); // a.1
    *(regs + 6) = *(locals + 2); // a.2
    *(regs + 7) = *(locals + 3); // a.3
    fun_446(vm, regs + 8); // call: operator*
    *(regs + 4) = Value(23.000000);
    *(regs + 5) = Value(34.000000);
    *(regs + 6) = Value(31.000000);
    *(regs + 7) = Value(46.000000);
    U_STEQ(vm, regs + 8, 4);
    U_ASSERT(vm, regs + 1, 27, 28, 601);
    vm.last = {29, 28};
    *(regs + 0) = *(locals + 0); // a.0
    *(regs + 1) = *(locals + 1); // a.1
    *(regs + 2) = *(locals + 2); // a.2
    *(regs + 3) = *(locals + 3); // a.3
    fun_448(vm, regs + 4); // call: determinant
    *(regs + 1) = Value(-2.000000);
    U_FEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 29, 28, 602);
    vm.last = {30, 28};
    *(regs + 0) = *(locals + 0); // a.0
    *(regs + 1) = *(locals + 1); // a.1
    *(regs + 2) = *(locals + 2); // a.2
    *(regs + 3) = *(locals + 3); // a.3
    fun_451(vm, regs + 4); // call: trace
    *(regs + 1) = Value(4.000000);
    U_FEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 30, 28, 603);
    vm.last = {31, 28};
    *(regs + 0) = *(locals + 0); // a.0
    *(regs + 1) = *(locals + 1); // a.1
    *(regs + 2) = *(locals + 2); // a.2
    *(regs + 3) = *(locals + 3); // a.3
    fun_449(vm, regs + 4); // call: adjugate
    *(regs + 4) = Value(4.000000);
    *(regs + 5) = Value(-2.000000);
    *(regs + 6) = Value(-3.000000);
    *(regs + 7) = Value(1.000000);
    U_STEQ(vm, regs + 8, 4);
    U_ASSERT(vm, regs + 1, 31, 28, 604);
    vm.last = {32, 28};
    *(regs + 0) = *(locals + 0); // a.0
    *(regs + 1) = *(locals + 1); // a.1
    *(regs + 2) = *(locals + 2); // a.2
    *(regs + 3) = *(locals + 3); // a.3
    fun_450(vm, regs + 4); // call: inverse
    *(regs + 4) = Value(-2.000000);
    *(regs + 5) = Value(1.000000);
    *(regs + 6) = Value(1.500000);
    *(regs + 7) = Value(-0.500000);
    U_STEQ(vm, regs + 8, 4);
    U_ASSERT(vm, regs + 1, 32, 28, 605);
    vm.last = {34, 28};
    *(regs + 0) = *(locals + 0); // a.0
    *(regs + 1) = *(locals + 1); // a.1
    *(regs + 2) = *(locals + 2); // a.2
    *(regs + 3) = *(locals + 3); // a.3
    *(regs + 4) = *(locals + 0); // a.0
    *(regs + 5) = *(locals + 1); // a.1
    *(regs + 6) = *(locals + 2); // a.2
    *(regs + 7) = *(locals + 3); // a.3
    fun_450(vm, regs + 8); // call: inverse
    fun_446(vm, regs + 8); // call: operator*
    U_PUSHVARVF(vm, regs + 4, 1150, 4); // mat2x2_identity+0
    U_STEQ(vm, regs + 8, 4);
    U_ASSERT(vm, regs + 1, 34, 28, 606);
    vm.last = {38, 28};
    *(regs + 0) = Value(2.000000);
    *(regs + 1) = Value(3.000000);
    vm.temp_lval = locals + 8; // c+0
    U_LV_WRITEV(vm, regs + 2, 2);
    vm.last = {40, 28};
    *(regs + 0) = *(locals + 0); // a.0
    *(regs + 1) = *(locals + 1); // a.1
    *(regs + 2) = *(locals + 2); // a.2
    *(regs + 3) = *(locals + 3); // a.3
    *(regs + 4) = *(locals + 8); // c.0
    *(regs + 5) = *(locals + 9); // c.1
    fun_447(vm, regs + 6); // call: operator*
    *(regs + 2) = Value(8.000000);
    *(regs + 3) = Value(18.000000);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 40, 28, 607);
    vm.last = {41, 28};
    *(regs + 0) = *(locals + 4); // b.0
    *(regs + 1) = *(locals + 5); // b.1
    *(regs + 2) = *(locals + 6); // b.2
    *(regs + 3) = *(locals + 7); // b.3
    *(regs + 4) = *(locals + 8); // c.0
    *(regs + 5) = *(locals + 9); // c.1
    fun_447(vm, regs + 6); // call: operator*
    *(regs + 2) = Value(28.000000);
    *(regs + 3) = Value(38.000000);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 41, 28, 608);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function397
static void fun_454(VMRef vm, StackPtr psp) {
    Value regs[7];
    Value keepvar[6];
    Value locals[9];
    *(locals + 0) = Value(0, lobster::V_NIL);
    *(locals + 1) = Value(0, lobster::V_NIL);
    *(locals + 2) = Value(0, lobster::V_NIL);
    *(locals + 3) = Value(0, lobster::V_NIL);
    *(locals + 4) = Value(0, lobster::V_NIL);
    *(locals + 5) = Value(0, lobster::V_NIL);
    *(locals + 6) = Value(0, lobster::V_NIL);
    *(locals + 7) = Value(0, lobster::V_NIL);
    *(locals + 8) = Value(0, lobster::V_NIL);
    PushFunId(vm, funinfo_table + 2800, locals);
    *(keepvar + 0) = Value(0, lobster::V_NIL);
    *(keepvar + 1) = Value(0, lobster::V_NIL);
    *(keepvar + 2) = Value(0, lobster::V_NIL);
    *(keepvar + 3) = Value(0, lobster::V_NIL);
    *(keepvar + 4) = Value(0, lobster::V_NIL);
    *(keepvar + 5) = Value(0, lobster::V_NIL);
    vm.last = {6, 30};
    vm.last = {6, 30};
    vm.last = {9, 30};
    vm.last = {9, 30};
    vm.last = {12, 30};
    vm.last = {12, 30};
    vm.last = {15, 30};
    vm.last = {15, 30};
    vm.last = {18, 30};
    vm.last = {18, 30};
    vm.last = {21, 30};
    vm.last = {21, 30};
    vm.last = {39, 30};
    *(regs + 0) = Value(0);
    *(regs + 1) = Value(1);
    U_NEWOBJECT(vm, regs + 2, 1623); // B
    *(regs + 1) = Value(0);
    *(regs + 2) = Value(2);
    *(regs + 3) = Value(3);
    U_NEWOBJECT(vm, regs + 4, 1635); // D
    *(regs + 2) = Value(0);
    *(regs + 3) = Value(2);
    *(regs + 4) = Value(4);
    U_NEWOBJECT(vm, regs + 5, 1662); // E
    *(regs + 3) = Value(0);
    *(regs + 4) = Value(2);
    *(regs + 5) = Value(4);
    *(regs + 6) = Value(5);
    U_NEWOBJECT(vm, regs + 7, 1677); // F
    U_NEWVEC(vm, regs + 4, 1336, 4);
    vm.temp_lval = locals + 0; // tests
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {42, 30};
    *(regs + 0) = *(locals + 0); // tests
    U_PUSHFUN(vm, regs + 1, 0, fun_455);
    fun_611(vm, regs + 2); // call: map
    vm.temp_lval = locals + 1; // results1
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {50, 30};
    *(regs + 0) = *(locals + 1); // results1
    *(regs + 1) = Value(1);
    *(regs + 2) = Value(3);
    *(regs + 3) = Value(4);
    *(regs + 4) = Value(5);
    U_NEWVEC(vm, regs + 5, 7, 4);
    *(keepvar + 0) = *(regs + 1);
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 50, 30, 609);
    vm.last = {53, 30};
    *(regs + 0) = *(locals + 0); // tests
    U_PUSHFUN(vm, regs + 1, 0, fun_456);
    fun_612(vm, regs + 2); // call: map
    vm.temp_lval = locals + 2; // results2
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {60, 30};
    *(regs + 0) = *(locals + 2); // results2
    *(regs + 1) = Value(1);
    *(regs + 2) = Value(3);
    *(regs + 3) = Value(4);
    *(regs + 4) = Value(4);
    U_NEWVEC(vm, regs + 5, 7, 4);
    *(keepvar + 1) = *(regs + 1);
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 60, 30, 610);
    vm.last = {63, 30};
    *(regs + 0) = *(locals + 0); // tests
    U_PUSHFUN(vm, regs + 1, 0, fun_457);
    fun_613(vm, regs + 2); // call: map
    vm.temp_lval = locals + 3; // results3
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {69, 30};
    *(regs + 0) = *(locals + 3); // results3
    *(regs + 1) = Value(1);
    *(regs + 2) = Value(2);
    *(regs + 3) = Value(2);
    *(regs + 4) = Value(2);
    U_NEWVEC(vm, regs + 5, 7, 4);
    *(keepvar + 2) = *(regs + 1);
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 69, 30, 611);
    vm.last = {72, 30};
    *(regs + 0) = *(locals + 0); // tests
    U_PUSHFUN(vm, regs + 1, 0, fun_458);
    fun_614(vm, regs + 2); // call: map
    vm.temp_lval = locals + 4; // results4
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {77, 30};
    *(regs + 0) = *(locals + 4); // results4
    *(regs + 1) = Value(0);
    *(regs + 2) = Value(0);
    *(regs + 3) = Value(0);
    *(regs + 4) = Value(5);
    U_NEWVEC(vm, regs + 5, 7, 4);
    *(keepvar + 3) = *(regs + 1);
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 77, 30, 612);
    vm.last = {82, 30};
    *(regs + 0) = Value(1);
    *(regs + 1) = Value(0);
    U_NEWVEC(vm, regs + 2, 1338, 2);
    vm.temp_lval = locals + 5; // btests
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {84, 30};
    *(regs + 0) = *(locals + 5); // btests
    U_PUSHFUN(vm, regs + 1, 0, fun_459);
    fun_615(vm, regs + 2); // call: map
    vm.temp_lval = locals + 6; // results5
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {91, 30};
    *(regs + 0) = *(locals + 6); // results5
    U_PUSHSTR(vm, regs + 1, 613); // "t"
    (regs + 1)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 2, 614); // "f"
    (regs + 2)->LTINCRTNIL();
    U_NEWVEC(vm, regs + 3, 11, 2);
    *(keepvar + 4) = *(regs + 1);
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 91, 30, 615);
    vm.last = {93, 30};
    *(regs + 0) = Value(1);
    *(regs + 1) = Value(0);
    *(regs + 2) = Value(2);
    U_NEWVEC(vm, regs + 3, 1338, 3);
    vm.temp_lval = locals + 7; // btests2
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {95, 30};
    *(regs + 0) = *(locals + 7); // btests2
    U_PUSHFUN(vm, regs + 1, 0, fun_460);
    fun_616(vm, regs + 2); // call: map
    vm.temp_lval = locals + 8; // results6
    U_LV_WRITEREF(vm, regs + 1);
    vm.last = {110, 30};
    *(regs + 0) = *(locals + 8); // results6
    U_PUSHSTR(vm, regs + 1, 616); // "t"
    (regs + 1)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 2, 617); // "f"
    (regs + 2)->LTINCRTNIL();
    U_PUSHSTR(vm, regs + 3, 618); // "?"
    (regs + 3)->LTINCRTNIL();
    U_NEWVEC(vm, regs + 4, 11, 3);
    *(keepvar + 5) = *(regs + 1);
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 110, 30, 619);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[2]);
    DecVal(vm, locals[3]);
    DecVal(vm, locals[4]);
    DecVal(vm, locals[5]);
    DecVal(vm, locals[6]);
    DecVal(vm, locals[7]);
    DecVal(vm, locals[8]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    DecVal(vm, keepvar[1]);
    DecVal(vm, keepvar[2]);
    DecVal(vm, keepvar[3]);
    DecVal(vm, keepvar[4]);
    DecVal(vm, keepvar[5]);
    PopFunId(vm);
}

// function398
static void fun_455(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2812, locals);
    vm.last = {43, 30};
    *(regs + 0) = *(locals + 0); // t
    switch (GetTypeSwitchID(vm, regs[0], 0)) {
    case 0:
    vm.last = {44, 30};
    *(regs + 0) = *(locals + 0); // t
    U_PUSHFLD(vm, regs + 1, 1);
    goto block5;
    case 1:
    vm.last = {45, 30};
    *(regs + 0) = *(locals + 0); // t
    U_PUSHFLD(vm, regs + 1, 2);
    goto block6;
    case 2:
    vm.last = {46, 30};
    *(regs + 0) = *(locals + 0); // t
    U_PUSHFLD(vm, regs + 1, 2);
    goto block7;
    case 3:
    vm.last = {47, 30};
    *(regs + 0) = *(locals + 0); // t
    U_PUSHFLD(vm, regs + 1, 3);
    } // switch
    block5:;
    block6:;
    block7:;
    DecVal(vm, locals[0]);
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function399
static void fun_456(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2816, locals);
    vm.last = {54, 30};
    *(regs + 0) = *(locals + 0); // t
    switch (GetTypeSwitchID(vm, regs[0], 1)) {
    case 0:
    vm.last = {55, 30};
    *(regs + 0) = *(locals + 0); // t
    U_PUSHFLD(vm, regs + 1, 1);
    goto block4;
    case 1:
    vm.last = {56, 30};
    *(regs + 0) = *(locals + 0); // t
    U_PUSHFLD(vm, regs + 1, 2);
    goto block5;
    case 2:
    vm.last = {57, 30};
    *(regs + 0) = *(locals + 0); // t
    U_PUSHFLD(vm, regs + 1, 2);
    } // switch
    block4:;
    block5:;
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function400
static void fun_457(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2820, locals);
    vm.last = {64, 30};
    *(regs + 0) = *(locals + 0); // t
    switch (GetTypeSwitchID(vm, regs[0], 2)) {
    case 0:
    vm.last = {65, 30};
    *(regs + 0) = *(locals + 0); // t
    U_PUSHFLD(vm, regs + 1, 1);
    goto block3;
    case 1:
    vm.last = {66, 30};
    *(regs + 0) = *(locals + 0); // t
    U_PUSHFLD(vm, regs + 1, 1);
    } // switch
    block3:;
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function401
static void fun_458(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2824, locals);
    vm.last = {73, 30};
    *(regs + 0) = *(locals + 0); // t
    switch (GetTypeSwitchID(vm, regs[0], 3)) {
    case 0:
    vm.last = {75, 30};
    *(regs + 0) = *(locals + 0); // t
    U_PUSHFLD(vm, regs + 1, 3);
    goto block3;
    case 1:default:
    vm.last = {74, 30};
    *(regs + 0) = *(locals + 0); // t
    U_PUSHFLD(vm, regs + 1, 0);
    } // switch
    block3:;
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function402
static void fun_459(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2828, locals);
    vm.last = {85, 30};
    *(regs + 0) = *(locals + 0); // t
    U_DUP(vm, regs + 1);
    *(regs + 2) = Value(1);
    U_IEQ(vm, regs + 3);
    if (!(regs + 1)->True()) goto block0;
    U_POP(vm, regs + 1);
    vm.last = {86, 30};
    U_PUSHSTR(vm, regs + 0, 620); // "t"
    (regs + 0)->LTINCRTNIL();
    goto block1;
    block0:;
    U_DUP(vm, regs + 1);
    *(regs + 2) = Value(0);
    U_IEQ(vm, regs + 3);
    if (!(regs + 1)->True()) goto block2;
    U_POP(vm, regs + 1);
    vm.last = {87, 30};
    U_PUSHSTR(vm, regs + 0, 621); // "f"
    (regs + 0)->LTINCRTNIL();
    goto block3;
    block2:;
    U_POP(vm, regs + 1);
    U_ENUM_RANGE_ERR(vm, regs + 0);
    block1:;
    block3:;
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function403
static void fun_460(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[1];
    *(locals + 0) = *(psp - 1);
    PushFunId(vm, funinfo_table + 2832, locals);
    vm.last = {96, 30};
    *(regs + 0) = *(locals + 0); // t
    U_DUP(vm, regs + 1);
    *(regs + 2) = Value(1);
    U_IEQ(vm, regs + 3);
    if (!(regs + 1)->True()) goto block0;
    U_POP(vm, regs + 1);
    vm.last = {97, 30};
    U_PUSHSTR(vm, regs + 0, 622); // "t"
    (regs + 0)->LTINCRTNIL();
    goto block1;
    block0:;
    U_DUP(vm, regs + 1);
    *(regs + 2) = Value(0);
    U_IEQ(vm, regs + 3);
    if (!(regs + 1)->True()) goto block2;
    U_POP(vm, regs + 1);
    vm.last = {98, 30};
    U_PUSHSTR(vm, regs + 0, 623); // "f"
    (regs + 0)->LTINCRTNIL();
    goto block3;
    block2:;
    U_POP(vm, regs + 1);
    vm.last = {99, 30};
    U_PUSHSTR(vm, regs + 0, 624); // "?"
    (regs + 0)->LTINCRTNIL();
    block1:;
    block3:;
    Pop(psp);
    *(psp++) = *(regs + 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

static void fun_10000002(VMRef vm, StackPtr psp) {
    Value regs[1];
    fun_0(vm, regs + 0); // call: __top_level_expression
    U_EXIT(vm, regs + 0, -1);
}

static const fun_base_t vtables[] = {
    fun_118,
    fun_119,
    fun_159,
    fun_160,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    fun_219,
    fun_220,
    fun_519,
    fun_221,
    fun_223,
    fun_218,
    fun_224,
    fun_222,
    fun_225,
    fun_226,
    fun_520,
    fun_227,
    fun_229,
    fun_230,
    fun_224,
    fun_228,
    0,
    0,
    fun_290,
    fun_291,
    fun_292,
    fun_293,
    fun_294,
    fun_295,
    fun_296,
    fun_297,
    fun_298,
    fun_299,
    fun_300,
    fun_301,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    fun_315,
    fun_315,
    fun_316,
    fun_316,
    fun_316,
    fun_317,
    fun_317,
    fun_318,
    fun_318,
    fun_320,
    fun_320,
    fun_320,
    fun_323,
    fun_323,
    0,
    0,
    0,
    0,
    (fun_base_t)0,
    (fun_base_t)0,
    (fun_base_t)0,
    (fun_base_t)1,
    (fun_base_t)1,
    (fun_base_t)1,
    (fun_base_t)1,
    (fun_base_t)1,
    (fun_base_t)2,
    (fun_base_t)2,
    (fun_base_t)1,
    (fun_base_t)1,
    (fun_base_t)3,
    (fun_base_t)2,
    (fun_base_t)1,
    (fun_base_t)0,
    fun_231,
    fun_220,
    fun_519,
    fun_221,
    fun_223,
    fun_218,
    fun_224,
    fun_222,
    0
};

const int funinfo_table[] = {
    0, 0, 241, 0, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 
    86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 
    102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 
    118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 
    134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 
    150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 
    166, 167, 168, 169, 170, 171, 172, 173, 798, 799, 800, 801, 802, 803, 804, 805, 
    806, 807, 808, 809, 810, 811, 812, 813, 814, 815, 816, 817, 818, 819, 820, 821, 
    822, 823, 824, 825, 826, 827, 828, 829, 830, 831, 832, 833, 834, 835, 836, 837, 
    838, 839, 840, 841, 842, 843, 844, 845, 846, 847, 848, 849, 850, 851, 852, 853, 
    854, 855, 856, 857, 858, 859, 860, 861, 862, 863, 864, 865, 866, 867, 868, 869, 
    870, 871, 872, 873, 874, 875, 876, 877, 878, 879, 880, 881, 882, 883, 884, 885, 
    886, 887, 888, 889, 890, 891, 892, 893, 894, 895, 896, 897, 898, 899, 900, 901, 
    902, 903, 904, 905, 906, 907, 908, 909, 910, 911, 912, 913, 914, 915, 916, 917, 
    918, 919, 920, 921, 922, 923, 924, 925, 926, 927, 928, 929, 1077, 1078, 1079, 1080, 
    1150, 1151, 1152, 1153, 3, 2, 1, 2, 0, 4, 2, 3, 4, 0, 5, 0, 
    0, 6, 2, 5, 6, 0, 616, 2, 1631, 1632, 3, 1633, 1634, 1635, 615, 2, 
    1626, 1627, 3, 1628, 1629, 1630, 614, 2, 1621, 1622, 3, 1623, 1624, 1625, 613, 2, 
    1616, 1617, 3, 1618, 1619, 1620, 612, 2, 1611, 1612, 3, 1613, 1614, 1615, 611, 2, 
    1606, 1607, 3, 1608, 1609, 1610, 604, 2, 1592, 1593, 5, 1594, 1595, 1596, 1597, 1598, 
    603, 2, 1587, 1588, 3, 1589, 1590, 1591, 602, 2, 1582, 1583, 3, 1584, 1585, 1586, 
    601, 2, 1577, 1578, 3, 1579, 1580, 1581, 593, 2, 1543, 1544, 3, 1545, 1546, 1547, 
    592, 2, 1537, 1538, 4, 1539, 1540, 1541, 1542, 587, 2, 1512, 1513, 4, 1514, 1515, 
    1516, 1517, 586, 2, 1507, 1508, 3, 1509, 1510, 1511, 585, 2, 1502, 1503, 3, 1504, 
    1505, 1506, 584, 2, 1497, 1498, 3, 1499, 1500, 1501, 582, 2, 1489, 1490, 3, 1491, 
    1492, 1493, 581, 2, 1484, 1485, 3, 1486, 1487, 1488, 579, 2, 1478, 1479, 3, 1480, 
    1481, 1482, 576, 2, 1472, 1473, 3, 1474, 1475, 1476, 573, 2, 1464, 1465, 3, 1466, 
    1467, 1468, 566, 2, 1434, 1435, 4, 1436, 1437, 1438, 1439, 565, 2, 1428, 1429, 4, 
    1430, 1431, 1432, 1433, 564, 2, 1423, 1424, 3, 1425, 1426, 1427, 563, 2, 1418, 1419, 
    3, 1420, 1421, 1422, 562, 2, 1413, 1414, 3, 1415, 1416, 1417, 558, 2, 1392, 1393, 
    3, 1394, 1395, 1396, 543, 2, 1355, 1356, 3, 1357, 1358, 1359, 542, 2, 1350, 1351, 
    3, 1352, 1353, 1354, 532, 2, 1324, 1325, 3, 1326, 1327, 1328, 505, 2, 1286, 1287, 
    3, 1288, 1289, 1290, 502, 2, 1279, 1280, 3, 1281, 1282, 1283, 492, 2, 1241, 1242, 
    3, 1243, 1244, 1245, 485, 2, 1218, 1219, 3, 1220, 1221, 1222, 482, 2, 1208, 1209, 
    3, 1210, 1211, 1212, 471, 2, 1189, 1190, 3, 1191, 1192, 1193, 7, 2, 7, 8, 
    3, 9, 10, 11, 567, 2, 1440, 1441, 4, 1442, 1443, 1444, 1445, 10, 2, 12, 
    13, 4, 14, 15, 16, 17, 595, 2, 1554, 1555, 4, 1556, 1557, 1558, 1559, 590, 
    2, 1526, 1527, 6, 1528, 1529, 1530, 1531, 1532, 1533, 13, 2, 18, 19, 4, 20, 
    21, 22, 23, 600, 2, 1573, 1574, 2, 1575, 1576, 599, 2, 1569, 1570, 2, 1571, 
    1572, 568, 2, 1446, 1447, 2, 1448, 1449, 15, 2, 24, 25, 2, 26, 27, 572, 
    3, 1460, 1461, 1462, 1, 1463, 571, 3, 1456, 1457, 1458, 1, 1459, 570, 3, 1452, 
    1453, 1454, 1, 1455, 559, 3, 1397, 1398, 1399, 1, 1400, 481, 3, 1204, 1205, 1206, 
    1, 1207, 17, 3, 28, 29, 30, 1, 31, 598, 2, 1565, 1566, 2, 1567, 1568, 
    19, 2, 32, 33, 2, 34, 35, 597, 2, 1561, 1562, 2, 1563, 1564, 23, 2, 
    36, 37, 2, 38, 39, 26, 2, 40, 41, 6, 42, 43, 44, 45, 46, 47, 
    472, 1, 1194, 1, 1195, 29, 1, 48, 1, 49, 30, 2, 50, 51, 0, 594, 
    2, 1548, 1549, 4, 1550, 1551, 1552, 1553, 588, 2, 1518, 1519, 6, 1520, 1521, 1522, 
    1523, 1524, 1525, 48, 2, 52, 53, 4, 54, 55, 56, 57, 596, 1, 1560, 0, 
    591, 3, 1534, 1535, 1536, 0, 49, 1, 58, 0, 50, 2, 59, 60, 0, 51, 
    2, 61, 62, 5, 63, 64, 65, 66, 67, 52, 2, 68, 69, 3, 70, 71, 
    72, 476, 1, 1196, 0, 70, 4, 174, 175, 176, 177, 0, 71, 4, 178, 179, 
    180, 181, 0, 72, 4, 182, 183, 184, 185, 0, 79, 4, 186, 187, 188, 189, 
    0, 89, 0, 0, 95, 2, 190, 191, 0, 98, 3, 192, 193, 194, 2, 195, 
    196, 574, 3, 1469, 1470, 1471, 0, 100, 3, 197, 198, 199, 0, 577, 1, 1477, 
    0, 101, 1, 200, 0, 580, 1, 1483, 0, 102, 1, 201, 0, 110, 0, 10, 
    73, 1179, 1180, 1181, 1182, 1183, 1184, 1185, 1186, 1187, 111, 0, 14, 202, 203, 204, 
    205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 112, 0, 11, 216, 217, 
    218, 219, 220, 221, 222, 223, 224, 225, 226, 113, 0, 4, 227, 228, 229, 230, 
    114, 0, 4, 236, 237, 238, 239, 115, 4, 231, 232, 233, 234, 0, 116, 1, 
    235, 0, 117, 0, 0, 118, 3, 240, 241, 242, 0, 119, 3, 243, 244, 245, 
    0, 120, 0, 12, 246, 247, 250, 251, 252, 253, 254, 255, 259, 260, 261, 262, 
    121, 2, 248, 249, 0, 122, 3, 256, 257, 258, 0, 123, 0, 2, 263, 264, 
    124, 2, 265, 266, 0, 125, 0, 10, 267, 268, 269, 270, 271, 272, 273, 274, 
    275, 276, 126, 0, 0, 127, 0, 0, 477, 2, 1197, 1198, 2, 1199, 1200, 128, 
    2, 277, 278, 3, 279, 280, 284, 478, 1, 1201, 0, 129, 1, 281, 0, 479, 
    1, 1202, 0, 130, 1, 282, 0, 134, 1, 283, 0, 537, 2, 1342, 1343, 2, 
    1344, 1345, 535, 2, 1334, 1335, 2, 1336, 1337, 507, 2, 1291, 1292, 2, 1293, 1294, 
    497, 4, 1269, 1270, 1271, 1272, 2, 1273, 1274, 495, 4, 1255, 1256, 1257, 1258, 2, 
    1259, 1260, 490, 2, 1236, 1237, 2, 1238, 1239, 488, 2, 1228, 1229, 2, 1230, 1231, 
    483, 2, 1213, 1214, 2, 1215, 1216, 138, 2, 285, 286, 2, 287, 288, 534, 3, 
    1329, 1330, 1331, 2, 1332, 1333, 494, 7, 1246, 1247, 1248, 1249, 1250, 1251, 1252, 2, 
    1253, 1254, 487, 3, 1223, 1224, 1225, 2, 1226, 1227, 139, 3, 289, 290, 291, 2, 
    292, 293, 144, 2, 294, 295, 1, 296, 536, 3, 1338, 1339, 1340, 1, 1341, 496, 
    7, 1261, 1262, 1263, 1264, 1265, 1266, 1267, 1, 1268, 489, 3, 1232, 1233, 1234, 1, 
    1235, 145, 3, 297, 298, 299, 1, 300, 531, 1, 1323, 0, 504, 1, 1285, 0, 
    491, 1, 1240, 0, 484, 1, 1217, 0, 151, 1, 301, 0, 153, 0, 0, 156, 
    0, 72, 303, 304, 314, 315, 316, 317, 318, 319, 320, 321, 322, 323, 324, 325, 
    326, 327, 328, 329, 330, 335, 336, 339, 342, 345, 346, 349, 352, 353, 354, 355, 
    356, 357, 358, 359, 360, 361, 362, 363, 364, 365, 366, 367, 368, 369, 378, 379, 
    380, 381, 382, 383, 384, 385, 386, 387, 391, 396, 397, 398, 399, 302, 305, 306, 
    307, 308, 313, 1188, 395, 1275, 1276, 1277, 1278, 1284, 159, 2, 309, 310, 0, 160, 
    2, 311, 312, 0, 162, 1, 331, 1, 332, 163, 0, 2, 333, 334, 164, 2, 
    337, 338, 0, 165, 2, 340, 341, 0, 166, 2, 343, 344, 0, 167, 1, 347, 
    0, 480, 1, 1203, 0, 168, 1, 348, 0, 169, 1, 350, 0, 170, 1, 351, 
    0, 473, 0, 0, 171, 0, 0, 172, 0, 0, 173, 0, 0, 174, 1, 370, 
    0, 175, 1, 371, 1, 372, 474, 0, 0, 176, 0, 0, 475, 0, 0, 177, 
    0, 0, 178, 0, 1, 373, 179, 1, 374, 0, 180, 0, 0, 181, 1, 375, 
    0, 182, 0, 0, 183, 2, 376, 377, 0, 184, 0, 2, 388, 389, 185, 0, 
    0, 186, 1, 390, 0, 187, 0, 0, 188, 0, 0, 190, 3, 392, 393, 394, 
    0, 193, 0, 0, 194, 0, 0, 195, 0, 1, 400, 196, 0, 1, 401, 197, 
    0, 2, 402, 403, 198, 0, 0, 199, 0, 1, 404, 200, 1, 405, 0, 201, 
    0, 0, 202, 0, 12, 1295, 1298, 1300, 1302, 1303, 1312, 1313, 1322, 1346, 1347, 1348, 
    1349, 203, 0, 0, 511, 1, 1297, 0, 509, 1, 1296, 0, 205, 1, 407, 1, 
    406, 206, 0, 0, 208, 0, 0, 209, 0, 0, 514, 1, 1299, 0, 210, 1, 
    408, 0, 211, 0, 0, 516, 1, 1301, 0, 212, 1, 409, 0, 213, 0, 0, 
    214, 0, 1, 410, 215, 0, 9, 435, 436, 437, 438, 439, 411, 412, 413, 441, 
    218, 1, 414, 0, 230, 1, 432, 0, 219, 2, 415, 416, 0, 225, 2, 424, 
    425, 0, 231, 2, 433, 434, 0, 519, 2, 1304, 1305, 0, 220, 2, 417, 418, 
    0, 520, 2, 1306, 1307, 0, 226, 2, 426, 427, 0, 221, 2, 419, 420, 0, 
    521, 2, 1308, 1309, 0, 227, 2, 428, 429, 0, 222, 1, 421, 0, 228, 1, 
    430, 0, 223, 1, 422, 0, 229, 1, 431, 1, 1310, 224, 1, 423, 0, 523, 
    1, 1311, 0, 232, 1, 440, 0, 234, 1, 442, 0, 235, 1, 443, 0, 236, 
    0, 2, 444, 445, 237, 1, 446, 0, 238, 0, 8, 455, 456, 460, 461, 457, 
    458, 1320, 1321, 526, 2, 1314, 1315, 0, 239, 2, 447, 448, 0, 527, 2, 1316, 
    1317, 0, 240, 2, 449, 450, 0, 241, 1, 451, 1, 452, 528, 2, 1318, 1319, 
    0, 242, 2, 453, 454, 0, 244, 1, 459, 0, 245, 0, 0, 246, 1, 462, 
    0, 250, 0, 0, 251, 1, 463, 0, 252, 1, 464, 0, 253, 0, 2, 465, 
    466, 254, 0, 1, 467, 257, 0, 3, 468, 469, 470, 258, 1, 471, 0, 259, 
    0, 0, 260, 0, 1, 472, 261, 1, 473, 0, 560, 4, 1401, 1402, 1403, 1404, 
    4, 1405, 1406, 1407, 1408, 549, 4, 1366, 1367, 1368, 1369, 4, 1370, 1371, 1372, 1373, 
    263, 4, 474, 475, 476, 477, 4, 478, 479, 485, 486, 561, 3, 1409, 1410, 1411, 
    1, 1412, 556, 4, 1385, 1386, 1387, 1388, 1, 1389, 264, 4, 480, 481, 482, 483, 
    1, 484, 545, 5, 1361, 1362, 1363, 1364, 1365, 0, 265, 5, 487, 488, 489, 490, 
    491, 0, 550, 1, 1374, 0, 266, 1, 492, 0, 552, 2, 1375, 1376, 0, 267, 
    2, 493, 494, 0, 555, 1, 1383, 1, 1384, 268, 1, 495, 1, 496, 557, 2, 
    1390, 1391, 0, 269, 2, 497, 498, 0, 270, 7, 499, 500, 501, 502, 503, 504, 
    505, 1, 506, 544, 1, 1360, 0, 271, 1, 507, 0, 554, 2, 1377, 1378, 4, 
    1379, 1380, 1381, 1382, 272, 2, 508, 509, 4, 510, 511, 512, 513, 273, 2, 514, 
    515, 4, 516, 517, 518, 519, 274, 2, 520, 521, 0, 277, 4, 522, 523, 524, 
    525, 2, 526, 527, 278, 1, 528, 0, 279, 2, 529, 530, 3, 531, 532, 533, 
    280, 1, 534, 0, 281, 1, 535, 0, 282, 0, 12, 536, 537, 538, 539, 540, 
    541, 542, 543, 544, 548, 553, 554, 283, 1, 545, 0, 284, 1, 546, 1, 547, 
    285, 2, 549, 550, 0, 286, 2, 551, 552, 0, 287, 1, 555, 0, 288, 2, 
    556, 557, 0, 289, 0, 2, 582, 583, 290, 2, 558, 559, 0, 292, 2, 562, 
    563, 0, 294, 2, 566, 567, 0, 296, 2, 570, 571, 0, 298, 2, 574, 575, 
    0, 300, 2, 578, 579, 0, 291, 2, 560, 561, 0, 293, 2, 564, 565, 0, 
    295, 2, 568, 569, 0, 297, 2, 572, 573, 0, 299, 2, 576, 577, 0, 301, 
    2, 580, 581, 0, 302, 1, 584, 0, 303, 1, 585, 0, 304, 1, 586, 0, 
    305, 0, 1, 587, 306, 2, 588, 589, 6, 590, 591, 592, 593, 594, 601, 308, 
    2, 595, 596, 0, 309, 2, 597, 598, 0, 310, 2, 599, 600, 0, 311, 2, 
    602, 603, 0, 312, 2, 604, 605, 0, 313, 1, 606, 0, 314, 0, 4, 640, 
    641, 642, 643, 315, 1, 607, 0, 318, 1, 610, 1, 611, 316, 1, 608, 0, 
    320, 1, 614, 0, 317, 1, 609, 0, 323, 1, 619, 0, 319, 2, 612, 613, 
    0, 321, 2, 615, 616, 0, 322, 2, 617, 618, 0, 324, 1, 620, 0, 325, 
    2, 621, 622, 0, 326, 1, 623, 6, 624, 625, 626, 627, 628, 639, 327, 1, 
    629, 0, 328, 0, 2, 630, 632, 329, 1, 631, 0, 330, 0, 0, 569, 1, 
    1450, 1, 1451, 331, 1, 633, 1, 634, 332, 0, 3, 635, 637, 638, 333, 1, 
    636, 0, 334, 5, 644, 645, 646, 647, 648, 6, 649, 652, 653, 661, 662, 663, 
    335, 2, 650, 651, 0, 336, 3, 654, 655, 656, 6, 657, 658, 659, 660, 664, 
    665, 337, 0, 0, 339, 0, 14, 666, 667, 668, 669, 670, 671, 672, 673, 674, 
    684, 690, 691, 692, 693, 340, 1, 675, 0, 341, 1, 676, 1, 677, 583, 1, 
    1494, 2, 1495, 1496, 342, 1, 678, 2, 679, 680, 343, 3, 681, 682, 683, 0, 
    344, 3, 685, 686, 687, 2, 688, 689, 345, 0, 0, 346, 0, 0, 347, 3, 
    694, 695, 696, 5, 697, 698, 701, 702, 703, 348, 1, 699, 1, 700, 349, 3, 
    704, 705, 706, 0, 350, 0, 8, 707, 708, 721, 722, 723, 724, 725, 726, 351, 
    4, 709, 710, 711, 712, 8, 713, 714, 715, 716, 717, 718, 719, 720, 355, 0, 
    41, 727, 728, 729, 743, 744, 747, 748, 749, 750, 751, 752, 753, 754, 761, 762, 
    763, 764, 765, 770, 771, 772, 773, 774, 775, 776, 778, 783, 784, 785, 786, 787, 
    788, 789, 790, 791, 792, 793, 794, 795, 796, 797, 356, 0, 13, 730, 731, 732, 
    733, 734, 735, 736, 737, 738, 739, 740, 741, 742, 358, 2, 745, 746, 0, 359, 
    6, 755, 756, 757, 758, 759, 760, 0, 361, 2, 766, 767, 0, 362, 2, 768, 
    769, 0, 363, 1, 777, 0, 364, 2, 779, 780, 0, 365, 1, 781, 0, 366, 
    1, 782, 0, 370, 0, 32, 944, 945, 946, 1015, 1016, 1017, 1018, 1019, 1020, 1021, 
    1022, 1023, 1024, 1025, 1026, 1027, 1028, 1029, 1030, 1031, 1032, 1033, 1034, 1035, 1036, 1037, 
    1038, 1039, 1040, 1041, 1042, 1043, 371, 7, 930, 931, 932, 933, 934, 935, 936, 7, 
    937, 938, 939, 940, 941, 942, 943, 372, 7, 947, 948, 949, 950, 951, 952, 953, 
    61, 954, 955, 956, 957, 958, 959, 960, 961, 962, 963, 964, 965, 966, 967, 968, 
    969, 970, 971, 972, 973, 974, 975, 976, 977, 978, 979, 980, 981, 982, 983, 984, 
    985, 986, 987, 988, 989, 990, 991, 992, 993, 994, 995, 996, 997, 998, 999, 1000, 
    1001, 1002, 1003, 1004, 1005, 1006, 1007, 1008, 1009, 1010, 1011, 1012, 1013, 1014, 373, 0, 
    0, 375, 1, 1044, 0, 376, 3, 1045, 1046, 1047, 0, 377, 0, 2, 1048, 1049, 
    378, 0, 4, 1050, 1051, 1052, 1053, 379, 0, 2, 1599, 1600, 380, 0, 2, 1054, 
    1055, 381, 0, 2, 1056, 1057, 382, 0, 1, 1601, 383, 0, 2, 1058, 1059, 384, 
    8, 1060, 1061, 1062, 1063, 1064, 1065, 1066, 1067, 0, 385, 5, 1068, 1069, 1070, 1071, 
    1072, 0, 396, 2, 1085, 1086, 0, 428, 2, 1106, 1107, 0, 446, 8, 1120, 1121, 
    1122, 1123, 1124, 1125, 1126, 1127, 0, 447, 6, 1128, 1129, 1130, 1131, 1132, 1133, 0, 
    386, 4, 1073, 1074, 1075, 1076, 0, 609, 1, 1604, 0, 421, 1, 1095, 0, 394, 
    2, 1081, 1082, 0, 426, 2, 1102, 1103, 0, 395, 2, 1083, 1084, 0, 427, 2, 
    1104, 1105, 0, 397, 2, 1087, 1088, 0, 429, 2, 1108, 1109, 0, 404, 2, 1089, 
    1090, 0, 608, 2, 1602, 1603, 0, 414, 2, 1091, 1092, 0, 436, 2, 1110, 1111, 
    0, 415, 2, 1093, 1094, 0, 422, 1, 1096, 0, 423, 1, 1097, 0, 424, 2, 
    1098, 1099, 0, 425, 2, 1100, 1101, 0, 445, 0, 8, 1112, 1113, 1114, 1115, 1116, 
    1117, 1118, 1119, 448, 4, 1134, 1135, 1136, 1137, 0, 449, 4, 1138, 1139, 1140, 1141, 
    0, 450, 4, 1142, 1143, 1144, 1145, 0, 451, 4, 1146, 1147, 1148, 1149, 0, 452, 
    0, 1, 1605, 453, 0, 10, 1154, 1155, 1156, 1157, 1158, 1159, 1160, 1161, 1162, 1163, 
    454, 0, 9, 1164, 1165, 1167, 1169, 1171, 1173, 1174, 1176, 1177, 455, 1, 1166, 0, 
    456, 1, 1168, 0, 457, 1, 1170, 0, 458, 1, 1172, 0, 459, 1, 1175, 0, 
    460, 1, 1178, 0,     0
};

static const int type_table[] = {
  1, -1, 2, -3, -4, -7, -6, -1, 0, -1, 2, -1, 3, -1, 7, -1,
  9, -1, 4, 3, 2, 3, 1, 3, 162, 4, 1, 2, 0, -1, -1, 0,
  -1, 0, 0, -1, 0, -1, 25, 3, 310, 3, 49, 3, 329, 3, 30, 3,
  325, -2, 50, 8, 41, 79, -1, 2, -1, 0, 2, -1, 0, 2, -1, 0,
  109, -1, 0, 111, -1, 0, 144, -1, 0, 146, -1, 0, 146, -1, 0, -2,
  51, 8, 41, -1, -1, 2, -1, 0, 2, -1, 0, 2, -1, 0, 109, -1,
  0, 111, -1, 0, 144, -1, 0, 146, -1, 0, 146, -1, 0, 0, 49, -2,
  43, 7, 29, -1, -1, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1,
  0, 0, -1, 0, 0, -1, 0, 0, -1, 0, -2, 40, 0, 27, -1, -1,
  0, 138, 1, 0, -1, 49, 3, 280, 3, 313, 3, 164, 3, 166, 3, 165,
  3, 111, 4, 0, 2, 0, -1, -1, 2, -1, 0, 2, -1, 0, 4, 2,
  3, 0, 162, -1, 2, -1, 0, 2, -1, 0, 2, -1, 0, 4, 4, 4,
  0, 174, -1, 2, -1, 0, 2, -1, 0, 2, -1, 0, 2, -1, 0, 4,
  3, 3, 0, 25, -1, 0, -1, 0, 0, -1, 0, 0, -1, 0, 4, 5,
  4, 0, 207, -1, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0,
  -1, 207, 4, 67, 4, 0, 189, -1, 2, -1, 0, 2, -1, 0, 2, -1,
  0, 2, -1, 0, 3, 358, 3, 307, 4, 7, 3, 0, -1, -1, 0, 279,
  0, 0, 279, 0, 0, -1, 0, 4, 6, 2, 0, -1, -1, 0, -1, 0,
  0, -1, 0, -2, 8, 3, 0, -1, -1, 0, 279, 0, 0, 279, 0, 0,
  -1, 0, -1, 13, -1, 37, -1, 308, -1, 264, -1, 312, -1, 314, -2, 9,
  5, 0, -1, -1, 0, 207, 0, 0, 207, 0, 0, 207, 0, 0, 25, 0,
  0, 25, 0, -5, 10, 5, 0, -1, -1, 0, 207, 0, 0, 207, 0, 0,
  207, 0, 0, 25, 0, 0, 25, 0, -5, 11, 2, 3, -1, -1, 3, -1,
  0, 3, -1, 0, -2, 12, 1, 0, -1, -1, 0, -1, 0, -2, 13, 1,
  1, 372, -1, 0, -1, 0, -5, 14, 2, 3, -1, -1, 3, -1, 0, 7,
  -1, 0, -1, 390, -5, 15, 3, 7, -1, -1, 3, -1, 0, 3, 390, 0,
  7, 390, 0, -1, 404, -5, 16, 2, 1, -1, -1, 3, -1, 0, 0, -1,
  0, 4, 18, 5, 0, -1, -1, 2, -1, 0, 2, -1, 0, 2, -1, 0,
  2, -1, 0, 2, -1, 0, 3, 182, 3, 135, 0, 3, -2, 74, 1, 82,
  -1, -1, 486, -1, 0, -2, 75, 3, 82, -1, -1, 484, -1, 0, 2, -1,
  0, 2, -1, 0, 0, 469, -1, 484, -2, 82, 1, 82, -1, -1, 542, -1,
  0, -2, 83, 3, 82, -1, -1, 512, -1, 0, 3, -1, 0, 514, -1, 0,
  0, 497, -2, 80, 1, 82, -1, -1, 540, -1, 0, -2, 81, 3, 82, -1,
  -1, 538, -1, 0, 3, -1, 0, 0, -1, 0, 0, 523, -1, 538, -1, 512,
  -2, 20, 1, 2, -1, -1, 0, -1, 1, -2, 21, 1, 2, -1, -1, 0,
  -1, 0, -2, 22, 2, 3, 553, 0, 0, -1, 0, 0, -1, 0, -2, 23,
  11, 4, -1, 123, 146, -1, 0, 0, -1, 0, 2, -1, 0, 2, 174, 0,
  2, 174, 0, 2, 174, 0, 3, -1, 0, 7, -1, 0, 458, -1, 0, 0,
  -1, 0, 553, -1, 0, 0, 574, 3, 169, 3, 170, -2, 24, 2, 4, -1,
  -1, 3, -1, 0, 0, -1, 0, -2, 76, 1, 82, -1, -1, 657, -1, 0,
  -2, 77, 3, 82, -1, -1, 655, -1, 0, 3, -1, 0, 458, -1, 0, 0,
  640, -1, 655, -2, 78, 1, 82, -1, -1, 697, -1, 0, -2, 79, 7, 82,
  -1, -1, 695, -1, 0, 0, 207, 0, 0, 207, 0, 0, 207, 0, 0, 207,
  0, 0, 207, 0, 0, 207, 0, 0, 668, -1, 695, -5, 25, 2, 3, -1,
  -1, 3, -1, 0, 11, -1, 0, -2, 26, 4, 4, -1, -1, 2, 162, 0,
  2, 162, 0, 3, -1, 0, 7, -1, 0, 3, 198, 3, 206, 3, 207, 4,
  29, 1, 0, -1, -1, 0, -1, 0, 4, 30, 2, 0, 735, -1, 0, -1,
  0, 0, -1, 0, -2, 31, 0, 4, -1, -1, -2, 32, 1, 11, 756, -1,
  0, -1, 0, -2, 33, 2, 19, 762, -1, 0, -1, 0, 0, -1, 0, -2,
  84, 2, 82, 762, -1, 0, -1, 0, 3, -1, 0, -2, 85, 1, 90, -1,
  -1, 0, -1, 0, -2, 34, 5, 27, -1, -1, 7, -1, 0, 825, -1, 0,
  827, -1, 0, 825, -1, 0, 850, -1, 0, 0, 804, -1, 804, -2, 35, 5,
  27, -1, -1, 11, -1, 0, 850, -1, 0, 852, -1, 0, 825, -1, 0, 850,
  -1, 0, 0, 829, -1, 829, -2, 87, 1, 90, -1, -1, 514, -1, 0, -2,
  88, 1, 90, -1, -1, 514, -1, 0, -2, 36, 1, 27, -1, -1, 0, -1,
  1, -1, 872, -2, 37, 2, 27, 872, -1, 0, -1, 1, 0, -1, 2, -1,
  883, -2, 39, 4, 27, -1, -1, 915, -1, 0, 0, -1, 1, 0, 25, 0,
  0, 25, 0, 0, 897, -1, 897, -2, 41, 12, 29, 961, -1, 2, -1, 0,
  2, -1, 0, 2, -1, 0, 997, -1, 0, 0, 25, 0, 0, 25, 0, 0,
  25, 0, 0, 25, 0, 146, -1, 0, 146, -1, 0, 0, -1, 0, 0, -1,
  0, -2, 42, 10, 29, -1, -1, 2, -1, 0, 2, -1, 0, 2, -1, 0,
  997, -1, 0, 0, 25, 0, 0, 25, 0, 0, 25, 0, 0, 25, 0, 146,
  -1, 0, 146, -1, 0, 0, 919, 3, 266, 3, 267, 3, 269, -1, 919, 3,
  286, 3, 273, 3, 272, 3, 264, 3, 285, 3, 268, -1, 138, 3, 302, 3,
  303, 3, 551, -1, 1005, -2, 44, 0, 29, 138, -1, -2, 45, 0, 31, 138,
  -1, -2, 46, 0, 33, 138, -1, -2, 47, 0, 35, 138, -1, -2, 48, 0,
  37, 138, -1, -2, 49, 0, 39, 138, -1, -2, 53, 1, 48, 1074, -1, 0,
  -1, 0, -2, 52, 0, 41, -1, -1, -2, 54, 2, 55, 1074, -1, 3, -1,
  0, 1092, -1, 0, -1, 1074, 0, 1074, 3, 338, -2, 55, 5, 62, -1, -1,
  0, -1, 0, 0, 25, 0, 0, 25, 0, 0, 25, 0, 0, 25, 0, -1,
  1098, -1, 1119, -2, 56, 3, 62, -1, -1, 2, -1, 0, 2, -1, 0, 2,
  -1, 0, -1, 1123, -1, 1138, 3, 347, 3, 345, 3, 348, -1, 162, 4, 59,
  3, 0, -1, -1, 0, -1, 0, 0, -1, 0, 2, -1, 0, -1, 1150, 4,
  60, 4, 0, -1, -1, 2, -1, 0, 2, -1, 0, 2, -1, 0, 2, -1,
  0, -2, 62, 11, 62, -1, -1, 2, -1, 0, 2, 174, 0, 2, 174, 0,
  2, 174, 0, 2, 174, 0, 2, 174, 0, 2, 174, 0, 2, 174, 0, 2,
  174, 0, 2, 174, 0, 1224, -1, 0, 1, 1, 4, 61, 6, 0, -1, -1,
  2, 174, 0, 2, 174, 0, 2, 174, 0, 2, 174, 0, 2, 174, 0, 2,
  174, 0, -1, 1185, -1, 174, -1, 1252, -1, 189, -1, 1256, -2, 63, 7, 62,
  -1, -1, 1287, -1, 0, 3, -1, 0, 2, 162, 0, 2, 162, 0, 1289, -1,
  0, 11, -1, 0, 13, -1, 0, -1, 1260, 0, 1260, 4, 64, 4, 0, 189,
  -1, 2, -1, 0, 2, -1, 0, 2, -1, 0, 2, -1, 0, -2, 65, 1,
  62, -1, -1, 0, -1, 0, 4, 66, 1, 0, -1, -1, 0, -1, 0, -2,
  68, 1, 62, -1, -1, 0, -1, 0, -1, 1327, -1, 146, 3, 112, 3, 113,
  3, 114, 3, 117, 3, 120, 3, 123, 3, 125, 3, 126, 3, 127, 3, 163,
  3, 167, 3, 178, 3, 180, 3, 181, 3, 183, 3, 152, 3, 486, 3, 195,
  3, 196, 3, 197, 3, 199, 3, 200, 3, 201, 3, 493, 3, 203, 3, 208,
  3, 510, 3, 209, 3, 211, 3, 214, 3, 215, -2, 86, 1, 90, -1, -1,
  3, -1, 0, 3, 236, 3, 238, 3, 253, 3, 506, 3, 254, 3, 257, 3,
  259, 3, 260, 3, 283, 3, 284, 3, 274, 3, 546, 3, 547, 3, 548, 3,
  553, 3, 287, 3, 288, 3, 278, 3, 279, 3, 281, 3, 304, 3, 101, 3,
  102, 3, 308, 3, 311, 3, 312, 3, 333, 3, 319, 3, 321, 3, 322, 3,
  324, 3, 335, 3, 575, 3, 578, 3, 340, 3, 341, 3, 349, 3, 353, 3,
  354, 3, 352, 3, 357, 3, 359, 3, 589, 3, 360, 3, 361, 3, 362, 3,
  363, 3, 364, 3, 365, 3, 366, 3, 373, 3, 374, 3, 375, 3, 376, 3,
  380, 3, 381, 3, 383, 3, 453, 3, 455, 3, 456, 3, 457, 3, 458, 3,
  459, 3, 460, 4, 17, 1, 0, -1, -1, 0, -1, 0, 4, 19, 5, 0,
  -1, -1, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1,
  0, -2, 27, 1, 4, -1, -1, 0, -1, 0, -2, 28, 2, 4, 1569, -1,
  0, -1, 0, 0, -1, 0, -2, 38, 1, 27, -1, 1, 0, -1, 2, -2,
  57, 2, 62, -1, -1, 1148, -1, 0, 7, -1, 0, -2, 58, 2, 62, -1,
  -1, 1599, -1, 0, 1599, -1, 0, -2, 69, 2, 66, 1327, -1, 0, -1, 0,
  0, -1, 1, -2, 71, 3, 70, 1650, -1, 0, -1, 0, 0, -1, 2, 0,
  -1, 3, -2, 70, 2, 70, 1327, -1, 0, -1, 0, 0, -1, 2, -2, 72,
  3, 74, 1650, -1, 0, -1, 0, 0, -1, 2, 0, -1, 4, -2, 73, 4,
  78, 1662, -1, 0, -1, 0, 0, -1, 2, 0, -1, 4, 0, -1, 5, -1,
  360, -1, 5,
};

static const string_view stringtable[] = {
    string_view("example", 7),
    string_view("struct", 6),
    string_view("misc", 4),
    string_view("types", 5),
    string_view("astar", 5),
    string_view("goap", 4),
    string_view("knightstour", 11),
    string_view("parsereval", 10),
    string_view("flood", 5),
    string_view("water", 5),
    string_view("gradient", 8),
    string_view("springs", 7),
    string_view("smallpt", 7),
    string_view("string", 6),
    string_view("misch", 5),
    string_view("lifetime", 8),
    string_view("builtins", 8),
    string_view("operators", 9),
    string_view("matrix", 6),
    string_view("typeswitch", 10),
    string_view("a", 1),
    string_view("b", 1),
    string_view("ab", 2),
    string_view("(== (lifetime change<0|1> (+ \"a\":string \"b\":string):string):string \"ab\":string)", 79),
    string_view("testing: ", 9),
    string_view("(length xs:[int])", 17),
    string_view("(length xs:[string])", 20),
    string_view("(== (+ (+ (.x (.a b:B):A):int (.y (.a b:B):A):int):int (.b b:B):int):int 18:int)", 80),
    string_view("(== (+ (.x aa:int2):int (.y aa:int2):int):int 14:int)", 53),
    string_view("(== (+ aa:int2 1:int):int2 (object_constructor 7:int 9:int):int2)", 65),
    string_view("(== a:int2 (object_constructor 6:int 8:int):int2)", 49),
    string_view("(== aa:int2 (object_constructor 7:int 9:int):int2)", 50),
    string_view("(== (+ (+ (.x (.a c:C):A):int (.y (.a c:C):A):int):int (.b c:C):int):int 33:int)", 80),
    string_view("(== (.a c:C):A (object_constructor 11:int 12:int):A)", 52),
    string_view("(== (.a c:C):A (object_constructor 12:int 14:int):A)", 52),
    string_view("(== (.a c:C):A (object_constructor 3:int 4:int):A)", 50),
    string_view("(== (int (object_constructor 1.0:float 2.0:float):float2):int2 (object_constructor 1:int 2:int):int2)", 101),
    string_view("(== (min (object_constructor 1:int 2:int):int2):int 1:int)", 58),
    string_view("(== (manhattan (object_constructor 2:int 3:int):int2):int 5:int)", 64),
    string_view("(< (abs (- (magnitude nv:float2):float 1.0:float):float):float 0.001:float)", 75),
    string_view("(< (abs (magnitude nv:float2):float):float 0.001:float)", 55),
    string_view("(== (indexing operation v:[int2] 1:int):int2 (object_constructor 3:int 4:int):int2)", 83),
    string_view("(== (indexing operation v:[int2] 1:int):int2 (object_constructor 5:int 6:int):int2)", 83),
    string_view("(== (.y (indexing operation v:[int2] 1:int):int2):int 6:int)", 60),
    string_view("(== (indexing operation v:[int2] 1:int):int2 (object_constructor 6:int 7:int):int2)", 83),
    string_view("(== (indexing operation v:[int2] 1:int):int2 (object_constructor 7:int 8:int):int2)", 83),
    string_view("(== (indexing operation tri:[[[int]]] (object_constructor 0:int 0:int 0:int):int3):int 123:int)", 95),
    string_view("(== (indexing operation tri:[[[int]]] (object_constructor 0:int 0:int 0:int):int3):int 124:int)", 95),
    string_view("(== (indexing operation triv:[[[int2]]] (object_constructor 0:int 0:int 0:int):int3):int2 (object_constructor 10:int 11:int):int2)", 130),
    string_view("(== (indexing operation triv:[[[int2]]] (object_constructor 0:int 0:int 0:int):int3):int2 (object_constructor 14:int 15:int):int2)", 130),
    string_view("(== (.a (indexing operation trivv:[[[B]]] (object_constructor 0:int 0:int 0:int):int3):B):A (object_constructor 5:int 6:int):A)", 127),
    string_view("(== (.y (.a (indexing operation trivv:[[[B]]] (object_constructor 0:int 0:int 0:int):int3):B):A):int 6:int)", 107),
    string_view("(== (.y (.a (indexing operation (indexing operation trivv:[[[B]]] (object_constructor 0:int 0:int):int2):[B] 0:int):B):A):int 6:int)", 132),
    string_view("(== (indexing operation (object_constructor 5:int 6:int 7:int):int3 1:int):int 6:int)", 85),
    string_view("(== a:int2 int2_y:int2)", 23),
    string_view("(== b:int2 int2_0:int2)", 23),
    string_view("(== (.f c:C):int2 int2_y:int2)", 30),
    string_view("(== (.f s:S):int2 int2_y:int2)", 30),
    string_view("(== (.f s:S):int2 int2_x:int2)", 30),
    string_view("(== (indexing operation v:[int2] 0:int):int2 int2_y:int2)", 57),
    string_view("(== (.y (if (manhattan (object_constructor 1:int 1:int):int2):int (block (object_constructor 1:int 2:int):int2):int2 (block (object_constructor 3:int 4:int):int2):int2):int2):int 2:int)", 185),
    string_view("(== (.y (block (object_constructor 1:int 2:int):int2):int2):int 2:int)", 70),
    string_view("(== (object_constructor 1:int 2:int):int2 (switch (manhattan (object_constructor 1:int 1:int):int2):int (list (case (list 2:int):void (block (object_constructor 1:int 2:int):int2):int2):int2 (case list:void (block (object_constructor 3:int 4:int):int2):int2):int2):void):int2)", 276),
    string_view("(== (+ (.x st:int2):int (.y st:int2):int):int 3:int)", 52),
    string_view("a", 1),
    string_view("bb", 2),
    string_view("ab", 2),
    string_view("b", 1),
    string_view("abb", 3),
    string_view("(== (lifetime change<0|1> (+ (.x st:xy_s):string (.y st:xy_s):string):string):string \"abb\":string)", 98),
    string_view("(== (f 1:int (object_constructor 2:int 3:int):int2 4:int):int 10:int)", 69),
    string_view("(and (and (== mr2:int2 (object_constructor 7:int 7:int):int2):bool (== mr1:int 1:int):bool):bool (== mr3:int 1:int):bool)", 121),
    string_view("(== (mm (object_constructor 1:int):mm1 (object_constructor 4:int 5:int):int2):int 10:int)", 89),
    string_view("(== (mm (object_constructor 0:int):mm2 (object_constructor 4:int 5:int):int2):int 9:int)", 88),
    string_view("hello", 5),
    string_view("hellohello", 10),
    string_view("(== (.a fr:R):string \"hellohello\":string)", 41),
    string_view("one", 3),
    string_view("two", 3),
    string_view("twotwo", 6),
    string_view("(== (.a fr:R):string \"twotwo\":string)", 37),
    string_view("A", 1),
    string_view("B", 1),
    string_view("AA", 2),
    string_view("(and (== (.a frn:RN):string \"AA\":string):bool (== (indexing operation (.b (.r frn:RN):R):[int] 1:int):int 64:int):bool)", 119),
    string_view("AA", 2),
    string_view("one", 3),
    string_view("BB", 2),
    string_view("two", 3),
    string_view("BBBB", 4),
    string_view("(and (== (.a frn:RN):string \"BBBB\":string):bool (== (indexing operation (.b (.r frn:RN):R):[int] 1:int):int 2:int):bool)", 120),
    string_view("hi", 2),
    string_view("hihi", 4),
    string_view("(and (== (.a rs:RS):string \"hihi\":string):bool (== (.b rs:RS):int 2:int):bool)", 78),
    string_view("RS{\"hihi\", 2}", 13),
    string_view("(== (lifetime change<0|1> (string (tostring rs:RS):string):string):string \"RS{\\\"hihi\\\", 2}\":string)", 99),
    string_view("(== (+ (object_constructor 1:int):V1 (object_constructor 1:int):V1):V1 (object_constructor 2:int):V1)", 101),
    string_view("(== (min (object_constructor 2:int):V1 (max (object_constructor 1:int):V1 (object_constructor -1:int):V1):V1):V1 (object_constructor 1:int):V1)", 143),
    string_view("(== (manhattan (object_constructor 2:int):V1):int 2:int)", 56),
    string_view("(== v5test2:V5 (* v5test:V5 2.0:float):V5)", 42),
    string_view("(== (magnitude (normalize v5test2:V5):V5):float 1.0:float)", 58),
    string_view("(== (min v5test:V5 (max v5test:V5 v5test2:V5):V5):V5 v5test:V5)", 63),
    string_view("(== (dot v5test:V5 v5test2:V5):float 110.0:float)", 49),
    string_view("(== (* (object_constructor 2:int 3:int):int2 5:int):int2 (object_constructor 10:int 15:int):int2)", 97),
    string_view("(== (* 5:int (object_constructor 2:int 3:int):int2):int2 (object_constructor 10:int 15:int):int2)", 97),
    string_view("(== (* (object_constructor 2:int 3:int 4:int):int3 5:int):int3 (object_constructor 10:int 15:int 20:int):int3)", 110),
    string_view("(== (* 5:int (object_constructor 2:int 3:int 4:int):int3):int3 (object_constructor 10:int 15:int 20:int):int3)", 110),
    string_view("(== (* (object_constructor 2.1:float 3.2:float):float2 5.0:float):float2 (object_constructor 10.5:float 16.0:float):float2)", 123),
    string_view("(== (* 5.0:float (object_constructor 2.1:float 3.2:float):float2):float2 (object_constructor 10.5:float 16.0:float):float2)", 123),
    string_view("(== (* (object_constructor 2:int 3:int 4:int 5:int 6:int):A 5:int):A (object_constructor 10:int 15:int 20:int 25:int 30:int):A)", 127),
    string_view("(== (* 5:int (object_constructor 2:int 3:int 4:int 5:int 6:int):A):A (object_constructor 10:int 15:int 20:int 25:int 30:int):A)", 127),
    string_view("(== (+ 5:int (object_constructor 2:int 3:int):int2):int2 (object_constructor 7:int 8:int):int2)", 95),
    string_view("(== (+ (object_constructor 2:int 3:int):int2 5:int):int2 (object_constructor 7:int 8:int):int2)", 95),
    string_view("(== (+ (object_constructor 2:int 3:int 4:int):int3 5:int):int3 (object_constructor 7:int 8:int 9:int):int3)", 107),
    string_view("(== (+ 5:int (object_constructor 2:int 3:int 4:int):int3):int3 (object_constructor 7:int 8:int 9:int):int3)", 107),
    string_view("(== (- 5:int (object_constructor 2:int 3:int):int2):int2 (object_constructor 3:int 2:int):int2)", 95),
    string_view("(== (- (object_constructor 2:int 3:int):int2 5:int):int2 (object_constructor -3:int -2:int):int2)", 97),
    string_view("(== (- 5:int (object_constructor 2:int 3:int 4:int):int3):int3 (object_constructor 3:int 2:int 1:int):int3)", 107),
    string_view("(== (- (object_constructor 2:int 3:int 4:int):int3 5:int):int3 (object_constructor -3:int -2:int -1:int):int3)", 110),
    string_view("(== (/ 10:int (object_constructor 5:int 10:int):int2):int2 (object_constructor 2:int 1:int):int2)", 97),
    string_view("(== (/ (object_constructor 10:int 20:int):int2 5:int):int2 (object_constructor 2:int 4:int):int2)", 97),
    string_view("(== (/ 10:int (object_constructor 5:int 10:int 1:int):int3):int3 (object_constructor 2:int 1:int 10:int):int3)", 110),
    string_view("(== (/ (object_constructor 10:int 20:int 25:int):int3 5:int):int3 (object_constructor 2:int 4:int 5:int):int3)", 110),
    string_view("(== (% 10:int (object_constructor 2:int 3:int):int2):int2 (object_constructor 0:int 1:int):int2)", 96),
    string_view("(== (% (object_constructor 10:int 21:int):int2 4:int):int2 (object_constructor 2:int 1:int):int2)", 97),
    string_view("(== (% 10:int (object_constructor 2:int 3:int 4:int):int3):int3 (object_constructor 0:int 1:int 2:int):int3)", 108),
    string_view("(== (% (object_constructor 10:int 21:int 24:int):int3 4:int):int3 (object_constructor 2:int 1:int 0:int):int3)", 110),
    string_view("(== (< (object_constructor 2:int 5:int):int2 5:int):int2 (object_constructor 1:int 0:int):int2)", 95),
    string_view("(== (< 5:int (object_constructor 2:int 5:int):int2):int2 (object_constructor 0:int 0:int):int2)", 95),
    string_view("(== (< (object_constructor 2:int 6:int):int2 5:int):int2 (object_constructor 1:int 0:int):int2)", 95),
    string_view("(== (< 5:int (object_constructor 2:int 6:int):int2):int2 (object_constructor 0:int 1:int):int2)", 95),
    string_view("(== (< (object_constructor 2:int 5:int 6:int):int3 5:int):int3 (object_constructor 1:int 0:int 0:int):int3)", 107),
    string_view("(== (< 5:int (object_constructor 2:int 5:int 6:int):int3):int3 (object_constructor 0:int 0:int 1:int):int3)", 107),
    string_view("(== (<= (object_constructor 2:int 5:int):int2 5:int):int2 (object_constructor 1:int 1:int):int2)", 96),
    string_view("(== (<= 5:int (object_constructor 2:int 5:int):int2):int2 (object_constructor 0:int 1:int):int2)", 96),
    string_view("(== (<= (object_constructor 2:int 6:int):int2 5:int):int2 (object_constructor 1:int 0:int):int2)", 96),
    string_view("(== (<= 5:int (object_constructor 2:int 6:int):int2):int2 (object_constructor 0:int 1:int):int2)", 96),
    string_view("(== (<= (object_constructor 2:int 5:int 6:int):int3 5:int):int3 (object_constructor 1:int 1:int 0:int):int3)", 108),
    string_view("(== (<= 5:int (object_constructor 2:int 5:int 6:int):int3):int3 (object_constructor 0:int 1:int 1:int):int3)", 108),
    string_view("(== (> (object_constructor 2:int 5:int):int2 5:int):int2 (object_constructor 0:int 0:int):int2)", 95),
    string_view("(== (> 5:int (object_constructor 2:int 5:int):int2):int2 (object_constructor 1:int 0:int):int2)", 95),
    string_view("(== (> (object_constructor 2:int 6:int):int2 5:int):int2 (object_constructor 0:int 1:int):int2)", 95),
    string_view("(== (> 5:int (object_constructor 2:int 6:int):int2):int2 (object_constructor 1:int 0:int):int2)", 95),
    string_view("(== (> (object_constructor 2:int 5:int 6:int):int3 5:int):int3 (object_constructor 0:int 0:int 1:int):int3)", 107),
    string_view("(== (> 5:int (object_constructor 2:int 5:int 6:int):int3):int3 (object_constructor 1:int 0:int 0:int):int3)", 107),
    string_view("(== (>= (object_constructor 2:int 5:int):int2 5:int):int2 (object_constructor 0:int 1:int):int2)", 96),
    string_view("(== (>= 5:int (object_constructor 2:int 5:int):int2):int2 (object_constructor 1:int 1:int):int2)", 96),
    string_view("(== (>= (object_constructor 2:int 6:int):int2 5:int):int2 (object_constructor 0:int 1:int):int2)", 96),
    string_view("(== (>= 5:int (object_constructor 2:int 6:int):int2):int2 (object_constructor 1:int 0:int):int2)", 96),
    string_view("(== (>= (object_constructor 2:int 5:int 6:int):int3 5:int):int3 (object_constructor 0:int 1:int 1:int):int3)", 108),
    string_view("(== (>= 5:int (object_constructor 2:int 5:int 6:int):int3):int3 (object_constructor 1:int 1:int 0:int):int3)", 108),
    string_view("(equal (lifetime change<0|1> (vector_constructor (object_constructor 1:int 2:int):int2):[int2]):[int2] (lifetime change<0|1> (vector_constructor (object_constructor 1:int 2:int):int2):[int2]):[int2])", 199),
    string_view("", 0),
    string_view("", 0),
    string_view("", 0),
    string_view("", 0),
    string_view("(== test_namespace.f:int 1:int)", 31),
    string_view("(== (.x (lifetime change<0|1> (object_constructor 1:int):test_namespace.g):test_namespace.g):int 1:int)", 103),
    string_view("(== (block (this var (object_constructor 1:int):test_namespace.g):void 2:int):int 2:int)", 88),
    string_view("(== (block (x var 1:int):void (y var 0:int):void 1:int):int 1:int)", 66),
    string_view("", 0),
    string_view("(== (block (x var \"\":string):void (y var 0:int):void 2:int):int 2:int)", 70),
    string_view("(== (tf (object_constructor 0:int 0:int):testb 4:int):int 4:int)", 64),
    string_view("(== (tf (block (a var (object_constructor 0:int 0:int):testb):void (lifetime change<1|0> a:testa):testa):testa 1:int):int 4:int)", 128),
    string_view("hello, world!\n\"\'\r\t\\\300", 20),
    string_view("(equal parsed:parsetest? direct:parsetest)", 42),
    string_view("parsetest { true, 1, 1.0, float3 {}, \"\", [], nil, -1, testb { 6, 7 } }", 70),
    string_view("", 0),
    string_view("(equal parsed:parsetest? groundv:parsetest)", 43),
    string_view("", 0),
    string_view("json", 4),
    string_view("{ a: 1, b: 1.0, d: \"\", e: [  ], g: -1, h: 1, s: { _type: \"testb\", a: 6, b: 7 } }", 80),
    string_view("(== json:string groundj:string)", 31),
    string_view("(not ferr:string?)", 18),
    string_view("", 0),
    string_view("json2", 5),
    string_view("(== json2:string groundj:string)", 32),
    string_view("(not fverr:string?)", 19),
    string_view("(equal fval:parsetest? groundv:parsetest)", 41),
    string_view("(not lberr:string?)", 19),
    string_view("(equal lbval:parsetest? groundv:parsetest)", 42),
    string_view("test_namespace.g {}", 19),
    string_view("(lifetime change<0|1> (parse_data (typeof test_namespace.g:test_namespace.g):typeid(test_namespace.g) \"test_namespace.g {}\":string):test_namespace.g?)", 150),
    string_view("(switch (rnd 2:int):int (list (case list:void (block true:bool):bool):bool):void)", 81),
    string_view("(equal (lifetime change<0|1> (string_to_unicode (lifetime change<0|1> (unicode_to_string unicodetests:[int]):string):string):[int]):[int] unicodetests:[int])", 157),
    string_view("what?", 5),
    string_view("no", 2),
    string_view("yes2", 4),
    string_view("yes3", 4),
    string_view("maybe", 5),
    string_view("maybe", 5),
    string_view("maybe", 5),
    string_view("what?", 5),
    string_view("maybe", 5),
    string_view("what?", 5),
    string_view("(equal st:[string] (lifetime change<0|1> (vector_constructor (lifetime change<1|0> \"what?\":string):string (lifetime change<1|0> \"no\":string):string (lifetime change<1|0> \"yes2\":string):string (lifetime change<1|0> \"yes3\":string):string (lifetime change<1|0> \"maybe\":string):string (lifetime change<1|0> \"maybe\":string):string (lifetime change<1|0> \"maybe\":string):string (lifetime change<1|0> \"what?\":string):string (lifetime change<1|0> \"maybe\":string):string (lifetime change<1|0> \"what?\":string):string):[string]):[string])", 526),
    string_view("yes2", 4),
    string_view("(switch (indexing operation st:[string] 2:int):string (list (case (list \"yes2\":string):void (block true:bool):bool):bool (case (list (indexing operation st:[string] 3:int):string):void (block false:bool):bool):bool (case list:void (block false:bool):bool):bool):void)", 267),
    string_view("(switch 3.14:float (list (case (list (range 10.0:float 20.0:float):float):void (block false:bool):bool):bool (case list:void (block true:bool):bool):bool):void)", 160),
    string_view("(equal sorted1:[int] (lifetime change<0|1> (vector_constructor 1:int 1:int 3:int 3:int 4:int 4:int 5:int 5:int 9:int 9:int):[int]):[int])", 137),
    string_view("(equal sorted1:[int] sorted2:[int])", 35),
    string_view("(equal sorted1:[int] sorted3:[int])", 35),
    string_view("(and (== found:int 2:int):bool (== findex:int 0:int):bool)", 58),
    string_view("(and (== found:int 2:int):bool (== findex:int 8:int):bool)", 58),
    string_view("(and (== found:int 2:int):bool (== findex:int 2:int):bool)", 58),
    string_view("(== 44:int (sum testvector:[int]):int)", 38),
    string_view("(== 264:int (sum (map testvector:[int] (def function160):function160(_:int) -> int):[int]):int)", 95),
    string_view("(== (factorial 7:int):int 5040:int)", 35),
    string_view("(== 16:int (function162 (def function163):function163(x:int) -> int):int)", 73),
    string_view("abc", 3),
    string_view("abc", 3),
    string_view("(== (lifetime change<0|1> (ensure_size (lifetime change<1|0> \"abc\":string):string 3:int 122:int 0:int):string):string \"abc\":string)", 131),
    string_view("abc", 3),
    string_view("abczz", 5),
    string_view("(== (lifetime change<0|1> (ensure_size (lifetime change<1|0> \"abc\":string):string 5:int 122:int 0:int):string):string \"abczz\":string)", 133),
    string_view("abc", 3),
    string_view("zzabc", 5),
    string_view("(== (lifetime change<0|1> (ensure_size (lifetime change<1|0> \"abc\":string):string -5:int 122:int 0:int):string):string \"zzabc\":string)", 134),
    string_view("", 0),
    string_view("\003\004\000\000", 4),
    string_view("(and (== ns:string \"\\x03\\x04\\x00\\x00\":string):bool (== ni:int 2:int):bool)", 74),
    string_view("(== (read_int32_le ns:string 0:int):int 1027:int)", 49),
    string_view("", 0),
    string_view("\000\000\002\010", 4),
    string_view("(and (== ns:string \"\\x00\\x00\\x02\\x08\":string):bool (== ni:int 2:int):bool)", 74),
    string_view("(== (read_int16_le_back ns:string 0:int):int 2050:int)", 54),
    string_view("a", 1),
    string_view("b", 1),
    string_view("(== (and s1:string s2:string):string s2:string)", 47),
    string_view("(== aa:int 1:int)", 17),
    string_view("(== (sign (object_constructor -6.0:float 0.0:float 6.0:float):float3):int3 (object_constructor -1:int 0:int 1:int):int3)", 120),
    string_view("(and (and (== bb:int 1:int):bool (== bb:int cc:int):bool):bool (== dd:int ee:int):bool)", 87),
    string_view("(and (== ff:int 2:int):bool (== gg:int 3:int):bool)", 51),
    string_view("(and (== ff:int 8:int):bool (== gg:int 9:int):bool)", 51),
    string_view("(and (== hh:int 6:int):bool (== ii:int 7:int):bool)", 51),
    string_view("(== ll:int 6:int)", 17),
    string_view("(and (== jj:int 6:int):bool (== kk:int 7:int):bool)", 51),
    string_view("(== nn:float 1.0:float)", 23),
    string_view("a", 1),
    string_view("(== mm:string \"a\":string)", 25),
    string_view("", 0),
    string_view("(lifetime change<0|1> (union_return 5:int):string?)", 51),
    string_view("(== nlr1:int 1:int)", 19),
    string_view("(equal testvector:[int] (lifetime change<0|1> (vector_constructor 3:int):[int]):[int])", 86),
    string_view("(in_range 0:int 1:int 0:int)", 28),
    string_view("(in_range 0:int 1:int 0:int)", 28),
    string_view("(== (% (object_constructor 10:int 20:int):int2 (object_constructor 3:int 6:int):int2):int2 (object_constructor 1:int 2:int):int2)", 129),
    string_view("(== (% (object_constructor 10:int 20:int):int2 3:int):int2 (object_constructor 1:int 2:int):int2)", 97),
    string_view("(< (abs (- (% 4.0:float 1.5:float):float 1.0:float):float):float 0.01:float)", 76),
    string_view("(< (max (abs (- (% (object_constructor 10.1:float 20.1:float):float2 (object_constructor 3.0:float 6.0:float):float2):float2 (object_constructor 1.1:float 2.1:float):float2):float2):float2):float 0.01:float)", 207),
    string_view("(< (max (abs (- (% (object_constructor 10.1:float 20.1:float):float2 3.0:float):float2 (object_constructor 1.1:float 2.1:float):float2):float2):float2):float 0.01:float)", 169),
    string_view("\342\272\206\343\213\207\344\254\210", 9),
    string_view("\342\272\206\343\213\207\344\254\210", 9),
    string_view("(== \"\\xE2\\xBA\\x86\\xE3\\x8B\\x87\\xE4\\xAC\\x88\":string \"\\xE2\\xBA\\x86\\xE3\\x8B\\x87\\xE4\\xAC\\x88\":string)", 96),
    string_view("(== (|= bit:int 2:int):int 3:int)", 33),
    string_view("(== (&= bit:int -3:int):int 1:int)", 34),
    string_view("(== (^= bit:int 3:int):int 2:int)", 33),
    string_view("(== (<<= bit:int 1:int):int 4:int)", 34),
    string_view("(== (>>= bit:int 1:int):int 2:int)", 34),
    string_view("(== (get dict:dictionary<float, float> 1.0:float -1.0:float):float -1.0:float)", 78),
    string_view("(== value:float key:float)", 26),
    string_view("a", 1),
    string_view("b", 1),
    string_view("a", 1),
    string_view("b", 1),
    string_view("(and (tobool val:string?):bool (== val:string \"b\":string):bool)", 63),
    string_view("(== (get dict3:dictionary<int3, int3> (object_constructor 1:int 2:int 3:int):int3 int3_0:int3):int3 (object_constructor 4:int 5:int 6:int):int3)", 144),
    string_view("(== lc:int 5:int)", 17),
    string_view("(== lc:int 1:int)", 17),
    string_view("(== (outer_dest2 0:int):int2 (object_constructor 1:int 0:int):int2)", 67),
    string_view("(== (outer_dest2 1:int):int2 (object_constructor 4:int 5:int):int2)", 67),
    string_view("(== (outer_dest2 2:int):int2 (object_constructor 3:int 0:int):int2)", 67),
    string_view("(== (+ (+ (block (a var 1:int):void a:int):int (da 1:int 2:int (+ dav:int 1:int):int):int):int (da 1:int 2:int 3:int):int):int 12:int)", 134),
    string_view("abcdabcd", 8),
    string_view("BC", 2),
    string_view("(== (find_string \"abcdabcd\":string \"BC\":string 0:int):int -1:int)", 65),
    string_view("abcdabcd", 8),
    string_view("bc", 2),
    string_view("(== (find_string \"abcdabcd\":string \"bc\":string 0:int):int 1:int)", 64),
    string_view("abcdabcd", 8),
    string_view("bc", 2),
    string_view("(== (find_string \"abcdabcd\":string \"bc\":string 4:int):int 5:int)", 64),
    string_view("abcdabcd", 8),
    string_view("bc", 2),
    string_view("(== (find_string_reverse \"abcdabcd\":string \"bc\":string 0:int):int 5:int)", 72),
    string_view("abcdabcd", 8),
    string_view("bc", 2),
    string_view("(== (find_string_reverse \"abcdabcd\":string \"bc\":string 4:int):int 1:int)", 72),
    string_view("abcdabcd", 8),
    string_view("bc", 2),
    string_view("X", 1),
    string_view("aXdaXd", 6),
    string_view("(== (lifetime change<0|1> (replace_string \"abcdabcd\":string \"bc\":string \"X\":string 0:int):string):string \"aXdaXd\":string)", 121),
    string_view("abcdabcd", 8),
    string_view("bc", 2),
    string_view("X", 1),
    string_view("aXdabcd", 7),
    string_view("(== (lifetime change<0|1> (replace_string \"abcdabcd\":string \"bc\":string \"X\":string 1:int):string):string \"aXdabcd\":string)", 122),
    string_view(" A | B C |", 10),
    string_view("|", 1),
    string_view(" ", 1),
    string_view("A", 1),
    string_view("B C", 3),
    string_view("", 0),
    string_view("(equal (lifetime change<0|1> (tokenize \" A | B C |\":string \"|\":string \" \":string true:bool):[string]):[string] (lifetime change<0|1> (vector_constructor (lifetime change<1|0> \"A\":string):string (lifetime change<1|0> \"B C\":string):string (lifetime change<1|0> \"\":string):string):[string]):[string])", 297),
    string_view("; A ; B C;; ", 12),
    string_view(";", 1),
    string_view(" ", 1),
    string_view("", 0),
    string_view("A", 1),
    string_view("B C", 3),
    string_view("", 0),
    string_view("(equal (lifetime change<0|1> (tokenize \"; A ; B C;; \":string \";\":string \" \":string 0:int):[string]):[string] (lifetime change<0|1> (vector_constructor (lifetime change<1|0> \"\":string):string (lifetime change<1|0> \"A\":string):string (lifetime change<1|0> \"B C\":string):string (lifetime change<1|0> \"\":string):string):[string]):[string])", 335),
    string_view(", A , B C,, ", 12),
    string_view(",", 1),
    string_view(" ", 1),
    string_view("", 0),
    string_view("A", 1),
    string_view("B C", 3),
    string_view("", 0),
    string_view("", 0),
    string_view("(equal (lifetime change<0|1> (tokenize \", A , B C,, \":string \",\":string \" \":string true:bool):[string]):[string] (lifetime change<0|1> (vector_constructor (lifetime change<1|0> \"\":string):string (lifetime change<1|0> \"A\":string):string (lifetime change<1|0> \"B C\":string):string (lifetime change<1|0> \"\":string):string (lifetime change<1|0> \"\":string):string):[string]):[string])", 379),
    string_view("(== G:int 5:int)", 16),
    string_view("a", 1),
    string_view("b", 1),
    string_view("ab", 2),
    string_view("(== (+= slv:string \"b\":string):string \"ab\":string)", 50),
    string_view("(equal (= vlv:[int] (vector_constructor 2:int):[int]):[int] (lifetime change<0|1> (vector_constructor 2:int):[int]):[int])", 122),
    string_view("a", 1),
    string_view("a", 1),
    string_view("b", 1),
    string_view("b", 1),
    string_view("b", 1),
    string_view("(== (indexing operation (.b (= stlv:STLV (object_constructor (lifetime change<1|0> \"b\":string):string (vector_constructor (lifetime change<1|0> \"b\":string):string):[string]):STLV):STLV):[string] 0:int):string \"b\":string)", 220),
    string_view("inf", 3),
    string_view("(== (lifetime change<0|1> (string (tostring inf:float):string):string):string \"inf\":string)", 91),
    string_view("nan", 3),
    string_view("(>= (find_string (lifetime change<0|1> (string (tostring (% 1.0:float 0.0:float):float):string):string):string \"nan\":string 0:int):int 0:int)", 141),
    string_view("inf", 3),
    string_view("(== (lifetime change<0|1> (string (tostring (.y (/ float2_1:float2 float2_0:float2):float2):float):string):string):string \"inf\":string)", 135),
    string_view("no", 2),
    string_view("yes", 3),
    string_view("maybe", 5),
    string_view("what?", 5),
    string_view("(== (+ (+ (+ (++ a:int):int (++ a:int):int):int (-- a:int):int):int (-- a:int):int):int 4:int)", 94),
    string_view("(== (+ (+ (+ (++ (indexing operation b:[int] 0:int):int):int (++ (indexing operation b:[int] 0:int):int):int):int (-- (indexing operation b:[int] 0:int):int):int):int (-- (indexing operation b:[int] 0:int):int):int):int 4:int)", 226),
    string_view("a", 1),
    string_view("foo", 3),
    string_view("", 0),
    string_view("(== (length v:string):int n:int)", 32),
    string_view("*", 1),
    string_view("(== (+ a:int b:int):int 3:int)", 30),
    string_view("false", 5),
    string_view("a", 1),
    string_view("(== (% 5.0:float divisor:float):float 1.0:float)", 48),
    string_view("(== (% (* float2_1:float2 5.0:float):float2 divisor:float):float2 float2_1:float2)", 82),
    string_view("(== (% (* int2_1:int2 5:int):int2 2:int):int2 int2_1:int2)", 58),
    string_view("hello", 5),
    string_view("Reflect", 7),
    string_view("(== (lifetime change<0|1> (type_string r:Reflect):string):string \"Reflect\":string)", 82),
    string_view("(== (type_field_count r:Reflect):int 4:int)", 43),
    string_view("float", 5),
    string_view("(== (lifetime change<0|1> (type_field_string r:Reflect 0:int):string):string \"float\":string)", 92),
    string_view("[int]", 5),
    string_view("(== (lifetime change<0|1> (type_field_string r:Reflect 3:int):string):string \"[int]\":string)", 92),
    string_view("s", 1),
    string_view("(== (lifetime change<0|1> (type_field_name r:Reflect 2:int):string):string \"s\":string)", 86),
    string_view("1.0", 3),
    string_view("(== (lifetime change<0|1> (type_field_value r:Reflect 0:int):string):string \"1.0\":string)", 89),
    string_view("int", 3),
    string_view("(== (lifetime change<0|1> (type_element_string (.v r:Reflect):[int]):string):string \"int\":string)", 97),
    string_view("(== state:int 2:int)", 20),
    string_view("(equal gs:[int] (lifetime change<0|1> (vector_constructor 10:int 20:int 30:int 40:int):[int]):[int])", 100),
    string_view("foo", 3),
    string_view("(== (lifetime change<0|1> (get (dictionary 17:int):dictionary<string, dictionary<string, int>> \"foo\":string):dictionary<string, int>?):dictionary<string, int>? nil:dictionary<string, int>?)", 189),
    string_view("(== (toint (typeof int:int):typeid(int)):int 0:int)", 51),
    string_view("(== 1:int (g (def function199):function199() -> int):int)", 57),
    string_view("a", 1),
    string_view("(== \"a\":string (lifetime change<0|1> (g (def function200):function200() -> string):string):string)", 98),
    string_view("a", 1),
    string_view("(statements (compile_time_if 1:int):int 1:int)", 46),
    string_view("", 0),
    string_view("(statements (lifetime change<0|1> (compile_time_if \"\":string):string):string 1:int)", 83),
    string_view("(is:testa (lifetime change<0|1> (return_from 1:int):testa):testa)", 65),
    string_view("", 0),
    string_view("(is:testa (lifetime change<0|1> (return_from \"\":string):testa):testa)", 69),
    string_view("(== b:bool true:bool)", 21),
    string_view("true", 4),
    string_view("(== (lifetime change<0|1> (string (tostring true:bool):string):string):string \"true\":string)", 92),
    string_view("[true]", 6),
    string_view("(parse_data (typeof [bool]:[bool]):typeid([bool]) \"[true]\":string)", 66),
    string_view("(== (indexing operation (lifetime change<0|1> (assert (parse_data (typeof [bool]:[bool]):typeid([bool]) \"[true]\":string):[bool]?):[bool]):[bool] 0:int):bool true:bool)", 167),
    string_view("false", 5),
    string_view("(== (block (this var (object_constructor 0:int):S1):void 1:int):int 1:int)", 74),
    string_view("(== (block (this var (object_constructor 0:int 0:int):S2):void 2:int):int 2:int)", 80),
    string_view("hello", 5),
    string_view("(== (f (lifetime change<1|0> x:C1):C1 0:int):int 2:int)", 55),
    string_view("(== (f (object_constructor 0:int 0:int):C2 0:int):int 2:int)", 60),
    string_view("hi", 2),
    string_view("hi2", 3),
    string_view("(== (lifetime change<0|1> (g (lifetime change<1|0> x:C1):C1 (lifetime change<1|0> \"hi\":string):string):string):string \"hi2\":string)", 131),
    string_view("(== (g (lifetime change<1|0> x:C1):C1 3:int):int 5:int)", 55),
    string_view("(== (f (lifetime change<1|0> x2:C1):C1 10:int):int 15:int)", 58),
    string_view("(== (r (lifetime change<1|0> x:C1):C1 10:int):int 0:int)", 56),
    string_view("(== (sup (lifetime change<1|0> x:C1):C1):int 2:int)", 51),
    string_view("(== (ad (lifetime change<1|0> x:C1):C1):int 11:int)", 51),
    string_view("(== (sni (lifetime change<1|0> x3:C1):C0):int 12:int)", 53),
    string_view("b", 1),
    string_view("(and (== mr1:int 2:int):bool (== mr2:string \"b\":string):bool)", 61),
    string_view("(== (+ (foo (object_constructor 1:int):D<int>):int (block (i var 2:int):void i:int):int):int 3:int)", 99),
    string_view("ha", 2),
    string_view("llo", 3),
    string_view("hallo", 5),
    string_view("(== (lifetime change<0|1> (+ (lifetime change<0|1> (foo (object_constructor (lifetime change<1|0> \"ha\":string):string):D<string>):string):string (lifetime change<0|1> (foo \"llo\":string):string):string):string):string \"hallo\":string)", 232),
    string_view("(== (foo 1.0:float):float 2.0:float)", 36),
    string_view("a", 1),
    string_view("b", 1),
    string_view("", 0),
    string_view("", 0),
    string_view("(== a:string \"\":string)", 23),
    string_view("", 0),
    string_view("(== b:string \"\":string)", 23),
    string_view("", 0),
    string_view("(== x:string \"\":string)", 23),
    string_view("(== (h a:Ai):int 1:int)", 23),
    string_view("hello", 5),
    string_view("hi", 2),
    string_view("yo", 2),
    string_view("sup", 3),
    string_view("(statements (block (x var 1.0:float):void (y var 1.0:float):void y:float):float 1:int)", 86),
    string_view("(statements (block (x var 1.0:float):void (y var 1.0:float):void y:float):float 1:int)", 86),
    string_view("(== (k 2.0:float):float 1.0:float)", 34),
    string_view("(== (k 2.0:float):float 1.0:float)", 34),
    string_view("(== (indexing operation (.ts (lifetime change<0|1> (t0 a:Ai):Ai):Ai):[int] 0:int):int 1:int)", 92),
    string_view("(block 1:int)", 13),
    string_view("(== (lifetime change<0|1> (block nil:string?):string?):string? nil:string?)", 75),
    string_view("(equal (lifetime change<0|1> (block vector_constructor:[string]):[string]):[string] (lifetime change<0|1> vector_constructor:[string]):[string])", 144),
    string_view("(== (toint p:typeid(string)):int 3:int)", 39),
    string_view("(== (indexing operation (.ts (lifetime change<0|1> (t1 a:Ai):Ai):Ai):[int] 0:int):int 1:int)", 92),
    string_view("(== (indexing operation (.ts (lifetime change<0|1> (t2 a:Ai):Ai):Ai):[int] 0:int):int 1:int)", 92),
    string_view("a", 1),
    string_view("a", 1),
    string_view("a", 1),
    string_view("a", 1),
    string_view("(== (get (.dict a1:set1<string>):dictionary<string, int> \"a\":string 0:int):int (get (.dict a2:set2<string>):dictionary<string, int> \"a\":string 0:int):int)", 154),
    string_view("(== (block (xs var (vector_constructor 1:int 2:int 3:int):[int]):void 2:int):int 2:int)", 87),
    string_view("(.a (indexing operation as:[A] 2:int):A)", 40),
    string_view("(== (.y (.c (assert (.a (indexing operation as:[A] 2:int):A):A?):A):int2):int 4:int)", 84),
    string_view("(== (.b a:A):int 1:int)", 23),
    string_view(".................................", 33),
    string_view("................S................", 33),
    string_view("........#..............#.........", 33),
    string_view("........#..............#/////////", 33),
    string_view("........#..............#.........", 33),
    string_view("........#..............#.........", 33),
    string_view("........################.........", 33),
    string_view(".................................", 33),
    string_view("...............///...............", 33),
    string_view(".............../E/...............", 33),
    string_view("...............///...............", 33),
    string_view(".................................", 33),
    string_view("......OXXXXXXXXXXXXXXXXXXXO......", 33),
    string_view(".....OXPPPPPPPPPPXXXXXXXXXXO.....", 33),
    string_view(".....OXP#XXXXXXXXXXXXXX#XXXO.....", 33),
    string_view(".....OXP#XXXXXXXXXXXXXX#XOO//////", 33),
    string_view(".....OXP#XXXXXXXXXXXXXX#XO.......", 33),
    string_view(".....OXP#XXXXXXXXXXXXXX#XO.......", 33),
    string_view(".....OXP################XO.......", 33),
    string_view(".....OXPPPPPPPPPPXXXXXXXXO.......", 33),
    string_view(".....OXXXXXXXXXOPOXXXXXXXO.......", 33),
    string_view(".....OXXXXXXXXXOPOXXXXXXXO.......", 33),
    string_view("......OXXXXXXXXO//OOOOOOO........", 33),
    string_view(".......OOOOOOOO..................", 33),
    string_view("(== (length path:[pathingcell]):int 27:int)", 43),
    string_view("(equal astar_result:[string] expected_result:[string])", 54),
    string_view("", 0),
    string_view("P", 1),
    string_view("X", 1),
    string_view("O", 1),
    string_view("(== (length goap_path:[spec_node]):int 9:int)", 45),
    string_view("Eat", 3),
    string_view("BuyPizza", 8),
    string_view("SellSkin", 8),
    string_view("KillWolf", 8),
    string_view("Eat", 3),
    string_view("BuyPizza", 8),
    string_view("SellSkin", 8),
    string_view("KillWolf", 8),
    string_view("(equal (lifetime change<0|1> (map goap_path:[spec_node] (def function274):function274(n:spec_node) -> string):[string]):[string] (lifetime change<0|1> (vector_constructor (lifetime change<1|0> \"Eat\":string):string (lifetime change<1|0> \"BuyPizza\":string):string (lifetime change<1|0> \"SellSkin\":string):string (lifetime change<1|0> \"KillWolf\":string):string (lifetime change<1|0> \"Eat\":string):string (lifetime change<1|0> \"BuyPizza\":string):string (lifetime change<1|0> \"SellSkin\":string):string (lifetime change<1|0> \"KillWolf\":string):string):[string]):[string])", 565),
    string_view("", 0),
    string_view("(lifetime change<0|1> (knights_tour (object_constructor 8:int 8:int):int2):[[int]]?)", 84),
    string_view("(+\n(* 2 3 4)  // Comment.\n10)\n", 30),
    string_view("", 0),
    string_view("(and (tobool ast:node?):bool (== err:string \"\":string):bool)", 60),
    string_view("(+ (* 2 3 4) 10)", 16),
    string_view("(== (lifetime change<0|1> (pretty (lifetime change<1|0> ast:integer):node):string):string \"(+ (* 2 3 4) 10)\":string)", 116),
    string_view("(== (eval (lifetime change<1|0> ast:integer):node):int 34:int)", 62),
    string_view("(((2*3)*4)+10)", 14),
    string_view("(== code:string \"(((2*3)*4)+10)\":string)", 40),
    string_view("(", 1),
    string_view(")", 1),
    string_view("+", 1),
    string_view("*", 1),
    string_view("false", 5),
    string_view("+", 1),
    string_view("*", 1),
    string_view("false", 5),
    string_view(" ", 1),
    string_view("(", 1),
    string_view(")", 1),
    string_view("", 0),
    string_view("", 0),
    string_view("eof", 3),
    string_view("", 0),
    string_view("error: line ", 12),
    string_view(": ", 2),
    string_view("()\177", 3),
    string_view("unprintable character: ", 23),
    string_view("int", 3),
    string_view("atom", 4),
    string_view("eof", 3),
    string_view("(", 1),
    string_view(")", 1),
    string_view("expected: ", 10),
    string_view(", found: ", 9),
    string_view("expected: ", 10),
    string_view(", found: ", 9),
    string_view("(", 1),
    string_view("atom", 4),
    string_view("+", 1),
    string_view("*", 1),
    string_view("unknown op: ", 12),
    string_view(")", 1),
    string_view("int", 3),
    string_view("cannot parse expression starting with: ", 39),
    string_view("(== (length (lifetime change<0|1> (floodfillpath (* int2_1:int2 20:int):int2 (* int2_1:int2 5:int):int2 (def function305):function305(v:int2) -> bool):[[pathmap]]):[[pathmap]]):int 20:int)", 188),
    string_view("(< (abs (- (indexing operation (lifetime change<0|1> (gradient_descent (lifetime change<1|0> x:[float]):[float] (lifetime change<1|0> d:[float]):[float] mu:float n_epochs:int):[float]):[float] 0:int):float 2.428925605728:float):float):float 0.01:float)", 252),
    string_view("(>= pidx:int 0:int)", 19),
    string_view("(== (length r:[[float4]]):int h:int)", 36),
    string_view("a = ", 4),
    string_view("!", 1),
    string_view("a = ", 4),
    string_view("!", 1),
    string_view("(== (lifetime change<0|1> (+ (lifetime change<0|1> (+ \"a = \":string (lifetime change<0|1> (tostring a:int):string):string):string):string \"!\":string):string):string (lifetime change<0|1> (+ (lifetime change<0|1> (+ \"a = \":string (lifetime change<0|1> (tostring a:int):string):string):string):string \"!\":string):string):string)", 326),
    string_view("a = ", 4),
    string_view(" and b = ", 9),
    string_view("!", 1),
    string_view("a = ", 4),
    string_view(" and b = ", 9),
    string_view("!", 1),
    string_view("(== (lifetime change<0|1> (+ (lifetime change<0|1> (+ (lifetime change<0|1> (+ (lifetime change<0|1> (+ \"a = \":string (lifetime change<0|1> (tostring a:int):string):string):string):string \" and b = \":string):string):string (lifetime change<0|1> (tostring b:[float]):string):string):string):string \"!\":string):string):string (lifetime change<0|1> (+ (lifetime change<0|1> (+ (lifetime change<0|1> (+ (lifetime change<0|1> (+ \"a = \":string (lifetime change<0|1> (tostring a:int):string):string):string):string \" and b = \":string):string):string (lifetime change<0|1> (tostring b:[float]):string):string):string):string \"!\":string):string):string)", 644),
    string_view("(== (lifetime change<0|1> (string (tostring (+ 3:int a:int):int):string):string):string (lifetime change<0|1> (string (tostring (+ 3:int a:int):int):string):string):string)", 172),
    string_view("(== (lifetime change<0|1> (+ (lifetime change<0|1> (string (tostring a:int):string):string):string (lifetime change<0|1> (tostring b:[float]):string):string):string):string (lifetime change<0|1> (+ (lifetime change<0|1> (string (tostring a:int):string):string):string (lifetime change<0|1> (tostring b:[float]):string):string):string):string)", 342),
    string_view("{ ", 2),
    string_view(" }", 2),
    string_view("{ ", 2),
    string_view(" }", 2),
    string_view("(== (lifetime change<0|1> (+ (lifetime change<0|1> (+ \"{ \":string (lifetime change<0|1> (tostring a:int):string):string):string):string \" }\":string):string):string (lifetime change<0|1> (+ (lifetime change<0|1> (+ \"{ \":string (lifetime change<0|1> (tostring a:int):string):string):string):string \" }\":string):string):string)", 324),
    string_view("{ ", 2),
    string_view(" }", 2),
    string_view("{ ", 2),
    string_view(" }", 2),
    string_view("(== (lifetime change<0|1> (+ (lifetime change<0|1> (+ \"{ \":string (lifetime change<0|1> (tostring a:int):string):string):string):string \" }\":string):string):string (lifetime change<0|1> (+ (lifetime change<0|1> (+ \"{ \":string (lifetime change<0|1> (tostring a:int):string):string):string):string \" }\":string):string):string)", 324),
    string_view("{", 1),
    string_view("}", 1),
    string_view("{", 1),
    string_view("}", 1),
    string_view("(== (lifetime change<0|1> (+ (lifetime change<0|1> (+ \"{\":string (lifetime change<0|1> (tostring (+ 3:int a:int):int):string):string):string):string \"}\":string):string):string (lifetime change<0|1> (+ (lifetime change<0|1> (+ \"{\":string (lifetime change<0|1> (tostring (+ 3:int a:int):int):string):string):string):string \"}\":string):string):string)", 348),
    string_view("(== (lifetime change<0|1> (string (string (string (tostring a:int):string):string):string):string):string (lifetime change<0|1> (string (tostring a:int):string):string):string)", 176),
    string_view("return 1 + 2", 12),
    string_view("3", 1),
    string_view("(== compres1:string \"3\":string)", 31),
    string_view("plugintest.lobster", 18),
    string_view("plugintest return value", 23),
    string_view("(== compres2:string \"plugintest return value\":string)", 53),
    string_view("", 0),
    string_view("a", 1),
    string_view("b", 1),
    string_view("c", 1),
    string_view("(equal (lifetime change<0|1> (deepcopy nested:Nest 10:int):Nest):Nest nested:Nest)", 82),
    string_view("(operator== (operator/ (operator* (operator- (operator+ c1:OC c2:OC):OC c1:OC):OC c2:OC):OC c1:OC):OC c4:OC)", 108),
    string_view("(operator== c3:OC c4:OC)", 24),
    string_view("(operator== c4:OC c3:OC)", 24),
    string_view("(operator== (operator- (operator- c4:OC):OC):OC c4:OC)", 54),
    string_view("(== (operator!= c4:OC c4:OC):bool false:bool)", 45),
    string_view("(== (operator[] c4:OC 0:int):int 4:int)", 39),
    string_view("(operator== (operator/ (operator* (operator- (operator+ s1:OS s2:OS):OS s1:OS):OS s2:OS):OS s1:OS):OS s4:OS)", 108),
    string_view("(== (operator- (object_constructor 1.0:float 1.0:float 1.0:float 1.0:float):quat):quat (object_constructor -1.0:float -1.0:float -1.0:float 1.0:float):quat)", 156),
    string_view("(== (operator* (object_constructor 1.0:float 1.0:float 1.0:float 1.0:float):quat (object_constructor 2.0:float 2.0:float 2.0:float 1.0:float):quat):quat (object_constructor 3.0:float 3.0:float 3.0:float -5.0:float):quat)", 220),
    string_view("(== (operator* (object_constructor 1.0:float 1.0:float 1.0:float 1.0:float):quat 2.0:float):quat (object_constructor 2.0:float 2.0:float 2.0:float 2.0:float):quat)", 163),
    string_view("(== a:mat2x2 a:mat2x2)", 22),
    string_view("(== a:mat2x2 (object_constructor 1.0:float 2.0:float 3.0:float 4.0:float):mat2x2)", 81),
    string_view("(!= a:mat2x2 b:mat2x2)", 22),
    string_view("(== (+ 5.0:float a:mat2x2):mat2x2 (object_constructor 6.0:float 7.0:float 8.0:float 9.0:float):mat2x2)", 102),
    string_view("(== (+ a:mat2x2 5.0:float):mat2x2 (object_constructor 6.0:float 7.0:float 8.0:float 9.0:float):mat2x2)", 102),
    string_view("(== (+ a:mat2x2 b:mat2x2):mat2x2 (object_constructor 6.0:float 8.0:float 10.0:float 12.0:float):mat2x2)", 103),
    string_view("(== (- 5.0:float a:mat2x2):mat2x2 (object_constructor 4.0:float 3.0:float 2.0:float 1.0:float):mat2x2)", 102),
    string_view("(== (- a:mat2x2 5.0:float):mat2x2 (object_constructor -4.0:float -3.0:float -2.0:float -1.0:float):mat2x2)", 106),
    string_view("(== (- b:mat2x2 a:mat2x2):mat2x2 (object_constructor 4.0:float 4.0:float 4.0:float 4.0:float):mat2x2)", 101),
    string_view("(== (* 5.0:float a:mat2x2):mat2x2 (object_constructor 5.0:float 10.0:float 15.0:float 20.0:float):mat2x2)", 105),
    string_view("(== (operator* a:mat2x2 b:mat2x2):mat2x2 (object_constructor 19.0:float 22.0:float 43.0:float 50.0:float):mat2x2)", 113),
    string_view("(== (operator* b:mat2x2 a:mat2x2):mat2x2 (object_constructor 23.0:float 34.0:float 31.0:float 46.0:float):mat2x2)", 113),
    string_view("(== (determinant a:mat2x2):float -2.0:float)", 44),
    string_view("(== (trace a:mat2x2):float 4.0:float)", 37),
    string_view("(== (adjugate a:mat2x2):mat2x2 (object_constructor 4.0:float -2.0:float -3.0:float 1.0:float):mat2x2)", 101),
    string_view("(== (inverse a:mat2x2):mat2x2 (object_constructor -2.0:float 1.0:float 1.5:float -0.5:float):mat2x2)", 100),
    string_view("(== (operator* a:mat2x2 (inverse a:mat2x2):mat2x2):mat2x2 mat2x2_identity:mat2x2)", 81),
    string_view("(== (operator* a:mat2x2 c:float2):float2 (object_constructor 8.0:float 18.0:float):float2)", 90),
    string_view("(== (operator* b:mat2x2 c:float2):float2 (object_constructor 28.0:float 38.0:float):float2)", 91),
    string_view("(equal results1:[int] (lifetime change<0|1> (vector_constructor 1:int 3:int 4:int 5:int):[int]):[int])", 102),
    string_view("(equal results2:[int] (lifetime change<0|1> (vector_constructor 1:int 3:int 4:int 4:int):[int]):[int])", 102),
    string_view("(equal results3:[int] (lifetime change<0|1> (vector_constructor 1:int 2:int 2:int 2:int):[int]):[int])", 102),
    string_view("(equal results4:[int] (lifetime change<0|1> (vector_constructor 0:int 0:int 0:int 5:int):[int]):[int])", 102),
    string_view("t", 1),
    string_view("f", 1),
    string_view("(equal results5:[string] (lifetime change<0|1> (vector_constructor (lifetime change<1|0> \"t\":string):string (lifetime change<1|0> \"f\":string):string):[string]):[string])", 169),
    string_view("t", 1),
    string_view("f", 1),
    string_view("?", 1),
    string_view("(equal results6:[string] (lifetime change<0|1> (vector_constructor (lifetime change<1|0> \"t\":string):string (lifetime change<1|0> \"f\":string):string (lifetime change<1|0> \"?\":string):string):[string]):[string])", 210),
    string_view("t", 1),
    string_view("f", 1),
    string_view("t", 1),
    string_view("f", 1),
    string_view("?", 1),
};

static const string_view file_names[] = {
    string_view("unittest.lobster", 16),
    string_view("stdtype.lobster", 15),
    string_view("testing.lobster", 15),
    string_view("structtest.lobster", 18),
    string_view("vec.lobster", 11),
    string_view("std.lobster", 11),
    string_view("misctest.lobster", 16),
    string_view("exception.lobster", 17),
    string_view("dictionary.lobster", 18),
    string_view("typetest.lobster", 16),
    string_view("astartest.lobster", 17),
    string_view("astar.lobster", 13),
    string_view("goaptest.lobster", 16),
    string_view("knightstest.lobster", 19),
    string_view("parsertest.lobster", 18),
    string_view("floodtest.lobster", 17),
    string_view("floodfillpath.lobster", 21),
    string_view("watertest.lobster", 17),
    string_view("gradienttest.lobster", 20),
    string_view("springstest.lobster", 19),
    string_view("smallpttest.lobster", 19),
    string_view("color.lobster", 13),
    string_view("stringtest.lobster", 18),
    string_view("mischtest.lobster", 17),
    string_view("lifetimetest.lobster", 20),
    string_view("builtintest.lobster", 19),
    string_view("operators.lobster", 17),
    string_view("quaternion.lobster", 18),
    string_view("matrixtest.lobster", 18),
    string_view("mat2x2.lobster", 14),
    string_view("typeswitch.lobster", 18),
};

static const string_view function_names[] = {
    string_view("__top_level_expression", 22),
    string_view("testf", 5),
    string_view("testrunnerf", 11),
    string_view("function3", 9),
    string_view("run_test", 8),
    string_view("function5", 9),
    string_view("function6", 9),
    string_view("map", 3),
    string_view("map2", 4),
    string_view("function9", 9),
    string_view("filter", 6),
    string_view("filter_map", 10),
    string_view("filter_indices", 14),
    string_view("partition", 9),
    string_view("partition_indices", 17),
    string_view("exists", 6),
    string_view("forever", 7),
    string_view("fold", 4),
    string_view("fold2", 5),
    string_view("reduce", 6),
    string_view("connect", 7),
    string_view("function21", 10),
    string_view("reduce_reverse", 14),
    string_view("find", 4),
    string_view("find_or", 7),
    string_view("find_or_push", 12),
    string_view("find_best", 9),
    string_view("weighted_pick", 13),
    string_view("function28", 10),
    string_view("sum", 3),
    string_view("function30", 10),
    string_view("product", 7),
    string_view("function32", 10),
    string_view("zip", 3),
    string_view("function34", 10),
    string_view("reverse", 7),
    string_view("reverse_map", 11),
    string_view("function37", 10),
    string_view("reverse_fold", 12),
    string_view("reverse_find", 12),
    string_view("reverse_list", 12),
    string_view("function41", 10),
    string_view("flatten", 7),
    string_view("function43", 10),
    string_view("split", 5),
    string_view("rnd_pick", 8),
    string_view("rnd_pick2", 9),
    string_view("rnd_consume", 11),
    string_view("qsort", 5),
    string_view("function49", 10),
    string_view("qsort_in_place", 14),
    string_view("rec", 3),
    string_view("insertion_sort", 14),
    string_view("insert_ordered", 14),
    string_view("shuffle", 7),
    string_view("nest_if", 7),
    string_view("return_after", 12),
    string_view("do", 2),
    string_view("for_bias", 8),
    string_view("for_scale", 9),
    string_view("for_range", 9),
    string_view("for_range_incl", 14),
    string_view("collect", 7),
    string_view("function63", 10),
    string_view("fatal_exit", 10),
    string_view("fatal", 5),
    string_view("check", 5),
    string_view("print", 5),
    string_view("xy", 2),
    string_view("xz", 2),
    string_view("yw", 2),
    string_view("zw", 2),
    string_view("xyz", 3),
    string_view("int3", 4),
    string_view("float3", 6),
    string_view("vec3", 4),
    string_view("int4", 4),
    string_view("float4", 6),
    string_view("vec4", 4),
    string_view("vec2_v", 6),
    string_view("vec3_v", 6),
    string_view("vec4_v", 6),
    string_view("v_vec2", 6),
    string_view("v_vec3", 6),
    string_view("v_vec4", 6),
    string_view("yx", 2),
    string_view("zyx", 3),
    string_view("wzyx", 4),
    string_view("rnd_float2", 10),
    string_view("rnd_float3", 10),
    string_view("rnd_float4", 10),
    string_view("rnd_float2_norm", 15),
    string_view("rnd_float3_norm", 15),
    string_view("rnd_int2", 8),
    string_view("rnd_int3", 8),
    string_view("forxy", 5),
    string_view("foryx", 5),
    string_view("mapxy", 5),
    string_view("function98", 10),
    string_view("function99", 10),
    string_view("forxyz", 6),
    string_view("mapxyz", 6),
    string_view("function102", 11),
    string_view("function103", 11),
    string_view("function104", 11),
    string_view("vecfromyawpitch", 15),
    string_view("rotate2D", 8),
    string_view("function107", 11),
    string_view("function108", 11),
    string_view("function109", 11),
    string_view("function110", 11),
    string_view("function111", 11),
    string_view("f", 1),
    string_view("g", 1),
    string_view("function114", 11),
    string_view("mm", 2),
    string_view("function116", 11),
    string_view("f", 1),
    string_view("fun", 3),
    string_view("function119", 11),
    string_view("frs", 3),
    string_view("function121", 11),
    string_view("function122", 11),
    string_view("function123", 11),
    string_view("try", 3),
    string_view("exception_handler", 17),
    string_view("throw", 5),
    string_view("throw_trace", 11),
    string_view("rethrow", 7),
    string_view("throw_if", 8),
    string_view("try", 3),
    string_view("function131", 11),
    string_view("protect", 7),
    string_view("function133", 11),
    string_view("lookup", 6),
    string_view("set", 3),
    string_view("del", 3),
    string_view("delete", 6),
    string_view("clear", 5),
    string_view("add", 3),
    string_view("get", 3),
    string_view("get", 3),
    string_view("get_or_set", 10),
    string_view("get_or_create", 13),
    string_view("get_keys", 8),
    string_view("get_values", 10),
    string_view("forkv", 5),
    string_view("dictionary", 10),
    string_view("function148", 11),
    string_view("test_namespace.f", 16),
    string_view("test_namespace.f2", 17),
    string_view("m", 1),
    string_view("function152", 11),
    string_view("tf", 2),
    string_view("to_a", 4),
    string_view("function155", 11),
    string_view("function156", 11),
    string_view("function157", 11),
    string_view("function158", 11),
    string_view("function159", 11),
    string_view("function160", 11),
    string_view("factorial", 9),
    string_view("function162", 11),
    string_view("function163", 11),
    string_view("multiret", 8),
    string_view("multiretpassthru", 16),
    string_view("multirettyped", 13),
    string_view("write", 5),
    string_view("union_return", 12),
    string_view("nlr1", 4),
    string_view("nlr2", 4),
    string_view("function171", 11),
    string_view("recursive_exceptions", 20),
    string_view("function173", 11),
    string_view("function174", 11),
    string_view("function175", 11),
    string_view("function176", 11),
    string_view("outer", 5),
    string_view("error", 5),
    string_view("outer_dest2", 11),
    string_view("outer_dest1", 11),
    string_view("return_past", 11),
    string_view("function_with_attributes", 24),
    string_view("da", 2),
    string_view("da", 2),
    string_view("G", 1),
    string_view("H", 1),
    string_view("function187", 11),
    string_view("function188", 11),
    string_view("function189", 11),
    string_view("function190", 11),
    string_view("function191", 11),
    string_view("function192", 11),
    string_view("function193", 11),
    string_view("function194", 11),
    string_view("function195", 11),
    string_view("h", 1),
    string_view("g", 1),
    string_view("function198", 11),
    string_view("function199", 11),
    string_view("function200", 11),
    string_view("function201", 11),
    string_view("compile_time_if", 15),
    string_view("function203", 11),
    string_view("return_from", 11),
    string_view("return_from2", 12),
    string_view("function206", 11),
    string_view("function207", 11),
    string_view("f", 1),
    string_view("ad", 2),
    string_view("f", 1),
    string_view("g", 1),
    string_view("r", 1),
    string_view("mr", 2),
    string_view("sup", 3),
    string_view("sni", 3),
    string_view("foo", 3),
    string_view("function217", 11),
    string_view("f", 1),
    string_view("function219", 11),
    string_view("f", 1),
    string_view("g", 1),
    string_view("h", 1),
    string_view("i", 1),
    string_view("j", 1),
    string_view("k", 1),
    string_view("q", 1),
    string_view("t0", 2),
    string_view("l", 1),
    string_view("n", 1),
    string_view("o", 1),
    string_view("p", 1),
    string_view("t1", 2),
    string_view("t2", 2),
    string_view("function234", 11),
    string_view("function235", 11),
    string_view("make_set", 8),
    string_view("function237", 11),
    string_view("f", 1),
    string_view("function239", 11),
    string_view("function240", 11),
    string_view("foo", 3),
    string_view("astar_clear", 11),
    string_view("astar_generic", 13),
    string_view("function244", 11),
    string_view("astar_graph", 11),
    string_view("function246", 11),
    string_view("function247", 11),
    string_view("function248", 11),
    string_view("function249", 11),
    string_view("astar_2dgrid", 12),
    string_view("astar_distance", 14),
    string_view("function252", 11),
    string_view("function253", 11),
    string_view("function254", 11),
    string_view("precondition", 12),
    string_view("effect", 6),
    string_view("astar_goap", 10),
    string_view("function258", 11),
    string_view("function259", 11),
    string_view("function260", 11),
    string_view("function261", 11),
    string_view("function262", 11),
    string_view("function263", 11),
    string_view("function264", 11),
    string_view("function265", 11),
    string_view("function266", 11),
    string_view("function267", 11),
    string_view("function268", 11),
    string_view("function269", 11),
    string_view("precondition", 12),
    string_view("effect", 6),
    string_view("function272", 11),
    string_view("function273", 11),
    string_view("function274", 11),
    string_view("function275", 11),
    string_view("knights_tour", 12),
    string_view("function277", 11),
    string_view("function278", 11),
    string_view("empty", 5),
    string_view("function280", 11),
    string_view("function281", 11),
    string_view("function282", 11),
    string_view("function283", 11),
    string_view("function284", 11),
    string_view("pretty", 6),
    string_view("eval", 4),
    string_view("compile", 7),
    string_view("function288", 11),
    string_view("function289", 11),
    string_view("function290", 11),
    string_view("function291", 11),
    string_view("function292", 11),
    string_view("parse", 5),
    string_view("error", 5),
    string_view("lex_atom", 8),
    string_view("function296", 11),
    string_view("lex_next", 8),
    string_view("expect", 6),
    string_view("parse_exp", 9),
    string_view("function300", 11),
    string_view("floodfillpath", 13),
    string_view("function302", 11),
    string_view("flood", 5),
    string_view("function304", 11),
    string_view("function305", 11),
    string_view("function306", 11),
    string_view("function307", 11),
    string_view("function308", 11),
    string_view("loopworld", 9),
    string_view("ifinworld", 9),
    string_view("forneighboursinworld", 20),
    string_view("function312", 11),
    string_view("sim", 3),
    string_view("function314", 11),
    string_view("function315", 11),
    string_view("function316", 11),
    string_view("function317", 11),
    string_view("gradient_descent", 16),
    string_view("function319", 11),
    string_view("function320", 11),
    string_view("function321", 11),
    string_view("function322", 11),
    string_view("wiggleverts", 11),
    string_view("function324", 11),
    string_view("function325", 11),
    string_view("function326", 11),
    string_view("function327", 11),
    string_view("function328", 11),
    string_view("function329", 11),
    string_view("function330", 11),
    string_view("function331", 11),
    string_view("function332", 11),
    string_view("function333", 11),
    string_view("from_srgb", 9),
    string_view("to_srgb", 7),
    string_view("luminance", 9),
    string_view("function337", 11),
    string_view("intersect", 9),
    string_view("radiance", 8),
    string_view("function340", 11),
    string_view("function341", 11),
    string_view("function342", 11),
    string_view("function343", 11),
    string_view("function344", 11),
    string_view("function345", 11),
    string_view("function346", 11),
    string_view("function347", 11),
    string_view("function348", 11),
    string_view("function349", 11),
    string_view("function350", 11),
    string_view("operator*", 9),
    string_view("operator-", 9),
    string_view("transform", 9),
    string_view("quat_from_angle_axis", 20),
    string_view("quat_from_vec", 13),
    string_view("quat_from_2vecs", 15),
    string_view("quat_from_euler", 15),
    string_view("quat_from_yaw", 13),
    string_view("quat_from_pitch", 15),
    string_view("operator+", 9),
    string_view("operator-", 9),
    string_view("operator/", 9),
    string_view("operator%", 9),
    string_view("operator&", 9),
    string_view("operator|", 9),
    string_view("operator^", 9),
    string_view("operator<<", 10),
    string_view("operator>>", 10),
    string_view("operator+=", 10),
    string_view("operator-=", 10),
    string_view("operator*=", 10),
    string_view("operator/=", 10),
    string_view("operator%=", 10),
    string_view("operator&=", 10),
    string_view("operator|=", 10),
    string_view("operator^=", 10),
    string_view("operator<<=", 11),
    string_view("operator>>=", 11),
    string_view("operator==", 10),
    string_view("operator!=", 10),
    string_view("operator<", 9),
    string_view("operator>", 9),
    string_view("operator<=", 10),
    string_view("operator>=", 10),
    string_view("operator~", 9),
    string_view("operator++", 10),
    string_view("operator--", 10),
    string_view("operator=", 9),
    string_view("operator[]", 10),
    string_view("function390", 11),
    string_view("determinant", 11),
    string_view("adjugate", 8),
    string_view("inverse", 7),
    string_view("trace", 5),
    string_view("function395", 11),
    string_view("function396", 11),
    string_view("function397", 11),
    string_view("function398", 11),
    string_view("function399", 11),
    string_view("function400", 11),
    string_view("function401", 11),
    string_view("function402", 11),
    string_view("function403", 11),
};

static const lobster::VMField float2_fields0[] = {
    { string_view("x", 1), 0 },
    { string_view("y", 1), 1 },
};

static const lobster::VMField int2_fields1[] = {
    { string_view("x", 1), 0 },
    { string_view("y", 1), 1 },
};

static const lobster::VMField float3_fields2[] = {
    { string_view("x", 1), 0 },
    { string_view("y", 1), 1 },
    { string_view("z", 1), 2 },
};

static const lobster::VMField int3_fields3[] = {
    { string_view("x", 1), 0 },
    { string_view("y", 1), 1 },
    { string_view("z", 1), 2 },
};

static const lobster::VMField float4_fields4[] = {
    { string_view("x", 1), 0 },
    { string_view("y", 1), 1 },
    { string_view("z", 1), 2 },
    { string_view("w", 1), 3 },
};

static const lobster::VMField int4_fields5[] = {
    { string_view("x", 1), 0 },
    { string_view("y", 1), 1 },
    { string_view("z", 1), 2 },
    { string_view("w", 1), 3 },
};

static const lobster::VMField A_fields6[] = {
    { string_view("x", 1), 0 },
    { string_view("y", 1), 1 },
};

static const lobster::VMField B_fields7[] = {
    { string_view("a", 1), 0 },
    { string_view("b", 1), 2 },
};

static const lobster::VMField C_fields8[] = {
    { string_view("a", 1), 0 },
    { string_view("b", 1), 2 },
};

static const lobster::VMField C_fields9[] = {
    { string_view("g", 1), 0 },
    { string_view("f", 1), 3 },
};

static const lobster::VMField S_fields10[] = {
    { string_view("g", 1), 0 },
    { string_view("f", 1), 3 },
};

static const lobster::VMField xy_s_fields11[] = {
    { string_view("x", 1), 0 },
    { string_view("y", 1), 1 },
};

static const lobster::VMField mm1_fields12[] = {
    { string_view("m", 1), 0 },
};

static const lobster::VMField mm2_fields13[] = {
    { string_view("m", 1), 0 },
};

static const lobster::VMField R_fields14[] = {
    { string_view("a", 1), 0 },
    { string_view("b", 1), 1 },
};

static const lobster::VMField RN_fields15[] = {
    { string_view("a", 1), 0 },
    { string_view("r", 1), 1 },
};

static const lobster::VMField RS_fields16[] = {
    { string_view("a", 1), 0 },
    { string_view("b", 1), 1 },
};

static const lobster::VMField V1_fields17[] = {
    { string_view("x", 1), 0 },
};

static const lobster::VMField V5_fields18[] = {
    { string_view("x", 1), 0 },
    { string_view("y", 1), 1 },
    { string_view("z", 1), 2 },
    { string_view("w", 1), 3 },
    { string_view("v", 1), 4 },
};

static const lobster::VMField A_fields19[] = {
    { string_view("a", 1), 0 },
    { string_view("b", 1), 1 },
    { string_view("c", 1), 2 },
    { string_view("d", 1), 3 },
    { string_view("e", 1), 4 },
};

static const lobster::VMField test_namespace_g_fields20[] = {
    { string_view("x", 1), 0 },
};

static const lobster::VMField testa_fields21[] = {
    { string_view("a", 1), 0 },
};

static const lobster::VMField testb_fields22[] = {
    { string_view("a", 1), 0 },
    { string_view("b", 1), 1 },
};

static const lobster::VMField parsetest_fields23[] = {
    { string_view("h", 1), 0 },
    { string_view("a", 1), 1 },
    { string_view("b", 1), 2 },
    { string_view("c", 1), 3 },
    { string_view("d", 1), 6 },
    { string_view("e", 1), 7 },
    { string_view("f", 1), 8 },
    { string_view("g", 1), 9 },
    { string_view("s", 1), 10 },
};

static const lobster::VMField mretfields_fields24[] = {
    { string_view("a", 1), 0 },
    { string_view("b", 1), 1 },
};

static const lobster::VMField STLV_fields25[] = {
    { string_view("a", 1), 0 },
    { string_view("b", 1), 1 },
};

static const lobster::VMField Reflect_fields26[] = {
    { string_view("c", 1), 0 },
    { string_view("s", 1), 2 },
    { string_view("v", 1), 3 },
};

static const lobster::VMField testa_fields27[] = {
    { string_view("a", 1), 0 },
};

static const lobster::VMField testb_fields28[] = {
    { string_view("a", 1), 0 },
    { string_view("b", 1), 1 },
};

static const lobster::VMField S1_fields29[] = {
    { string_view("a", 1), 0 },
};

static const lobster::VMField S2_fields30[] = {
    { string_view("a", 1), 0 },
    { string_view("b", 1), 1 },
};

static const lobster::VMField C1_fields32[] = {
    { string_view("a", 1), 0 },
};

static const lobster::VMField C2_fields33[] = {
    { string_view("a", 1), 0 },
    { string_view("b", 1), 1 },
};

static const lobster::VMField Ai_fields34[] = {
    { string_view("ts", 2), 0 },
    { string_view("next", 4), 1 },
    { string_view("nexts", 5), 2 },
    { string_view("nexti", 5), 3 },
    { string_view("nexto", 5), 4 },
};

static const lobster::VMField A_fields35[] = {
    { string_view("ts", 2), 0 },
    { string_view("next", 4), 1 },
    { string_view("nexts", 5), 2 },
    { string_view("nexti", 5), 3 },
    { string_view("nexto", 5), 4 },
};

static const lobster::VMField A_fields36[] = {
    { string_view("a", 1), 0 },
};

static const lobster::VMField B_fields37[] = {
    { string_view("a", 1), 0 },
    { string_view("b", 1), 1 },
};

static const lobster::VMField A_fields38[] = {
    { string_view("a", 1), 0 },
};

static const lobster::VMField A_fields39[] = {
    { string_view("a", 1), 0 },
    { string_view("b", 1), 1 },
    { string_view("c", 1), 2 },
};

static const lobster::VMField pathingcell_fields41[] = {
    { string_view("G", 1), 0 },
    { string_view("H", 1), 1 },
    { string_view("F", 1), 2 },
    { string_view("previous", 8), 3 },
    { string_view("state", 5), 4 },
    { string_view("delta", 5), 6 },
    { string_view("open", 4), 8 },
    { string_view("closed", 6), 9 },
    { string_view("path", 4), 10 },
    { string_view("c", 1), 11 },
};

static const lobster::VMField astar_node_fields42[] = {
    { string_view("G", 1), 0 },
    { string_view("H", 1), 1 },
    { string_view("F", 1), 2 },
    { string_view("previous", 8), 3 },
    { string_view("state", 5), 4 },
    { string_view("delta", 5), 6 },
    { string_view("open", 4), 8 },
    { string_view("closed", 6), 9 },
};

static const lobster::VMField resources_fields43[] = {
    { string_view("wolves", 6), 0 },
    { string_view("skins", 5), 1 },
    { string_view("money", 5), 2 },
    { string_view("flour", 5), 3 },
    { string_view("bread", 5), 4 },
    { string_view("pizza", 5), 5 },
    { string_view("hungry", 6), 6 },
};

static const lobster::VMField spec_node_fields50[] = {
    { string_view("G", 1), 0 },
    { string_view("H", 1), 1 },
    { string_view("F", 1), 2 },
    { string_view("previous", 8), 3 },
    { string_view("state", 5), 4 },
    { string_view("delta", 5), 5 },
    { string_view("open", 4), 6 },
    { string_view("closed", 6), 7 },
};

static const lobster::VMField astar_node_fields51[] = {
    { string_view("G", 1), 0 },
    { string_view("H", 1), 1 },
    { string_view("F", 1), 2 },
    { string_view("previous", 8), 3 },
    { string_view("state", 5), 4 },
    { string_view("delta", 5), 5 },
    { string_view("open", 4), 6 },
    { string_view("closed", 6), 7 },
};

static const lobster::VMField integer_fields53[] = {
    { string_view("i", 1), 0 },
};

static const lobster::VMField inst_fields54[] = {
    { string_view("atom", 4), 0 },
    { string_view("args", 4), 1 },
};

static const lobster::VMField pathmap_fields55[] = {
    { string_view("steps", 5), 0 },
    { string_view("dir", 3), 1 },
    { string_view("pos", 3), 3 },
};

static const lobster::VMField cell_fields56[] = {
    { string_view("h", 1), 0 },
    { string_view("wh", 2), 1 },
    { string_view("nw", 2), 2 },
};

static const lobster::VMField triangle_fields57[] = {
    { string_view("verts", 5), 0 },
    { string_view("neighbors", 9), 1 },
};

static const lobster::VMField node_fields58[] = {
    { string_view("t1", 2), 0 },
    { string_view("t2", 2), 1 },
};

static const lobster::VMField pedge_fields59[] = {
    { string_view("i1", 2), 0 },
    { string_view("i2", 2), 1 },
    { string_view("dist", 4), 2 },
};

static const lobster::VMField color_fields60[] = {
    { string_view("red", 3), 0 },
    { string_view("green", 5), 1 },
    { string_view("blue", 4), 2 },
    { string_view("alpha", 5), 3 },
};

static const lobster::VMField Ray_fields61[] = {
    { string_view("o", 1), 0 },
    { string_view("d", 1), 3 },
};

static const lobster::VMField Sphere_fields62[] = {
    { string_view("rad", 3), 0 },
    { string_view("p", 1), 1 },
    { string_view("e", 1), 4 },
    { string_view("c", 1), 7 },
    { string_view("refl", 4), 10 },
};

static const lobster::VMField Nest_fields63[] = {
    { string_view("a", 1), 0 },
    { string_view("b", 1), 1 },
    { string_view("c", 1), 2 },
    { string_view("d", 1), 4 },
    { string_view("e", 1), 5 },
    { string_view("f", 1), 6 },
};

static const lobster::VMField quat_fields64[] = {
    { string_view("x", 1), 0 },
    { string_view("y", 1), 1 },
    { string_view("z", 1), 2 },
    { string_view("w", 1), 3 },
};

static const lobster::VMField OC_fields65[] = {
    { string_view("a", 1), 0 },
};

static const lobster::VMField OS_fields66[] = {
    { string_view("a", 1), 0 },
};

static const lobster::VMField mat2x2_fields67[] = {
    { string_view("x", 1), 0 },
    { string_view("y", 1), 1 },
    { string_view("z", 1), 2 },
    { string_view("w", 1), 3 },
};

static const lobster::VMField A_fields68[] = {
    { string_view("a", 1), 0 },
};

static const lobster::VMField B_fields69[] = {
    { string_view("a", 1), 0 },
    { string_view("b", 1), 1 },
};

static const lobster::VMField C_fields70[] = {
    { string_view("a", 1), 0 },
    { string_view("c", 1), 1 },
};

static const lobster::VMField D_fields71[] = {
    { string_view("a", 1), 0 },
    { string_view("c", 1), 1 },
    { string_view("d", 1), 2 },
};

static const lobster::VMField E_fields72[] = {
    { string_view("a", 1), 0 },
    { string_view("c", 1), 1 },
    { string_view("e", 1), 2 },
};

static const lobster::VMField F_fields73[] = {
    { string_view("a", 1), 0 },
    { string_view("c", 1), 1 },
    { string_view("e", 1), 2 },
    { string_view("f", 1), 3 },
};

static const lobster::VMField dictionary_fields74[] = {
    { string_view("buckets", 7), 0 },
};

static const lobster::VMField chain_fields75[] = {
    { string_view("next", 4), 0 },
    { string_view("key", 3), 1 },
    { string_view("value", 5), 2 },
};

static const lobster::VMField dictionary_fields76[] = {
    { string_view("buckets", 7), 0 },
};

static const lobster::VMField chain_fields77[] = {
    { string_view("next", 4), 0 },
    { string_view("key", 3), 1 },
    { string_view("value", 5), 2 },
};

static const lobster::VMField dictionary_fields78[] = {
    { string_view("buckets", 7), 0 },
};

static const lobster::VMField chain_fields79[] = {
    { string_view("next", 4), 0 },
    { string_view("key", 3), 1 },
    { string_view("value", 5), 4 },
};

static const lobster::VMField dictionary_fields80[] = {
    { string_view("buckets", 7), 0 },
};

static const lobster::VMField chain_fields81[] = {
    { string_view("next", 4), 0 },
    { string_view("key", 3), 1 },
    { string_view("value", 5), 2 },
};

static const lobster::VMField dictionary_fields82[] = {
    { string_view("buckets", 7), 0 },
};

static const lobster::VMField chain_fields83[] = {
    { string_view("next", 4), 0 },
    { string_view("key", 3), 1 },
    { string_view("value", 5), 2 },
};

static const lobster::VMField C3_fields84[] = {
    { string_view("a", 1), 0 },
    { string_view("b", 1), 1 },
};

static const lobster::VMField D_fields85[] = {
    { string_view("x", 1), 0 },
};

static const lobster::VMField D_fields86[] = {
    { string_view("x", 1), 0 },
};

static const lobster::VMField set1_fields87[] = {
    { string_view("dict", 4), 0 },
};

static const lobster::VMField set2_fields88[] = {
    { string_view("dict", 4), 0 },
};

static const lobster::VMUDT udts[] = {
    { string_view("float2", 6), 0, 2, -1, 162, span(float2_fields0) },
    { string_view("int2", 4), 1, 2, -1, 25, span(int2_fields1) },
    { string_view("float3", 6), 2, 3, 0, 174, span(float3_fields2) },
    { string_view("int3", 4), 3, 3, 1, 207, span(int3_fields3) },
    { string_view("float4", 6), 4, 4, 2, 189, span(float4_fields4) },
    { string_view("int4", 4), 5, 4, 3, 222, span(int4_fields5) },
    { string_view("A", 1), 6, 2, -1, 279, span(A_fields6) },
    { string_view("B", 1), 7, 3, -1, 264, span(B_fields7) },
    { string_view("C", 1), 8, 3, -1, 291, span(C_fields8) },
    { string_view("C", 1), 9, 5, -1, 318, span(C_fields9) },
    { string_view("S", 1), 10, 5, -1, 339, span(S_fields10) },
    { string_view("xy_s", 4), 11, 2, -1, 360, span(xy_s_fields11) },
    { string_view("mm1", 3), 12, 1, -1, 372, span(mm1_fields12) },
    { string_view("mm2", 3), 13, 1, 12, 381, span(mm2_fields13) },
    { string_view("R", 1), 14, 2, -1, 390, span(R_fields14) },
    { string_view("RN", 2), 15, 3, -1, 404, span(RN_fields15) },
    { string_view("RS", 2), 16, 2, -1, 421, span(RS_fields16) },
    { string_view("V1", 2), 17, 1, -1, 1539, span(V1_fields17) },
    { string_view("V5", 2), 18, 5, -1, 433, span(V5_fields18) },
    { string_view("A", 1), 19, 5, -1, 1548, span(A_fields19) },
    { string_view("test_namespace.g", 16), 20, 1, -1, 544, span(test_namespace_g_fields20) },
    { string_view("testa", 5), 21, 1, -1, 553, span(testa_fields21) },
    { string_view("testb", 5), 22, 2, 21, 562, span(testb_fields22) },
    { string_view("parsetest", 9), 23, 11, -1, 574, span(parsetest_fields23) },
    { string_view("mretfields", 10), 24, 2, -1, 619, span(mretfields_fields24) },
    { string_view("STLV", 4), 25, 2, -1, 699, span(STLV_fields25) },
    { string_view("Reflect", 7), 26, 4, -1, 711, span(Reflect_fields26) },
    { string_view("testa", 5), 27, 1, -1, 1569, span(testa_fields27) },
    { string_view("testb", 5), 28, 2, 27, 1578, span(testb_fields28) },
    { string_view("S1", 2), 29, 1, -1, 735, span(S1_fields29) },
    { string_view("S2", 2), 30, 2, 29, 744, span(S2_fields30) },
    { string_view("C0", 2), 31, 0, -1, 756, {} },
    { string_view("C1", 2), 32, 1, 31, 762, span(C1_fields32) },
    { string_view("C2", 2), 33, 2, 32, 771, span(C2_fields33) },
    { string_view("Ai", 2), 34, 5, -1, 804, span(Ai_fields34) },
    { string_view("A", 1), 35, 5, -1, 829, span(A_fields35) },
    { string_view("A", 1), 36, 1, -1, 872, span(A_fields36) },
    { string_view("B", 1), 37, 2, 36, 883, span(B_fields37) },
    { string_view("A", 1), 38, 1, -1, 1590, span(A_fields38) },
    { string_view("A", 1), 39, 4, -1, 897, span(A_fields39) },
    { string_view("goapaction", 10), 40, 0, -1, 138, {} },
    { string_view("pathingcell", 11), 41, 12, 42, 919, span(pathingcell_fields41) },
    { string_view("astar_node", 10), 42, 10, -1, 961, span(astar_node_fields42) },
    { string_view("resources", 9), 43, 7, -1, 111, span(resources_fields43) },
    { string_view("KillWolf", 8), 44, 0, 40, 1029, {} },
    { string_view("SellSkin", 8), 45, 0, 40, 1035, {} },
    { string_view("BuyPizza", 8), 46, 0, 40, 1041, {} },
    { string_view("BuyFlour", 8), 47, 0, 40, 1047, {} },
    { string_view("BakeBread", 9), 48, 0, 40, 1053, {} },
    { string_view("Eat", 3), 49, 0, 40, 1059, {} },
    { string_view("spec_node", 9), 50, 8, 51, 49, span(spec_node_fields50) },
    { string_view("astar_node", 10), 51, 8, -1, 79, span(astar_node_fields51) },
    { string_view("node", 4), 52, 0, -1, 1074, {} },
    { string_view("integer", 7), 53, 1, 52, 1065, span(integer_fields53) },
    { string_view("inst", 4), 54, 2, 52, 1080, span(inst_fields54) },
    { string_view("pathmap", 7), 55, 5, -1, 1098, span(pathmap_fields55) },
    { string_view("cell", 4), 56, 3, -1, 1123, span(cell_fields56) },
    { string_view("triangle", 8), 57, 2, -1, 1599, span(triangle_fields57) },
    { string_view("node", 4), 58, 2, -1, 1611, span(node_fields58) },
    { string_view("pedge", 5), 59, 3, -1, 1150, span(pedge_fields59) },
    { string_view("color", 5), 60, 4, -1, 1167, span(color_fields60) },
    { string_view("Ray", 3), 61, 6, -1, 1226, span(Ray_fields61) },
    { string_view("Sphere", 6), 62, 11, -1, 1185, span(Sphere_fields62) },
    { string_view("Nest", 4), 63, 7, -1, 1260, span(Nest_fields63) },
    { string_view("quat", 4), 64, 4, 4, 1291, span(quat_fields64) },
    { string_view("OC", 2), 65, 1, -1, 1309, span(OC_fields65) },
    { string_view("OS", 2), 66, 1, -1, 1318, span(OS_fields66) },
    { string_view("mat2x2", 6), 67, 4, 4, 242, span(mat2x2_fields67) },
    { string_view("A", 1), 68, 1, -1, 1327, span(A_fields68) },
    { string_view("B", 1), 69, 2, 68, 1623, span(B_fields69) },
    { string_view("C", 1), 70, 2, 68, 1650, span(C_fields70) },
    { string_view("D", 1), 71, 3, 70, 1635, span(D_fields71) },
    { string_view("E", 1), 72, 3, 70, 1662, span(E_fields72) },
    { string_view("F", 1), 73, 4, 72, 1677, span(F_fields73) },
    { string_view("dictionary", 10), 74, 1, -1, 460, span(dictionary_fields74) },
    { string_view("chain", 5), 75, 3, -1, 469, span(chain_fields75) },
    { string_view("dictionary", 10), 76, 1, -1, 631, span(dictionary_fields76) },
    { string_view("chain", 5), 77, 3, -1, 640, span(chain_fields77) },
    { string_view("dictionary", 10), 78, 1, -1, 659, span(dictionary_fields78) },
    { string_view("chain", 5), 79, 7, -1, 668, span(chain_fields79) },
    { string_view("dictionary", 10), 80, 1, -1, 514, span(dictionary_fields80) },
    { string_view("chain", 5), 81, 3, -1, 523, span(chain_fields81) },
    { string_view("dictionary", 10), 82, 1, -1, 488, span(dictionary_fields82) },
    { string_view("chain", 5), 83, 3, -1, 497, span(chain_fields83) },
    { string_view("C3", 2), 84, 2, 32, 783, span(C3_fields84) },
    { string_view("D", 1), 85, 1, -1, 795, span(D_fields85) },
    { string_view("D", 1), 86, 1, -1, 1402, span(D_fields86) },
    { string_view("set1", 4), 87, 1, -1, 854, span(set1_fields87) },
    { string_view("set2", 4), 88, 1, -1, 863, span(set2_fields88) },
};

static const lobster::VMSpecIdent specidents[] = {
    { string_view("current_test_runner", 19), 2, 19, 1, 0, 1 },
    { string_view("name", 4), 3, 3, 0, 0, 0 },
    { string_view("t", 1), 4, 21, 0, 0, 0 },
    { string_view("name", 4), 5, 3, 0, 0, 0 },
    { string_view("t", 1), 6, 21, 0, 0, 0 },
    { string_view("name", 4), 7, 3, 0, 0, 0 },
    { string_view("t", 1), 8, 21, 0, 0, 0 },
    { string_view("xs", 2), 9, 0, 0, 0, 0 },
    { string_view("fun", 3), 10, 23, 0, 0, 0 },
    { string_view("r", 1), 11, 11, 0, 1, 0 },
    { string_view("x", 1), 12, 0, 0, 0, 0 },
    { string_view("i", 1), 13, 0, 0, 0, 0 },
    { string_view("xs", 2), 18, 37, 0, 0, 0 },
    { string_view("fun", 3), 19, 39, 0, 0, 0 },
    { string_view("r", 1), 20, 37, 0, 1, 0 },
    { string_view("x", 1), 21, 25, 0, 0, 0 },
    { string_view("x", 1), 21, 25, 0, 0, 0 },
    { string_view("i", 1), 22, 0, 0, 0, 0 },
    { string_view("xs", 2), 34, 7, 0, 0, 0 },
    { string_view("fun", 3), 35, 41, 0, 0, 0 },
    { string_view("t", 1), 36, 7, 0, 1, 0 },
    { string_view("f", 1), 37, 7, 0, 1, 0 },
    { string_view("x", 1), 38, 0, 0, 0, 0 },
    { string_view("i", 1), 39, 0, 0, 0, 0 },
    { string_view("xs", 2), 46, 3, 0, 0, 0 },
    { string_view("fun", 3), 47, 43, 0, 0, 0 },
    { string_view("x", 1), 48, 0, 0, 0, 0 },
    { string_view("i", 1), 49, 0, 0, 0, 0 },
    { string_view("xs", 2), 51, 7, 0, 0, 0 },
    { string_view("acc", 3), 52, 0, 0, 0, 0 },
    { string_view("fun", 3), 53, 45, 0, 0, 0 },
    { string_view("_", 1), 54, 0, 0, 1, 0 },
    { string_view("xs", 2), 60, 11, 0, 0, 0 },
    { string_view("fun", 3), 61, 47, 0, 0, 0 },
    { string_view("acc", 3), 62, 3, 0, 0, 0 },
    { string_view("i", 1), 63, 0, 0, 0, 0 },
    { string_view("xs", 2), 71, 148, 0, 0, 0 },
    { string_view("fun", 3), 72, 150, 0, 0, 0 },
    { string_view("x", 1), 73, 49, 0, 0, 0 },
    { string_view("i", 1), 74, 0, 0, 0, 0 },
    { string_view("xs", 2), 85, 7, 0, 0, 0 },
    { string_view("fun", 3), 86, 152, 0, 0, 0 },
    { string_view("init", 4), 87, 0, 0, 1, 0 },
    { string_view("best", 4), 88, 0, 0, 0, 0 },
    { string_view("i", 1), 89, 0, 0, 0, 0 },
    { string_view("x", 1), 90, 0, 0, 0, 0 },
    { string_view("j", 1), 91, 0, 0, 0, 0 },
    { string_view("v", 1), 92, 0, 0, 1, 0 },
    { string_view("xs", 2), 103, 7, 0, 0, 0 },
    { string_view("init", 4), 104, 0, 0, 1, 0 },
    { string_view("_x", 2), 105, 0, 0, 0, 0 },
    { string_view("_y", 2), 106, 0, 0, 0, 0 },
    { string_view("xs", 2), 144, 7, 0, 0, 0 },
    { string_view("lt", 2), 145, 154, 1, 0, 0 },
    { string_view("pivot", 5), 146, 0, 1, 1, 0 },
    { string_view("tail", 4), 147, 7, 0, 1, 0 },
    { string_view("f1", 2), 148, 7, 0, 1, 0 },
    { string_view("f2", 2), 149, 7, 0, 1, 0 },
    { string_view("_", 1), 150, 0, 0, 0, 0 },
    { string_view("xs", 2), 151, 7, 1, 0, 0 },
    { string_view("lt", 2), 152, 156, 1, 0, 0 },
    { string_view("s", 1), 153, 0, 0, 0, 0 },
    { string_view("e", 1), 154, 0, 0, 0, 0 },
    { string_view("l", 1), 155, 0, 0, 1, 0 },
    { string_view("pivot", 5), 156, 0, 0, 1, 0 },
    { string_view("sp", 2), 157, 0, 0, 0, 0 },
    { string_view("ep", 2), 158, 0, 0, 0, 0 },
    { string_view("c", 1), 159, 0, 0, 1, 0 },
    { string_view("xs", 2), 160, 7, 0, 0, 0 },
    { string_view("lt", 2), 161, 158, 0, 0, 0 },
    { string_view("key", 3), 162, 0, 0, 0, 0 },
    { string_view("i", 1), 163, 0, 0, 0, 0 },
    { string_view("j", 1), 164, 0, 0, 0, 0 },
    { string_view("f", 1), 179, 160, 0, 0, 0 },
    { string_view("float2_0", 8), 203, 162, 1, 1, 1 },
    { string_view("float2_0", 8), 203, 162, 1, 1, 1 },
    { string_view("float2_1", 8), 204, 162, 1, 1, 1 },
    { string_view("float2_1", 8), 204, 162, 1, 1, 1 },
    { string_view("float2_h", 8), 205, 162, 0, 1, 1 },
    { string_view("float2_h", 8), 205, 162, 0, 1, 1 },
    { string_view("float2_x", 8), 206, 162, 0, 1, 1 },
    { string_view("float2_x", 8), 206, 162, 0, 1, 1 },
    { string_view("float2_y", 8), 207, 162, 0, 1, 1 },
    { string_view("float2_y", 8), 207, 162, 0, 1, 1 },
    { string_view("float3_0", 8), 208, 174, 1, 1, 1 },
    { string_view("float3_0", 8), 208, 174, 1, 1, 1 },
    { string_view("float3_0", 8), 208, 174, 1, 1, 1 },
    { string_view("float3_1", 8), 209, 174, 1, 1, 1 },
    { string_view("float3_1", 8), 209, 174, 1, 1, 1 },
    { string_view("float3_1", 8), 209, 174, 1, 1, 1 },
    { string_view("float3_x", 8), 210, 174, 1, 1, 1 },
    { string_view("float3_x", 8), 210, 174, 1, 1, 1 },
    { string_view("float3_x", 8), 210, 174, 1, 1, 1 },
    { string_view("float3_y", 8), 211, 174, 1, 1, 1 },
    { string_view("float3_y", 8), 211, 174, 1, 1, 1 },
    { string_view("float3_y", 8), 211, 174, 1, 1, 1 },
    { string_view("float3_z", 8), 212, 174, 0, 1, 1 },
    { string_view("float3_z", 8), 212, 174, 0, 1, 1 },
    { string_view("float3_z", 8), 212, 174, 0, 1, 1 },
    { string_view("float4_0", 8), 213, 189, 0, 1, 1 },
    { string_view("float4_0", 8), 213, 189, 0, 1, 1 },
    { string_view("float4_0", 8), 213, 189, 0, 1, 1 },
    { string_view("float4_0", 8), 213, 189, 0, 1, 1 },
    { string_view("float4_1", 8), 214, 189, 0, 1, 1 },
    { string_view("float4_1", 8), 214, 189, 0, 1, 1 },
    { string_view("float4_1", 8), 214, 189, 0, 1, 1 },
    { string_view("float4_1", 8), 214, 189, 0, 1, 1 },
    { string_view("float4_x", 8), 215, 189, 0, 1, 1 },
    { string_view("float4_x", 8), 215, 189, 0, 1, 1 },
    { string_view("float4_x", 8), 215, 189, 0, 1, 1 },
    { string_view("float4_x", 8), 215, 189, 0, 1, 1 },
    { string_view("float4_y", 8), 216, 189, 0, 1, 1 },
    { string_view("float4_y", 8), 216, 189, 0, 1, 1 },
    { string_view("float4_y", 8), 216, 189, 0, 1, 1 },
    { string_view("float4_y", 8), 216, 189, 0, 1, 1 },
    { string_view("float4_z", 8), 217, 189, 0, 1, 1 },
    { string_view("float4_z", 8), 217, 189, 0, 1, 1 },
    { string_view("float4_z", 8), 217, 189, 0, 1, 1 },
    { string_view("float4_z", 8), 217, 189, 0, 1, 1 },
    { string_view("float4_w", 8), 218, 189, 0, 1, 1 },
    { string_view("float4_w", 8), 218, 189, 0, 1, 1 },
    { string_view("float4_w", 8), 218, 189, 0, 1, 1 },
    { string_view("float4_w", 8), 218, 189, 0, 1, 1 },
    { string_view("int2_0", 6), 219, 25, 1, 1, 1 },
    { string_view("int2_0", 6), 219, 25, 1, 1, 1 },
    { string_view("int2_1", 6), 220, 25, 1, 1, 1 },
    { string_view("int2_1", 6), 220, 25, 1, 1, 1 },
    { string_view("int2_x", 6), 221, 25, 1, 1, 1 },
    { string_view("int2_x", 6), 221, 25, 1, 1, 1 },
    { string_view("int2_y", 6), 222, 25, 1, 1, 1 },
    { string_view("int2_y", 6), 222, 25, 1, 1, 1 },
    { string_view("int3_0", 6), 223, 207, 1, 1, 1 },
    { string_view("int3_0", 6), 223, 207, 1, 1, 1 },
    { string_view("int3_0", 6), 223, 207, 1, 1, 1 },
    { string_view("int3_1", 6), 224, 207, 1, 1, 1 },
    { string_view("int3_1", 6), 224, 207, 1, 1, 1 },
    { string_view("int3_1", 6), 224, 207, 1, 1, 1 },
    { string_view("int3_x", 6), 225, 207, 0, 1, 1 },
    { string_view("int3_x", 6), 225, 207, 0, 1, 1 },
    { string_view("int3_x", 6), 225, 207, 0, 1, 1 },
    { string_view("int3_y", 6), 226, 207, 0, 1, 1 },
    { string_view("int3_y", 6), 226, 207, 0, 1, 1 },
    { string_view("int3_y", 6), 226, 207, 0, 1, 1 },
    { string_view("int3_z", 6), 227, 207, 0, 1, 1 },
    { string_view("int3_z", 6), 227, 207, 0, 1, 1 },
    { string_view("int3_z", 6), 227, 207, 0, 1, 1 },
    { string_view("int4_0", 6), 228, 222, 0, 1, 1 },
    { string_view("int4_0", 6), 228, 222, 0, 1, 1 },
    { string_view("int4_0", 6), 228, 222, 0, 1, 1 },
    { string_view("int4_0", 6), 228, 222, 0, 1, 1 },
    { string_view("int4_1", 6), 229, 222, 0, 1, 1 },
    { string_view("int4_1", 6), 229, 222, 0, 1, 1 },
    { string_view("int4_1", 6), 229, 222, 0, 1, 1 },
    { string_view("int4_1", 6), 229, 222, 0, 1, 1 },
    { string_view("int4_x", 6), 230, 222, 0, 1, 1 },
    { string_view("int4_x", 6), 230, 222, 0, 1, 1 },
    { string_view("int4_x", 6), 230, 222, 0, 1, 1 },
    { string_view("int4_x", 6), 230, 222, 0, 1, 1 },
    { string_view("int4_y", 6), 231, 222, 0, 1, 1 },
    { string_view("int4_y", 6), 231, 222, 0, 1, 1 },
    { string_view("int4_y", 6), 231, 222, 0, 1, 1 },
    { string_view("int4_y", 6), 231, 222, 0, 1, 1 },
    { string_view("int4_z", 6), 232, 222, 0, 1, 1 },
    { string_view("int4_z", 6), 232, 222, 0, 1, 1 },
    { string_view("int4_z", 6), 232, 222, 0, 1, 1 },
    { string_view("int4_z", 6), 232, 222, 0, 1, 1 },
    { string_view("int4_w", 6), 233, 222, 0, 1, 1 },
    { string_view("int4_w", 6), 233, 222, 0, 1, 1 },
    { string_view("int4_w", 6), 233, 222, 0, 1, 1 },
    { string_view("int4_w", 6), 233, 222, 0, 1, 1 },
    { string_view("cardinal_directions", 19), 234, 37, 1, 1, 1 },
    { string_view("diagonal_directions", 19), 235, 37, 1, 1, 1 },
    { string_view("positive_directions", 19), 236, 37, 0, 1, 1 },
    { string_view("octant_directions", 17), 237, 240, 0, 1, 1 },
    { string_view("v", 1), 240, 242, 0, 0, 0 },
    { string_view("v", 1), 240, 242, 0, 0, 0 },
    { string_view("v", 1), 240, 242, 0, 0, 0 },
    { string_view("v", 1), 240, 242, 0, 0, 0 },
    { string_view("v", 1), 241, 242, 0, 0, 0 },
    { string_view("v", 1), 241, 242, 0, 0, 0 },
    { string_view("v", 1), 241, 242, 0, 0, 0 },
    { string_view("v", 1), 241, 242, 0, 0, 0 },
    { string_view("v", 1), 242, 242, 0, 0, 0 },
    { string_view("v", 1), 242, 242, 0, 0, 0 },
    { string_view("v", 1), 242, 242, 0, 0, 0 },
    { string_view("v", 1), 242, 242, 0, 0, 0 },
    { string_view("v", 1), 254, 174, 0, 0, 0 },
    { string_view("v", 1), 254, 174, 0, 0, 0 },
    { string_view("v", 1), 254, 174, 0, 0, 0 },
    { string_view("w", 1), 255, 2, 0, 0, 0 },
    { string_view("n", 1), 266, 25, 0, 0, 0 },
    { string_view("n", 1), 266, 25, 0, 0, 0 },
    { string_view("v", 1), 269, 25, 0, 0, 0 },
    { string_view("v", 1), 269, 25, 0, 0, 0 },
    { string_view("fun", 3), 270, 260, 0, 0, 0 },
    { string_view("y", 1), 271, 0, 0, 0, 0 },
    { string_view("x", 1), 272, 0, 0, 0, 0 },
    { string_view("v", 1), 277, 25, 1, 0, 0 },
    { string_view("v", 1), 277, 25, 1, 0, 0 },
    { string_view("fun", 3), 278, 262, 1, 0, 0 },
    { string_view("y", 1), 279, 0, 0, 0, 0 },
    { string_view("x", 1), 280, 0, 0, 0, 0 },
    { string_view("a", 1), 299, 25, 0, 0, 0 },
    { string_view("a", 1), 299, 25, 0, 0, 0 },
    { string_view("b", 1), 300, 264, 0, 1, 0 },
    { string_view("b", 1), 300, 264, 0, 1, 0 },
    { string_view("b", 1), 300, 264, 0, 1, 0 },
    { string_view("aa", 2), 301, 25, 0, 0, 0 },
    { string_view("aa", 2), 301, 25, 0, 0, 0 },
    { string_view("c", 1), 302, 291, 0, 1, 0 },
    { string_view("nv", 2), 303, 162, 0, 0, 0 },
    { string_view("nv", 2), 303, 162, 0, 0, 0 },
    { string_view("v", 1), 304, 37, 0, 1, 0 },
    { string_view("tri", 3), 305, 306, 0, 1, 0 },
    { string_view("triv", 4), 306, 310, 0, 1, 0 },
    { string_view("trivv", 5), 307, 316, 0, 1, 0 },
    { string_view("a", 1), 308, 25, 0, 1, 0 },
    { string_view("a", 1), 308, 25, 0, 1, 0 },
    { string_view("b", 1), 309, 25, 0, 1, 0 },
    { string_view("b", 1), 309, 25, 0, 1, 0 },
    { string_view("c", 1), 310, 318, 0, 1, 0 },
    { string_view("s", 1), 311, 339, 0, 1, 0 },
    { string_view("s", 1), 311, 339, 0, 1, 0 },
    { string_view("s", 1), 311, 339, 0, 1, 0 },
    { string_view("s", 1), 311, 339, 0, 1, 0 },
    { string_view("s", 1), 311, 339, 0, 1, 0 },
    { string_view("v", 1), 312, 37, 0, 1, 0 },
    { string_view("st", 2), 313, 25, 0, 0, 0 },
    { string_view("st", 2), 313, 25, 0, 0, 0 },
    { string_view("st", 2), 314, 360, 0, 0, 0 },
    { string_view("st", 2), 314, 360, 0, 0, 0 },
    { string_view("a", 1), 315, 0, 0, 0, 0 },
    { string_view("b", 1), 316, 25, 0, 0, 0 },
    { string_view("b", 1), 316, 25, 0, 0, 0 },
    { string_view("c", 1), 317, 0, 0, 0, 0 },
    { string_view("a", 1), 318, 0, 0, 0, 0 },
    { string_view("mr1", 3), 319, 0, 0, 1, 0 },
    { string_view("mr2", 3), 320, 25, 0, 1, 0 },
    { string_view("mr2", 3), 320, 25, 0, 1, 0 },
    { string_view("mr3", 3), 321, 0, 0, 1, 0 },
    { string_view("a", 1), 322, 372, 0, 0, 0 },
    { string_view("b", 1), 323, 25, 0, 0, 0 },
    { string_view("b", 1), 323, 25, 0, 0, 0 },
    { string_view("a", 1), 324, 381, 0, 0, 0 },
    { string_view("b", 1), 325, 25, 0, 0, 0 },
    { string_view("b", 1), 325, 25, 0, 0, 0 },
    { string_view("r", 1), 326, 390, 0, 1, 0 },
    { string_view("r", 1), 326, 390, 0, 1, 0 },
    { string_view("x", 1), 327, 390, 0, 0, 0 },
    { string_view("x", 1), 327, 390, 0, 0, 0 },
    { string_view("fr", 2), 328, 390, 0, 0, 0 },
    { string_view("fr", 2), 328, 390, 0, 0, 0 },
    { string_view("rs", 2), 329, 402, 0, 1, 0 },
    { string_view("rn", 2), 330, 404, 0, 1, 0 },
    { string_view("rn", 2), 330, 404, 0, 1, 0 },
    { string_view("rn", 2), 330, 404, 0, 1, 0 },
    { string_view("x", 1), 331, 404, 0, 0, 0 },
    { string_view("x", 1), 331, 404, 0, 0, 0 },
    { string_view("x", 1), 331, 404, 0, 0, 0 },
    { string_view("frn", 3), 332, 404, 0, 0, 0 },
    { string_view("frn", 3), 332, 404, 0, 0, 0 },
    { string_view("frn", 3), 332, 404, 0, 0, 0 },
    { string_view("rns", 3), 333, 419, 0, 1, 0 },
    { string_view("rs", 2), 334, 421, 0, 0, 0 },
    { string_view("rs", 2), 334, 421, 0, 0, 0 },
    { string_view("x", 1), 335, 421, 0, 0, 0 },
    { string_view("x", 1), 335, 421, 0, 0, 0 },
    { string_view("v5test", 6), 336, 433, 0, 1, 0 },
    { string_view("v5test", 6), 336, 433, 0, 1, 0 },
    { string_view("v5test", 6), 336, 433, 0, 1, 0 },
    { string_view("v5test", 6), 336, 433, 0, 1, 0 },
    { string_view("v5test", 6), 336, 433, 0, 1, 0 },
    { string_view("v5test2", 7), 337, 433, 0, 1, 0 },
    { string_view("v5test2", 7), 337, 433, 0, 1, 0 },
    { string_view("v5test2", 7), 337, 433, 0, 1, 0 },
    { string_view("v5test2", 7), 337, 433, 0, 1, 0 },
    { string_view("v5test2", 7), 337, 433, 0, 1, 0 },
    { string_view("body", 4), 338, 454, 0, 0, 0 },
    { string_view("catch", 5), 339, 456, 0, 0, 0 },
    { string_view("err", 3), 340, 458, 0, 1, 0 },
    { string_view("stack_trace", 11), 341, 3, 0, 1, 0 },
    { string_view("body", 4), 342, 454, 0, 0, 0 },
    { string_view("v", 1), 343, 3, 0, 0, 0 },
    { string_view("body", 4), 348, 454, 0, 0, 0 },
    { string_view("_", 1), 349, 3, 0, 0, 0 },
    { string_view("this", 4), 353, 460, 0, 0, 0 },
    { string_view("key", 3), 354, 2, 0, 0, 0 },
    { string_view("h", 1), 355, 0, 0, 1, 0 },
    { string_view("c", 1), 356, 484, 0, 0, 0 },
    { string_view("this", 4), 357, 460, 0, 0, 0 },
    { string_view("key", 3), 358, 2, 0, 0, 0 },
    { string_view("value", 5), 359, 2, 0, 0, 0 },
    { string_view("c", 1), 360, 484, 0, 1, 0 },
    { string_view("h", 1), 361, 0, 0, 1, 0 },
    { string_view("this", 4), 373, 488, 0, 0, 0 },
    { string_view("key", 3), 374, 3, 0, 0, 0 },
    { string_view("c", 1), 375, 512, 0, 1, 0 },
    { string_view("this", 4), 376, 460, 0, 0, 0 },
    { string_view("key", 3), 377, 2, 0, 0, 0 },
    { string_view("default_val", 11), 378, 2, 0, 0, 0 },
    { string_view("c", 1), 379, 484, 0, 1, 0 },
    { string_view("size", 4), 399, 0, 0, 0, 0 },
    { string_view("this", 4), 400, 544, 0, 0, 0 },
    { string_view("vardef", 6), 401, 0, 0, 0, 0 },
    { string_view("constdef", 8), 402, 0, 0, 1, 0 },
    { string_view("x", 1), 403, 0, 0, 0, 0 },
    { string_view("y", 1), 404, 0, 0, 0, 0 },
    { string_view("x", 1), 405, 3, 0, 0, 0 },
    { string_view("y", 1), 406, 0, 0, 0, 0 },
    { string_view("x", 1), 407, 553, 0, 0, 0 },
    { string_view("y", 1), 408, 0, 0, 0, 0 },
    { string_view("x", 1), 409, 562, 0, 0, 0 },
    { string_view("y", 1), 410, 0, 0, 0, 0 },
    { string_view("a", 1), 411, 553, 0, 0, 0 },
    { string_view("direct", 6), 412, 574, 0, 1, 0 },
    { string_view("parsed", 6), 413, 613, 0, 0, 0 },
    { string_view("err", 3), 414, 458, 0, 0, 0 },
    { string_view("groundv", 7), 415, 574, 0, 1, 0 },
    { string_view("flex", 4), 416, 3, 0, 1, 0 },
    { string_view("json", 4), 417, 458, 0, 1, 0 },
    { string_view("groundj", 7), 418, 3, 0, 1, 0 },
    { string_view("flex2", 5), 419, 3, 0, 1, 0 },
    { string_view("ferr", 4), 420, 458, 0, 1, 0 },
    { string_view("json2", 5), 421, 458, 0, 1, 0 },
    { string_view("fval", 4), 422, 613, 0, 1, 0 },
    { string_view("fverr", 5), 423, 458, 0, 1, 0 },
    { string_view("lb", 2), 424, 3, 0, 1, 0 },
    { string_view("lbval", 5), 425, 613, 0, 1, 0 },
    { string_view("lberr", 5), 426, 458, 0, 1, 0 },
    { string_view("unicodetests", 12), 427, 7, 0, 1, 0 },
    { string_view("st", 2), 428, 11, 0, 1, 0 },
    { string_view("i", 1), 429, 0, 0, 0, 0 },
    { string_view("x", 1), 430, 0, 0, 1, 0 },
    { string_view("a", 1), 431, 0, 0, 0, 0 },
    { string_view("b", 1), 432, 7, 0, 1, 0 },
    { string_view("testvector", 10), 433, 7, 0, 1, 0 },
    { string_view("sorted1", 7), 434, 7, 0, 1, 0 },
    { string_view("_a", 2), 435, 0, 0, 0, 0 },
    { string_view("_b", 2), 436, 0, 0, 0, 0 },
    { string_view("sorted2", 7), 437, 7, 0, 1, 0 },
    { string_view("_a", 2), 438, 0, 0, 0, 0 },
    { string_view("_b", 2), 439, 0, 0, 0, 0 },
    { string_view("sorted3", 7), 440, 7, 0, 1, 0 },
    { string_view("_a", 2), 441, 0, 0, 0, 0 },
    { string_view("_b", 2), 442, 0, 0, 0, 0 },
    { string_view("found", 5), 443, 0, 0, 0, 0 },
    { string_view("findex", 6), 444, 0, 0, 0, 0 },
    { string_view("_", 1), 445, 0, 0, 0, 0 },
    { string_view("n", 1), 446, 0, 0, 0, 0 },
    { string_view("applyto4", 8), 447, 615, 0, 1, 0 },
    { string_view("f", 1), 448, 617, 0, 0, 0 },
    { string_view("x", 1), 449, 0, 0, 0, 0 },
    { string_view("ns", 2), 450, 3, 0, 0, 0 },
    { string_view("ni", 2), 451, 0, 0, 0, 0 },
    { string_view("s1", 2), 452, 3, 0, 0, 0 },
    { string_view("s2", 2), 453, 3, 0, 0, 0 },
    { string_view("aa", 2), 454, 0, 0, 0, 0 },
    { string_view("bb", 2), 455, 0, 0, 1, 0 },
    { string_view("cc", 2), 456, 0, 0, 1, 0 },
    { string_view("dd", 2), 457, 0, 0, 1, 0 },
    { string_view("ee", 2), 458, 0, 0, 1, 0 },
    { string_view("ff", 2), 459, 0, 0, 0, 0 },
    { string_view("gg", 2), 460, 0, 0, 0, 0 },
    { string_view("hh", 2), 461, 0, 0, 1, 0 },
    { string_view("ii", 2), 462, 0, 0, 1, 0 },
    { string_view("ll", 2), 463, 0, 0, 1, 0 },
    { string_view("jj", 2), 464, 0, 0, 1, 0 },
    { string_view("kk", 2), 465, 0, 0, 1, 0 },
    { string_view("nn", 2), 466, 2, 0, 1, 0 },
    { string_view("mm", 2), 467, 3, 0, 1, 0 },
    { string_view("this", 4), 468, 619, 0, 0, 0 },
    { string_view("n", 1), 469, 0, 0, 0, 0 },
    { string_view("a", 1), 470, 0, 0, 0, 0 },
    { string_view("i", 1), 471, 0, 0, 0, 0 },
    { string_view("n", 1), 472, 0, 1, 0, 0 },
    { string_view("v", 1), 473, 3, 0, 0, 0 },
    { string_view("_x", 2), 474, 0, 0, 0, 0 },
    { string_view("_y", 2), 475, 0, 0, 0, 0 },
    { string_view("bit", 3), 476, 0, 0, 0, 0 },
    { string_view("dict", 4), 477, 460, 0, 1, 0 },
    { string_view("key", 3), 478, 2, 0, 1, 0 },
    { string_view("key", 3), 479, 2, 0, 1, 0 },
    { string_view("value", 5), 480, 2, 0, 1, 0 },
    { string_view("dict2", 5), 481, 631, 0, 1, 0 },
    { string_view("val", 3), 482, 458, 0, 1, 0 },
    { string_view("dict3", 5), 483, 659, 0, 1, 0 },
    { string_view("lc", 2), 484, 0, 0, 0, 0 },
    { string_view("i", 1), 485, 0, 0, 0, 0 },
    { string_view("a", 1), 486, 0, 0, 1, 0 },
    { string_view("b", 1), 487, 0, 0, 1, 0 },
    { string_view("mode", 4), 488, 0, 1, 0, 0 },
    { string_view("dav", 3), 489, 0, 0, 1, 0 },
    { string_view("a", 1), 490, 0, 0, 0, 0 },
    { string_view("b", 1), 491, 0, 0, 0, 0 },
    { string_view("c", 1), 492, 0, 0, 0, 0 },
    { string_view("a", 1), 496, 0, 0, 0, 0 },
    { string_view("slv", 3), 497, 3, 0, 0, 0 },
    { string_view("vlv", 3), 498, 7, 0, 0, 0 },
    { string_view("stlv", 4), 499, 699, 0, 0, 0 },
    { string_view("stlv", 4), 499, 699, 0, 0, 0 },
    { string_view("divisor", 7), 500, 2, 0, 1, 0 },
    { string_view("r", 1), 501, 711, 0, 1, 0 },
    { string_view("state", 5), 502, 0, 1, 0, 0 },
    { string_view("fv", 2), 503, 729, 0, 1, 0 },
    { string_view("gs", 2), 504, 7, 0, 1, 0 },
    { string_view("i", 1), 505, 0, 0, 0, 0 },
    { string_view("i", 1), 506, 731, 0, 0, 0 },
    { string_view("f", 1), 507, 733, 1, 0, 0 },
    { string_view("x", 1), 508, 0, 0, 0, 0 },
    { string_view("a", 1), 509, 0, 0, 0, 0 },
    { string_view("b", 1), 510, 146, 0, 0, 0 },
    { string_view("this", 4), 511, 735, 0, 0, 0 },
    { string_view("this", 4), 512, 744, 0, 0, 0 },
    { string_view("this", 4), 512, 744, 0, 0, 0 },
    { string_view("this", 4), 513, 756, 0, 0, 0 },
    { string_view("this", 4), 514, 762, 0, 0, 0 },
    { string_view("c", 1), 515, 0, 0, 0, 0 },
    { string_view("this", 4), 516, 762, 0, 0, 0 },
    { string_view("c", 1), 517, 3, 0, 0, 0 },
    { string_view("this", 4), 518, 762, 0, 0, 0 },
    { string_view("c", 1), 519, 0, 0, 0, 0 },
    { string_view("this", 4), 520, 762, 0, 0, 0 },
    { string_view("this", 4), 521, 762, 0, 0, 0 },
    { string_view("this", 4), 522, 762, 0, 0, 0 },
    { string_view("this", 4), 523, 771, 0, 0, 0 },
    { string_view("c", 1), 524, 0, 0, 0, 0 },
    { string_view("this", 4), 525, 771, 0, 0, 0 },
    { string_view("c", 1), 526, 3, 0, 0, 0 },
    { string_view("this", 4), 527, 771, 0, 0, 0 },
    { string_view("c", 1), 528, 0, 0, 0, 0 },
    { string_view("this", 4), 529, 771, 0, 0, 0 },
    { string_view("this", 4), 530, 771, 0, 0, 0 },
    { string_view("this", 4), 531, 771, 0, 0, 0 },
    { string_view("this", 4), 532, 783, 0, 0, 0 },
    { string_view("c", 1), 533, 0, 0, 0, 0 },
    { string_view("x", 1), 534, 762, 0, 1, 0 },
    { string_view("x2", 2), 535, 762, 0, 1, 0 },
    { string_view("x3", 2), 536, 756, 0, 1, 0 },
    { string_view("mr1", 3), 537, 0, 0, 1, 0 },
    { string_view("mr2", 3), 538, 3, 0, 1, 0 },
    { string_view("d", 1), 539, 795, 0, 0, 0 },
    { string_view("i", 1), 540, 0, 0, 0, 0 },
    { string_view("s", 1), 541, 3, 0, 0, 0 },
    { string_view("t", 1), 542, 2, 0, 0, 0 },
    { string_view("a", 1), 543, 458, 0, 1, 0 },
    { string_view("b", 1), 544, 458, 0, 1, 0 },
    { string_view("x", 1), 545, 3, 0, 0, 0 },
    { string_view("this", 4), 546, 804, 0, 0, 0 },
    { string_view("t", 1), 547, 0, 0, 0, 0 },
    { string_view("this", 4), 548, 804, 0, 0, 0 },
    { string_view("u", 1), 549, 0, 0, 0, 0 },
    { string_view("this", 4), 550, 804, 0, 0, 0 },
    { string_view("x", 1), 551, 0, 0, 1, 0 },
    { string_view("a", 1), 552, 804, 0, 0, 0 },
    { string_view("t", 1), 553, 0, 0, 0, 0 },
    { string_view("a", 1), 554, 804, 0, 1, 0 },
    { string_view("b", 1), 555, 829, 0, 1, 0 },
    { string_view("x", 1), 556, 2, 0, 0, 0 },
    { string_view("y", 1), 557, 2, 0, 0, 0 },
    { string_view("t", 1), 558, 2, 0, 0, 0 },
    { string_view("r", 1), 559, 2, 0, 1, 0 },
    { string_view("s", 1), 560, 2, 0, 1, 0 },
    { string_view("aa", 2), 561, 825, 0, 0, 0 },
    { string_view("aa", 2), 562, 804, 0, 0, 0 },
    { string_view("aa", 2), 563, 825, 0, 0, 0 },
    { string_view("a1", 2), 564, 854, 0, 1, 0 },
    { string_view("a2", 2), 565, 863, 0, 1, 0 },
    { string_view("xs", 2), 567, 7, 0, 0, 0 },
    { string_view("as", 2), 568, 881, 0, 1, 0 },
    { string_view("bs", 2), 569, 895, 0, 1, 0 },
    { string_view("xs", 2), 570, 881, 0, 1, 0 },
    { string_view("ys", 2), 571, 881, 0, 0, 0 },
    { string_view("as", 2), 572, 917, 0, 1, 0 },
    { string_view("a", 1), 573, 897, 0, 0, 0 },
    { string_view("startnode", 9), 575, 919, 0, 0, 0 },
    { string_view("endcondition", 12), 576, 999, 0, 0, 0 },
    { string_view("generatenewstates", 17), 577, 1001, 0, 0, 0 },
    { string_view("heuristic", 9), 578, 1003, 1, 0, 0 },
    { string_view("openlist", 8), 579, 1005, 1, 1, 0 },
    { string_view("n", 1), 580, 997, 1, 0, 0 },
    { string_view("delta", 5), 581, 25, 0, 0, 0 },
    { string_view("delta", 5), 581, 25, 0, 0, 0 },
    { string_view("cost", 4), 582, 0, 0, 0, 0 },
    { string_view("nn", 2), 583, 919, 0, 0, 0 },
    { string_view("G", 1), 584, 2, 0, 1, 0 },
    { string_view("c", 1), 585, 919, 0, 0, 0 },
    { string_view("path", 4), 586, 1005, 0, 1, 0 },
    { string_view("startnode", 9), 587, 919, 0, 0, 0 },
    { string_view("endnode", 7), 588, 919, 1, 0, 0 },
    { string_view("costf", 5), 589, 1007, 1, 0, 0 },
    { string_view("distancef", 9), 590, 1009, 1, 0, 0 },
    { string_view("neighbors", 9), 591, 1011, 1, 0, 0 },
    { string_view("n", 1), 592, 919, 0, 0, 0 },
    { string_view("n", 1), 593, 919, 1, 0, 0 },
    { string_view("f", 1), 594, 1013, 1, 0, 0 },
    { string_view("nn", 2), 595, 919, 0, 0, 0 },
    { string_view("cost", 4), 596, 0, 0, 1, 0 },
    { string_view("state", 5), 597, 25, 0, 0, 0 },
    { string_view("state", 5), 597, 25, 0, 0, 0 },
    { string_view("isocta", 6), 598, 146, 0, 0, 0 },
    { string_view("gridsize", 8), 599, 25, 1, 0, 0 },
    { string_view("gridsize", 8), 599, 25, 1, 0, 0 },
    { string_view("startnode", 9), 600, 919, 1, 0, 0 },
    { string_view("endnode", 7), 601, 919, 1, 0, 0 },
    { string_view("getnode", 7), 602, 1015, 1, 0, 0 },
    { string_view("costf", 5), 603, 1007, 1, 0, 0 },
    { string_view("directions", 10), 604, 37, 1, 0, 0 },
    { string_view("distancef", 9), 605, 1009, 0, 0, 0 },
    { string_view("n", 1), 606, 919, 0, 0, 0 },
    { string_view("f", 1), 607, 1017, 0, 0, 0 },
    { string_view("delta", 5), 608, 25, 0, 0, 0 },
    { string_view("delta", 5), 608, 25, 0, 0, 0 },
    { string_view("np", 2), 609, 25, 0, 1, 0 },
    { string_view("np", 2), 609, 25, 0, 1, 0 },
    { string_view("v", 1), 610, 25, 0, 0, 0 },
    { string_view("v", 1), 610, 25, 0, 0, 0 },
    { string_view("x", 1), 611, 0, 0, 1, 0 },
    { string_view("y", 1), 612, 0, 0, 1, 0 },
    { string_view("big", 3), 613, 0, 0, 1, 0 },
    { string_view("small", 5), 614, 0, 0, 1, 0 },
    { string_view("_", 1), 615, 25, 0, 0, 0 },
    { string_view("_", 1), 615, 25, 0, 0, 0 },
    { string_view("goapactions", 11), 620, 1019, 1, 0, 0 },
    { string_view("initstate", 9), 621, 111, 0, 0, 0 },
    { string_view("heuristic", 9), 622, 1021, 1, 0, 0 },
    { string_view("endcondition", 12), 623, 1023, 1, 0, 0 },
    { string_view("H", 1), 624, 0, 0, 1, 0 },
    { string_view("existingnodes", 13), 625, 148, 1, 1, 0 },
    { string_view("_", 1), 626, 49, 0, 0, 0 },
    { string_view("n", 1), 627, 49, 0, 0, 0 },
    { string_view("f", 1), 628, 1025, 0, 0, 0 },
    { string_view("act", 3), 629, 138, 0, 0, 0 },
    { string_view("nstate", 6), 630, 111, 1, 1, 0 },
    { string_view("i", 1), 631, 0, 0, 0, 0 },
    { string_view("_", 1), 632, 49, 0, 0, 0 },
    { string_view("_", 1), 633, 111, 0, 0, 0 },
    { string_view("initworld", 9), 634, 11, 1, 1, 0 },
    { string_view("expected_result", 15), 635, 11, 0, 1, 0 },
    { string_view("worldsize", 9), 636, 25, 1, 1, 0 },
    { string_view("worldsize", 9), 636, 25, 1, 1, 0 },
    { string_view("startpos", 8), 637, 25, 1, 0, 0 },
    { string_view("startpos", 8), 637, 25, 1, 0, 0 },
    { string_view("endpos", 6), 638, 25, 1, 0, 0 },
    { string_view("endpos", 6), 638, 25, 1, 0, 0 },
    { string_view("world", 5), 639, 1027, 1, 1, 0 },
    { string_view("y", 1), 640, 0, 1, 0, 0 },
    { string_view("x", 1), 641, 0, 0, 0, 0 },
    { string_view("c", 1), 642, 0, 0, 1, 0 },
    { string_view("path", 4), 643, 1005, 0, 1, 0 },
    { string_view("_", 1), 644, 25, 0, 0, 0 },
    { string_view("_", 1), 644, 25, 0, 0, 0 },
    { string_view("n", 1), 645, 919, 0, 0, 0 },
    { string_view("nn", 2), 646, 919, 0, 0, 0 },
    { string_view("n", 1), 647, 919, 0, 0, 0 },
    { string_view("astar_result", 12), 648, 11, 0, 1, 0 },
    { string_view("row", 3), 649, 1005, 0, 0, 0 },
    { string_view("accum", 5), 650, 3, 0, 0, 0 },
    { string_view("n", 1), 651, 919, 0, 0, 0 },
    { string_view("this", 4), 652, 1029, 0, 0, 0 },
    { string_view("s", 1), 653, 111, 0, 0, 0 },
    { string_view("this", 4), 654, 1029, 0, 0, 0 },
    { string_view("s", 1), 655, 111, 0, 0, 0 },
    { string_view("this", 4), 656, 1035, 0, 0, 0 },
    { string_view("s", 1), 657, 111, 0, 0, 0 },
    { string_view("this", 4), 658, 1035, 0, 0, 0 },
    { string_view("s", 1), 659, 111, 0, 0, 0 },
    { string_view("this", 4), 660, 1041, 0, 0, 0 },
    { string_view("s", 1), 661, 111, 0, 0, 0 },
    { string_view("this", 4), 662, 1041, 0, 0, 0 },
    { string_view("s", 1), 663, 111, 0, 0, 0 },
    { string_view("this", 4), 664, 1047, 0, 0, 0 },
    { string_view("s", 1), 665, 111, 0, 0, 0 },
    { string_view("this", 4), 666, 1047, 0, 0, 0 },
    { string_view("s", 1), 667, 111, 0, 0, 0 },
    { string_view("this", 4), 668, 1053, 0, 0, 0 },
    { string_view("s", 1), 669, 111, 0, 0, 0 },
    { string_view("this", 4), 670, 1053, 0, 0, 0 },
    { string_view("s", 1), 671, 111, 0, 0, 0 },
    { string_view("this", 4), 672, 1059, 0, 0, 0 },
    { string_view("s", 1), 673, 111, 0, 0, 0 },
    { string_view("this", 4), 674, 1059, 0, 0, 0 },
    { string_view("s", 1), 675, 111, 0, 0, 0 },
    { string_view("goapactions", 11), 676, 1019, 0, 1, 0 },
    { string_view("goap_path", 9), 677, 148, 0, 1, 0 },
    { string_view("state", 5), 678, 111, 0, 0, 0 },
    { string_view("s", 1), 679, 111, 0, 0, 0 },
    { string_view("n", 1), 680, 49, 0, 0, 0 },
    { string_view("knight_moves", 12), 681, 37, 1, 1, 0 },
    { string_view("dim", 3), 682, 25, 1, 0, 0 },
    { string_view("dim", 3), 682, 25, 1, 0, 0 },
    { string_view("board", 5), 683, 13, 1, 1, 0 },
    { string_view("cur", 3), 684, 25, 1, 0, 0 },
    { string_view("cur", 3), 684, 25, 1, 0, 0 },
    { string_view("step", 4), 685, 0, 0, 0, 0 },
    { string_view("candidates", 10), 686, 37, 0, 0, 0 },
    { string_view("m", 1), 687, 25, 0, 0, 0 },
    { string_view("m", 1), 687, 25, 0, 0, 0 },
    { string_view("v", 1), 688, 25, 0, 0, 0 },
    { string_view("v", 1), 688, 25, 0, 0, 0 },
    { string_view("c", 1), 689, 25, 0, 0, 0 },
    { string_view("c", 1), 689, 25, 0, 0, 0 },
    { string_view("degrees", 7), 690, 7, 0, 1, 0 },
    { string_view("c", 1), 691, 25, 1, 0, 0 },
    { string_view("c", 1), 691, 25, 1, 0, 0 },
    { string_view("m", 1), 692, 25, 0, 0, 0 },
    { string_view("m", 1), 692, 25, 0, 0, 0 },
    { string_view("d", 1), 693, 0, 0, 0, 0 },
    { string_view("this", 4), 694, 1065, 0, 0, 0 },
    { string_view("this", 4), 695, 1065, 0, 0, 0 },
    { string_view("this", 4), 696, 1065, 0, 0, 0 },
    { string_view("this", 4), 697, 1080, 0, 0, 0 },
    { string_view("elems", 5), 698, 3, 0, 1, 0 },
    { string_view("_a", 2), 699, 3, 0, 0, 0 },
    { string_view("_e", 2), 700, 1074, 0, 0, 0 },
    { string_view("this", 4), 701, 1080, 0, 0, 0 },
    { string_view("_a", 2), 702, 0, 0, 0, 0 },
    { string_view("_e", 2), 703, 1074, 0, 0, 0 },
    { string_view("_a", 2), 704, 0, 0, 0, 0 },
    { string_view("_e", 2), 705, 1074, 0, 0, 0 },
    { string_view("this", 4), 706, 1080, 1, 0, 0 },
    { string_view("_", 1), 707, 1074, 0, 0, 0 },
    { string_view("_a", 2), 708, 3, 0, 0, 0 },
    { string_view("_e", 2), 709, 3, 0, 0, 0 },
    { string_view("s", 1), 710, 3, 1, 0, 0 },
    { string_view("token", 5), 711, 3, 1, 0, 0 },
    { string_view("atom", 4), 712, 3, 1, 0, 0 },
    { string_view("i", 1), 713, 0, 1, 0, 0 },
    { string_view("line", 4), 714, 0, 1, 0, 0 },
    { string_view("ival", 4), 715, 0, 1, 0, 0 },
    { string_view("err", 3), 716, 3, 0, 0, 0 },
    { string_view("start", 5), 717, 0, 0, 1, 0 },
    { string_view("_", 1), 718, 0, 0, 0, 0 },
    { string_view("all", 3), 719, 146, 0, 0, 0 },
    { string_view("tok", 3), 720, 3, 0, 0, 0 },
    { string_view("a", 1), 721, 3, 0, 1, 0 },
    { string_view("name", 4), 722, 3, 1, 1, 0 },
    { string_view("_", 1), 723, 3, 0, 0, 0 },
    { string_view("n", 1), 724, 1080, 0, 1, 0 },
    { string_view("n", 1), 725, 1065, 0, 1, 0 },
    { string_view("root", 4), 726, 1074, 0, 1, 0 },
    { string_view("test_code", 9), 727, 3, 0, 1, 0 },
    { string_view("ast", 3), 728, 1094, 0, 1, 0 },
    { string_view("err", 3), 729, 3, 0, 1, 0 },
    { string_view("code", 4), 730, 3, 0, 1, 0 },
    { string_view("sz", 2), 731, 25, 1, 0, 0 },
    { string_view("sz", 2), 731, 25, 1, 0, 0 },
    { string_view("target", 6), 732, 25, 0, 0, 0 },
    { string_view("target", 6), 732, 25, 0, 0, 0 },
    { string_view("traversablef", 12), 733, 1096, 1, 0, 0 },
    { string_view("m", 1), 734, 1121, 1, 1, 0 },
    { string_view("v", 1), 735, 25, 0, 0, 0 },
    { string_view("v", 1), 735, 25, 0, 0, 0 },
    { string_view("end", 3), 736, 1098, 0, 1, 0 },
    { string_view("pathq", 5), 737, 1119, 1, 1, 0 },
    { string_view("dir", 3), 738, 25, 0, 0, 0 },
    { string_view("dir", 3), 738, 25, 0, 0, 0 },
    { string_view("parent", 6), 739, 1098, 0, 0, 0 },
    { string_view("pos", 3), 740, 25, 0, 1, 0 },
    { string_view("pos", 3), 740, 25, 0, 1, 0 },
    { string_view("c", 1), 741, 1098, 0, 1, 0 },
    { string_view("i", 1), 742, 0, 0, 0, 0 },
    { string_view("c", 1), 743, 1098, 0, 1, 0 },
    { string_view("x", 1), 744, 0, 0, 0, 0 },
    { string_view("y", 1), 745, 0, 0, 0, 0 },
    { string_view("v", 1), 746, 25, 0, 0, 0 },
    { string_view("v", 1), 746, 25, 0, 0, 0 },
    { string_view("lssize", 6), 747, 0, 1, 1, 0 },
    { string_view("hmargin", 7), 748, 2, 1, 1, 0 },
    { string_view("highest", 7), 749, 25, 1, 0, 0 },
    { string_view("highest", 7), 749, 25, 1, 0, 0 },
    { string_view("lowest", 6), 750, 25, 1, 0, 0 },
    { string_view("lowest", 6), 750, 25, 1, 0, 0 },
    { string_view("highh", 5), 751, 2, 1, 0, 0 },
    { string_view("lowh", 4), 752, 2, 1, 0, 0 },
    { string_view("cells", 5), 753, 1140, 1, 1, 0 },
    { string_view("y", 1), 754, 0, 1, 0, 0 },
    { string_view("x", 1), 755, 0, 0, 0, 0 },
    { string_view("h", 1), 756, 2, 0, 0, 0 },
    { string_view("f", 1), 757, 1142, 0, 0, 0 },
    { string_view("y", 1), 758, 0, 0, 0, 0 },
    { string_view("x", 1), 759, 0, 0, 0, 0 },
    { string_view("pos", 3), 760, 25, 0, 0, 0 },
    { string_view("pos", 3), 760, 25, 0, 0, 0 },
    { string_view("f", 1), 761, 1144, 0, 0, 0 },
    { string_view("neighbours", 10), 762, 37, 1, 1, 0 },
    { string_view("x", 1), 763, 0, 0, 0, 0 },
    { string_view("y", 1), 764, 0, 0, 0, 0 },
    { string_view("f", 1), 765, 1146, 1, 0, 0 },
    { string_view("n", 1), 766, 25, 1, 0, 0 },
    { string_view("n", 1), 766, 25, 1, 0, 0 },
    { string_view("starting_extra_water", 20), 767, 2, 0, 1, 0 },
    { string_view("water_per_second", 16), 768, 2, 1, 1, 0 },
    { string_view("give_water_divider", 18), 769, 2, 1, 1, 0 },
    { string_view("minimum_water_accumulation", 26), 770, 2, 1, 1, 0 },
    { string_view("x", 1), 771, 0, 0, 0, 0 },
    { string_view("y", 1), 772, 0, 0, 0, 0 },
    { string_view("c", 1), 773, 1123, 1, 0, 0 },
    { string_view("totald", 6), 774, 2, 1, 0, 0 },
    { string_view("lower", 5), 775, 1138, 1, 1, 0 },
    { string_view("nc", 2), 776, 1123, 0, 0, 0 },
    { string_view("d", 1), 777, 2, 0, 1, 0 },
    { string_view("w", 1), 778, 2, 0, 1, 0 },
    { string_view("nc", 2), 779, 1123, 0, 0, 0 },
    { string_view("give", 4), 780, 2, 0, 1, 0 },
    { string_view("x", 1), 781, 0, 0, 0, 0 },
    { string_view("y", 1), 782, 0, 0, 0, 0 },
    { string_view("c", 1), 783, 1123, 0, 0, 0 },
    { string_view("N", 1), 784, 0, 1, 1, 0 },
    { string_view("f", 1), 785, 2, 1, 1, 0 },
    { string_view("x", 1), 786, 9, 0, 0, 0 },
    { string_view("d", 1), 787, 9, 0, 0, 0 },
    { string_view("mu", 2), 788, 2, 0, 0, 0 },
    { string_view("n_epochs", 8), 789, 0, 0, 0, 0 },
    { string_view("y", 1), 790, 9, 0, 1, 0 },
    { string_view("w0", 2), 791, 2, 0, 0, 0 },
    { string_view("w1", 2), 792, 2, 0, 0, 0 },
    { string_view("grad0", 5), 793, 2, 0, 0, 0 },
    { string_view("grad1", 5), 794, 2, 0, 0, 0 },
    { string_view("i", 1), 795, 0, 0, 0, 0 },
    { string_view("err", 3), 796, 2, 0, 1, 0 },
    { string_view("i", 1), 797, 0, 0, 0, 0 },
    { string_view("sigma", 5), 798, 2, 0, 1, 0 },
    { string_view("mu", 2), 799, 2, 0, 1, 0 },
    { string_view("n_epochs", 8), 800, 0, 0, 1, 0 },
    { string_view("x", 1), 801, 9, 0, 1, 0 },
    { string_view("d", 1), 802, 9, 0, 1, 0 },
    { string_view("i", 1), 803, 0, 0, 0, 0 },
    { string_view("verts", 5), 804, 1148, 1, 0, 0 },
    { string_view("edges", 5), 805, 13, 0, 0, 0 },
    { string_view("quads", 5), 806, 13, 0, 0, 0 },
    { string_view("disps", 5), 807, 1148, 0, 1, 0 },
    { string_view("v", 1), 808, 162, 0, 0, 0 },
    { string_view("v", 1), 808, 162, 0, 0, 0 },
    { string_view("w", 1), 809, 162, 0, 0, 0 },
    { string_view("w", 1), 809, 162, 0, 0, 0 },
    { string_view("i", 1), 810, 0, 0, 0, 0 },
    { string_view("tow", 3), 811, 162, 0, 1, 0 },
    { string_view("tow", 3), 811, 162, 0, 1, 0 },
    { string_view("dist", 4), 812, 2, 0, 1, 0 },
    { string_view("totaldisp", 9), 813, 2, 0, 0, 0 },
    { string_view("d", 1), 814, 162, 0, 0, 0 },
    { string_view("d", 1), 814, 162, 0, 0, 0 },
    { string_view("i", 1), 815, 0, 0, 0, 0 },
    { string_view("wsize", 5), 816, 25, 1, 1, 0 },
    { string_view("wsize", 5), 816, 25, 1, 1, 0 },
    { string_view("p", 1), 817, 25, 0, 0, 0 },
    { string_view("p", 1), 817, 25, 0, 0, 0 },
    { string_view("pedges", 6), 818, 1165, 0, 0, 0 },
    { string_view("v", 1), 819, 162, 0, 0, 0 },
    { string_view("v", 1), 819, 162, 0, 0, 0 },
    { string_view("i", 1), 820, 0, 0, 0, 0 },
    { string_view("w", 1), 821, 162, 0, 0, 0 },
    { string_view("w", 1), 821, 162, 0, 0, 0 },
    { string_view("j", 1), 822, 0, 0, 0, 0 },
    { string_view("dist", 4), 823, 2, 0, 1, 0 },
    { string_view("a", 1), 824, 1150, 0, 0, 0 },
    { string_view("a", 1), 824, 1150, 0, 0, 0 },
    { string_view("a", 1), 824, 1150, 0, 0, 0 },
    { string_view("b", 1), 825, 1150, 0, 0, 0 },
    { string_view("b", 1), 825, 1150, 0, 0, 0 },
    { string_view("b", 1), 825, 1150, 0, 0, 0 },
    { string_view("edgedistnocross", 15), 826, 2, 0, 1, 0 },
    { string_view("vnbs", 4), 827, 13, 0, 0, 0 },
    { string_view("pe", 2), 828, 1150, 0, 0, 0 },
    { string_view("pe", 2), 828, 1150, 0, 0, 0 },
    { string_view("pe", 2), 828, 1150, 0, 0, 0 },
    { string_view("nbl", 3), 829, 7, 0, 0, 0 },
    { string_view("i", 1), 830, 0, 1, 0, 0 },
    { string_view("a", 1), 831, 0, 0, 0, 0 },
    { string_view("b", 1), 832, 0, 0, 0, 0 },
    { string_view("tris", 4), 833, 13, 0, 1, 0 },
    { string_view("i", 1), 834, 0, 0, 0, 0 },
    { string_view("j", 1), 835, 0, 0, 0, 0 },
    { string_view("shape", 5), 836, 7, 1, 1, 0 },
    { string_view("p", 1), 837, 0, 1, 0, 0 },
    { string_view("nbl", 3), 838, 7, 0, 1, 0 },
    { string_view("pidx", 4), 839, 0, 0, 1, 0 },
    { string_view("_", 1), 840, 0, 0, 0, 0 },
    { string_view("start", 5), 841, 0, 0, 1, 0 },
    { string_view("_a", 2), 842, 0, 0, 0, 0 },
    { string_view("_b", 2), 843, 0, 0, 0, 0 },
    { string_view("_", 1), 844, 7, 0, 0, 0 },
    { string_view("_", 1), 845, 7, 0, 0, 0 },
    { string_view("besti", 5), 846, 0, 0, 0, 0 },
    { string_view("bestqc", 6), 847, 0, 0, 0, 0 },
    { string_view("t", 1), 848, 7, 0, 0, 0 },
    { string_view("i", 1), 849, 0, 0, 0, 0 },
    { string_view("qc", 2), 850, 0, 0, 0, 0 },
    { string_view("vi1", 3), 851, 0, 0, 0, 0 },
    { string_view("q", 1), 852, 7, 0, 0, 0 },
    { string_view("vi2", 3), 853, 0, 0, 0, 0 },
    { string_view("t1", 2), 854, 7, 0, 1, 0 },
    { string_view("found", 5), 855, 146, 0, 0, 0 },
    { string_view("vi", 2), 856, 0, 0, 0, 0 },
    { string_view("vii", 3), 857, 0, 0, 0, 0 },
    { string_view("ni", 2), 858, 0, 0, 1, 0 },
    { string_view("li", 2), 859, 0, 0, 1, 0 },
    { string_view("t", 1), 860, 7, 0, 0, 0 },
    { string_view("color_clear", 11), 861, 1167, 0, 1, 1 },
    { string_view("color_clear", 11), 861, 1167, 0, 1, 1 },
    { string_view("color_clear", 11), 861, 1167, 0, 1, 1 },
    { string_view("color_clear", 11), 861, 1167, 0, 1, 1 },
    { string_view("color_black", 11), 862, 1167, 0, 1, 1 },
    { string_view("color_black", 11), 862, 1167, 0, 1, 1 },
    { string_view("color_black", 11), 862, 1167, 0, 1, 1 },
    { string_view("color_black", 11), 862, 1167, 0, 1, 1 },
    { string_view("color_white", 11), 863, 1167, 0, 1, 1 },
    { string_view("color_white", 11), 863, 1167, 0, 1, 1 },
    { string_view("color_white", 11), 863, 1167, 0, 1, 1 },
    { string_view("color_white", 11), 863, 1167, 0, 1, 1 },
    { string_view("color_red", 9), 864, 1167, 0, 1, 1 },
    { string_view("color_red", 9), 864, 1167, 0, 1, 1 },
    { string_view("color_red", 9), 864, 1167, 0, 1, 1 },
    { string_view("color_red", 9), 864, 1167, 0, 1, 1 },
    { string_view("color_green", 11), 865, 1167, 0, 1, 1 },
    { string_view("color_green", 11), 865, 1167, 0, 1, 1 },
    { string_view("color_green", 11), 865, 1167, 0, 1, 1 },
    { string_view("color_green", 11), 865, 1167, 0, 1, 1 },
    { string_view("color_blue", 10), 866, 1167, 0, 1, 1 },
    { string_view("color_blue", 10), 866, 1167, 0, 1, 1 },
    { string_view("color_blue", 10), 866, 1167, 0, 1, 1 },
    { string_view("color_blue", 10), 866, 1167, 0, 1, 1 },
    { string_view("color_yellow", 12), 867, 1167, 0, 1, 1 },
    { string_view("color_yellow", 12), 867, 1167, 0, 1, 1 },
    { string_view("color_yellow", 12), 867, 1167, 0, 1, 1 },
    { string_view("color_yellow", 12), 867, 1167, 0, 1, 1 },
    { string_view("color_cyan", 10), 868, 1167, 0, 1, 1 },
    { string_view("color_cyan", 10), 868, 1167, 0, 1, 1 },
    { string_view("color_cyan", 10), 868, 1167, 0, 1, 1 },
    { string_view("color_cyan", 10), 868, 1167, 0, 1, 1 },
    { string_view("color_pink", 10), 869, 1167, 0, 1, 1 },
    { string_view("color_pink", 10), 869, 1167, 0, 1, 1 },
    { string_view("color_pink", 10), 869, 1167, 0, 1, 1 },
    { string_view("color_pink", 10), 869, 1167, 0, 1, 1 },
    { string_view("color_grey", 10), 870, 1167, 0, 1, 1 },
    { string_view("color_grey", 10), 870, 1167, 0, 1, 1 },
    { string_view("color_grey", 10), 870, 1167, 0, 1, 1 },
    { string_view("color_grey", 10), 870, 1167, 0, 1, 1 },
    { string_view("color_dark_grey", 15), 871, 1167, 0, 1, 1 },
    { string_view("color_dark_grey", 15), 871, 1167, 0, 1, 1 },
    { string_view("color_dark_grey", 15), 871, 1167, 0, 1, 1 },
    { string_view("color_dark_grey", 15), 871, 1167, 0, 1, 1 },
    { string_view("color_light_grey", 16), 872, 1167, 0, 1, 1 },
    { string_view("color_light_grey", 16), 872, 1167, 0, 1, 1 },
    { string_view("color_light_grey", 16), 872, 1167, 0, 1, 1 },
    { string_view("color_light_grey", 16), 872, 1167, 0, 1, 1 },
    { string_view("color_lightest_grey", 19), 873, 1167, 0, 1, 1 },
    { string_view("color_lightest_grey", 19), 873, 1167, 0, 1, 1 },
    { string_view("color_lightest_grey", 19), 873, 1167, 0, 1, 1 },
    { string_view("color_lightest_grey", 19), 873, 1167, 0, 1, 1 },
    { string_view("color_darkest_grey", 18), 874, 1167, 0, 1, 1 },
    { string_view("color_darkest_grey", 18), 874, 1167, 0, 1, 1 },
    { string_view("color_darkest_grey", 18), 874, 1167, 0, 1, 1 },
    { string_view("color_darkest_grey", 18), 874, 1167, 0, 1, 1 },
    { string_view("color_light_red", 15), 875, 1167, 0, 1, 1 },
    { string_view("color_light_red", 15), 875, 1167, 0, 1, 1 },
    { string_view("color_light_red", 15), 875, 1167, 0, 1, 1 },
    { string_view("color_light_red", 15), 875, 1167, 0, 1, 1 },
    { string_view("color_light_green", 17), 876, 1167, 0, 1, 1 },
    { string_view("color_light_green", 17), 876, 1167, 0, 1, 1 },
    { string_view("color_light_green", 17), 876, 1167, 0, 1, 1 },
    { string_view("color_light_green", 17), 876, 1167, 0, 1, 1 },
    { string_view("color_light_blue", 16), 877, 1167, 0, 1, 1 },
    { string_view("color_light_blue", 16), 877, 1167, 0, 1, 1 },
    { string_view("color_light_blue", 16), 877, 1167, 0, 1, 1 },
    { string_view("color_light_blue", 16), 877, 1167, 0, 1, 1 },
    { string_view("color_light_yellow", 18), 878, 1167, 0, 1, 1 },
    { string_view("color_light_yellow", 18), 878, 1167, 0, 1, 1 },
    { string_view("color_light_yellow", 18), 878, 1167, 0, 1, 1 },
    { string_view("color_light_yellow", 18), 878, 1167, 0, 1, 1 },
    { string_view("color_light_cyan", 16), 879, 1167, 0, 1, 1 },
    { string_view("color_light_cyan", 16), 879, 1167, 0, 1, 1 },
    { string_view("color_light_cyan", 16), 879, 1167, 0, 1, 1 },
    { string_view("color_light_cyan", 16), 879, 1167, 0, 1, 1 },
    { string_view("color_light_pink", 16), 880, 1167, 0, 1, 1 },
    { string_view("color_light_pink", 16), 880, 1167, 0, 1, 1 },
    { string_view("color_light_pink", 16), 880, 1167, 0, 1, 1 },
    { string_view("color_light_pink", 16), 880, 1167, 0, 1, 1 },
    { string_view("color_dark_red", 14), 881, 1167, 0, 1, 1 },
    { string_view("color_dark_red", 14), 881, 1167, 0, 1, 1 },
    { string_view("color_dark_red", 14), 881, 1167, 0, 1, 1 },
    { string_view("color_dark_red", 14), 881, 1167, 0, 1, 1 },
    { string_view("color_dark_green", 16), 882, 1167, 0, 1, 1 },
    { string_view("color_dark_green", 16), 882, 1167, 0, 1, 1 },
    { string_view("color_dark_green", 16), 882, 1167, 0, 1, 1 },
    { string_view("color_dark_green", 16), 882, 1167, 0, 1, 1 },
    { string_view("color_dark_blue", 15), 883, 1167, 0, 1, 1 },
    { string_view("color_dark_blue", 15), 883, 1167, 0, 1, 1 },
    { string_view("color_dark_blue", 15), 883, 1167, 0, 1, 1 },
    { string_view("color_dark_blue", 15), 883, 1167, 0, 1, 1 },
    { string_view("color_olive", 11), 884, 1167, 0, 1, 1 },
    { string_view("color_olive", 11), 884, 1167, 0, 1, 1 },
    { string_view("color_olive", 11), 884, 1167, 0, 1, 1 },
    { string_view("color_olive", 11), 884, 1167, 0, 1, 1 },
    { string_view("color_teal", 10), 885, 1167, 0, 1, 1 },
    { string_view("color_teal", 10), 885, 1167, 0, 1, 1 },
    { string_view("color_teal", 10), 885, 1167, 0, 1, 1 },
    { string_view("color_teal", 10), 885, 1167, 0, 1, 1 },
    { string_view("color_purple", 12), 886, 1167, 0, 1, 1 },
    { string_view("color_purple", 12), 886, 1167, 0, 1, 1 },
    { string_view("color_purple", 12), 886, 1167, 0, 1, 1 },
    { string_view("color_purple", 12), 886, 1167, 0, 1, 1 },
    { string_view("color_orange", 12), 887, 1167, 0, 1, 1 },
    { string_view("color_orange", 12), 887, 1167, 0, 1, 1 },
    { string_view("color_orange", 12), 887, 1167, 0, 1, 1 },
    { string_view("color_orange", 12), 887, 1167, 0, 1, 1 },
    { string_view("color_chartreuse", 16), 888, 1167, 0, 1, 1 },
    { string_view("color_chartreuse", 16), 888, 1167, 0, 1, 1 },
    { string_view("color_chartreuse", 16), 888, 1167, 0, 1, 1 },
    { string_view("color_chartreuse", 16), 888, 1167, 0, 1, 1 },
    { string_view("color_springgreen", 17), 889, 1167, 0, 1, 1 },
    { string_view("color_springgreen", 17), 889, 1167, 0, 1, 1 },
    { string_view("color_springgreen", 17), 889, 1167, 0, 1, 1 },
    { string_view("color_springgreen", 17), 889, 1167, 0, 1, 1 },
    { string_view("color_dodgerblue", 16), 890, 1167, 0, 1, 1 },
    { string_view("color_dodgerblue", 16), 890, 1167, 0, 1, 1 },
    { string_view("color_dodgerblue", 16), 890, 1167, 0, 1, 1 },
    { string_view("color_dodgerblue", 16), 890, 1167, 0, 1, 1 },
    { string_view("color_pink_violet", 17), 891, 1167, 0, 1, 1 },
    { string_view("color_pink_violet", 17), 891, 1167, 0, 1, 1 },
    { string_view("color_pink_violet", 17), 891, 1167, 0, 1, 1 },
    { string_view("color_pink_violet", 17), 891, 1167, 0, 1, 1 },
    { string_view("color_blue_violet", 17), 892, 1167, 0, 1, 1 },
    { string_view("color_blue_violet", 17), 892, 1167, 0, 1, 1 },
    { string_view("color_blue_violet", 17), 892, 1167, 0, 1, 1 },
    { string_view("color_blue_violet", 17), 892, 1167, 0, 1, 1 },
    { string_view("color_luminance_weights", 23), 895, 1167, 0, 1, 1 },
    { string_view("color_luminance_weights", 23), 895, 1167, 0, 1, 1 },
    { string_view("color_luminance_weights", 23), 895, 1167, 0, 1, 1 },
    { string_view("color_luminance_weights", 23), 895, 1167, 0, 1, 1 },
    { string_view("sphere", 6), 897, 1185, 0, 0, 0 },
    { string_view("r", 1), 898, 1226, 0, 0, 0 },
    { string_view("r", 1), 898, 1226, 0, 0, 0 },
    { string_view("r", 1), 898, 1226, 0, 0, 0 },
    { string_view("r", 1), 898, 1226, 0, 0, 0 },
    { string_view("r", 1), 898, 1226, 0, 0, 0 },
    { string_view("r", 1), 898, 1226, 0, 0, 0 },
    { string_view("op", 2), 899, 174, 0, 1, 0 },
    { string_view("op", 2), 899, 174, 0, 1, 0 },
    { string_view("op", 2), 899, 174, 0, 1, 0 },
    { string_view("eps", 3), 900, 2, 0, 1, 0 },
    { string_view("b", 1), 901, 2, 0, 1, 0 },
    { string_view("det", 3), 902, 2, 0, 0, 0 },
    { string_view("t", 1), 903, 2, 0, 0, 0 },
    { string_view("bigrad", 6), 904, 2, 0, 1, 0 },
    { string_view("lrad", 4), 905, 2, 0, 1, 0 },
    { string_view("spheres", 7), 906, 1250, 1, 1, 0 },
    { string_view("r", 1), 907, 1226, 0, 0, 0 },
    { string_view("r", 1), 907, 1226, 0, 0, 0 },
    { string_view("r", 1), 907, 1226, 0, 0, 0 },
    { string_view("r", 1), 907, 1226, 0, 0, 0 },
    { string_view("r", 1), 907, 1226, 0, 0, 0 },
    { string_view("r", 1), 907, 1226, 0, 0, 0 },
    { string_view("depth", 5), 908, 0, 0, 0, 0 },
    { string_view("t", 1), 909, 2, 0, 0, 0 },
    { string_view("id", 2), 910, 0, 0, 0, 0 },
    { string_view("s", 1), 911, 1185, 0, 0, 0 },
    { string_view("i", 1), 912, 0, 0, 0, 0 },
    { string_view("d", 1), 913, 2, 0, 1, 0 },
    { string_view("obj", 3), 914, 1185, 0, 1, 0 },
    { string_view("x", 1), 915, 174, 0, 1, 0 },
    { string_view("x", 1), 915, 174, 0, 1, 0 },
    { string_view("x", 1), 915, 174, 0, 1, 0 },
    { string_view("n", 1), 916, 174, 0, 1, 0 },
    { string_view("n", 1), 916, 174, 0, 1, 0 },
    { string_view("n", 1), 916, 174, 0, 1, 0 },
    { string_view("nl", 2), 917, 174, 0, 1, 0 },
    { string_view("nl", 2), 917, 174, 0, 1, 0 },
    { string_view("nl", 2), 917, 174, 0, 1, 0 },
    { string_view("f", 1), 918, 174, 0, 0, 0 },
    { string_view("f", 1), 918, 174, 0, 0, 0 },
    { string_view("f", 1), 918, 174, 0, 0, 0 },
    { string_view("p", 1), 919, 2, 0, 1, 0 },
    { string_view("r1", 2), 920, 2, 0, 1, 0 },
    { string_view("r2", 2), 921, 2, 0, 1, 0 },
    { string_view("r2s", 3), 922, 2, 0, 1, 0 },
    { string_view("w", 1), 923, 174, 0, 1, 0 },
    { string_view("w", 1), 923, 174, 0, 1, 0 },
    { string_view("w", 1), 923, 174, 0, 1, 0 },
    { string_view("u", 1), 924, 174, 0, 1, 0 },
    { string_view("u", 1), 924, 174, 0, 1, 0 },
    { string_view("u", 1), 924, 174, 0, 1, 0 },
    { string_view("v", 1), 925, 174, 0, 1, 0 },
    { string_view("v", 1), 925, 174, 0, 1, 0 },
    { string_view("v", 1), 925, 174, 0, 1, 0 },
    { string_view("d", 1), 926, 174, 0, 1, 0 },
    { string_view("d", 1), 926, 174, 0, 1, 0 },
    { string_view("d", 1), 926, 174, 0, 1, 0 },
    { string_view("reflRay", 7), 927, 1226, 0, 1, 0 },
    { string_view("reflRay", 7), 927, 1226, 0, 1, 0 },
    { string_view("reflRay", 7), 927, 1226, 0, 1, 0 },
    { string_view("reflRay", 7), 927, 1226, 0, 1, 0 },
    { string_view("reflRay", 7), 927, 1226, 0, 1, 0 },
    { string_view("reflRay", 7), 927, 1226, 0, 1, 0 },
    { string_view("into", 4), 928, 146, 0, 1, 0 },
    { string_view("nc", 2), 929, 2, 0, 1, 0 },
    { string_view("nt", 2), 930, 2, 0, 1, 0 },
    { string_view("nnt", 3), 931, 2, 0, 1, 0 },
    { string_view("ddn", 3), 932, 2, 0, 1, 0 },
    { string_view("cos2t", 5), 933, 2, 0, 1, 0 },
    { string_view("tdir", 4), 934, 174, 0, 1, 0 },
    { string_view("tdir", 4), 934, 174, 0, 1, 0 },
    { string_view("tdir", 4), 934, 174, 0, 1, 0 },
    { string_view("a", 1), 935, 2, 0, 1, 0 },
    { string_view("b", 1), 936, 2, 0, 1, 0 },
    { string_view("R0", 2), 937, 2, 0, 1, 0 },
    { string_view("c", 1), 938, 2, 0, 1, 0 },
    { string_view("Re", 2), 939, 2, 0, 1, 0 },
    { string_view("Tr", 2), 940, 2, 0, 1, 0 },
    { string_view("P", 1), 941, 2, 0, 1, 0 },
    { string_view("RP", 2), 942, 2, 0, 1, 0 },
    { string_view("TP", 2), 943, 2, 0, 1, 0 },
    { string_view("temp", 4), 944, 174, 0, 1, 0 },
    { string_view("temp", 4), 944, 174, 0, 1, 0 },
    { string_view("temp", 4), 944, 174, 0, 1, 0 },
    { string_view("w", 1), 945, 0, 1, 1, 0 },
    { string_view("h", 1), 946, 0, 0, 1, 0 },
    { string_view("cam", 3), 947, 1226, 0, 1, 0 },
    { string_view("cam", 3), 947, 1226, 0, 1, 0 },
    { string_view("cam", 3), 947, 1226, 0, 1, 0 },
    { string_view("cam", 3), 947, 1226, 0, 1, 0 },
    { string_view("cam", 3), 947, 1226, 0, 1, 0 },
    { string_view("cam", 3), 947, 1226, 0, 1, 0 },
    { string_view("cx", 2), 948, 174, 0, 1, 0 },
    { string_view("cx", 2), 948, 174, 0, 1, 0 },
    { string_view("cx", 2), 948, 174, 0, 1, 0 },
    { string_view("cy", 2), 949, 174, 0, 1, 0 },
    { string_view("cy", 2), 949, 174, 0, 1, 0 },
    { string_view("cy", 2), 949, 174, 0, 1, 0 },
    { string_view("c", 1), 950, 1254, 0, 1, 0 },
    { string_view("samples", 7), 951, 0, 1, 0, 0 },
    { string_view("y", 1), 952, 0, 0, 0, 0 },
    { string_view("x", 1), 953, 0, 0, 0, 0 },
    { string_view("r1", 2), 954, 2, 0, 1, 0 },
    { string_view("dx", 2), 955, 2, 0, 1, 0 },
    { string_view("r2", 2), 956, 2, 0, 1, 0 },
    { string_view("dy", 2), 957, 2, 0, 1, 0 },
    { string_view("d", 1), 958, 174, 0, 0, 0 },
    { string_view("d", 1), 958, 174, 0, 0, 0 },
    { string_view("d", 1), 958, 174, 0, 0, 0 },
    { string_view("r", 1), 959, 174, 0, 1, 0 },
    { string_view("r", 1), 959, 174, 0, 1, 0 },
    { string_view("r", 1), 959, 174, 0, 1, 0 },
    { string_view("r", 1), 960, 1258, 0, 1, 0 },
    { string_view("row", 3), 961, 1252, 0, 0, 0 },
    { string_view("col", 3), 962, 174, 0, 0, 0 },
    { string_view("col", 3), 962, 174, 0, 0, 0 },
    { string_view("col", 3), 962, 174, 0, 0, 0 },
    { string_view("a", 1), 963, 0, 0, 1, 0 },
    { string_view("b", 1), 964, 9, 0, 1, 0 },
    { string_view("compres1", 8), 965, 3, 0, 1, 0 },
    { string_view("comperr1", 8), 966, 458, 0, 1, 0 },
    { string_view("compres2", 8), 967, 3, 0, 1, 0 },
    { string_view("comperr2", 8), 968, 458, 0, 1, 0 },
    { string_view("v", 1), 969, 11, 0, 1, 0 },
    { string_view("k", 1), 970, 3, 0, 0, 0 },
    { string_view("v", 1), 971, 7, 0, 1, 0 },
    { string_view("k", 1), 972, 0, 0, 0, 0 },
    { string_view("nested", 6), 973, 1260, 0, 1, 0 },
    { string_view("sub", 3), 974, 1260, 0, 1, 0 },
    { string_view("this", 4), 975, 1291, 0, 0, 0 },
    { string_view("this", 4), 975, 1291, 0, 0, 0 },
    { string_view("this", 4), 975, 1291, 0, 0, 0 },
    { string_view("this", 4), 975, 1291, 0, 0, 0 },
    { string_view("o", 1), 976, 1291, 0, 0, 0 },
    { string_view("o", 1), 976, 1291, 0, 0, 0 },
    { string_view("o", 1), 976, 1291, 0, 0, 0 },
    { string_view("o", 1), 976, 1291, 0, 0, 0 },
    { string_view("this", 4), 977, 1291, 0, 0, 0 },
    { string_view("this", 4), 977, 1291, 0, 0, 0 },
    { string_view("this", 4), 977, 1291, 0, 0, 0 },
    { string_view("this", 4), 977, 1291, 0, 0, 0 },
    { string_view("f", 1), 978, 2, 0, 0, 0 },
    { string_view("this", 4), 979, 1291, 0, 0, 0 },
    { string_view("this", 4), 979, 1291, 0, 0, 0 },
    { string_view("this", 4), 979, 1291, 0, 0, 0 },
    { string_view("this", 4), 979, 1291, 0, 0, 0 },
    { string_view("quat_unit", 9), 1003, 1291, 0, 1, 1 },
    { string_view("quat_unit", 9), 1003, 1291, 0, 1, 1 },
    { string_view("quat_unit", 9), 1003, 1291, 0, 1, 1 },
    { string_view("quat_unit", 9), 1003, 1291, 0, 1, 1 },
    { string_view("this", 4), 1004, 1309, 0, 0, 0 },
    { string_view("o", 1), 1005, 1309, 0, 0, 0 },
    { string_view("this", 4), 1006, 1309, 0, 0, 0 },
    { string_view("o", 1), 1007, 1309, 0, 0, 0 },
    { string_view("this", 4), 1008, 1309, 0, 0, 0 },
    { string_view("o", 1), 1009, 1309, 0, 0, 0 },
    { string_view("this", 4), 1010, 1309, 0, 0, 0 },
    { string_view("o", 1), 1011, 1309, 0, 0, 0 },
    { string_view("this", 4), 1024, 1309, 0, 0, 0 },
    { string_view("o", 1), 1025, 1309, 0, 0, 0 },
    { string_view("this", 4), 1044, 1309, 0, 0, 0 },
    { string_view("o", 1), 1045, 1309, 0, 0, 0 },
    { string_view("this", 4), 1046, 1309, 0, 0, 0 },
    { string_view("o", 1), 1047, 1309, 0, 0, 0 },
    { string_view("this", 4), 1057, 1309, 0, 0, 0 },
    { string_view("this", 4), 1058, 1309, 0, 0, 0 },
    { string_view("this", 4), 1059, 1309, 0, 0, 0 },
    { string_view("this", 4), 1060, 1309, 0, 0, 0 },
    { string_view("o", 1), 1061, 1309, 0, 0, 0 },
    { string_view("this", 4), 1062, 1309, 0, 0, 0 },
    { string_view("i", 1), 1063, 0, 0, 0, 0 },
    { string_view("this", 4), 1064, 1318, 0, 0, 0 },
    { string_view("o", 1), 1065, 1318, 0, 0, 0 },
    { string_view("this", 4), 1066, 1318, 0, 0, 0 },
    { string_view("o", 1), 1067, 1318, 0, 0, 0 },
    { string_view("this", 4), 1068, 1318, 0, 0, 0 },
    { string_view("o", 1), 1069, 1318, 0, 0, 0 },
    { string_view("this", 4), 1070, 1318, 0, 0, 0 },
    { string_view("o", 1), 1071, 1318, 0, 0, 0 },
    { string_view("this", 4), 1084, 1318, 0, 0, 0 },
    { string_view("o", 1), 1085, 1318, 0, 0, 0 },
    { string_view("c1", 2), 1100, 1309, 0, 1, 0 },
    { string_view("c2", 2), 1101, 1309, 0, 1, 0 },
    { string_view("c3", 2), 1102, 1309, 0, 0, 0 },
    { string_view("c4", 2), 1103, 1309, 0, 0, 0 },
    { string_view("s1", 2), 1104, 1318, 0, 1, 0 },
    { string_view("s2", 2), 1105, 1318, 0, 1, 0 },
    { string_view("s3", 2), 1106, 1318, 0, 1, 0 },
    { string_view("s4", 2), 1107, 1318, 0, 1, 0 },
    { string_view("this", 4), 1108, 242, 0, 0, 0 },
    { string_view("this", 4), 1108, 242, 0, 0, 0 },
    { string_view("this", 4), 1108, 242, 0, 0, 0 },
    { string_view("this", 4), 1108, 242, 0, 0, 0 },
    { string_view("o", 1), 1109, 242, 0, 0, 0 },
    { string_view("o", 1), 1109, 242, 0, 0, 0 },
    { string_view("o", 1), 1109, 242, 0, 0, 0 },
    { string_view("o", 1), 1109, 242, 0, 0, 0 },
    { string_view("this", 4), 1110, 242, 0, 0, 0 },
    { string_view("this", 4), 1110, 242, 0, 0, 0 },
    { string_view("this", 4), 1110, 242, 0, 0, 0 },
    { string_view("this", 4), 1110, 242, 0, 0, 0 },
    { string_view("o", 1), 1111, 162, 0, 0, 0 },
    { string_view("o", 1), 1111, 162, 0, 0, 0 },
    { string_view("this", 4), 1112, 242, 0, 0, 0 },
    { string_view("this", 4), 1112, 242, 0, 0, 0 },
    { string_view("this", 4), 1112, 242, 0, 0, 0 },
    { string_view("this", 4), 1112, 242, 0, 0, 0 },
    { string_view("this", 4), 1113, 242, 0, 0, 0 },
    { string_view("this", 4), 1113, 242, 0, 0, 0 },
    { string_view("this", 4), 1113, 242, 0, 0, 0 },
    { string_view("this", 4), 1113, 242, 0, 0, 0 },
    { string_view("this", 4), 1114, 242, 0, 0, 0 },
    { string_view("this", 4), 1114, 242, 0, 0, 0 },
    { string_view("this", 4), 1114, 242, 0, 0, 0 },
    { string_view("this", 4), 1114, 242, 0, 0, 0 },
    { string_view("this", 4), 1115, 242, 0, 0, 0 },
    { string_view("this", 4), 1115, 242, 0, 0, 0 },
    { string_view("this", 4), 1115, 242, 0, 0, 0 },
    { string_view("this", 4), 1115, 242, 0, 0, 0 },
    { string_view("mat2x2_identity", 15), 1116, 242, 1, 1, 1 },
    { string_view("mat2x2_identity", 15), 1116, 242, 1, 1, 1 },
    { string_view("mat2x2_identity", 15), 1116, 242, 1, 1, 1 },
    { string_view("mat2x2_identity", 15), 1116, 242, 1, 1, 1 },
    { string_view("a", 1), 1117, 242, 0, 1, 0 },
    { string_view("a", 1), 1117, 242, 0, 1, 0 },
    { string_view("a", 1), 1117, 242, 0, 1, 0 },
    { string_view("a", 1), 1117, 242, 0, 1, 0 },
    { string_view("b", 1), 1118, 242, 0, 1, 0 },
    { string_view("b", 1), 1118, 242, 0, 1, 0 },
    { string_view("b", 1), 1118, 242, 0, 1, 0 },
    { string_view("b", 1), 1118, 242, 0, 1, 0 },
    { string_view("c", 1), 1119, 162, 0, 1, 0 },
    { string_view("c", 1), 1119, 162, 0, 1, 0 },
    { string_view("tests", 5), 1120, 1336, 0, 1, 0 },
    { string_view("results1", 8), 1121, 7, 0, 1, 0 },
    { string_view("t", 1), 1122, 1327, 0, 0, 0 },
    { string_view("results2", 8), 1123, 7, 0, 1, 0 },
    { string_view("t", 1), 1124, 1327, 0, 0, 0 },
    { string_view("results3", 8), 1125, 7, 0, 1, 0 },
    { string_view("t", 1), 1126, 1327, 0, 0, 0 },
    { string_view("results4", 8), 1127, 7, 0, 1, 0 },
    { string_view("t", 1), 1128, 1327, 0, 0, 0 },
    { string_view("btests", 6), 1129, 1338, 0, 1, 0 },
    { string_view("results5", 8), 1130, 11, 0, 1, 0 },
    { string_view("t", 1), 1131, 146, 0, 0, 0 },
    { string_view("btests2", 7), 1132, 1338, 0, 1, 0 },
    { string_view("results6", 8), 1133, 11, 0, 1, 0 },
    { string_view("t", 1), 1134, 146, 0, 0, 0 },
    { string_view("f", 1), 1135, 1340, 0, 0, 0 },
    { string_view("f", 1), 1136, 1342, 0, 0, 0 },
    { string_view("f", 1), 1137, 1344, 0, 0, 0 },
    { string_view("f", 1), 1138, 1346, 0, 0, 0 },
    { string_view("f", 1), 1139, 1348, 0, 0, 0 },
    { string_view("f", 1), 1140, 1350, 0, 0, 0 },
    { string_view("f", 1), 1141, 1352, 0, 0, 0 },
    { string_view("f", 1), 1142, 1354, 0, 0, 0 },
    { string_view("f", 1), 1143, 1356, 0, 0, 0 },
    { string_view("f", 1), 1144, 1358, 0, 0, 0 },
    { string_view("xs", 2), 1145, 7, 0, 0, 0 },
    { string_view("fun", 3), 1146, 1360, 0, 0, 0 },
    { string_view("r", 1), 1147, 7, 0, 1, 0 },
    { string_view("x", 1), 1148, 0, 0, 0, 0 },
    { string_view("i", 1), 1149, 0, 0, 0, 0 },
    { string_view("xs", 2), 1150, 7, 0, 0, 0 },
    { string_view("init", 4), 1151, 0, 0, 1, 0 },
    { string_view("f", 1), 1152, 1362, 0, 0, 0 },
    { string_view("body", 4), 1153, 1364, 0, 0, 0 },
    { string_view("catch", 5), 1154, 1366, 0, 0, 0 },
    { string_view("err", 3), 1155, 458, 0, 1, 0 },
    { string_view("stack_trace", 11), 1156, 3, 0, 1, 0 },
    { string_view("body", 4), 1157, 1364, 0, 0, 0 },
    { string_view("v", 1), 1158, 3, 0, 0, 0 },
    { string_view("n", 1), 1159, 0, 0, 0, 0 },
    { string_view("xs", 2), 1160, 7, 0, 0, 0 },
    { string_view("acc", 3), 1161, 0, 0, 0, 0 },
    { string_view("fun", 3), 1162, 1368, 0, 0, 0 },
    { string_view("_", 1), 1163, 0, 0, 1, 0 },
    { string_view("xs", 2), 1164, 0, 0, 0, 0 },
    { string_view("fun", 3), 1165, 1370, 0, 0, 0 },
    { string_view("r", 1), 1166, 486, 0, 1, 0 },
    { string_view("x", 1), 1167, 0, 0, 0, 0 },
    { string_view("i", 1), 1168, 0, 0, 0, 0 },
    { string_view("this", 4), 1169, 460, 0, 0, 0 },
    { string_view("key", 3), 1170, 2, 0, 0, 0 },
    { string_view("h", 1), 1171, 0, 0, 1, 0 },
    { string_view("c", 1), 1172, 484, 0, 0, 0 },
    { string_view("size", 4), 1173, 0, 0, 0, 0 },
    { string_view("xs", 2), 1174, 0, 0, 0, 0 },
    { string_view("fun", 3), 1175, 1370, 0, 0, 0 },
    { string_view("r", 1), 1176, 657, 0, 1, 0 },
    { string_view("x", 1), 1177, 0, 0, 0, 0 },
    { string_view("i", 1), 1178, 0, 0, 0, 0 },
    { string_view("this", 4), 1179, 631, 0, 0, 0 },
    { string_view("key", 3), 1180, 3, 0, 0, 0 },
    { string_view("value", 5), 1181, 458, 0, 0, 0 },
    { string_view("c", 1), 1182, 655, 0, 1, 0 },
    { string_view("h", 1), 1183, 0, 0, 1, 0 },
    { string_view("this", 4), 1184, 631, 0, 0, 0 },
    { string_view("key", 3), 1185, 3, 0, 0, 0 },
    { string_view("h", 1), 1186, 0, 0, 1, 0 },
    { string_view("c", 1), 1187, 655, 0, 0, 0 },
    { string_view("this", 4), 1188, 631, 0, 0, 0 },
    { string_view("key", 3), 1189, 3, 0, 0, 0 },
    { string_view("default_val", 11), 1190, 458, 0, 0, 0 },
    { string_view("c", 1), 1191, 655, 0, 1, 0 },
    { string_view("this", 4), 1192, 631, 0, 0, 0 },
    { string_view("key", 3), 1193, 3, 0, 0, 0 },
    { string_view("h", 1), 1194, 0, 0, 1, 0 },
    { string_view("c", 1), 1195, 655, 0, 0, 0 },
    { string_view("size", 4), 1196, 0, 0, 0, 0 },
    { string_view("xs", 2), 1197, 0, 0, 0, 0 },
    { string_view("fun", 3), 1198, 1372, 0, 0, 0 },
    { string_view("r", 1), 1199, 697, 0, 1, 0 },
    { string_view("x", 1), 1200, 0, 0, 0, 0 },
    { string_view("i", 1), 1201, 0, 0, 0, 0 },
    { string_view("this", 4), 1202, 659, 0, 0, 0 },
    { string_view("key", 3), 1203, 207, 0, 0, 0 },
    { string_view("key", 3), 1203, 207, 0, 0, 0 },
    { string_view("key", 3), 1203, 207, 0, 0, 0 },
    { string_view("value", 5), 1204, 207, 0, 0, 0 },
    { string_view("value", 5), 1204, 207, 0, 0, 0 },
    { string_view("value", 5), 1204, 207, 0, 0, 0 },
    { string_view("c", 1), 1205, 695, 0, 1, 0 },
    { string_view("h", 1), 1206, 0, 0, 1, 0 },
    { string_view("this", 4), 1207, 659, 0, 0, 0 },
    { string_view("key", 3), 1208, 207, 0, 0, 0 },
    { string_view("key", 3), 1208, 207, 0, 0, 0 },
    { string_view("key", 3), 1208, 207, 0, 0, 0 },
    { string_view("h", 1), 1209, 0, 0, 1, 0 },
    { string_view("c", 1), 1210, 695, 0, 0, 0 },
    { string_view("this", 4), 1211, 659, 0, 0, 0 },
    { string_view("key", 3), 1212, 207, 0, 0, 0 },
    { string_view("key", 3), 1212, 207, 0, 0, 0 },
    { string_view("key", 3), 1212, 207, 0, 0, 0 },
    { string_view("default_val", 11), 1213, 207, 0, 0, 0 },
    { string_view("default_val", 11), 1213, 207, 0, 0, 0 },
    { string_view("default_val", 11), 1213, 207, 0, 0, 0 },
    { string_view("c", 1), 1214, 695, 0, 1, 0 },
    { string_view("this", 4), 1215, 659, 0, 0, 0 },
    { string_view("key", 3), 1216, 207, 0, 0, 0 },
    { string_view("key", 3), 1216, 207, 0, 0, 0 },
    { string_view("key", 3), 1216, 207, 0, 0, 0 },
    { string_view("h", 1), 1217, 0, 0, 1, 0 },
    { string_view("c", 1), 1218, 695, 0, 0, 0 },
    { string_view("f", 1), 1219, 1374, 0, 0, 0 },
    { string_view("f", 1), 1220, 1376, 0, 0, 0 },
    { string_view("f", 1), 1221, 1378, 0, 0, 0 },
    { string_view("f", 1), 1222, 1380, 0, 0, 0 },
    { string_view("xs", 2), 1223, 0, 0, 0, 0 },
    { string_view("fun", 3), 1224, 1382, 0, 0, 0 },
    { string_view("r", 1), 1225, 7, 0, 1, 0 },
    { string_view("x", 1), 1226, 0, 0, 0, 0 },
    { string_view("i", 1), 1227, 0, 0, 0, 0 },
    { string_view("f", 1), 1228, 1384, 0, 0, 0 },
    { string_view("size", 4), 1229, 0, 0, 0, 0 },
    { string_view("xs", 2), 1230, 0, 0, 0, 0 },
    { string_view("fun", 3), 1231, 1386, 0, 0, 0 },
    { string_view("r", 1), 1232, 542, 0, 1, 0 },
    { string_view("x", 1), 1233, 0, 0, 0, 0 },
    { string_view("i", 1), 1234, 0, 0, 0, 0 },
    { string_view("this", 4), 1235, 488, 0, 0, 0 },
    { string_view("key", 3), 1236, 3, 0, 0, 0 },
    { string_view("h", 1), 1237, 0, 0, 1, 0 },
    { string_view("c", 1), 1238, 512, 0, 0, 0 },
    { string_view("f", 1), 1239, 1388, 0, 0, 0 },
    { string_view("f", 1), 1240, 1390, 1, 0, 0 },
    { string_view("i", 1), 1241, 1392, 0, 0, 0 },
    { string_view("f", 1), 1242, 1394, 0, 0, 0 },
    { string_view("x", 1), 1243, 3, 0, 0, 0 },
    { string_view("f", 1), 1244, 1396, 0, 0, 0 },
    { string_view("a", 1), 1245, 3, 0, 0, 0 },
    { string_view("f", 1), 1246, 1398, 0, 0, 0 },
    { string_view("f", 1), 1247, 1400, 0, 0, 0 },
    { string_view("this", 4), 1248, 762, 0, 0, 0 },
    { string_view("c", 1), 1249, 0, 0, 0, 0 },
    { string_view("this", 4), 1250, 771, 0, 0, 0 },
    { string_view("c", 1), 1251, 0, 0, 0, 0 },
    { string_view("this", 4), 1252, 771, 0, 0, 0 },
    { string_view("c", 1), 1253, 0, 0, 0, 0 },
    { string_view("this", 4), 1254, 762, 0, 0, 0 },
    { string_view("d", 1), 1255, 1402, 0, 0, 0 },
    { string_view("f", 1), 1256, 1411, 0, 0, 0 },
    { string_view("f", 1), 1257, 1413, 0, 0, 0 },
    { string_view("this", 4), 1258, 829, 0, 0, 0 },
    { string_view("t", 1), 1259, 3, 0, 0, 0 },
    { string_view("this", 4), 1260, 829, 0, 0, 0 },
    { string_view("u", 1), 1261, 3, 0, 0, 0 },
    { string_view("a", 1), 1262, 829, 0, 0, 0 },
    { string_view("t", 1), 1263, 3, 0, 0, 0 },
    { string_view("x", 1), 1264, 2, 0, 0, 0 },
    { string_view("y", 1), 1265, 2, 0, 0, 0 },
    { string_view("f", 1), 1266, 1415, 0, 0, 0 },
    { string_view("size", 4), 1267, 0, 0, 0, 0 },
    { string_view("xs", 2), 1268, 0, 0, 0, 0 },
    { string_view("fun", 3), 1269, 1417, 0, 0, 0 },
    { string_view("r", 1), 1270, 540, 0, 1, 0 },
    { string_view("x", 1), 1271, 0, 0, 0, 0 },
    { string_view("i", 1), 1272, 0, 0, 0, 0 },
    { string_view("this", 4), 1273, 514, 0, 0, 0 },
    { string_view("key", 3), 1274, 3, 0, 0, 0 },
    { string_view("value", 5), 1275, 0, 0, 0, 0 },
    { string_view("c", 1), 1276, 538, 0, 1, 0 },
    { string_view("h", 1), 1277, 0, 0, 1, 0 },
    { string_view("this", 4), 1278, 514, 0, 0, 0 },
    { string_view("key", 3), 1279, 3, 0, 0, 0 },
    { string_view("h", 1), 1280, 0, 0, 1, 0 },
    { string_view("c", 1), 1281, 538, 0, 0, 0 },
    { string_view("this", 4), 1282, 514, 0, 0, 0 },
    { string_view("key", 3), 1283, 3, 0, 0, 0 },
    { string_view("default_val", 11), 1284, 0, 0, 0, 0 },
    { string_view("c", 1), 1285, 538, 0, 1, 0 },
    { string_view("this", 4), 1286, 514, 0, 0, 0 },
    { string_view("key", 3), 1287, 3, 0, 0, 0 },
    { string_view("h", 1), 1288, 0, 0, 1, 0 },
    { string_view("c", 1), 1289, 538, 0, 0, 0 },
    { string_view("f", 1), 1290, 1419, 0, 0, 0 },
    { string_view("f", 1), 1291, 1421, 0, 0, 0 },
    { string_view("f", 1), 1292, 1423, 0, 0, 0 },
    { string_view("f", 1), 1293, 1425, 0, 0, 0 },
    { string_view("xs", 2), 1294, 0, 0, 0, 0 },
    { string_view("fun", 3), 1295, 1427, 0, 0, 0 },
    { string_view("r", 1), 1296, 1027, 0, 1, 0 },
    { string_view("x", 1), 1297, 0, 0, 0, 0 },
    { string_view("i", 1), 1298, 0, 0, 0, 0 },
    { string_view("xs", 2), 1299, 0, 0, 0, 0 },
    { string_view("fun", 3), 1300, 1429, 0, 0, 0 },
    { string_view("r", 1), 1301, 1005, 0, 1, 0 },
    { string_view("x", 1), 1302, 0, 0, 0, 0 },
    { string_view("i", 1), 1303, 0, 0, 0, 0 },
    { string_view("distancef", 9), 1304, 1431, 0, 0, 0 },
    { string_view("startnode", 9), 1305, 919, 0, 0, 0 },
    { string_view("endnode", 7), 1306, 919, 1, 0, 0 },
    { string_view("costf", 5), 1307, 1007, 1, 0, 0 },
    { string_view("distancef", 9), 1308, 1431, 1, 0, 0 },
    { string_view("neighbors", 9), 1309, 1011, 1, 0, 0 },
    { string_view("startnode", 9), 1314, 919, 0, 0, 0 },
    { string_view("endcondition", 12), 1315, 1433, 0, 0, 0 },
    { string_view("generatenewstates", 17), 1316, 1435, 0, 0, 0 },
    { string_view("heuristic", 9), 1317, 1437, 1, 0, 0 },
    { string_view("openlist", 8), 1318, 1005, 1, 1, 0 },
    { string_view("n", 1), 1319, 997, 1, 0, 0 },
    { string_view("c", 1), 1320, 919, 0, 0, 0 },
    { string_view("path", 4), 1321, 1005, 0, 1, 0 },
    { string_view("n", 1), 1322, 919, 0, 0, 0 },
    { string_view("n", 1), 1327, 919, 1, 0, 0 },
    { string_view("f", 1), 1328, 1025, 1, 0, 0 },
    { string_view("n", 1), 1331, 919, 0, 0, 0 },
    { string_view("f", 1), 1332, 1439, 0, 0, 0 },
    { string_view("delta", 5), 1333, 25, 0, 0, 0 },
    { string_view("delta", 5), 1333, 25, 0, 0, 0 },
    { string_view("np", 2), 1334, 25, 0, 1, 0 },
    { string_view("np", 2), 1334, 25, 0, 1, 0 },
    { string_view("nn", 2), 1335, 919, 0, 0, 0 },
    { string_view("cost", 4), 1336, 0, 0, 1, 0 },
    { string_view("delta", 5), 1337, 25, 0, 0, 0 },
    { string_view("delta", 5), 1337, 25, 0, 0, 0 },
    { string_view("cost", 4), 1338, 0, 0, 0, 0 },
    { string_view("nn", 2), 1339, 919, 0, 0, 0 },
    { string_view("G", 1), 1340, 2, 0, 1, 0 },
    { string_view("state", 5), 1341, 25, 0, 0, 0 },
    { string_view("state", 5), 1341, 25, 0, 0, 0 },
    { string_view("xs", 2), 1342, 1027, 0, 0, 0 },
    { string_view("fun", 3), 1343, 1441, 0, 0, 0 },
    { string_view("r", 1), 1344, 11, 0, 1, 0 },
    { string_view("x", 1), 1345, 1005, 0, 0, 0 },
    { string_view("i", 1), 1346, 0, 0, 0, 0 },
    { string_view("xs", 2), 1347, 1005, 0, 0, 0 },
    { string_view("acc", 3), 1348, 3, 0, 0, 0 },
    { string_view("fun", 3), 1349, 1443, 0, 0, 0 },
    { string_view("_", 1), 1350, 919, 0, 1, 0 },
    { string_view("startnode", 9), 1351, 49, 0, 0, 0 },
    { string_view("endcondition", 12), 1352, 1445, 0, 0, 0 },
    { string_view("generatenewstates", 17), 1353, 1447, 0, 0, 0 },
    { string_view("heuristic", 9), 1354, 1449, 1, 0, 0 },
    { string_view("openlist", 8), 1355, 148, 1, 1, 0 },
    { string_view("n", 1), 1356, 109, 1, 0, 0 },
    { string_view("c", 1), 1357, 49, 0, 0, 0 },
    { string_view("path", 4), 1358, 148, 0, 1, 0 },
    { string_view("delta", 5), 1359, 138, 0, 0, 0 },
    { string_view("cost", 4), 1360, 0, 0, 0, 0 },
    { string_view("nn", 2), 1361, 49, 0, 0, 0 },
    { string_view("G", 1), 1362, 2, 0, 1, 0 },
    { string_view("xs", 2), 1363, 148, 0, 0, 0 },
    { string_view("fun", 3), 1364, 1451, 0, 0, 0 },
    { string_view("r", 1), 1365, 11, 0, 1, 0 },
    { string_view("x", 1), 1366, 49, 0, 0, 0 },
    { string_view("i", 1), 1367, 0, 0, 0, 0 },
    { string_view("xs", 2), 1368, 0, 0, 0, 0 },
    { string_view("fun", 3), 1369, 1453, 0, 0, 0 },
    { string_view("r", 1), 1370, 13, 0, 1, 0 },
    { string_view("x", 1), 1371, 0, 0, 0, 0 },
    { string_view("i", 1), 1372, 0, 0, 0, 0 },
    { string_view("xs", 2), 1373, 0, 0, 0, 0 },
    { string_view("fun", 3), 1374, 1455, 0, 0, 0 },
    { string_view("r", 1), 1375, 7, 0, 1, 0 },
    { string_view("x", 1), 1376, 0, 0, 0, 0 },
    { string_view("i", 1), 1377, 0, 0, 0, 0 },
    { string_view("xs", 2), 1378, 37, 0, 0, 0 },
    { string_view("fun", 3), 1379, 1457, 0, 0, 0 },
    { string_view("r", 1), 1380, 37, 0, 1, 0 },
    { string_view("x", 1), 1381, 25, 0, 0, 0 },
    { string_view("x", 1), 1381, 25, 0, 0, 0 },
    { string_view("i", 1), 1382, 0, 0, 0, 0 },
    { string_view("xs", 2), 1383, 37, 0, 0, 0 },
    { string_view("fun", 3), 1384, 1459, 0, 0, 0 },
    { string_view("r", 1), 1385, 7, 0, 1, 0 },
    { string_view("x", 1), 1386, 25, 0, 0, 0 },
    { string_view("x", 1), 1386, 25, 0, 0, 0 },
    { string_view("i", 1), 1387, 0, 0, 0, 0 },
    { string_view("xs", 2), 1388, 37, 0, 0, 0 },
    { string_view("fun", 3), 1389, 1461, 0, 0, 0 },
    { string_view("r", 1), 1390, 37, 0, 1, 0 },
    { string_view("x", 1), 1391, 25, 0, 0, 0 },
    { string_view("x", 1), 1391, 25, 0, 0, 0 },
    { string_view("i", 1), 1392, 0, 0, 0, 0 },
    { string_view("xs", 2), 1393, 11, 0, 0, 0 },
    { string_view("fun", 3), 1394, 1463, 0, 0, 0 },
    { string_view("x", 1), 1395, 3, 0, 0, 0 },
    { string_view("i", 1), 1396, 0, 0, 0, 0 },
    { string_view("tok", 3), 1397, 3, 0, 0, 0 },
    { string_view("a", 1), 1398, 3, 0, 1, 0 },
    { string_view("xs", 2), 1399, 1092, 0, 0, 0 },
    { string_view("acc", 3), 1400, 3, 0, 0, 0 },
    { string_view("fun", 3), 1401, 1465, 0, 0, 0 },
    { string_view("_", 1), 1402, 1074, 0, 1, 0 },
    { string_view("xs", 2), 1403, 1092, 0, 0, 0 },
    { string_view("acc", 3), 1404, 0, 0, 0, 0 },
    { string_view("fun", 3), 1405, 1467, 0, 0, 0 },
    { string_view("_", 1), 1406, 1074, 0, 1, 0 },
    { string_view("xs", 2), 1407, 1092, 0, 0, 0 },
    { string_view("acc", 3), 1408, 0, 0, 0, 0 },
    { string_view("fun", 3), 1409, 1469, 0, 0, 0 },
    { string_view("_", 1), 1410, 1074, 0, 1, 0 },
    { string_view("xs", 2), 1411, 1092, 0, 0, 0 },
    { string_view("fun", 3), 1412, 1471, 0, 0, 0 },
    { string_view("r", 1), 1413, 11, 0, 1, 0 },
    { string_view("x", 1), 1414, 1074, 0, 0, 0 },
    { string_view("i", 1), 1415, 0, 0, 0, 0 },
    { string_view("v", 1), 1416, 25, 1, 0, 0 },
    { string_view("v", 1), 1416, 25, 1, 0, 0 },
    { string_view("fun", 3), 1417, 1473, 1, 0, 0 },
    { string_view("xs", 2), 1419, 0, 0, 0, 0 },
    { string_view("fun", 3), 1420, 1475, 0, 0, 0 },
    { string_view("r", 1), 1421, 1121, 0, 1, 0 },
    { string_view("x", 1), 1422, 0, 0, 0, 0 },
    { string_view("i", 1), 1423, 0, 0, 0, 0 },
    { string_view("y", 1), 1424, 0, 1, 0, 0 },
    { string_view("xs", 2), 1426, 0, 0, 0, 0 },
    { string_view("fun", 3), 1427, 1477, 0, 0, 0 },
    { string_view("r", 1), 1428, 1119, 0, 1, 0 },
    { string_view("x", 1), 1429, 0, 0, 0, 0 },
    { string_view("i", 1), 1430, 0, 0, 0, 0 },
    { string_view("x", 1), 1431, 0, 0, 0, 0 },
    { string_view("xs", 2), 1432, 0, 0, 0, 0 },
    { string_view("fun", 3), 1433, 1479, 0, 0, 0 },
    { string_view("r", 1), 1434, 1140, 0, 1, 0 },
    { string_view("x", 1), 1435, 0, 0, 0, 0 },
    { string_view("i", 1), 1436, 0, 0, 0, 0 },
    { string_view("xs", 2), 1437, 0, 0, 0, 0 },
    { string_view("fun", 3), 1438, 1481, 0, 0, 0 },
    { string_view("r", 1), 1439, 1138, 0, 1, 0 },
    { string_view("x", 1), 1440, 0, 0, 0, 0 },
    { string_view("i", 1), 1441, 0, 0, 0, 0 },
    { string_view("f", 1), 1442, 1483, 0, 0, 0 },
    { string_view("y", 1), 1443, 0, 0, 0, 0 },
    { string_view("x", 1), 1444, 0, 0, 0, 0 },
    { string_view("xs", 2), 1445, 0, 0, 0, 0 },
    { string_view("fun", 3), 1446, 1485, 0, 0, 0 },
    { string_view("r", 1), 1447, 9, 0, 1, 0 },
    { string_view("x", 1), 1448, 0, 0, 0, 0 },
    { string_view("i", 1), 1449, 0, 0, 0, 0 },
    { string_view("xs", 2), 1450, 0, 0, 0, 0 },
    { string_view("fun", 3), 1451, 1487, 0, 0, 0 },
    { string_view("r", 1), 1452, 9, 0, 1, 0 },
    { string_view("x", 1), 1453, 0, 0, 0, 0 },
    { string_view("i", 1), 1454, 0, 0, 0, 0 },
    { string_view("xs", 2), 1455, 0, 0, 0, 0 },
    { string_view("fun", 3), 1456, 1489, 0, 0, 0 },
    { string_view("r", 1), 1457, 9, 0, 1, 0 },
    { string_view("x", 1), 1458, 0, 0, 0, 0 },
    { string_view("i", 1), 1459, 0, 0, 0, 0 },
    { string_view("xs", 2), 1460, 1148, 0, 0, 0 },
    { string_view("fun", 3), 1461, 1491, 0, 0, 0 },
    { string_view("r", 1), 1462, 1148, 0, 1, 0 },
    { string_view("x", 1), 1463, 162, 0, 0, 0 },
    { string_view("x", 1), 1463, 162, 0, 0, 0 },
    { string_view("i", 1), 1464, 0, 0, 0, 0 },
    { string_view("xs", 2), 1465, 1165, 0, 0, 0 },
    { string_view("lt", 2), 1466, 1493, 1, 0, 0 },
    { string_view("pivot", 5), 1467, 1150, 1, 1, 0 },
    { string_view("pivot", 5), 1467, 1150, 1, 1, 0 },
    { string_view("pivot", 5), 1467, 1150, 1, 1, 0 },
    { string_view("tail", 4), 1468, 1165, 0, 1, 0 },
    { string_view("f1", 2), 1469, 1165, 0, 1, 0 },
    { string_view("f2", 2), 1470, 1165, 0, 1, 0 },
    { string_view("xs", 2), 1472, 1165, 0, 0, 0 },
    { string_view("fun", 3), 1473, 1495, 0, 0, 0 },
    { string_view("t", 1), 1474, 1165, 0, 1, 0 },
    { string_view("f", 1), 1475, 1165, 0, 1, 0 },
    { string_view("x", 1), 1476, 1150, 0, 0, 0 },
    { string_view("x", 1), 1476, 1150, 0, 0, 0 },
    { string_view("x", 1), 1476, 1150, 0, 0, 0 },
    { string_view("i", 1), 1477, 0, 0, 0, 0 },
    { string_view("_", 1), 1478, 1150, 0, 0, 0 },
    { string_view("_", 1), 1478, 1150, 0, 0, 0 },
    { string_view("_", 1), 1478, 1150, 0, 0, 0 },
    { string_view("xs", 2), 1479, 1148, 0, 0, 0 },
    { string_view("fun", 3), 1480, 1497, 0, 0, 0 },
    { string_view("r", 1), 1481, 13, 0, 1, 0 },
    { string_view("x", 1), 1482, 162, 0, 0, 0 },
    { string_view("x", 1), 1482, 162, 0, 0, 0 },
    { string_view("i", 1), 1483, 0, 0, 0, 0 },
    { string_view("xs", 2), 1484, 13, 0, 0, 0 },
    { string_view("fun", 3), 1485, 1499, 0, 0, 0 },
    { string_view("r", 1), 1486, 13, 0, 1, 0 },
    { string_view("x", 1), 1487, 7, 0, 0, 0 },
    { string_view("i", 1), 1488, 0, 0, 0, 0 },
    { string_view("xs", 2), 1489, 7, 0, 0, 0 },
    { string_view("lt", 2), 1490, 1501, 1, 0, 0 },
    { string_view("pivot", 5), 1491, 0, 1, 1, 0 },
    { string_view("tail", 4), 1492, 7, 0, 1, 0 },
    { string_view("f1", 2), 1493, 7, 0, 1, 0 },
    { string_view("f2", 2), 1494, 7, 0, 1, 0 },
    { string_view("xs", 2), 1495, 7, 0, 0, 0 },
    { string_view("fun", 3), 1496, 1495, 0, 0, 0 },
    { string_view("t", 1), 1497, 7, 0, 1, 0 },
    { string_view("f", 1), 1498, 7, 0, 1, 0 },
    { string_view("x", 1), 1499, 0, 0, 0, 0 },
    { string_view("i", 1), 1500, 0, 0, 0, 0 },
    { string_view("_", 1), 1501, 0, 0, 0, 0 },
    { string_view("xs", 2), 1502, 7, 0, 0, 0 },
    { string_view("fun", 3), 1503, 1503, 0, 0, 0 },
    { string_view("x", 1), 1504, 0, 0, 0, 0 },
    { string_view("i", 1), 1505, 0, 0, 0, 0 },
    { string_view("xs", 2), 1506, 7, 0, 0, 0 },
    { string_view("fun", 3), 1507, 1505, 0, 0, 0 },
    { string_view("acc", 3), 1508, 0, 0, 0, 0 },
    { string_view("i", 1), 1509, 0, 0, 0, 0 },
    { string_view("xs", 2), 1510, 13, 0, 0, 0 },
    { string_view("fun", 3), 1511, 1507, 0, 0, 0 },
    { string_view("x", 1), 1512, 7, 0, 0, 0 },
    { string_view("i", 1), 1513, 0, 0, 0, 0 },
    { string_view("xs", 2), 1514, 13, 0, 0, 0 },
    { string_view("fun", 3), 1515, 1509, 0, 0, 0 },
    { string_view("x", 1), 1516, 7, 0, 0, 0 },
    { string_view("i", 1), 1517, 0, 0, 0, 0 },
    { string_view("xs", 2), 1518, 0, 0, 0, 0 },
    { string_view("fun", 3), 1519, 1511, 0, 0, 0 },
    { string_view("r", 1), 1520, 1254, 0, 1, 0 },
    { string_view("x", 1), 1521, 0, 0, 0, 0 },
    { string_view("i", 1), 1522, 0, 0, 0, 0 },
    { string_view("xs", 2), 1523, 0, 0, 0, 0 },
    { string_view("fun", 3), 1524, 1513, 0, 0, 0 },
    { string_view("r", 1), 1525, 1252, 0, 1, 0 },
    { string_view("x", 1), 1526, 0, 0, 0, 0 },
    { string_view("i", 1), 1527, 0, 0, 0, 0 },
    { string_view("xs", 2), 1528, 1254, 0, 0, 0 },
    { string_view("fun", 3), 1529, 1515, 0, 0, 0 },
    { string_view("r", 1), 1530, 1258, 0, 1, 0 },
    { string_view("x", 1), 1531, 1252, 0, 0, 0 },
    { string_view("i", 1), 1532, 0, 0, 0, 0 },
    { string_view("xs", 2), 1533, 1252, 0, 0, 0 },
    { string_view("fun", 3), 1534, 1517, 0, 0, 0 },
    { string_view("r", 1), 1535, 1256, 0, 1, 0 },
    { string_view("x", 1), 1536, 174, 0, 0, 0 },
    { string_view("x", 1), 1536, 174, 0, 0, 0 },
    { string_view("x", 1), 1536, 174, 0, 0, 0 },
    { string_view("i", 1), 1537, 0, 0, 0, 0 },
    { string_view("f", 1), 1538, 1519, 0, 0, 0 },
    { string_view("f", 1), 1539, 1521, 0, 0, 0 },
    { string_view("f", 1), 1540, 1523, 0, 0, 0 },
    { string_view("this", 4), 1541, 1309, 0, 0, 0 },
    { string_view("o", 1), 1542, 1309, 0, 0, 0 },
    { string_view("this", 4), 1543, 1309, 0, 0, 0 },
    { string_view("f", 1), 1544, 1525, 0, 0, 0 },
    { string_view("xs", 2), 1545, 1336, 0, 0, 0 },
    { string_view("fun", 3), 1546, 1527, 0, 0, 0 },
    { string_view("r", 1), 1547, 7, 0, 1, 0 },
    { string_view("x", 1), 1548, 1327, 0, 0, 0 },
    { string_view("i", 1), 1549, 0, 0, 0, 0 },
    { string_view("xs", 2), 1550, 1336, 0, 0, 0 },
    { string_view("fun", 3), 1551, 1529, 0, 0, 0 },
    { string_view("r", 1), 1552, 7, 0, 1, 0 },
    { string_view("x", 1), 1553, 1327, 0, 0, 0 },
    { string_view("i", 1), 1554, 0, 0, 0, 0 },
    { string_view("xs", 2), 1555, 1336, 0, 0, 0 },
    { string_view("fun", 3), 1556, 1531, 0, 0, 0 },
    { string_view("r", 1), 1557, 7, 0, 1, 0 },
    { string_view("x", 1), 1558, 1327, 0, 0, 0 },
    { string_view("i", 1), 1559, 0, 0, 0, 0 },
    { string_view("xs", 2), 1560, 1336, 0, 0, 0 },
    { string_view("fun", 3), 1561, 1533, 0, 0, 0 },
    { string_view("r", 1), 1562, 7, 0, 1, 0 },
    { string_view("x", 1), 1563, 1327, 0, 0, 0 },
    { string_view("i", 1), 1564, 0, 0, 0, 0 },
    { string_view("xs", 2), 1565, 1338, 0, 0, 0 },
    { string_view("fun", 3), 1566, 1535, 0, 0, 0 },
    { string_view("r", 1), 1567, 11, 0, 1, 0 },
    { string_view("x", 1), 1568, 146, 0, 0, 0 },
    { string_view("i", 1), 1569, 0, 0, 0, 0 },
    { string_view("xs", 2), 1570, 1338, 0, 0, 0 },
    { string_view("fun", 3), 1571, 1537, 0, 0, 0 },
    { string_view("r", 1), 1572, 11, 0, 1, 0 },
    { string_view("x", 1), 1573, 146, 0, 0, 0 },
    { string_view("i", 1), 1574, 0, 0, 0, 0 },
};

static const lobster::VMEnumVal bool_vals0[] = {
    { string_view("false", 5), 0 },
    { string_view("true", 4), 1 },
};

static const lobster::VMEnumVal Material_vals1[] = {
    { string_view("DIFF", 4), 0 },
    { string_view("SPEC", 4), 1 },
    { string_view("REFR", 4), 2 },
};

static const lobster::VMEnum enums[] = {
    { string_view("bool", 4), span(bool_vals0), 0 },
    { string_view("Material", 8), span(Material_vals1), 0 },
};

static const int ser_ids[] = {
  562, 1590, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 574,
};

static const int subfunctions_to_function[] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
  16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
  32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
  48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
  64, 65, 66, 67, 68, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78,
  79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 93,
  94, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108,
  109, 110, 111, 112, 113, 114, 115, 115, 116, 117, 118, 119, 120, 121, 122, 123,
  124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139,
  140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 153, 153,
  153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168,
  169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 183,
  184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199,
  200, 201, 202, 203, 204, 205, 206, 207, 208, 208, 209, 210, 211, 212, 213, 214,
  215, 210, 211, 212, 213, 214, 209, 210, 216, 216, 216, 216, 217, 218, 219, 220,
  221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236,
  236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251,
  252, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267,
  268, 269, 270, 271, 270, 271, 270, 271, 270, 271, 270, 271, 270, 271, 272, 273,
  274, 275, 276, 277, 278, 279, 280, 281, 282, 283, 284, 285, 286, 287, 285, 288,
  286, 289, 290, 287, 291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 302,
  303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318,
  319, 320, 321, 322, 323, 324, 325, 326, 327, 328, 329, 330, 331, 332, 333, 334,
  335, 336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 346, 347, 348, 349, 350,
  351, 351, 352, 353, 354, 355, 356, 357, 358, 359, 360, 361, 351, 362, 363, 364,
  365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 380,
  381, 382, 383, 384, 385, 352, 386, 387, 388, 389, 360, 361, 351, 362, 363, 364,
  365, 366, 367, 368, 379, 380, 381, 382, 383, 384, 385, 352, 389, 390, 351, 351,
  391, 392, 393, 394, 395, 396, 397, 398, 399, 400, 401, 402, 403, 57, 57, 57,
  57, 57, 57, 57, 57, 57, 57, 7, 29, 164, 169, 170, 57, 124, 125, 126,
  161, 17, 7, 134, 147, 7, 148, 135, 134, 141, 134, 147, 7, 148, 135, 134,
  141, 134, 57, 57, 57, 57, 7, 57, 147, 7, 148, 134, 57, 197, 198, 196,
  198, 57, 202, 57, 204, 57, 57, 211, 211, 212, 214, 216, 57, 57, 220, 221,
  223, 224, 57, 147, 7, 148, 135, 134, 141, 134, 57, 57, 57, 57, 7, 7,
  251, 245, 246, 247, 249, 243, 246, 244, 247, 248, 252, 248, 244, 249, 7, 17,
  243, 244, 7, 7, 7, 7, 7, 10, 15, 298, 17, 17, 17, 7, 97, 98,
  7, 98, 99, 7, 99, 7, 7, 309, 7, 7, 7, 7, 48, 49, 13, 49,
  7, 7, 48, 13, 49, 23, 19, 15, 15, 7, 7, 7, 7, 57, 57, 57,
  379, 352, 57, 7, 7, 7, 7, 7, 7,
};

extern "C" void compiled_entry_point(VMRef vm, StackPtr sp) {
    if (vm.vma.nfr.HashAll() != 12557089539824322831ULL) vm.BuiltinError("code compiled with mismatching builtin function library");
    fun_10000002(vm, sp);
}

int main(int argc, char *argv[]) {
    // This is hard-coded to call compiled_entry_point()
    lobster::VMMetaData vmmeta = {
        23,
        span((const lobster::type_elem_t *)&type_table, sizeof(type_table) / sizeof(int)),
        span(stringtable),
        span(file_names),
        span(function_names),
        span(udts),
        span(specidents),
        span(enums),
        span(ser_ids),
        string_view("Sun Jun  8 22:40:03 2025", 24),
        0x883db8532f6e1faa,
        span(subfunctions_to_function),
    };
    return RunCompiledCodeMain(argc, argv, &vmmeta, vtables, nullptr, "");
}
