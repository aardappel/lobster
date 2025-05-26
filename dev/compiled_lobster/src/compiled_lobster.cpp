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

static void fun_0(VMRef, StackPtr);
static void fun_3(VMRef, StackPtr);
static void fun_4(VMRef, StackPtr);
static void fun_5(VMRef, StackPtr);
static void fun_6(VMRef, StackPtr);
static void fun_613(VMRef, StackPtr);
static void fun_612(VMRef, StackPtr);
static void fun_611(VMRef, StackPtr);
static void fun_610(VMRef, StackPtr);
static void fun_609(VMRef, StackPtr);
static void fun_608(VMRef, StackPtr);
static void fun_601(VMRef, StackPtr);
static void fun_600(VMRef, StackPtr);
static void fun_599(VMRef, StackPtr);
static void fun_598(VMRef, StackPtr);
static void fun_590(VMRef, StackPtr);
static void fun_589(VMRef, StackPtr);
static void fun_584(VMRef, StackPtr);
static void fun_583(VMRef, StackPtr);
static void fun_582(VMRef, StackPtr);
static void fun_581(VMRef, StackPtr);
static void fun_579(VMRef, StackPtr);
static void fun_578(VMRef, StackPtr);
static void fun_576(VMRef, StackPtr);
static void fun_573(VMRef, StackPtr);
static void fun_570(VMRef, StackPtr);
static void fun_563(VMRef, StackPtr);
static void fun_562(VMRef, StackPtr);
static void fun_561(VMRef, StackPtr);
static void fun_560(VMRef, StackPtr);
static void fun_559(VMRef, StackPtr);
static void fun_555(VMRef, StackPtr);
static void fun_540(VMRef, StackPtr);
static void fun_539(VMRef, StackPtr);
static void fun_529(VMRef, StackPtr);
static void fun_502(VMRef, StackPtr);
static void fun_499(VMRef, StackPtr);
static void fun_489(VMRef, StackPtr);
static void fun_482(VMRef, StackPtr);
static void fun_479(VMRef, StackPtr);
static void fun_468(VMRef, StackPtr);
static void fun_7(VMRef, StackPtr);
static void fun_564(VMRef, StackPtr);
static void fun_10(VMRef, StackPtr);
static void fun_592(VMRef, StackPtr);
static void fun_587(VMRef, StackPtr);
static void fun_13(VMRef, StackPtr);
static void fun_597(VMRef, StackPtr);
static void fun_596(VMRef, StackPtr);
static void fun_565(VMRef, StackPtr);
static void fun_15(VMRef, StackPtr);
static void fun_569(VMRef, StackPtr);
static void fun_568(VMRef, StackPtr);
static void fun_567(VMRef, StackPtr);
static void fun_556(VMRef, StackPtr);
static void fun_478(VMRef, StackPtr);
static void fun_17(VMRef, StackPtr);
static void fun_595(VMRef, StackPtr);
static void fun_19(VMRef, StackPtr);
static void fun_594(VMRef, StackPtr);
static void fun_23(VMRef, StackPtr);
static void fun_26(VMRef, StackPtr);
static void fun_469(VMRef, StackPtr);
static void fun_29(VMRef, StackPtr);
static void fun_30(VMRef, StackPtr);
static void fun_591(VMRef, StackPtr);
static void fun_585(VMRef, StackPtr);
static void fun_48(VMRef, StackPtr);
static void fun_593(VMRef, StackPtr);
static void fun_588(VMRef, StackPtr);
static void fun_49(VMRef, StackPtr);
static void fun_50(VMRef, StackPtr);
static void fun_51(VMRef, StackPtr);
static void fun_52(VMRef, StackPtr);
static void fun_473(VMRef, StackPtr);
static void fun_70(VMRef, StackPtr);
static void fun_71(VMRef, StackPtr);
static void fun_72(VMRef, StackPtr);
static void fun_79(VMRef, StackPtr);
static void fun_89(VMRef, StackPtr);
static void fun_95(VMRef, StackPtr);
static void fun_98(VMRef, StackPtr);
static void fun_571(VMRef, StackPtr);
static void fun_100(VMRef, StackPtr);
static void fun_574(VMRef, StackPtr);
static void fun_101(VMRef, StackPtr);
static void fun_577(VMRef, StackPtr);
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
static void fun_474(VMRef, StackPtr);
static void fun_128(VMRef, StackPtr);
static void fun_475(VMRef, StackPtr);
static void fun_129(VMRef, StackPtr);
static void fun_476(VMRef, StackPtr);
static void fun_130(VMRef, StackPtr);
static void fun_133(VMRef, StackPtr);
static void fun_534(VMRef, StackPtr);
static void fun_532(VMRef, StackPtr);
static void fun_504(VMRef, StackPtr);
static void fun_494(VMRef, StackPtr);
static void fun_492(VMRef, StackPtr);
static void fun_487(VMRef, StackPtr);
static void fun_485(VMRef, StackPtr);
static void fun_480(VMRef, StackPtr);
static void fun_137(VMRef, StackPtr);
static void fun_531(VMRef, StackPtr);
static void fun_491(VMRef, StackPtr);
static void fun_484(VMRef, StackPtr);
static void fun_138(VMRef, StackPtr);
static void fun_143(VMRef, StackPtr);
static void fun_533(VMRef, StackPtr);
static void fun_493(VMRef, StackPtr);
static void fun_486(VMRef, StackPtr);
static void fun_144(VMRef, StackPtr);
static void fun_528(VMRef, StackPtr);
static void fun_501(VMRef, StackPtr);
static void fun_488(VMRef, StackPtr);
static void fun_481(VMRef, StackPtr);
static void fun_148(VMRef, StackPtr);
static void fun_150(VMRef, StackPtr);
static void fun_153(VMRef, StackPtr);
static void fun_156(VMRef, StackPtr);
static void fun_157(VMRef, StackPtr);
static void fun_159(VMRef, StackPtr);
static void fun_160(VMRef, StackPtr);
static void fun_161(VMRef, StackPtr);
static void fun_162(VMRef, StackPtr);
static void fun_163(VMRef, StackPtr);
static void fun_164(VMRef, StackPtr);
static void fun_477(VMRef, StackPtr);
static void fun_165(VMRef, StackPtr);
static void fun_166(VMRef, StackPtr);
static void fun_167(VMRef, StackPtr);
static void fun_470(VMRef, StackPtr);
static void fun_168(VMRef, StackPtr);
static void fun_169(VMRef, StackPtr);
static void fun_170(VMRef, StackPtr);
static void fun_171(VMRef, StackPtr);
static void fun_172(VMRef, StackPtr);
static void fun_471(VMRef, StackPtr);
static void fun_173(VMRef, StackPtr);
static void fun_472(VMRef, StackPtr);
static void fun_174(VMRef, StackPtr);
static void fun_175(VMRef, StackPtr);
static void fun_176(VMRef, StackPtr);
static void fun_177(VMRef, StackPtr);
static void fun_178(VMRef, StackPtr);
static void fun_179(VMRef, StackPtr);
static void fun_180(VMRef, StackPtr);
static void fun_181(VMRef, StackPtr);
static void fun_182(VMRef, StackPtr);
static void fun_183(VMRef, StackPtr);
static void fun_184(VMRef, StackPtr);
static void fun_185(VMRef, StackPtr);
static void fun_187(VMRef, StackPtr);
static void fun_190(VMRef, StackPtr);
static void fun_191(VMRef, StackPtr);
static void fun_192(VMRef, StackPtr);
static void fun_193(VMRef, StackPtr);
static void fun_194(VMRef, StackPtr);
static void fun_195(VMRef, StackPtr);
static void fun_196(VMRef, StackPtr);
static void fun_197(VMRef, StackPtr);
static void fun_198(VMRef, StackPtr);
static void fun_199(VMRef, StackPtr);
static void fun_200(VMRef, StackPtr);
static void fun_508(VMRef, StackPtr);
static void fun_506(VMRef, StackPtr);
static void fun_202(VMRef, StackPtr);
static void fun_203(VMRef, StackPtr);
static void fun_205(VMRef, StackPtr);
static void fun_206(VMRef, StackPtr);
static void fun_511(VMRef, StackPtr);
static void fun_207(VMRef, StackPtr);
static void fun_208(VMRef, StackPtr);
static void fun_513(VMRef, StackPtr);
static void fun_209(VMRef, StackPtr);
static void fun_210(VMRef, StackPtr);
static void fun_211(VMRef, StackPtr);
static void fun_212(VMRef, StackPtr);
static void fun_215(VMRef, StackPtr);
static void fun_227(VMRef, StackPtr);
static void fun_216(VMRef, StackPtr);
static void fun_222(VMRef, StackPtr);
static void fun_228(VMRef, StackPtr);
static void fun_516(VMRef, StackPtr);
static void fun_217(VMRef, StackPtr);
static void fun_517(VMRef, StackPtr);
static void fun_223(VMRef, StackPtr);
static void fun_218(VMRef, StackPtr);
static void fun_518(VMRef, StackPtr);
static void fun_224(VMRef, StackPtr);
static void fun_219(VMRef, StackPtr);
static void fun_225(VMRef, StackPtr);
static void fun_220(VMRef, StackPtr);
static void fun_226(VMRef, StackPtr);
static void fun_221(VMRef, StackPtr);
static void fun_520(VMRef, StackPtr);
static void fun_229(VMRef, StackPtr);
static void fun_231(VMRef, StackPtr);
static void fun_232(VMRef, StackPtr);
static void fun_233(VMRef, StackPtr);
static void fun_234(VMRef, StackPtr);
static void fun_235(VMRef, StackPtr);
static void fun_523(VMRef, StackPtr);
static void fun_236(VMRef, StackPtr);
static void fun_524(VMRef, StackPtr);
static void fun_237(VMRef, StackPtr);
static void fun_238(VMRef, StackPtr);
static void fun_525(VMRef, StackPtr);
static void fun_239(VMRef, StackPtr);
static void fun_241(VMRef, StackPtr);
static void fun_242(VMRef, StackPtr);
static void fun_243(VMRef, StackPtr);
static void fun_247(VMRef, StackPtr);
static void fun_248(VMRef, StackPtr);
static void fun_249(VMRef, StackPtr);
static void fun_250(VMRef, StackPtr);
static void fun_251(VMRef, StackPtr);
static void fun_254(VMRef, StackPtr);
static void fun_255(VMRef, StackPtr);
static void fun_256(VMRef, StackPtr);
static void fun_257(VMRef, StackPtr);
static void fun_258(VMRef, StackPtr);
static void fun_557(VMRef, StackPtr);
static void fun_546(VMRef, StackPtr);
static void fun_260(VMRef, StackPtr);
static void fun_558(VMRef, StackPtr);
static void fun_553(VMRef, StackPtr);
static void fun_261(VMRef, StackPtr);
static void fun_542(VMRef, StackPtr);
static void fun_262(VMRef, StackPtr);
static void fun_547(VMRef, StackPtr);
static void fun_263(VMRef, StackPtr);
static void fun_549(VMRef, StackPtr);
static void fun_264(VMRef, StackPtr);
static void fun_552(VMRef, StackPtr);
static void fun_265(VMRef, StackPtr);
static void fun_554(VMRef, StackPtr);
static void fun_266(VMRef, StackPtr);
static void fun_267(VMRef, StackPtr);
static void fun_541(VMRef, StackPtr);
static void fun_268(VMRef, StackPtr);
static void fun_551(VMRef, StackPtr);
static void fun_269(VMRef, StackPtr);
static void fun_270(VMRef, StackPtr);
static void fun_271(VMRef, StackPtr);
static void fun_274(VMRef, StackPtr);
static void fun_275(VMRef, StackPtr);
static void fun_276(VMRef, StackPtr);
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
static void fun_289(VMRef, StackPtr);
static void fun_291(VMRef, StackPtr);
static void fun_293(VMRef, StackPtr);
static void fun_295(VMRef, StackPtr);
static void fun_297(VMRef, StackPtr);
static void fun_288(VMRef, StackPtr);
static void fun_290(VMRef, StackPtr);
static void fun_292(VMRef, StackPtr);
static void fun_294(VMRef, StackPtr);
static void fun_296(VMRef, StackPtr);
static void fun_298(VMRef, StackPtr);
static void fun_299(VMRef, StackPtr);
static void fun_300(VMRef, StackPtr);
static void fun_301(VMRef, StackPtr);
static void fun_302(VMRef, StackPtr);
static void fun_303(VMRef, StackPtr);
static void fun_305(VMRef, StackPtr);
static void fun_306(VMRef, StackPtr);
static void fun_307(VMRef, StackPtr);
static void fun_308(VMRef, StackPtr);
static void fun_309(VMRef, StackPtr);
static void fun_310(VMRef, StackPtr);
static void fun_311(VMRef, StackPtr);
static void fun_312(VMRef, StackPtr);
static void fun_315(VMRef, StackPtr);
static void fun_313(VMRef, StackPtr);
static void fun_317(VMRef, StackPtr);
static void fun_314(VMRef, StackPtr);
static void fun_320(VMRef, StackPtr);
static void fun_316(VMRef, StackPtr);
static void fun_318(VMRef, StackPtr);
static void fun_319(VMRef, StackPtr);
static void fun_321(VMRef, StackPtr);
static void fun_322(VMRef, StackPtr);
static void fun_323(VMRef, StackPtr);
static void fun_324(VMRef, StackPtr);
static void fun_325(VMRef, StackPtr);
static void fun_326(VMRef, StackPtr);
static void fun_327(VMRef, StackPtr);
static void fun_566(VMRef, StackPtr);
static void fun_328(VMRef, StackPtr);
static void fun_329(VMRef, StackPtr);
static void fun_330(VMRef, StackPtr);
static void fun_331(VMRef, StackPtr);
static void fun_332(VMRef, StackPtr);
static void fun_333(VMRef, StackPtr);
static void fun_334(VMRef, StackPtr);
static void fun_336(VMRef, StackPtr);
static void fun_337(VMRef, StackPtr);
static void fun_338(VMRef, StackPtr);
static void fun_580(VMRef, StackPtr);
static void fun_339(VMRef, StackPtr);
static void fun_340(VMRef, StackPtr);
static void fun_341(VMRef, StackPtr);
static void fun_342(VMRef, StackPtr);
static void fun_343(VMRef, StackPtr);
static void fun_344(VMRef, StackPtr);
static void fun_345(VMRef, StackPtr);
static void fun_346(VMRef, StackPtr);
static void fun_347(VMRef, StackPtr);
static void fun_348(VMRef, StackPtr);
static void fun_352(VMRef, StackPtr);
static void fun_353(VMRef, StackPtr);
static void fun_355(VMRef, StackPtr);
static void fun_356(VMRef, StackPtr);
static void fun_358(VMRef, StackPtr);
static void fun_359(VMRef, StackPtr);
static void fun_360(VMRef, StackPtr);
static void fun_361(VMRef, StackPtr);
static void fun_362(VMRef, StackPtr);
static void fun_363(VMRef, StackPtr);
static void fun_367(VMRef, StackPtr);
static void fun_368(VMRef, StackPtr);
static void fun_369(VMRef, StackPtr);
static void fun_370(VMRef, StackPtr);
static void fun_372(VMRef, StackPtr);
static void fun_373(VMRef, StackPtr);
static void fun_374(VMRef, StackPtr);
static void fun_375(VMRef, StackPtr);
static void fun_376(VMRef, StackPtr);
static void fun_377(VMRef, StackPtr);
static void fun_378(VMRef, StackPtr);
static void fun_379(VMRef, StackPtr);
static void fun_380(VMRef, StackPtr);
static void fun_381(VMRef, StackPtr);
static void fun_382(VMRef, StackPtr);
static void fun_393(VMRef, StackPtr);
static void fun_425(VMRef, StackPtr);
static void fun_443(VMRef, StackPtr);
static void fun_444(VMRef, StackPtr);
static void fun_383(VMRef, StackPtr);
static void fun_606(VMRef, StackPtr);
static void fun_418(VMRef, StackPtr);
static void fun_391(VMRef, StackPtr);
static void fun_423(VMRef, StackPtr);
static void fun_392(VMRef, StackPtr);
static void fun_424(VMRef, StackPtr);
static void fun_394(VMRef, StackPtr);
static void fun_426(VMRef, StackPtr);
static void fun_401(VMRef, StackPtr);
static void fun_605(VMRef, StackPtr);
static void fun_411(VMRef, StackPtr);
static void fun_433(VMRef, StackPtr);
static void fun_412(VMRef, StackPtr);
static void fun_419(VMRef, StackPtr);
static void fun_420(VMRef, StackPtr);
static void fun_421(VMRef, StackPtr);
static void fun_422(VMRef, StackPtr);
static void fun_442(VMRef, StackPtr);
static void fun_445(VMRef, StackPtr);
static void fun_446(VMRef, StackPtr);
static void fun_447(VMRef, StackPtr);
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

static void fun_10000001(VMRef vm, StackPtr psp) {
    Value regs[1];
    (void)regs;
    U_ABORT(vm, regs + 0);
}

// __top_level_expression
static void fun_0(VMRef vm, StackPtr psp) {
    Value regs[24];
    Value locals[204];
    BackupVar(vm, 0);
    BackupVar(vm, 74);
    BackupVar(vm, 75);
    BackupVar(vm, 76);
    BackupVar(vm, 77);
    locals[0] = lobster::NilVal();
    locals[1] = lobster::NilVal();
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    locals[5] = lobster::NilVal();
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
    locals[6] = lobster::NilVal();
    locals[7] = lobster::NilVal();
    locals[8] = lobster::NilVal();
    locals[9] = lobster::NilVal();
    locals[10] = lobster::NilVal();
    locals[11] = lobster::NilVal();
    locals[12] = lobster::NilVal();
    locals[13] = lobster::NilVal();
    locals[14] = lobster::NilVal();
    locals[15] = lobster::NilVal();
    locals[16] = lobster::NilVal();
    locals[17] = lobster::NilVal();
    locals[18] = lobster::NilVal();
    locals[19] = lobster::NilVal();
    locals[20] = lobster::NilVal();
    locals[21] = lobster::NilVal();
    locals[22] = lobster::NilVal();
    locals[23] = lobster::NilVal();
    locals[24] = lobster::NilVal();
    locals[25] = lobster::NilVal();
    locals[26] = lobster::NilVal();
    locals[27] = lobster::NilVal();
    locals[28] = lobster::NilVal();
    locals[29] = lobster::NilVal();
    locals[30] = lobster::NilVal();
    locals[31] = lobster::NilVal();
    locals[32] = lobster::NilVal();
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
    locals[33] = lobster::NilVal();
    locals[34] = lobster::NilVal();
    locals[35] = lobster::NilVal();
    locals[36] = lobster::NilVal();
    locals[37] = lobster::NilVal();
    locals[38] = lobster::NilVal();
    locals[39] = lobster::NilVal();
    locals[40] = lobster::NilVal();
    locals[41] = lobster::NilVal();
    locals[42] = lobster::NilVal();
    locals[43] = lobster::NilVal();
    locals[44] = lobster::NilVal();
    locals[45] = lobster::NilVal();
    locals[46] = lobster::NilVal();
    locals[47] = lobster::NilVal();
    locals[48] = lobster::NilVal();
    locals[49] = lobster::NilVal();
    locals[50] = lobster::NilVal();
    locals[51] = lobster::NilVal();
    locals[52] = lobster::NilVal();
    locals[53] = lobster::NilVal();
    locals[54] = lobster::NilVal();
    locals[55] = lobster::NilVal();
    locals[56] = lobster::NilVal();
    locals[57] = lobster::NilVal();
    locals[58] = lobster::NilVal();
    locals[59] = lobster::NilVal();
    locals[60] = lobster::NilVal();
    locals[61] = lobster::NilVal();
    locals[62] = lobster::NilVal();
    locals[63] = lobster::NilVal();
    locals[64] = lobster::NilVal();
    locals[65] = lobster::NilVal();
    BackupVar(vm, 170);
    BackupVar(vm, 171);
    locals[66] = lobster::NilVal();
    locals[67] = lobster::NilVal();
    locals[68] = lobster::NilVal();
    locals[69] = lobster::NilVal();
    locals[70] = lobster::NilVal();
    locals[71] = lobster::NilVal();
    locals[72] = lobster::NilVal();
    locals[73] = lobster::NilVal();
    locals[74] = lobster::NilVal();
    locals[75] = lobster::NilVal();
    locals[76] = lobster::NilVal();
    locals[77] = lobster::NilVal();
    locals[78] = lobster::NilVal();
    locals[79] = lobster::NilVal();
    locals[80] = lobster::NilVal();
    locals[81] = lobster::NilVal();
    locals[82] = lobster::NilVal();
    locals[83] = lobster::NilVal();
    locals[84] = lobster::NilVal();
    locals[85] = lobster::NilVal();
    locals[86] = lobster::NilVal();
    locals[87] = lobster::NilVal();
    locals[88] = lobster::NilVal();
    locals[89] = lobster::NilVal();
    locals[90] = lobster::NilVal();
    locals[91] = lobster::NilVal();
    locals[92] = lobster::NilVal();
    locals[93] = lobster::NilVal();
    locals[94] = lobster::NilVal();
    locals[95] = lobster::NilVal();
    locals[96] = lobster::NilVal();
    locals[97] = lobster::NilVal();
    locals[98] = lobster::NilVal();
    locals[99] = lobster::NilVal();
    locals[100] = lobster::NilVal();
    locals[101] = lobster::NilVal();
    locals[102] = lobster::NilVal();
    locals[103] = lobster::NilVal();
    locals[104] = lobster::NilVal();
    locals[105] = lobster::NilVal();
    locals[106] = lobster::NilVal();
    locals[107] = lobster::NilVal();
    locals[108] = lobster::NilVal();
    locals[109] = lobster::NilVal();
    locals[110] = lobster::NilVal();
    locals[111] = lobster::NilVal();
    locals[112] = lobster::NilVal();
    locals[113] = lobster::NilVal();
    locals[114] = lobster::NilVal();
    locals[115] = lobster::NilVal();
    locals[116] = lobster::NilVal();
    locals[117] = lobster::NilVal();
    locals[118] = lobster::NilVal();
    locals[119] = lobster::NilVal();
    locals[120] = lobster::NilVal();
    locals[121] = lobster::NilVal();
    locals[122] = lobster::NilVal();
    locals[123] = lobster::NilVal();
    locals[124] = lobster::NilVal();
    locals[125] = lobster::NilVal();
    locals[126] = lobster::NilVal();
    locals[127] = lobster::NilVal();
    locals[128] = lobster::NilVal();
    locals[129] = lobster::NilVal();
    locals[130] = lobster::NilVal();
    locals[131] = lobster::NilVal();
    locals[132] = lobster::NilVal();
    locals[133] = lobster::NilVal();
    locals[134] = lobster::NilVal();
    locals[135] = lobster::NilVal();
    locals[136] = lobster::NilVal();
    locals[137] = lobster::NilVal();
    locals[138] = lobster::NilVal();
    locals[139] = lobster::NilVal();
    locals[140] = lobster::NilVal();
    locals[141] = lobster::NilVal();
    locals[142] = lobster::NilVal();
    locals[143] = lobster::NilVal();
    locals[144] = lobster::NilVal();
    locals[145] = lobster::NilVal();
    locals[146] = lobster::NilVal();
    locals[147] = lobster::NilVal();
    locals[148] = lobster::NilVal();
    locals[149] = lobster::NilVal();
    locals[150] = lobster::NilVal();
    locals[151] = lobster::NilVal();
    locals[152] = lobster::NilVal();
    locals[153] = lobster::NilVal();
    locals[154] = lobster::NilVal();
    locals[155] = lobster::NilVal();
    locals[156] = lobster::NilVal();
    locals[157] = lobster::NilVal();
    locals[158] = lobster::NilVal();
    locals[159] = lobster::NilVal();
    locals[160] = lobster::NilVal();
    locals[161] = lobster::NilVal();
    locals[162] = lobster::NilVal();
    locals[163] = lobster::NilVal();
    locals[164] = lobster::NilVal();
    locals[165] = lobster::NilVal();
    locals[166] = lobster::NilVal();
    locals[167] = lobster::NilVal();
    locals[168] = lobster::NilVal();
    locals[169] = lobster::NilVal();
    locals[170] = lobster::NilVal();
    locals[171] = lobster::NilVal();
    locals[172] = lobster::NilVal();
    locals[173] = lobster::NilVal();
    locals[174] = lobster::NilVal();
    locals[175] = lobster::NilVal();
    locals[176] = lobster::NilVal();
    locals[177] = lobster::NilVal();
    locals[178] = lobster::NilVal();
    locals[179] = lobster::NilVal();
    locals[180] = lobster::NilVal();
    locals[181] = lobster::NilVal();
    locals[182] = lobster::NilVal();
    locals[183] = lobster::NilVal();
    locals[184] = lobster::NilVal();
    locals[185] = lobster::NilVal();
    locals[186] = lobster::NilVal();
    locals[187] = lobster::NilVal();
    locals[188] = lobster::NilVal();
    locals[189] = lobster::NilVal();
    locals[190] = lobster::NilVal();
    locals[191] = lobster::NilVal();
    locals[192] = lobster::NilVal();
    locals[193] = lobster::NilVal();
    locals[194] = lobster::NilVal();
    locals[195] = lobster::NilVal();
    locals[196] = lobster::NilVal();
    locals[197] = lobster::NilVal();
    locals[198] = lobster::NilVal();
    locals[199] = lobster::NilVal();
    locals[200] = lobster::NilVal();
    locals[201] = lobster::NilVal();
    locals[202] = lobster::NilVal();
    locals[203] = lobster::NilVal();
    BackupVar(vm, 1150);
    BackupVar(vm, 1151);
    BackupVar(vm, 1152);
    BackupVar(vm, 1153);
    PushFunId(vm, funinfo_table + 0, locals);
    U_STATEMENT(vm, regs + 0, 8, 1);
    U_STATEMENT(vm, regs + 0, 12, 1);
    U_STATEMENT(vm, regs + 0, 15, 1);
    U_STATEMENT(vm, regs + 0, 20, 1);
    U_STATEMENT(vm, regs + 0, 21, 1);
    U_STATEMENT(vm, regs + 0, 23, 1);
    U_STATEMENT(vm, regs + 0, 24, 1);
    U_STATEMENT(vm, regs + 0, 26, 1);
    U_STATEMENT(vm, regs + 0, 27, 1);
    U_STATEMENT(vm, regs + 0, 31, 1);
    U_STATEMENT(vm, regs + 0, 4, 2);
    U_STATEMENT(vm, regs + 0, 5, 2);
    U_STATEMENT(vm, regs + 0, 8, 2);
    U_PUSHFUN(vm, regs + 0, 0, fun_3);
    U_LVAL_VARF(vm, regs + 1, 0); // current_test_runner
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 10, 2);
    U_STATEMENT(vm, regs + 0, 14, 2);
    U_PUSHSTR(vm, regs + 0, 0); // "example"
    U_PUSHFUN(vm, regs + 1, 0, fun_5);
    fun_4(vm, regs + 2); // call: run_test
    U_STATEMENT(vm, regs + 0, 7, 0);
    U_PUSHFUN(vm, regs + 0, 0, fun_6);
    U_LVAL_VARF(vm, regs + 1, 0); // current_test_runner
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 3, 5);
    U_STATEMENT(vm, regs + 0, 9, 5);
    U_STATEMENT(vm, regs + 0, 11, 5);
    U_STATEMENT(vm, regs + 0, 21, 5);
    U_STATEMENT(vm, regs + 0, 29, 5);
    U_STATEMENT(vm, regs + 0, 36, 5);
    U_STATEMENT(vm, regs + 0, 46, 5);
    U_STATEMENT(vm, regs + 0, 56, 5);
    U_STATEMENT(vm, regs + 0, 62, 5);
    U_STATEMENT(vm, regs + 0, 66, 5);
    U_STATEMENT(vm, regs + 0, 71, 5);
    U_STATEMENT(vm, regs + 0, 76, 5);
    U_STATEMENT(vm, regs + 0, 82, 5);
    U_STATEMENT(vm, regs + 0, 85, 5);
    U_STATEMENT(vm, regs + 0, 92, 5);
    U_STATEMENT(vm, regs + 0, 99, 5);
    U_STATEMENT(vm, regs + 0, 106, 5);
    U_STATEMENT(vm, regs + 0, 114, 5);
    U_STATEMENT(vm, regs + 0, 127, 5);
    U_STATEMENT(vm, regs + 0, 139, 5);
    U_STATEMENT(vm, regs + 0, 142, 5);
    U_STATEMENT(vm, regs + 0, 146, 5);
    U_STATEMENT(vm, regs + 0, 148, 5);
    U_STATEMENT(vm, regs + 0, 153, 5);
    U_STATEMENT(vm, regs + 0, 158, 5);
    U_STATEMENT(vm, regs + 0, 164, 5);
    U_STATEMENT(vm, regs + 0, 171, 5);
    U_STATEMENT(vm, regs + 0, 176, 5);
    U_STATEMENT(vm, regs + 0, 180, 5);
    U_STATEMENT(vm, regs + 0, 186, 5);
    U_STATEMENT(vm, regs + 0, 188, 5);
    U_STATEMENT(vm, regs + 0, 190, 5);
    U_STATEMENT(vm, regs + 0, 193, 5);
    U_STATEMENT(vm, regs + 0, 202, 5);
    U_STATEMENT(vm, regs + 0, 222, 5);
    U_STATEMENT(vm, regs + 0, 230, 5);
    U_STATEMENT(vm, regs + 0, 237, 5);
    U_STATEMENT(vm, regs + 0, 245, 5);
    U_STATEMENT(vm, regs + 0, 249, 5);
    U_STATEMENT(vm, regs + 0, 253, 5);
    U_STATEMENT(vm, regs + 0, 255, 5);
    U_STATEMENT(vm, regs + 0, 258, 5);
    U_STATEMENT(vm, regs + 0, 262, 5);
    U_STATEMENT(vm, regs + 0, 264, 5);
    U_STATEMENT(vm, regs + 0, 269, 5);
    U_STATEMENT(vm, regs + 0, 276, 5);
    U_STATEMENT(vm, regs + 0, 281, 5);
    U_STATEMENT(vm, regs + 0, 284, 5);
    U_STATEMENT(vm, regs + 0, 290, 5);
    U_STATEMENT(vm, regs + 0, 7, 4);
    U_PUSHFLT(vm, regs + 0, 0);
    U_PUSHFLT(vm, regs + 1, 0);
    U_LVAL_VARF(vm, regs + 2, 74); // float2_0+0
    U_LV_WRITEV(vm, regs + 2, 2);
    U_STATEMENT(vm, regs + 0, 8, 4);
    U_PUSHFLT(vm, regs + 0, 1065353216);
    U_PUSHFLT(vm, regs + 1, 1065353216);
    U_LVAL_VARF(vm, regs + 2, 76); // float2_1+0
    U_LV_WRITEV(vm, regs + 2, 2);
    U_STATEMENT(vm, regs + 0, 9, 4);
    U_PUSHFLT(vm, regs + 0, 1056964608);
    U_PUSHFLT(vm, regs + 1, 1056964608);
    SetLVal(vm, &locals[0]); // float2_h+0
    U_LV_WRITEV(vm, regs + 2, 2);
    U_STATEMENT(vm, regs + 0, 10, 4);
    U_PUSHFLT(vm, regs + 0, 1065353216);
    U_PUSHFLT(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // float2_x+0
    U_LV_WRITEV(vm, regs + 2, 2);
    U_STATEMENT(vm, regs + 0, 11, 4);
    U_PUSHFLT(vm, regs + 0, 0);
    U_PUSHFLT(vm, regs + 1, 1065353216);
    SetLVal(vm, &locals[4]); // float2_y+0
    U_LV_WRITEV(vm, regs + 2, 2);
    U_STATEMENT(vm, regs + 0, 13, 4);
    U_PUSHFLT(vm, regs + 0, 0);
    U_PUSHFLT(vm, regs + 1, 0);
    U_PUSHFLT(vm, regs + 2, 0);
    U_LVAL_VARF(vm, regs + 3, 84); // float3_0+0
    U_LV_WRITEV(vm, regs + 3, 3);
    U_STATEMENT(vm, regs + 0, 14, 4);
    U_PUSHFLT(vm, regs + 0, 1065353216);
    U_PUSHFLT(vm, regs + 1, 1065353216);
    U_PUSHFLT(vm, regs + 2, 1065353216);
    U_LVAL_VARF(vm, regs + 3, 87); // float3_1+0
    U_LV_WRITEV(vm, regs + 3, 3);
    U_STATEMENT(vm, regs + 0, 15, 4);
    U_PUSHFLT(vm, regs + 0, 1065353216);
    U_PUSHFLT(vm, regs + 1, 0);
    U_PUSHFLT(vm, regs + 2, 0);
    U_LVAL_VARF(vm, regs + 3, 90); // float3_x+0
    U_LV_WRITEV(vm, regs + 3, 3);
    U_STATEMENT(vm, regs + 0, 16, 4);
    U_PUSHFLT(vm, regs + 0, 0);
    U_PUSHFLT(vm, regs + 1, 1065353216);
    U_PUSHFLT(vm, regs + 2, 0);
    U_LVAL_VARF(vm, regs + 3, 93); // float3_y+0
    U_LV_WRITEV(vm, regs + 3, 3);
    U_STATEMENT(vm, regs + 0, 17, 4);
    U_PUSHFLT(vm, regs + 0, 0);
    U_PUSHFLT(vm, regs + 1, 0);
    U_PUSHFLT(vm, regs + 2, 1065353216);
    SetLVal(vm, &locals[6]); // float3_z+0
    U_LV_WRITEV(vm, regs + 3, 3);
    U_STATEMENT(vm, regs + 0, 19, 4);
    U_PUSHFLT(vm, regs + 0, 0);
    U_PUSHFLT(vm, regs + 1, 0);
    U_PUSHFLT(vm, regs + 2, 0);
    U_PUSHFLT(vm, regs + 3, 0);
    SetLVal(vm, &locals[9]); // float4_0+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 20, 4);
    U_PUSHFLT(vm, regs + 0, 1065353216);
    U_PUSHFLT(vm, regs + 1, 1065353216);
    U_PUSHFLT(vm, regs + 2, 1065353216);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    SetLVal(vm, &locals[13]); // float4_1+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 21, 4);
    U_PUSHFLT(vm, regs + 0, 1065353216);
    U_PUSHFLT(vm, regs + 1, 0);
    U_PUSHFLT(vm, regs + 2, 0);
    U_PUSHFLT(vm, regs + 3, 0);
    SetLVal(vm, &locals[17]); // float4_x+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 22, 4);
    U_PUSHFLT(vm, regs + 0, 0);
    U_PUSHFLT(vm, regs + 1, 1065353216);
    U_PUSHFLT(vm, regs + 2, 0);
    U_PUSHFLT(vm, regs + 3, 0);
    SetLVal(vm, &locals[21]); // float4_y+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 23, 4);
    U_PUSHFLT(vm, regs + 0, 0);
    U_PUSHFLT(vm, regs + 1, 0);
    U_PUSHFLT(vm, regs + 2, 1065353216);
    U_PUSHFLT(vm, regs + 3, 0);
    SetLVal(vm, &locals[25]); // float4_z+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 24, 4);
    U_PUSHFLT(vm, regs + 0, 0);
    U_PUSHFLT(vm, regs + 1, 0);
    U_PUSHFLT(vm, regs + 2, 0);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    SetLVal(vm, &locals[29]); // float4_w+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 28, 4);
    U_PUSHINT(vm, regs + 0, 0);
    U_PUSHINT(vm, regs + 1, 0);
    U_LVAL_VARF(vm, regs + 2, 123); // int2_0+0
    U_LV_WRITEV(vm, regs + 2, 2);
    U_STATEMENT(vm, regs + 0, 29, 4);
    U_PUSHINT(vm, regs + 0, 1);
    U_PUSHINT(vm, regs + 1, 1);
    U_LVAL_VARF(vm, regs + 2, 125); // int2_1+0
    U_LV_WRITEV(vm, regs + 2, 2);
    U_STATEMENT(vm, regs + 0, 30, 4);
    U_PUSHINT(vm, regs + 0, 1);
    U_PUSHINT(vm, regs + 1, 0);
    U_LVAL_VARF(vm, regs + 2, 127); // int2_x+0
    U_LV_WRITEV(vm, regs + 2, 2);
    U_STATEMENT(vm, regs + 0, 31, 4);
    U_PUSHINT(vm, regs + 0, 0);
    U_PUSHINT(vm, regs + 1, 1);
    U_LVAL_VARF(vm, regs + 2, 129); // int2_y+0
    U_LV_WRITEV(vm, regs + 2, 2);
    U_STATEMENT(vm, regs + 0, 33, 4);
    U_PUSHINT(vm, regs + 0, 0);
    U_PUSHINT(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 2, 0);
    U_LVAL_VARF(vm, regs + 3, 131); // int3_0+0
    U_LV_WRITEV(vm, regs + 3, 3);
    U_STATEMENT(vm, regs + 0, 34, 4);
    U_PUSHINT(vm, regs + 0, 1);
    U_PUSHINT(vm, regs + 1, 1);
    U_PUSHINT(vm, regs + 2, 1);
    U_LVAL_VARF(vm, regs + 3, 134); // int3_1+0
    U_LV_WRITEV(vm, regs + 3, 3);
    U_STATEMENT(vm, regs + 0, 35, 4);
    U_PUSHINT(vm, regs + 0, 1);
    U_PUSHINT(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 2, 0);
    SetLVal(vm, &locals[33]); // int3_x+0
    U_LV_WRITEV(vm, regs + 3, 3);
    U_STATEMENT(vm, regs + 0, 36, 4);
    U_PUSHINT(vm, regs + 0, 0);
    U_PUSHINT(vm, regs + 1, 1);
    U_PUSHINT(vm, regs + 2, 0);
    SetLVal(vm, &locals[36]); // int3_y+0
    U_LV_WRITEV(vm, regs + 3, 3);
    U_STATEMENT(vm, regs + 0, 37, 4);
    U_PUSHINT(vm, regs + 0, 0);
    U_PUSHINT(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 2, 1);
    SetLVal(vm, &locals[39]); // int3_z+0
    U_LV_WRITEV(vm, regs + 3, 3);
    U_STATEMENT(vm, regs + 0, 39, 4);
    U_PUSHINT(vm, regs + 0, 0);
    U_PUSHINT(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 3, 0);
    SetLVal(vm, &locals[42]); // int4_0+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 40, 4);
    U_PUSHINT(vm, regs + 0, 1);
    U_PUSHINT(vm, regs + 1, 1);
    U_PUSHINT(vm, regs + 2, 1);
    U_PUSHINT(vm, regs + 3, 1);
    SetLVal(vm, &locals[46]); // int4_1+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 41, 4);
    U_PUSHINT(vm, regs + 0, 1);
    U_PUSHINT(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 3, 0);
    SetLVal(vm, &locals[50]); // int4_x+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 42, 4);
    U_PUSHINT(vm, regs + 0, 0);
    U_PUSHINT(vm, regs + 1, 1);
    U_PUSHINT(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 3, 0);
    SetLVal(vm, &locals[54]); // int4_y+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 43, 4);
    U_PUSHINT(vm, regs + 0, 0);
    U_PUSHINT(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 2, 1);
    U_PUSHINT(vm, regs + 3, 0);
    SetLVal(vm, &locals[58]); // int4_z+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 44, 4);
    U_PUSHINT(vm, regs + 0, 0);
    U_PUSHINT(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 3, 1);
    SetLVal(vm, &locals[62]); // int4_w+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 46, 4);
    U_PUSHINT(vm, regs + 0, 0);
    U_PUSHINT(vm, regs + 1, -1);
    U_PUSHINT(vm, regs + 2, 1);
    U_PUSHINT(vm, regs + 3, 0);
    U_PUSHINT(vm, regs + 4, 0);
    U_PUSHINT(vm, regs + 5, 1);
    U_PUSHINT(vm, regs + 6, -1);
    U_PUSHINT(vm, regs + 7, 0);
    U_NEWVEC(vm, regs + 8, 37, 4);
    U_LVAL_VARF(vm, regs + 1, 170); // cardinal_directions
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 47, 4);
    U_PUSHINT(vm, regs + 0, -1);
    U_PUSHINT(vm, regs + 1, -1);
    U_PUSHINT(vm, regs + 2, 1);
    U_PUSHINT(vm, regs + 3, 1);
    U_PUSHINT(vm, regs + 4, 1);
    U_PUSHINT(vm, regs + 5, -1);
    U_PUSHINT(vm, regs + 6, -1);
    U_PUSHINT(vm, regs + 7, 1);
    U_NEWVEC(vm, regs + 8, 37, 4);
    U_LVAL_VARF(vm, regs + 1, 171); // diagonal_directions
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 48, 4);
    U_PUSHINT(vm, regs + 0, 0);
    U_PUSHINT(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 2, 1);
    U_PUSHINT(vm, regs + 3, 0);
    U_PUSHINT(vm, regs + 4, 1);
    U_PUSHINT(vm, regs + 5, 1);
    U_PUSHINT(vm, regs + 6, 0);
    U_PUSHINT(vm, regs + 7, 1);
    U_NEWVEC(vm, regs + 8, 37, 4);
    SetLVal(vm, &locals[66]); // positive_directions
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 50, 4);
    U_PUSHINT(vm, regs + 0, 1);
    U_PUSHINT(vm, regs + 1, 1);
    U_PUSHINT(vm, regs + 2, 1);
    U_PUSHINT(vm, regs + 3, 1);
    U_PUSHINT(vm, regs + 4, 1);
    U_PUSHINT(vm, regs + 5, -1);
    U_PUSHINT(vm, regs + 6, 1);
    U_PUSHINT(vm, regs + 7, -1);
    U_PUSHINT(vm, regs + 8, 1);
    U_PUSHINT(vm, regs + 9, 1);
    U_PUSHINT(vm, regs + 10, -1);
    U_PUSHINT(vm, regs + 11, -1);
    U_PUSHINT(vm, regs + 12, -1);
    U_PUSHINT(vm, regs + 13, 1);
    U_PUSHINT(vm, regs + 14, 1);
    U_PUSHINT(vm, regs + 15, -1);
    U_PUSHINT(vm, regs + 16, 1);
    U_PUSHINT(vm, regs + 17, -1);
    U_PUSHINT(vm, regs + 18, -1);
    U_PUSHINT(vm, regs + 19, -1);
    U_PUSHINT(vm, regs + 20, 1);
    U_PUSHINT(vm, regs + 21, -1);
    U_PUSHINT(vm, regs + 22, -1);
    U_PUSHINT(vm, regs + 23, -1);
    U_NEWVEC(vm, regs + 24, 240, 8);
    SetLVal(vm, &locals[67]); // octant_directions
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 62, 4);
    U_STATEMENT(vm, regs + 0, 63, 4);
    U_STATEMENT(vm, regs + 0, 64, 4);
    U_STATEMENT(vm, regs + 0, 65, 4);
    U_STATEMENT(vm, regs + 0, 66, 4);
    U_STATEMENT(vm, regs + 0, 68, 4);
    U_STATEMENT(vm, regs + 0, 71, 4);
    U_STATEMENT(vm, regs + 0, 72, 4);
    U_STATEMENT(vm, regs + 0, 73, 4);
    U_STATEMENT(vm, regs + 0, 74, 4);
    U_STATEMENT(vm, regs + 0, 75, 4);
    U_STATEMENT(vm, regs + 0, 76, 4);
    U_STATEMENT(vm, regs + 0, 79, 4);
    U_STATEMENT(vm, regs + 0, 80, 4);
    U_STATEMENT(vm, regs + 0, 81, 4);
    U_STATEMENT(vm, regs + 0, 83, 4);
    U_STATEMENT(vm, regs + 0, 84, 4);
    U_STATEMENT(vm, regs + 0, 85, 4);
    U_STATEMENT(vm, regs + 0, 88, 4);
    U_STATEMENT(vm, regs + 0, 89, 4);
    U_STATEMENT(vm, regs + 0, 90, 4);
    U_STATEMENT(vm, regs + 0, 92, 4);
    U_STATEMENT(vm, regs + 0, 93, 4);
    U_STATEMENT(vm, regs + 0, 94, 4);
    U_STATEMENT(vm, regs + 0, 96, 4);
    U_STATEMENT(vm, regs + 0, 97, 4);
    U_STATEMENT(vm, regs + 0, 99, 4);
    U_STATEMENT(vm, regs + 0, 100, 4);
    U_STATEMENT(vm, regs + 0, 101, 4);
    U_STATEMENT(vm, regs + 0, 102, 4);
    U_STATEMENT(vm, regs + 0, 104, 4);
    U_STATEMENT(vm, regs + 0, 105, 4);
    U_STATEMENT(vm, regs + 0, 106, 4);
    U_STATEMENT(vm, regs + 0, 108, 4);
    U_STATEMENT(vm, regs + 0, 109, 4);
    U_STATEMENT(vm, regs + 0, 111, 4);
    U_STATEMENT(vm, regs + 0, 115, 4);
    U_STATEMENT(vm, regs + 0, 4, 3);
    U_PUSHSTR(vm, regs + 0, 1); // "struct"
    U_PUSHFUN(vm, regs + 1, 0, fun_110);
    fun_4(vm, regs + 2); // call: run_test
    U_STATEMENT(vm, regs + 0, 3, 7);
    U_STATEMENT(vm, regs + 0, 8, 7);
    U_STATEMENT(vm, regs + 0, 16, 7);
    U_STATEMENT(vm, regs + 0, 19, 7);
    U_STATEMENT(vm, regs + 0, 23, 7);
    U_STATEMENT(vm, regs + 0, 28, 7);
    U_STATEMENT(vm, regs + 0, 33, 7);
    U_STATEMENT(vm, regs + 0, 16, 8);
    U_STATEMENT(vm, regs + 0, 24, 8);
    U_STATEMENT(vm, regs + 0, 33, 8);
    U_STATEMENT(vm, regs + 0, 40, 8);
    U_STATEMENT(vm, regs + 0, 50, 8);
    U_STATEMENT(vm, regs + 0, 56, 8);
    U_STATEMENT(vm, regs + 0, 65, 8);
    U_STATEMENT(vm, regs + 0, 72, 8);
    U_STATEMENT(vm, regs + 0, 76, 8);
    U_STATEMENT(vm, regs + 0, 84, 8);
    U_STATEMENT(vm, regs + 0, 92, 8);
    U_STATEMENT(vm, regs + 0, 21, 8);
    U_STATEMENT(vm, regs + 0, 98, 8);
    U_STATEMENT(vm, regs + 0, 11, 6);
    U_STATEMENT(vm, regs + 0, 12, 6);
    U_STATEMENT(vm, regs + 0, 15, 6);
    U_STATEMENT(vm, regs + 0, 13, 6);
    U_STATEMENT(vm, regs + 0, 13, 6);
    U_STATEMENT(vm, regs + 0, 19, 6);
    U_PUSHSTR(vm, regs + 0, 2); // "misc"
    U_PUSHFUN(vm, regs + 1, 0, fun_153);
    fun_4(vm, regs + 2); // call: run_test
    U_STATEMENT(vm, regs + 0, 8, 9);
    U_PUSHSTR(vm, regs + 0, 3); // "types"
    U_PUSHFUN(vm, regs + 1, 0, fun_199);
    fun_4(vm, regs + 2); // call: run_test
    U_STATEMENT(vm, regs + 0, 15, 11);
    U_STATEMENT(vm, regs + 0, 5, 11);
    U_STATEMENT(vm, regs + 0, 22, 11);
    U_STATEMENT(vm, regs + 0, 54, 11);
    U_STATEMENT(vm, regs + 0, 67, 11);
    U_STATEMENT(vm, regs + 0, 89, 11);
    U_STATEMENT(vm, regs + 0, 91, 11);
    U_STATEMENT(vm, regs + 0, 88, 11);
    U_STATEMENT(vm, regs + 0, 88, 11);
    U_STATEMENT(vm, regs + 0, 94, 11);
    U_STATEMENT(vm, regs + 0, 96, 11);
    U_STATEMENT(vm, regs + 0, 4, 10);
    U_PUSHSTR(vm, regs + 0, 4); // "astar"
    U_PUSHFUN(vm, regs + 1, 0, fun_279);
    fun_4(vm, regs + 2); // call: run_test
    U_STATEMENT(vm, regs + 0, 4, 12);
    U_PUSHSTR(vm, regs + 0, 5); // "goap"
    U_PUSHFUN(vm, regs + 1, 0, fun_286);
    fun_4(vm, regs + 2); // call: run_test
    U_STATEMENT(vm, regs + 0, 4, 13);
    U_PUSHSTR(vm, regs + 0, 6); // "knightstour"
    U_PUSHFUN(vm, regs + 1, 0, fun_302);
    fun_4(vm, regs + 2); // call: run_test
    U_STATEMENT(vm, regs + 0, 6, 14);
    U_PUSHSTR(vm, regs + 0, 7); // "parsereval"
    U_PUSHFUN(vm, regs + 1, 0, fun_311);
    fun_4(vm, regs + 2); // call: run_test
    U_STATEMENT(vm, regs + 0, 3, 16);
    U_STATEMENT(vm, regs + 0, 3, 16);
    U_STATEMENT(vm, regs + 0, 13, 16);
    U_STATEMENT(vm, regs + 0, 5, 15);
    U_PUSHSTR(vm, regs + 0, 8); // "flood"
    U_PUSHFUN(vm, regs + 1, 0, fun_334);
    fun_4(vm, regs + 2); // call: run_test
    U_STATEMENT(vm, regs + 0, 7, 17);
    U_PUSHSTR(vm, regs + 0, 9); // "water"
    U_PUSHFUN(vm, regs + 1, 0, fun_336);
    fun_4(vm, regs + 2); // call: run_test
    U_STATEMENT(vm, regs + 0, 5, 18);
    U_PUSHSTR(vm, regs + 0, 10); // "gradient"
    U_PUSHFUN(vm, regs + 1, 0, fun_347);
    fun_4(vm, regs + 2); // call: run_test
    U_STATEMENT(vm, regs + 0, 6, 19);
    U_PUSHSTR(vm, regs + 0, 11); // "springs"
    U_PUSHFUN(vm, regs + 1, 0, fun_352);
    fun_4(vm, regs + 2); // call: run_test
    U_STATEMENT(vm, regs + 0, 3, 21);
    U_STATEMENT(vm, regs + 0, 3, 21);
    U_STATEMENT(vm, regs + 0, 9, 21);
    U_PUSHFLT(vm, regs + 0, 0);
    U_PUSHFLT(vm, regs + 1, 0);
    U_PUSHFLT(vm, regs + 2, 0);
    U_PUSHFLT(vm, regs + 3, 0);
    SetLVal(vm, &locals[68]); // color_clear+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 10, 21);
    U_PUSHFLT(vm, regs + 0, 0);
    U_PUSHFLT(vm, regs + 1, 0);
    U_PUSHFLT(vm, regs + 2, 0);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    SetLVal(vm, &locals[72]); // color_black+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 11, 21);
    U_PUSHFLT(vm, regs + 0, 1065353216);
    U_PUSHFLT(vm, regs + 1, 1065353216);
    U_PUSHFLT(vm, regs + 2, 1065353216);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    SetLVal(vm, &locals[76]); // color_white+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 13, 21);
    U_PUSHFLT(vm, regs + 0, 1065353216);
    U_PUSHFLT(vm, regs + 1, 0);
    U_PUSHFLT(vm, regs + 2, 0);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    SetLVal(vm, &locals[80]); // color_red+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 14, 21);
    U_PUSHFLT(vm, regs + 0, 0);
    U_PUSHFLT(vm, regs + 1, 1065353216);
    U_PUSHFLT(vm, regs + 2, 0);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    SetLVal(vm, &locals[84]); // color_green+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 15, 21);
    U_PUSHFLT(vm, regs + 0, 0);
    U_PUSHFLT(vm, regs + 1, 0);
    U_PUSHFLT(vm, regs + 2, 1065353216);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    SetLVal(vm, &locals[88]); // color_blue+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 17, 21);
    U_PUSHFLT(vm, regs + 0, 1065353216);
    U_PUSHFLT(vm, regs + 1, 1065353216);
    U_PUSHFLT(vm, regs + 2, 0);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    SetLVal(vm, &locals[92]); // color_yellow+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 18, 21);
    U_PUSHFLT(vm, regs + 0, 0);
    U_PUSHFLT(vm, regs + 1, 1065353216);
    U_PUSHFLT(vm, regs + 2, 1065353216);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    SetLVal(vm, &locals[96]); // color_cyan+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 19, 21);
    U_PUSHFLT(vm, regs + 0, 1065353216);
    U_PUSHFLT(vm, regs + 1, 0);
    U_PUSHFLT(vm, regs + 2, 1065353216);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    SetLVal(vm, &locals[100]); // color_pink+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 21, 21);
    U_PUSHFLT(vm, regs + 0, 1056964608);
    U_PUSHFLT(vm, regs + 1, 1056964608);
    U_PUSHFLT(vm, regs + 2, 1056964608);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    SetLVal(vm, &locals[104]); // color_grey+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 22, 21);
    U_PUSHFLT(vm, regs + 0, 1048576000);
    U_PUSHFLT(vm, regs + 1, 1048576000);
    U_PUSHFLT(vm, regs + 2, 1048576000);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    SetLVal(vm, &locals[108]); // color_dark_grey+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 23, 21);
    U_PUSHFLT(vm, regs + 0, 1061158912);
    U_PUSHFLT(vm, regs + 1, 1061158912);
    U_PUSHFLT(vm, regs + 2, 1061158912);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    SetLVal(vm, &locals[112]); // color_light_grey+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 24, 21);
    U_PUSHFLT64(vm, regs + 0, -858993459, 1072483532);
    U_PUSHFLT64(vm, regs + 1, -858993459, 1072483532);
    U_PUSHFLT64(vm, regs + 2, -858993459, 1072483532);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    SetLVal(vm, &locals[116]); // color_lightest_grey+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 25, 21);
    U_PUSHFLT64(vm, regs + 0, -1717986918, 1069128089);
    U_PUSHFLT64(vm, regs + 1, -1717986918, 1069128089);
    U_PUSHFLT64(vm, regs + 2, -1717986918, 1069128089);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    SetLVal(vm, &locals[120]); // color_darkest_grey+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 27, 21);
    U_PUSHFLT(vm, regs + 0, 1065353216);
    U_PUSHFLT(vm, regs + 1, 1056964608);
    U_PUSHFLT(vm, regs + 2, 1056964608);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    SetLVal(vm, &locals[124]); // color_light_red+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 28, 21);
    U_PUSHFLT(vm, regs + 0, 1056964608);
    U_PUSHFLT(vm, regs + 1, 1065353216);
    U_PUSHFLT(vm, regs + 2, 1056964608);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    SetLVal(vm, &locals[128]); // color_light_green+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 29, 21);
    U_PUSHFLT(vm, regs + 0, 1056964608);
    U_PUSHFLT(vm, regs + 1, 1056964608);
    U_PUSHFLT(vm, regs + 2, 1065353216);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    SetLVal(vm, &locals[132]); // color_light_blue+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 30, 21);
    U_PUSHFLT(vm, regs + 0, 1065353216);
    U_PUSHFLT(vm, regs + 1, 1065353216);
    U_PUSHFLT(vm, regs + 2, 1056964608);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    SetLVal(vm, &locals[136]); // color_light_yellow+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 31, 21);
    U_PUSHFLT(vm, regs + 0, 1056964608);
    U_PUSHFLT(vm, regs + 1, 1065353216);
    U_PUSHFLT(vm, regs + 2, 1065353216);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    SetLVal(vm, &locals[140]); // color_light_cyan+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 32, 21);
    U_PUSHFLT(vm, regs + 0, 1065353216);
    U_PUSHFLT(vm, regs + 1, 1056964608);
    U_PUSHFLT(vm, regs + 2, 1065353216);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    SetLVal(vm, &locals[144]); // color_light_pink+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 34, 21);
    U_PUSHFLT(vm, regs + 0, 1056964608);
    U_PUSHFLT(vm, regs + 1, 0);
    U_PUSHFLT(vm, regs + 2, 0);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    SetLVal(vm, &locals[148]); // color_dark_red+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 35, 21);
    U_PUSHFLT(vm, regs + 0, 0);
    U_PUSHFLT(vm, regs + 1, 1056964608);
    U_PUSHFLT(vm, regs + 2, 0);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    SetLVal(vm, &locals[152]); // color_dark_green+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 36, 21);
    U_PUSHFLT(vm, regs + 0, 0);
    U_PUSHFLT(vm, regs + 1, 0);
    U_PUSHFLT(vm, regs + 2, 1056964608);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    SetLVal(vm, &locals[156]); // color_dark_blue+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 37, 21);
    U_PUSHFLT(vm, regs + 0, 1056964608);
    U_PUSHFLT(vm, regs + 1, 1056964608);
    U_PUSHFLT(vm, regs + 2, 0);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    SetLVal(vm, &locals[160]); // color_olive+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 38, 21);
    U_PUSHFLT(vm, regs + 0, 0);
    U_PUSHFLT(vm, regs + 1, 1056964608);
    U_PUSHFLT(vm, regs + 2, 1056964608);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    SetLVal(vm, &locals[164]); // color_teal+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 39, 21);
    U_PUSHFLT(vm, regs + 0, 1056964608);
    U_PUSHFLT(vm, regs + 1, 0);
    U_PUSHFLT(vm, regs + 2, 1056964608);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    SetLVal(vm, &locals[168]); // color_purple+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 41, 21);
    U_PUSHFLT(vm, regs + 0, 1065353216);
    U_PUSHFLT(vm, regs + 1, 1056964608);
    U_PUSHFLT(vm, regs + 2, 0);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    SetLVal(vm, &locals[172]); // color_orange+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 42, 21);
    U_PUSHFLT(vm, regs + 0, 1056964608);
    U_PUSHFLT(vm, regs + 1, 1065353216);
    U_PUSHFLT(vm, regs + 2, 0);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    SetLVal(vm, &locals[176]); // color_chartreuse+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 43, 21);
    U_PUSHFLT(vm, regs + 0, 0);
    U_PUSHFLT(vm, regs + 1, 1065353216);
    U_PUSHFLT(vm, regs + 2, 1056964608);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    SetLVal(vm, &locals[180]); // color_springgreen+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 44, 21);
    U_PUSHFLT(vm, regs + 0, 0);
    U_PUSHFLT(vm, regs + 1, 1056964608);
    U_PUSHFLT(vm, regs + 2, 1065353216);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    SetLVal(vm, &locals[184]); // color_dodgerblue+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 45, 21);
    U_PUSHFLT(vm, regs + 0, 1065353216);
    U_PUSHFLT(vm, regs + 1, 0);
    U_PUSHFLT(vm, regs + 2, 1056964608);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    SetLVal(vm, &locals[188]); // color_pink_violet+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 46, 21);
    U_PUSHFLT(vm, regs + 0, 1056964608);
    U_PUSHFLT(vm, regs + 1, 0);
    U_PUSHFLT(vm, regs + 2, 1065353216);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    SetLVal(vm, &locals[192]); // color_blue_violet+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 48, 21);
    U_STATEMENT(vm, regs + 0, 49, 21);
    U_STATEMENT(vm, regs + 0, 51, 21);
    U_PUSHFLT64(vm, regs + 0, -446676599, 1070801616);
    U_PUSHFLT64(vm, regs + 1, 962072674, 1071827124);
    U_PUSHFLT64(vm, regs + 2, -1614907703, 1069362970);
    U_PUSHFLT(vm, regs + 3, 0);
    SetLVal(vm, &locals[196]); // color_luminance_weights+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 52, 21);
    U_STATEMENT(vm, regs + 0, 7, 20);
    U_PUSHSTR(vm, regs + 0, 12); // "smallpt"
    U_PUSHFUN(vm, regs + 1, 0, fun_367);
    fun_4(vm, regs + 2); // call: run_test
    U_STATEMENT(vm, regs + 0, 3, 22);
    U_PUSHSTR(vm, regs + 0, 13); // "string"
    U_PUSHFUN(vm, regs + 1, 0, fun_374);
    fun_4(vm, regs + 2); // call: run_test
    U_STATEMENT(vm, regs + 0, 8, 23);
    U_PUSHSTR(vm, regs + 0, 14); // "misch"
    U_PUSHFUN(vm, regs + 1, 0, fun_375);
    fun_4(vm, regs + 2); // call: run_test
    U_STATEMENT(vm, regs + 0, 6, 24);
    U_PUSHSTR(vm, regs + 0, 15); // "lifetime"
    U_PUSHFUN(vm, regs + 1, 0, fun_376);
    fun_4(vm, regs + 2); // call: run_test
    U_STATEMENT(vm, regs + 0, 6, 25);
    U_PUSHSTR(vm, regs + 0, 16); // "builtins"
    U_PUSHFUN(vm, regs + 1, 0, fun_379);
    fun_4(vm, regs + 2); // call: run_test
    U_STATEMENT(vm, regs + 0, 9, 27);
    U_STATEMENT(vm, regs + 0, 15, 27);
    U_STATEMENT(vm, regs + 0, 18, 27);
    U_STATEMENT(vm, regs + 0, 21, 27);
    U_STATEMENT(vm, regs + 0, 7, 27);
    U_STATEMENT(vm, regs + 0, 7, 27);
    U_STATEMENT(vm, regs + 0, 24, 27);
    U_STATEMENT(vm, regs + 0, 29, 27);
    U_STATEMENT(vm, regs + 0, 32, 27);
    U_STATEMENT(vm, regs + 0, 50, 27);
    U_STATEMENT(vm, regs + 0, 57, 27);
    U_STATEMENT(vm, regs + 0, 62, 27);
    U_STATEMENT(vm, regs + 0, 67, 27);
    U_PUSHFLT(vm, regs + 0, 0);
    U_PUSHFLT(vm, regs + 1, 0);
    U_PUSHFLT(vm, regs + 2, 0);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    SetLVal(vm, &locals[200]); // quat_unit+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 10, 26);
    U_STATEMENT(vm, regs + 0, 11, 26);
    U_STATEMENT(vm, regs + 0, 12, 26);
    U_STATEMENT(vm, regs + 0, 13, 26);
    U_STATEMENT(vm, regs + 0, 14, 26);
    U_STATEMENT(vm, regs + 0, 15, 26);
    U_STATEMENT(vm, regs + 0, 16, 26);
    U_STATEMENT(vm, regs + 0, 17, 26);
    U_STATEMENT(vm, regs + 0, 18, 26);
    U_STATEMENT(vm, regs + 0, 19, 26);
    U_STATEMENT(vm, regs + 0, 21, 26);
    U_STATEMENT(vm, regs + 0, 22, 26);
    U_STATEMENT(vm, regs + 0, 23, 26);
    U_STATEMENT(vm, regs + 0, 24, 26);
    U_STATEMENT(vm, regs + 0, 25, 26);
    U_STATEMENT(vm, regs + 0, 26, 26);
    U_STATEMENT(vm, regs + 0, 27, 26);
    U_STATEMENT(vm, regs + 0, 28, 26);
    U_STATEMENT(vm, regs + 0, 29, 26);
    U_STATEMENT(vm, regs + 0, 30, 26);
    U_STATEMENT(vm, regs + 0, 32, 26);
    U_STATEMENT(vm, regs + 0, 33, 26);
    U_STATEMENT(vm, regs + 0, 34, 26);
    U_STATEMENT(vm, regs + 0, 35, 26);
    U_STATEMENT(vm, regs + 0, 36, 26);
    U_STATEMENT(vm, regs + 0, 37, 26);
    U_STATEMENT(vm, regs + 0, 39, 26);
    U_STATEMENT(vm, regs + 0, 40, 26);
    U_STATEMENT(vm, regs + 0, 41, 26);
    U_STATEMENT(vm, regs + 0, 42, 26);
    U_STATEMENT(vm, regs + 0, 44, 26);
    U_STATEMENT(vm, regs + 0, 46, 26);
    U_STATEMENT(vm, regs + 0, 7, 26);
    U_STATEMENT(vm, regs + 0, 7, 26);
    U_STATEMENT(vm, regs + 0, 54, 26);
    U_STATEMENT(vm, regs + 0, 55, 26);
    U_STATEMENT(vm, regs + 0, 56, 26);
    U_STATEMENT(vm, regs + 0, 57, 26);
    U_STATEMENT(vm, regs + 0, 58, 26);
    U_STATEMENT(vm, regs + 0, 59, 26);
    U_STATEMENT(vm, regs + 0, 60, 26);
    U_STATEMENT(vm, regs + 0, 61, 26);
    U_STATEMENT(vm, regs + 0, 62, 26);
    U_STATEMENT(vm, regs + 0, 63, 26);
    U_STATEMENT(vm, regs + 0, 65, 26);
    U_STATEMENT(vm, regs + 0, 66, 26);
    U_STATEMENT(vm, regs + 0, 67, 26);
    U_STATEMENT(vm, regs + 0, 68, 26);
    U_STATEMENT(vm, regs + 0, 69, 26);
    U_STATEMENT(vm, regs + 0, 70, 26);
    U_STATEMENT(vm, regs + 0, 72, 26);
    U_STATEMENT(vm, regs + 0, 73, 26);
    U_STATEMENT(vm, regs + 0, 75, 26);
    U_STATEMENT(vm, regs + 0, 51, 26);
    U_STATEMENT(vm, regs + 0, 51, 26);
    U_STATEMENT(vm, regs + 0, 78, 26);
    U_PUSHSTR(vm, regs + 0, 17); // "operators"
    U_PUSHFUN(vm, regs + 1, 0, fun_442);
    fun_4(vm, regs + 2); // call: run_test
    U_STATEMENT(vm, regs + 0, 9, 29);
    U_STATEMENT(vm, regs + 0, 13, 29);
    U_STATEMENT(vm, regs + 0, 17, 29);
    U_STATEMENT(vm, regs + 0, 20, 29);
    U_STATEMENT(vm, regs + 0, 23, 29);
    U_STATEMENT(vm, regs + 0, 26, 29);
    U_STATEMENT(vm, regs + 0, 7, 29);
    U_STATEMENT(vm, regs + 0, 7, 29);
    U_STATEMENT(vm, regs + 0, 33, 29);
    U_PUSHFLT(vm, regs + 0, 1065353216);
    U_PUSHFLT(vm, regs + 1, 0);
    U_PUSHFLT(vm, regs + 2, 0);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    U_LVAL_VARF(vm, regs + 4, 1150); // mat2x2_identity+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 4, 28);
    U_PUSHSTR(vm, regs + 0, 18); // "matrix"
    U_PUSHFUN(vm, regs + 1, 0, fun_449);
    fun_4(vm, regs + 2); // call: run_test
    U_STATEMENT(vm, regs + 0, 4, 30);
    U_PUSHSTR(vm, regs + 0, 19); // "typeswitch"
    U_PUSHFUN(vm, regs + 1, 0, fun_451);
    fun_4(vm, regs + 2); // call: run_test
    U_STATEMENT(vm, regs + 0, 30, 0);
    U_RETURNLOCAL(vm, 0, 0);
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
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 246, locals);
    U_STATEMENT(vm, regs + 0, 8, 2);
    regs[0] = locals[1]; // t
    U_CALLV(vm, regs + 1); vm.next_call_target(vm, regs + 0);
    U_RETURNLOCAL(vm, 0, 0);
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
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 253, locals);
    U_STATEMENT(vm, regs + 0, 11, 2);
    regs[0] = locals[0]; // name
    regs[1] = locals[1]; // t
    U_PUSHVARF(vm, regs + 2, 0); // current_test_runner
    U_CALLV(vm, regs + 3); vm.next_call_target(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 0);
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
    PushFunId(vm, funinfo_table + 260, 0);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 15, 2);
    U_PUSHSTR(vm, regs + 0, 20); // "a"
    U_PUSHSTR(vm, regs + 1, 21); // "b"
    U_SADD(vm, regs + 2);
    keepvar[0] = TopM(regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 22); // "ab"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 15, 2, 23);
    U_RETURNLOCAL(vm, 0, 0);
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
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 265, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 6, 0);
    U_PUSHSTR(vm, regs + 0, 24); // "testing: "
    regs[1] = locals[0]; // name
    U_SADD(vm, regs + 2);
    keepvar[0] = TopM(regs + 1, 0);
    U_BCALLRET1(vm, regs + 1, 0, 0); // print
    U_STATEMENT(vm, regs + 0, 7, 0);
    regs[0] = locals[1]; // t
    U_CALLV(vm, regs + 1); vm.next_call_target(vm, regs + 0);
    U_RETURNLOCAL(vm, 0, 0);
    Pop(psp);
    Pop(psp);
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
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 272, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 5);
    U_NEWVEC(vm, regs + 0, 11, 0);
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block3182:;
    if (!U_VFOR(vm, regs + 2)) goto block3234;
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_VFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[4]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 6, 5);
    regs[2] = locals[2]; // r
    regs[3] = locals[3]; // x
    fun_457(vm, regs + 4); // call: function400
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block3182;
    block3234:;
    U_STATEMENT(vm, regs + 0, 7, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
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
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 282, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 5);
    U_NEWVEC(vm, regs + 0, 11, 0);
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block3300:;
    if (!U_VFOR(vm, regs + 2)) goto block3352;
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_VFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[4]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 6, 5);
    regs[2] = locals[2]; // r
    regs[3] = locals[3]; // x
    fun_456(vm, regs + 4); // call: function399
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block3300;
    block3352:;
    U_STATEMENT(vm, regs + 0, 7, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
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
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 292, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 5);
    U_NEWVEC(vm, regs + 0, 7, 0);
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block3419:;
    if (!U_VFOR(vm, regs + 2)) goto block3471;
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_VFORELEMREF(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x
    U_LV_WRITEREF(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[4]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 6, 5);
    regs[2] = locals[2]; // r
    regs[3] = locals[3]; // x
    fun_455(vm, regs + 4); // call: function398
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block3419;
    block3471:;
    U_STATEMENT(vm, regs + 0, 7, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_610(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 302, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 5);
    U_NEWVEC(vm, regs + 0, 7, 0);
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block3538:;
    if (!U_VFOR(vm, regs + 2)) goto block3590;
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_VFORELEMREF(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x
    U_LV_WRITEREF(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[4]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 6, 5);
    regs[2] = locals[2]; // r
    regs[3] = locals[3]; // x
    fun_454(vm, regs + 4); // call: function397
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block3538;
    block3590:;
    U_STATEMENT(vm, regs + 0, 7, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_609(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 312, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 5);
    U_NEWVEC(vm, regs + 0, 7, 0);
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block3657:;
    if (!U_VFOR(vm, regs + 2)) goto block3709;
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_VFORELEMREF(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x
    U_LV_WRITEREF(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[4]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 6, 5);
    regs[2] = locals[2]; // r
    regs[3] = locals[3]; // x
    fun_453(vm, regs + 4); // call: function396
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block3657;
    block3709:;
    U_STATEMENT(vm, regs + 0, 7, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_608(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 322, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 5);
    U_NEWVEC(vm, regs + 0, 7, 0);
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block3776:;
    if (!U_VFOR(vm, regs + 2)) goto block3831;
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_VFORELEMREF(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x
    U_LV_WRITEREF(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[4]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 6, 5);
    regs[2] = locals[2]; // r
    regs[3] = locals[3]; // x
    U_INCREF(vm, regs + 4, 0);
    fun_452(vm, regs + 4); // call: function395
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block3776;
    block3831:;
    U_STATEMENT(vm, regs + 0, 7, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_601(VMRef vm, StackPtr psp) {
    Value regs[8];
    Value keepvar[1];
    Value locals[7];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    locals[5] = lobster::NilVal();
    locals[6] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 332, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 5);
    U_NEWVEC(vm, regs + 0, 1256, 0);
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block3899:;
    if (!U_VFOR(vm, regs + 2)) goto block3953;
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_VFORELEM2S(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x+0
    U_LV_WRITEV(vm, regs + 5, 3);
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[6]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 6, 5);
    regs[2] = locals[2]; // r
    regs[3] = locals[3];regs[4] = locals[4];regs[5] = locals[5]; // x
    fun_373(vm, regs + 6); // call: function340
    U_PUSHINT(vm, regs + 7, 4);
    U_BCALLRETV(vm, regs + 8, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block3899;
    block3953:;
    U_STATEMENT(vm, regs + 0, 7, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_600(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 344, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 5);
    U_NEWVEC(vm, regs + 0, 1258, 0);
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block4020:;
    if (!U_VFOR(vm, regs + 2)) goto block4072;
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_VFORELEMREF(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x
    U_LV_WRITEREF(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[4]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 6, 5);
    regs[2] = locals[2]; // r
    regs[3] = locals[3]; // x
    fun_372(vm, regs + 4); // call: function339
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block4020;
    block4072:;
    U_STATEMENT(vm, regs + 0, 7, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_599(VMRef vm, StackPtr psp) {
    Value regs[7];
    Value keepvar[1];
    Value locals[5];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 354, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 5);
    U_NEWVEC(vm, regs + 0, 1252, 0);
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block4138:;
    if (!U_IFOR(vm, regs + 2)) goto block4192;
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_IFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[4]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 6, 5);
    regs[2] = locals[2]; // r
    U_STATEMENT(vm, regs + 3, 119, 20);
    U_PUSHVARVF(vm, regs + 3, 84, 3); // float3_0+0
    U_PUSHINT(vm, regs + 6, 3);
    U_BCALLRETV(vm, regs + 7, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block4138;
    block4192:;
    U_STATEMENT(vm, regs + 0, 7, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_598(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 364, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 5);
    U_NEWVEC(vm, regs + 0, 1254, 0);
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block4258:;
    if (!U_IFOR(vm, regs + 2)) goto block4307;
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_IFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[4]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 6, 5);
    regs[2] = locals[2]; // r
    fun_370(vm, regs + 3); // call: function337
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block4258;
    block4307:;
    U_STATEMENT(vm, regs + 0, 7, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_590(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 374, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 5);
    U_NEWVEC(vm, regs + 0, 13, 0);
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block4375:;
    if (!U_VFOR(vm, regs + 2)) goto block4430;
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_VFORELEMREF(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x
    U_LV_WRITEREF(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[4]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 6, 5);
    regs[2] = locals[2]; // r
    regs[3] = locals[3]; // x
    regs[4] = locals[4]; // i
    fun_358(vm, regs + 5); // call: function325
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block4375;
    block4430:;
    U_STATEMENT(vm, regs + 0, 7, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_589(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[6];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    locals[5] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 384, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 5);
    U_NEWVEC(vm, regs + 0, 13, 0);
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block4498:;
    if (!U_VFOR(vm, regs + 2)) goto block4553;
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_VFORELEM2S(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x+0
    U_LV_WRITEV(vm, regs + 4, 2);
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[5]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 6, 5);
    regs[2] = locals[2]; // r
    U_STATEMENT(vm, regs + 3, 55, 19);
    U_NEWVEC(vm, regs + 3, 7, 0);
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block4498;
    block4553:;
    U_STATEMENT(vm, regs + 0, 7, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_584(VMRef vm, StackPtr psp) {
    Value regs[6];
    Value keepvar[1];
    Value locals[6];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    locals[5] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 395, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 5);
    U_NEWVEC(vm, regs + 0, 1148, 0);
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block4621:;
    if (!U_VFOR(vm, regs + 2)) goto block4676;
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_VFORELEM2S(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x+0
    U_LV_WRITEV(vm, regs + 4, 2);
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[5]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 6, 5);
    regs[2] = locals[2]; // r
    U_STATEMENT(vm, regs + 3, 23, 19);
    U_PUSHVARVF(vm, regs + 3, 74, 2); // float2_0+0
    U_PUSHINT(vm, regs + 5, 2);
    U_BCALLRETV(vm, regs + 6, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block4621;
    block4676:;
    U_STATEMENT(vm, regs + 0, 7, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_583(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 406, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 5);
    U_NEWVEC(vm, regs + 0, 9, 0);
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block4742:;
    if (!U_IFOR(vm, regs + 2)) goto block4795;
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_IFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[4]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 6, 5);
    regs[2] = locals[2]; // r
    U_STATEMENT(vm, regs + 3, 13, 18);
    U_PUSHFLT(vm, regs + 3, 0);
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block4742;
    block4795:;
    U_STATEMENT(vm, regs + 0, 7, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
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
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 416, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 5);
    U_NEWVEC(vm, regs + 0, 9, 0);
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block4861:;
    if (!U_IFOR(vm, regs + 2)) goto block4914;
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_IFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[4]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 6, 5);
    regs[2] = locals[2]; // r
    U_STATEMENT(vm, regs + 3, 34, 18);
    U_PUSHFLT(vm, regs + 3, 0);
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block4861;
    block4914:;
    U_STATEMENT(vm, regs + 0, 7, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
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
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 426, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 5);
    U_NEWVEC(vm, regs + 0, 9, 0);
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block4980:;
    if (!U_IFOR(vm, regs + 2)) goto block5033;
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_IFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[4]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 6, 5);
    regs[2] = locals[2]; // r
    U_STATEMENT(vm, regs + 3, 33, 18);
    U_PUSHFLT(vm, regs + 3, 0);
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block4980;
    block5033:;
    U_STATEMENT(vm, regs + 0, 7, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
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
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 436, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 5);
    U_NEWVEC(vm, regs + 0, 1138, 0);
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block5099:;
    if (!U_IFOR(vm, regs + 2)) goto block5151;
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_IFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[4]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 6, 5);
    regs[2] = locals[2]; // r
    regs[3] = locals[3]; // x
    fun_338(vm, regs + 4); // call: function305
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block5099;
    block5151:;
    U_STATEMENT(vm, regs + 0, 7, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_578(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 446, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 5);
    U_NEWVEC(vm, regs + 0, 1140, 0);
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block5217:;
    if (!U_IFOR(vm, regs + 2)) goto block5269;
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_IFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[4]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 6, 5);
    regs[2] = locals[2]; // r
    regs[3] = locals[3]; // x
    fun_337(vm, regs + 4); // call: function304
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block5217;
    block5269:;
    U_STATEMENT(vm, regs + 0, 7, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
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
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 456, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 5);
    U_NEWVEC(vm, regs + 0, 1119, 0);
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block5335:;
    if (!U_IFOR(vm, regs + 2)) goto block5387;
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_IFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[4]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 6, 5);
    regs[2] = locals[2]; // r
    regs[3] = locals[3]; // x
    fun_577(vm, regs + 4); // call: function99
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block5335;
    block5387:;
    U_STATEMENT(vm, regs + 0, 7, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
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
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 466, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 5);
    U_NEWVEC(vm, regs + 0, 1121, 0);
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block5453:;
    if (!U_IFOR(vm, regs + 2)) goto block5505;
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_IFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[4]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 6, 5);
    regs[2] = locals[2]; // r
    regs[3] = locals[3]; // x
    fun_574(vm, regs + 4); // call: function98
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block5453;
    block5505:;
    U_STATEMENT(vm, regs + 0, 7, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_570(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 476, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 5);
    U_NEWVEC(vm, regs + 0, 11, 0);
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block5572:;
    if (!U_VFOR(vm, regs + 2)) goto block5624;
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_VFORELEMREF(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x
    U_LV_WRITEREF(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[4]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 6, 5);
    regs[2] = locals[2]; // r
    regs[3] = locals[3]; // x
    fun_321(vm, regs + 4); // call: function288
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block5572;
    block5624:;
    U_STATEMENT(vm, regs + 0, 7, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_563(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[6];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    locals[5] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 486, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 5);
    U_NEWVEC(vm, regs + 0, 7, 0);
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block5692:;
    if (!U_VFOR(vm, regs + 2)) goto block5746;
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_VFORELEM2S(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x+0
    U_LV_WRITEV(vm, regs + 4, 2);
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[5]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 6, 5);
    regs[2] = locals[2]; // r
    regs[3] = locals[3];regs[4] = locals[4]; // x
    fun_308(vm, regs + 5); // call: function278
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block5692;
    block5746:;
    U_STATEMENT(vm, regs + 0, 7, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_562(VMRef vm, StackPtr psp) {
    Value regs[6];
    Value keepvar[1];
    Value locals[6];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    locals[5] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 497, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 5);
    U_NEWVEC(vm, regs + 0, 37, 0);
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block5813:;
    if (!U_VFOR(vm, regs + 2)) goto block5867;
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_VFORELEM2S(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x+0
    U_LV_WRITEV(vm, regs + 4, 2);
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[5]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 6, 5);
    regs[2] = locals[2]; // r
    regs[3] = locals[3];regs[4] = locals[4]; // x
    fun_305(vm, regs + 5); // call: function275
    U_PUSHINT(vm, regs + 5, 2);
    U_BCALLRETV(vm, regs + 6, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block5813;
    block5867:;
    U_STATEMENT(vm, regs + 0, 7, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_561(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 508, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 5);
    U_NEWVEC(vm, regs + 0, 7, 0);
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block5933:;
    if (!U_IFOR(vm, regs + 2)) goto block5985;
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_IFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[4]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 6, 5);
    regs[2] = locals[2]; // r
    regs[3] = locals[3]; // x
    fun_102(vm, regs + 4); // call: function99
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block5933;
    block5985:;
    U_STATEMENT(vm, regs + 0, 7, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_560(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 518, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 5);
    U_NEWVEC(vm, regs + 0, 13, 0);
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block6051:;
    if (!U_IFOR(vm, regs + 2)) goto block6103;
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_IFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[4]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 6, 5);
    regs[2] = locals[2]; // r
    regs[3] = locals[3]; // x
    fun_101(vm, regs + 4); // call: function98
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block6051;
    block6103:;
    U_STATEMENT(vm, regs + 0, 7, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_559(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 528, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 5);
    U_NEWVEC(vm, regs + 0, 11, 0);
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block6170:;
    if (!U_VFOR(vm, regs + 2)) goto block6222;
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_VFORELEMREF(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x
    U_LV_WRITEREF(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[4]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 6, 5);
    regs[2] = locals[2]; // r
    regs[3] = locals[3]; // x
    fun_301(vm, regs + 4); // call: function271
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block6170;
    block6222:;
    U_STATEMENT(vm, regs + 0, 7, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_555(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 538, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 5);
    U_NEWVEC(vm, regs + 0, 11, 0);
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block6289:;
    if (!U_VFOR(vm, regs + 2)) goto block6341;
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_VFORELEMREF(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x
    U_LV_WRITEREF(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[4]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 6, 5);
    regs[2] = locals[2]; // r
    regs[3] = locals[3]; // x
    fun_284(vm, regs + 4); // call: function264
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block6289;
    block6341:;
    U_STATEMENT(vm, regs + 0, 7, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_540(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 548, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 5);
    U_NEWVEC(vm, regs + 0, 1005, 0);
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block6407:;
    if (!U_IFOR(vm, regs + 2)) goto block6459;
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_IFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[4]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 6, 5);
    regs[2] = locals[2]; // r
    regs[3] = locals[3]; // x
    fun_281(vm, regs + 4); // call: function261
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block6407;
    block6459:;
    U_STATEMENT(vm, regs + 0, 7, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_539(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 558, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 5);
    U_NEWVEC(vm, regs + 0, 1027, 0);
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block6525:;
    if (!U_IFOR(vm, regs + 2)) goto block6577;
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_IFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[4]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 6, 5);
    regs[2] = locals[2]; // r
    regs[3] = locals[3]; // x
    fun_280(vm, regs + 4); // call: function260
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block6525;
    block6577:;
    U_STATEMENT(vm, regs + 0, 7, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_529(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 568, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 5);
    U_NEWVEC(vm, regs + 0, 540, 0);
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block6643:;
    if (!U_IFOR(vm, regs + 2)) goto block6695;
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_IFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[4]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 6, 5);
    regs[2] = locals[2]; // r
    U_STATEMENT(vm, regs + 3, 99, 8);
    U_PUSHNIL(vm, regs + 3);
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block6643;
    block6695:;
    U_STATEMENT(vm, regs + 0, 7, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
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
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 578, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 5);
    U_NEWVEC(vm, regs + 0, 542, 0);
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block6761:;
    if (!U_IFOR(vm, regs + 2)) goto block6813;
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_IFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[4]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 6, 5);
    regs[2] = locals[2]; // r
    U_STATEMENT(vm, regs + 3, 99, 8);
    U_PUSHNIL(vm, regs + 3);
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block6761;
    block6813:;
    U_STATEMENT(vm, regs + 0, 7, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_499(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 588, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 5);
    U_NEWVEC(vm, regs + 0, 7, 0);
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block6879:;
    if (!U_IFOR(vm, regs + 2)) goto block6931;
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_IFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[4]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 6, 5);
    regs[2] = locals[2]; // r
    regs[3] = locals[3]; // x
    fun_197(vm, regs + 4); // call: function189
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block6879;
    block6931:;
    U_STATEMENT(vm, regs + 0, 7, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_489(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 598, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 5);
    U_NEWVEC(vm, regs + 0, 697, 0);
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block6997:;
    if (!U_IFOR(vm, regs + 2)) goto block7049;
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_IFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[4]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 6, 5);
    regs[2] = locals[2]; // r
    U_STATEMENT(vm, regs + 3, 99, 8);
    U_PUSHNIL(vm, regs + 3);
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block6997;
    block7049:;
    U_STATEMENT(vm, regs + 0, 7, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
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
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 608, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 5);
    U_NEWVEC(vm, regs + 0, 657, 0);
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block7115:;
    if (!U_IFOR(vm, regs + 2)) goto block7167;
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_IFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[4]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 6, 5);
    regs[2] = locals[2]; // r
    U_STATEMENT(vm, regs + 3, 99, 8);
    U_PUSHNIL(vm, regs + 3);
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block7115;
    block7167:;
    U_STATEMENT(vm, regs + 0, 7, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_479(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 618, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 5);
    U_NEWVEC(vm, regs + 0, 486, 0);
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block7233:;
    if (!U_IFOR(vm, regs + 2)) goto block7285;
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_IFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[4]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 6, 5);
    regs[2] = locals[2]; // r
    U_STATEMENT(vm, regs + 3, 99, 8);
    U_PUSHNIL(vm, regs + 3);
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block7233;
    block7285:;
    U_STATEMENT(vm, regs + 0, 7, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// map
static void fun_468(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[5];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 628, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 5);
    U_NEWVEC(vm, regs + 0, 7, 0);
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block7351:;
    if (!U_VFOR(vm, regs + 2)) goto block7403;
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_VFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[4]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 6, 5);
    regs[2] = locals[2]; // r
    regs[3] = locals[3]; // x
    fun_164(vm, regs + 4); // call: function157
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block7351;
    block7403:;
    U_STATEMENT(vm, regs + 0, 7, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
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
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 638, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 5);
    U_NEWVEC(vm, regs + 0, 11, 0);
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 11, 1); // length
    U_BCALLRET2(vm, regs + 2, 10, 1); // vector_capacity
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block7469:;
    if (!U_IFOR(vm, regs + 2)) goto block7521;
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_IFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 5, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[4]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 6, 5);
    regs[2] = locals[2]; // r
    regs[3] = locals[3]; // x
    fun_159(vm, regs + 4); // call: function152
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block7469;
    block7521:;
    U_STATEMENT(vm, regs + 0, 7, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// filter
static void fun_564(VMRef vm, StackPtr psp) {
    Value regs[6];
    Value locals[6];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    locals[5] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 648, locals);
    U_STATEMENT(vm, regs + 0, 12, 5);
    U_NEWVEC(vm, regs + 0, 37, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 13, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block7570:;
    if (!U_VFOR(vm, regs + 2)) goto block7637;
    U_STATEMENT(vm, regs + 2, 13, 5);
    U_VFORELEM2S(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x+0
    U_LV_WRITEV(vm, regs + 4, 2);
    U_STATEMENT(vm, regs + 2, 13, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[5]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 14, 5);
    regs[2] = locals[3];regs[3] = locals[4]; // x
    fun_309(vm, regs + 4); // call: function279
    if (!U_JUMPFAIL(vm, regs + 3)) goto block7632;
    U_STATEMENT(vm, regs + 2, 15, 5);
    regs[2] = locals[2]; // r
    regs[3] = locals[3];regs[4] = locals[4]; // x
    U_PUSHINT(vm, regs + 5, 2);
    U_BCALLRETV(vm, regs + 6, 15, 1); // push
    U_POP(vm, regs + 3);
    block7632:;
    goto block7570;
    block7637:;
    U_STATEMENT(vm, regs + 0, 16, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// filter
static void fun_10(VMRef vm, StackPtr psp) {
    Value regs[6];
    Value locals[6];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    locals[5] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 659, locals);
    U_STATEMENT(vm, regs + 0, 12, 5);
    U_NEWVEC(vm, regs + 0, 37, 0);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 13, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block7687:;
    if (!U_VFOR(vm, regs + 2)) goto block7754;
    U_STATEMENT(vm, regs + 2, 13, 5);
    U_VFORELEM2S(vm, regs + 2);
    SetLVal(vm, &locals[3]); // x+0
    U_LV_WRITEV(vm, regs + 4, 2);
    U_STATEMENT(vm, regs + 2, 13, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[5]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 14, 5);
    regs[2] = locals[3];regs[3] = locals[4]; // x
    fun_307(vm, regs + 4); // call: function277
    if (!U_JUMPFAIL(vm, regs + 3)) goto block7749;
    U_STATEMENT(vm, regs + 2, 15, 5);
    regs[2] = locals[2]; // r
    regs[3] = locals[3];regs[4] = locals[4]; // x
    U_PUSHINT(vm, regs + 5, 2);
    U_BCALLRETV(vm, regs + 6, 15, 1); // push
    U_POP(vm, regs + 3);
    block7749:;
    goto block7687;
    block7754:;
    U_STATEMENT(vm, regs + 0, 16, 5);
    regs[0] = locals[2]; // r
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// partition
static void fun_592(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value locals[6];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    locals[5] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 670, locals);
    U_STATEMENT(vm, regs + 0, 37, 5);
    U_NEWVEC(vm, regs + 0, 7, 0);
    SetLVal(vm, &locals[2]); // t
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 38, 5);
    U_NEWVEC(vm, regs + 0, 7, 0);
    SetLVal(vm, &locals[3]); // f
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 39, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block7819:;
    if (!U_VFOR(vm, regs + 2)) goto block7907;
    U_STATEMENT(vm, regs + 2, 39, 5);
    U_VFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[4]); // x
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 39, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[5]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 40, 5);
    regs[2] = locals[4]; // x
    fun_593(vm, regs + 3); // call: function49
    if (!U_JUMPFAIL(vm, regs + 3)) goto block7881;
    U_STATEMENT(vm, regs + 2, 41, 5);
    regs[2] = locals[2]; // t
    regs[3] = locals[4]; // x
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block7902;
    block7881:;
    U_STATEMENT(vm, regs + 2, 43, 5);
    regs[2] = locals[3]; // f
    regs[3] = locals[4]; // x
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    block7902:;
    goto block7819;
    block7907:;
    U_STATEMENT(vm, regs + 0, 44, 5);
    regs[0] = locals[2]; // t
    regs[1] = locals[3]; // f
    U_INCREF(vm, regs + 2, 0);
    U_INCREF(vm, regs + 2, 1);
    U_RETURNLOCAL(vm, 0, 2);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[2]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// partition
static void fun_587(VMRef vm, StackPtr psp) {
    Value regs[7];
    Value locals[8];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    locals[5] = lobster::NilVal();
    locals[6] = lobster::NilVal();
    locals[7] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 681, locals);
    U_STATEMENT(vm, regs + 0, 37, 5);
    U_NEWVEC(vm, regs + 0, 1165, 0);
    SetLVal(vm, &locals[2]); // t
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 38, 5);
    U_NEWVEC(vm, regs + 0, 1165, 0);
    SetLVal(vm, &locals[3]); // f
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 39, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block7983:;
    if (!U_VFOR(vm, regs + 2)) goto block8075;
    U_STATEMENT(vm, regs + 2, 39, 5);
    U_VFORELEM2S(vm, regs + 2);
    SetLVal(vm, &locals[4]); // x+0
    U_LV_WRITEV(vm, regs + 5, 3);
    U_STATEMENT(vm, regs + 2, 39, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[7]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 40, 5);
    regs[2] = locals[4];regs[3] = locals[5];regs[4] = locals[6]; // x
    fun_588(vm, regs + 5); // call: function49
    if (!U_JUMPFAIL(vm, regs + 3)) goto block8048;
    U_STATEMENT(vm, regs + 2, 41, 5);
    regs[2] = locals[2]; // t
    regs[3] = locals[4];regs[4] = locals[5];regs[5] = locals[6]; // x
    U_PUSHINT(vm, regs + 6, 3);
    U_BCALLRETV(vm, regs + 7, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block8070;
    block8048:;
    U_STATEMENT(vm, regs + 2, 43, 5);
    regs[2] = locals[3]; // f
    regs[3] = locals[4];regs[4] = locals[5];regs[5] = locals[6]; // x
    U_PUSHINT(vm, regs + 6, 3);
    U_BCALLRETV(vm, regs + 7, 15, 1); // push
    U_POP(vm, regs + 3);
    block8070:;
    goto block7983;
    block8075:;
    U_STATEMENT(vm, regs + 0, 44, 5);
    regs[0] = locals[2]; // t
    regs[1] = locals[3]; // f
    U_INCREF(vm, regs + 2, 0);
    U_INCREF(vm, regs + 2, 1);
    U_RETURNLOCAL(vm, 0, 2);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[2]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// partition
static void fun_13(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value locals[6];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    locals[5] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 694, locals);
    U_STATEMENT(vm, regs + 0, 37, 5);
    U_NEWVEC(vm, regs + 0, 7, 0);
    SetLVal(vm, &locals[2]); // t
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 38, 5);
    U_NEWVEC(vm, regs + 0, 7, 0);
    SetLVal(vm, &locals[3]); // f
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 39, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block8149:;
    if (!U_VFOR(vm, regs + 2)) goto block8237;
    U_STATEMENT(vm, regs + 2, 39, 5);
    U_VFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[4]); // x
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 39, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[5]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 40, 5);
    regs[2] = locals[4]; // x
    fun_49(vm, regs + 3); // call: function49
    if (!U_JUMPFAIL(vm, regs + 3)) goto block8211;
    U_STATEMENT(vm, regs + 2, 41, 5);
    regs[2] = locals[2]; // t
    regs[3] = locals[4]; // x
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    goto block8232;
    block8211:;
    U_STATEMENT(vm, regs + 2, 43, 5);
    regs[2] = locals[3]; // f
    regs[3] = locals[4]; // x
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    block8232:;
    goto block8149;
    block8237:;
    U_STATEMENT(vm, regs + 0, 44, 5);
    regs[0] = locals[2]; // t
    regs[1] = locals[3]; // f
    U_INCREF(vm, regs + 2, 0);
    U_INCREF(vm, regs + 2, 1);
    U_RETURNLOCAL(vm, 0, 2);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[2]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// exists
static void fun_597(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[4];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 705, locals);
    U_STATEMENT(vm, regs + 0, 57, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block8281:;
    if (!U_VFOR(vm, regs + 2)) goto block8340;
    U_STATEMENT(vm, regs + 2, 57, 5);
    U_VFORELEMREF(vm, regs + 2);
    SetLVal(vm, &locals[2]); // x
    U_LV_WRITEREF(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 57, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[3]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 58, 5);
    regs[2] = locals[2]; // x
    fun_363(vm, regs + 3); // call: function330
    if (!U_JUMPFAIL(vm, regs + 3)) goto block8335;
    U_STATEMENT(vm, regs + 2, 59, 5);
    U_POP(vm, regs + 2);
    U_POP(vm, regs + 1);
    U_PUSHINT(vm, regs + 0, 1);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[2]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    block8335:;
    goto block8281;
    block8340:;
    U_STATEMENT(vm, regs + 0, 60, 5);
    U_PUSHINT(vm, regs + 0, 0);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[2]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// exists
static void fun_596(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[4];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 714, locals);
    U_STATEMENT(vm, regs + 0, 57, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block8376:;
    if (!U_VFOR(vm, regs + 2)) goto block8435;
    U_STATEMENT(vm, regs + 2, 57, 5);
    U_VFORELEMREF(vm, regs + 2);
    SetLVal(vm, &locals[2]); // x
    U_LV_WRITEREF(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 57, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[3]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 58, 5);
    regs[2] = locals[2]; // x
    fun_362(vm, regs + 3); // call: function329
    if (!U_JUMPFAIL(vm, regs + 3)) goto block8430;
    U_STATEMENT(vm, regs + 2, 59, 5);
    U_POP(vm, regs + 2);
    U_POP(vm, regs + 1);
    U_PUSHINT(vm, regs + 0, 1);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[2]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    block8430:;
    goto block8376;
    block8435:;
    U_STATEMENT(vm, regs + 0, 60, 5);
    U_PUSHINT(vm, regs + 0, 0);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[2]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// exists
static void fun_565(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[4];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 723, locals);
    U_STATEMENT(vm, regs + 0, 57, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block8471:;
    if (!U_VFOR(vm, regs + 2)) goto block8530;
    U_STATEMENT(vm, regs + 2, 57, 5);
    U_VFORELEMREF(vm, regs + 2);
    SetLVal(vm, &locals[2]); // x
    U_LV_WRITEREF(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 57, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[3]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 58, 5);
    regs[2] = locals[2]; // x
    fun_330(vm, regs + 3); // call: function297
    if (!U_JUMPFAIL(vm, regs + 3)) goto block8525;
    U_STATEMENT(vm, regs + 2, 59, 5);
    U_POP(vm, regs + 2);
    U_POP(vm, regs + 1);
    U_PUSHINT(vm, regs + 0, 1);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[2]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    block8525:;
    goto block8471;
    block8530:;
    U_STATEMENT(vm, regs + 0, 60, 5);
    U_PUSHINT(vm, regs + 0, 0);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[2]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// exists
static void fun_15(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[4];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 732, locals);
    U_STATEMENT(vm, regs + 0, 57, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block8564:;
    if (!U_SFOR(vm, regs + 2)) goto block8623;
    U_STATEMENT(vm, regs + 2, 57, 5);
    U_SFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[2]); // x
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 57, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[3]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 58, 5);
    regs[2] = locals[2]; // x
    fun_326(vm, regs + 3); // call: function293
    if (!U_JUMPFAIL(vm, regs + 3)) goto block8618;
    U_STATEMENT(vm, regs + 2, 59, 5);
    U_POP(vm, regs + 2);
    U_POP(vm, regs + 1);
    U_PUSHINT(vm, regs + 0, 1);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    block8618:;
    goto block8564;
    block8623:;
    U_STATEMENT(vm, regs + 0, 60, 5);
    U_PUSHINT(vm, regs + 0, 0);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// fold
static void fun_569(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[4];
    locals[0] = *(psp - 3);
    locals[1] = *(psp - 2);
    locals[2] = *(psp - 1);
    locals[3] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 741, locals);
    U_STATEMENT(vm, regs + 0, 67, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block8658:;
    if (!U_VFOR(vm, regs + 2)) goto block8695;
    U_STATEMENT(vm, regs + 2, 68, 5);
    U_VFORELEMREF(vm, regs + 2);
    SetLVal(vm, &locals[3]); // _
    U_LV_WRITEREF(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 68, 5);
    regs[2] = locals[1]; // acc
    regs[3] = locals[3]; // _
    fun_319(vm, regs + 4); // call: function287
    SetLVal(vm, &locals[1]); // acc
    U_LV_WRITE(vm, regs + 3);
    goto block8658;
    block8695:;
    U_STATEMENT(vm, regs + 0, 69, 5);
    regs[0] = locals[1]; // acc
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// fold
static void fun_568(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[4];
    locals[0] = *(psp - 3);
    locals[1] = *(psp - 2);
    locals[2] = *(psp - 1);
    locals[3] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 750, locals);
    U_STATEMENT(vm, regs + 0, 67, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block8730:;
    if (!U_VFOR(vm, regs + 2)) goto block8767;
    U_STATEMENT(vm, regs + 2, 68, 5);
    U_VFORELEMREF(vm, regs + 2);
    SetLVal(vm, &locals[3]); // _
    U_LV_WRITEREF(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 68, 5);
    regs[2] = locals[1]; // acc
    regs[3] = locals[3]; // _
    fun_318(vm, regs + 4); // call: function286
    SetLVal(vm, &locals[1]); // acc
    U_LV_WRITE(vm, regs + 3);
    goto block8730;
    block8767:;
    U_STATEMENT(vm, regs + 0, 69, 5);
    regs[0] = locals[1]; // acc
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// fold
static void fun_567(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[4];
    locals[0] = *(psp - 3);
    locals[1] = *(psp - 2);
    locals[2] = *(psp - 1);
    locals[3] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 759, locals);
    U_STATEMENT(vm, regs + 0, 67, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block8802:;
    if (!U_VFOR(vm, regs + 2)) goto block8839;
    U_STATEMENT(vm, regs + 2, 68, 5);
    U_VFORELEMREF(vm, regs + 2);
    SetLVal(vm, &locals[3]); // _
    U_LV_WRITEREF(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 68, 5);
    regs[2] = locals[1]; // acc
    regs[3] = locals[3]; // _
    fun_316(vm, regs + 4); // call: function285
    SetLVal(vm, &locals[1]); // acc
    U_LV_WRITEREF(vm, regs + 3);
    goto block8802;
    block8839:;
    U_STATEMENT(vm, regs + 0, 69, 5);
    regs[0] = locals[1]; // acc
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// fold
static void fun_556(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[4];
    locals[0] = *(psp - 3);
    locals[1] = *(psp - 2);
    locals[2] = *(psp - 1);
    locals[3] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 768, locals);
    U_STATEMENT(vm, regs + 0, 67, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block8874:;
    if (!U_VFOR(vm, regs + 2)) goto block8911;
    U_STATEMENT(vm, regs + 2, 68, 5);
    U_VFORELEMREF(vm, regs + 2);
    SetLVal(vm, &locals[3]); // _
    U_LV_WRITEREF(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 68, 5);
    regs[2] = locals[1]; // acc
    regs[3] = locals[3]; // _
    fun_285(vm, regs + 4); // call: function265
    SetLVal(vm, &locals[1]); // acc
    U_LV_WRITEREF(vm, regs + 3);
    goto block8874;
    block8911:;
    U_STATEMENT(vm, regs + 0, 69, 5);
    regs[0] = locals[1]; // acc
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// fold
static void fun_478(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[4];
    locals[0] = *(psp - 3);
    locals[1] = *(psp - 2);
    locals[2] = *(psp - 1);
    locals[3] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 777, locals);
    U_STATEMENT(vm, regs + 0, 67, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block8945:;
    if (!U_VFOR(vm, regs + 2)) goto block8982;
    U_STATEMENT(vm, regs + 2, 68, 5);
    U_VFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[3]); // _
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 68, 5);
    regs[2] = locals[1]; // acc
    regs[3] = locals[3]; // _
    fun_180(vm, regs + 4); // call: function173
    SetLVal(vm, &locals[1]); // acc
    U_LV_WRITE(vm, regs + 3);
    goto block8945;
    block8982:;
    U_STATEMENT(vm, regs + 0, 69, 5);
    U_RETURNLOCAL(vm, 0, 0);
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
    locals[0] = *(psp - 3);
    locals[1] = *(psp - 2);
    locals[2] = *(psp - 1);
    locals[3] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 786, locals);
    U_STATEMENT(vm, regs + 0, 67, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block9013:;
    if (!U_VFOR(vm, regs + 2)) goto block9050;
    U_STATEMENT(vm, regs + 2, 68, 5);
    U_VFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[3]); // _
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 68, 5);
    regs[2] = locals[1]; // acc
    regs[3] = locals[3]; // _
    fun_30(vm, regs + 4); // call: function30
    SetLVal(vm, &locals[1]); // acc
    U_LV_WRITE(vm, regs + 3);
    goto block9013;
    block9050:;
    U_STATEMENT(vm, regs + 0, 69, 5);
    regs[0] = locals[1]; // acc
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// reduce
static void fun_595(VMRef vm, StackPtr psp) {
    Value regs[6];
    Value locals[4];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 795, locals);
    U_STATEMENT(vm, regs + 0, 77, 5);
    regs[0] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 1, 13, 1); // length
    U_ASSERT(vm, regs + 1, 77, 5, 25);
    U_STATEMENT(vm, regs + 0, 78, 5);
    regs[0] = locals[0]; // xs
    U_PUSHINT(vm, regs + 1, 0);
    U_VPUSHIDXI(vm, regs + 2);
    SetLVal(vm, &locals[2]); // acc
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 79, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_PUSHINT(vm, regs + 2, 1);
    U_ISUB(vm, regs + 3);
    block9127:;
    if (!U_IFOR(vm, regs + 2)) goto block9174;
    U_STATEMENT(vm, regs + 2, 79, 5);
    U_IFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[3]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 79, 5);
    regs[2] = locals[2]; // acc
    regs[3] = locals[0]; // xs
    regs[4] = locals[3]; // i
    U_PUSHINT(vm, regs + 5, 1);
    U_IADD(vm, regs + 6);
    U_VPUSHIDXI(vm, regs + 5);
    fun_361(vm, regs + 4); // call: function328
    SetLVal(vm, &locals[2]); // acc
    U_LV_WRITE(vm, regs + 3);
    goto block9127;
    block9174:;
    U_STATEMENT(vm, regs + 0, 80, 5);
    regs[0] = locals[2]; // acc
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// reduce
static void fun_19(VMRef vm, StackPtr psp) {
    Value regs[6];
    Value locals[4];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 804, locals);
    U_STATEMENT(vm, regs + 0, 77, 5);
    regs[0] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 1, 13, 1); // length
    U_ASSERT(vm, regs + 1, 77, 5, 26);
    U_STATEMENT(vm, regs + 0, 78, 5);
    regs[0] = locals[0]; // xs
    U_PUSHINT(vm, regs + 1, 0);
    U_VPUSHIDXI(vm, regs + 2);
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // acc
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 79, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_PUSHINT(vm, regs + 2, 1);
    U_ISUB(vm, regs + 3);
    block9254:;
    if (!U_IFOR(vm, regs + 2)) goto block9301;
    U_STATEMENT(vm, regs + 2, 79, 5);
    U_IFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[3]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 79, 5);
    regs[2] = locals[2]; // acc
    regs[3] = locals[0]; // xs
    regs[4] = locals[3]; // i
    U_PUSHINT(vm, regs + 5, 1);
    U_IADD(vm, regs + 6);
    U_VPUSHIDXI(vm, regs + 5);
    fun_322(vm, regs + 4); // call: function289
    SetLVal(vm, &locals[2]); // acc
    U_LV_WRITEREF(vm, regs + 3);
    goto block9254;
    block9301:;
    U_STATEMENT(vm, regs + 0, 80, 5);
    regs[0] = locals[2]; // acc
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// find
static void fun_594(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[4];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 813, locals);
    U_STATEMENT(vm, regs + 0, 93, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block9335:;
    if (!U_VFOR(vm, regs + 2)) goto block9394;
    U_STATEMENT(vm, regs + 2, 93, 5);
    U_VFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[2]); // x
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 93, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[3]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 94, 5);
    regs[2] = locals[2]; // x
    fun_360(vm, regs + 3); // call: function327
    if (!U_JUMPFAIL(vm, regs + 3)) goto block9389;
    U_STATEMENT(vm, regs + 2, 95, 5);
    U_POP(vm, regs + 2);
    U_POP(vm, regs + 1);
    regs[0] = locals[3]; // i
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    block9389:;
    goto block9335;
    block9394:;
    U_STATEMENT(vm, regs + 0, 96, 5);
    U_PUSHINT(vm, regs + 0, -1);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// find
static void fun_23(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[4];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 822, locals);
    U_STATEMENT(vm, regs + 0, 93, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block9429:;
    if (!U_VFOR(vm, regs + 2)) goto block9488;
    U_STATEMENT(vm, regs + 2, 93, 5);
    U_VFORELEMREF(vm, regs + 2);
    SetLVal(vm, &locals[2]); // x
    U_LV_WRITEREF(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 93, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[3]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 94, 5);
    regs[2] = locals[2]; // x
    fun_277(vm, regs + 3); // call: function257
    if (!U_JUMPFAIL(vm, regs + 3)) goto block9483;
    U_STATEMENT(vm, regs + 2, 95, 5);
    U_POP(vm, regs + 2);
    U_POP(vm, regs + 1);
    regs[0] = locals[3]; // i
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[2]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    block9483:;
    goto block9429;
    block9488:;
    U_STATEMENT(vm, regs + 0, 96, 5);
    U_PUSHINT(vm, regs + 0, -1);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[2]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// find_best
static void fun_26(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[8];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    locals[5] = lobster::NilVal();
    locals[6] = lobster::NilVal();
    locals[7] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 831, locals);
    U_STATEMENT(vm, regs + 0, 115, 5);
    U_PUSHINT(vm, regs + 0, 0);
    SetLVal(vm, &locals[2]); // init
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 116, 5);
    regs[0] = locals[2]; // init
    SetLVal(vm, &locals[3]); // best
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 117, 5);
    U_PUSHINT(vm, regs + 0, -1);
    SetLVal(vm, &locals[4]); // i
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 118, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block9563:;
    if (!U_VFOR(vm, regs + 2)) goto block9664;
    U_STATEMENT(vm, regs + 2, 118, 5);
    U_VFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[5]); // x
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 118, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[6]); // j
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 119, 5);
    regs[2] = locals[5]; // x
    fun_310(vm, regs + 3); // call: function280
    SetLVal(vm, &locals[7]); // v
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 120, 5);
    regs[2] = locals[4]; // i
    U_PUSHINT(vm, regs + 3, 0);
    U_ILT(vm, regs + 4);
    if (!U_JUMPNOFAILR(vm, regs + 3)) goto block9630;
    U_POP(vm, regs + 3);
    regs[2] = locals[7]; // v
    regs[3] = locals[3]; // best
    U_IGT(vm, regs + 4);
    block9630:;
    if (!U_JUMPFAIL(vm, regs + 3)) goto block9659;
    U_STATEMENT(vm, regs + 2, 121, 5);
    regs[2] = locals[6]; // j
    SetLVal(vm, &locals[4]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 122, 5);
    regs[2] = locals[7]; // v
    SetLVal(vm, &locals[3]); // best
    U_LV_WRITE(vm, regs + 3);
    block9659:;
    goto block9563;
    block9664:;
    U_STATEMENT(vm, regs + 0, 123, 5);
    regs[0] = locals[4]; // i
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// sum
static void fun_469(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    locals[0] = *(psp - 1);
    locals[1] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 844, locals);
    U_STATEMENT(vm, regs + 0, 140, 5);
    U_PUSHINT(vm, regs + 0, 0);
    SetLVal(vm, &locals[1]); // init
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 141, 5);
    regs[0] = locals[0]; // xs
    regs[1] = locals[1]; // init
    U_PUSHFUN(vm, regs + 2, 0, fun_30);
    fun_17(vm, regs + 3); // call: fold
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// sum
static void fun_29(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    locals[0] = *(psp - 1);
    locals[1] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 851, locals);
    U_STATEMENT(vm, regs + 0, 140, 5);
    U_PUSHINT(vm, regs + 0, 0);
    SetLVal(vm, &locals[1]); // init
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 141, 5);
    regs[0] = locals[0]; // xs
    regs[1] = locals[1]; // init
    U_PUSHFUN(vm, regs + 2, 0, fun_30);
    fun_17(vm, regs + 3); // call: fold
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function30
static void fun_30(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 858, locals);
    U_STATEMENT(vm, regs + 0, 141, 5);
    regs[0] = locals[0]; // _x
    regs[1] = locals[1]; // _y
    U_IADD(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// qsort
static void fun_591(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value keepvar[4];
    Value locals[4];
    locals[0] = *(psp - 2);
    SwapVars(vm, 1549, psp, 1);
    BackupVar(vm, 1550);
    locals[1] = lobster::NilVal();
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 865, locals);
    keepvar[0] = lobster::NilVal();
    keepvar[1] = lobster::NilVal();
    keepvar[2] = lobster::NilVal();
    keepvar[3] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 194, 5);
    regs[0] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 1, 13, 1); // length
    U_PUSHINT(vm, regs + 1, 1);
    U_IGT(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block9951;
    U_STATEMENT(vm, regs + 0, 196, 5);
    regs[0] = locals[0]; // xs
    U_PUSHINT(vm, regs + 1, 0);
    U_VPUSHIDXI(vm, regs + 2);
    U_LVAL_VARF(vm, regs + 1, 1550); // pivot
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 197, 5);
    regs[0] = locals[0]; // xs
    U_PUSHINT(vm, regs + 1, 1);
    U_PUSHINT(vm, regs + 2, -1);
    U_BCALLRET3(vm, regs + 3, 31, 1); // slice
    SetLVal(vm, &locals[1]); // tail
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 198, 5);
    regs[0] = locals[1]; // tail
    U_INCREF(vm, regs + 1, 0);
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_592(vm, regs + 2); // call: partition
    SetLVal(vm, &locals[3]); // f2
    U_LV_WRITEREF(vm, regs + 2);
    SetLVal(vm, &locals[2]); // f1
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 200, 5);
    regs[0] = locals[2]; // f1
    U_INCREF(vm, regs + 1, 0);
    U_PUSHVARF(vm, regs + 1, 1549); // lt
    fun_591(vm, regs + 2); // call: qsort
    keepvar[0] = TopM(regs + 1, 0);
    U_PUSHVARF(vm, regs + 1, 1550); // pivot
    U_NEWVEC(vm, regs + 2, 7, 1);
    keepvar[1] = TopM(regs + 2, 0);
    U_BCALLRET2(vm, regs + 2, 8, 1); // append
    keepvar[2] = TopM(regs + 1, 0);
    regs[1] = locals[3]; // f2
    U_INCREF(vm, regs + 2, 0);
    U_PUSHVARF(vm, regs + 2, 1549); // lt
    fun_591(vm, regs + 3); // call: qsort
    keepvar[3] = TopM(regs + 2, 0);
    U_BCALLRET2(vm, regs + 2, 8, 1); // append
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[2]);
    DecVal(vm, locals[3]);
    psp = PopArg(vm, 1549, psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    goto block9966;
    block9951:;
    U_STATEMENT(vm, regs + 0, 195, 5);
    regs[0] = locals[0]; // xs
    U_INCREF(vm, regs + 1, 0);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[2]);
    DecVal(vm, locals[3]);
    psp = PopArg(vm, 1549, psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    block9966:;
    U_RETURNLOCAL(vm, 0, 0);
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
static void fun_585(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value keepvar[4];
    Value locals[4];
    locals[0] = *(psp - 2);
    SwapVars(vm, 1519, psp, 1);
    BackupVar(vm, 1520);
    BackupVar(vm, 1521);
    BackupVar(vm, 1522);
    locals[1] = lobster::NilVal();
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 876, locals);
    keepvar[0] = lobster::NilVal();
    keepvar[1] = lobster::NilVal();
    keepvar[2] = lobster::NilVal();
    keepvar[3] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 194, 5);
    regs[0] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 1, 13, 1); // length
    U_PUSHINT(vm, regs + 1, 1);
    U_IGT(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block10142;
    U_STATEMENT(vm, regs + 0, 196, 5);
    regs[0] = locals[0]; // xs
    U_PUSHINT(vm, regs + 1, 0);
    U_VPUSHIDXI2V(vm, regs + 2);
    U_LVAL_VARF(vm, regs + 3, 1520); // pivot+0
    U_LV_WRITEV(vm, regs + 3, 3);
    U_STATEMENT(vm, regs + 0, 197, 5);
    regs[0] = locals[0]; // xs
    U_PUSHINT(vm, regs + 1, 1);
    U_PUSHINT(vm, regs + 2, -1);
    U_BCALLRET3(vm, regs + 3, 31, 1); // slice
    SetLVal(vm, &locals[1]); // tail
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 198, 5);
    regs[0] = locals[1]; // tail
    U_INCREF(vm, regs + 1, 0);
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_587(vm, regs + 2); // call: partition
    SetLVal(vm, &locals[3]); // f2
    U_LV_WRITEREF(vm, regs + 2);
    SetLVal(vm, &locals[2]); // f1
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 200, 5);
    regs[0] = locals[2]; // f1
    U_INCREF(vm, regs + 1, 0);
    U_PUSHVARF(vm, regs + 1, 1519); // lt
    fun_585(vm, regs + 2); // call: qsort
    keepvar[0] = TopM(regs + 1, 0);
    U_PUSHVARVF(vm, regs + 1, 1520, 3); // pivot+0
    U_NEWVEC(vm, regs + 4, 1165, 1);
    keepvar[1] = TopM(regs + 2, 0);
    U_BCALLRET2(vm, regs + 2, 8, 1); // append
    keepvar[2] = TopM(regs + 1, 0);
    regs[1] = locals[3]; // f2
    U_INCREF(vm, regs + 2, 0);
    U_PUSHVARF(vm, regs + 2, 1519); // lt
    fun_585(vm, regs + 3); // call: qsort
    keepvar[3] = TopM(regs + 2, 0);
    U_BCALLRET2(vm, regs + 2, 8, 1); // append
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[2]);
    DecVal(vm, locals[3]);
    psp = PopArg(vm, 1519, psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    goto block10157;
    block10142:;
    U_STATEMENT(vm, regs + 0, 195, 5);
    regs[0] = locals[0]; // xs
    U_INCREF(vm, regs + 1, 0);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[2]);
    DecVal(vm, locals[3]);
    psp = PopArg(vm, 1519, psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    block10157:;
    U_RETURNLOCAL(vm, 0, 0);
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
    locals[0] = *(psp - 2);
    SwapVars(vm, 53, psp, 1);
    BackupVar(vm, 54);
    locals[1] = lobster::NilVal();
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 889, locals);
    keepvar[0] = lobster::NilVal();
    keepvar[1] = lobster::NilVal();
    keepvar[2] = lobster::NilVal();
    keepvar[3] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 194, 5);
    regs[0] = locals[0]; // xs
    U_BCALLRET1(vm, regs + 1, 13, 1); // length
    U_PUSHINT(vm, regs + 1, 1);
    U_IGT(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block10329;
    U_STATEMENT(vm, regs + 0, 196, 5);
    regs[0] = locals[0]; // xs
    U_PUSHINT(vm, regs + 1, 0);
    U_VPUSHIDXI(vm, regs + 2);
    U_LVAL_VARF(vm, regs + 1, 54); // pivot
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 197, 5);
    regs[0] = locals[0]; // xs
    U_PUSHINT(vm, regs + 1, 1);
    U_PUSHINT(vm, regs + 2, -1);
    U_BCALLRET3(vm, regs + 3, 31, 1); // slice
    SetLVal(vm, &locals[1]); // tail
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 198, 5);
    regs[0] = locals[1]; // tail
    U_INCREF(vm, regs + 1, 0);
    U_PUSHFUN(vm, regs + 1, 0, fun_49);
    fun_13(vm, regs + 2); // call: partition
    SetLVal(vm, &locals[3]); // f2
    U_LV_WRITEREF(vm, regs + 2);
    SetLVal(vm, &locals[2]); // f1
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 200, 5);
    regs[0] = locals[2]; // f1
    U_INCREF(vm, regs + 1, 0);
    U_PUSHVARF(vm, regs + 1, 53); // lt
    fun_48(vm, regs + 2); // call: qsort
    keepvar[0] = TopM(regs + 1, 0);
    U_PUSHVARF(vm, regs + 1, 54); // pivot
    U_NEWVEC(vm, regs + 2, 7, 1);
    keepvar[1] = TopM(regs + 2, 0);
    U_BCALLRET2(vm, regs + 2, 8, 1); // append
    keepvar[2] = TopM(regs + 1, 0);
    regs[1] = locals[3]; // f2
    U_INCREF(vm, regs + 2, 0);
    U_PUSHVARF(vm, regs + 2, 53); // lt
    fun_48(vm, regs + 3); // call: qsort
    keepvar[3] = TopM(regs + 2, 0);
    U_BCALLRET2(vm, regs + 2, 8, 1); // append
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[2]);
    DecVal(vm, locals[3]);
    psp = PopArg(vm, 53, psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    goto block10344;
    block10329:;
    U_STATEMENT(vm, regs + 0, 195, 5);
    regs[0] = locals[0]; // xs
    U_INCREF(vm, regs + 1, 0);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[2]);
    DecVal(vm, locals[3]);
    psp = PopArg(vm, 53, psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    block10344:;
    U_RETURNLOCAL(vm, 0, 0);
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
static void fun_593(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 900, locals);
    U_STATEMENT(vm, regs + 0, 198, 5);
    regs[0] = locals[0]; // _
    U_PUSHVARF(vm, regs + 1, 1550); // pivot
    fun_359(vm, regs + 2); // call: function326
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function49
static void fun_588(VMRef vm, StackPtr psp) {
    Value regs[6];
    Value locals[3];
    locals[0] = *(psp - 3);
    locals[1] = *(psp - 2);
    locals[2] = *(psp - 1);
    PushFunId(vm, funinfo_table + 906, locals);
    U_STATEMENT(vm, regs + 0, 198, 5);
    regs[0] = locals[0];regs[1] = locals[1];regs[2] = locals[2]; // _
    U_PUSHVARVF(vm, regs + 3, 1520, 3); // pivot+0
    fun_356(vm, regs + 6); // call: function323
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function49
static void fun_49(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 914, locals);
    U_STATEMENT(vm, regs + 0, 198, 5);
    regs[0] = locals[0]; // _
    U_PUSHVARF(vm, regs + 1, 54); // pivot
    fun_161(vm, regs + 2); // call: function154
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// qsort_in_place
static void fun_50(VMRef vm, StackPtr psp) {
    Value regs[2];
    SwapVars(vm, 59, psp, 2);
    SwapVars(vm, 60, psp, 1);
    PushFunId(vm, funinfo_table + 920, 0);
    U_STATEMENT(vm, regs + 0, 203, 5);
    U_STATEMENT(vm, regs + 0, 220, 5);
    U_PUSHINT(vm, regs + 0, 0);
    U_PUSHVARF(vm, regs + 1, 59); // xs
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    fun_51(vm, regs + 2); // call: rec
    U_RETURNLOCAL(vm, 0, 0);
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
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    locals[5] = lobster::NilVal();
    locals[6] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 927, locals);
    U_STATEMENT(vm, regs + 0, 204, 5);
    regs[0] = locals[1]; // e
    regs[1] = locals[0]; // s
    U_ISUB(vm, regs + 2);
    SetLVal(vm, &locals[2]); // l
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 205, 5);
    regs[0] = locals[2]; // l
    U_PUSHINT(vm, regs + 1, 1);
    U_IGT(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block10747;
    U_STATEMENT(vm, regs + 0, 206, 5);
    U_PUSHVARF(vm, regs + 0, 59); // xs
    regs[1] = locals[0]; // s
    U_VPUSHIDXI(vm, regs + 2);
    SetLVal(vm, &locals[3]); // pivot
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 207, 5);
    regs[0] = locals[0]; // s
    U_PUSHINT(vm, regs + 1, 1);
    U_IADD(vm, regs + 2);
    SetLVal(vm, &locals[4]); // sp
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 208, 5);
    regs[0] = locals[1]; // e
    SetLVal(vm, &locals[5]); // ep
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 209, 5);
    block10559:;
    regs[0] = locals[4]; // sp
    regs[1] = locals[5]; // ep
    U_ILT(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block10697;
    U_STATEMENT(vm, regs + 0, 210, 5);
    U_PUSHVARF(vm, regs + 0, 59); // xs
    regs[1] = locals[4]; // sp
    U_VPUSHIDXI(vm, regs + 2);
    SetLVal(vm, &locals[6]); // c
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 211, 5);
    regs[0] = locals[6]; // c
    regs[1] = locals[3]; // pivot
    fun_163(vm, regs + 2); // call: function156
    if (!U_JUMPFAIL(vm, regs + 1)) goto block10645;
    U_STATEMENT(vm, regs + 0, 212, 5);
    U_PUSHVARF(vm, regs + 0, 59); // xs
    regs[1] = locals[4]; // sp
    U_VPUSHIDXI(vm, regs + 2);
    U_PUSHVARF(vm, regs + 1, 59); // xs
    regs[2] = locals[4]; // sp
    U_PUSHINT(vm, regs + 3, 1);
    U_ISUB(vm, regs + 4);
    U_LVAL_IDXVI(vm, regs + 3, 0);
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 213, 5);
    SetLVal(vm, &locals[4]); // sp
    U_LV_IPP(vm, regs + 0);
    goto block10692;
    block10645:;
    U_STATEMENT(vm, regs + 0, 215, 5);
    U_PUSHVARF(vm, regs + 0, 59); // xs
    SetLVal(vm, &locals[5]); // ep
    U_LV_IMM(vm, regs + 1);
    U_LV_DUP(vm, regs + 1);
    U_VPUSHIDXI(vm, regs + 2);
    U_PUSHVARF(vm, regs + 1, 59); // xs
    regs[2] = locals[4]; // sp
    U_LVAL_IDXVI(vm, regs + 3, 0);
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 216, 5);
    regs[0] = locals[6]; // c
    U_PUSHVARF(vm, regs + 1, 59); // xs
    regs[2] = locals[5]; // ep
    U_LVAL_IDXVI(vm, regs + 3, 0);
    U_LV_WRITE(vm, regs + 1);
    block10692:;
    goto block10559;
    block10697:;
    U_STATEMENT(vm, regs + 0, 217, 5);
    regs[0] = locals[3]; // pivot
    U_PUSHVARF(vm, regs + 1, 59); // xs
    SetLVal(vm, &locals[4]); // sp
    U_LV_IMM(vm, regs + 2);
    U_LV_DUP(vm, regs + 2);
    U_LVAL_IDXVI(vm, regs + 3, 0);
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 218, 5);
    regs[0] = locals[0]; // s
    regs[1] = locals[4]; // sp
    fun_51(vm, regs + 2); // call: rec
    U_STATEMENT(vm, regs + 0, 219, 5);
    regs[0] = locals[5]; // ep
    regs[1] = locals[1]; // e
    fun_51(vm, regs + 2); // call: rec
    block10747:;
    U_RETURNLOCAL(vm, 0, 0);
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
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 939, locals);
    U_STATEMENT(vm, regs + 0, 223, 5);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // xs
    block10776:;
    if (!U_VFOR(vm, regs + 2)) goto block10928;
    U_STATEMENT(vm, regs + 2, 223, 5);
    U_VFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[2]); // key
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 223, 5);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[3]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 224, 5);
    regs[2] = locals[3]; // i
    if (!U_JUMPFAIL(vm, regs + 3)) goto block10923;
    U_STATEMENT(vm, regs + 2, 225, 5);
    regs[2] = locals[3]; // i
    SetLVal(vm, &locals[4]); // j
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 226, 5);
    block10829:;
    regs[2] = locals[4]; // j
    U_PUSHINT(vm, regs + 3, 0);
    U_IGT(vm, regs + 4);
    if (!U_JUMPFAILR(vm, regs + 3)) goto block10863;
    U_POP(vm, regs + 3);
    regs[2] = locals[2]; // key
    regs[3] = locals[0]; // xs
    regs[4] = locals[4]; // j
    U_PUSHINT(vm, regs + 5, 1);
    U_ISUB(vm, regs + 6);
    U_VPUSHIDXI(vm, regs + 5);
    fun_162(vm, regs + 4); // call: function155
    block10863:;
    if (!U_JUMPFAIL(vm, regs + 3)) goto block10903;
    U_STATEMENT(vm, regs + 2, 227, 5);
    regs[2] = locals[0]; // xs
    regs[3] = locals[4]; // j
    U_PUSHINT(vm, regs + 4, 1);
    U_ISUB(vm, regs + 5);
    U_VPUSHIDXI(vm, regs + 4);
    regs[3] = locals[0]; // xs
    SetLVal(vm, &locals[4]); // j
    U_LV_DUP(vm, regs + 4);
    U_LV_IMM(vm, regs + 5);
    U_LVAL_IDXVI(vm, regs + 5, 0);
    U_LV_WRITE(vm, regs + 3);
    goto block10829;
    block10903:;
    U_STATEMENT(vm, regs + 2, 228, 5);
    regs[2] = locals[2]; // key
    regs[3] = locals[0]; // xs
    regs[4] = locals[4]; // j
    U_LVAL_IDXVI(vm, regs + 5, 0);
    U_LV_WRITE(vm, regs + 3);
    block10923:;
    goto block10776;
    block10928:;
    U_RETURNLOCAL(vm, 0, 0);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// do
static void fun_473(VMRef vm, StackPtr psp) {
    Value regs[1];
    (void)regs;
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 949, locals);
    U_STATEMENT(vm, regs + 0, 253, 5);
    fun_175(vm, regs + 0); // call: function168
    if (!U_JUMPIFUNWOUND(vm, regs + 0, 168)) goto block10958;
    U_RETURNANY(vm, 0, 0);
    Pop(psp);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block10958:;
    U_RETURNLOCAL(vm, 0, 0);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// xz
static void fun_70(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[4];
    locals[0] = *(psp - 4);
    locals[1] = *(psp - 3);
    locals[2] = *(psp - 2);
    locals[3] = *(psp - 1);
    PushFunId(vm, funinfo_table + 955, locals);
    U_STATEMENT(vm, regs + 0, 64, 4);
    regs[0] = locals[0]; // v+0
    regs[1] = locals[2]; // v+2
    U_RETURNLOCAL(vm, 0, 2);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// yw
static void fun_71(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[4];
    locals[0] = *(psp - 4);
    locals[1] = *(psp - 3);
    locals[2] = *(psp - 2);
    locals[3] = *(psp - 1);
    PushFunId(vm, funinfo_table + 964, locals);
    U_STATEMENT(vm, regs + 0, 65, 4);
    regs[0] = locals[1]; // v+1
    regs[1] = locals[3]; // v+3
    U_RETURNLOCAL(vm, 0, 2);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// zw
static void fun_72(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[4];
    locals[0] = *(psp - 4);
    locals[1] = *(psp - 3);
    locals[2] = *(psp - 2);
    locals[3] = *(psp - 1);
    PushFunId(vm, funinfo_table + 973, locals);
    U_STATEMENT(vm, regs + 0, 66, 4);
    regs[0] = locals[2]; // v+2
    regs[1] = locals[3]; // v+3
    U_RETURNLOCAL(vm, 0, 2);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// vec4
static void fun_79(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[4];
    locals[0] = *(psp - 4);
    locals[1] = *(psp - 3);
    locals[2] = *(psp - 2);
    locals[3] = *(psp - 1);
    PushFunId(vm, funinfo_table + 982, locals);
    U_STATEMENT(vm, regs + 0, 76, 4);
    regs[0] = locals[0]; // v+0
    regs[1] = locals[1]; // v+1
    regs[2] = locals[2]; // v+2
    regs[3] = locals[3]; // w
    U_RETURNLOCAL(vm, 0, 4);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    Push(psp, regs[2]);
    Push(psp, regs[3]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// rnd_float2
static void fun_89(VMRef vm, StackPtr psp) {
    Value regs[2];
    PushFunId(vm, funinfo_table + 991, 0);
    U_STATEMENT(vm, regs + 0, 92, 4);
    U_BCALLRET0(vm, regs + 0, 95, 1); // rnd_float
    U_BCALLRET0(vm, regs + 1, 95, 1); // rnd_float
    U_RETURNLOCAL(vm, 0, 2);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// rnd_int2
static void fun_95(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 996, locals);
    U_STATEMENT(vm, regs + 0, 100, 4);
    regs[0] = locals[0]; // n+0
    U_BCALLRET1(vm, regs + 1, 93, 1); // rnd
    regs[1] = locals[1]; // n+1
    U_BCALLRET1(vm, regs + 2, 93, 1); // rnd
    U_RETURNLOCAL(vm, 0, 2);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// forxy
static void fun_98(VMRef vm, StackPtr psp) {
    Value regs[6];
    Value locals[5];
    locals[0] = *(psp - 3);
    locals[1] = *(psp - 2);
    locals[2] = *(psp - 1);
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1003, locals);
    U_STATEMENT(vm, regs + 0, 104, 4);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[1]; // v+1
    block11146:;
    if (!U_IFOR(vm, regs + 2)) goto block11209;
    U_STATEMENT(vm, regs + 2, 104, 4);
    U_IFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[3]); // y
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 104, 4);
    U_PUSHINT(vm, regs + 2, -1);
    regs[3] = locals[0]; // v+0
    block11172:;
    if (!U_IFOR(vm, regs + 4)) goto block11204;
    U_STATEMENT(vm, regs + 4, 104, 4);
    U_IFORELEM(vm, regs + 4);
    SetLVal(vm, &locals[4]); // x
    U_LV_WRITE(vm, regs + 5);
    U_STATEMENT(vm, regs + 4, 104, 4);
    regs[4] = locals[4]; // x
    regs[5] = locals[3]; // y
    fun_355(vm, regs + 6); // call: function322
    goto block11172;
    block11204:;
    goto block11146;
    block11209:;
    U_RETURNLOCAL(vm, 0, 0);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// mapxy
static void fun_571(VMRef vm, StackPtr psp) {
    Value regs[2];
    SwapVars(vm, 1469, psp, 3);
    SwapVars(vm, 1470, psp, 2);
    SwapVars(vm, 1471, psp, 1);
    PushFunId(vm, funinfo_table + 1013, 0);
    U_STATEMENT(vm, regs + 0, 106, 4);
    U_PUSHVARF(vm, regs + 0, 1470); // v+1
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_573(vm, regs + 2); // call: map
    U_RETURNLOCAL(vm, 0, 1);
    psp = PopArg(vm, 1471, psp);
    psp = PopArg(vm, 1470, psp);
    psp = PopArg(vm, 1469, psp);
    Push(psp, regs[0]);
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
    PushFunId(vm, funinfo_table + 1021, 0);
    U_STATEMENT(vm, regs + 0, 106, 4);
    U_PUSHVARF(vm, regs + 0, 198); // v+1
    U_PUSHFUN(vm, regs + 1, 0, fun_101);
    fun_560(vm, regs + 2); // call: map
    U_RETURNLOCAL(vm, 0, 1);
    psp = PopArg(vm, 199, psp);
    psp = PopArg(vm, 198, psp);
    psp = PopArg(vm, 197, psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function98
static void fun_574(VMRef vm, StackPtr psp) {
    Value regs[2];
    SwapVars(vm, 1477, psp, 1);
    PushFunId(vm, funinfo_table + 1029, 0);
    U_STATEMENT(vm, regs + 0, 106, 4);
    U_PUSHVARF(vm, regs + 0, 1469); // v+0
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_576(vm, regs + 2); // call: map
    U_RETURNLOCAL(vm, 0, 1);
    psp = PopArg(vm, 1477, psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function98
static void fun_101(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1035, locals);
    U_STATEMENT(vm, regs + 0, 106, 4);
    U_PUSHVARF(vm, regs + 0, 197); // v+0
    U_PUSHFUN(vm, regs + 1, 0, fun_102);
    fun_561(vm, regs + 2); // call: map
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function99
static void fun_577(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1041, locals);
    U_STATEMENT(vm, regs + 0, 106, 4);
    regs[0] = locals[0]; // x
    U_PUSHVARF(vm, regs + 1, 1477); // y
    fun_332(vm, regs + 2); // call: function299
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function99
static void fun_102(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1047, locals);
    U_STATEMENT(vm, regs + 0, 106, 4);
    U_STATEMENT(vm, regs + 0, 10, 13);
    U_PUSHINT(vm, regs + 0, -1);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function107
static void fun_110(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[10];
    locals[0] = lobster::NilVal();
    locals[1] = lobster::NilVal();
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    locals[5] = lobster::NilVal();
    locals[6] = lobster::NilVal();
    locals[7] = lobster::NilVal();
    locals[8] = lobster::NilVal();
    locals[9] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1053, locals);
    U_STATEMENT(vm, regs + 0, 7, 3);
    U_STATEMENT(vm, regs + 0, 7, 3);
    U_PUSHFUN(vm, regs + 0, 0, fun_111);
    SetLVal(vm, &locals[0]); // f
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 253, 5);
    fun_111(vm, regs + 0); // call: function108
    U_STATEMENT(vm, regs + 0, 67, 3);
    U_STATEMENT(vm, regs + 0, 67, 3);
    U_PUSHFUN(vm, regs + 0, 0, fun_112);
    SetLVal(vm, &locals[1]); // f
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 253, 5);
    fun_112(vm, regs + 0); // call: function109
    U_STATEMENT(vm, regs + 0, 97, 3);
    U_STATEMENT(vm, regs + 0, 97, 3);
    U_PUSHFUN(vm, regs + 0, 0, fun_113);
    SetLVal(vm, &locals[2]); // f
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 253, 5);
    fun_113(vm, regs + 0); // call: function110
    U_STATEMENT(vm, regs + 0, 108, 3);
    U_STATEMENT(vm, regs + 0, 108, 3);
    U_PUSHFUN(vm, regs + 0, 0, fun_114);
    SetLVal(vm, &locals[3]); // f
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 253, 5);
    fun_114(vm, regs + 0); // call: function111
    U_STATEMENT(vm, regs + 0, 117, 3);
    U_STATEMENT(vm, regs + 0, 117, 3);
    U_PUSHFUN(vm, regs + 0, 0, fun_117);
    SetLVal(vm, &locals[4]); // f
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 253, 5);
    fun_117(vm, regs + 0); // call: function114
    U_STATEMENT(vm, regs + 0, 128, 3);
    U_STATEMENT(vm, regs + 0, 128, 3);
    U_PUSHFUN(vm, regs + 0, 0, fun_120);
    SetLVal(vm, &locals[5]); // f
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 253, 5);
    fun_120(vm, regs + 0); // call: function116
    U_STATEMENT(vm, regs + 0, 151, 3);
    U_STATEMENT(vm, regs + 0, 151, 3);
    U_PUSHFUN(vm, regs + 0, 0, fun_123);
    SetLVal(vm, &locals[6]); // f
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 253, 5);
    fun_123(vm, regs + 0); // call: function119
    U_STATEMENT(vm, regs + 0, 162, 3);
    U_STATEMENT(vm, regs + 0, 162, 3);
    U_PUSHFUN(vm, regs + 0, 0, fun_125);
    SetLVal(vm, &locals[7]); // f
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 253, 5);
    fun_125(vm, regs + 0); // call: function121
    U_STATEMENT(vm, regs + 0, 183, 3);
    U_STATEMENT(vm, regs + 0, 183, 3);
    U_PUSHFUN(vm, regs + 0, 0, fun_126);
    SetLVal(vm, &locals[8]); // f
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 253, 5);
    fun_126(vm, regs + 0); // call: function122
    U_STATEMENT(vm, regs + 0, 272, 3);
    U_STATEMENT(vm, regs + 0, 272, 3);
    U_PUSHFUN(vm, regs + 0, 0, fun_127);
    SetLVal(vm, &locals[9]); // f
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 253, 5);
    fun_127(vm, regs + 0); // call: function123
    U_RETURNLOCAL(vm, 0, 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function108
static void fun_111(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value locals[14];
    locals[0] = lobster::NilVal();
    locals[1] = lobster::NilVal();
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    locals[5] = lobster::NilVal();
    locals[6] = lobster::NilVal();
    locals[7] = lobster::NilVal();
    locals[8] = lobster::NilVal();
    locals[9] = lobster::NilVal();
    locals[10] = lobster::NilVal();
    locals[11] = lobster::NilVal();
    locals[12] = lobster::NilVal();
    locals[13] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1068, locals);
    U_STATEMENT(vm, regs + 0, 8, 3);
    U_STATEMENT(vm, regs + 0, 8, 3);
    U_STATEMENT(vm, regs + 0, 11, 3);
    U_STATEMENT(vm, regs + 0, 11, 3);
    U_STATEMENT(vm, regs + 0, 14, 3);
    U_STATEMENT(vm, regs + 0, 14, 3);
    U_STATEMENT(vm, regs + 0, 17, 3);
    U_PUSHINT(vm, regs + 0, 3);
    U_PUSHINT(vm, regs + 1, 4);
    U_PUSHINT(vm, regs + 2, 1);
    U_PUSHINT(vm, regs + 3, 1);
    U_IVVADD(vm, regs + 4, 2);
    U_PUSHINT(vm, regs + 2, 1);
    U_PUSHINT(vm, regs + 3, 1);
    U_IVVSUB(vm, regs + 4, 2);
    SetLVal(vm, &locals[0]); // a+0
    U_LV_WRITEV(vm, regs + 2, 2);
    U_STATEMENT(vm, regs + 0, 18, 3);
    U_PUSHINT(vm, regs + 0, 5);
    U_PUSHINT(vm, regs + 1, 6);
    regs[2] = locals[0]; // a+0
    regs[3] = locals[1]; // a+1
    U_IADD(vm, regs + 4);
    SetLVal(vm, &locals[2]); // b+0
    U_LV_WRITEV(vm, regs + 3, 3);
    U_STATEMENT(vm, regs + 0, 19, 3);
    regs[0] = locals[2]; // b+0
    regs[1] = locals[3]; // b+1
    U_IADD(vm, regs + 2);
    regs[1] = locals[4]; // b+2
    U_IADD(vm, regs + 2);
    U_PUSHINT(vm, regs + 1, 18);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 19, 3, 27);
    U_STATEMENT(vm, regs + 0, 20, 3);
    regs[0] = locals[0];regs[1] = locals[1]; // a
    regs[2] = locals[0];regs[3] = locals[1]; // a
    U_IVVADD(vm, regs + 4, 2);
    SetLVal(vm, &locals[5]); // aa+0
    U_LV_WRITEV(vm, regs + 2, 2);
    U_STATEMENT(vm, regs + 0, 21, 3);
    regs[0] = locals[5]; // aa+0
    regs[1] = locals[6]; // aa+1
    U_IADD(vm, regs + 2);
    U_PUSHINT(vm, regs + 1, 14);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 21, 3, 28);
    U_STATEMENT(vm, regs + 0, 22, 3);
    regs[0] = locals[5];regs[1] = locals[6]; // aa
    U_PUSHINT(vm, regs + 2, 1);
    U_IVSADD(vm, regs + 3, 2);
    U_PUSHINT(vm, regs + 2, 7);
    U_PUSHINT(vm, regs + 3, 9);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 22, 3, 29);
    U_STATEMENT(vm, regs + 0, 23, 3);
    regs[0] = locals[0];regs[1] = locals[1]; // a
    SetLVal(vm, &locals[0]); // a+0
    U_LV_IVVADD(vm, regs + 2, 2);
    U_STATEMENT(vm, regs + 0, 24, 3);
    regs[0] = locals[0];regs[1] = locals[1]; // a
    U_PUSHINT(vm, regs + 2, 6);
    U_PUSHINT(vm, regs + 3, 8);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 24, 3, 30);
    U_STATEMENT(vm, regs + 0, 25, 3);
    U_PUSHINT(vm, regs + 0, 1);
    SetLVal(vm, &locals[5]); // aa+0
    U_LV_IVSADD(vm, regs + 1, 2);
    U_STATEMENT(vm, regs + 0, 26, 3);
    regs[0] = locals[5];regs[1] = locals[6]; // aa
    U_PUSHINT(vm, regs + 2, 7);
    U_PUSHINT(vm, regs + 3, 9);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 26, 3, 31);
    U_STATEMENT(vm, regs + 0, 27, 3);
    U_PUSHINT(vm, regs + 0, 10);
    U_PUSHINT(vm, regs + 1, 11);
    U_PUSHINT(vm, regs + 2, 12);
    U_NEWOBJECT(vm, regs + 3, 291); // C
    SetLVal(vm, &locals[7]); // c
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 28, 3);
    regs[0] = locals[7]; // c
    U_PUSHFLD(vm, regs + 1, 0);
    regs[1] = locals[7]; // c
    U_PUSHFLD(vm, regs + 2, 1);
    U_IADD(vm, regs + 2);
    regs[1] = locals[7]; // c
    U_PUSHFLD(vm, regs + 2, 2);
    U_IADD(vm, regs + 2);
    U_PUSHINT(vm, regs + 1, 33);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 28, 3, 32);
    U_STATEMENT(vm, regs + 0, 29, 3);
    U_PUSHINT(vm, regs + 0, 1);
    regs[1] = locals[7]; // c
    U_LVAL_FLD(vm, regs + 2, 0);
    U_LV_IVSADD(vm, regs + 1, 2);
    U_STATEMENT(vm, regs + 0, 30, 3);
    regs[0] = locals[7]; // c
    U_PUSHFLD2V(vm, regs + 1, 0, 2);
    U_PUSHINT(vm, regs + 2, 11);
    U_PUSHINT(vm, regs + 3, 12);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 30, 3, 33);
    U_STATEMENT(vm, regs + 0, 31, 3);
    U_PUSHINT(vm, regs + 0, 1);
    U_PUSHINT(vm, regs + 1, 2);
    regs[2] = locals[7]; // c
    U_LVAL_FLD(vm, regs + 3, 0);
    U_LV_IVVADD(vm, regs + 2, 2);
    U_STATEMENT(vm, regs + 0, 32, 3);
    regs[0] = locals[7]; // c
    U_PUSHFLD2V(vm, regs + 1, 0, 2);
    U_PUSHINT(vm, regs + 2, 12);
    U_PUSHINT(vm, regs + 3, 14);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 32, 3, 34);
    U_STATEMENT(vm, regs + 0, 33, 3);
    U_PUSHINT(vm, regs + 0, 3);
    U_PUSHINT(vm, regs + 1, 4);
    regs[2] = locals[7]; // c
    U_LVAL_FLD(vm, regs + 3, 0);
    U_LV_WRITEV(vm, regs + 2, 2);
    U_STATEMENT(vm, regs + 0, 34, 3);
    regs[0] = locals[7]; // c
    U_PUSHFLD2V(vm, regs + 1, 0, 2);
    U_PUSHINT(vm, regs + 2, 3);
    U_PUSHINT(vm, regs + 3, 4);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 34, 3, 35);
    U_STATEMENT(vm, regs + 0, 35, 3);
    U_PUSHFLT(vm, regs + 0, 1065353216);
    U_PUSHFLT(vm, regs + 1, 1073741824);
    U_PUSHINT(vm, regs + 2, 2);
    U_BCALLRETV(vm, regs + 3, 62, 1); // int
    U_PUSHINT(vm, regs + 2, 1);
    U_PUSHINT(vm, regs + 3, 2);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 35, 3, 36);
    U_STATEMENT(vm, regs + 0, 36, 3);
    U_PUSHINT(vm, regs + 0, 1);
    U_PUSHINT(vm, regs + 1, 2);
    U_PUSHINT(vm, regs + 2, 2);
    U_BCALLRETV(vm, regs + 3, 123, 1); // min
    U_PUSHINT(vm, regs + 1, 1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 36, 3, 37);
    U_STATEMENT(vm, regs + 0, 37, 3);
    U_PUSHINT(vm, regs + 0, 2);
    U_PUSHINT(vm, regs + 1, 3);
    U_PUSHINT(vm, regs + 2, 2);
    U_BCALLRETV(vm, regs + 3, 89, 1); // manhattan
    U_PUSHINT(vm, regs + 1, 5);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 37, 3, 38);
    U_STATEMENT(vm, regs + 0, 38, 3);
    U_PUSHVARVF(vm, regs + 0, 74, 2); // float2_0+0
    SetLVal(vm, &locals[8]); // nv+0
    U_LV_WRITEV(vm, regs + 2, 2);
    U_STATEMENT(vm, regs + 0, 39, 3);
    U_PUSHFLT(vm, regs + 0, 1123418112);
    U_PUSHFLT(vm, regs + 1, 1139015680);
    U_PUSHINT(vm, regs + 2, 2);
    U_BCALLRETV(vm, regs + 3, 84, 1); // normalize
    SetLVal(vm, &locals[8]); // nv+0
    U_LV_WRITEV(vm, regs + 2, 2);
    U_STATEMENT(vm, regs + 0, 40, 3);
    regs[0] = locals[8];regs[1] = locals[9]; // nv
    U_PUSHINT(vm, regs + 2, 2);
    U_BCALLRETV(vm, regs + 3, 86, 1); // magnitude
    U_PUSHFLT(vm, regs + 1, 1065353216);
    U_FSUB(vm, regs + 2);
    U_BCALLRET1(vm, regs + 1, 112, 1); // abs
    U_PUSHFLT64(vm, regs + 1, -755914244, 1062232653);
    U_FLT(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 40, 3, 39);
    U_STATEMENT(vm, regs + 0, 41, 3);
    U_PUSHVARVF(vm, regs + 0, 74, 2); // float2_0+0
    U_PUSHINT(vm, regs + 2, 2);
    U_BCALLRETV(vm, regs + 3, 84, 1); // normalize
    SetLVal(vm, &locals[8]); // nv+0
    U_LV_WRITEV(vm, regs + 2, 2);
    U_STATEMENT(vm, regs + 0, 42, 3);
    regs[0] = locals[8];regs[1] = locals[9]; // nv
    U_PUSHINT(vm, regs + 2, 2);
    U_BCALLRETV(vm, regs + 3, 86, 1); // magnitude
    U_BCALLRET1(vm, regs + 1, 112, 1); // abs
    U_PUSHFLT64(vm, regs + 1, -755914244, 1062232653);
    U_FLT(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 42, 3, 40);
    U_STATEMENT(vm, regs + 0, 43, 3);
    U_PUSHINT(vm, regs + 0, 1);
    U_PUSHINT(vm, regs + 1, 2);
    U_PUSHINT(vm, regs + 2, 3);
    U_PUSHINT(vm, regs + 3, 4);
    U_NEWVEC(vm, regs + 4, 37, 2);
    SetLVal(vm, &locals[10]); // v
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 44, 3);
    regs[0] = locals[10]; // v
    U_PUSHINT(vm, regs + 1, 1);
    U_VPUSHIDXI2V(vm, regs + 2);
    U_PUSHINT(vm, regs + 2, 3);
    U_PUSHINT(vm, regs + 3, 4);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 44, 3, 41);
    U_STATEMENT(vm, regs + 0, 45, 3);
    U_PUSHINT(vm, regs + 0, 5);
    U_PUSHINT(vm, regs + 1, 6);
    regs[2] = locals[10]; // v
    U_PUSHINT(vm, regs + 3, 1);
    U_LVAL_IDXVI(vm, regs + 4, 0);
    U_LV_WRITEV(vm, regs + 2, 2);
    U_STATEMENT(vm, regs + 0, 46, 3);
    regs[0] = locals[10]; // v
    U_PUSHINT(vm, regs + 1, 1);
    U_VPUSHIDXI2V(vm, regs + 2);
    U_PUSHINT(vm, regs + 2, 5);
    U_PUSHINT(vm, regs + 3, 6);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 46, 3, 42);
    U_STATEMENT(vm, regs + 0, 47, 3);
    regs[0] = locals[10]; // v
    U_PUSHINT(vm, regs + 1, 1);
    U_VPUSHIDXIS2V(vm, regs + 2, 1, 1);
    U_PUSHINT(vm, regs + 1, 6);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 47, 3, 43);
    U_STATEMENT(vm, regs + 0, 48, 3);
    U_PUSHINT(vm, regs + 0, 1);
    U_PUSHINT(vm, regs + 1, 1);
    regs[2] = locals[10]; // v
    U_PUSHINT(vm, regs + 3, 1);
    U_LVAL_IDXVI(vm, regs + 4, 0);
    U_LV_IVVADD(vm, regs + 2, 2);
    U_STATEMENT(vm, regs + 0, 49, 3);
    regs[0] = locals[10]; // v
    U_PUSHINT(vm, regs + 1, 1);
    U_VPUSHIDXI2V(vm, regs + 2);
    U_PUSHINT(vm, regs + 2, 6);
    U_PUSHINT(vm, regs + 3, 7);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 49, 3, 44);
    U_STATEMENT(vm, regs + 0, 50, 3);
    U_PUSHINT(vm, regs + 0, 1);
    regs[1] = locals[10]; // v
    U_PUSHINT(vm, regs + 2, 1);
    U_LVAL_IDXVI(vm, regs + 3, 0);
    U_LV_IVSADD(vm, regs + 1, 2);
    U_STATEMENT(vm, regs + 0, 51, 3);
    regs[0] = locals[10]; // v
    U_PUSHINT(vm, regs + 1, 1);
    U_VPUSHIDXI2V(vm, regs + 2);
    U_PUSHINT(vm, regs + 2, 7);
    U_PUSHINT(vm, regs + 3, 8);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 51, 3, 45);
    U_STATEMENT(vm, regs + 0, 52, 3);
    U_PUSHINT(vm, regs + 0, 123);
    U_NEWVEC(vm, regs + 1, 7, 1);
    U_NEWVEC(vm, regs + 1, 13, 1);
    U_NEWVEC(vm, regs + 1, 306, 1);
    SetLVal(vm, &locals[11]); // tri
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 53, 3);
    regs[0] = locals[11]; // tri
    U_PUSHINT(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 3, 0);
    U_VPUSHIDXV(vm, regs + 4, 3);
    U_PUSHINT(vm, regs + 1, 123);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 53, 3, 46);
    U_STATEMENT(vm, regs + 0, 54, 3);
    U_PUSHINT(vm, regs + 0, 124);
    regs[1] = locals[11]; // tri
    U_PUSHINT(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 3, 0);
    U_PUSHINT(vm, regs + 4, 0);
    U_LVAL_IDXVV(vm, regs + 5, 0, 3);
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 55, 3);
    regs[0] = locals[11]; // tri
    U_PUSHINT(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 3, 0);
    U_VPUSHIDXV(vm, regs + 4, 3);
    U_PUSHINT(vm, regs + 1, 124);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 55, 3, 47);
    U_STATEMENT(vm, regs + 0, 56, 3);
    U_PUSHINT(vm, regs + 0, 10);
    U_PUSHINT(vm, regs + 1, 11);
    U_NEWVEC(vm, regs + 2, 37, 1);
    U_NEWVEC(vm, regs + 1, 308, 1);
    U_NEWVEC(vm, regs + 1, 310, 1);
    SetLVal(vm, &locals[12]); // triv
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 57, 3);
    regs[0] = locals[12]; // triv
    U_PUSHINT(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 3, 0);
    U_VPUSHIDXV(vm, regs + 4, 3);
    U_PUSHINT(vm, regs + 2, 10);
    U_PUSHINT(vm, regs + 3, 11);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 57, 3, 48);
    U_STATEMENT(vm, regs + 0, 58, 3);
    U_PUSHINT(vm, regs + 0, 4);
    regs[1] = locals[12]; // triv
    U_PUSHINT(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 3, 0);
    U_PUSHINT(vm, regs + 4, 0);
    U_LVAL_IDXVV(vm, regs + 5, 0, 3);
    U_LV_IVSADD(vm, regs + 1, 2);
    U_STATEMENT(vm, regs + 0, 59, 3);
    regs[0] = locals[12]; // triv
    U_PUSHINT(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 3, 0);
    U_VPUSHIDXV(vm, regs + 4, 3);
    U_PUSHINT(vm, regs + 2, 14);
    U_PUSHINT(vm, regs + 3, 15);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 59, 3, 49);
    U_STATEMENT(vm, regs + 0, 60, 3);
    U_PUSHINT(vm, regs + 0, 5);
    U_PUSHINT(vm, regs + 1, 6);
    U_PUSHINT(vm, regs + 2, 7);
    U_NEWVEC(vm, regs + 3, 312, 1);
    U_NEWVEC(vm, regs + 1, 314, 1);
    U_NEWVEC(vm, regs + 1, 316, 1);
    SetLVal(vm, &locals[13]); // trivv
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 61, 3);
    regs[0] = locals[13]; // trivv
    U_PUSHINT(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 3, 0);
    U_VPUSHIDXVS(vm, regs + 4, 3, 2, 0);
    U_PUSHINT(vm, regs + 2, 5);
    U_PUSHINT(vm, regs + 3, 6);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 61, 3, 50);
    U_STATEMENT(vm, regs + 0, 62, 3);
    regs[0] = locals[13]; // trivv
    U_PUSHINT(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 3, 0);
    U_VPUSHIDXVS(vm, regs + 4, 3, 1, 1);
    U_PUSHINT(vm, regs + 1, 6);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 62, 3, 51);
    U_STATEMENT(vm, regs + 0, 63, 3);
    regs[0] = locals[13]; // trivv
    U_PUSHINT(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 2, 0);
    U_VPUSHIDXV(vm, regs + 3, 2);
    U_PUSHINT(vm, regs + 1, 0);
    U_VPUSHIDXIS2V(vm, regs + 2, 1, 1);
    U_PUSHINT(vm, regs + 1, 6);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 63, 3, 52);
    U_STATEMENT(vm, regs + 0, 64, 3);
    U_PUSHINT(vm, regs + 0, 5);
    U_PUSHINT(vm, regs + 1, 6);
    U_PUSHINT(vm, regs + 2, 7);
    U_PUSHINT(vm, regs + 3, 1);
    U_NPUSHIDXI(vm, regs + 4, 3);
    U_PUSHINT(vm, regs + 1, 6);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 64, 3, 53);
    U_RETURNLOCAL(vm, 0, 0);
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
    locals[0] = lobster::NilVal();
    locals[1] = lobster::NilVal();
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    locals[5] = lobster::NilVal();
    locals[6] = lobster::NilVal();
    locals[7] = lobster::NilVal();
    locals[8] = lobster::NilVal();
    locals[9] = lobster::NilVal();
    locals[10] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1087, locals);
    U_STATEMENT(vm, regs + 0, 69, 3);
    U_PUSHVARVF(vm, regs + 0, 123, 2); // int2_0+0
    SetLVal(vm, &locals[0]); // a+0
    U_LV_WRITEV(vm, regs + 2, 2);
    U_STATEMENT(vm, regs + 0, 70, 3);
    regs[0] = locals[0];regs[1] = locals[1]; // a
    SetLVal(vm, &locals[2]); // b+0
    U_LV_WRITEV(vm, regs + 2, 2);
    U_STATEMENT(vm, regs + 0, 71, 3);
    U_PUSHINT(vm, regs + 0, 1);
    SetLVal(vm, &locals[1]); // a+1
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 72, 3);
    regs[0] = locals[0];regs[1] = locals[1]; // a
    U_PUSHVARVF(vm, regs + 2, 129, 2); // int2_y+0
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 72, 3, 54);
    U_STATEMENT(vm, regs + 0, 73, 3);
    regs[0] = locals[2];regs[1] = locals[3]; // b
    U_PUSHVARVF(vm, regs + 2, 123, 2); // int2_0+0
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 73, 3, 55);
    U_STATEMENT(vm, regs + 0, 75, 3);
    U_STATEMENT(vm, regs + 0, 75, 3);
    U_STATEMENT(vm, regs + 0, 78, 3);
    U_PUSHVARVF(vm, regs + 0, 134, 3); // int3_1+0
    U_PUSHVARVF(vm, regs + 3, 123, 2); // int2_0+0
    U_NEWOBJECT(vm, regs + 5, 318); // C
    SetLVal(vm, &locals[4]); // c
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 79, 3);
    U_PUSHINT(vm, regs + 0, 1);
    regs[1] = locals[4]; // c
    U_LVAL_FLD(vm, regs + 2, 4);
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 80, 3);
    regs[0] = locals[4]; // c
    U_PUSHFLD2V(vm, regs + 1, 3, 2);
    U_PUSHVARVF(vm, regs + 2, 129, 2); // int2_y+0
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 80, 3, 56);
    U_STATEMENT(vm, regs + 0, 82, 3);
    U_STATEMENT(vm, regs + 0, 82, 3);
    U_STATEMENT(vm, regs + 0, 85, 3);
    U_PUSHVARVF(vm, regs + 0, 134, 3); // int3_1+0
    U_PUSHVARVF(vm, regs + 3, 123, 2); // int2_0+0
    SetLVal(vm, &locals[5]); // s+0
    U_LV_WRITEREFV(vm, regs + 5, 5, 0);
    U_STATEMENT(vm, regs + 0, 86, 3);
    U_PUSHINT(vm, regs + 0, 1);
    SetLVal(vm, &locals[9]); // s+4
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 87, 3);
    regs[0] = locals[8];regs[1] = locals[9]; // s
    U_PUSHVARVF(vm, regs + 2, 129, 2); // int2_y+0
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 87, 3, 57);
    U_STATEMENT(vm, regs + 0, 89, 3);
    U_PUSHVARVF(vm, regs + 0, 127, 2); // int2_x+0
    SetLVal(vm, &locals[8]); // s+3
    U_LV_WRITEV(vm, regs + 2, 2);
    U_STATEMENT(vm, regs + 0, 90, 3);
    regs[0] = locals[8];regs[1] = locals[9]; // s
    U_PUSHVARVF(vm, regs + 2, 127, 2); // int2_x+0
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 90, 3, 58);
    U_STATEMENT(vm, regs + 0, 92, 3);
    U_PUSHVARVF(vm, regs + 0, 123, 2); // int2_0+0
    U_NEWVEC(vm, regs + 2, 37, 1);
    SetLVal(vm, &locals[10]); // v
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 93, 3);
    U_PUSHINT(vm, regs + 0, 1);
    regs[1] = locals[10]; // v
    U_PUSHINT(vm, regs + 2, 0);
    U_LVAL_IDXVI(vm, regs + 3, 1);
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 94, 3);
    regs[0] = locals[10]; // v
    U_PUSHINT(vm, regs + 1, 0);
    U_VPUSHIDXI2V(vm, regs + 2);
    U_PUSHVARVF(vm, regs + 2, 129, 2); // int2_y+0
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 94, 3, 59);
    U_RETURNLOCAL(vm, 0, 0);
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
    locals[0] = lobster::NilVal();
    locals[1] = lobster::NilVal();
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1103, locals);
    keepvar[0] = lobster::NilVal();
    keepvar[1] = lobster::NilVal();
    keepvar[2] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 98, 3);
    U_PUSHINT(vm, regs + 0, 1);
    U_PUSHINT(vm, regs + 1, 1);
    U_PUSHINT(vm, regs + 2, 2);
    U_BCALLRETV(vm, regs + 3, 89, 1); // manhattan
    if (!U_JUMPFAIL(vm, regs + 1)) goto block13261;
    U_STATEMENT(vm, regs + 0, 98, 3);
    U_PUSHINT(vm, regs + 0, 1);
    U_PUSHINT(vm, regs + 1, 2);
    goto block13273;
    block13261:;
    U_STATEMENT(vm, regs + 0, 98, 3);
    U_PUSHINT(vm, regs + 0, 3);
    U_PUSHINT(vm, regs + 1, 4);
    block13273:;
    U_PUSHFLDV(vm, regs + 2, 1, 2);
    U_PUSHINT(vm, regs + 1, 2);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 98, 3, 60);
    U_STATEMENT(vm, regs + 0, 99, 3);
    U_STATEMENT(vm, regs + 0, 99, 3);
    U_PUSHINT(vm, regs + 0, 1);
    U_PUSHINT(vm, regs + 1, 2);
    U_PUSHFLDV(vm, regs + 2, 1, 2);
    U_PUSHINT(vm, regs + 1, 2);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 99, 3, 61);
    U_STATEMENT(vm, regs + 0, 100, 3);
    U_PUSHINT(vm, regs + 0, 1);
    U_PUSHINT(vm, regs + 1, 2);
    U_PUSHINT(vm, regs + 2, 1);
    U_PUSHINT(vm, regs + 3, 1);
    U_PUSHINT(vm, regs + 4, 2);
    U_BCALLRETV(vm, regs + 5, 89, 1); // manhattan
    U_DUP(vm, regs + 3);
    U_PUSHINT(vm, regs + 4, 2);
    U_IEQ(vm, regs + 5);
    if (!U_JUMPFAIL(vm, regs + 4)) goto block13365;
    U_POP(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 101, 3);
    U_PUSHINT(vm, regs + 2, 1);
    U_PUSHINT(vm, regs + 3, 2);
    goto block13379;
    block13365:;
    U_POP(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 102, 3);
    U_PUSHINT(vm, regs + 2, 3);
    U_PUSHINT(vm, regs + 3, 4);
    block13379:;
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 102, 3, 62);
    U_STATEMENT(vm, regs + 0, 103, 3);
    U_PUSHINT(vm, regs + 0, -1);
    U_PUSHINT(vm, regs + 1, 1);
    U_PUSHINT(vm, regs + 2, 2);
    U_PUSHINT(vm, regs + 3, 0);
    U_PUSHINT(vm, regs + 4, 3);
    U_NEWVEC(vm, regs + 5, 37, 2);
    keepvar[0] = TopM(regs + 2, 0);
    block13416:;
    if (!U_VFOR(vm, regs + 2)) goto block13458;
    U_STATEMENT(vm, regs + 2, 103, 3);
    U_VFORELEM2S(vm, regs + 2);
    SetLVal(vm, &locals[0]); // st+0
    U_LV_WRITEV(vm, regs + 4, 2);
    U_STATEMENT(vm, regs + 2, 103, 3);
    regs[2] = locals[0]; // st+0
    regs[3] = locals[1]; // st+1
    U_IADD(vm, regs + 4);
    U_PUSHINT(vm, regs + 3, 3);
    U_IEQ(vm, regs + 4);
    U_ASSERT(vm, regs + 3, 103, 3, 63);
    goto block13416;
    block13458:;
    U_STATEMENT(vm, regs + 0, 104, 3);
    U_STATEMENT(vm, regs + 0, 105, 3);
    U_PUSHINT(vm, regs + 0, -1);
    U_PUSHSTR(vm, regs + 1, 64); // "a"
    U_INCREF(vm, regs + 2, 0);
    U_PUSHSTR(vm, regs + 2, 65); // "bb"
    U_INCREF(vm, regs + 3, 0);
    U_PUSHSTR(vm, regs + 3, 66); // "ab"
    U_INCREF(vm, regs + 4, 0);
    U_PUSHSTR(vm, regs + 4, 67); // "b"
    U_INCREF(vm, regs + 5, 0);
    U_NEWVEC(vm, regs + 5, 1695, 2);
    keepvar[1] = TopM(regs + 2, 0);
    block13503:;
    if (!U_VFOR(vm, regs + 2)) goto block13551;
    U_STATEMENT(vm, regs + 2, 105, 3);
    U_VFORELEMREF2S(vm, regs + 2, 3);
    SetLVal(vm, &locals[2]); // st+0
    U_LV_WRITEREFV(vm, regs + 4, 2, 3);
    U_STATEMENT(vm, regs + 2, 105, 3);
    regs[2] = locals[2]; // st+0
    regs[3] = locals[3]; // st+1
    U_SADD(vm, regs + 4);
    DecVal(vm, keepvar[2]); keepvar[2] = TopM(regs + 3, 0);
    U_PUSHSTR(vm, regs + 3, 68); // "abb"
    U_SEQ(vm, regs + 4);
    U_ASSERT(vm, regs + 3, 105, 3, 69);
    goto block13503;
    block13551:;
    U_RETURNLOCAL(vm, 0, 0);
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
    locals[0] = lobster::NilVal();
    locals[1] = lobster::NilVal();
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1112, locals);
    U_STATEMENT(vm, regs + 0, 109, 3);
    U_STATEMENT(vm, regs + 0, 111, 3);
    U_STATEMENT(vm, regs + 0, 112, 3);
    U_PUSHINT(vm, regs + 0, 1);
    U_PUSHINT(vm, regs + 1, 2);
    U_PUSHINT(vm, regs + 2, 3);
    U_PUSHINT(vm, regs + 3, 4);
    fun_115(vm, regs + 4); // call: f
    U_PUSHINT(vm, regs + 1, 10);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 112, 3, 70);
    U_STATEMENT(vm, regs + 0, 113, 3);
    U_PUSHINT(vm, regs + 0, 7);
    fun_116(vm, regs + 1); // call: g
    SetLVal(vm, &locals[3]); // mr3
    U_LV_WRITE(vm, regs + 4);
    SetLVal(vm, &locals[1]); // mr2+0
    U_LV_WRITEV(vm, regs + 3, 2);
    SetLVal(vm, &locals[0]); // mr1
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 114, 3);
    regs[0] = locals[1];regs[1] = locals[2]; // mr2
    U_PUSHINT(vm, regs + 2, 7);
    U_PUSHINT(vm, regs + 3, 7);
    U_STEQ(vm, regs + 4, 2);
    if (!U_JUMPFAILR(vm, regs + 1)) goto block13661;
    U_POP(vm, regs + 1);
    regs[0] = locals[0]; // mr1
    U_PUSHINT(vm, regs + 1, 1);
    U_IEQ(vm, regs + 2);
    block13661:;
    if (!U_JUMPFAILR(vm, regs + 1)) goto block13676;
    U_POP(vm, regs + 1);
    regs[0] = locals[3]; // mr3
    U_PUSHINT(vm, regs + 1, 1);
    U_IEQ(vm, regs + 2);
    block13676:;
    U_ASSERT(vm, regs + 1, 114, 3, 71);
    U_RETURNLOCAL(vm, 0, 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// f
static void fun_115(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[4];
    locals[0] = *(psp - 4);
    locals[1] = *(psp - 3);
    locals[2] = *(psp - 2);
    locals[3] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1121, locals);
    U_STATEMENT(vm, regs + 0, 110, 3);
    regs[0] = locals[0]; // a
    regs[1] = locals[1]; // b+0
    U_IADD(vm, regs + 2);
    regs[1] = locals[2]; // b+1
    U_IADD(vm, regs + 2);
    regs[1] = locals[3]; // c
    U_IADD(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// g
static void fun_116(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1130, locals);
    U_STATEMENT(vm, regs + 0, 111, 3);
    U_PUSHINT(vm, regs + 0, 1);
    regs[1] = locals[0]; // a
    regs[2] = locals[0]; // a
    U_PUSHINT(vm, regs + 3, 1);
    U_RETURNLOCAL(vm, 0, 4);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    Push(psp, regs[2]);
    Push(psp, regs[3]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function114
static void fun_117(VMRef vm, StackPtr psp) {
    Value regs[3];
    PushFunId(vm, funinfo_table + 1136, 0);
    U_STATEMENT(vm, regs + 0, 118, 3);
    U_STATEMENT(vm, regs + 0, 118, 3);
    U_STATEMENT(vm, regs + 0, 120, 3);
    U_STATEMENT(vm, regs + 0, 120, 3);
    U_STATEMENT(vm, regs + 0, 121, 3);
    U_STATEMENT(vm, regs + 0, 122, 3);
    U_STATEMENT(vm, regs + 0, 123, 3);
    U_PUSHINT(vm, regs + 0, 1);
    U_NEWOBJECT(vm, regs + 1, 372); // mm1
    U_PUSHINT(vm, regs + 1, 4);
    U_PUSHINT(vm, regs + 2, 5);
    U_DDCALL(vm, regs + 3, 0, 2); vm.next_call_target(vm, regs + 3);
    U_PUSHINT(vm, regs + 1, 10);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 123, 3, 72);
    U_STATEMENT(vm, regs + 0, 124, 3);
    U_PUSHINT(vm, regs + 0, 0);
    U_NEWOBJECT(vm, regs + 1, 381); // mm2
    U_PUSHINT(vm, regs + 1, 4);
    U_PUSHINT(vm, regs + 2, 5);
    fun_119(vm, regs + 3); // call: mm
    U_PUSHINT(vm, regs + 1, 9);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 124, 3, 73);
    U_RETURNLOCAL(vm, 0, 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// mm
static void fun_118(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[3];
    locals[0] = *(psp - 3);
    locals[1] = *(psp - 2);
    locals[2] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1141, locals);
    U_STATEMENT(vm, regs + 0, 121, 3);
    regs[0] = locals[0]; // a
    U_PUSHFLD(vm, regs + 1, 0);
    regs[1] = locals[1]; // b+0
    U_IADD(vm, regs + 2);
    regs[1] = locals[2]; // b+1
    U_IADD(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// mm
static void fun_119(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[3];
    locals[0] = *(psp - 3);
    locals[1] = *(psp - 2);
    locals[2] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1149, locals);
    U_STATEMENT(vm, regs + 0, 122, 3);
    regs[0] = locals[0]; // a
    U_PUSHFLD(vm, regs + 1, 0);
    regs[1] = locals[1]; // b+0
    U_IADD(vm, regs + 2);
    regs[1] = locals[2]; // b+1
    U_IADD(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function116
static void fun_120(VMRef vm, StackPtr psp) {
    Value regs[6];
    Value locals[12];
    locals[0] = lobster::NilVal();
    locals[1] = lobster::NilVal();
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    locals[5] = lobster::NilVal();
    locals[6] = lobster::NilVal();
    locals[7] = lobster::NilVal();
    locals[8] = lobster::NilVal();
    locals[9] = lobster::NilVal();
    locals[10] = lobster::NilVal();
    locals[11] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1157, locals);
    U_STATEMENT(vm, regs + 0, 129, 3);
    U_STATEMENT(vm, regs + 0, 129, 3);
    U_STATEMENT(vm, regs + 0, 132, 3);
    U_STATEMENT(vm, regs + 0, 132, 3);
    U_STATEMENT(vm, regs + 0, 135, 3);
    U_PUSHSTR(vm, regs + 0, 74); // "hello"
    U_INCREF(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 1, 123);
    U_NEWVEC(vm, regs + 2, 7, 1);
    SetLVal(vm, &locals[0]); // r+0
    U_LV_WRITEREFV(vm, regs + 2, 2, 3);
    U_STATEMENT(vm, regs + 0, 136, 3);
    U_STATEMENT(vm, regs + 0, 137, 3);
    regs[0] = locals[0];regs[1] = locals[1]; // r
    U_INCREF(vm, regs + 2, 1);
    U_INCREF(vm, regs + 2, 0);
    fun_121(vm, regs + 2); // call: f
    SetLVal(vm, &locals[2]); // fr+0
    U_LV_WRITEREFV(vm, regs + 2, 2, 3);
    U_STATEMENT(vm, regs + 0, 138, 3);
    regs[0] = locals[2]; // fr+0
    U_PUSHSTR(vm, regs + 1, 75); // "hellohello"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 138, 3, 76);
    U_STATEMENT(vm, regs + 0, 139, 3);
    U_PUSHSTR(vm, regs + 0, 77); // "one"
    U_INCREF(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 1, 1);
    U_NEWVEC(vm, regs + 2, 7, 1);
    U_PUSHSTR(vm, regs + 2, 78); // "two"
    U_INCREF(vm, regs + 3, 0);
    U_PUSHINT(vm, regs + 3, 2);
    U_NEWVEC(vm, regs + 4, 7, 1);
    U_NEWVEC(vm, regs + 4, 402, 2);
    SetLVal(vm, &locals[4]); // rs
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 140, 3);
    regs[0] = locals[4]; // rs
    U_PUSHINT(vm, regs + 1, 1);
    U_VPUSHIDXI2V(vm, regs + 2);
    U_INCREF(vm, regs + 2, 1);
    U_INCREF(vm, regs + 2, 0);
    fun_121(vm, regs + 2); // call: f
    SetLVal(vm, &locals[2]); // fr+0
    U_LV_WRITEREFV(vm, regs + 2, 2, 3);
    U_STATEMENT(vm, regs + 0, 141, 3);
    regs[0] = locals[2]; // fr+0
    U_PUSHSTR(vm, regs + 1, 79); // "twotwo"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 141, 3, 80);
    U_STATEMENT(vm, regs + 0, 142, 3);
    U_PUSHSTR(vm, regs + 0, 81); // "A"
    U_INCREF(vm, regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 82); // "B"
    U_INCREF(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 2, 64);
    U_NEWVEC(vm, regs + 3, 7, 1);
    SetLVal(vm, &locals[5]); // rn+0
    U_LV_WRITEREFV(vm, regs + 3, 3, 7);
    U_STATEMENT(vm, regs + 0, 143, 3);
    U_STATEMENT(vm, regs + 0, 144, 3);
    regs[0] = locals[5];regs[1] = locals[6];regs[2] = locals[7]; // rn
    U_INCREF(vm, regs + 3, 2);
    U_INCREF(vm, regs + 3, 1);
    U_INCREF(vm, regs + 3, 0);
    fun_122(vm, regs + 3); // call: fun
    SetLVal(vm, &locals[8]); // frn+0
    U_LV_WRITEREFV(vm, regs + 3, 3, 7);
    U_STATEMENT(vm, regs + 0, 145, 3);
    regs[0] = locals[8]; // frn+0
    U_PUSHSTR(vm, regs + 1, 83); // "AA"
    U_SEQ(vm, regs + 2);
    if (!U_JUMPFAILR(vm, regs + 1)) goto block14207;
    U_POP(vm, regs + 1);
    regs[0] = locals[10]; // frn+2
    U_PUSHINT(vm, regs + 1, 1);
    U_VPUSHIDXI(vm, regs + 2);
    U_PUSHINT(vm, regs + 1, 64);
    U_IEQ(vm, regs + 2);
    block14207:;
    U_ASSERT(vm, regs + 1, 145, 3, 84);
    U_STATEMENT(vm, regs + 0, 146, 3);
    U_PUSHSTR(vm, regs + 0, 85); // "AA"
    U_INCREF(vm, regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 86); // "one"
    U_INCREF(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 2, 1);
    U_NEWVEC(vm, regs + 3, 7, 1);
    U_PUSHSTR(vm, regs + 3, 87); // "BB"
    U_INCREF(vm, regs + 4, 0);
    U_PUSHSTR(vm, regs + 4, 88); // "two"
    U_INCREF(vm, regs + 5, 0);
    U_PUSHINT(vm, regs + 5, 2);
    U_NEWVEC(vm, regs + 6, 7, 1);
    U_NEWVEC(vm, regs + 6, 419, 2);
    SetLVal(vm, &locals[11]); // rns
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 147, 3);
    regs[0] = locals[11]; // rns
    U_PUSHINT(vm, regs + 1, 1);
    U_VPUSHIDXI2V(vm, regs + 2);
    U_INCREF(vm, regs + 3, 2);
    U_INCREF(vm, regs + 3, 1);
    U_INCREF(vm, regs + 3, 0);
    fun_122(vm, regs + 3); // call: fun
    SetLVal(vm, &locals[8]); // frn+0
    U_LV_WRITEREFV(vm, regs + 3, 3, 7);
    U_STATEMENT(vm, regs + 0, 148, 3);
    regs[0] = locals[8]; // frn+0
    U_PUSHSTR(vm, regs + 1, 89); // "BBBB"
    U_SEQ(vm, regs + 2);
    if (!U_JUMPFAILR(vm, regs + 1)) goto block14326;
    U_POP(vm, regs + 1);
    regs[0] = locals[10]; // frn+2
    U_PUSHINT(vm, regs + 1, 1);
    U_VPUSHIDXI(vm, regs + 2);
    U_PUSHINT(vm, regs + 1, 2);
    U_IEQ(vm, regs + 2);
    block14326:;
    U_ASSERT(vm, regs + 1, 148, 3, 90);
    U_RETURNLOCAL(vm, 0, 0);
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
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1174, locals);
    U_STATEMENT(vm, regs + 0, 136, 3);
    regs[0] = locals[0]; // x+0
    regs[1] = locals[0]; // x+0
    U_SADD(vm, regs + 2);
    regs[1] = locals[1]; // x+1
    regs[2] = locals[1]; // x+1
    U_BCALLRET2(vm, regs + 3, 8, 1); // append
    U_RETURNLOCAL(vm, 0, 2);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// fun
static void fun_122(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[3];
    locals[0] = *(psp - 3);
    locals[1] = *(psp - 2);
    locals[2] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1181, locals);
    U_STATEMENT(vm, regs + 0, 143, 3);
    regs[0] = locals[0]; // x+0
    regs[1] = locals[0]; // x+0
    U_SADD(vm, regs + 2);
    regs[1] = locals[1]; // x+1
    regs[2] = locals[1]; // x+1
    U_SADD(vm, regs + 3);
    regs[2] = locals[2]; // x+2
    regs[3] = locals[2]; // x+2
    U_BCALLRET2(vm, regs + 4, 8, 1); // append
    U_RETURNLOCAL(vm, 0, 3);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[2]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    Push(psp, regs[2]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function119
static void fun_123(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value keepvar[1];
    Value locals[2];
    locals[0] = lobster::NilVal();
    locals[1] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1189, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 152, 3);
    U_STATEMENT(vm, regs + 0, 152, 3);
    U_STATEMENT(vm, regs + 0, 155, 3);
    U_PUSHSTR(vm, regs + 0, 91); // "hi"
    U_INCREF(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 1, 1);
    SetLVal(vm, &locals[0]); // rs+0
    U_LV_WRITEREFV(vm, regs + 2, 2, 1);
    U_STATEMENT(vm, regs + 0, 156, 3);
    U_STATEMENT(vm, regs + 0, 157, 3);
    regs[0] = locals[0];regs[1] = locals[1]; // rs
    U_INCREF(vm, regs + 2, 1);
    fun_124(vm, regs + 2); // call: frs
    SetLVal(vm, &locals[0]); // rs+0
    U_LV_WRITEREFV(vm, regs + 2, 2, 1);
    U_STATEMENT(vm, regs + 0, 158, 3);
    regs[0] = locals[0]; // rs+0
    U_PUSHSTR(vm, regs + 1, 92); // "hihi"
    U_SEQ(vm, regs + 2);
    if (!U_JUMPFAILR(vm, regs + 1)) goto block14509;
    U_POP(vm, regs + 1);
    regs[0] = locals[1]; // rs+1
    U_PUSHINT(vm, regs + 1, 2);
    U_IEQ(vm, regs + 2);
    block14509:;
    U_ASSERT(vm, regs + 1, 158, 3, 93);
    U_STATEMENT(vm, regs + 0, 159, 3);
    regs[0] = locals[0];regs[1] = locals[1]; // rs
    U_ST2S(vm, regs + 2, 421); // RS
    keepvar[0] = TopM(regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 94); // "RS{\"hihi\", 2}"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 159, 3, 95);
    U_RETURNLOCAL(vm, 0, 0);
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
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1196, locals);
    U_STATEMENT(vm, regs + 0, 156, 3);
    regs[0] = locals[0]; // x+0
    regs[1] = locals[0]; // x+0
    U_SADD(vm, regs + 2);
    regs[1] = locals[1]; // x+1
    regs[2] = locals[1]; // x+1
    U_IADD(vm, regs + 3);
    U_RETURNLOCAL(vm, 0, 2);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function121
static void fun_125(VMRef vm, StackPtr psp) {
    Value regs[18];
    Value locals[10];
    locals[0] = lobster::NilVal();
    locals[1] = lobster::NilVal();
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    locals[5] = lobster::NilVal();
    locals[6] = lobster::NilVal();
    locals[7] = lobster::NilVal();
    locals[8] = lobster::NilVal();
    locals[9] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1203, locals);
    U_STATEMENT(vm, regs + 0, 163, 3);
    U_STATEMENT(vm, regs + 0, 163, 3);
    U_STATEMENT(vm, regs + 0, 165, 3);
    U_PUSHINT(vm, regs + 0, 1);
    U_PUSHINT(vm, regs + 1, 1);
    U_IVVADD(vm, regs + 2, 1);
    U_PUSHINT(vm, regs + 1, 2);
    U_STEQ(vm, regs + 2, 1);
    U_ASSERT(vm, regs + 1, 165, 3, 96);
    U_STATEMENT(vm, regs + 0, 166, 3);
    U_PUSHINT(vm, regs + 0, 2);
    U_PUSHINT(vm, regs + 1, 1);
    U_PUSHINT(vm, regs + 2, 1);
    U_PUSHINT(vm, regs + 3, 1);
    U_PUSHINT(vm, regs + 4, -1);
    U_PUSHINT(vm, regs + 5, 1);
    U_BCALLRETV(vm, regs + 6, 129, 1); // max
    U_PUSHINT(vm, regs + 3, 1);
    U_BCALLRETV(vm, regs + 4, 121, 1); // min
    U_PUSHINT(vm, regs + 1, 1);
    U_STEQ(vm, regs + 2, 1);
    U_ASSERT(vm, regs + 1, 166, 3, 97);
    U_STATEMENT(vm, regs + 0, 167, 3);
    U_PUSHINT(vm, regs + 0, 2);
    U_PUSHINT(vm, regs + 1, 1);
    U_BCALLRETV(vm, regs + 2, 89, 1); // manhattan
    U_PUSHINT(vm, regs + 1, 2);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 167, 3, 98);
    U_STATEMENT(vm, regs + 0, 169, 3);
    U_STATEMENT(vm, regs + 0, 169, 3);
    U_STATEMENT(vm, regs + 0, 175, 3);
    U_PUSHFLT(vm, regs + 0, 1065353216);
    U_PUSHFLT(vm, regs + 1, 1073741824);
    U_PUSHFLT(vm, regs + 2, 1077936128);
    U_PUSHFLT(vm, regs + 3, 1082130432);
    U_PUSHFLT(vm, regs + 4, 1084227584);
    SetLVal(vm, &locals[0]); // v5test+0
    U_LV_WRITEV(vm, regs + 5, 5);
    U_STATEMENT(vm, regs + 0, 176, 3);
    regs[0] = locals[0];regs[1] = locals[1];regs[2] = locals[2];regs[3] = locals[3];regs[4] = locals[4]; // v5test
    regs[5] = locals[0];regs[6] = locals[1];regs[7] = locals[2];regs[8] = locals[3];regs[9] = locals[4]; // v5test
    U_FVVADD(vm, regs + 10, 5);
    SetLVal(vm, &locals[5]); // v5test2+0
    U_LV_WRITEV(vm, regs + 5, 5);
    U_STATEMENT(vm, regs + 0, 177, 3);
    regs[0] = locals[5];regs[1] = locals[6];regs[2] = locals[7];regs[3] = locals[8];regs[4] = locals[9]; // v5test2
    regs[5] = locals[0];regs[6] = locals[1];regs[7] = locals[2];regs[8] = locals[3];regs[9] = locals[4]; // v5test
    U_PUSHFLT(vm, regs + 10, 1073741824);
    U_FVSMUL(vm, regs + 11, 5);
    U_STEQ(vm, regs + 10, 5);
    U_ASSERT(vm, regs + 1, 177, 3, 99);
    U_STATEMENT(vm, regs + 0, 178, 3);
    regs[0] = locals[5];regs[1] = locals[6];regs[2] = locals[7];regs[3] = locals[8];regs[4] = locals[9]; // v5test2
    U_PUSHINT(vm, regs + 5, 5);
    U_BCALLRETV(vm, regs + 6, 84, 1); // normalize
    U_PUSHINT(vm, regs + 5, 5);
    U_BCALLRETV(vm, regs + 6, 86, 1); // magnitude
    U_PUSHFLT(vm, regs + 1, 1065353216);
    U_FEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 178, 3, 100);
    U_STATEMENT(vm, regs + 0, 179, 3);
    regs[0] = locals[0];regs[1] = locals[1];regs[2] = locals[2];regs[3] = locals[3];regs[4] = locals[4]; // v5test
    U_PUSHINT(vm, regs + 5, 5);
    regs[6] = locals[0];regs[7] = locals[1];regs[8] = locals[2];regs[9] = locals[3];regs[10] = locals[4]; // v5test
    U_PUSHINT(vm, regs + 11, 5);
    regs[12] = locals[5];regs[13] = locals[6];regs[14] = locals[7];regs[15] = locals[8];regs[16] = locals[9]; // v5test2
    U_PUSHINT(vm, regs + 17, 5);
    U_BCALLRETV(vm, regs + 18, 130, 1); // max
    U_PUSHINT(vm, regs + 11, 5);
    U_BCALLRETV(vm, regs + 12, 122, 1); // min
    regs[5] = locals[0];regs[6] = locals[1];regs[7] = locals[2];regs[8] = locals[3];regs[9] = locals[4]; // v5test
    U_STEQ(vm, regs + 10, 5);
    U_ASSERT(vm, regs + 1, 179, 3, 101);
    U_STATEMENT(vm, regs + 0, 180, 3);
    regs[0] = locals[0];regs[1] = locals[1];regs[2] = locals[2];regs[3] = locals[3];regs[4] = locals[4]; // v5test
    U_PUSHINT(vm, regs + 5, 5);
    regs[6] = locals[5];regs[7] = locals[6];regs[8] = locals[7];regs[9] = locals[8];regs[10] = locals[9]; // v5test2
    U_PUSHINT(vm, regs + 11, 5);
    U_BCALLRETV(vm, regs + 12, 85, 1); // dot
    U_PUSHFLT(vm, regs + 1, 1121714176);
    U_FEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 180, 3, 102);
    U_RETURNLOCAL(vm, 0, 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function122
static void fun_126(VMRef vm, StackPtr psp) {
    Value regs[10];
    PushFunId(vm, funinfo_table + 1218, 0);
    U_STATEMENT(vm, regs + 0, 187, 3);
    U_PUSHINT(vm, regs + 0, 2);
    U_PUSHINT(vm, regs + 1, 3);
    U_PUSHINT(vm, regs + 2, 5);
    U_IVSMUL(vm, regs + 3, 2);
    U_PUSHINT(vm, regs + 2, 10);
    U_PUSHINT(vm, regs + 3, 15);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 187, 3, 103);
    U_STATEMENT(vm, regs + 0, 188, 3);
    U_PUSHINT(vm, regs + 0, 5);
    U_PUSHINT(vm, regs + 1, 2);
    U_PUSHINT(vm, regs + 2, 3);
    U_SIVMUL(vm, regs + 3, 2);
    U_PUSHINT(vm, regs + 2, 10);
    U_PUSHINT(vm, regs + 3, 15);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 188, 3, 104);
    U_STATEMENT(vm, regs + 0, 189, 3);
    U_PUSHINT(vm, regs + 0, 2);
    U_PUSHINT(vm, regs + 1, 3);
    U_PUSHINT(vm, regs + 2, 4);
    U_PUSHINT(vm, regs + 3, 5);
    U_IVSMUL(vm, regs + 4, 3);
    U_PUSHINT(vm, regs + 3, 10);
    U_PUSHINT(vm, regs + 4, 15);
    U_PUSHINT(vm, regs + 5, 20);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 189, 3, 105);
    U_STATEMENT(vm, regs + 0, 190, 3);
    U_PUSHINT(vm, regs + 0, 5);
    U_PUSHINT(vm, regs + 1, 2);
    U_PUSHINT(vm, regs + 2, 3);
    U_PUSHINT(vm, regs + 3, 4);
    U_SIVMUL(vm, regs + 4, 3);
    U_PUSHINT(vm, regs + 3, 10);
    U_PUSHINT(vm, regs + 4, 15);
    U_PUSHINT(vm, regs + 5, 20);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 190, 3, 106);
    U_STATEMENT(vm, regs + 0, 193, 3);
    U_PUSHFLT64(vm, regs + 0, -858993459, 1073794252);
    U_PUSHFLT64(vm, regs + 1, -1717986918, 1074370969);
    U_PUSHFLT(vm, regs + 2, 1084227584);
    U_FVSMUL(vm, regs + 3, 2);
    U_PUSHFLT(vm, regs + 2, 1093140480);
    U_PUSHFLT(vm, regs + 3, 1098907648);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 193, 3, 107);
    U_STATEMENT(vm, regs + 0, 194, 3);
    U_PUSHFLT(vm, regs + 0, 1084227584);
    U_PUSHFLT64(vm, regs + 1, -858993459, 1073794252);
    U_PUSHFLT64(vm, regs + 2, -1717986918, 1074370969);
    U_SFVMUL(vm, regs + 3, 2);
    U_PUSHFLT(vm, regs + 2, 1093140480);
    U_PUSHFLT(vm, regs + 3, 1098907648);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 194, 3, 108);
    U_STATEMENT(vm, regs + 0, 197, 3);
    U_STATEMENT(vm, regs + 0, 197, 3);
    U_STATEMENT(vm, regs + 0, 204, 3);
    U_PUSHINT(vm, regs + 0, 2);
    U_PUSHINT(vm, regs + 1, 3);
    U_PUSHINT(vm, regs + 2, 4);
    U_PUSHINT(vm, regs + 3, 5);
    U_PUSHINT(vm, regs + 4, 6);
    U_PUSHINT(vm, regs + 5, 5);
    U_IVSMUL(vm, regs + 6, 5);
    U_PUSHINT(vm, regs + 5, 10);
    U_PUSHINT(vm, regs + 6, 15);
    U_PUSHINT(vm, regs + 7, 20);
    U_PUSHINT(vm, regs + 8, 25);
    U_PUSHINT(vm, regs + 9, 30);
    U_STEQ(vm, regs + 10, 5);
    U_ASSERT(vm, regs + 1, 204, 3, 109);
    U_STATEMENT(vm, regs + 0, 205, 3);
    U_PUSHINT(vm, regs + 0, 5);
    U_PUSHINT(vm, regs + 1, 2);
    U_PUSHINT(vm, regs + 2, 3);
    U_PUSHINT(vm, regs + 3, 4);
    U_PUSHINT(vm, regs + 4, 5);
    U_PUSHINT(vm, regs + 5, 6);
    U_SIVMUL(vm, regs + 6, 5);
    U_PUSHINT(vm, regs + 5, 10);
    U_PUSHINT(vm, regs + 6, 15);
    U_PUSHINT(vm, regs + 7, 20);
    U_PUSHINT(vm, regs + 8, 25);
    U_PUSHINT(vm, regs + 9, 30);
    U_STEQ(vm, regs + 10, 5);
    U_ASSERT(vm, regs + 1, 205, 3, 110);
    U_STATEMENT(vm, regs + 0, 208, 3);
    U_PUSHINT(vm, regs + 0, 5);
    U_PUSHINT(vm, regs + 1, 2);
    U_PUSHINT(vm, regs + 2, 3);
    U_SIVADD(vm, regs + 3, 2);
    U_PUSHINT(vm, regs + 2, 7);
    U_PUSHINT(vm, regs + 3, 8);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 208, 3, 111);
    U_STATEMENT(vm, regs + 0, 209, 3);
    U_PUSHINT(vm, regs + 0, 2);
    U_PUSHINT(vm, regs + 1, 3);
    U_PUSHINT(vm, regs + 2, 5);
    U_IVSADD(vm, regs + 3, 2);
    U_PUSHINT(vm, regs + 2, 7);
    U_PUSHINT(vm, regs + 3, 8);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 209, 3, 112);
    U_STATEMENT(vm, regs + 0, 210, 3);
    U_PUSHINT(vm, regs + 0, 2);
    U_PUSHINT(vm, regs + 1, 3);
    U_PUSHINT(vm, regs + 2, 4);
    U_PUSHINT(vm, regs + 3, 5);
    U_IVSADD(vm, regs + 4, 3);
    U_PUSHINT(vm, regs + 3, 7);
    U_PUSHINT(vm, regs + 4, 8);
    U_PUSHINT(vm, regs + 5, 9);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 210, 3, 113);
    U_STATEMENT(vm, regs + 0, 211, 3);
    U_PUSHINT(vm, regs + 0, 5);
    U_PUSHINT(vm, regs + 1, 2);
    U_PUSHINT(vm, regs + 2, 3);
    U_PUSHINT(vm, regs + 3, 4);
    U_SIVADD(vm, regs + 4, 3);
    U_PUSHINT(vm, regs + 3, 7);
    U_PUSHINT(vm, regs + 4, 8);
    U_PUSHINT(vm, regs + 5, 9);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 211, 3, 114);
    U_STATEMENT(vm, regs + 0, 216, 3);
    U_PUSHINT(vm, regs + 0, 5);
    U_PUSHINT(vm, regs + 1, 2);
    U_PUSHINT(vm, regs + 2, 3);
    U_SIVSUB(vm, regs + 3, 2);
    U_PUSHINT(vm, regs + 2, 3);
    U_PUSHINT(vm, regs + 3, 2);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 216, 3, 115);
    U_STATEMENT(vm, regs + 0, 217, 3);
    U_PUSHINT(vm, regs + 0, 2);
    U_PUSHINT(vm, regs + 1, 3);
    U_PUSHINT(vm, regs + 2, 5);
    U_IVSSUB(vm, regs + 3, 2);
    U_PUSHINT(vm, regs + 2, -3);
    U_PUSHINT(vm, regs + 3, -2);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 217, 3, 116);
    U_STATEMENT(vm, regs + 0, 218, 3);
    U_PUSHINT(vm, regs + 0, 5);
    U_PUSHINT(vm, regs + 1, 2);
    U_PUSHINT(vm, regs + 2, 3);
    U_PUSHINT(vm, regs + 3, 4);
    U_SIVSUB(vm, regs + 4, 3);
    U_PUSHINT(vm, regs + 3, 3);
    U_PUSHINT(vm, regs + 4, 2);
    U_PUSHINT(vm, regs + 5, 1);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 218, 3, 117);
    U_STATEMENT(vm, regs + 0, 219, 3);
    U_PUSHINT(vm, regs + 0, 2);
    U_PUSHINT(vm, regs + 1, 3);
    U_PUSHINT(vm, regs + 2, 4);
    U_PUSHINT(vm, regs + 3, 5);
    U_IVSSUB(vm, regs + 4, 3);
    U_PUSHINT(vm, regs + 3, -3);
    U_PUSHINT(vm, regs + 4, -2);
    U_PUSHINT(vm, regs + 5, -1);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 219, 3, 118);
    U_STATEMENT(vm, regs + 0, 222, 3);
    U_PUSHINT(vm, regs + 0, 10);
    U_PUSHINT(vm, regs + 1, 5);
    U_PUSHINT(vm, regs + 2, 10);
    U_SIVDIV(vm, regs + 3, 2);
    U_PUSHINT(vm, regs + 2, 2);
    U_PUSHINT(vm, regs + 3, 1);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 222, 3, 119);
    U_STATEMENT(vm, regs + 0, 223, 3);
    U_PUSHINT(vm, regs + 0, 10);
    U_PUSHINT(vm, regs + 1, 20);
    U_PUSHINT(vm, regs + 2, 5);
    U_IVSDIV(vm, regs + 3, 2);
    U_PUSHINT(vm, regs + 2, 2);
    U_PUSHINT(vm, regs + 3, 4);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 223, 3, 120);
    U_STATEMENT(vm, regs + 0, 224, 3);
    U_PUSHINT(vm, regs + 0, 10);
    U_PUSHINT(vm, regs + 1, 5);
    U_PUSHINT(vm, regs + 2, 10);
    U_PUSHINT(vm, regs + 3, 1);
    U_SIVDIV(vm, regs + 4, 3);
    U_PUSHINT(vm, regs + 3, 2);
    U_PUSHINT(vm, regs + 4, 1);
    U_PUSHINT(vm, regs + 5, 10);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 224, 3, 121);
    U_STATEMENT(vm, regs + 0, 225, 3);
    U_PUSHINT(vm, regs + 0, 10);
    U_PUSHINT(vm, regs + 1, 20);
    U_PUSHINT(vm, regs + 2, 25);
    U_PUSHINT(vm, regs + 3, 5);
    U_IVSDIV(vm, regs + 4, 3);
    U_PUSHINT(vm, regs + 3, 2);
    U_PUSHINT(vm, regs + 4, 4);
    U_PUSHINT(vm, regs + 5, 5);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 225, 3, 122);
    U_STATEMENT(vm, regs + 0, 228, 3);
    U_PUSHINT(vm, regs + 0, 10);
    U_PUSHINT(vm, regs + 1, 2);
    U_PUSHINT(vm, regs + 2, 3);
    U_SIVMOD(vm, regs + 3, 2);
    U_PUSHINT(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 3, 1);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 228, 3, 123);
    U_STATEMENT(vm, regs + 0, 229, 3);
    U_PUSHINT(vm, regs + 0, 10);
    U_PUSHINT(vm, regs + 1, 21);
    U_PUSHINT(vm, regs + 2, 4);
    U_IVSMOD(vm, regs + 3, 2);
    U_PUSHINT(vm, regs + 2, 2);
    U_PUSHINT(vm, regs + 3, 1);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 229, 3, 124);
    U_STATEMENT(vm, regs + 0, 230, 3);
    U_PUSHINT(vm, regs + 0, 10);
    U_PUSHINT(vm, regs + 1, 2);
    U_PUSHINT(vm, regs + 2, 3);
    U_PUSHINT(vm, regs + 3, 4);
    U_SIVMOD(vm, regs + 4, 3);
    U_PUSHINT(vm, regs + 3, 0);
    U_PUSHINT(vm, regs + 4, 1);
    U_PUSHINT(vm, regs + 5, 2);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 230, 3, 125);
    U_STATEMENT(vm, regs + 0, 231, 3);
    U_PUSHINT(vm, regs + 0, 10);
    U_PUSHINT(vm, regs + 1, 21);
    U_PUSHINT(vm, regs + 2, 24);
    U_PUSHINT(vm, regs + 3, 4);
    U_IVSMOD(vm, regs + 4, 3);
    U_PUSHINT(vm, regs + 3, 2);
    U_PUSHINT(vm, regs + 4, 1);
    U_PUSHINT(vm, regs + 5, 0);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 231, 3, 126);
    U_STATEMENT(vm, regs + 0, 236, 3);
    U_PUSHINT(vm, regs + 0, 2);
    U_PUSHINT(vm, regs + 1, 5);
    U_PUSHINT(vm, regs + 2, 5);
    U_IVSLT(vm, regs + 3, 2);
    U_PUSHINT(vm, regs + 2, 1);
    U_PUSHINT(vm, regs + 3, 0);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 236, 3, 127);
    U_STATEMENT(vm, regs + 0, 237, 3);
    U_PUSHINT(vm, regs + 0, 5);
    U_PUSHINT(vm, regs + 1, 2);
    U_PUSHINT(vm, regs + 2, 5);
    U_SIVLT(vm, regs + 3, 2);
    U_PUSHINT(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 3, 0);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 237, 3, 128);
    U_STATEMENT(vm, regs + 0, 238, 3);
    U_PUSHINT(vm, regs + 0, 2);
    U_PUSHINT(vm, regs + 1, 6);
    U_PUSHINT(vm, regs + 2, 5);
    U_IVSLT(vm, regs + 3, 2);
    U_PUSHINT(vm, regs + 2, 1);
    U_PUSHINT(vm, regs + 3, 0);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 238, 3, 129);
    U_STATEMENT(vm, regs + 0, 239, 3);
    U_PUSHINT(vm, regs + 0, 5);
    U_PUSHINT(vm, regs + 1, 2);
    U_PUSHINT(vm, regs + 2, 6);
    U_SIVLT(vm, regs + 3, 2);
    U_PUSHINT(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 3, 1);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 239, 3, 130);
    U_STATEMENT(vm, regs + 0, 241, 3);
    U_PUSHINT(vm, regs + 0, 2);
    U_PUSHINT(vm, regs + 1, 5);
    U_PUSHINT(vm, regs + 2, 6);
    U_PUSHINT(vm, regs + 3, 5);
    U_IVSLT(vm, regs + 4, 3);
    U_PUSHINT(vm, regs + 3, 1);
    U_PUSHINT(vm, regs + 4, 0);
    U_PUSHINT(vm, regs + 5, 0);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 241, 3, 131);
    U_STATEMENT(vm, regs + 0, 242, 3);
    U_PUSHINT(vm, regs + 0, 5);
    U_PUSHINT(vm, regs + 1, 2);
    U_PUSHINT(vm, regs + 2, 5);
    U_PUSHINT(vm, regs + 3, 6);
    U_SIVLT(vm, regs + 4, 3);
    U_PUSHINT(vm, regs + 3, 0);
    U_PUSHINT(vm, regs + 4, 0);
    U_PUSHINT(vm, regs + 5, 1);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 242, 3, 132);
    U_STATEMENT(vm, regs + 0, 245, 3);
    U_PUSHINT(vm, regs + 0, 2);
    U_PUSHINT(vm, regs + 1, 5);
    U_PUSHINT(vm, regs + 2, 5);
    U_IVSLE(vm, regs + 3, 2);
    U_PUSHINT(vm, regs + 2, 1);
    U_PUSHINT(vm, regs + 3, 1);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 245, 3, 133);
    U_STATEMENT(vm, regs + 0, 246, 3);
    U_PUSHINT(vm, regs + 0, 5);
    U_PUSHINT(vm, regs + 1, 2);
    U_PUSHINT(vm, regs + 2, 5);
    U_SIVLE(vm, regs + 3, 2);
    U_PUSHINT(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 3, 1);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 246, 3, 134);
    U_STATEMENT(vm, regs + 0, 247, 3);
    U_PUSHINT(vm, regs + 0, 2);
    U_PUSHINT(vm, regs + 1, 6);
    U_PUSHINT(vm, regs + 2, 5);
    U_IVSLE(vm, regs + 3, 2);
    U_PUSHINT(vm, regs + 2, 1);
    U_PUSHINT(vm, regs + 3, 0);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 247, 3, 135);
    U_STATEMENT(vm, regs + 0, 248, 3);
    U_PUSHINT(vm, regs + 0, 5);
    U_PUSHINT(vm, regs + 1, 2);
    U_PUSHINT(vm, regs + 2, 6);
    U_SIVLE(vm, regs + 3, 2);
    U_PUSHINT(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 3, 1);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 248, 3, 136);
    U_STATEMENT(vm, regs + 0, 250, 3);
    U_PUSHINT(vm, regs + 0, 2);
    U_PUSHINT(vm, regs + 1, 5);
    U_PUSHINT(vm, regs + 2, 6);
    U_PUSHINT(vm, regs + 3, 5);
    U_IVSLE(vm, regs + 4, 3);
    U_PUSHINT(vm, regs + 3, 1);
    U_PUSHINT(vm, regs + 4, 1);
    U_PUSHINT(vm, regs + 5, 0);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 250, 3, 137);
    U_STATEMENT(vm, regs + 0, 251, 3);
    U_PUSHINT(vm, regs + 0, 5);
    U_PUSHINT(vm, regs + 1, 2);
    U_PUSHINT(vm, regs + 2, 5);
    U_PUSHINT(vm, regs + 3, 6);
    U_SIVLE(vm, regs + 4, 3);
    U_PUSHINT(vm, regs + 3, 0);
    U_PUSHINT(vm, regs + 4, 1);
    U_PUSHINT(vm, regs + 5, 1);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 251, 3, 138);
    U_STATEMENT(vm, regs + 0, 254, 3);
    U_PUSHINT(vm, regs + 0, 2);
    U_PUSHINT(vm, regs + 1, 5);
    U_PUSHINT(vm, regs + 2, 5);
    U_IVSGT(vm, regs + 3, 2);
    U_PUSHINT(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 3, 0);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 254, 3, 139);
    U_STATEMENT(vm, regs + 0, 255, 3);
    U_PUSHINT(vm, regs + 0, 5);
    U_PUSHINT(vm, regs + 1, 2);
    U_PUSHINT(vm, regs + 2, 5);
    U_SIVGT(vm, regs + 3, 2);
    U_PUSHINT(vm, regs + 2, 1);
    U_PUSHINT(vm, regs + 3, 0);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 255, 3, 140);
    U_STATEMENT(vm, regs + 0, 256, 3);
    U_PUSHINT(vm, regs + 0, 2);
    U_PUSHINT(vm, regs + 1, 6);
    U_PUSHINT(vm, regs + 2, 5);
    U_IVSGT(vm, regs + 3, 2);
    U_PUSHINT(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 3, 1);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 256, 3, 141);
    U_STATEMENT(vm, regs + 0, 257, 3);
    U_PUSHINT(vm, regs + 0, 5);
    U_PUSHINT(vm, regs + 1, 2);
    U_PUSHINT(vm, regs + 2, 6);
    U_SIVGT(vm, regs + 3, 2);
    U_PUSHINT(vm, regs + 2, 1);
    U_PUSHINT(vm, regs + 3, 0);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 257, 3, 142);
    U_STATEMENT(vm, regs + 0, 259, 3);
    U_PUSHINT(vm, regs + 0, 2);
    U_PUSHINT(vm, regs + 1, 5);
    U_PUSHINT(vm, regs + 2, 6);
    U_PUSHINT(vm, regs + 3, 5);
    U_IVSGT(vm, regs + 4, 3);
    U_PUSHINT(vm, regs + 3, 0);
    U_PUSHINT(vm, regs + 4, 0);
    U_PUSHINT(vm, regs + 5, 1);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 259, 3, 143);
    U_STATEMENT(vm, regs + 0, 260, 3);
    U_PUSHINT(vm, regs + 0, 5);
    U_PUSHINT(vm, regs + 1, 2);
    U_PUSHINT(vm, regs + 2, 5);
    U_PUSHINT(vm, regs + 3, 6);
    U_SIVGT(vm, regs + 4, 3);
    U_PUSHINT(vm, regs + 3, 1);
    U_PUSHINT(vm, regs + 4, 0);
    U_PUSHINT(vm, regs + 5, 0);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 260, 3, 144);
    U_STATEMENT(vm, regs + 0, 263, 3);
    U_PUSHINT(vm, regs + 0, 2);
    U_PUSHINT(vm, regs + 1, 5);
    U_PUSHINT(vm, regs + 2, 5);
    U_IVSGE(vm, regs + 3, 2);
    U_PUSHINT(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 3, 1);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 263, 3, 145);
    U_STATEMENT(vm, regs + 0, 264, 3);
    U_PUSHINT(vm, regs + 0, 5);
    U_PUSHINT(vm, regs + 1, 2);
    U_PUSHINT(vm, regs + 2, 5);
    U_SIVGE(vm, regs + 3, 2);
    U_PUSHINT(vm, regs + 2, 1);
    U_PUSHINT(vm, regs + 3, 1);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 264, 3, 146);
    U_STATEMENT(vm, regs + 0, 265, 3);
    U_PUSHINT(vm, regs + 0, 2);
    U_PUSHINT(vm, regs + 1, 6);
    U_PUSHINT(vm, regs + 2, 5);
    U_IVSGE(vm, regs + 3, 2);
    U_PUSHINT(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 3, 1);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 265, 3, 147);
    U_STATEMENT(vm, regs + 0, 266, 3);
    U_PUSHINT(vm, regs + 0, 5);
    U_PUSHINT(vm, regs + 1, 2);
    U_PUSHINT(vm, regs + 2, 6);
    U_SIVGE(vm, regs + 3, 2);
    U_PUSHINT(vm, regs + 2, 1);
    U_PUSHINT(vm, regs + 3, 0);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 266, 3, 148);
    U_STATEMENT(vm, regs + 0, 268, 3);
    U_PUSHINT(vm, regs + 0, 2);
    U_PUSHINT(vm, regs + 1, 5);
    U_PUSHINT(vm, regs + 2, 6);
    U_PUSHINT(vm, regs + 3, 5);
    U_IVSGE(vm, regs + 4, 3);
    U_PUSHINT(vm, regs + 3, 0);
    U_PUSHINT(vm, regs + 4, 1);
    U_PUSHINT(vm, regs + 5, 1);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 268, 3, 149);
    U_STATEMENT(vm, regs + 0, 269, 3);
    U_PUSHINT(vm, regs + 0, 5);
    U_PUSHINT(vm, regs + 1, 2);
    U_PUSHINT(vm, regs + 2, 5);
    U_PUSHINT(vm, regs + 3, 6);
    U_SIVGE(vm, regs + 4, 3);
    U_PUSHINT(vm, regs + 3, 1);
    U_PUSHINT(vm, regs + 4, 1);
    U_PUSHINT(vm, regs + 5, 0);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 269, 3, 150);
    U_RETURNLOCAL(vm, 0, 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function123
static void fun_127(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value keepvar[2];
    PushFunId(vm, funinfo_table + 1223, 0);
    keepvar[0] = lobster::NilVal();
    keepvar[1] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 273, 3);
    U_PUSHINT(vm, regs + 0, 1);
    U_PUSHINT(vm, regs + 1, 2);
    U_NEWVEC(vm, regs + 2, 37, 1);
    keepvar[0] = TopM(regs + 1, 0);
    U_PUSHINT(vm, regs + 1, 1);
    U_PUSHINT(vm, regs + 2, 2);
    U_NEWVEC(vm, regs + 3, 37, 1);
    keepvar[1] = TopM(regs + 2, 0);
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 273, 3, 151);
    U_RETURNLOCAL(vm, 0, 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    DecVal(vm, keepvar[1]);
    PopFunId(vm);
}

// try
static void fun_474(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[4];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1228, locals);
    U_STATEMENT(vm, regs + 0, 4, 7);
    regs[0] = locals[0]; // body
    fun_475(vm, regs + 1); // call: exception_handler
    SetLVal(vm, &locals[3]); // stack_trace
    U_LV_WRITEREF(vm, regs + 2);
    SetLVal(vm, &locals[2]); // err
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 7);
    regs[0] = locals[2]; // err
    if (!U_JUMPFAIL(vm, regs + 1)) goto block16615;
    U_STATEMENT(vm, regs + 0, 6, 7);
    regs[0] = locals[2]; // err
    fun_178(vm, regs + 1); // call: function171
    if (!U_JUMPIFUNWOUND(vm, regs + 2, 171)) goto block16613;
    U_RETURNANY(vm, 0, 0);
    DecVal(vm, locals[2]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block16613:;
    block16615:;
    U_RETURNNONLOCAL(vm, 0, 0, 124);
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
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1237, locals);
    U_STATEMENT(vm, regs + 0, 4, 7);
    regs[0] = locals[0]; // body
    fun_129(vm, regs + 1); // call: exception_handler
    SetLVal(vm, &locals[3]); // stack_trace
    U_LV_WRITEREF(vm, regs + 2);
    SetLVal(vm, &locals[2]); // err
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 7);
    regs[0] = locals[2]; // err
    if (!U_JUMPFAIL(vm, regs + 1)) goto block16686;
    U_STATEMENT(vm, regs + 0, 6, 7);
    U_STATEMENT(vm, regs + 0, 6, 7);
    regs[0] = locals[2]; // err
    SetLVal(vm, &locals[4]); // _
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 29, 7);
    block16686:;
    U_RETURNLOCAL(vm, 0, 0);
    DecVal(vm, locals[2]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// exception_handler
static void fun_475(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1247, locals);
    U_STATEMENT(vm, regs + 0, 9, 7);
    fun_177(vm, regs + 0); // call: function170
    if (!U_JUMPIFUNWOUND(vm, regs + 2, 170)) goto block16716;
    U_RETURNANY(vm, 0, 0);
    Pop(psp);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block16716:;
    U_STATEMENT(vm, regs + 0, 10, 7);
    U_PUSHNIL(vm, regs + 0);
    U_PUSHSTR(vm, regs + 1, 152); // ""
    U_INCREF(vm, regs + 2, 0);
    U_RETURNLOCAL(vm, 0, 2);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// exception_handler
static void fun_129(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1253, locals);
    U_STATEMENT(vm, regs + 0, 9, 7);
    fun_179(vm, regs + 0); // call: function172
    if (!U_JUMPIFUNWOUND(vm, regs + 2, 172)) goto block16758;
    U_RETURNANY(vm, 0, 0);
    Pop(psp);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block16758:;
    U_STATEMENT(vm, regs + 0, 10, 7);
    U_PUSHNIL(vm, regs + 0);
    U_PUSHSTR(vm, regs + 1, 153); // ""
    U_INCREF(vm, regs + 2, 0);
    U_RETURNLOCAL(vm, 0, 2);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// throw
static void fun_476(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1259, locals);
    U_STATEMENT(vm, regs + 0, 17, 7);
    regs[0] = locals[0]; // v
    U_BCALLRET0(vm, regs + 1, 168, 1); // get_stack_trace
    U_INCREF(vm, regs + 2, 1);
    U_RETURNNONLOCAL(vm, 0, 2, 125);
    DecVal(vm, locals[0]);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// throw
static void fun_130(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1265, locals);
    U_STATEMENT(vm, regs + 0, 17, 7);
    regs[0] = locals[0]; // v
    U_BCALLRET0(vm, regs + 1, 168, 1); // get_stack_trace
    U_INCREF(vm, regs + 2, 1);
    U_RETURNNONLOCAL(vm, 0, 2, 125);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// try
static void fun_133(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1271, locals);
    U_STATEMENT(vm, regs + 0, 29, 7);
    regs[0] = locals[0]; // body
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_128(vm, regs + 2); // call: try
    U_RETURNLOCAL(vm, 0, 0);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// lookup
static void fun_534(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[4];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1277, locals);
    U_STATEMENT(vm, regs + 0, 25, 8);
    regs[0] = locals[1]; // key
    U_BCALLRET1(vm, regs + 1, 148, 1); // hash
    regs[1] = locals[0]; // this
    U_PUSHFLD(vm, regs + 2, 0);
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_IMOD(vm, regs + 2);
    SetLVal(vm, &locals[2]); // h
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 26, 8);
    regs[0] = locals[0]; // this
    U_PUSHFLD(vm, regs + 1, 0);
    regs[1] = locals[2]; // h
    U_VPUSHIDXI(vm, regs + 2);
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[3]); // c
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 27, 8);
    block16925:;
    regs[0] = locals[3]; // c
    if (!U_JUMPFAIL(vm, regs + 1)) goto block16987;
    U_STATEMENT(vm, regs + 0, 28, 8);
    regs[0] = locals[3]; // c
    U_PUSHFLD(vm, regs + 1, 1);
    regs[1] = locals[1]; // key
    U_SEQ(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block16964;
    U_STATEMENT(vm, regs + 0, 29, 8);
    regs[0] = locals[3]; // c
    U_INCREF(vm, regs + 1, 0);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    block16964:;
    U_STATEMENT(vm, regs + 0, 30, 8);
    regs[0] = locals[3]; // c
    U_PUSHFLD(vm, regs + 1, 0);
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[3]); // c
    U_LV_WRITEREF(vm, regs + 1);
    goto block16925;
    block16987:;
    U_STATEMENT(vm, regs + 0, 31, 8);
    U_PUSHNIL(vm, regs + 0);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// lookup
static void fun_532(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[4];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1286, locals);
    U_STATEMENT(vm, regs + 0, 25, 8);
    regs[0] = locals[1]; // key
    U_BCALLRET1(vm, regs + 1, 148, 1); // hash
    regs[1] = locals[0]; // this
    U_PUSHFLD(vm, regs + 2, 0);
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_IMOD(vm, regs + 2);
    SetLVal(vm, &locals[2]); // h
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 26, 8);
    regs[0] = locals[0]; // this
    U_PUSHFLD(vm, regs + 1, 0);
    regs[1] = locals[2]; // h
    U_VPUSHIDXI(vm, regs + 2);
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[3]); // c
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 27, 8);
    block17068:;
    regs[0] = locals[3]; // c
    if (!U_JUMPFAIL(vm, regs + 1)) goto block17133;
    U_STATEMENT(vm, regs + 0, 28, 8);
    regs[0] = locals[3]; // c
    U_PUSHFLD(vm, regs + 1, 1);
    regs[1] = locals[1]; // key
    U_SEQ(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block17110;
    U_STATEMENT(vm, regs + 0, 29, 8);
    regs[0] = locals[3]; // c
    regs[1] = locals[2]; // h
    U_INCREF(vm, regs + 2, 1);
    U_RETURNLOCAL(vm, 0, 2);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    goto epilogue;
    block17110:;
    U_STATEMENT(vm, regs + 0, 30, 8);
    regs[0] = locals[3]; // c
    U_PUSHFLD(vm, regs + 1, 0);
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[3]); // c
    U_LV_WRITEREF(vm, regs + 1);
    goto block17068;
    block17133:;
    U_STATEMENT(vm, regs + 0, 31, 8);
    U_PUSHNIL(vm, regs + 0);
    regs[1] = locals[2]; // h
    U_RETURNLOCAL(vm, 0, 2);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// lookup
static void fun_504(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[4];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1295, locals);
    U_STATEMENT(vm, regs + 0, 25, 8);
    regs[0] = locals[1]; // key
    U_BCALLRET1(vm, regs + 1, 148, 1); // hash
    regs[1] = locals[0]; // this
    U_PUSHFLD(vm, regs + 2, 0);
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_IMOD(vm, regs + 2);
    SetLVal(vm, &locals[2]); // h
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 26, 8);
    regs[0] = locals[0]; // this
    U_PUSHFLD(vm, regs + 1, 0);
    regs[1] = locals[2]; // h
    U_VPUSHIDXI(vm, regs + 2);
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[3]); // c
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 27, 8);
    block17217:;
    regs[0] = locals[3]; // c
    if (!U_JUMPFAIL(vm, regs + 1)) goto block17279;
    U_STATEMENT(vm, regs + 0, 28, 8);
    regs[0] = locals[3]; // c
    U_PUSHFLD(vm, regs + 1, 1);
    regs[1] = locals[1]; // key
    U_SEQ(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block17256;
    U_STATEMENT(vm, regs + 0, 29, 8);
    regs[0] = locals[3]; // c
    U_INCREF(vm, regs + 1, 0);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    block17256:;
    U_STATEMENT(vm, regs + 0, 30, 8);
    regs[0] = locals[3]; // c
    U_PUSHFLD(vm, regs + 1, 0);
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[3]); // c
    U_LV_WRITEREF(vm, regs + 1);
    goto block17217;
    block17279:;
    U_STATEMENT(vm, regs + 0, 31, 8);
    U_PUSHNIL(vm, regs + 0);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// lookup
static void fun_494(VMRef vm, StackPtr psp) {
    Value regs[6];
    Value locals[6];
    locals[0] = *(psp - 4);
    locals[1] = *(psp - 3);
    locals[2] = *(psp - 2);
    locals[3] = *(psp - 1);
    locals[4] = lobster::NilVal();
    locals[5] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1304, locals);
    U_STATEMENT(vm, regs + 0, 25, 8);
    regs[0] = locals[1];regs[1] = locals[2];regs[2] = locals[3]; // key
    U_PUSHINT(vm, regs + 3, 3);
    U_BCALLRETV(vm, regs + 4, 151, 1); // hash
    regs[1] = locals[0]; // this
    U_PUSHFLD(vm, regs + 2, 0);
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_IMOD(vm, regs + 2);
    SetLVal(vm, &locals[4]); // h
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 26, 8);
    regs[0] = locals[0]; // this
    U_PUSHFLD(vm, regs + 1, 0);
    regs[1] = locals[4]; // h
    U_VPUSHIDXI(vm, regs + 2);
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[5]); // c
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 27, 8);
    block17365:;
    regs[0] = locals[5]; // c
    if (!U_JUMPFAIL(vm, regs + 1)) goto block17430;
    U_STATEMENT(vm, regs + 0, 28, 8);
    regs[0] = locals[5]; // c
    U_PUSHFLD2V(vm, regs + 1, 1, 3);
    regs[3] = locals[1];regs[4] = locals[2];regs[5] = locals[3]; // key
    U_STEQ(vm, regs + 6, 3);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block17407;
    U_STATEMENT(vm, regs + 0, 29, 8);
    regs[0] = locals[5]; // c
    U_INCREF(vm, regs + 1, 0);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[5]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    block17407:;
    U_STATEMENT(vm, regs + 0, 30, 8);
    regs[0] = locals[5]; // c
    U_PUSHFLD(vm, regs + 1, 0);
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[5]); // c
    U_LV_WRITEREF(vm, regs + 1);
    goto block17365;
    block17430:;
    U_STATEMENT(vm, regs + 0, 31, 8);
    U_PUSHNIL(vm, regs + 0);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[5]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// lookup
static void fun_492(VMRef vm, StackPtr psp) {
    Value regs[6];
    Value locals[6];
    locals[0] = *(psp - 4);
    locals[1] = *(psp - 3);
    locals[2] = *(psp - 2);
    locals[3] = *(psp - 1);
    locals[4] = lobster::NilVal();
    locals[5] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1315, locals);
    U_STATEMENT(vm, regs + 0, 25, 8);
    regs[0] = locals[1];regs[1] = locals[2];regs[2] = locals[3]; // key
    U_PUSHINT(vm, regs + 3, 3);
    U_BCALLRETV(vm, regs + 4, 151, 1); // hash
    regs[1] = locals[0]; // this
    U_PUSHFLD(vm, regs + 2, 0);
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_IMOD(vm, regs + 2);
    SetLVal(vm, &locals[4]); // h
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 26, 8);
    regs[0] = locals[0]; // this
    U_PUSHFLD(vm, regs + 1, 0);
    regs[1] = locals[4]; // h
    U_VPUSHIDXI(vm, regs + 2);
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[5]); // c
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 27, 8);
    block17516:;
    regs[0] = locals[5]; // c
    if (!U_JUMPFAIL(vm, regs + 1)) goto block17584;
    U_STATEMENT(vm, regs + 0, 28, 8);
    regs[0] = locals[5]; // c
    U_PUSHFLD2V(vm, regs + 1, 1, 3);
    regs[3] = locals[1];regs[4] = locals[2];regs[5] = locals[3]; // key
    U_STEQ(vm, regs + 6, 3);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block17561;
    U_STATEMENT(vm, regs + 0, 29, 8);
    regs[0] = locals[5]; // c
    regs[1] = locals[4]; // h
    U_INCREF(vm, regs + 2, 1);
    U_RETURNLOCAL(vm, 0, 2);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[5]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    goto epilogue;
    block17561:;
    U_STATEMENT(vm, regs + 0, 30, 8);
    regs[0] = locals[5]; // c
    U_PUSHFLD(vm, regs + 1, 0);
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[5]); // c
    U_LV_WRITEREF(vm, regs + 1);
    goto block17516;
    block17584:;
    U_STATEMENT(vm, regs + 0, 31, 8);
    U_PUSHNIL(vm, regs + 0);
    regs[1] = locals[4]; // h
    U_RETURNLOCAL(vm, 0, 2);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[5]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// lookup
static void fun_487(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[4];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1326, locals);
    U_STATEMENT(vm, regs + 0, 25, 8);
    regs[0] = locals[1]; // key
    U_BCALLRET1(vm, regs + 1, 148, 1); // hash
    regs[1] = locals[0]; // this
    U_PUSHFLD(vm, regs + 2, 0);
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_IMOD(vm, regs + 2);
    SetLVal(vm, &locals[2]); // h
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 26, 8);
    regs[0] = locals[0]; // this
    U_PUSHFLD(vm, regs + 1, 0);
    regs[1] = locals[2]; // h
    U_VPUSHIDXI(vm, regs + 2);
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[3]); // c
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 27, 8);
    block17668:;
    regs[0] = locals[3]; // c
    if (!U_JUMPFAIL(vm, regs + 1)) goto block17730;
    U_STATEMENT(vm, regs + 0, 28, 8);
    regs[0] = locals[3]; // c
    U_PUSHFLD(vm, regs + 1, 1);
    regs[1] = locals[1]; // key
    U_SEQ(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block17707;
    U_STATEMENT(vm, regs + 0, 29, 8);
    regs[0] = locals[3]; // c
    U_INCREF(vm, regs + 1, 0);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    block17707:;
    U_STATEMENT(vm, regs + 0, 30, 8);
    regs[0] = locals[3]; // c
    U_PUSHFLD(vm, regs + 1, 0);
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[3]); // c
    U_LV_WRITEREF(vm, regs + 1);
    goto block17668;
    block17730:;
    U_STATEMENT(vm, regs + 0, 31, 8);
    U_PUSHNIL(vm, regs + 0);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// lookup
static void fun_485(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[4];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1335, locals);
    U_STATEMENT(vm, regs + 0, 25, 8);
    regs[0] = locals[1]; // key
    U_BCALLRET1(vm, regs + 1, 148, 1); // hash
    regs[1] = locals[0]; // this
    U_PUSHFLD(vm, regs + 2, 0);
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_IMOD(vm, regs + 2);
    SetLVal(vm, &locals[2]); // h
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 26, 8);
    regs[0] = locals[0]; // this
    U_PUSHFLD(vm, regs + 1, 0);
    regs[1] = locals[2]; // h
    U_VPUSHIDXI(vm, regs + 2);
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[3]); // c
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 27, 8);
    block17811:;
    regs[0] = locals[3]; // c
    if (!U_JUMPFAIL(vm, regs + 1)) goto block17876;
    U_STATEMENT(vm, regs + 0, 28, 8);
    regs[0] = locals[3]; // c
    U_PUSHFLD(vm, regs + 1, 1);
    regs[1] = locals[1]; // key
    U_SEQ(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block17853;
    U_STATEMENT(vm, regs + 0, 29, 8);
    regs[0] = locals[3]; // c
    regs[1] = locals[2]; // h
    U_INCREF(vm, regs + 2, 1);
    U_RETURNLOCAL(vm, 0, 2);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    goto epilogue;
    block17853:;
    U_STATEMENT(vm, regs + 0, 30, 8);
    regs[0] = locals[3]; // c
    U_PUSHFLD(vm, regs + 1, 0);
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[3]); // c
    U_LV_WRITEREF(vm, regs + 1);
    goto block17811;
    block17876:;
    U_STATEMENT(vm, regs + 0, 31, 8);
    U_PUSHNIL(vm, regs + 0);
    regs[1] = locals[2]; // h
    U_RETURNLOCAL(vm, 0, 2);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// lookup
static void fun_480(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[4];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1344, locals);
    U_STATEMENT(vm, regs + 0, 25, 8);
    regs[0] = locals[1]; // key
    U_BCALLRET1(vm, regs + 1, 150, 1); // hash
    regs[1] = locals[0]; // this
    U_PUSHFLD(vm, regs + 2, 0);
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_IMOD(vm, regs + 2);
    SetLVal(vm, &locals[2]); // h
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 26, 8);
    regs[0] = locals[0]; // this
    U_PUSHFLD(vm, regs + 1, 0);
    regs[1] = locals[2]; // h
    U_VPUSHIDXI(vm, regs + 2);
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[3]); // c
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 27, 8);
    block17959:;
    regs[0] = locals[3]; // c
    if (!U_JUMPFAIL(vm, regs + 1)) goto block18024;
    U_STATEMENT(vm, regs + 0, 28, 8);
    regs[0] = locals[3]; // c
    U_PUSHFLD(vm, regs + 1, 1);
    regs[1] = locals[1]; // key
    U_FEQ(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block18001;
    U_STATEMENT(vm, regs + 0, 29, 8);
    regs[0] = locals[3]; // c
    regs[1] = locals[2]; // h
    U_INCREF(vm, regs + 2, 1);
    U_RETURNLOCAL(vm, 0, 2);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    goto epilogue;
    block18001:;
    U_STATEMENT(vm, regs + 0, 30, 8);
    regs[0] = locals[3]; // c
    U_PUSHFLD(vm, regs + 1, 0);
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[3]); // c
    U_LV_WRITEREF(vm, regs + 1);
    goto block17959;
    block18024:;
    U_STATEMENT(vm, regs + 0, 31, 8);
    U_PUSHNIL(vm, regs + 0);
    regs[1] = locals[2]; // h
    U_RETURNLOCAL(vm, 0, 2);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// lookup
static void fun_137(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[4];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1353, locals);
    U_STATEMENT(vm, regs + 0, 25, 8);
    regs[0] = locals[1]; // key
    U_BCALLRET1(vm, regs + 1, 150, 1); // hash
    regs[1] = locals[0]; // this
    U_PUSHFLD(vm, regs + 2, 0);
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_IMOD(vm, regs + 2);
    SetLVal(vm, &locals[2]); // h
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 26, 8);
    regs[0] = locals[0]; // this
    U_PUSHFLD(vm, regs + 1, 0);
    regs[1] = locals[2]; // h
    U_VPUSHIDXI(vm, regs + 2);
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[3]); // c
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 27, 8);
    block18107:;
    regs[0] = locals[3]; // c
    if (!U_JUMPFAIL(vm, regs + 1)) goto block18169;
    U_STATEMENT(vm, regs + 0, 28, 8);
    regs[0] = locals[3]; // c
    U_PUSHFLD(vm, regs + 1, 1);
    regs[1] = locals[1]; // key
    U_FEQ(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block18146;
    U_STATEMENT(vm, regs + 0, 29, 8);
    regs[0] = locals[3]; // c
    U_INCREF(vm, regs + 1, 0);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    block18146:;
    U_STATEMENT(vm, regs + 0, 30, 8);
    regs[0] = locals[3]; // c
    U_PUSHFLD(vm, regs + 1, 0);
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[3]); // c
    U_LV_WRITEREF(vm, regs + 1);
    goto block18107;
    block18169:;
    U_STATEMENT(vm, regs + 0, 31, 8);
    U_PUSHNIL(vm, regs + 0);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// set
static void fun_531(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[5];
    locals[0] = *(psp - 3);
    locals[1] = *(psp - 2);
    locals[2] = *(psp - 1);
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1362, locals);
    U_STATEMENT(vm, regs + 0, 34, 8);
    regs[0] = locals[0]; // this
    U_INCREF(vm, regs + 1, 0);
    regs[1] = locals[1]; // key
    U_INCREF(vm, regs + 2, 0);
    fun_532(vm, regs + 2); // call: lookup
    SetLVal(vm, &locals[4]); // h
    U_LV_WRITE(vm, regs + 2);
    SetLVal(vm, &locals[3]); // c
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 35, 8);
    regs[0] = locals[3]; // c
    if (!U_JUMPFAIL(vm, regs + 1)) goto block18251;
    U_STATEMENT(vm, regs + 0, 36, 8);
    regs[0] = locals[2]; // value
    regs[1] = locals[3]; // c
    U_LVAL_FLD(vm, regs + 2, 2);
    U_LV_WRITE(vm, regs + 1);
    goto block18297;
    block18251:;
    U_STATEMENT(vm, regs + 0, 38, 8);
    regs[0] = locals[0]; // this
    U_PUSHFLD(vm, regs + 1, 0);
    regs[1] = locals[4]; // h
    U_VPUSHIDXI(vm, regs + 2);
    U_INCREF(vm, regs + 1, 0);
    regs[1] = locals[1]; // key
    U_INCREF(vm, regs + 2, 0);
    regs[2] = locals[2]; // value
    U_NEWOBJECT(vm, regs + 3, 523); // chain
    regs[1] = locals[0]; // this
    U_PUSHFLD(vm, regs + 2, 0);
    regs[2] = locals[4]; // h
    U_LVAL_IDXVI(vm, regs + 3, 0);
    U_LV_WRITEREF(vm, regs + 1);
    block18297:;
    U_RETURNLOCAL(vm, 0, 0);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// set
static void fun_491(VMRef vm, StackPtr psp) {
    Value regs[7];
    Value locals[9];
    locals[0] = *(psp - 7);
    locals[1] = *(psp - 6);
    locals[2] = *(psp - 5);
    locals[3] = *(psp - 4);
    locals[4] = *(psp - 3);
    locals[5] = *(psp - 2);
    locals[6] = *(psp - 1);
    locals[7] = lobster::NilVal();
    locals[8] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1372, locals);
    U_STATEMENT(vm, regs + 0, 34, 8);
    regs[0] = locals[0]; // this
    U_INCREF(vm, regs + 1, 0);
    regs[1] = locals[1];regs[2] = locals[2];regs[3] = locals[3]; // key
    fun_492(vm, regs + 4); // call: lookup
    SetLVal(vm, &locals[8]); // h
    U_LV_WRITE(vm, regs + 2);
    SetLVal(vm, &locals[7]); // c
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 35, 8);
    regs[0] = locals[7]; // c
    if (!U_JUMPFAIL(vm, regs + 1)) goto block18377;
    U_STATEMENT(vm, regs + 0, 36, 8);
    regs[0] = locals[4];regs[1] = locals[5];regs[2] = locals[6]; // value
    regs[3] = locals[7]; // c
    U_LVAL_FLD(vm, regs + 4, 4);
    U_LV_WRITEV(vm, regs + 3, 3);
    goto block18422;
    block18377:;
    U_STATEMENT(vm, regs + 0, 38, 8);
    regs[0] = locals[0]; // this
    U_PUSHFLD(vm, regs + 1, 0);
    regs[1] = locals[8]; // h
    U_VPUSHIDXI(vm, regs + 2);
    U_INCREF(vm, regs + 1, 0);
    regs[1] = locals[1];regs[2] = locals[2];regs[3] = locals[3]; // key
    regs[4] = locals[4];regs[5] = locals[5];regs[6] = locals[6]; // value
    U_NEWOBJECT(vm, regs + 7, 668); // chain
    regs[1] = locals[0]; // this
    U_PUSHFLD(vm, regs + 2, 0);
    regs[2] = locals[8]; // h
    U_LVAL_IDXVI(vm, regs + 3, 0);
    U_LV_WRITEREF(vm, regs + 1);
    block18422:;
    U_RETURNLOCAL(vm, 0, 0);
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
static void fun_484(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[5];
    locals[0] = *(psp - 3);
    locals[1] = *(psp - 2);
    locals[2] = *(psp - 1);
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1386, locals);
    U_STATEMENT(vm, regs + 0, 34, 8);
    regs[0] = locals[0]; // this
    U_INCREF(vm, regs + 1, 0);
    regs[1] = locals[1]; // key
    U_INCREF(vm, regs + 2, 0);
    fun_485(vm, regs + 2); // call: lookup
    SetLVal(vm, &locals[4]); // h
    U_LV_WRITE(vm, regs + 2);
    SetLVal(vm, &locals[3]); // c
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 35, 8);
    regs[0] = locals[3]; // c
    if (!U_JUMPFAIL(vm, regs + 1)) goto block18501;
    U_STATEMENT(vm, regs + 0, 36, 8);
    regs[0] = locals[2]; // value
    U_INCREF(vm, regs + 1, 0);
    regs[1] = locals[3]; // c
    U_LVAL_FLD(vm, regs + 2, 2);
    U_LV_WRITEREF(vm, regs + 1);
    goto block18550;
    block18501:;
    U_STATEMENT(vm, regs + 0, 38, 8);
    regs[0] = locals[0]; // this
    U_PUSHFLD(vm, regs + 1, 0);
    regs[1] = locals[4]; // h
    U_VPUSHIDXI(vm, regs + 2);
    U_INCREF(vm, regs + 1, 0);
    regs[1] = locals[1]; // key
    U_INCREF(vm, regs + 2, 0);
    regs[2] = locals[2]; // value
    U_INCREF(vm, regs + 3, 0);
    U_NEWOBJECT(vm, regs + 3, 640); // chain
    regs[1] = locals[0]; // this
    U_PUSHFLD(vm, regs + 2, 0);
    regs[2] = locals[4]; // h
    U_LVAL_IDXVI(vm, regs + 3, 0);
    U_LV_WRITEREF(vm, regs + 1);
    block18550:;
    U_RETURNLOCAL(vm, 0, 0);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// set
static void fun_138(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[5];
    locals[0] = *(psp - 3);
    locals[1] = *(psp - 2);
    locals[2] = *(psp - 1);
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1396, locals);
    U_STATEMENT(vm, regs + 0, 34, 8);
    regs[0] = locals[0]; // this
    U_INCREF(vm, regs + 1, 0);
    regs[1] = locals[1]; // key
    fun_480(vm, regs + 2); // call: lookup
    SetLVal(vm, &locals[4]); // h
    U_LV_WRITE(vm, regs + 2);
    SetLVal(vm, &locals[3]); // c
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 35, 8);
    regs[0] = locals[3]; // c
    if (!U_JUMPFAIL(vm, regs + 1)) goto block18623;
    U_STATEMENT(vm, regs + 0, 36, 8);
    regs[0] = locals[2]; // value
    regs[1] = locals[3]; // c
    U_LVAL_FLD(vm, regs + 2, 2);
    U_LV_WRITE(vm, regs + 1);
    goto block18666;
    block18623:;
    U_STATEMENT(vm, regs + 0, 38, 8);
    regs[0] = locals[0]; // this
    U_PUSHFLD(vm, regs + 1, 0);
    regs[1] = locals[4]; // h
    U_VPUSHIDXI(vm, regs + 2);
    U_INCREF(vm, regs + 1, 0);
    regs[1] = locals[1]; // key
    regs[2] = locals[2]; // value
    U_NEWOBJECT(vm, regs + 3, 469); // chain
    regs[1] = locals[0]; // this
    U_PUSHFLD(vm, regs + 2, 0);
    regs[2] = locals[4]; // h
    U_LVAL_IDXVI(vm, regs + 3, 0);
    U_LV_WRITEREF(vm, regs + 1);
    block18666:;
    U_RETURNLOCAL(vm, 0, 0);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// get
static void fun_143(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[3];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1406, locals);
    U_STATEMENT(vm, regs + 0, 66, 8);
    regs[0] = locals[0]; // this
    U_INCREF(vm, regs + 1, 0);
    regs[1] = locals[1]; // key
    U_INCREF(vm, regs + 2, 0);
    fun_504(vm, regs + 2); // call: lookup
    SetLVal(vm, &locals[2]); // c
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 67, 8);
    regs[0] = locals[2]; // c
    if (!U_JUMPFAILR(vm, regs + 1)) goto block18726;
    U_POP(vm, regs + 1);
    regs[0] = locals[2]; // c
    U_PUSHFLD(vm, regs + 1, 2);
    block18726:;
    if (!U_JUMPNOFAILR(vm, regs + 1)) goto block18735;
    U_POP(vm, regs + 1);
    U_PUSHNIL(vm, regs + 0);
    block18735:;
    U_INCREF(vm, regs + 1, 0);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[2]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// get
static void fun_533(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[4];
    locals[0] = *(psp - 3);
    locals[1] = *(psp - 2);
    locals[2] = *(psp - 1);
    locals[3] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1414, locals);
    U_STATEMENT(vm, regs + 0, 73, 8);
    regs[0] = locals[0]; // this
    U_INCREF(vm, regs + 1, 0);
    regs[1] = locals[1]; // key
    U_INCREF(vm, regs + 2, 0);
    fun_534(vm, regs + 2); // call: lookup
    SetLVal(vm, &locals[3]); // c
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 74, 8);
    regs[0] = locals[3]; // c
    if (!U_JUMPFAIL(vm, regs + 1)) goto block18803;
    U_STATEMENT(vm, regs + 0, 74, 8);
    regs[0] = locals[3]; // c
    U_PUSHFLD(vm, regs + 1, 2);
    goto block18812;
    block18803:;
    U_STATEMENT(vm, regs + 0, 74, 8);
    regs[0] = locals[2]; // default_val
    block18812:;
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// get
static void fun_493(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[8];
    locals[0] = *(psp - 7);
    locals[1] = *(psp - 6);
    locals[2] = *(psp - 5);
    locals[3] = *(psp - 4);
    locals[4] = *(psp - 3);
    locals[5] = *(psp - 2);
    locals[6] = *(psp - 1);
    locals[7] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1423, locals);
    U_STATEMENT(vm, regs + 0, 73, 8);
    regs[0] = locals[0]; // this
    U_INCREF(vm, regs + 1, 0);
    regs[1] = locals[1];regs[2] = locals[2];regs[3] = locals[3]; // key
    fun_494(vm, regs + 4); // call: lookup
    SetLVal(vm, &locals[7]); // c
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 74, 8);
    regs[0] = locals[7]; // c
    if (!U_JUMPFAIL(vm, regs + 1)) goto block18880;
    U_STATEMENT(vm, regs + 0, 74, 8);
    regs[0] = locals[7]; // c
    U_PUSHFLD2V(vm, regs + 1, 4, 3);
    goto block18890;
    block18880:;
    U_STATEMENT(vm, regs + 0, 74, 8);
    regs[0] = locals[4];regs[1] = locals[5];regs[2] = locals[6]; // default_val
    block18890:;
    U_RETURNLOCAL(vm, 0, 3);
    DecVal(vm, locals[7]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    Push(psp, regs[2]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// get
static void fun_486(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[4];
    locals[0] = *(psp - 3);
    locals[1] = *(psp - 2);
    locals[2] = *(psp - 1);
    locals[3] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1436, locals);
    U_STATEMENT(vm, regs + 0, 73, 8);
    regs[0] = locals[0]; // this
    U_INCREF(vm, regs + 1, 0);
    regs[1] = locals[1]; // key
    U_INCREF(vm, regs + 2, 0);
    fun_487(vm, regs + 2); // call: lookup
    SetLVal(vm, &locals[3]); // c
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 74, 8);
    regs[0] = locals[3]; // c
    if (!U_JUMPFAIL(vm, regs + 1)) goto block18958;
    U_STATEMENT(vm, regs + 0, 74, 8);
    regs[0] = locals[3]; // c
    U_PUSHFLD(vm, regs + 1, 2);
    U_INCREF(vm, regs + 1, 0);
    goto block18970;
    block18958:;
    U_STATEMENT(vm, regs + 0, 74, 8);
    regs[0] = locals[2]; // default_val
    U_INCREF(vm, regs + 1, 0);
    block18970:;
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// get
static void fun_144(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[4];
    locals[0] = *(psp - 3);
    locals[1] = *(psp - 2);
    locals[2] = *(psp - 1);
    locals[3] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1445, locals);
    U_STATEMENT(vm, regs + 0, 73, 8);
    regs[0] = locals[0]; // this
    U_INCREF(vm, regs + 1, 0);
    regs[1] = locals[1]; // key
    fun_137(vm, regs + 2); // call: lookup
    SetLVal(vm, &locals[3]); // c
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 74, 8);
    regs[0] = locals[3]; // c
    if (!U_JUMPFAIL(vm, regs + 1)) goto block19032;
    U_STATEMENT(vm, regs + 0, 74, 8);
    regs[0] = locals[3]; // c
    U_PUSHFLD(vm, regs + 1, 2);
    goto block19041;
    block19032:;
    U_STATEMENT(vm, regs + 0, 74, 8);
    regs[0] = locals[2]; // default_val
    block19041:;
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// dictionary
static void fun_528(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1454, locals);
    U_STATEMENT(vm, regs + 0, 99, 8);
    regs[0] = locals[0]; // size
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_529(vm, regs + 2); // call: map
    U_NEWOBJECT(vm, regs + 1, 514); // dictionary
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// dictionary
static void fun_501(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1460, locals);
    U_STATEMENT(vm, regs + 0, 99, 8);
    regs[0] = locals[0]; // size
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_502(vm, regs + 2); // call: map
    U_NEWOBJECT(vm, regs + 1, 488); // dictionary
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// dictionary
static void fun_488(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1466, locals);
    U_STATEMENT(vm, regs + 0, 99, 8);
    regs[0] = locals[0]; // size
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_489(vm, regs + 2); // call: map
    U_NEWOBJECT(vm, regs + 1, 659); // dictionary
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// dictionary
static void fun_481(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1472, locals);
    U_STATEMENT(vm, regs + 0, 99, 8);
    regs[0] = locals[0]; // size
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_482(vm, regs + 2); // call: map
    U_NEWOBJECT(vm, regs + 1, 631); // dictionary
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// dictionary
static void fun_148(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1478, locals);
    U_STATEMENT(vm, regs + 0, 99, 8);
    regs[0] = locals[0]; // size
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_479(vm, regs + 2); // call: map
    U_NEWOBJECT(vm, regs + 1, 460); // dictionary
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// test_namespace.f
static void fun_150(VMRef vm, StackPtr psp) {
    Value regs[1];
    PushFunId(vm, funinfo_table + 1484, 0);
    U_STATEMENT(vm, regs + 0, 11, 6);
    U_STATEMENT(vm, regs + 0, 12, 6);
    U_PUSHINT(vm, regs + 0, 1);
    U_RETURNLOCAL(vm, 0, 1);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function149
static void fun_153(VMRef vm, StackPtr psp) {
    Value regs[12];
    Value keepvar[24];
    Value locals[72];
    locals[0] = lobster::NilVal();
    locals[1] = lobster::NilVal();
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    locals[5] = lobster::NilVal();
    locals[6] = lobster::NilVal();
    locals[7] = lobster::NilVal();
    locals[8] = lobster::NilVal();
    locals[9] = lobster::NilVal();
    locals[10] = lobster::NilVal();
    locals[11] = lobster::NilVal();
    locals[12] = lobster::NilVal();
    locals[13] = lobster::NilVal();
    locals[14] = lobster::NilVal();
    locals[15] = lobster::NilVal();
    locals[16] = lobster::NilVal();
    locals[17] = lobster::NilVal();
    locals[18] = lobster::NilVal();
    locals[19] = lobster::NilVal();
    locals[20] = lobster::NilVal();
    locals[21] = lobster::NilVal();
    locals[22] = lobster::NilVal();
    locals[23] = lobster::NilVal();
    locals[24] = lobster::NilVal();
    locals[25] = lobster::NilVal();
    locals[26] = lobster::NilVal();
    locals[27] = lobster::NilVal();
    locals[28] = lobster::NilVal();
    locals[29] = lobster::NilVal();
    locals[30] = lobster::NilVal();
    locals[31] = lobster::NilVal();
    locals[32] = lobster::NilVal();
    locals[33] = lobster::NilVal();
    locals[34] = lobster::NilVal();
    locals[35] = lobster::NilVal();
    locals[36] = lobster::NilVal();
    locals[37] = lobster::NilVal();
    locals[38] = lobster::NilVal();
    locals[39] = lobster::NilVal();
    locals[40] = lobster::NilVal();
    locals[41] = lobster::NilVal();
    locals[42] = lobster::NilVal();
    locals[43] = lobster::NilVal();
    locals[44] = lobster::NilVal();
    locals[45] = lobster::NilVal();
    locals[46] = lobster::NilVal();
    locals[47] = lobster::NilVal();
    locals[48] = lobster::NilVal();
    locals[49] = lobster::NilVal();
    locals[50] = lobster::NilVal();
    locals[51] = lobster::NilVal();
    locals[52] = lobster::NilVal();
    locals[53] = lobster::NilVal();
    locals[54] = lobster::NilVal();
    locals[55] = lobster::NilVal();
    locals[56] = lobster::NilVal();
    locals[57] = lobster::NilVal();
    locals[58] = lobster::NilVal();
    locals[59] = lobster::NilVal();
    locals[60] = lobster::NilVal();
    locals[61] = lobster::NilVal();
    locals[62] = lobster::NilVal();
    locals[63] = lobster::NilVal();
    locals[64] = lobster::NilVal();
    locals[65] = lobster::NilVal();
    locals[66] = lobster::NilVal();
    locals[67] = lobster::NilVal();
    locals[68] = lobster::NilVal();
    locals[69] = lobster::NilVal();
    locals[70] = lobster::NilVal();
    locals[71] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1489, locals);
    keepvar[0] = lobster::NilVal();
    keepvar[1] = lobster::NilVal();
    keepvar[2] = lobster::NilVal();
    keepvar[3] = lobster::NilVal();
    keepvar[4] = lobster::NilVal();
    keepvar[5] = lobster::NilVal();
    keepvar[6] = lobster::NilVal();
    keepvar[7] = lobster::NilVal();
    keepvar[8] = lobster::NilVal();
    keepvar[9] = lobster::NilVal();
    keepvar[10] = lobster::NilVal();
    keepvar[11] = lobster::NilVal();
    keepvar[12] = lobster::NilVal();
    keepvar[13] = lobster::NilVal();
    keepvar[14] = lobster::NilVal();
    keepvar[15] = lobster::NilVal();
    keepvar[16] = lobster::NilVal();
    keepvar[17] = lobster::NilVal();
    keepvar[18] = lobster::NilVal();
    keepvar[19] = lobster::NilVal();
    keepvar[20] = lobster::NilVal();
    keepvar[21] = lobster::NilVal();
    keepvar[22] = lobster::NilVal();
    keepvar[23] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 20, 6);
    U_STATEMENT(vm, regs + 0, 27, 6);
    U_PUSHINT(vm, regs + 0, 1);
    SetLVal(vm, &locals[0]); // vardef
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 28, 6);
    SetLVal(vm, &locals[0]); // vardef
    U_LV_IPP(vm, regs + 0);
    U_STATEMENT(vm, regs + 0, 29, 6);
    U_PUSHINT(vm, regs + 0, 1);
    SetLVal(vm, &locals[1]); // constdef
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 32, 6);
    fun_150(vm, regs + 0); // call: test_namespace.f
    U_PUSHINT(vm, regs + 1, 1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 32, 6, 154);
    U_STATEMENT(vm, regs + 0, 33, 6);
    U_PUSHINT(vm, regs + 0, 1);
    U_NEWOBJECT(vm, regs + 1, 544); // test_namespace.g
    keepvar[0] = TopM(regs + 1, 0);
    U_PUSHFLD(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 1, 1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 33, 6, 155);
    U_STATEMENT(vm, regs + 0, 34, 6);
    U_STATEMENT(vm, regs + 0, 34, 6);
    U_PUSHINT(vm, regs + 0, 1);
    U_NEWOBJECT(vm, regs + 1, 544); // test_namespace.g
    SetLVal(vm, &locals[59]); // this
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 15, 6);
    U_PUSHINT(vm, regs + 0, 2);
    U_PUSHINT(vm, regs + 1, 2);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 34, 6, 156);
    U_STATEMENT(vm, regs + 0, 36, 6);
    U_STATEMENT(vm, regs + 0, 36, 6);
    U_STATEMENT(vm, regs + 0, 38, 6);
    U_STATEMENT(vm, regs + 0, 38, 6);
    U_STATEMENT(vm, regs + 0, 42, 6);
    U_STATEMENT(vm, regs + 0, 43, 6);
    U_STATEMENT(vm, regs + 0, 44, 6);
    U_STATEMENT(vm, regs + 0, 45, 6);
    U_STATEMENT(vm, regs + 0, 48, 6);
    U_STATEMENT(vm, regs + 0, 48, 6);
    U_PUSHINT(vm, regs + 0, 1);
    SetLVal(vm, &locals[60]); // x
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 48, 6);
    U_PUSHINT(vm, regs + 0, 0);
    SetLVal(vm, &locals[61]); // y
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 42, 6);
    U_PUSHINT(vm, regs + 0, 1);
    U_PUSHINT(vm, regs + 1, 1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 48, 6, 157);
    U_STATEMENT(vm, regs + 0, 49, 6);
    U_STATEMENT(vm, regs + 0, 49, 6);
    U_PUSHSTR(vm, regs + 0, 158); // ""
    SetLVal(vm, &locals[62]); // x
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 49, 6);
    U_PUSHINT(vm, regs + 0, 0);
    SetLVal(vm, &locals[63]); // y
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 43, 6);
    U_PUSHINT(vm, regs + 0, 2);
    U_PUSHINT(vm, regs + 1, 2);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 49, 6, 159);
    U_STATEMENT(vm, regs + 0, 50, 6);
    U_PUSHINT(vm, regs + 0, 0);
    U_PUSHINT(vm, regs + 1, 0);
    U_NEWOBJECT(vm, regs + 2, 562); // testb
    U_PUSHINT(vm, regs + 1, 4);
    fun_157(vm, regs + 2); // call: tf
    U_PUSHINT(vm, regs + 1, 4);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 50, 6, 160);
    U_STATEMENT(vm, regs + 0, 53, 6);
    U_STATEMENT(vm, regs + 0, 54, 6);
    U_STATEMENT(vm, regs + 0, 54, 6);
    U_PUSHINT(vm, regs + 0, 0);
    U_PUSHINT(vm, regs + 1, 0);
    U_NEWOBJECT(vm, regs + 2, 562); // testb
    SetLVal(vm, &locals[64]); // a
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 53, 6);
    regs[0] = locals[64]; // a
    U_INCREF(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 1, 1);
    U_DDCALL(vm, regs + 2, 0, 1); vm.next_call_target(vm, regs + 2);
    U_PUSHINT(vm, regs + 1, 4);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 54, 6, 161);
    U_STATEMENT(vm, regs + 0, 56, 6);
    U_STATEMENT(vm, regs + 0, 56, 6);
    U_STATEMENT(vm, regs + 0, 68, 6);
    U_PUSHINT(vm, regs + 0, 0);
    U_PUSHINT(vm, regs + 1, 1);
    U_PUSHFLT(vm, regs + 2, 1073741824);
    U_PUSHFLT(vm, regs + 3, 1077936128);
    U_PUSHFLT(vm, regs + 4, 1082130432);
    U_PUSHFLT(vm, regs + 5, 1084227584);
    U_PUSHSTR(vm, regs + 6, 162); // "hello, world!\n\"\'\r\t\\\xC0"
    U_INCREF(vm, regs + 7, 0);
    U_PUSHINT(vm, regs + 7, 0);
    U_PUSHINT(vm, regs + 8, -64);
    U_NEWVEC(vm, regs + 9, 7, 2);
    U_PUSHNIL(vm, regs + 8);
    U_PUSHINT(vm, regs + 9, 1);
    U_PUSHINT(vm, regs + 10, 6);
    U_PUSHINT(vm, regs + 11, 7);
    U_NEWOBJECT(vm, regs + 12, 562); // testb
    U_NEWOBJECT(vm, regs + 11, 574); // parsetest
    SetLVal(vm, &locals[2]); // direct
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 72, 6);
    U_PUSHINT(vm, regs + 0, 574);
    regs[1] = locals[2]; // direct
    U_A2S(vm, regs + 2, 574);
    keepvar[1] = TopM(regs + 2, 0);
    U_BCALLRETV(vm, regs + 2, 253, 1); // parse_data
    SetLVal(vm, &locals[4]); // err
    U_LV_WRITEREF(vm, regs + 2);
    SetLVal(vm, &locals[3]); // parsed
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 73, 6);
    regs[0] = locals[4]; // err
    if (!U_JUMPFAIL(vm, regs + 1)) goto block19764;
    U_STATEMENT(vm, regs + 0, 73, 6);
    regs[0] = locals[4]; // err
    U_BCALLRET1(vm, regs + 1, 0, 0); // print
    block19764:;
    U_STATEMENT(vm, regs + 0, 74, 6);
    regs[0] = locals[3]; // parsed
    regs[1] = locals[2]; // direct
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 74, 6, 163);
    U_STATEMENT(vm, regs + 0, 76, 6);
    U_PUSHINT(vm, regs + 0, 574);
    U_PUSHSTR(vm, regs + 1, 164); // "parsetest { true, 1, 1.0, float3 {}, \"\", [], nil, "
    U_BCALLRETV(vm, regs + 2, 253, 1); // parse_data
    SetLVal(vm, &locals[4]); // err
    U_LV_WRITEREF(vm, regs + 2);
    SetLVal(vm, &locals[3]); // parsed
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 77, 6);
    regs[0] = locals[4]; // err
    if (!U_JUMPFAIL(vm, regs + 1)) goto block19830;
    U_STATEMENT(vm, regs + 0, 77, 6);
    regs[0] = locals[4]; // err
    U_BCALLRET1(vm, regs + 1, 0, 0); // print
    block19830:;
    U_STATEMENT(vm, regs + 0, 78, 6);
    U_PUSHINT(vm, regs + 0, 1);
    U_PUSHINT(vm, regs + 1, 1);
    U_PUSHFLT(vm, regs + 2, 1065353216);
    U_PUSHFLT(vm, regs + 3, 0);
    U_PUSHFLT(vm, regs + 4, 0);
    U_PUSHFLT(vm, regs + 5, 0);
    U_PUSHSTR(vm, regs + 6, 165); // ""
    U_INCREF(vm, regs + 7, 0);
    U_NEWVEC(vm, regs + 7, 7, 0);
    U_PUSHNIL(vm, regs + 8);
    U_PUSHINT(vm, regs + 9, -1);
    U_PUSHINT(vm, regs + 10, 6);
    U_PUSHINT(vm, regs + 11, 7);
    U_NEWOBJECT(vm, regs + 12, 562); // testb
    U_NEWOBJECT(vm, regs + 11, 574); // parsetest
    SetLVal(vm, &locals[5]); // groundv
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 79, 6);
    regs[0] = locals[3]; // parsed
    regs[1] = locals[5]; // groundv
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 79, 6, 166);
    U_STATEMENT(vm, regs + 0, 80, 6);
    regs[0] = locals[3]; // parsed
    U_PUSHINT(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 2, 0);
    U_BCALLRET3(vm, regs + 3, 254, 1); // flexbuffers_value_to_binary
    SetLVal(vm, &locals[6]); // flex
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 81, 6);
    regs[0] = locals[6]; // flex
    U_PUSHINT(vm, regs + 1, 0);
    U_PUSHSTR(vm, regs + 2, 167); // ""
    U_BCALLRETV(vm, regs + 3, 256, 1); // flexbuffers_binary_to_json
    U_POPREF(vm, regs + 2);
    SetLVal(vm, &locals[7]); // json
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 82, 6);
    regs[0] = locals[7]; // json
    U_ASSERT(vm, regs + 1, 82, 6, 168);
    U_STATEMENT(vm, regs + 0, 84, 6);
    U_PUSHSTR(vm, regs + 0, 169); // "{ a: 1, b: 1.0, d: \"\", e: [  ], g: -1, h: 1, s: { "
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[8]); // groundj
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 85, 6);
    regs[0] = locals[7]; // json
    regs[1] = locals[8]; // groundj
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 85, 6, 170);
    U_STATEMENT(vm, regs + 0, 86, 6);
    regs[0] = locals[7]; // json
    U_PUSHNIL(vm, regs + 1);
    U_BCALLRET2(vm, regs + 2, 257, 1); // flexbuffers_json_to_binary
    SetLVal(vm, &locals[10]); // ferr
    U_LV_WRITEREF(vm, regs + 2);
    SetLVal(vm, &locals[9]); // flex2
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 87, 6);
    regs[0] = locals[10]; // ferr
    U_LOGNOTREF(vm, regs + 1);
    U_ASSERT(vm, regs + 1, 87, 6, 171);
    U_STATEMENT(vm, regs + 0, 88, 6);
    regs[0] = locals[9]; // flex2
    U_PUSHINT(vm, regs + 1, 0);
    U_PUSHSTR(vm, regs + 2, 172); // ""
    U_BCALLRETV(vm, regs + 3, 256, 1); // flexbuffers_binary_to_json
    U_POPREF(vm, regs + 2);
    SetLVal(vm, &locals[11]); // json2
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 89, 6);
    regs[0] = locals[11]; // json2
    U_ASSERT(vm, regs + 1, 89, 6, 173);
    U_STATEMENT(vm, regs + 0, 90, 6);
    regs[0] = locals[11]; // json2
    regs[1] = locals[8]; // groundj
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 90, 6, 174);
    U_STATEMENT(vm, regs + 0, 91, 6);
    U_PUSHINT(vm, regs + 0, 574);
    regs[1] = locals[9]; // flex2
    U_BCALLRETV(vm, regs + 2, 255, 1); // flexbuffers_binary_to_value
    SetLVal(vm, &locals[13]); // fverr
    U_LV_WRITEREF(vm, regs + 2);
    SetLVal(vm, &locals[12]); // fval
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 92, 6);
    regs[0] = locals[13]; // fverr
    U_LOGNOTREF(vm, regs + 1);
    U_ASSERT(vm, regs + 1, 92, 6, 175);
    U_STATEMENT(vm, regs + 0, 93, 6);
    regs[0] = locals[12]; // fval
    regs[1] = locals[5]; // groundv
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 93, 6, 176);
    U_STATEMENT(vm, regs + 0, 94, 6);
    regs[0] = locals[3]; // parsed
    U_BCALLRET1(vm, regs + 1, 258, 1); // lobster_value_to_binary
    SetLVal(vm, &locals[14]); // lb
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 95, 6);
    U_PUSHINT(vm, regs + 0, 574);
    regs[1] = locals[14]; // lb
    U_BCALLRETV(vm, regs + 2, 259, 1); // lobster_binary_to_value
    SetLVal(vm, &locals[16]); // lberr
    U_LV_WRITEREF(vm, regs + 2);
    SetLVal(vm, &locals[15]); // lbval
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 96, 6);
    regs[0] = locals[16]; // lberr
    U_LOGNOTREF(vm, regs + 1);
    U_ASSERT(vm, regs + 1, 96, 6, 177);
    U_STATEMENT(vm, regs + 0, 97, 6);
    regs[0] = locals[15]; // lbval
    regs[1] = locals[5]; // groundv
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 97, 6, 178);
    U_STATEMENT(vm, regs + 0, 99, 6);
    U_PUSHINT(vm, regs + 0, 544);
    U_PUSHSTR(vm, regs + 1, 179); // "test_namespace.g {}"
    U_BCALLRETV(vm, regs + 2, 253, 1); // parse_data
    U_POPREF(vm, regs + 2);
    keepvar[2] = TopM(regs + 1, 0);
    U_ASSERT(vm, regs + 1, 99, 6, 180);
    U_STATEMENT(vm, regs + 0, 101, 6);
    U_PUSHINT(vm, regs + 0, 2);
    U_BCALLRET1(vm, regs + 1, 93, 1); // rnd
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 102, 6);
    U_PUSHINT(vm, regs + 0, 1);
    U_ASSERT(vm, regs + 1, 101, 6, 181);
    U_STATEMENT(vm, regs + 0, 104, 6);
    U_PUSHINT(vm, regs + 0, 12518);
    U_PUSHINT(vm, regs + 1, 12540);
    U_PUSHINT(vm, regs + 2, 12470);
    U_PUSHINT(vm, regs + 3, 12540);
    U_PUSHINT(vm, regs + 4, 21029);
    U_PUSHINT(vm, regs + 5, 12469);
    U_PUSHINT(vm, regs + 6, 12452);
    U_PUSHINT(vm, regs + 7, 12488);
    U_NEWVEC(vm, regs + 8, 7, 8);
    SetLVal(vm, &locals[17]); // unicodetests
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 105, 6);
    regs[0] = locals[17]; // unicodetests
    U_BCALLRET1(vm, regs + 1, 43, 1); // unicode_to_string
    keepvar[3] = TopM(regs + 1, 0);
    U_BCALLRET1(vm, regs + 1, 44, 1); // string_to_unicode
    U_POP(vm, regs + 2);
    keepvar[4] = TopM(regs + 1, 0);
    regs[1] = locals[17]; // unicodetests
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 105, 6, 182);
    U_STATEMENT(vm, regs + 0, 107, 6);
    U_PUSHINT(vm, regs + 0, 10);
    U_PUSHFUN(vm, regs + 1, 0, fun_159);
    fun_7(vm, regs + 2); // call: map
    SetLVal(vm, &locals[18]); // st
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 115, 6);
    regs[0] = locals[18]; // st
    U_PUSHSTR(vm, regs + 1, 183); // "what?"
    U_INCREF(vm, regs + 2, 0);
    U_PUSHSTR(vm, regs + 2, 184); // "no"
    U_INCREF(vm, regs + 3, 0);
    U_PUSHSTR(vm, regs + 3, 185); // "yes2"
    U_INCREF(vm, regs + 4, 0);
    U_PUSHSTR(vm, regs + 4, 186); // "yes3"
    U_INCREF(vm, regs + 5, 0);
    U_PUSHSTR(vm, regs + 5, 187); // "maybe"
    U_INCREF(vm, regs + 6, 0);
    U_PUSHSTR(vm, regs + 6, 188); // "maybe"
    U_INCREF(vm, regs + 7, 0);
    U_PUSHSTR(vm, regs + 7, 189); // "maybe"
    U_INCREF(vm, regs + 8, 0);
    U_PUSHSTR(vm, regs + 8, 190); // "what?"
    U_INCREF(vm, regs + 9, 0);
    U_PUSHSTR(vm, regs + 9, 191); // "maybe"
    U_INCREF(vm, regs + 10, 0);
    U_PUSHSTR(vm, regs + 10, 192); // "what?"
    U_INCREF(vm, regs + 11, 0);
    U_NEWVEC(vm, regs + 11, 11, 10);
    keepvar[5] = TopM(regs + 2, 0);
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 115, 6, 193);
    U_STATEMENT(vm, regs + 0, 116, 6);
    regs[0] = locals[18]; // st
    U_PUSHINT(vm, regs + 1, 2);
    U_VPUSHIDXI(vm, regs + 2);
    U_DUP(vm, regs + 1);
    U_PUSHSTR(vm, regs + 2, 194); // "yes2"
    U_SEQ(vm, regs + 3);
    if (!U_JUMPFAIL(vm, regs + 2)) goto block20475;
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 117, 6);
    U_PUSHINT(vm, regs + 0, 1);
    goto block20515;
    block20475:;
    U_DUP(vm, regs + 1);
    regs[2] = locals[18]; // st
    U_PUSHINT(vm, regs + 3, 3);
    U_VPUSHIDXI(vm, regs + 4);
    U_SEQ(vm, regs + 3);
    if (!U_JUMPFAIL(vm, regs + 2)) goto block20504;
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 118, 6);
    U_PUSHINT(vm, regs + 0, 0);
    goto block20515;
    block20504:;
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 119, 6);
    U_PUSHINT(vm, regs + 0, 0);
    block20515:;
    U_ASSERT(vm, regs + 1, 116, 6, 195);
    U_STATEMENT(vm, regs + 0, 120, 6);
    U_PUSHFLT64(vm, regs + 0, 1374389535, 1074339512);
    U_DUP(vm, regs + 1);
    U_PUSHFLT(vm, regs + 2, 1092616192);
    U_FGE(vm, regs + 3);
    if (!U_JUMPFAIL(vm, regs + 2)) goto block20562;
    U_DUP(vm, regs + 1);
    U_PUSHFLT(vm, regs + 2, 1101004800);
    U_FLE(vm, regs + 3);
    if (!U_JUMPFAIL(vm, regs + 2)) goto block20562;
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 121, 6);
    U_PUSHINT(vm, regs + 0, 0);
    goto block20573;
    block20562:;
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 122, 6);
    U_PUSHINT(vm, regs + 0, 1);
    block20573:;
    U_ASSERT(vm, regs + 1, 120, 6, 196);
    U_STATEMENT(vm, regs + 0, 124, 6);
    U_STATEMENT(vm, regs + 0, 124, 6);
    U_PUSHFUN(vm, regs + 0, 0, fun_160);
    SetLVal(vm, &locals[65]); // f
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 253, 5);
    fun_160(vm, regs + 0); // call: function153
    U_STATEMENT(vm, regs + 0, 130, 6);
    U_PUSHINT(vm, regs + 0, 3);
    U_PUSHINT(vm, regs + 1, 9);
    U_PUSHINT(vm, regs + 2, 5);
    U_PUSHINT(vm, regs + 3, 4);
    U_PUSHINT(vm, regs + 4, 1);
    U_PUSHINT(vm, regs + 5, 3);
    U_PUSHINT(vm, regs + 6, 9);
    U_PUSHINT(vm, regs + 7, 5);
    U_PUSHINT(vm, regs + 8, 4);
    U_PUSHINT(vm, regs + 9, 1);
    U_NEWVEC(vm, regs + 10, 7, 10);
    SetLVal(vm, &locals[19]); // testvector
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 132, 6);
    regs[0] = locals[19]; // testvector
    U_INCREF(vm, regs + 1, 0);
    U_PUSHFUN(vm, regs + 1, 0, fun_161);
    fun_48(vm, regs + 2); // call: qsort
    SetLVal(vm, &locals[20]); // sorted1
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 133, 6);
    regs[0] = locals[19]; // testvector
    U_BCALLRET1(vm, regs + 1, 29, 1); // copy
    SetLVal(vm, &locals[21]); // sorted2
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 134, 6);
    regs[0] = locals[21]; // sorted2
    U_INCREF(vm, regs + 1, 0);
    U_PUSHFUN(vm, regs + 1, 0, fun_162);
    fun_52(vm, regs + 2); // call: insertion_sort
    U_STATEMENT(vm, regs + 0, 135, 6);
    regs[0] = locals[19]; // testvector
    U_BCALLRET1(vm, regs + 1, 29, 1); // copy
    SetLVal(vm, &locals[22]); // sorted3
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 136, 6);
    regs[0] = locals[22]; // sorted3
    U_PUSHFUN(vm, regs + 1, 0, fun_163);
    fun_50(vm, regs + 2); // call: qsort_in_place
    U_STATEMENT(vm, regs + 0, 138, 6);
    regs[0] = locals[20]; // sorted1
    U_PUSHINT(vm, regs + 1, 1);
    U_PUSHINT(vm, regs + 2, 1);
    U_PUSHINT(vm, regs + 3, 3);
    U_PUSHINT(vm, regs + 4, 3);
    U_PUSHINT(vm, regs + 5, 4);
    U_PUSHINT(vm, regs + 6, 4);
    U_PUSHINT(vm, regs + 7, 5);
    U_PUSHINT(vm, regs + 8, 5);
    U_PUSHINT(vm, regs + 9, 9);
    U_PUSHINT(vm, regs + 10, 9);
    U_NEWVEC(vm, regs + 11, 7, 10);
    keepvar[6] = TopM(regs + 2, 0);
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 138, 6, 197);
    U_STATEMENT(vm, regs + 0, 139, 6);
    regs[0] = locals[20]; // sorted1
    regs[1] = locals[21]; // sorted2
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 139, 6, 198);
    U_STATEMENT(vm, regs + 0, 140, 6);
    regs[0] = locals[20]; // sorted1
    regs[1] = locals[22]; // sorted3
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 140, 6, 199);
    U_STATEMENT(vm, regs + 0, 142, 6);
    regs[0] = locals[20]; // sorted1
    U_PUSHINT(vm, regs + 1, 1);
    U_BCALLRET2(vm, regs + 2, 23, 1); // binary_search
    SetLVal(vm, &locals[24]); // findex
    U_LV_WRITE(vm, regs + 2);
    SetLVal(vm, &locals[23]); // found
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 143, 6);
    regs[0] = locals[23]; // found
    U_PUSHINT(vm, regs + 1, 2);
    U_IEQ(vm, regs + 2);
    if (!U_JUMPFAILR(vm, regs + 1)) goto block20869;
    U_POP(vm, regs + 1);
    regs[0] = locals[24]; // findex
    U_PUSHINT(vm, regs + 1, 0);
    U_IEQ(vm, regs + 2);
    block20869:;
    U_ASSERT(vm, regs + 1, 143, 6, 200);
    U_STATEMENT(vm, regs + 0, 144, 6);
    regs[0] = locals[20]; // sorted1
    U_PUSHINT(vm, regs + 1, 9);
    U_BCALLRET2(vm, regs + 2, 23, 1); // binary_search
    SetLVal(vm, &locals[24]); // findex
    U_LV_WRITE(vm, regs + 2);
    SetLVal(vm, &locals[23]); // found
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 145, 6);
    regs[0] = locals[23]; // found
    U_PUSHINT(vm, regs + 1, 2);
    U_IEQ(vm, regs + 2);
    if (!U_JUMPFAILR(vm, regs + 1)) goto block20925;
    U_POP(vm, regs + 1);
    regs[0] = locals[24]; // findex
    U_PUSHINT(vm, regs + 1, 8);
    U_IEQ(vm, regs + 2);
    block20925:;
    U_ASSERT(vm, regs + 1, 145, 6, 201);
    U_STATEMENT(vm, regs + 0, 146, 6);
    regs[0] = locals[20]; // sorted1
    U_PUSHINT(vm, regs + 1, 3);
    U_BCALLRET2(vm, regs + 2, 23, 1); // binary_search
    SetLVal(vm, &locals[24]); // findex
    U_LV_WRITE(vm, regs + 2);
    SetLVal(vm, &locals[23]); // found
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 147, 6);
    regs[0] = locals[23]; // found
    U_PUSHINT(vm, regs + 1, 2);
    U_IEQ(vm, regs + 2);
    if (!U_JUMPFAILR(vm, regs + 1)) goto block20981;
    U_POP(vm, regs + 1);
    regs[0] = locals[24]; // findex
    U_PUSHINT(vm, regs + 1, 2);
    U_IEQ(vm, regs + 2);
    block20981:;
    U_ASSERT(vm, regs + 1, 147, 6, 202);
    U_STATEMENT(vm, regs + 0, 149, 6);
    U_PUSHINT(vm, regs + 0, 44);
    regs[1] = locals[19]; // testvector
    fun_29(vm, regs + 2); // call: sum
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 149, 6, 203);
    U_STATEMENT(vm, regs + 0, 150, 6);
    U_PUSHINT(vm, regs + 0, 264);
    regs[1] = locals[19]; // testvector
    U_PUSHFUN(vm, regs + 2, 0, fun_164);
    fun_468(vm, regs + 3); // call: map
    fun_469(vm, regs + 2); // call: sum
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 150, 6, 204);
    U_STATEMENT(vm, regs + 0, 153, 6);
    U_STATEMENT(vm, regs + 0, 154, 6);
    U_PUSHINT(vm, regs + 0, 7);
    fun_165(vm, regs + 1); // call: factorial
    U_PUSHINT(vm, regs + 1, 5040);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 154, 6, 205);
    U_STATEMENT(vm, regs + 0, 156, 6);
    U_PUSHFUN(vm, regs + 0, 0, fun_166);
    SetLVal(vm, &locals[25]); // applyto4
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 157, 6);
    U_PUSHINT(vm, regs + 0, 16);
    U_PUSHFUN(vm, regs + 1, 0, fun_167);
    fun_166(vm, regs + 2); // call: function159
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 157, 6, 206);
    U_STATEMENT(vm, regs + 0, 159, 6);
    U_PUSHSTR(vm, regs + 0, 207); // "abc"
    U_INCREF(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 1, 3);
    U_PUSHINT(vm, regs + 2, 122);
    U_PUSHINT(vm, regs + 3, 0);
    U_BCALLRET4(vm, regs + 4, 197, 1); // ensure_size
    keepvar[7] = TopM(regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 208); // "abc"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 159, 6, 209);
    U_STATEMENT(vm, regs + 0, 160, 6);
    U_PUSHSTR(vm, regs + 0, 210); // "abc"
    U_INCREF(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 1, 5);
    U_PUSHINT(vm, regs + 2, 122);
    U_PUSHINT(vm, regs + 3, 0);
    U_BCALLRET4(vm, regs + 4, 197, 1); // ensure_size
    keepvar[8] = TopM(regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 211); // "abczz"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 160, 6, 212);
    U_STATEMENT(vm, regs + 0, 161, 6);
    U_PUSHSTR(vm, regs + 0, 213); // "abc"
    U_INCREF(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 1, -5);
    U_PUSHINT(vm, regs + 2, 122);
    U_PUSHINT(vm, regs + 3, 0);
    U_BCALLRET4(vm, regs + 4, 197, 1); // ensure_size
    keepvar[9] = TopM(regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 214); // "zzabc"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 161, 6, 215);
    U_STATEMENT(vm, regs + 0, 162, 6);
    U_PUSHSTR(vm, regs + 0, 216); // ""
    U_INCREF(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 2, 1027);
    U_BCALLRET3(vm, regs + 3, 200, 1); // write_int16_le
    SetLVal(vm, &locals[27]); // ni
    U_LV_WRITE(vm, regs + 2);
    SetLVal(vm, &locals[26]); // ns
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 163, 6);
    regs[0] = locals[26]; // ns
    U_PUSHSTR(vm, regs + 1, 217); // "\x03\x04\x00\x00"
    U_SEQ(vm, regs + 2);
    if (!U_JUMPFAILR(vm, regs + 1)) goto block21256;
    U_POP(vm, regs + 1);
    regs[0] = locals[27]; // ni
    U_PUSHINT(vm, regs + 1, 2);
    U_IEQ(vm, regs + 2);
    block21256:;
    U_ASSERT(vm, regs + 1, 163, 6, 218);
    U_STATEMENT(vm, regs + 0, 164, 6);
    regs[0] = locals[26]; // ns
    U_PUSHINT(vm, regs + 1, 0);
    U_BCALLRET2(vm, regs + 2, 214, 1); // read_int32_le
    U_POP(vm, regs + 2);
    U_PUSHINT(vm, regs + 1, 1027);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 164, 6, 219);
    U_STATEMENT(vm, regs + 0, 165, 6);
    U_PUSHSTR(vm, regs + 0, 220); // ""
    U_INCREF(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 2, 2050);
    U_BCALLRET3(vm, regs + 3, 206, 1); // write_int16_le_back
    SetLVal(vm, &locals[27]); // ni
    U_LV_WRITE(vm, regs + 2);
    SetLVal(vm, &locals[26]); // ns
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 166, 6);
    regs[0] = locals[26]; // ns
    U_PUSHSTR(vm, regs + 1, 221); // "\x00\x00\x02\x08"
    U_SEQ(vm, regs + 2);
    if (!U_JUMPFAILR(vm, regs + 1)) goto block21344;
    U_POP(vm, regs + 1);
    regs[0] = locals[27]; // ni
    U_PUSHINT(vm, regs + 1, 2);
    U_IEQ(vm, regs + 2);
    block21344:;
    U_ASSERT(vm, regs + 1, 166, 6, 222);
    U_STATEMENT(vm, regs + 0, 167, 6);
    regs[0] = locals[26]; // ns
    U_PUSHINT(vm, regs + 1, 0);
    U_BCALLRET2(vm, regs + 2, 225, 1); // read_int16_le_back
    U_POP(vm, regs + 2);
    U_PUSHINT(vm, regs + 1, 2050);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 167, 6, 223);
    U_STATEMENT(vm, regs + 0, 169, 6);
    U_PUSHSTR(vm, regs + 0, 224); // "a"
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[28]); // s1
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 170, 6);
    U_PUSHSTR(vm, regs + 0, 225); // "b"
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[29]); // s2
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 171, 6);
    regs[0] = locals[28]; // s1
    if (!U_JUMPFAILR(vm, regs + 1)) goto block21422;
    U_POP(vm, regs + 1);
    regs[0] = locals[29]; // s2
    block21422:;
    regs[1] = locals[29]; // s2
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 171, 6, 226);
    U_STATEMENT(vm, regs + 0, 172, 6);
    regs[0] = locals[29]; // s2
    regs[1] = locals[28]; // s1
    U_INCREF(vm, regs + 2, 0);
    U_INCREF(vm, regs + 2, 1);
    SetLVal(vm, &locals[29]); // s2
    U_LV_WRITEREF(vm, regs + 2);
    SetLVal(vm, &locals[28]); // s1
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 174, 6);
    U_PUSHINT(vm, regs + 0, 5);
    SetLVal(vm, &locals[30]); // aa
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 175, 6);
    U_PUSHINT(vm, regs + 0, 2);
    SetLVal(vm, &locals[30]); // aa
    U_LV_IMOD(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 176, 6);
    regs[0] = locals[30]; // aa
    U_PUSHINT(vm, regs + 1, 1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 176, 6, 227);
    U_STATEMENT(vm, regs + 0, 177, 6);
    U_PUSHFLT(vm, regs + 0, -1061158912);
    U_PUSHFLT(vm, regs + 1, 0);
    U_PUSHFLT(vm, regs + 2, 1086324736);
    U_PUSHINT(vm, regs + 3, 3);
    U_BCALLRETV(vm, regs + 4, 118, 1); // sign
    U_PUSHINT(vm, regs + 3, -1);
    U_PUSHINT(vm, regs + 4, 0);
    U_PUSHINT(vm, regs + 5, 1);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 177, 6, 228);
    U_STATEMENT(vm, regs + 0, 180, 6);
    U_PUSHINT(vm, regs + 0, 1);
    U_PUSHINT(vm, regs + 1, 1);
    U_PUSHINT(vm, regs + 2, 1);
    U_PUSHINT(vm, regs + 3, 1);
    SetLVal(vm, &locals[34]); // ee
    U_LV_WRITE(vm, regs + 4);
    SetLVal(vm, &locals[33]); // dd
    U_LV_WRITE(vm, regs + 3);
    SetLVal(vm, &locals[32]); // cc
    U_LV_WRITE(vm, regs + 2);
    SetLVal(vm, &locals[31]); // bb
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 181, 6);
    regs[0] = locals[31]; // bb
    U_PUSHINT(vm, regs + 1, 1);
    U_IEQ(vm, regs + 2);
    if (!U_JUMPFAILR(vm, regs + 1)) goto block21599;
    U_POP(vm, regs + 1);
    regs[0] = locals[31]; // bb
    regs[1] = locals[32]; // cc
    U_IEQ(vm, regs + 2);
    block21599:;
    if (!U_JUMPFAILR(vm, regs + 1)) goto block21614;
    U_POP(vm, regs + 1);
    regs[0] = locals[33]; // dd
    regs[1] = locals[34]; // ee
    U_IEQ(vm, regs + 2);
    block21614:;
    U_ASSERT(vm, regs + 1, 181, 6, 229);
    U_STATEMENT(vm, regs + 0, 182, 6);
    U_PUSHINT(vm, regs + 0, 2);
    U_PUSHINT(vm, regs + 1, 3);
    SetLVal(vm, &locals[36]); // gg
    U_LV_WRITE(vm, regs + 2);
    SetLVal(vm, &locals[35]); // ff
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 183, 6);
    regs[0] = locals[35]; // ff
    U_PUSHINT(vm, regs + 1, 2);
    U_IEQ(vm, regs + 2);
    if (!U_JUMPFAILR(vm, regs + 1)) goto block21666;
    U_POP(vm, regs + 1);
    regs[0] = locals[36]; // gg
    U_PUSHINT(vm, regs + 1, 3);
    U_IEQ(vm, regs + 2);
    block21666:;
    U_ASSERT(vm, regs + 1, 183, 6, 230);
    U_STATEMENT(vm, regs + 0, 184, 6);
    U_PUSHINT(vm, regs + 0, 8);
    U_PUSHINT(vm, regs + 1, 9);
    SetLVal(vm, &locals[36]); // gg
    U_LV_WRITE(vm, regs + 2);
    SetLVal(vm, &locals[35]); // ff
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 185, 6);
    regs[0] = locals[35]; // ff
    U_PUSHINT(vm, regs + 1, 8);
    U_IEQ(vm, regs + 2);
    if (!U_JUMPFAILR(vm, regs + 1)) goto block21718;
    U_POP(vm, regs + 1);
    regs[0] = locals[36]; // gg
    U_PUSHINT(vm, regs + 1, 9);
    U_IEQ(vm, regs + 2);
    block21718:;
    U_ASSERT(vm, regs + 1, 185, 6, 231);
    U_STATEMENT(vm, regs + 0, 186, 6);
    U_STATEMENT(vm, regs + 0, 189, 6);
    fun_168(vm, regs + 0); // call: multiret
    SetLVal(vm, &locals[38]); // ii
    U_LV_WRITE(vm, regs + 2);
    SetLVal(vm, &locals[37]); // hh
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 190, 6);
    regs[0] = locals[37]; // hh
    U_PUSHINT(vm, regs + 1, 6);
    U_IEQ(vm, regs + 2);
    if (!U_JUMPFAILR(vm, regs + 1)) goto block21771;
    U_POP(vm, regs + 1);
    regs[0] = locals[38]; // ii
    U_PUSHINT(vm, regs + 1, 7);
    U_IEQ(vm, regs + 2);
    block21771:;
    U_ASSERT(vm, regs + 1, 190, 6, 232);
    U_STATEMENT(vm, regs + 0, 191, 6);
    fun_470(vm, regs + 0); // call: multiret
    SetLVal(vm, &locals[39]); // ll
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 192, 6);
    regs[0] = locals[39]; // ll
    U_PUSHINT(vm, regs + 1, 6);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 192, 6, 233);
    U_STATEMENT(vm, regs + 0, 193, 6);
    U_STATEMENT(vm, regs + 0, 194, 6);
    fun_169(vm, regs + 0); // call: multiretpassthru
    SetLVal(vm, &locals[41]); // kk
    U_LV_WRITE(vm, regs + 2);
    SetLVal(vm, &locals[40]); // jj
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 195, 6);
    regs[0] = locals[40]; // jj
    U_PUSHINT(vm, regs + 1, 6);
    U_IEQ(vm, regs + 2);
    if (!U_JUMPFAILR(vm, regs + 1)) goto block21853;
    U_POP(vm, regs + 1);
    regs[0] = locals[41]; // kk
    U_PUSHINT(vm, regs + 1, 7);
    U_IEQ(vm, regs + 2);
    block21853:;
    U_ASSERT(vm, regs + 1, 195, 6, 234);
    U_STATEMENT(vm, regs + 0, 196, 6);
    U_STATEMENT(vm, regs + 0, 197, 6);
    fun_170(vm, regs + 0); // call: multirettyped
    SetLVal(vm, &locals[43]); // mm
    U_LV_WRITEREF(vm, regs + 2);
    SetLVal(vm, &locals[42]); // nn
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 198, 6);
    regs[0] = locals[42]; // nn
    U_PUSHFLT(vm, regs + 1, 1065353216);
    U_FEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 198, 6, 235);
    U_STATEMENT(vm, regs + 0, 199, 6);
    regs[0] = locals[43]; // mm
    U_PUSHSTR(vm, regs + 1, 236); // "a"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 199, 6, 237);
    U_STATEMENT(vm, regs + 0, 203, 6);
    U_STATEMENT(vm, regs + 0, 200, 6);
    U_STATEMENT(vm, regs + 0, 200, 6);
    U_STATEMENT(vm, regs + 0, 205, 6);
    U_PUSHSTR(vm, regs + 0, 238); // ""
    U_INCREF(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 1, 0);
    U_NEWOBJECT(vm, regs + 2, 619); // mretfields
    fun_171(vm, regs + 1); // call: write
    U_STATEMENT(vm, regs + 0, 208, 6);
    U_STATEMENT(vm, regs + 0, 212, 6);
    U_PUSHINT(vm, regs + 0, 5);
    fun_172(vm, regs + 1); // call: union_return
    keepvar[10] = TopM(regs + 1, 0);
    U_ASSERT(vm, regs + 1, 212, 6, 239);
    U_STATEMENT(vm, regs + 0, 215, 6);
    U_STATEMENT(vm, regs + 0, 218, 6);
    U_STATEMENT(vm, regs + 0, 220, 6);
    fun_173(vm, regs + 0); // call: nlr1
    U_PUSHINT(vm, regs + 1, 1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 220, 6, 240);
    U_STATEMENT(vm, regs + 0, 221, 6);
    fun_471(vm, regs + 0); // call: nlr1
    U_STATEMENT(vm, regs + 0, 223, 6);
    U_PUSHFUN(vm, regs + 0, 0, fun_175);
    fun_473(vm, regs + 1); // call: do
    U_STATEMENT(vm, regs + 0, 228, 6);
    U_STATEMENT(vm, regs + 0, 235, 6);
    U_PUSHFUN(vm, regs + 0, 0, fun_179);
    fun_133(vm, regs + 1); // call: try
    U_STATEMENT(vm, regs + 0, 238, 6);
    U_PUSHINT(vm, regs + 0, -1);
    U_PUSHINT(vm, regs + 1, 10);
    block22035:;
    if (!U_IFOR(vm, regs + 2)) goto block22069;
    U_STATEMENT(vm, regs + 2, 239, 6);
    U_PUSHINT(vm, regs + 2, 7);
    fun_477(vm, regs + 3); // call: factorial
    U_STATEMENT(vm, regs + 2, 240, 6);
    regs[2] = locals[19]; // testvector
    U_PUSHINT(vm, regs + 3, 0);
    U_PUSHFUN(vm, regs + 4, 0, fun_180);
    fun_478(vm, regs + 5); // call: fold
    goto block22035;
    block22069:;
    U_STATEMENT(vm, regs + 0, 242, 6);
    regs[0] = locals[19]; // testvector
    U_PUSHINT(vm, regs + 1, 1);
    U_BCALLRETV(vm, regs + 2, 22, 0); // truncate
    U_STATEMENT(vm, regs + 0, 243, 6);
    regs[0] = locals[19]; // testvector
    U_PUSHINT(vm, regs + 1, 3);
    U_NEWVEC(vm, regs + 2, 7, 1);
    keepvar[11] = TopM(regs + 2, 0);
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 243, 6, 241);
    U_STATEMENT(vm, regs + 0, 245, 6);
    U_PUSHINT(vm, regs + 0, 0);
    U_PUSHINT(vm, regs + 1, 1);
    U_PUSHINT(vm, regs + 2, 0);
    U_BCALLRET3(vm, regs + 3, 105, 1); // in_range
    U_ASSERT(vm, regs + 1, 245, 6, 242);
    U_STATEMENT(vm, regs + 0, 246, 6);
    U_PUSHINT(vm, regs + 0, 0);
    U_PUSHINT(vm, regs + 1, 1);
    U_PUSHINT(vm, regs + 2, 0);
    U_BCALLRET3(vm, regs + 3, 105, 1); // in_range
    U_ASSERT(vm, regs + 1, 246, 6, 243);
    U_STATEMENT(vm, regs + 0, 249, 6);
    U_PUSHINT(vm, regs + 0, 10);
    U_PUSHINT(vm, regs + 1, 20);
    U_PUSHINT(vm, regs + 2, 3);
    U_PUSHINT(vm, regs + 3, 6);
    U_IVVMOD(vm, regs + 4, 2);
    U_PUSHINT(vm, regs + 2, 1);
    U_PUSHINT(vm, regs + 3, 2);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 249, 6, 244);
    U_STATEMENT(vm, regs + 0, 250, 6);
    U_PUSHINT(vm, regs + 0, 10);
    U_PUSHINT(vm, regs + 1, 20);
    U_PUSHINT(vm, regs + 2, 3);
    U_IVSMOD(vm, regs + 3, 2);
    U_PUSHINT(vm, regs + 2, 1);
    U_PUSHINT(vm, regs + 3, 2);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 250, 6, 245);
    U_STATEMENT(vm, regs + 0, 251, 6);
    U_PUSHFLT(vm, regs + 0, 1082130432);
    U_PUSHFLT(vm, regs + 1, 1069547520);
    U_FMOD(vm, regs + 2);
    U_PUSHFLT(vm, regs + 1, 1065353216);
    U_FSUB(vm, regs + 2);
    U_BCALLRET1(vm, regs + 1, 112, 1); // abs
    U_PUSHFLT64(vm, regs + 1, 1202590843, 1065646817);
    U_FLT(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 251, 6, 246);
    U_STATEMENT(vm, regs + 0, 252, 6);
    U_PUSHFLT64(vm, regs + 0, 858993459, 1076114227);
    U_PUSHFLT64(vm, regs + 1, -1717986918, 1077156249);
    U_PUSHFLT(vm, regs + 2, 1077936128);
    U_PUSHFLT(vm, regs + 3, 1086324736);
    U_FVVMOD(vm, regs + 4, 2);
    U_PUSHFLT64(vm, regs + 2, -1717986918, 1072798105);
    U_PUSHFLT64(vm, regs + 3, -858993459, 1073794252);
    U_FVVSUB(vm, regs + 4, 2);
    U_PUSHINT(vm, regs + 2, 2);
    U_BCALLRETV(vm, regs + 3, 114, 1); // abs
    U_PUSHINT(vm, regs + 2, 2);
    U_BCALLRETV(vm, regs + 3, 132, 1); // max
    U_PUSHFLT64(vm, regs + 1, 1202590843, 1065646817);
    U_FLT(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 252, 6, 247);
    U_STATEMENT(vm, regs + 0, 253, 6);
    U_PUSHFLT64(vm, regs + 0, 858993459, 1076114227);
    U_PUSHFLT64(vm, regs + 1, -1717986918, 1077156249);
    U_PUSHFLT(vm, regs + 2, 1077936128);
    U_FVSMOD(vm, regs + 3, 2);
    U_PUSHFLT64(vm, regs + 2, -1717986918, 1072798105);
    U_PUSHFLT64(vm, regs + 3, -858993459, 1073794252);
    U_FVVSUB(vm, regs + 4, 2);
    U_PUSHINT(vm, regs + 2, 2);
    U_BCALLRETV(vm, regs + 3, 114, 1); // abs
    U_PUSHINT(vm, regs + 2, 2);
    U_BCALLRETV(vm, regs + 3, 132, 1); // max
    U_PUSHFLT64(vm, regs + 1, 1202590843, 1065646817);
    U_FLT(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 253, 6, 248);
    U_STATEMENT(vm, regs + 0, 256, 6);
    U_PUSHSTR(vm, regs + 0, 249); // "\xE2\xBA\x86\xE3\x8B\x87\xE4\xAC\x88"
    U_PUSHSTR(vm, regs + 1, 250); // "\xE2\xBA\x86\xE3\x8B\x87\xE4\xAC\x88"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 256, 6, 251);
    U_STATEMENT(vm, regs + 0, 259, 6);
    U_PUSHINT(vm, regs + 0, 1);
    SetLVal(vm, &locals[44]); // bit
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 260, 6);
    U_PUSHINT(vm, regs + 0, 2);
    SetLVal(vm, &locals[44]); // bit
    U_LV_BINOR(vm, regs + 1);
    U_LV_DUP(vm, regs + 0);
    U_PUSHINT(vm, regs + 1, 3);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 260, 6, 252);
    U_STATEMENT(vm, regs + 0, 261, 6);
    U_PUSHINT(vm, regs + 0, -3);
    SetLVal(vm, &locals[44]); // bit
    U_LV_BINAND(vm, regs + 1);
    U_LV_DUP(vm, regs + 0);
    U_PUSHINT(vm, regs + 1, 1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 261, 6, 253);
    U_STATEMENT(vm, regs + 0, 262, 6);
    U_PUSHINT(vm, regs + 0, 3);
    SetLVal(vm, &locals[44]); // bit
    U_LV_XOR(vm, regs + 1);
    U_LV_DUP(vm, regs + 0);
    U_PUSHINT(vm, regs + 1, 2);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 262, 6, 254);
    U_STATEMENT(vm, regs + 0, 263, 6);
    U_PUSHINT(vm, regs + 0, 1);
    SetLVal(vm, &locals[44]); // bit
    U_LV_ASL(vm, regs + 1);
    U_LV_DUP(vm, regs + 0);
    U_PUSHINT(vm, regs + 1, 4);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 263, 6, 255);
    U_STATEMENT(vm, regs + 0, 264, 6);
    U_PUSHINT(vm, regs + 0, 1);
    SetLVal(vm, &locals[44]); // bit
    U_LV_ASR(vm, regs + 1);
    U_LV_DUP(vm, regs + 0);
    U_PUSHINT(vm, regs + 1, 2);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 264, 6, 256);
    U_STATEMENT(vm, regs + 0, 267, 6);
    U_PUSHINT(vm, regs + 0, 17);
    fun_148(vm, regs + 1); // call: dictionary
    SetLVal(vm, &locals[45]); // dict
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 269, 6);
    regs[0] = locals[45]; // dict
    U_PUSHFLT(vm, regs + 1, 1065353216);
    U_PUSHFLT(vm, regs + 2, -1082130432);
    fun_144(vm, regs + 3); // call: get
    U_PUSHFLT(vm, regs + 1, -1082130432);
    U_FEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 269, 6, 257);
    U_STATEMENT(vm, regs + 0, 270, 6);
    U_PUSHINT(vm, regs + 0, 0);
    U_BCALLRET1(vm, regs + 1, 97, 0); // rnd_seed
    U_STATEMENT(vm, regs + 0, 271, 6);
    U_PUSHINT(vm, regs + 0, -1);
    U_PUSHINT(vm, regs + 1, 32);
    block22573:;
    if (!U_IFOR(vm, regs + 2)) goto block22610;
    U_STATEMENT(vm, regs + 2, 272, 6);
    U_BCALLRET0(vm, regs + 2, 95, 1); // rnd_float
    SetLVal(vm, &locals[46]); // key
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 273, 6);
    regs[2] = locals[45]; // dict
    regs[3] = locals[46]; // key
    regs[4] = locals[46]; // key
    fun_138(vm, regs + 5); // call: set
    goto block22573;
    block22610:;
    U_STATEMENT(vm, regs + 0, 274, 6);
    U_PUSHINT(vm, regs + 0, 0);
    U_BCALLRET1(vm, regs + 1, 97, 0); // rnd_seed
    U_STATEMENT(vm, regs + 0, 275, 6);
    U_PUSHINT(vm, regs + 0, -1);
    U_PUSHINT(vm, regs + 1, 32);
    block22633:;
    if (!U_IFOR(vm, regs + 2)) goto block22692;
    U_STATEMENT(vm, regs + 2, 276, 6);
    U_BCALLRET0(vm, regs + 2, 95, 1); // rnd_float
    SetLVal(vm, &locals[47]); // key
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 277, 6);
    regs[2] = locals[45]; // dict
    regs[3] = locals[47]; // key
    U_PUSHFLT(vm, regs + 4, -1082130432);
    fun_144(vm, regs + 5); // call: get
    SetLVal(vm, &locals[48]); // value
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 278, 6);
    regs[2] = locals[48]; // value
    regs[3] = locals[47]; // key
    U_FEQ(vm, regs + 4);
    U_ASSERT(vm, regs + 3, 278, 6, 258);
    goto block22633;
    block22692:;
    U_STATEMENT(vm, regs + 0, 280, 6);
    U_PUSHINT(vm, regs + 0, 17);
    fun_481(vm, regs + 1); // call: dictionary
    SetLVal(vm, &locals[49]); // dict2
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 281, 6);
    regs[0] = locals[49]; // dict2
    U_PUSHSTR(vm, regs + 1, 259); // "a"
    U_PUSHSTR(vm, regs + 2, 260); // "b"
    fun_484(vm, regs + 3); // call: set
    U_STATEMENT(vm, regs + 0, 282, 6);
    regs[0] = locals[49]; // dict2
    U_PUSHSTR(vm, regs + 1, 261); // "a"
    U_PUSHNIL(vm, regs + 2);
    fun_486(vm, regs + 3); // call: get
    SetLVal(vm, &locals[50]); // val
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 283, 6);
    regs[0] = locals[50]; // val
    U_E2B(vm, regs + 1);
    if (!U_JUMPFAILR(vm, regs + 1)) goto block22767;
    U_POP(vm, regs + 1);
    regs[0] = locals[50]; // val
    U_PUSHSTR(vm, regs + 1, 262); // "b"
    U_SEQ(vm, regs + 2);
    block22767:;
    U_ASSERT(vm, regs + 1, 283, 6, 263);
    U_STATEMENT(vm, regs + 0, 285, 6);
    U_PUSHINT(vm, regs + 0, 17);
    fun_488(vm, regs + 1); // call: dictionary
    SetLVal(vm, &locals[51]); // dict3
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 286, 6);
    regs[0] = locals[51]; // dict3
    U_PUSHINT(vm, regs + 1, 1);
    U_PUSHINT(vm, regs + 2, 2);
    U_PUSHINT(vm, regs + 3, 3);
    U_PUSHINT(vm, regs + 4, 4);
    U_PUSHINT(vm, regs + 5, 5);
    U_PUSHINT(vm, regs + 6, 6);
    fun_491(vm, regs + 7); // call: set
    U_STATEMENT(vm, regs + 0, 287, 6);
    regs[0] = locals[51]; // dict3
    U_PUSHINT(vm, regs + 1, 1);
    U_PUSHINT(vm, regs + 2, 2);
    U_PUSHINT(vm, regs + 3, 3);
    U_PUSHVARVF(vm, regs + 4, 131, 3); // int3_0+0
    fun_493(vm, regs + 7); // call: get
    U_PUSHINT(vm, regs + 3, 4);
    U_PUSHINT(vm, regs + 4, 5);
    U_PUSHINT(vm, regs + 5, 6);
    U_STEQ(vm, regs + 6, 3);
    U_ASSERT(vm, regs + 1, 287, 6, 264);
    U_STATEMENT(vm, regs + 0, 290, 6);
    U_PUSHINT(vm, regs + 0, 0);
    SetLVal(vm, &locals[52]); // lc
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 291, 6);
    U_PUSHINT(vm, regs + 0, -1);
    U_PUSHINT(vm, regs + 1, 10);
    block22879:;
    if (!U_IFOR(vm, regs + 2)) goto block22935;
    U_STATEMENT(vm, regs + 2, 291, 6);
    U_IFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[53]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 292, 6);
    regs[2] = locals[53]; // i
    U_PUSHINT(vm, regs + 3, 5);
    U_IEQ(vm, regs + 4);
    if (!U_JUMPFAIL(vm, regs + 3)) goto block22921;
    U_STATEMENT(vm, regs + 2, 292, 6);
    U_POP(vm, regs + 2);
    U_POP(vm, regs + 1);
    goto block22937;
    block22921:;
    U_STATEMENT(vm, regs + 2, 293, 6);
    SetLVal(vm, &locals[52]); // lc
    U_LV_IPP(vm, regs + 2);
    goto block22879;
    block22935:;
    block22937:;
    U_STATEMENT(vm, regs + 0, 294, 6);
    regs[0] = locals[52]; // lc
    U_PUSHINT(vm, regs + 1, 5);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 294, 6, 265);
    U_STATEMENT(vm, regs + 0, 295, 6);
    block22960:;
    regs[0] = locals[52]; // lc
    if (!U_JUMPFAIL(vm, regs + 1)) goto block23004;
    U_STATEMENT(vm, regs + 0, 296, 6);
    regs[0] = locals[52]; // lc
    U_PUSHINT(vm, regs + 1, 1);
    U_IEQ(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block22990;
    U_STATEMENT(vm, regs + 0, 296, 6);
    goto block23006;
    block22990:;
    U_STATEMENT(vm, regs + 0, 297, 6);
    SetLVal(vm, &locals[52]); // lc
    U_LV_IMM(vm, regs + 0);
    goto block22960;
    block23004:;
    block23006:;
    U_STATEMENT(vm, regs + 0, 298, 6);
    regs[0] = locals[52]; // lc
    U_PUSHINT(vm, regs + 1, 1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 298, 6, 266);
    U_STATEMENT(vm, regs + 0, 301, 6);
    U_STATEMENT(vm, regs + 0, 310, 6);
    fun_181(vm, regs + 0); // call: outer
    U_STATEMENT(vm, regs + 0, 314, 6);
    U_STATEMENT(vm, regs + 0, 324, 6);
    U_PUSHINT(vm, regs + 0, 0);
    fun_183(vm, regs + 1); // call: outer_dest2
    U_PUSHINT(vm, regs + 2, 1);
    U_PUSHINT(vm, regs + 3, 0);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 324, 6, 267);
    U_STATEMENT(vm, regs + 0, 325, 6);
    U_PUSHINT(vm, regs + 0, 1);
    fun_183(vm, regs + 1); // call: outer_dest2
    U_PUSHINT(vm, regs + 2, 4);
    U_PUSHINT(vm, regs + 3, 5);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 325, 6, 268);
    U_STATEMENT(vm, regs + 0, 326, 6);
    U_PUSHINT(vm, regs + 0, 2);
    fun_183(vm, regs + 1); // call: outer_dest2
    U_PUSHINT(vm, regs + 2, 3);
    U_PUSHINT(vm, regs + 3, 0);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 326, 6, 269);
    U_STATEMENT(vm, regs + 0, 329, 6);
    U_STATEMENT(vm, regs + 0, 332, 6);
    U_STATEMENT(vm, regs + 0, 338, 6);
    U_PUSHINT(vm, regs + 0, 1);
    SetLVal(vm, &locals[54]); // dav
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 339, 6);
    U_STATEMENT(vm, regs + 0, 340, 6);
    U_STATEMENT(vm, regs + 0, 341, 6);
    U_STATEMENT(vm, regs + 0, 342, 6);
    U_STATEMENT(vm, regs + 0, 342, 6);
    U_PUSHINT(vm, regs + 0, 1);
    SetLVal(vm, &locals[66]); // a
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 341, 6);
    regs[0] = locals[66]; // a
    U_PUSHINT(vm, regs + 1, 1);
    U_PUSHINT(vm, regs + 2, 2);
    regs[3] = locals[54]; // dav
    U_PUSHINT(vm, regs + 4, 1);
    U_IADD(vm, regs + 5);
    fun_187(vm, regs + 4); // call: da
    U_IADD(vm, regs + 2);
    U_PUSHINT(vm, regs + 1, 1);
    U_PUSHINT(vm, regs + 2, 2);
    U_PUSHINT(vm, regs + 3, 3);
    fun_187(vm, regs + 4); // call: da
    U_IADD(vm, regs + 2);
    U_PUSHINT(vm, regs + 1, 12);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 342, 6, 270);
    U_STATEMENT(vm, regs + 0, 345, 6);
    U_PUSHSTR(vm, regs + 0, 271); // "abcdabcd"
    U_PUSHSTR(vm, regs + 1, 272); // "BC"
    U_PUSHINT(vm, regs + 2, 0);
    U_BCALLRET3(vm, regs + 3, 37, 1); // find_string
    U_PUSHINT(vm, regs + 1, -1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 345, 6, 273);
    U_STATEMENT(vm, regs + 0, 346, 6);
    U_PUSHSTR(vm, regs + 0, 274); // "abcdabcd"
    U_PUSHSTR(vm, regs + 1, 275); // "bc"
    U_PUSHINT(vm, regs + 2, 0);
    U_BCALLRET3(vm, regs + 3, 37, 1); // find_string
    U_PUSHINT(vm, regs + 1, 1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 346, 6, 276);
    U_STATEMENT(vm, regs + 0, 347, 6);
    U_PUSHSTR(vm, regs + 0, 277); // "abcdabcd"
    U_PUSHSTR(vm, regs + 1, 278); // "bc"
    U_PUSHINT(vm, regs + 2, 4);
    U_BCALLRET3(vm, regs + 3, 37, 1); // find_string
    U_PUSHINT(vm, regs + 1, 5);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 347, 6, 279);
    U_STATEMENT(vm, regs + 0, 348, 6);
    U_PUSHSTR(vm, regs + 0, 280); // "abcdabcd"
    U_PUSHSTR(vm, regs + 1, 281); // "bc"
    U_PUSHINT(vm, regs + 2, 0);
    U_BCALLRET3(vm, regs + 3, 38, 1); // find_string_reverse
    U_PUSHINT(vm, regs + 1, 5);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 348, 6, 282);
    U_STATEMENT(vm, regs + 0, 349, 6);
    U_PUSHSTR(vm, regs + 0, 283); // "abcdabcd"
    U_PUSHSTR(vm, regs + 1, 284); // "bc"
    U_PUSHINT(vm, regs + 2, 4);
    U_BCALLRET3(vm, regs + 3, 38, 1); // find_string_reverse
    U_PUSHINT(vm, regs + 1, 1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 349, 6, 285);
    U_STATEMENT(vm, regs + 0, 350, 6);
    U_PUSHSTR(vm, regs + 0, 286); // "abcdabcd"
    U_PUSHSTR(vm, regs + 1, 287); // "bc"
    U_PUSHSTR(vm, regs + 2, 288); // "X"
    U_PUSHINT(vm, regs + 3, 0);
    U_BCALLRET4(vm, regs + 4, 39, 1); // replace_string
    keepvar[12] = TopM(regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 289); // "aXdaXd"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 350, 6, 290);
    U_STATEMENT(vm, regs + 0, 351, 6);
    U_PUSHSTR(vm, regs + 0, 291); // "abcdabcd"
    U_PUSHSTR(vm, regs + 1, 292); // "bc"
    U_PUSHSTR(vm, regs + 2, 293); // "X"
    U_PUSHINT(vm, regs + 3, 1);
    U_BCALLRET4(vm, regs + 4, 39, 1); // replace_string
    keepvar[13] = TopM(regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 294); // "aXdabcd"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 351, 6, 295);
    U_STATEMENT(vm, regs + 0, 353, 6);
    U_PUSHSTR(vm, regs + 0, 296); // " A | B C |"
    U_PUSHSTR(vm, regs + 1, 297); // "|"
    U_PUSHSTR(vm, regs + 2, 298); // " "
    U_PUSHINT(vm, regs + 3, 1);
    U_BCALLRET4(vm, regs + 4, 42, 1); // tokenize
    keepvar[14] = TopM(regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 299); // "A"
    U_INCREF(vm, regs + 2, 0);
    U_PUSHSTR(vm, regs + 2, 300); // "B C"
    U_INCREF(vm, regs + 3, 0);
    U_PUSHSTR(vm, regs + 3, 301); // ""
    U_INCREF(vm, regs + 4, 0);
    U_NEWVEC(vm, regs + 4, 11, 3);
    keepvar[15] = TopM(regs + 2, 0);
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 353, 6, 302);
    U_STATEMENT(vm, regs + 0, 354, 6);
    U_PUSHSTR(vm, regs + 0, 303); // "; A ; B C;; "
    U_PUSHSTR(vm, regs + 1, 304); // ";"
    U_PUSHSTR(vm, regs + 2, 305); // " "
    U_PUSHINT(vm, regs + 3, 0);
    U_BCALLRET4(vm, regs + 4, 42, 1); // tokenize
    keepvar[16] = TopM(regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 306); // ""
    U_INCREF(vm, regs + 2, 0);
    U_PUSHSTR(vm, regs + 2, 307); // "A"
    U_INCREF(vm, regs + 3, 0);
    U_PUSHSTR(vm, regs + 3, 308); // "B C"
    U_INCREF(vm, regs + 4, 0);
    U_PUSHSTR(vm, regs + 4, 309); // ""
    U_INCREF(vm, regs + 5, 0);
    U_NEWVEC(vm, regs + 5, 11, 4);
    keepvar[17] = TopM(regs + 2, 0);
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 354, 6, 310);
    U_STATEMENT(vm, regs + 0, 355, 6);
    U_PUSHSTR(vm, regs + 0, 311); // ", A , B C,, "
    U_PUSHSTR(vm, regs + 1, 312); // ","
    U_PUSHSTR(vm, regs + 2, 313); // " "
    U_PUSHINT(vm, regs + 3, 1);
    U_BCALLRET4(vm, regs + 4, 42, 1); // tokenize
    keepvar[18] = TopM(regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 314); // ""
    U_INCREF(vm, regs + 2, 0);
    U_PUSHSTR(vm, regs + 2, 315); // "A"
    U_INCREF(vm, regs + 3, 0);
    U_PUSHSTR(vm, regs + 3, 316); // "B C"
    U_INCREF(vm, regs + 4, 0);
    U_PUSHSTR(vm, regs + 4, 317); // ""
    U_INCREF(vm, regs + 5, 0);
    U_PUSHSTR(vm, regs + 5, 318); // ""
    U_INCREF(vm, regs + 6, 0);
    U_NEWVEC(vm, regs + 6, 11, 5);
    keepvar[19] = TopM(regs + 2, 0);
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 355, 6, 319);
    U_STATEMENT(vm, regs + 0, 358, 6);
    U_STATEMENT(vm, regs + 0, 360, 6);
    U_STATEMENT(vm, regs + 0, 364, 6);
    fun_190(vm, regs + 0); // call: G
    U_PUSHINT(vm, regs + 1, 5);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 364, 6, 320);
    U_STATEMENT(vm, regs + 0, 367, 6);
    U_PUSHSTR(vm, regs + 0, 321); // "a"
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[55]); // slv
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 368, 6);
    U_PUSHSTR(vm, regs + 0, 322); // "b"
    SetLVal(vm, &locals[55]); // slv
    U_LV_SADD(vm, regs + 1);
    U_LV_DUP(vm, regs + 0);
    U_PUSHSTR(vm, regs + 1, 323); // "ab"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 368, 6, 324);
    U_STATEMENT(vm, regs + 0, 369, 6);
    U_PUSHINT(vm, regs + 0, 1);
    U_NEWVEC(vm, regs + 1, 7, 1);
    SetLVal(vm, &locals[56]); // vlv
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 370, 6);
    U_PUSHINT(vm, regs + 0, 2);
    U_NEWVEC(vm, regs + 1, 7, 1);
    SetLVal(vm, &locals[56]); // vlv
    U_LV_WRITEREF(vm, regs + 1);
    U_LV_DUP(vm, regs + 0);
    U_PUSHINT(vm, regs + 1, 2);
    U_NEWVEC(vm, regs + 2, 7, 1);
    keepvar[20] = TopM(regs + 2, 0);
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 370, 6, 325);
    U_STATEMENT(vm, regs + 0, 371, 6);
    U_STATEMENT(vm, regs + 0, 371, 6);
    U_STATEMENT(vm, regs + 0, 374, 6);
    U_PUSHSTR(vm, regs + 0, 326); // "a"
    U_INCREF(vm, regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 327); // "a"
    U_INCREF(vm, regs + 2, 0);
    U_NEWVEC(vm, regs + 2, 11, 1);
    SetLVal(vm, &locals[57]); // stlv+0
    U_LV_WRITEREFV(vm, regs + 2, 2, 3);
    U_STATEMENT(vm, regs + 0, 375, 6);
    U_PUSHSTR(vm, regs + 0, 328); // "b"
    U_INCREF(vm, regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 329); // "b"
    U_INCREF(vm, regs + 2, 0);
    U_NEWVEC(vm, regs + 2, 11, 1);
    SetLVal(vm, &locals[57]); // stlv+0
    U_LV_WRITEREFV(vm, regs + 2, 2, 3);
    U_LV_DUPV(vm, regs + 0, 2);
    U_PUSHFLDV(vm, regs + 2, 1, 2);
    U_PUSHINT(vm, regs + 1, 0);
    U_VPUSHIDXI(vm, regs + 2);
    U_PUSHSTR(vm, regs + 1, 330); // "b"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 375, 6, 331);
    U_STATEMENT(vm, regs + 0, 378, 6);
    U_PUSHFLT(vm, regs + 0, 2139095040);
    U_A2S(vm, regs + 1, 2);
    keepvar[21] = TopM(regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 332); // "inf"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 378, 6, 333);
    U_STATEMENT(vm, regs + 0, 379, 6);
    U_PUSHFLT(vm, regs + 0, 1065353216);
    U_PUSHFLT(vm, regs + 1, 0);
    U_FMOD(vm, regs + 2);
    U_A2S(vm, regs + 1, 2);
    keepvar[22] = TopM(regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 334); // "nan"
    U_PUSHINT(vm, regs + 2, 0);
    U_BCALLRET3(vm, regs + 3, 37, 1); // find_string
    U_PUSHINT(vm, regs + 1, 0);
    U_IGE(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 379, 6, 335);
    U_STATEMENT(vm, regs + 0, 380, 6);
    U_PUSHVARVF(vm, regs + 0, 76, 2); // float2_1+0
    U_PUSHVARVF(vm, regs + 2, 74, 2); // float2_0+0
    U_FVVDIV(vm, regs + 4, 2);
    U_PUSHFLDV(vm, regs + 2, 1, 2);
    U_A2S(vm, regs + 1, 2);
    keepvar[23] = TopM(regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 336); // "inf"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 380, 6, 337);
    U_STATEMENT(vm, regs + 0, 382, 6);
    U_STATEMENT(vm, regs + 0, 382, 6);
    U_PUSHFUN(vm, regs + 0, 0, fun_192);
    SetLVal(vm, &locals[67]); // f
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 253, 5);
    fun_192(vm, regs + 0); // call: function184
    U_STATEMENT(vm, regs + 0, 389, 6);
    U_STATEMENT(vm, regs + 0, 389, 6);
    U_PUSHFUN(vm, regs + 0, 0, fun_193);
    SetLVal(vm, &locals[68]); // f
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 253, 5);
    fun_193(vm, regs + 0); // call: function185
    U_STATEMENT(vm, regs + 0, 404, 6);
    U_STATEMENT(vm, regs + 0, 404, 6);
    U_PUSHFUN(vm, regs + 0, 0, fun_194);
    SetLVal(vm, &locals[69]); // f
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 253, 5);
    fun_194(vm, regs + 0); // call: function186
    U_STATEMENT(vm, regs + 0, 413, 6);
    U_STATEMENT(vm, regs + 0, 413, 6);
    U_PUSHFUN(vm, regs + 0, 0, fun_196);
    SetLVal(vm, &locals[70]); // f
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 253, 5);
    fun_196(vm, regs + 0); // call: function188
    U_STATEMENT(vm, regs + 0, 428, 6);
    U_STATEMENT(vm, regs + 0, 428, 6);
    U_PUSHFUN(vm, regs + 0, 0, fun_198);
    SetLVal(vm, &locals[71]); // f
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 253, 5);
    fun_198(vm, regs + 0); // call: function190
    U_RETURNLOCAL(vm, 0, 0);
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
static void fun_156(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1566, locals);
    U_STATEMENT(vm, regs + 0, 44, 6);
    U_PUSHINT(vm, regs + 0, 3);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// tf
static void fun_157(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1573, locals);
    U_STATEMENT(vm, regs + 0, 45, 6);
    U_PUSHINT(vm, regs + 0, 4);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function152
static void fun_159(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value keepvar[1];
    Value locals[2];
    locals[0] = *(psp - 1);
    locals[1] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1580, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 107, 6);
    regs[0] = locals[0]; // i
    switch (regs[0].ival()) {
    case 1:
    U_STATEMENT(vm, regs + 0, 108, 6);
    U_PUSHSTR(vm, regs + 0, 338); // "no"
    U_INCREF(vm, regs + 1, 0);
    goto block24179;
    case 2:case 3:
    U_STATEMENT(vm, regs + 0, 110, 6);
    regs[0] = locals[0]; // i
    SetLVal(vm, &locals[1]); // x
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 111, 6);
    U_PUSHSTR(vm, regs + 0, 339); // "yes"
    regs[1] = locals[1]; // x
    U_A2S(vm, regs + 2, 0);
    keepvar[0] = TopM(regs + 2, 0);
    U_SADD(vm, regs + 2);
    goto block24179;
    case 4:case 5:case 6:case 8:
    U_STATEMENT(vm, regs + 0, 112, 6);
    U_PUSHSTR(vm, regs + 0, 340); // "maybe"
    U_INCREF(vm, regs + 1, 0);
    goto block24179;
    case 7:default:
    U_STATEMENT(vm, regs + 0, 113, 6);
    U_PUSHSTR(vm, regs + 0, 341); // "what?"
    U_INCREF(vm, regs + 1, 0);
    } // switch
    block24179:;
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// function153
static void fun_160(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    locals[0] = lobster::NilVal();
    locals[1] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1587, locals);
    U_STATEMENT(vm, regs + 0, 125, 6);
    U_PUSHINT(vm, regs + 0, 0);
    SetLVal(vm, &locals[0]); // a
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 126, 6);
    SetLVal(vm, &locals[0]); // a
    U_LV_DUP(vm, regs + 0);
    U_LV_IPP(vm, regs + 1);
    SetLVal(vm, &locals[0]); // a
    U_LV_IPP(vm, regs + 1);
    U_LV_DUP(vm, regs + 1);
    U_IADD(vm, regs + 2);
    SetLVal(vm, &locals[0]); // a
    U_LV_DUP(vm, regs + 1);
    U_LV_IMM(vm, regs + 2);
    U_IADD(vm, regs + 2);
    SetLVal(vm, &locals[0]); // a
    U_LV_IMM(vm, regs + 1);
    U_LV_DUP(vm, regs + 1);
    U_IADD(vm, regs + 2);
    U_PUSHINT(vm, regs + 1, 4);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 126, 6, 342);
    U_STATEMENT(vm, regs + 0, 127, 6);
    U_PUSHINT(vm, regs + 0, 0);
    U_NEWVEC(vm, regs + 1, 7, 1);
    SetLVal(vm, &locals[1]); // b
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 128, 6);
    regs[0] = locals[1]; // b
    U_PUSHINT(vm, regs + 1, 0);
    U_LVAL_IDXVI(vm, regs + 2, 0);
    U_LV_DUP(vm, regs + 0);
    U_LV_IPP(vm, regs + 1);
    regs[1] = locals[1]; // b
    U_PUSHINT(vm, regs + 2, 0);
    U_LVAL_IDXVI(vm, regs + 3, 0);
    U_LV_IPP(vm, regs + 1);
    U_LV_DUP(vm, regs + 1);
    U_IADD(vm, regs + 2);
    regs[1] = locals[1]; // b
    U_PUSHINT(vm, regs + 2, 0);
    U_LVAL_IDXVI(vm, regs + 3, 0);
    U_LV_DUP(vm, regs + 1);
    U_LV_IMM(vm, regs + 2);
    U_IADD(vm, regs + 2);
    regs[1] = locals[1]; // b
    U_PUSHINT(vm, regs + 2, 0);
    U_LVAL_IDXVI(vm, regs + 3, 0);
    U_LV_IMM(vm, regs + 1);
    U_LV_DUP(vm, regs + 1);
    U_IADD(vm, regs + 2);
    U_PUSHINT(vm, regs + 1, 4);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 128, 6, 343);
    U_RETURNLOCAL(vm, 0, 0);
    DecVal(vm, locals[1]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function154
static void fun_161(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1594, locals);
    U_STATEMENT(vm, regs + 0, 132, 6);
    regs[0] = locals[0]; // _a
    regs[1] = locals[1]; // _b
    U_ILT(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function155
static void fun_162(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1601, locals);
    U_STATEMENT(vm, regs + 0, 134, 6);
    regs[0] = locals[0]; // _a
    regs[1] = locals[1]; // _b
    U_ILT(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function156
static void fun_163(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1608, locals);
    U_STATEMENT(vm, regs + 0, 136, 6);
    regs[0] = locals[0]; // _a
    regs[1] = locals[1]; // _b
    U_ILT(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function157
static void fun_164(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1615, locals);
    U_STATEMENT(vm, regs + 0, 150, 6);
    regs[0] = locals[0]; // _
    regs[1] = locals[0]; // _
    U_IMUL(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// factorial
static void fun_477(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1621, locals);
    U_STATEMENT(vm, regs + 0, 153, 6);
    U_PUSHINT(vm, regs + 0, 1);
    regs[1] = locals[0]; // n
    U_IGT(vm, regs + 2);
    if (!U_JUMPNOFAIL(vm, regs + 1)) goto block24482;
    regs[0] = locals[0]; // n
    U_PUSHINT(vm, regs + 1, 1);
    U_ISUB(vm, regs + 2);
    fun_165(vm, regs + 1); // call: factorial
    U_POP(vm, regs + 1);
    block24482:;
    U_RETURNLOCAL(vm, 0, 0);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// factorial
static void fun_165(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1627, locals);
    U_STATEMENT(vm, regs + 0, 153, 6);
    U_PUSHINT(vm, regs + 0, 1);
    regs[1] = locals[0]; // n
    U_IGT(vm, regs + 2);
    if (!U_JUMPNOFAILR(vm, regs + 1)) goto block24529;
    U_POP(vm, regs + 1);
    regs[0] = locals[0]; // n
    U_PUSHINT(vm, regs + 1, 1);
    U_ISUB(vm, regs + 2);
    fun_165(vm, regs + 1); // call: factorial
    regs[1] = locals[0]; // n
    U_IMUL(vm, regs + 2);
    block24529:;
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function159
static void fun_166(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1633, locals);
    U_STATEMENT(vm, regs + 0, 156, 6);
    U_PUSHINT(vm, regs + 0, 4);
    fun_167(vm, regs + 1); // call: function160
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function160
static void fun_167(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1639, locals);
    U_STATEMENT(vm, regs + 0, 157, 6);
    regs[0] = locals[0]; // x
    regs[1] = locals[0]; // x
    U_IMUL(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// multiret
static void fun_470(VMRef vm, StackPtr psp) {
    Value regs[1];
    PushFunId(vm, funinfo_table + 1645, 0);
    U_STATEMENT(vm, regs + 0, 187, 6);
    U_PUSHFLT(vm, regs + 0, 0);
    U_BCALLRET1(vm, regs + 1, 69, 1); // sin
    if (!U_JUMPFAIL(vm, regs + 1)) goto block24612;
    U_STATEMENT(vm, regs + 0, 187, 6);
    U_PUSHINT(vm, regs + 0, 4);
    U_RETURNLOCAL(vm, 0, 1);
    Push(psp, regs[0]);
    goto epilogue;
    block24612:;
    U_STATEMENT(vm, regs + 0, 188, 6);
    U_PUSHINT(vm, regs + 0, 6);
    U_RETURNLOCAL(vm, 0, 1);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// multiret
static void fun_168(VMRef vm, StackPtr psp) {
    Value regs[2];
    PushFunId(vm, funinfo_table + 1650, 0);
    U_STATEMENT(vm, regs + 0, 187, 6);
    U_PUSHFLT(vm, regs + 0, 0);
    U_BCALLRET1(vm, regs + 1, 69, 1); // sin
    if (!U_JUMPFAIL(vm, regs + 1)) goto block24659;
    U_STATEMENT(vm, regs + 0, 187, 6);
    U_PUSHINT(vm, regs + 0, 4);
    U_PUSHINT(vm, regs + 1, 5);
    U_RETURNLOCAL(vm, 0, 2);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    goto epilogue;
    block24659:;
    U_STATEMENT(vm, regs + 0, 188, 6);
    U_PUSHINT(vm, regs + 0, 6);
    U_PUSHINT(vm, regs + 1, 7);
    U_RETURNLOCAL(vm, 0, 2);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// multiretpassthru
static void fun_169(VMRef vm, StackPtr psp) {
    Value regs[2];
    PushFunId(vm, funinfo_table + 1655, 0);
    U_STATEMENT(vm, regs + 0, 193, 6);
    fun_168(vm, regs + 0); // call: multiret
    U_RETURNLOCAL(vm, 0, 2);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// multirettyped
static void fun_170(VMRef vm, StackPtr psp) {
    Value regs[2];
    PushFunId(vm, funinfo_table + 1660, 0);
    U_STATEMENT(vm, regs + 0, 196, 6);
    U_PUSHFLT(vm, regs + 0, 1065353216);
    U_PUSHSTR(vm, regs + 1, 344); // "a"
    U_INCREF(vm, regs + 2, 0);
    U_RETURNLOCAL(vm, 0, 2);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// write
static void fun_171(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1665, locals);
    U_STATEMENT(vm, regs + 0, 204, 6);
    regs[0] = locals[0]; // this
    U_PUSHFLD(vm, regs + 1, 0);
    U_INCREF(vm, regs + 1, 0);
    regs[1] = locals[0]; // this
    U_PUSHFLD(vm, regs + 2, 1);
    U_PUSHINT(vm, regs + 2, 12345);
    U_BCALLRET3(vm, regs + 3, 200, 1); // write_int16_le
    regs[2] = locals[0]; // this
    U_LVAL_FLD(vm, regs + 3, 1);
    U_LV_WRITE(vm, regs + 2);
    regs[1] = locals[0]; // this
    U_LVAL_FLD(vm, regs + 2, 0);
    U_LV_WRITEREF(vm, regs + 1);
    U_RETURNLOCAL(vm, 0, 0);
    DecVal(vm, locals[0]);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// union_return
static void fun_172(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[2];
    locals[0] = *(psp - 1);
    locals[1] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1671, locals);
    U_STATEMENT(vm, regs + 0, 209, 6);
    U_PUSHINT(vm, regs + 0, -1);
    U_PUSHINT(vm, regs + 1, 10);
    block24791:;
    if (!U_IFOR(vm, regs + 2)) goto block24844;
    U_STATEMENT(vm, regs + 2, 209, 6);
    U_IFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[1]); // a
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 210, 6);
    regs[2] = locals[1]; // a
    regs[3] = locals[0]; // n
    U_IEQ(vm, regs + 4);
    if (!U_JUMPFAIL(vm, regs + 3)) goto block24839;
    U_STATEMENT(vm, regs + 2, 210, 6);
    U_POP(vm, regs + 2);
    U_POP(vm, regs + 1);
    U_PUSHSTR(vm, regs + 0, 345); // "foo"
    U_INCREF(vm, regs + 1, 0);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    block24839:;
    goto block24791;
    block24844:;
    U_STATEMENT(vm, regs + 0, 211, 6);
    U_PUSHNIL(vm, regs + 0);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// nlr1
static void fun_471(VMRef vm, StackPtr psp) {
    Value regs[1];
    (void)regs;
    PushFunId(vm, funinfo_table + 1678, 0);
    U_STATEMENT(vm, regs + 0, 216, 6);
    fun_472(vm, regs + 0); // call: nlr2
    if (!U_JUMPIFUNWOUND(vm, regs + 0, 167)) goto block24879;
    U_RETURNANY(vm, 0, 0);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block24879:;
    U_STATEMENT(vm, regs + 0, 217, 6);
    U_RETURNLOCAL(vm, 0, 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// nlr1
static void fun_173(VMRef vm, StackPtr psp) {
    Value regs[1];
    PushFunId(vm, funinfo_table + 1683, 0);
    U_STATEMENT(vm, regs + 0, 216, 6);
    fun_174(vm, regs + 0); // call: nlr2
    if (!U_JUMPIFUNWOUND(vm, regs + 1, 167)) goto block24912;
    U_RETURNANY(vm, 0, 0);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block24912:;
    U_STATEMENT(vm, regs + 0, 217, 6);
    U_PUSHINT(vm, regs + 0, 2);
    U_RETURNLOCAL(vm, 0, 1);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// nlr2
static void fun_472(VMRef vm, StackPtr psp) {
    Value regs[1];
    (void)regs;
    PushFunId(vm, funinfo_table + 1688, 0);
    U_STATEMENT(vm, regs + 0, 219, 6);
    U_RETURNNONLOCAL(vm, 0, 0, 166);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// nlr2
static void fun_174(VMRef vm, StackPtr psp) {
    Value regs[1];
    PushFunId(vm, funinfo_table + 1693, 0);
    U_STATEMENT(vm, regs + 0, 219, 6);
    U_PUSHINT(vm, regs + 0, 1);
    U_RETURNNONLOCAL(vm, 0, 1, 166);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function168
static void fun_175(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[1];
    locals[0] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1698, locals);
    U_STATEMENT(vm, regs + 0, 224, 6);
    U_PUSHINT(vm, regs + 0, -1);
    U_PUSHINT(vm, regs + 1, 2);
    block24978:;
    if (!U_IFOR(vm, regs + 2)) goto block25021;
    U_STATEMENT(vm, regs + 2, 224, 6);
    U_IFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[0]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 225, 6);
    regs[2] = locals[0]; // i
    if (!U_JUMPFAIL(vm, regs + 3)) goto block25016;
    U_STATEMENT(vm, regs + 2, 226, 6);
    U_POP(vm, regs + 2);
    U_POP(vm, regs + 1);
    U_RETURNNONLOCAL(vm, 0, 0, 57);
    goto epilogue;
    block25016:;
    goto block24978;
    block25021:;
    U_RETURNNONLOCAL(vm, 0, 0, 168);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// recursive_exceptions
static void fun_176(VMRef vm, StackPtr psp) {
    Value regs[2];
    SwapVars(vm, 374, psp, 1);
    PushFunId(vm, funinfo_table + 1704, 0);
    U_STATEMENT(vm, regs + 0, 229, 6);
    U_PUSHFUN(vm, regs + 0, 0, fun_177);
    U_PUSHFUN(vm, regs + 1, 0, fun_178);
    fun_474(vm, regs + 2); // call: try
    if (!U_JUMPIFUNWOUND(vm, regs + 2, 124)) goto block25058;
    U_RETURNANY(vm, 0, 0);
    psp = PopArg(vm, 374, psp);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block25058:;
    U_RETURNNONLOCAL(vm, 0, 0, 169);
    psp = PopArg(vm, 374, psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function170
static void fun_177(VMRef vm, StackPtr psp) {
    Value regs[2];
    PushFunId(vm, funinfo_table + 1710, 0);
    U_STATEMENT(vm, regs + 0, 230, 6);
    U_PUSHVARF(vm, regs + 0, 374); // n
    if (!U_JUMPFAIL(vm, regs + 1)) goto block25111;
    U_STATEMENT(vm, regs + 0, 230, 6);
    U_PUSHVARF(vm, regs + 0, 374); // n
    U_PUSHINT(vm, regs + 1, 1);
    U_ISUB(vm, regs + 2);
    fun_176(vm, regs + 1); // call: recursive_exceptions
    if (!U_JUMPIFUNWOUND(vm, regs + 2, 169)) goto block25106;
    U_RETURNANY(vm, 0, 0);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block25106:;
    goto block25134;
    block25111:;
    U_STATEMENT(vm, regs + 0, 231, 6);
    U_PUSHSTR(vm, regs + 0, 346); // ""
    fun_130(vm, regs + 1); // call: throw
    if (!U_JUMPIFUNWOUND(vm, regs + 2, 126)) goto block25132;
    U_RETURNANY(vm, 0, 0);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block25132:;
    block25134:;
    U_RETURNNONLOCAL(vm, 0, 0, 170);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function171
static void fun_178(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1715, locals);
    U_STATEMENT(vm, regs + 0, 233, 6);
    regs[0] = locals[0]; // v
    U_BCALLRET1(vm, regs + 1, 12, 1); // length
    U_PUSHVARF(vm, regs + 1, 374); // n
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 233, 6, 347);
    U_STATEMENT(vm, regs + 0, 234, 6);
    regs[0] = locals[0]; // v
    U_PUSHSTR(vm, regs + 1, 348); // "*"
    U_SADD(vm, regs + 2);
    fun_476(vm, regs + 1); // call: throw
    if (!U_JUMPIFUNWOUND(vm, regs + 2, 126)) goto block25194;
    U_RETURNANY(vm, 0, 0);
    Pop(psp);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block25194:;
    U_RETURNNONLOCAL(vm, 0, 0, 171);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function172
static void fun_179(VMRef vm, StackPtr psp) {
    Value regs[2];
    PushFunId(vm, funinfo_table + 1721, 0);
    U_STATEMENT(vm, regs + 0, 236, 6);
    U_PUSHINT(vm, regs + 0, 10);
    fun_176(vm, regs + 1); // call: recursive_exceptions
    if (!U_JUMPIFUNWOUND(vm, regs + 2, 169)) goto block25227;
    U_RETURNANY(vm, 0, 0);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block25227:;
    U_RETURNNONLOCAL(vm, 0, 0, 172);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function173
static void fun_180(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1726, locals);
    U_STATEMENT(vm, regs + 0, 240, 6);
    regs[0] = locals[0]; // _x
    regs[1] = locals[1]; // _y
    U_IADD(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// outer
static void fun_181(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    locals[0] = lobster::NilVal();
    locals[1] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1733, locals);
    U_STATEMENT(vm, regs + 0, 302, 6);
    U_STATEMENT(vm, regs + 0, 305, 6);
    U_PUSHINT(vm, regs + 0, 1);
    U_BCALLRET1(vm, regs + 1, 111, 1); // abs
    U_PUSHINT(vm, regs + 1, 1);
    U_IEQ(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block25301;
    U_STATEMENT(vm, regs + 0, 305, 6);
    U_PUSHINT(vm, regs + 0, 1);
    goto block25321;
    block25301:;
    U_STATEMENT(vm, regs + 0, 305, 6);
    fun_182(vm, regs + 0); // call: error
    if (!U_JUMPIFUNWOUND(vm, regs + 0, 175)) goto block25319;
    U_RETURNANY(vm, 0, 0);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block25319:;
    block25321:;
    SetLVal(vm, &locals[0]); // a
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 306, 6);
    regs[0] = locals[0]; // a
    U_DUP(vm, regs + 1);
    U_PUSHINT(vm, regs + 2, 1);
    U_IEQ(vm, regs + 3);
    if (!U_JUMPFAIL(vm, regs + 2)) goto block25357;
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 307, 6);
    U_PUSHINT(vm, regs + 0, 2);
    goto block25379;
    block25357:;
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 308, 6);
    fun_182(vm, regs + 0); // call: error
    if (!U_JUMPIFUNWOUND(vm, regs + 0, 175)) goto block25377;
    U_RETURNANY(vm, 0, 0);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block25377:;
    block25379:;
    SetLVal(vm, &locals[1]); // b
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 309, 6);
    regs[0] = locals[0]; // a
    regs[1] = locals[1]; // b
    U_IADD(vm, regs + 2);
    U_PUSHINT(vm, regs + 1, 3);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 309, 6, 349);
    U_RETURNLOCAL(vm, 0, 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// error
static void fun_182(VMRef vm, StackPtr psp) {
    Value regs[1];
    PushFunId(vm, funinfo_table + 1740, 0);
    U_STATEMENT(vm, regs + 0, 303, 6);
    U_PUSHINT(vm, regs + 0, 0);
    U_ASSERT(vm, regs + 1, 303, 6, 350);
    U_STATEMENT(vm, regs + 0, 304, 6);
    U_RETURNNONLOCAL(vm, 0, 0, 174);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// outer_dest2
static void fun_183(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value keepvar[1];
    SwapVars(vm, 390, psp, 1);
    PushFunId(vm, funinfo_table + 1745, 0);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 315, 6);
    U_STATEMENT(vm, regs + 0, 323, 6);
    fun_184(vm, regs + 0); // call: outer_dest1
    if (!U_JUMPIFUNWOUND(vm, regs + 2, 177)) goto block25468;
    U_RETURNANY(vm, 0, 1);
    psp = PopArg(vm, 390, psp);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block25468:;
    keepvar[0] = TopM(regs + 1, 0);
    U_BCALLRET1(vm, regs + 1, 12, 1); // length
    U_PUSHINT(vm, regs + 1, 0);
    U_RETURNLOCAL(vm, 0, 2);
    psp = PopArg(vm, 390, psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// outer_dest1
static void fun_184(VMRef vm, StackPtr psp) {
    Value regs[2];
    PushFunId(vm, funinfo_table + 1751, 0);
    U_STATEMENT(vm, regs + 0, 316, 6);
    U_STATEMENT(vm, regs + 0, 322, 6);
    fun_185(vm, regs + 0); // call: return_past
    if (!U_JUMPIFUNWOUND(vm, regs + 2, 178)) goto block25512;
    U_RETURNANY(vm, 0, 1);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block25512:;
    U_A2S(vm, regs + 1, 2);
    U_RETURNNONLOCAL(vm, 0, 1, 177);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// return_past
static void fun_185(VMRef vm, StackPtr psp) {
    Value regs[3];
    PushFunId(vm, funinfo_table + 1756, 0);
    U_STATEMENT(vm, regs + 0, 317, 6);
    U_PUSHVARF(vm, regs + 0, 390); // mode
    U_DUP(vm, regs + 1);
    U_PUSHINT(vm, regs + 2, 0);
    U_IEQ(vm, regs + 3);
    if (!U_JUMPFAIL(vm, regs + 2)) goto block25565;
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 318, 6);
    U_PUSHSTR(vm, regs + 0, 351); // "a"
    U_INCREF(vm, regs + 1, 0);
    U_RETURNNONLOCAL(vm, 0, 1, 177);
    Push(psp, regs[0]);
    goto epilogue;
    goto block25612;
    block25565:;
    U_DUP(vm, regs + 1);
    U_PUSHINT(vm, regs + 2, 1);
    U_IEQ(vm, regs + 3);
    if (!U_JUMPFAIL(vm, regs + 2)) goto block25596;
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 319, 6);
    U_PUSHINT(vm, regs + 0, 4);
    U_PUSHINT(vm, regs + 1, 5);
    U_RETURNNONLOCAL(vm, 0, 2, 176);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    goto epilogue;
    goto block25612;
    block25596:;
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 320, 6);
    U_PUSHFLT64(vm, regs + 0, -858993459, 1074318540);
    U_RETURNNONLOCAL(vm, 0, 1, 178);
    Push(psp, regs[0]);
    goto epilogue;
    block25612:;
    U_RETURNNONLOCAL(vm, 0, 0, 178);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// da
static void fun_187(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[3];
    locals[0] = *(psp - 3);
    locals[1] = *(psp - 2);
    locals[2] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1761, locals);
    U_STATEMENT(vm, regs + 0, 339, 6);
    regs[0] = locals[0]; // a
    regs[1] = locals[1]; // b
    U_IADD(vm, regs + 2);
    regs[1] = locals[2]; // c
    U_IADD(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// G
static void fun_190(VMRef vm, StackPtr psp) {
    Value regs[2];
    PushFunId(vm, funinfo_table + 1769, 0);
    U_STATEMENT(vm, regs + 0, 359, 6);
    fun_191(vm, regs + 0); // call: H
    if (!U_JUMPIFUNWOUND(vm, regs + 1, 183)) goto block25673;
    U_RETURNANY(vm, 0, 1);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block25673:;
    U_PUSHINT(vm, regs + 1, 1);
    U_IADD(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// H
static void fun_191(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value keepvar[1];
    PushFunId(vm, funinfo_table + 1774, 0);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 361, 6);
    U_NEWVEC(vm, regs + 0, 1697, 0);
    keepvar[0] = TopM(regs + 1, 0);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block25717;
    U_STATEMENT(vm, regs + 0, 362, 6);
    U_PUSHINT(vm, regs + 0, 4);
    U_RETURNNONLOCAL(vm, 0, 1, 183);
    Push(psp, regs[0]);
    goto epilogue;
    block25717:;
    U_STATEMENT(vm, regs + 0, 363, 6);
    U_PUSHINT(vm, regs + 0, 3);
    U_RETURNNONLOCAL(vm, 0, 1, 182);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// function184
static void fun_192(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[1];
    locals[0] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1779, locals);
    U_STATEMENT(vm, regs + 0, 383, 6);
    U_PUSHFLT(vm, regs + 0, 1073741824);
    SetLVal(vm, &locals[0]); // divisor
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 385, 6);
    U_PUSHFLT(vm, regs + 0, 1084227584);
    regs[1] = locals[0]; // divisor
    U_FMOD(vm, regs + 2);
    U_PUSHFLT(vm, regs + 1, 1065353216);
    U_FEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 385, 6, 352);
    U_STATEMENT(vm, regs + 0, 386, 6);
    U_PUSHVARVF(vm, regs + 0, 76, 2); // float2_1+0
    U_PUSHFLT(vm, regs + 2, 1084227584);
    U_FVSMUL(vm, regs + 3, 2);
    regs[2] = locals[0]; // divisor
    U_FVSMOD(vm, regs + 3, 2);
    U_PUSHVARVF(vm, regs + 2, 76, 2); // float2_1+0
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 386, 6, 353);
    U_STATEMENT(vm, regs + 0, 387, 6);
    U_PUSHVARVF(vm, regs + 0, 125, 2); // int2_1+0
    U_PUSHINT(vm, regs + 2, 5);
    U_IVSMUL(vm, regs + 3, 2);
    U_PUSHINT(vm, regs + 2, 2);
    U_IVSMOD(vm, regs + 3, 2);
    U_PUSHVARVF(vm, regs + 2, 125, 2); // int2_1+0
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 387, 6, 354);
    U_RETURNLOCAL(vm, 0, 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function185
static void fun_193(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value keepvar[6];
    Value locals[1];
    locals[0] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1785, locals);
    keepvar[0] = lobster::NilVal();
    keepvar[1] = lobster::NilVal();
    keepvar[2] = lobster::NilVal();
    keepvar[3] = lobster::NilVal();
    keepvar[4] = lobster::NilVal();
    keepvar[5] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 391, 6);
    U_STATEMENT(vm, regs + 0, 391, 6);
    U_STATEMENT(vm, regs + 0, 395, 6);
    U_PUSHVARVF(vm, regs + 0, 76, 2); // float2_1+0
    U_PUSHSTR(vm, regs + 2, 355); // "hello"
    U_INCREF(vm, regs + 3, 0);
    U_PUSHINT(vm, regs + 3, 2);
    U_NEWVEC(vm, regs + 4, 7, 1);
    U_NEWOBJECT(vm, regs + 4, 711); // Reflect
    SetLVal(vm, &locals[0]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 396, 6);
    regs[0] = locals[0]; // r
    U_BCALLRETV(vm, regs + 1, 155, 1); // type_string
    keepvar[0] = TopM(regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 356); // "Reflect"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 396, 6, 357);
    U_STATEMENT(vm, regs + 0, 397, 6);
    regs[0] = locals[0]; // r
    U_BCALLRETV(vm, regs + 1, 157, 1); // type_field_count
    U_PUSHINT(vm, regs + 1, 4);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 397, 6, 358);
    U_STATEMENT(vm, regs + 0, 398, 6);
    regs[0] = locals[0]; // r
    U_PUSHINT(vm, regs + 1, 0);
    U_BCALLRETV(vm, regs + 2, 158, 1); // type_field_string
    keepvar[1] = TopM(regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 359); // "float"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 398, 6, 360);
    U_STATEMENT(vm, regs + 0, 399, 6);
    regs[0] = locals[0]; // r
    U_PUSHINT(vm, regs + 1, 3);
    U_BCALLRETV(vm, regs + 2, 158, 1); // type_field_string
    keepvar[2] = TopM(regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 361); // "[int]"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 399, 6, 362);
    U_STATEMENT(vm, regs + 0, 400, 6);
    regs[0] = locals[0]; // r
    U_PUSHINT(vm, regs + 1, 2);
    U_BCALLRETV(vm, regs + 2, 159, 1); // type_field_name
    keepvar[3] = TopM(regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 363); // "s"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 400, 6, 364);
    U_STATEMENT(vm, regs + 0, 401, 6);
    regs[0] = locals[0]; // r
    U_PUSHINT(vm, regs + 1, 0);
    U_BCALLRETV(vm, regs + 2, 160, 1); // type_field_value
    keepvar[4] = TopM(regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 365); // "1.0"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 401, 6, 366);
    U_STATEMENT(vm, regs + 0, 402, 6);
    regs[0] = locals[0]; // r
    U_PUSHFLD(vm, regs + 1, 3);
    U_BCALLRETV(vm, regs + 1, 156, 1); // type_element_string
    keepvar[5] = TopM(regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 367); // "int"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 402, 6, 368);
    U_RETURNLOCAL(vm, 0, 0);
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

// function186
static void fun_194(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    BackupVar(vm, 402);
    locals[0] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1791, locals);
    U_STATEMENT(vm, regs + 0, 406, 6);
    U_PUSHINT(vm, regs + 0, 1);
    U_LVAL_VARF(vm, regs + 1, 402); // state
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 407, 6);
    U_PUSHFUN(vm, regs + 0, 0, fun_195);
    SetLVal(vm, &locals[0]); // fv
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 410, 6);
    regs[0] = locals[0]; // fv
    U_BCALLRET1(vm, regs + 1, 153, 0); // call_function_value
    U_STATEMENT(vm, regs + 0, 411, 6);
    U_PUSHVARF(vm, regs + 0, 402); // state
    U_PUSHINT(vm, regs + 1, 2);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 411, 6, 369);
    U_RETURNLOCAL(vm, 0, 0);
    goto epilogue;
    epilogue:;
    RestoreBackup(vm, 402);
    PopFunId(vm);
}

// function187
static void fun_195(VMRef vm, StackPtr psp) {
    Value regs[1];
    PushFunId(vm, funinfo_table + 1798, 0);
    U_STATEMENT(vm, regs + 0, 409, 6);
    U_PUSHINT(vm, regs + 0, 2);
    U_LVAL_VARF(vm, regs + 1, 402); // state
    U_LV_WRITE(vm, regs + 1);
    U_RETURNLOCAL(vm, 0, 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function188
static void fun_196(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[1];
    locals[0] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1803, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 415, 6);
    U_PUSHINT(vm, regs + 0, 4);
    U_PUSHFUN(vm, regs + 1, 0, fun_197);
    fun_499(vm, regs + 2); // call: map
    SetLVal(vm, &locals[0]); // gs
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 423, 6);
    regs[0] = locals[0]; // gs
    U_PUSHINT(vm, regs + 1, 10);
    U_PUSHINT(vm, regs + 2, 20);
    U_PUSHINT(vm, regs + 3, 30);
    U_PUSHINT(vm, regs + 4, 40);
    U_NEWVEC(vm, regs + 5, 7, 4);
    keepvar[0] = TopM(regs + 2, 0);
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 423, 6, 370);
    U_STATEMENT(vm, regs + 0, 424, 6);
    U_STATEMENT(vm, regs + 0, 425, 6);
    U_RETURNLOCAL(vm, 0, 0);
    DecVal(vm, locals[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// function189
static void fun_197(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1809, locals);
    U_STATEMENT(vm, regs + 0, 416, 6);
    regs[0] = locals[0]; // i
    U_PUSHINT(vm, regs + 1, 2);
    U_IGE(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block26302;
    U_STATEMENT(vm, regs + 0, 420, 6);
    regs[0] = locals[0]; // i
    U_PUSHINT(vm, regs + 1, 3);
    U_IGE(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block26288;
    U_STATEMENT(vm, regs + 0, 422, 6);
    U_PUSHINT(vm, regs + 0, 40);
    goto block26297;
    block26288:;
    U_STATEMENT(vm, regs + 0, 421, 6);
    U_PUSHINT(vm, regs + 0, 30);
    block26297:;
    goto block26340;
    block26302:;
    U_STATEMENT(vm, regs + 0, 417, 6);
    regs[0] = locals[0]; // i
    U_PUSHINT(vm, regs + 1, 1);
    U_IGE(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block26329;
    U_STATEMENT(vm, regs + 0, 419, 6);
    U_PUSHINT(vm, regs + 0, 20);
    goto block26338;
    block26329:;
    U_STATEMENT(vm, regs + 0, 418, 6);
    U_PUSHINT(vm, regs + 0, 10);
    block26338:;
    block26340:;
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function190
static void fun_198(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value keepvar[1];
    PushFunId(vm, funinfo_table + 1815, 0);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 430, 6);
    U_PUSHINT(vm, regs + 0, 17);
    fun_501(vm, regs + 1); // call: dictionary
    U_PUSHSTR(vm, regs + 1, 371); // "foo"
    fun_143(vm, regs + 2); // call: get
    keepvar[0] = TopM(regs + 1, 0);
    U_PUSHNIL(vm, regs + 1);
    U_AEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 430, 6, 372);
    U_RETURNLOCAL(vm, 0, 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// function191
static void fun_199(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[12];
    locals[0] = lobster::NilVal();
    locals[1] = lobster::NilVal();
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    locals[5] = lobster::NilVal();
    locals[6] = lobster::NilVal();
    locals[7] = lobster::NilVal();
    locals[8] = lobster::NilVal();
    locals[9] = lobster::NilVal();
    locals[10] = lobster::NilVal();
    locals[11] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1820, locals);
    U_STATEMENT(vm, regs + 0, 10, 9);
    U_STATEMENT(vm, regs + 0, 10, 9);
    U_PUSHFUN(vm, regs + 0, 0, fun_200);
    SetLVal(vm, &locals[0]); // f
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 253, 5);
    fun_200(vm, regs + 0); // call: function192
    U_STATEMENT(vm, regs + 0, 18, 9);
    U_STATEMENT(vm, regs + 0, 18, 9);
    U_PUSHFUN(vm, regs + 0, 0, fun_206);
    SetLVal(vm, &locals[1]); // f
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 253, 5);
    fun_206(vm, regs + 0); // call: function198
    U_STATEMENT(vm, regs + 0, 34, 9);
    U_PUSHINT(vm, regs + 0, 0);
    U_PUSHINT(vm, regs + 1, 0);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 34, 9, 373);
    U_STATEMENT(vm, regs + 0, 36, 9);
    U_STATEMENT(vm, regs + 0, 36, 9);
    U_PUSHFUN(vm, regs + 0, 0, fun_208);
    SetLVal(vm, &locals[2]); // f
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 253, 5);
    fun_208(vm, regs + 0); // call: function200
    U_STATEMENT(vm, regs + 0, 50, 9);
    U_STATEMENT(vm, regs + 0, 50, 9);
    U_PUSHFUN(vm, regs + 0, 0, fun_211);
    SetLVal(vm, &locals[3]); // f
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 253, 5);
    fun_211(vm, regs + 0); // call: function203
    U_STATEMENT(vm, regs + 0, 61, 9);
    U_STATEMENT(vm, regs + 0, 61, 9);
    U_PUSHFUN(vm, regs + 0, 0, fun_212);
    SetLVal(vm, &locals[4]); // f
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 253, 5);
    fun_212(vm, regs + 0); // call: function204
    U_STATEMENT(vm, regs + 0, 127, 9);
    U_STATEMENT(vm, regs + 0, 127, 9);
    U_PUSHFUN(vm, regs + 0, 0, fun_233);
    SetLVal(vm, &locals[5]); // f
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 253, 5);
    fun_233(vm, regs + 0); // call: function214
    U_STATEMENT(vm, regs + 0, 139, 9);
    U_STATEMENT(vm, regs + 0, 139, 9);
    U_PUSHFUN(vm, regs + 0, 0, fun_235);
    SetLVal(vm, &locals[6]); // f
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 253, 5);
    fun_235(vm, regs + 0); // call: function216
    U_STATEMENT(vm, regs + 0, 227, 9);
    U_STATEMENT(vm, regs + 0, 227, 9);
    U_PUSHFUN(vm, regs + 0, 0, fun_250);
    SetLVal(vm, &locals[7]); // f
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 253, 5);
    fun_250(vm, regs + 0); // call: function231
    U_STATEMENT(vm, regs + 0, 239, 9);
    U_STATEMENT(vm, regs + 0, 239, 9);
    U_PUSHFUN(vm, regs + 0, 0, fun_251);
    SetLVal(vm, &locals[8]); // f
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 253, 5);
    fun_251(vm, regs + 0); // call: function232
    U_STATEMENT(vm, regs + 0, 245, 9);
    U_STATEMENT(vm, regs + 0, 245, 9);
    U_PUSHFUN(vm, regs + 0, 0, fun_254);
    SetLVal(vm, &locals[9]); // f
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 253, 5);
    fun_254(vm, regs + 0); // call: function234
    U_STATEMENT(vm, regs + 0, 265, 9);
    U_STATEMENT(vm, regs + 0, 265, 9);
    U_PUSHFUN(vm, regs + 0, 0, fun_256);
    SetLVal(vm, &locals[10]); // f
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 253, 5);
    fun_256(vm, regs + 0); // call: function236
    U_STATEMENT(vm, regs + 0, 272, 9);
    U_STATEMENT(vm, regs + 0, 272, 9);
    U_PUSHFUN(vm, regs + 0, 0, fun_257);
    SetLVal(vm, &locals[11]); // f
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 253, 5);
    fun_257(vm, regs + 0); // call: function237
    U_RETURNLOCAL(vm, 0, 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function192
static void fun_200(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value keepvar[1];
    PushFunId(vm, funinfo_table + 1837, 0);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 11, 9);
    U_STATEMENT(vm, regs + 0, 12, 9);
    U_STATEMENT(vm, regs + 0, 15, 9);
    U_PUSHINT(vm, regs + 0, 1);
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_202(vm, regs + 2); // call: g
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 15, 9, 374);
    U_STATEMENT(vm, regs + 0, 16, 9);
    U_PUSHSTR(vm, regs + 0, 375); // "a"
    U_PUSHFUN(vm, regs + 1, 0, fun_205);
    fun_506(vm, regs + 2); // call: g
    keepvar[0] = TopM(regs + 2, 0);
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 16, 9, 376);
    U_RETURNLOCAL(vm, 0, 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// h
static void fun_508(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1842, locals);
    U_STATEMENT(vm, regs + 0, 11, 9);
    U_STATEMENT(vm, regs + 0, 14, 9);
    fun_205(vm, regs + 0); // call: function197
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// g
static void fun_506(VMRef vm, StackPtr psp) {
    Value regs[1];
    SwapVars(vm, 1296, psp, 1);
    PushFunId(vm, funinfo_table + 1848, 0);
    U_STATEMENT(vm, regs + 0, 14, 9);
    U_PUSHFUN(vm, regs + 0, 0, fun_10000001);
    fun_508(vm, regs + 1); // call: h
    U_RETURNLOCAL(vm, 0, 1);
    psp = PopArg(vm, 1296, psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// g
static void fun_202(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    SwapVars(vm, 407, psp, 1);
    locals[0] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1854, locals);
    U_STATEMENT(vm, regs + 0, 14, 9);
    U_STATEMENT(vm, regs + 0, 14, 9);
    U_PUSHFUN(vm, regs + 0, 0, fun_203);
    SetLVal(vm, &locals[0]); // i
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 11, 9);
    fun_203(vm, regs + 0); // call: function195
    U_RETURNLOCAL(vm, 0, 1);
    psp = PopArg(vm, 407, psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function195
static void fun_203(VMRef vm, StackPtr psp) {
    Value regs[1];
    PushFunId(vm, funinfo_table + 1861, 0);
    U_STATEMENT(vm, regs + 0, 14, 9);
    U_STATEMENT(vm, regs + 0, 15, 9);
    U_PUSHINT(vm, regs + 0, 1);
    U_RETURNLOCAL(vm, 0, 1);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function197
static void fun_205(VMRef vm, StackPtr psp) {
    Value regs[1];
    PushFunId(vm, funinfo_table + 1866, 0);
    U_STATEMENT(vm, regs + 0, 16, 9);
    U_PUSHSTR(vm, regs + 0, 377); // "a"
    U_INCREF(vm, regs + 1, 0);
    U_RETURNLOCAL(vm, 0, 1);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function198
static void fun_206(VMRef vm, StackPtr psp) {
    Value regs[1];
    PushFunId(vm, funinfo_table + 1871, 0);
    U_STATEMENT(vm, regs + 0, 19, 9);
    U_STATEMENT(vm, regs + 0, 31, 9);
    U_PUSHINT(vm, regs + 0, 1);
    fun_207(vm, regs + 1); // call: compile_time_if
    U_POP(vm, regs + 1);
    U_PUSHINT(vm, regs + 0, 1);
    U_ASSERT(vm, regs + 1, 31, 9, 378);
    U_STATEMENT(vm, regs + 0, 32, 9);
    U_PUSHSTR(vm, regs + 0, 379); // ""
    fun_511(vm, regs + 1); // call: compile_time_if
    U_POPREF(vm, regs + 1);
    U_PUSHINT(vm, regs + 0, 1);
    U_ASSERT(vm, regs + 1, 32, 9, 380);
    U_RETURNLOCAL(vm, 0, 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// compile_time_if
static void fun_511(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1876, locals);
    U_STATEMENT(vm, regs + 0, 30, 9);
    U_STATEMENT(vm, regs + 0, 30, 9);
    regs[0] = locals[0]; // x
    U_INCREF(vm, regs + 1, 0);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// compile_time_if
static void fun_207(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1882, locals);
    U_STATEMENT(vm, regs + 0, 30, 9);
    U_STATEMENT(vm, regs + 0, 26, 9);
    U_PUSHINT(vm, regs + 0, 1);
    regs[1] = locals[0]; // x
    U_IDIV(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function200
static void fun_208(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value keepvar[2];
    PushFunId(vm, funinfo_table + 1888, 0);
    keepvar[0] = lobster::NilVal();
    keepvar[1] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 37, 9);
    U_STATEMENT(vm, regs + 0, 37, 9);
    U_STATEMENT(vm, regs + 0, 39, 9);
    U_STATEMENT(vm, regs + 0, 39, 9);
    U_STATEMENT(vm, regs + 0, 43, 9);
    U_STATEMENT(vm, regs + 0, 46, 9);
    U_STATEMENT(vm, regs + 0, 47, 9);
    U_PUSHINT(vm, regs + 0, 1);
    fun_209(vm, regs + 1); // call: return_from
    keepvar[0] = TopM(regs + 1, 0);
    U_ISTYPE(vm, regs + 1, 1569); // testa
    U_ASSERT(vm, regs + 1, 47, 9, 381);
    U_STATEMENT(vm, regs + 0, 48, 9);
    U_PUSHSTR(vm, regs + 0, 382); // ""
    fun_513(vm, regs + 1); // call: return_from
    keepvar[1] = TopM(regs + 1, 0);
    U_ISTYPE(vm, regs + 1, 1569); // testa
    U_ASSERT(vm, regs + 1, 48, 9, 383);
    U_RETURNLOCAL(vm, 0, 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    DecVal(vm, keepvar[1]);
    PopFunId(vm);
}

// return_from
static void fun_513(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1893, locals);
    U_STATEMENT(vm, regs + 0, 44, 9);
    fun_210(vm, regs + 0); // call: return_from2
    if (!U_JUMPIFUNWOUND(vm, regs + 1, 202)) goto block27101;
    U_RETURNANY(vm, 0, 0);
    Pop(psp);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block27101:;
    U_STATEMENT(vm, regs + 0, 45, 9);
    U_PUSHINT(vm, regs + 0, 0);
    U_PUSHINT(vm, regs + 1, 0);
    U_NEWOBJECT(vm, regs + 2, 1578); // testb
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// return_from
static void fun_209(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1899, locals);
    U_STATEMENT(vm, regs + 0, 44, 9);
    fun_210(vm, regs + 0); // call: return_from2
    if (!U_JUMPIFUNWOUND(vm, regs + 1, 202)) goto block27144;
    U_RETURNANY(vm, 0, 0);
    Pop(psp);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block27144:;
    U_STATEMENT(vm, regs + 0, 45, 9);
    U_PUSHINT(vm, regs + 0, 0);
    U_PUSHINT(vm, regs + 1, 0);
    U_NEWOBJECT(vm, regs + 2, 1578); // testb
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// return_from2
static void fun_210(VMRef vm, StackPtr psp) {
    Value regs[1];
    PushFunId(vm, funinfo_table + 1905, 0);
    U_STATEMENT(vm, regs + 0, 46, 9);
    U_PUSHINT(vm, regs + 0, 0);
    U_NEWOBJECT(vm, regs + 1, 1569); // testa
    U_RETURNNONLOCAL(vm, 0, 1, 201);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function203
static void fun_211(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value keepvar[2];
    Value locals[1];
    locals[0] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1910, locals);
    keepvar[0] = lobster::NilVal();
    keepvar[1] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 52, 9);
    U_PUSHINT(vm, regs + 0, 1);
    SetLVal(vm, &locals[0]); // b
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 53, 9);
    U_PUSHINT(vm, regs + 0, 1);
    SetLVal(vm, &locals[0]); // b
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 54, 9);
    regs[0] = locals[0]; // b
    U_PUSHINT(vm, regs + 1, 1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 54, 9, 384);
    U_STATEMENT(vm, regs + 0, 55, 9);
    U_PUSHINT(vm, regs + 0, 1);
    U_A2S(vm, regs + 1, 146);
    keepvar[0] = TopM(regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 385); // "true"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 55, 9, 386);
    U_STATEMENT(vm, regs + 0, 56, 9);
    U_PUSHINT(vm, regs + 0, 1338);
    U_PUSHSTR(vm, regs + 1, 387); // "[true]"
    U_BCALLRETV(vm, regs + 2, 253, 1); // parse_data
    U_POPREF(vm, regs + 2);
    U_ASSERTR(vm, regs + 1, 56, 9, 388);
    keepvar[1] = TopM(regs + 1, 0);
    U_PUSHINT(vm, regs + 1, 0);
    U_VPUSHIDXI(vm, regs + 2);
    U_PUSHINT(vm, regs + 1, 1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 56, 9, 389);
    U_STATEMENT(vm, regs + 0, 57, 9);
    U_PUSHINT(vm, regs + 0, 1);
    U_DUP(vm, regs + 1);
    U_PUSHINT(vm, regs + 2, 0);
    U_IEQ(vm, regs + 3);
    if (!U_JUMPFAIL(vm, regs + 2)) goto block27332;
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 58, 9);
    U_PUSHINT(vm, regs + 0, 0);
    U_ASSERT(vm, regs + 1, 58, 9, 390);
    goto block27359;
    block27332:;
    U_DUP(vm, regs + 1);
    U_PUSHINT(vm, regs + 2, 1);
    U_IEQ(vm, regs + 3);
    if (!U_JUMPFAIL(vm, regs + 2)) goto block27353;
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 59, 9);
    goto block27359;
    block27353:;
    U_POP(vm, regs + 1);
    U_ENUM_RANGE_ERR(vm, regs + 0);
    block27359:;
    U_RETURNLOCAL(vm, 0, 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    DecVal(vm, keepvar[1]);
    PopFunId(vm);
}

// function204
static void fun_212(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value keepvar[4];
    Value locals[9];
    locals[0] = lobster::NilVal();
    locals[1] = lobster::NilVal();
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    locals[5] = lobster::NilVal();
    locals[6] = lobster::NilVal();
    locals[7] = lobster::NilVal();
    locals[8] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 1916, locals);
    keepvar[0] = lobster::NilVal();
    keepvar[1] = lobster::NilVal();
    keepvar[2] = lobster::NilVal();
    keepvar[3] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 65, 9);
    U_STATEMENT(vm, regs + 0, 63, 9);
    U_STATEMENT(vm, regs + 0, 63, 9);
    U_STATEMENT(vm, regs + 0, 68, 9);
    U_STATEMENT(vm, regs + 0, 66, 9);
    U_STATEMENT(vm, regs + 0, 66, 9);
    U_STATEMENT(vm, regs + 0, 71, 9);
    U_STATEMENT(vm, regs + 0, 71, 9);
    U_PUSHINT(vm, regs + 0, 0);
    SetLVal(vm, &locals[5]); // this+0
    U_LV_WRITEV(vm, regs + 1, 1);
    U_STATEMENT(vm, regs + 0, 65, 9);
    U_PUSHINT(vm, regs + 0, 1);
    U_PUSHINT(vm, regs + 1, 1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 71, 9, 391);
    U_STATEMENT(vm, regs + 0, 72, 9);
    U_STATEMENT(vm, regs + 0, 72, 9);
    U_PUSHINT(vm, regs + 0, 0);
    U_PUSHINT(vm, regs + 1, 0);
    SetLVal(vm, &locals[6]); // this+0
    U_LV_WRITEV(vm, regs + 2, 2);
    U_STATEMENT(vm, regs + 0, 68, 9);
    U_PUSHINT(vm, regs + 0, 2);
    U_PUSHINT(vm, regs + 1, 2);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 72, 9, 392);
    U_STATEMENT(vm, regs + 0, 74, 9);
    U_STATEMENT(vm, regs + 0, 73, 9);
    U_STATEMENT(vm, regs + 0, 73, 9);
    U_STATEMENT(vm, regs + 0, 77, 9);
    U_STATEMENT(vm, regs + 0, 78, 9);
    U_STATEMENT(vm, regs + 0, 79, 9);
    U_STATEMENT(vm, regs + 0, 80, 9);
    U_STATEMENT(vm, regs + 0, 81, 9);
    U_STATEMENT(vm, regs + 0, 82, 9);
    U_STATEMENT(vm, regs + 0, 75, 9);
    U_STATEMENT(vm, regs + 0, 75, 9);
    U_STATEMENT(vm, regs + 0, 85, 9);
    U_STATEMENT(vm, regs + 0, 86, 9);
    U_STATEMENT(vm, regs + 0, 87, 9);
    U_STATEMENT(vm, regs + 0, 88, 9);
    U_STATEMENT(vm, regs + 0, 89, 9);
    U_STATEMENT(vm, regs + 0, 90, 9);
    U_STATEMENT(vm, regs + 0, 83, 9);
    U_STATEMENT(vm, regs + 0, 83, 9);
    U_STATEMENT(vm, regs + 0, 93, 9);
    U_STATEMENT(vm, regs + 0, 91, 9);
    U_STATEMENT(vm, regs + 0, 94, 9);
    U_PUSHINT(vm, regs + 0, 0);
    U_PUSHINT(vm, regs + 1, 0);
    U_NEWOBJECT(vm, regs + 2, 771); // C2
    SetLVal(vm, &locals[0]); // x
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 95, 9);
    U_PUSHINT(vm, regs + 0, 0);
    U_PUSHSTR(vm, regs + 1, 393); // "hello"
    U_INCREF(vm, regs + 2, 0);
    U_NEWOBJECT(vm, regs + 2, 783); // C3
    SetLVal(vm, &locals[1]); // x2
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 97, 9);
    regs[0] = locals[0]; // x
    U_INCREF(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 1, 0);
    U_DDCALL(vm, regs + 2, 0, 1); vm.next_call_target(vm, regs + 2);
    U_PUSHINT(vm, regs + 1, 2);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 97, 9, 394);
    U_STATEMENT(vm, regs + 0, 99, 9);
    U_PUSHINT(vm, regs + 0, 0);
    U_PUSHINT(vm, regs + 1, 0);
    U_NEWOBJECT(vm, regs + 2, 771); // C2
    U_PUSHINT(vm, regs + 1, 0);
    fun_222(vm, regs + 2); // call: f
    U_PUSHINT(vm, regs + 1, 2);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 99, 9, 395);
    U_STATEMENT(vm, regs + 0, 101, 9);
    regs[0] = locals[0]; // x
    U_INCREF(vm, regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 396); // "hi"
    U_INCREF(vm, regs + 2, 0);
    U_DDCALL(vm, regs + 2, 1, 1); vm.next_call_target(vm, regs + 2);
    keepvar[0] = TopM(regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 397); // "hi2"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 101, 9, 398);
    U_STATEMENT(vm, regs + 0, 102, 9);
    regs[0] = locals[0]; // x
    U_INCREF(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 1, 3);
    U_DDCALL(vm, regs + 2, 2, 1); vm.next_call_target(vm, regs + 2);
    U_PUSHINT(vm, regs + 1, 5);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 102, 9, 399);
    U_STATEMENT(vm, regs + 0, 103, 9);
    regs[0] = locals[1]; // x2
    U_INCREF(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 1, 10);
    U_DDCALL(vm, regs + 2, 0, 1); vm.next_call_target(vm, regs + 2);
    U_PUSHINT(vm, regs + 1, 15);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 103, 9, 400);
    U_STATEMENT(vm, regs + 0, 105, 9);
    regs[0] = locals[0]; // x
    U_INCREF(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 1, 10);
    U_DDCALL(vm, regs + 2, 3, 1); vm.next_call_target(vm, regs + 2);
    U_PUSHINT(vm, regs + 1, 0);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 105, 9, 401);
    U_STATEMENT(vm, regs + 0, 107, 9);
    regs[0] = locals[0]; // x
    U_INCREF(vm, regs + 1, 0);
    U_DDCALL(vm, regs + 1, 4, 0); vm.next_call_target(vm, regs + 1);
    U_PUSHINT(vm, regs + 1, 2);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 107, 9, 402);
    U_STATEMENT(vm, regs + 0, 109, 9);
    regs[0] = locals[0]; // x
    U_INCREF(vm, regs + 1, 0);
    U_DDCALL(vm, regs + 1, 5, 0); vm.next_call_target(vm, regs + 1);
    U_PUSHINT(vm, regs + 1, 11);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 109, 9, 403);
    U_STATEMENT(vm, regs + 0, 110, 9);
    U_PUSHINT(vm, regs + 0, 0);
    U_NEWOBJECT(vm, regs + 1, 762); // C1
    SetLVal(vm, &locals[2]); // x3
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 112, 9);
    regs[0] = locals[2]; // x3
    U_INCREF(vm, regs + 1, 0);
    U_DDCALL(vm, regs + 1, 6, 0); vm.next_call_target(vm, regs + 1);
    U_PUSHINT(vm, regs + 1, 12);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 112, 9, 404);
    U_STATEMENT(vm, regs + 0, 114, 9);
    regs[0] = locals[0]; // x
    U_INCREF(vm, regs + 1, 0);
    U_DDCALL(vm, regs + 1, 7, 0); vm.next_call_target(vm, regs + 1);
    SetLVal(vm, &locals[4]); // mr2
    U_LV_WRITEREF(vm, regs + 2);
    SetLVal(vm, &locals[3]); // mr1
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 115, 9);
    regs[0] = locals[3]; // mr1
    U_PUSHINT(vm, regs + 1, 2);
    U_IEQ(vm, regs + 2);
    if (!U_JUMPFAILR(vm, regs + 1)) goto block27910;
    U_POP(vm, regs + 1);
    regs[0] = locals[4]; // mr2
    U_PUSHSTR(vm, regs + 1, 405); // "b"
    U_SEQ(vm, regs + 2);
    block27910:;
    U_ASSERT(vm, regs + 1, 115, 9, 406);
    U_STATEMENT(vm, regs + 0, 117, 9);
    U_STATEMENT(vm, regs + 0, 119, 9);
    U_STATEMENT(vm, regs + 0, 120, 9);
    U_STATEMENT(vm, regs + 0, 121, 9);
    U_STATEMENT(vm, regs + 0, 122, 9);
    U_STATEMENT(vm, regs + 0, 123, 9);
    U_PUSHINT(vm, regs + 0, 1);
    U_NEWOBJECT(vm, regs + 1, 795); // D
    fun_229(vm, regs + 1); // call: foo
    U_STATEMENT(vm, regs + 1, 123, 9);
    U_PUSHINT(vm, regs + 1, 2);
    SetLVal(vm, &locals[8]); // i
    U_LV_WRITE(vm, regs + 2);
    U_STATEMENT(vm, regs + 1, 120, 9);
    regs[1] = locals[8]; // i
    U_IADD(vm, regs + 2);
    U_PUSHINT(vm, regs + 1, 3);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 123, 9, 407);
    U_STATEMENT(vm, regs + 0, 124, 9);
    U_PUSHSTR(vm, regs + 0, 408); // "ha"
    U_INCREF(vm, regs + 1, 0);
    U_NEWOBJECT(vm, regs + 1, 1402); // D
    fun_520(vm, regs + 1); // call: foo
    keepvar[1] = TopM(regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 409); // "llo"
    fun_231(vm, regs + 2); // call: foo
    keepvar[2] = TopM(regs + 2, 0);
    U_SADD(vm, regs + 2);
    keepvar[3] = TopM(regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 410); // "hallo"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 124, 9, 411);
    U_STATEMENT(vm, regs + 0, 125, 9);
    U_PUSHFLT(vm, regs + 0, 1065353216);
    fun_232(vm, regs + 1); // call: foo
    U_PUSHFLT(vm, regs + 1, 1073741824);
    U_FEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 125, 9, 412);
    U_RETURNLOCAL(vm, 0, 0);
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
static void fun_215(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1930, locals);
    U_STATEMENT(vm, regs + 0, 74, 9);
    U_PUSHINT(vm, regs + 0, 10);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// ad
static void fun_227(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1936, locals);
    U_STATEMENT(vm, regs + 0, 90, 9);
    U_PUSHINT(vm, regs + 0, 11);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// f
static void fun_216(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1942, locals);
    U_STATEMENT(vm, regs + 0, 77, 9);
    U_PUSHINT(vm, regs + 0, 1);
    regs[1] = locals[0]; // this
    U_PUSHFLD(vm, regs + 2, 0);
    U_IADD(vm, regs + 2);
    regs[1] = locals[1]; // c
    U_IADD(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// f
static void fun_222(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1949, locals);
    U_STATEMENT(vm, regs + 0, 85, 9);
    U_PUSHINT(vm, regs + 0, 2);
    regs[1] = locals[0]; // this
    U_PUSHFLD(vm, regs + 2, 0);
    U_IADD(vm, regs + 2);
    regs[1] = locals[0]; // this
    U_PUSHFLD(vm, regs + 2, 1);
    U_IADD(vm, regs + 2);
    regs[1] = locals[1]; // c
    U_IADD(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// f
static void fun_228(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1956, locals);
    U_STATEMENT(vm, regs + 0, 93, 9);
    regs[0] = locals[1]; // c
    regs[1] = locals[0]; // this
    U_PUSHFLD(vm, regs + 2, 1);
    U_BCALLRET1(vm, regs + 2, 12, 1); // length
    U_IADD(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// g
static void fun_516(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1963, locals);
    U_STATEMENT(vm, regs + 0, 78, 9);
    regs[0] = locals[1]; // c
    U_PUSHINT(vm, regs + 1, 1);
    U_IADD(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// g
static void fun_217(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value keepvar[1];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1970, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 78, 9);
    regs[0] = locals[1]; // c
    U_PUSHINT(vm, regs + 1, 1);
    U_A2S(vm, regs + 2, 0);
    keepvar[0] = TopM(regs + 2, 0);
    U_SADD(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// g
static void fun_517(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1977, locals);
    U_STATEMENT(vm, regs + 0, 86, 9);
    regs[0] = locals[1]; // c
    U_PUSHINT(vm, regs + 1, 2);
    U_IADD(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// g
static void fun_223(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value keepvar[1];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1984, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 86, 9);
    regs[0] = locals[1]; // c
    U_PUSHINT(vm, regs + 1, 2);
    U_A2S(vm, regs + 2, 0);
    keepvar[0] = TopM(regs + 2, 0);
    U_SADD(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// r
static void fun_218(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1991, locals);
    U_STATEMENT(vm, regs + 0, 79, 9);
    regs[0] = locals[1]; // c
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// r
static void fun_518(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 1998, locals);
    U_STATEMENT(vm, regs + 0, 87, 9);
    regs[0] = locals[1]; // c
    if (!U_JUMPFAIL(vm, regs + 1)) goto block28381;
    U_STATEMENT(vm, regs + 0, 87, 9);
    regs[0] = locals[0]; // this
    regs[1] = locals[1]; // c
    U_PUSHINT(vm, regs + 2, 1);
    U_ISUB(vm, regs + 3);
    fun_518(vm, regs + 2); // call: r
    goto block28400;
    block28381:;
    U_STATEMENT(vm, regs + 0, 87, 9);
    U_PUSHINT(vm, regs + 0, 0);
    U_NEWOBJECT(vm, regs + 1, 762); // C1
    regs[1] = locals[1]; // c
    U_DDCALL(vm, regs + 2, 3, 1); vm.next_call_target(vm, regs + 2);
    block28400:;
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// r
static void fun_224(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2005, locals);
    U_STATEMENT(vm, regs + 0, 87, 9);
    regs[0] = locals[1]; // c
    if (!U_JUMPFAIL(vm, regs + 1)) goto block28447;
    U_STATEMENT(vm, regs + 0, 87, 9);
    regs[0] = locals[0]; // this
    regs[1] = locals[1]; // c
    U_PUSHINT(vm, regs + 2, 1);
    U_ISUB(vm, regs + 3);
    fun_518(vm, regs + 2); // call: r
    goto block28466;
    block28447:;
    U_STATEMENT(vm, regs + 0, 87, 9);
    U_PUSHINT(vm, regs + 0, 0);
    U_NEWOBJECT(vm, regs + 1, 762); // C1
    regs[1] = locals[1]; // c
    U_DDCALL(vm, regs + 2, 3, 1); vm.next_call_target(vm, regs + 2);
    block28466:;
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// mr
static void fun_219(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2012, locals);
    U_STATEMENT(vm, regs + 0, 80, 9);
    U_PUSHINT(vm, regs + 0, 1);
    U_PUSHSTR(vm, regs + 1, 413); // "a"
    U_INCREF(vm, regs + 2, 0);
    U_RETURNLOCAL(vm, 0, 2);
    DecVal(vm, locals[0]);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// mr
static void fun_225(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2018, locals);
    U_STATEMENT(vm, regs + 0, 88, 9);
    U_PUSHINT(vm, regs + 0, 2);
    U_PUSHSTR(vm, regs + 1, 414); // "b"
    U_INCREF(vm, regs + 2, 0);
    U_RETURNLOCAL(vm, 0, 2);
    DecVal(vm, locals[0]);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// sup
static void fun_220(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2024, locals);
    U_STATEMENT(vm, regs + 0, 81, 9);
    U_PUSHINT(vm, regs + 0, 1);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// sup
static void fun_226(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 1);
    locals[1] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2030, locals);
    U_STATEMENT(vm, regs + 0, 89, 9);
    U_PUSHINT(vm, regs + 0, 1);
    U_STATEMENT(vm, regs + 1, 89, 9);
    regs[1] = locals[0]; // this
    SetLVal(vm, &locals[1]); // this
    U_LV_WRITE(vm, regs + 2);
    U_STATEMENT(vm, regs + 1, 81, 9);
    U_PUSHINT(vm, regs + 1, 1);
    U_IADD(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// sni
static void fun_221(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2037, locals);
    U_STATEMENT(vm, regs + 0, 82, 9);
    U_PUSHINT(vm, regs + 0, 12);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// foo
static void fun_520(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2043, locals);
    U_STATEMENT(vm, regs + 0, 119, 9);
    regs[0] = locals[0]; // d
    U_PUSHFLD(vm, regs + 1, 0);
    U_INCREF(vm, regs + 1, 0);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// foo
static void fun_229(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2049, locals);
    U_STATEMENT(vm, regs + 0, 119, 9);
    regs[0] = locals[0]; // d
    U_PUSHFLD(vm, regs + 1, 0);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// foo
static void fun_231(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2055, locals);
    U_STATEMENT(vm, regs + 0, 121, 9);
    regs[0] = locals[0]; // s
    U_INCREF(vm, regs + 1, 0);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// foo
static void fun_232(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2061, locals);
    U_STATEMENT(vm, regs + 0, 122, 9);
    regs[0] = locals[0]; // t
    U_PUSHFLT(vm, regs + 1, 1065353216);
    U_FADD(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function214
static void fun_233(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = lobster::NilVal();
    locals[1] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2067, locals);
    U_STATEMENT(vm, regs + 0, 129, 9);
    U_PUSHSTR(vm, regs + 0, 415); // ""
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[0]); // a
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 130, 9);
    regs[0] = locals[0]; // a
    U_PUSHSTR(vm, regs + 1, 416); // ""
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 130, 9, 417);
    U_STATEMENT(vm, regs + 0, 131, 9);
    U_PUSHNIL(vm, regs + 0);
    SetLVal(vm, &locals[1]); // b
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 132, 9);
    regs[0] = locals[1]; // b
    U_PUSHNIL(vm, regs + 1);
    U_AEQ(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block28776;
    U_STATEMENT(vm, regs + 0, 133, 9);
    goto block28795;
    block28776:;
    U_STATEMENT(vm, regs + 0, 135, 9);
    regs[0] = locals[1]; // b
    U_PUSHSTR(vm, regs + 1, 418); // ""
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 135, 9, 419);
    block28795:;
    U_STATEMENT(vm, regs + 0, 136, 9);
    U_STATEMENT(vm, regs + 0, 137, 9);
    regs[0] = locals[0]; // a
    fun_234(vm, regs + 1); // call: f
    U_RETURNLOCAL(vm, 0, 0);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// f
static void fun_234(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2074, locals);
    U_STATEMENT(vm, regs + 0, 136, 9);
    regs[0] = locals[0]; // x
    U_PUSHSTR(vm, regs + 1, 420); // ""
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 136, 9, 421);
    U_RETURNLOCAL(vm, 0, 0);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function216
static void fun_235(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[6];
    Value locals[8];
    locals[0] = lobster::NilVal();
    locals[1] = lobster::NilVal();
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    locals[5] = lobster::NilVal();
    locals[6] = lobster::NilVal();
    locals[7] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2080, locals);
    keepvar[0] = lobster::NilVal();
    keepvar[1] = lobster::NilVal();
    keepvar[2] = lobster::NilVal();
    keepvar[3] = lobster::NilVal();
    keepvar[4] = lobster::NilVal();
    keepvar[5] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 147, 9);
    U_STATEMENT(vm, regs + 0, 149, 9);
    U_STATEMENT(vm, regs + 0, 151, 9);
    U_STATEMENT(vm, regs + 0, 141, 9);
    U_STATEMENT(vm, regs + 0, 154, 9);
    U_STATEMENT(vm, regs + 0, 157, 9);
    U_STATEMENT(vm, regs + 0, 158, 9);
    U_PUSHINT(vm, regs + 0, 1);
    U_NEWVEC(vm, regs + 1, 7, 1);
    U_PUSHNIL(vm, regs + 1);
    U_NEWVEC(vm, regs + 2, 827, 0);
    U_PUSHNIL(vm, regs + 3);
    U_PUSHNIL(vm, regs + 4);
    U_NEWOBJECT(vm, regs + 5, 804); // Ai
    SetLVal(vm, &locals[0]); // a
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 159, 9);
    regs[0] = locals[0]; // a
    U_PUSHFLD(vm, regs + 1, 2);
    regs[1] = locals[0]; // a
    U_INCREF(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 2, 1);
    U_BCALLRETV(vm, regs + 3, 15, 1); // push
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 160, 9);
    U_NEWVEC(vm, regs + 0, 827, 0);
    regs[1] = locals[0]; // a
    U_LVAL_FLD(vm, regs + 2, 2);
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 161, 9);
    regs[0] = locals[0]; // a
    U_INCREF(vm, regs + 1, 0);
    regs[1] = locals[0]; // a
    U_LVAL_FLD(vm, regs + 2, 3);
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 162, 9);
    U_PUSHNIL(vm, regs + 0);
    regs[1] = locals[0]; // a
    U_LVAL_FLD(vm, regs + 2, 3);
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 163, 9);
    regs[0] = locals[0]; // a
    U_PUSHINT(vm, regs + 1, 2);
    fun_236(vm, regs + 2); // call: f
    U_STATEMENT(vm, regs + 0, 164, 9);
    regs[0] = locals[0]; // a
    U_PUSHINT(vm, regs + 1, 3);
    fun_237(vm, regs + 2); // call: g
    U_STATEMENT(vm, regs + 0, 165, 9);
    regs[0] = locals[0]; // a
    U_PUSHINT(vm, regs + 1, 4);
    fun_239(vm, regs + 2); // call: i
    U_STATEMENT(vm, regs + 0, 166, 9);
    regs[0] = locals[0]; // a
    fun_238(vm, regs + 1); // call: h
    U_PUSHINT(vm, regs + 1, 1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 166, 9, 422);
    U_STATEMENT(vm, regs + 0, 168, 9);
    U_PUSHSTR(vm, regs + 0, 423); // "hello"
    U_INCREF(vm, regs + 1, 0);
    U_NEWVEC(vm, regs + 1, 11, 1);
    U_PUSHNIL(vm, regs + 1);
    U_NEWVEC(vm, regs + 2, 852, 0);
    U_PUSHNIL(vm, regs + 3);
    U_PUSHNIL(vm, regs + 4);
    U_NEWOBJECT(vm, regs + 5, 829); // A
    SetLVal(vm, &locals[1]); // b
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 169, 9);
    regs[0] = locals[1]; // b
    U_PUSHFLD(vm, regs + 1, 2);
    regs[1] = locals[1]; // b
    U_INCREF(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 2, 1);
    U_BCALLRETV(vm, regs + 3, 15, 1); // push
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 170, 9);
    U_NEWVEC(vm, regs + 0, 852, 0);
    regs[1] = locals[1]; // b
    U_LVAL_FLD(vm, regs + 2, 2);
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 171, 9);
    regs[0] = locals[1]; // b
    U_INCREF(vm, regs + 1, 0);
    regs[1] = locals[1]; // b
    U_LVAL_FLD(vm, regs + 2, 4);
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 172, 9);
    U_PUSHNIL(vm, regs + 0);
    regs[1] = locals[1]; // b
    U_LVAL_FLD(vm, regs + 2, 4);
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 173, 9);
    regs[0] = locals[1]; // b
    U_PUSHSTR(vm, regs + 1, 424); // "hi"
    fun_523(vm, regs + 2); // call: f
    U_STATEMENT(vm, regs + 0, 174, 9);
    regs[0] = locals[1]; // b
    U_PUSHSTR(vm, regs + 1, 425); // "yo"
    fun_524(vm, regs + 2); // call: g
    U_STATEMENT(vm, regs + 0, 175, 9);
    regs[0] = locals[1]; // b
    U_PUSHSTR(vm, regs + 1, 426); // "sup"
    fun_525(vm, regs + 2); // call: i
    U_STATEMENT(vm, regs + 0, 195, 9);
    U_STATEMENT(vm, regs + 0, 196, 9);
    U_STATEMENT(vm, regs + 0, 196, 9);
    U_PUSHFLT(vm, regs + 0, 1065353216);
    SetLVal(vm, &locals[4]); // x
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 196, 9);
    U_PUSHFLT(vm, regs + 0, 1065353216);
    SetLVal(vm, &locals[5]); // y
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 195, 9);
    U_PUSHINT(vm, regs + 0, 1);
    U_ASSERT(vm, regs + 1, 196, 9, 427);
    U_STATEMENT(vm, regs + 0, 197, 9);
    U_STATEMENT(vm, regs + 0, 197, 9);
    U_PUSHFLT(vm, regs + 0, 1065353216);
    SetLVal(vm, &locals[6]); // x
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 197, 9);
    U_PUSHFLT(vm, regs + 0, 1065353216);
    SetLVal(vm, &locals[7]); // y
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 195, 9);
    U_PUSHINT(vm, regs + 0, 1);
    U_ASSERT(vm, regs + 1, 197, 9, 428);
    U_STATEMENT(vm, regs + 0, 198, 9);
    U_STATEMENT(vm, regs + 0, 199, 9);
    U_PUSHFLT(vm, regs + 0, 1073741824);
    fun_241(vm, regs + 1); // call: k
    U_PUSHFLT(vm, regs + 1, 1065353216);
    U_FEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 199, 9, 429);
    U_STATEMENT(vm, regs + 0, 200, 9);
    U_PUSHFLT(vm, regs + 0, 1073741824);
    fun_241(vm, regs + 1); // call: k
    U_PUSHFLT(vm, regs + 1, 1065353216);
    U_FEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 200, 9, 430);
    U_STATEMENT(vm, regs + 0, 201, 9);
    U_STATEMENT(vm, regs + 0, 203, 9);
    fun_242(vm, regs + 0); // call: q
    SetLVal(vm, &locals[3]); // s
    U_LV_WRITE(vm, regs + 2);
    SetLVal(vm, &locals[2]); // r
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 204, 9);
    U_STATEMENT(vm, regs + 0, 205, 9);
    U_STATEMENT(vm, regs + 0, 207, 9);
    regs[0] = locals[0]; // a
    fun_243(vm, regs + 1); // call: t0
    keepvar[0] = TopM(regs + 1, 0);
    U_PUSHFLD(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 1, 0);
    U_VPUSHIDXI(vm, regs + 2);
    U_PUSHINT(vm, regs + 1, 1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 207, 9, 431);
    U_STATEMENT(vm, regs + 0, 210, 9);
    U_STATEMENT(vm, regs + 0, 211, 9);
    U_STATEMENT(vm, regs + 0, 210, 9);
    U_PUSHINT(vm, regs + 0, 1);
    U_ASSERT(vm, regs + 1, 211, 9, 432);
    U_STATEMENT(vm, regs + 0, 212, 9);
    U_STATEMENT(vm, regs + 0, 213, 9);
    U_STATEMENT(vm, regs + 0, 212, 9);
    U_PUSHNIL(vm, regs + 0);
    keepvar[1] = TopM(regs + 1, 0);
    U_PUSHNIL(vm, regs + 1);
    U_AEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 213, 9, 433);
    U_STATEMENT(vm, regs + 0, 214, 9);
    U_STATEMENT(vm, regs + 0, 215, 9);
    U_STATEMENT(vm, regs + 0, 214, 9);
    U_NEWVEC(vm, regs + 0, 11, 0);
    keepvar[2] = TopM(regs + 1, 0);
    U_NEWVEC(vm, regs + 1, 11, 0);
    keepvar[3] = TopM(regs + 2, 0);
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 215, 9, 434);
    U_STATEMENT(vm, regs + 0, 216, 9);
    U_STATEMENT(vm, regs + 0, 217, 9);
    fun_247(vm, regs + 0); // call: p
    U_PUSHINT(vm, regs + 1, 3);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 217, 9, 435);
    U_STATEMENT(vm, regs + 0, 220, 9);
    U_STATEMENT(vm, regs + 0, 222, 9);
    regs[0] = locals[0]; // a
    fun_248(vm, regs + 1); // call: t1
    keepvar[4] = TopM(regs + 1, 0);
    U_PUSHFLD(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 1, 0);
    U_VPUSHIDXI(vm, regs + 2);
    U_PUSHINT(vm, regs + 1, 1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 222, 9, 436);
    U_STATEMENT(vm, regs + 0, 223, 9);
    U_STATEMENT(vm, regs + 0, 225, 9);
    regs[0] = locals[0]; // a
    fun_249(vm, regs + 1); // call: t2
    keepvar[5] = TopM(regs + 1, 0);
    U_PUSHFLD(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 1, 0);
    U_VPUSHIDXI(vm, regs + 2);
    U_PUSHINT(vm, regs + 1, 1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 225, 9, 437);
    U_RETURNLOCAL(vm, 0, 0);
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
static void fun_523(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2093, locals);
    U_STATEMENT(vm, regs + 0, 148, 9);
    regs[0] = locals[0]; // this
    U_PUSHFLD(vm, regs + 1, 0);
    regs[1] = locals[1]; // t
    U_INCREF(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 2, 1);
    U_BCALLRETV(vm, regs + 3, 15, 1); // push
    U_POP(vm, regs + 1);
    U_RETURNLOCAL(vm, 0, 0);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// f
static void fun_236(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2100, locals);
    U_STATEMENT(vm, regs + 0, 148, 9);
    regs[0] = locals[0]; // this
    U_PUSHFLD(vm, regs + 1, 0);
    regs[1] = locals[1]; // t
    U_PUSHINT(vm, regs + 2, 1);
    U_BCALLRETV(vm, regs + 3, 15, 1); // push
    U_POP(vm, regs + 1);
    U_RETURNLOCAL(vm, 0, 0);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// g
static void fun_524(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2107, locals);
    U_STATEMENT(vm, regs + 0, 150, 9);
    regs[0] = locals[0]; // this
    U_PUSHFLD(vm, regs + 1, 0);
    regs[1] = locals[1]; // u
    U_INCREF(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 2, 1);
    U_BCALLRETV(vm, regs + 3, 15, 1); // push
    U_POP(vm, regs + 1);
    U_RETURNLOCAL(vm, 0, 0);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// g
static void fun_237(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2114, locals);
    U_STATEMENT(vm, regs + 0, 150, 9);
    regs[0] = locals[0]; // this
    U_PUSHFLD(vm, regs + 1, 0);
    regs[1] = locals[1]; // u
    U_PUSHINT(vm, regs + 2, 1);
    U_BCALLRETV(vm, regs + 3, 15, 1); // push
    U_POP(vm, regs + 1);
    U_RETURNLOCAL(vm, 0, 0);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// h
static void fun_238(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[2];
    locals[0] = *(psp - 1);
    locals[1] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2121, locals);
    U_STATEMENT(vm, regs + 0, 152, 9);
    U_PUSHINT(vm, regs + 0, 1);
    SetLVal(vm, &locals[1]); // x
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 153, 9);
    regs[0] = locals[1]; // x
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// i
static void fun_525(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2128, locals);
    U_STATEMENT(vm, regs + 0, 155, 9);
    regs[0] = locals[0]; // a
    U_PUSHFLD(vm, regs + 1, 0);
    regs[1] = locals[1]; // t
    U_INCREF(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 2, 1);
    U_BCALLRETV(vm, regs + 3, 15, 1); // push
    U_POP(vm, regs + 1);
    U_RETURNLOCAL(vm, 0, 0);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// i
static void fun_239(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2135, locals);
    U_STATEMENT(vm, regs + 0, 155, 9);
    regs[0] = locals[0]; // a
    U_PUSHFLD(vm, regs + 1, 0);
    regs[1] = locals[1]; // t
    U_PUSHINT(vm, regs + 2, 1);
    U_BCALLRETV(vm, regs + 3, 15, 1); // push
    U_POP(vm, regs + 1);
    U_RETURNLOCAL(vm, 0, 0);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// k
static void fun_241(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2142, locals);
    U_STATEMENT(vm, regs + 0, 198, 9);
    U_PUSHFLT(vm, regs + 0, 1065353216);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// q
static void fun_242(VMRef vm, StackPtr psp) {
    Value regs[2];
    PushFunId(vm, funinfo_table + 2148, 0);
    U_STATEMENT(vm, regs + 0, 202, 9);
    U_PUSHFLT(vm, regs + 0, 1065353216);
    U_PUSHFLT(vm, regs + 1, 1065353216);
    U_RETURNLOCAL(vm, 0, 2);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// t0
static void fun_243(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2153, locals);
    U_STATEMENT(vm, regs + 0, 206, 9);
    regs[0] = locals[0]; // aa
    if (!U_JUMPFAIL(vm, regs + 1)) goto block29887;
    U_STATEMENT(vm, regs + 0, 206, 9);
    regs[0] = locals[0]; // aa
    U_PUSHFLD(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 1, 0);
    U_VPUSHIDXI(vm, regs + 2);
    goto block29896;
    block29887:;
    U_STATEMENT(vm, regs + 0, 206, 9);
    U_PUSHINT(vm, regs + 0, 2);
    block29896:;
    U_NEWVEC(vm, regs + 1, 7, 1);
    U_PUSHNIL(vm, regs + 1);
    U_NEWVEC(vm, regs + 2, 827, 0);
    U_PUSHNIL(vm, regs + 3);
    U_PUSHNIL(vm, regs + 4);
    U_NEWOBJECT(vm, regs + 5, 804); // Ai
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// p
static void fun_247(VMRef vm, StackPtr psp) {
    Value regs[1];
    PushFunId(vm, funinfo_table + 2159, 0);
    U_STATEMENT(vm, regs + 0, 216, 9);
    U_PUSHINT(vm, regs + 0, 3);
    U_RETURNLOCAL(vm, 0, 1);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// t1
static void fun_248(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2164, locals);
    U_STATEMENT(vm, regs + 0, 221, 9);
    regs[0] = locals[0]; // aa
    U_PUSHFLD(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 1, 0);
    U_VPUSHIDXI(vm, regs + 2);
    U_NEWVEC(vm, regs + 1, 7, 1);
    U_PUSHNIL(vm, regs + 1);
    U_NEWVEC(vm, regs + 2, 827, 0);
    U_PUSHNIL(vm, regs + 3);
    U_PUSHNIL(vm, regs + 4);
    U_NEWOBJECT(vm, regs + 5, 804); // Ai
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// t2
static void fun_249(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2170, locals);
    U_STATEMENT(vm, regs + 0, 224, 9);
    regs[0] = locals[0]; // aa
    if (!U_JUMPFAIL(vm, regs + 1)) goto block30017;
    U_STATEMENT(vm, regs + 0, 224, 9);
    regs[0] = locals[0]; // aa
    U_PUSHFLD(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 1, 0);
    U_VPUSHIDXI(vm, regs + 2);
    goto block30026;
    block30017:;
    U_STATEMENT(vm, regs + 0, 224, 9);
    U_PUSHINT(vm, regs + 0, 2);
    block30026:;
    U_NEWVEC(vm, regs + 1, 7, 1);
    U_PUSHNIL(vm, regs + 1);
    U_NEWVEC(vm, regs + 2, 827, 0);
    U_PUSHNIL(vm, regs + 3);
    U_PUSHNIL(vm, regs + 4);
    U_NEWOBJECT(vm, regs + 5, 804); // Ai
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function231
static void fun_250(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[2];
    locals[0] = lobster::NilVal();
    locals[1] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2176, locals);
    U_STATEMENT(vm, regs + 0, 229, 9);
    U_STATEMENT(vm, regs + 0, 231, 9);
    U_STATEMENT(vm, regs + 0, 233, 9);
    U_PUSHINT(vm, regs + 0, 5);
    fun_528(vm, regs + 1); // call: dictionary
    U_NEWOBJECT(vm, regs + 1, 854); // set1
    SetLVal(vm, &locals[0]); // a1
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 234, 9);
    U_PUSHINT(vm, regs + 0, 15);
    fun_528(vm, regs + 1); // call: dictionary
    U_NEWOBJECT(vm, regs + 1, 863); // set2
    SetLVal(vm, &locals[1]); // a2
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 235, 9);
    regs[0] = locals[0]; // a1
    U_PUSHFLD(vm, regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 438); // "a"
    U_PUSHINT(vm, regs + 2, 13);
    fun_531(vm, regs + 3); // call: set
    U_STATEMENT(vm, regs + 0, 236, 9);
    regs[0] = locals[1]; // a2
    U_PUSHFLD(vm, regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 439); // "a"
    U_PUSHINT(vm, regs + 2, 13);
    fun_531(vm, regs + 3); // call: set
    U_STATEMENT(vm, regs + 0, 237, 9);
    regs[0] = locals[0]; // a1
    U_PUSHFLD(vm, regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 440); // "a"
    U_PUSHINT(vm, regs + 2, 0);
    fun_533(vm, regs + 3); // call: get
    regs[1] = locals[1]; // a2
    U_PUSHFLD(vm, regs + 2, 0);
    U_PUSHSTR(vm, regs + 2, 441); // "a"
    U_PUSHINT(vm, regs + 3, 0);
    fun_533(vm, regs + 4); // call: get
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 237, 9, 442);
    U_RETURNLOCAL(vm, 0, 0);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function232
static void fun_251(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[1];
    locals[0] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2183, locals);
    U_STATEMENT(vm, regs + 0, 241, 9);
    U_STATEMENT(vm, regs + 0, 242, 9);
    U_STATEMENT(vm, regs + 0, 243, 9);
    U_STATEMENT(vm, regs + 0, 243, 9);
    U_PUSHINT(vm, regs + 0, 1);
    U_PUSHINT(vm, regs + 1, 2);
    U_PUSHINT(vm, regs + 2, 3);
    U_NEWVEC(vm, regs + 3, 7, 3);
    SetLVal(vm, &locals[0]); // xs
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 242, 9);
    U_PUSHINT(vm, regs + 0, 2);
    U_PUSHINT(vm, regs + 1, 2);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 243, 9, 443);
    U_RETURNLOCAL(vm, 0, 0);
    DecVal(vm, locals[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function234
static void fun_254(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[3];
    locals[0] = lobster::NilVal();
    locals[1] = lobster::NilVal();
    locals[2] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2189, locals);
    U_STATEMENT(vm, regs + 0, 248, 9);
    U_STATEMENT(vm, regs + 0, 248, 9);
    U_STATEMENT(vm, regs + 0, 250, 9);
    U_STATEMENT(vm, regs + 0, 250, 9);
    U_STATEMENT(vm, regs + 0, 252, 9);
    U_PUSHINT(vm, regs + 0, 1);
    U_NEWOBJECT(vm, regs + 1, 872); // A
    U_NEWVEC(vm, regs + 1, 881, 1);
    SetLVal(vm, &locals[0]); // as
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 253, 9);
    U_PUSHINT(vm, regs + 0, 1);
    U_PUSHINT(vm, regs + 1, 2);
    U_NEWOBJECT(vm, regs + 2, 883); // B
    U_NEWVEC(vm, regs + 1, 895, 1);
    SetLVal(vm, &locals[1]); // bs
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 255, 9);
    regs[0] = locals[0]; // as
    regs[1] = locals[1]; // bs
    U_BCALLRET2(vm, regs + 2, 8, 1); // append
    U_POPREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 256, 9);
    regs[0] = locals[1]; // bs
    regs[1] = locals[0]; // as
    U_BCALLRET2(vm, regs + 2, 8, 1); // append
    U_POPREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 258, 9);
    U_PUSHINT(vm, regs + 0, 1);
    U_PUSHINT(vm, regs + 1, 2);
    U_NEWOBJECT(vm, regs + 2, 883); // B
    U_NEWVEC(vm, regs + 1, 881, 1);
    SetLVal(vm, &locals[2]); // xs
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 260, 9);
    U_STATEMENT(vm, regs + 0, 261, 9);
    U_PUSHINT(vm, regs + 0, 1);
    U_PUSHINT(vm, regs + 1, 2);
    U_NEWOBJECT(vm, regs + 2, 883); // B
    U_NEWVEC(vm, regs + 1, 895, 1);
    fun_255(vm, regs + 1); // call: f
    U_STATEMENT(vm, regs + 0, 263, 9);
    regs[0] = locals[0]; // as
    U_PUSHINT(vm, regs + 1, 0);
    U_VPUSHIDXI(vm, regs + 2);
    U_PUSHFLD(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 1, 1);
    U_IEQ(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block30439;
    U_STATEMENT(vm, regs + 0, 263, 9);
    U_PUSHINT(vm, regs + 0, 1);
    U_NEWOBJECT(vm, regs + 1, 872); // A
    U_NEWVEC(vm, regs + 1, 881, 1);
    goto block30458;
    block30439:;
    U_STATEMENT(vm, regs + 0, 263, 9);
    U_PUSHINT(vm, regs + 0, 1);
    U_PUSHINT(vm, regs + 1, 2);
    U_NEWOBJECT(vm, regs + 2, 883); // B
    U_NEWVEC(vm, regs + 1, 895, 1);
    block30458:;
    fun_255(vm, regs + 1); // call: f
    U_RETURNLOCAL(vm, 0, 0);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[2]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// f
static void fun_255(VMRef vm, StackPtr psp) {
    Value regs[1];
    (void)regs;
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2197, locals);
    U_STATEMENT(vm, regs + 0, 260, 9);
    U_RETURNLOCAL(vm, 0, 0);
    DecVal(vm, locals[0]);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function236
static void fun_256(VMRef vm, StackPtr psp) {
    Value regs[1];
    (void)regs;
    PushFunId(vm, funinfo_table + 2203, 0);
    U_STATEMENT(vm, regs + 0, 267, 9);
    U_STATEMENT(vm, regs + 0, 267, 9);
    U_STATEMENT(vm, regs + 0, 270, 9);
    U_RETURNLOCAL(vm, 0, 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function237
static void fun_257(VMRef vm, StackPtr psp) {
    Value regs[6];
    Value locals[1];
    locals[0] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2208, locals);
    U_STATEMENT(vm, regs + 0, 274, 9);
    U_STATEMENT(vm, regs + 0, 274, 9);
    U_STATEMENT(vm, regs + 0, 279, 9);
    U_PUSHNIL(vm, regs + 0);
    U_PUSHINT(vm, regs + 1, 1);
    U_PUSHVARVF(vm, regs + 2, 123, 2); // int2_0+0
    U_NEWOBJECT(vm, regs + 4, 897); // A
    U_PUSHNIL(vm, regs + 1);
    U_PUSHINT(vm, regs + 2, 1);
    U_PUSHVARVF(vm, regs + 3, 123, 2); // int2_0+0
    U_NEWOBJECT(vm, regs + 5, 897); // A
    U_PUSHINT(vm, regs + 2, 1);
    U_PUSHVARVF(vm, regs + 3, 123, 2); // int2_0+0
    U_NEWOBJECT(vm, regs + 5, 897); // A
    U_PUSHNIL(vm, regs + 2);
    U_PUSHINT(vm, regs + 3, 2);
    U_PUSHINT(vm, regs + 4, 3);
    U_PUSHINT(vm, regs + 5, 4);
    U_NEWOBJECT(vm, regs + 6, 897); // A
    U_PUSHINT(vm, regs + 3, 1);
    U_PUSHVARVF(vm, regs + 4, 123, 2); // int2_0+0
    U_NEWOBJECT(vm, regs + 6, 897); // A
    U_NEWVEC(vm, regs + 3, 917, 3);
    SetLVal(vm, &locals[0]); // as
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 284, 9);
    regs[0] = locals[0]; // as
    U_PUSHINT(vm, regs + 1, 2);
    U_VPUSHIDXI(vm, regs + 2);
    U_PUSHFLD(vm, regs + 1, 0);
    U_ASSERTR(vm, regs + 1, 284, 9, 444);
    U_PUSHFLD(vm, regs + 1, 3);
    U_PUSHINT(vm, regs + 1, 4);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 284, 9, 445);
    U_STATEMENT(vm, regs + 0, 286, 9);
    U_STATEMENT(vm, regs + 0, 288, 9);
    U_PUSHNIL(vm, regs + 0);
    U_PUSHINT(vm, regs + 1, 1);
    U_PUSHVARVF(vm, regs + 2, 123, 2); // int2_0+0
    U_NEWOBJECT(vm, regs + 4, 897); // A
    fun_258(vm, regs + 1); // call: foo
    U_STATEMENT(vm, regs + 0, 289, 9);
    U_PUSHNIL(vm, regs + 0);
    U_PUSHINT(vm, regs + 1, 1);
    U_PUSHVARVF(vm, regs + 2, 123, 2); // int2_0+0
    U_NEWOBJECT(vm, regs + 4, 897); // A
    U_PUSHINT(vm, regs + 1, 1);
    U_PUSHVARVF(vm, regs + 2, 123, 2); // int2_0+0
    U_NEWOBJECT(vm, regs + 4, 897); // A
    fun_258(vm, regs + 1); // call: foo
    U_STATEMENT(vm, regs + 0, 290, 9);
    U_PUSHNIL(vm, regs + 0);
    U_PUSHINT(vm, regs + 1, 2);
    U_PUSHVARVF(vm, regs + 2, 123, 2); // int2_0+0
    U_NEWOBJECT(vm, regs + 4, 897); // A
    U_PUSHINT(vm, regs + 1, 1);
    U_PUSHVARVF(vm, regs + 2, 123, 2); // int2_0+0
    U_NEWOBJECT(vm, regs + 4, 897); // A
    fun_258(vm, regs + 1); // call: foo
    U_RETURNLOCAL(vm, 0, 0);
    DecVal(vm, locals[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// foo
static void fun_258(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2214, locals);
    U_STATEMENT(vm, regs + 0, 287, 9);
    regs[0] = locals[0]; // a
    U_PUSHFLD(vm, regs + 1, 1);
    U_PUSHINT(vm, regs + 1, 1);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 287, 9, 446);
    U_RETURNLOCAL(vm, 0, 0);
    DecVal(vm, locals[0]);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// astar_generic
static void fun_557(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[5];
    locals[0] = *(psp - 4);
    locals[1] = *(psp - 3);
    locals[2] = *(psp - 2);
    SwapVars(vm, 1404, psp, 1);
    BackupVar(vm, 1405);
    BackupVar(vm, 1406);
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2220, locals);
    U_STATEMENT(vm, regs + 0, 23, 11);
    regs[0] = locals[0]; // startnode
    U_INCREF(vm, regs + 1, 0);
    U_NEWVEC(vm, regs + 1, 148, 1);
    U_LVAL_VARF(vm, regs + 1, 1405); // openlist
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 24, 11);
    regs[0] = locals[0]; // startnode
    if (!U_JUMPNOFAILR(vm, regs + 1)) goto block30798;
    U_POP(vm, regs + 1);
    U_PUSHNIL(vm, regs + 0);
    block30798:;
    U_INCREF(vm, regs + 1, 0);
    U_LVAL_VARF(vm, regs + 1, 1406); // n
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 25, 11);
    block30812:;
    U_PUSHVARF(vm, regs + 0, 1406); // n
    U_E2B(vm, regs + 1);
    if (!U_JUMPFAILR(vm, regs + 1)) goto block30832;
    U_POP(vm, regs + 1);
    U_PUSHVARF(vm, regs + 0, 1406); // n
    fun_275(vm, regs + 1); // call: function255
    U_LOGNOT(vm, regs + 1);
    block30832:;
    if (!U_JUMPFAIL(vm, regs + 1)) goto block31021;
    U_STATEMENT(vm, regs + 0, 26, 11);
    U_PUSHVARF(vm, regs + 0, 1405); // openlist
    U_PUSHVARF(vm, regs + 1, 1406); // n
    U_BCALLRET2(vm, regs + 2, 21, 1); // remove_obj
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 27, 11);
    U_PUSHINT(vm, regs + 0, 1);
    U_PUSHVARF(vm, regs + 1, 1406); // n
    U_LVAL_FLD(vm, regs + 2, 7);
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 37, 11);
    U_PUSHVARF(vm, regs + 0, 1406); // n
    U_INCREF(vm, regs + 1, 0);
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_276(vm, regs + 2); // call: function256
    U_STATEMENT(vm, regs + 0, 38, 11);
    U_PUSHNIL(vm, regs + 0);
    U_LVAL_VARF(vm, regs + 1, 1406); // n
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 39, 11);
    U_PUSHINT(vm, regs + 0, -1);
    U_PUSHVARF(vm, regs + 1, 1405); // openlist
    block30905:;
    if (!U_VFOR(vm, regs + 2)) goto block31016;
    U_STATEMENT(vm, regs + 2, 39, 11);
    U_VFORELEMREF(vm, regs + 2);
    SetLVal(vm, &locals[3]); // c
    U_LV_WRITEREF(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 40, 11);
    U_PUSHVARF(vm, regs + 2, 1406); // n
    U_LOGNOTREF(vm, regs + 3);
    if (!U_JUMPNOFAILR(vm, regs + 3)) goto block30949;
    U_POP(vm, regs + 3);
    regs[2] = locals[3]; // c
    U_PUSHFLD(vm, regs + 3, 2);
    U_PUSHVARF(vm, regs + 3, 1406); // n
    U_PUSHFLD(vm, regs + 4, 2);
    U_FLT(vm, regs + 4);
    block30949:;
    if (!U_JUMPNOFAILR(vm, regs + 3)) goto block30991;
    U_POP(vm, regs + 3);
    regs[2] = locals[3]; // c
    U_PUSHFLD(vm, regs + 3, 2);
    U_PUSHVARF(vm, regs + 3, 1406); // n
    U_PUSHFLD(vm, regs + 4, 2);
    U_FEQ(vm, regs + 4);
    if (!U_JUMPFAILR(vm, regs + 3)) goto block30989;
    U_POP(vm, regs + 3);
    regs[2] = locals[3]; // c
    U_PUSHFLD(vm, regs + 3, 1);
    U_PUSHVARF(vm, regs + 3, 1406); // n
    U_PUSHFLD(vm, regs + 4, 1);
    U_FLT(vm, regs + 4);
    block30989:;
    block30991:;
    if (!U_JUMPFAIL(vm, regs + 3)) goto block31011;
    U_STATEMENT(vm, regs + 2, 41, 11);
    regs[2] = locals[3]; // c
    U_INCREF(vm, regs + 3, 0);
    U_LVAL_VARF(vm, regs + 3, 1406); // n
    U_LV_WRITEREF(vm, regs + 3);
    block31011:;
    goto block30905;
    block31016:;
    goto block30812;
    block31021:;
    U_STATEMENT(vm, regs + 0, 42, 11);
    U_NEWVEC(vm, regs + 0, 148, 0);
    SetLVal(vm, &locals[4]); // path
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 43, 11);
    block31040:;
    U_PUSHVARF(vm, regs + 0, 1406); // n
    if (!U_JUMPFAIL(vm, regs + 1)) goto block31091;
    U_STATEMENT(vm, regs + 0, 44, 11);
    regs[0] = locals[4]; // path
    U_PUSHVARF(vm, regs + 1, 1406); // n
    U_INCREF(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 2, 1);
    U_BCALLRETV(vm, regs + 3, 15, 1); // push
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 45, 11);
    U_PUSHVARF(vm, regs + 0, 1406); // n
    U_PUSHFLD(vm, regs + 1, 3);
    U_INCREF(vm, regs + 1, 0);
    U_LVAL_VARF(vm, regs + 1, 1406); // n
    U_LV_WRITEREF(vm, regs + 1);
    goto block31040;
    block31091:;
    U_STATEMENT(vm, regs + 0, 46, 11);
    regs[0] = locals[4]; // path
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    DecOwned(vm, 1405);
    DecOwned(vm, 1406);
    DecVal(vm, locals[3]);
    psp = PopArg(vm, 1404, psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    RestoreBackup(vm, 1406);
    RestoreBackup(vm, 1405);
    PopFunId(vm);
}

// astar_generic
static void fun_546(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[5];
    locals[0] = *(psp - 4);
    locals[1] = *(psp - 3);
    locals[2] = *(psp - 2);
    SwapVars(vm, 1369, psp, 1);
    BackupVar(vm, 1370);
    BackupVar(vm, 1371);
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2233, locals);
    U_STATEMENT(vm, regs + 0, 23, 11);
    regs[0] = locals[0]; // startnode
    U_INCREF(vm, regs + 1, 0);
    U_NEWVEC(vm, regs + 1, 1005, 1);
    U_LVAL_VARF(vm, regs + 1, 1370); // openlist
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 24, 11);
    regs[0] = locals[0]; // startnode
    if (!U_JUMPNOFAILR(vm, regs + 1)) goto block31156;
    U_POP(vm, regs + 1);
    U_PUSHNIL(vm, regs + 0);
    block31156:;
    U_INCREF(vm, regs + 1, 0);
    U_LVAL_VARF(vm, regs + 1, 1371); // n
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 25, 11);
    block31170:;
    U_PUSHVARF(vm, regs + 0, 1371); // n
    U_E2B(vm, regs + 1);
    if (!U_JUMPFAILR(vm, regs + 1)) goto block31190;
    U_POP(vm, regs + 1);
    U_PUSHVARF(vm, regs + 0, 1371); // n
    fun_547(vm, regs + 1); // call: function243
    U_LOGNOT(vm, regs + 1);
    block31190:;
    if (!U_JUMPFAIL(vm, regs + 1)) goto block31379;
    U_STATEMENT(vm, regs + 0, 26, 11);
    U_PUSHVARF(vm, regs + 0, 1370); // openlist
    U_PUSHVARF(vm, regs + 1, 1371); // n
    U_BCALLRET2(vm, regs + 2, 21, 1); // remove_obj
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 27, 11);
    U_PUSHINT(vm, regs + 0, 1);
    U_PUSHVARF(vm, regs + 1, 1371); // n
    U_LVAL_FLD(vm, regs + 2, 9);
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 37, 11);
    U_PUSHVARF(vm, regs + 0, 1371); // n
    U_INCREF(vm, regs + 1, 0);
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_549(vm, regs + 2); // call: function244
    U_STATEMENT(vm, regs + 0, 38, 11);
    U_PUSHNIL(vm, regs + 0);
    U_LVAL_VARF(vm, regs + 1, 1371); // n
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 39, 11);
    U_PUSHINT(vm, regs + 0, -1);
    U_PUSHVARF(vm, regs + 1, 1370); // openlist
    block31263:;
    if (!U_VFOR(vm, regs + 2)) goto block31374;
    U_STATEMENT(vm, regs + 2, 39, 11);
    U_VFORELEMREF(vm, regs + 2);
    SetLVal(vm, &locals[3]); // c
    U_LV_WRITEREF(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 40, 11);
    U_PUSHVARF(vm, regs + 2, 1371); // n
    U_LOGNOTREF(vm, regs + 3);
    if (!U_JUMPNOFAILR(vm, regs + 3)) goto block31307;
    U_POP(vm, regs + 3);
    regs[2] = locals[3]; // c
    U_PUSHFLD(vm, regs + 3, 2);
    U_PUSHVARF(vm, regs + 3, 1371); // n
    U_PUSHFLD(vm, regs + 4, 2);
    U_FLT(vm, regs + 4);
    block31307:;
    if (!U_JUMPNOFAILR(vm, regs + 3)) goto block31349;
    U_POP(vm, regs + 3);
    regs[2] = locals[3]; // c
    U_PUSHFLD(vm, regs + 3, 2);
    U_PUSHVARF(vm, regs + 3, 1371); // n
    U_PUSHFLD(vm, regs + 4, 2);
    U_FEQ(vm, regs + 4);
    if (!U_JUMPFAILR(vm, regs + 3)) goto block31347;
    U_POP(vm, regs + 3);
    regs[2] = locals[3]; // c
    U_PUSHFLD(vm, regs + 3, 1);
    U_PUSHVARF(vm, regs + 3, 1371); // n
    U_PUSHFLD(vm, regs + 4, 1);
    U_FLT(vm, regs + 4);
    block31347:;
    block31349:;
    if (!U_JUMPFAIL(vm, regs + 3)) goto block31369;
    U_STATEMENT(vm, regs + 2, 41, 11);
    regs[2] = locals[3]; // c
    U_INCREF(vm, regs + 3, 0);
    U_LVAL_VARF(vm, regs + 3, 1371); // n
    U_LV_WRITEREF(vm, regs + 3);
    block31369:;
    goto block31263;
    block31374:;
    goto block31170;
    block31379:;
    U_STATEMENT(vm, regs + 0, 42, 11);
    U_NEWVEC(vm, regs + 0, 1005, 0);
    SetLVal(vm, &locals[4]); // path
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 43, 11);
    block31398:;
    U_PUSHVARF(vm, regs + 0, 1371); // n
    if (!U_JUMPFAIL(vm, regs + 1)) goto block31449;
    U_STATEMENT(vm, regs + 0, 44, 11);
    regs[0] = locals[4]; // path
    U_PUSHVARF(vm, regs + 1, 1371); // n
    U_INCREF(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 2, 1);
    U_BCALLRETV(vm, regs + 3, 15, 1); // push
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 45, 11);
    U_PUSHVARF(vm, regs + 0, 1371); // n
    U_PUSHFLD(vm, regs + 1, 3);
    U_INCREF(vm, regs + 1, 0);
    U_LVAL_VARF(vm, regs + 1, 1371); // n
    U_LV_WRITEREF(vm, regs + 1);
    goto block31398;
    block31449:;
    U_STATEMENT(vm, regs + 0, 46, 11);
    regs[0] = locals[4]; // path
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    DecOwned(vm, 1370);
    DecOwned(vm, 1371);
    DecVal(vm, locals[3]);
    psp = PopArg(vm, 1369, psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    RestoreBackup(vm, 1371);
    RestoreBackup(vm, 1370);
    PopFunId(vm);
}

// astar_generic
static void fun_260(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[5];
    locals[0] = *(psp - 4);
    locals[1] = *(psp - 3);
    locals[2] = *(psp - 2);
    SwapVars(vm, 477, psp, 1);
    BackupVar(vm, 478);
    BackupVar(vm, 479);
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2246, locals);
    U_STATEMENT(vm, regs + 0, 23, 11);
    regs[0] = locals[0]; // startnode
    U_INCREF(vm, regs + 1, 0);
    U_NEWVEC(vm, regs + 1, 1005, 1);
    U_LVAL_VARF(vm, regs + 1, 478); // openlist
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 24, 11);
    regs[0] = locals[0]; // startnode
    if (!U_JUMPNOFAILR(vm, regs + 1)) goto block31514;
    U_POP(vm, regs + 1);
    U_PUSHNIL(vm, regs + 0);
    block31514:;
    U_INCREF(vm, regs + 1, 0);
    U_LVAL_VARF(vm, regs + 1, 479); // n
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 25, 11);
    block31528:;
    U_PUSHVARF(vm, regs + 0, 479); // n
    U_E2B(vm, regs + 1);
    if (!U_JUMPFAILR(vm, regs + 1)) goto block31548;
    U_POP(vm, regs + 1);
    U_PUSHVARF(vm, regs + 0, 479); // n
    fun_263(vm, regs + 1); // call: function243
    U_LOGNOT(vm, regs + 1);
    block31548:;
    if (!U_JUMPFAIL(vm, regs + 1)) goto block31737;
    U_STATEMENT(vm, regs + 0, 26, 11);
    U_PUSHVARF(vm, regs + 0, 478); // openlist
    U_PUSHVARF(vm, regs + 1, 479); // n
    U_BCALLRET2(vm, regs + 2, 21, 1); // remove_obj
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 27, 11);
    U_PUSHINT(vm, regs + 0, 1);
    U_PUSHVARF(vm, regs + 1, 479); // n
    U_LVAL_FLD(vm, regs + 2, 9);
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 37, 11);
    U_PUSHVARF(vm, regs + 0, 479); // n
    U_INCREF(vm, regs + 1, 0);
    U_PUSHFUN(vm, regs + 1, 0, fun_261);
    fun_264(vm, regs + 2); // call: function244
    U_STATEMENT(vm, regs + 0, 38, 11);
    U_PUSHNIL(vm, regs + 0);
    U_LVAL_VARF(vm, regs + 1, 479); // n
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 39, 11);
    U_PUSHINT(vm, regs + 0, -1);
    U_PUSHVARF(vm, regs + 1, 478); // openlist
    block31621:;
    if (!U_VFOR(vm, regs + 2)) goto block31732;
    U_STATEMENT(vm, regs + 2, 39, 11);
    U_VFORELEMREF(vm, regs + 2);
    SetLVal(vm, &locals[3]); // c
    U_LV_WRITEREF(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 40, 11);
    U_PUSHVARF(vm, regs + 2, 479); // n
    U_LOGNOTREF(vm, regs + 3);
    if (!U_JUMPNOFAILR(vm, regs + 3)) goto block31665;
    U_POP(vm, regs + 3);
    regs[2] = locals[3]; // c
    U_PUSHFLD(vm, regs + 3, 2);
    U_PUSHVARF(vm, regs + 3, 479); // n
    U_PUSHFLD(vm, regs + 4, 2);
    U_FLT(vm, regs + 4);
    block31665:;
    if (!U_JUMPNOFAILR(vm, regs + 3)) goto block31707;
    U_POP(vm, regs + 3);
    regs[2] = locals[3]; // c
    U_PUSHFLD(vm, regs + 3, 2);
    U_PUSHVARF(vm, regs + 3, 479); // n
    U_PUSHFLD(vm, regs + 4, 2);
    U_FEQ(vm, regs + 4);
    if (!U_JUMPFAILR(vm, regs + 3)) goto block31705;
    U_POP(vm, regs + 3);
    regs[2] = locals[3]; // c
    U_PUSHFLD(vm, regs + 3, 1);
    U_PUSHVARF(vm, regs + 3, 479); // n
    U_PUSHFLD(vm, regs + 4, 1);
    U_FLT(vm, regs + 4);
    block31705:;
    block31707:;
    if (!U_JUMPFAIL(vm, regs + 3)) goto block31727;
    U_STATEMENT(vm, regs + 2, 41, 11);
    regs[2] = locals[3]; // c
    U_INCREF(vm, regs + 3, 0);
    U_LVAL_VARF(vm, regs + 3, 479); // n
    U_LV_WRITEREF(vm, regs + 3);
    block31727:;
    goto block31621;
    block31732:;
    goto block31528;
    block31737:;
    U_STATEMENT(vm, regs + 0, 42, 11);
    U_NEWVEC(vm, regs + 0, 1005, 0);
    SetLVal(vm, &locals[4]); // path
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 43, 11);
    block31756:;
    U_PUSHVARF(vm, regs + 0, 479); // n
    if (!U_JUMPFAIL(vm, regs + 1)) goto block31807;
    U_STATEMENT(vm, regs + 0, 44, 11);
    regs[0] = locals[4]; // path
    U_PUSHVARF(vm, regs + 1, 479); // n
    U_INCREF(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 2, 1);
    U_BCALLRETV(vm, regs + 3, 15, 1); // push
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 45, 11);
    U_PUSHVARF(vm, regs + 0, 479); // n
    U_PUSHFLD(vm, regs + 1, 3);
    U_INCREF(vm, regs + 1, 0);
    U_LVAL_VARF(vm, regs + 1, 479); // n
    U_LV_WRITEREF(vm, regs + 1);
    goto block31756;
    block31807:;
    U_STATEMENT(vm, regs + 0, 46, 11);
    regs[0] = locals[4]; // path
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    DecOwned(vm, 478);
    DecOwned(vm, 479);
    DecVal(vm, locals[3]);
    psp = PopArg(vm, 477, psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    RestoreBackup(vm, 479);
    RestoreBackup(vm, 478);
    PopFunId(vm);
}

// function241
static void fun_558(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[4];
    locals[0] = *(psp - 3);
    locals[1] = *(psp - 2);
    locals[2] = *(psp - 1);
    locals[3] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2259, locals);
    U_STATEMENT(vm, regs + 0, 29, 11);
    regs[0] = locals[2]; // nn
    U_PUSHFLD(vm, regs + 1, 7);
    U_LOGNOT(vm, regs + 1);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block32042;
    U_STATEMENT(vm, regs + 0, 30, 11);
    U_PUSHVARF(vm, regs + 0, 1406); // n
    U_PUSHFLD(vm, regs + 1, 0);
    regs[1] = locals[1]; // cost
    U_I2F(vm, regs + 2);
    U_FADD(vm, regs + 2);
    SetLVal(vm, &locals[3]); // G
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 31, 11);
    regs[0] = locals[2]; // nn
    U_PUSHFLD(vm, regs + 1, 6);
    U_LOGNOT(vm, regs + 1);
    if (!U_JUMPFAILR(vm, regs + 1)) goto block31903;
    U_POP(vm, regs + 1);
    U_PUSHVARF(vm, regs + 0, 1405); // openlist
    regs[1] = locals[2]; // nn
    U_INCREF(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 2, 1);
    U_BCALLRETV(vm, regs + 3, 15, 1); // push
    block31903:;
    U_E2B(vm, regs + 1);
    if (!U_JUMPNOFAILR(vm, regs + 1)) goto block31923;
    U_POP(vm, regs + 1);
    regs[0] = locals[3]; // G
    regs[1] = locals[2]; // nn
    U_PUSHFLD(vm, regs + 2, 0);
    U_FLT(vm, regs + 2);
    block31923:;
    if (!U_JUMPFAIL(vm, regs + 1)) goto block32040;
    U_STATEMENT(vm, regs + 0, 32, 11);
    U_PUSHINT(vm, regs + 0, 1);
    regs[1] = locals[2]; // nn
    U_LVAL_FLD(vm, regs + 2, 6);
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 33, 11);
    regs[0] = locals[0]; // delta
    U_INCREF(vm, regs + 1, 0);
    regs[1] = locals[2]; // nn
    U_LVAL_FLD(vm, regs + 2, 5);
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 34, 11);
    U_PUSHVARF(vm, regs + 0, 1406); // n
    U_INCREF(vm, regs + 1, 0);
    regs[1] = locals[2]; // nn
    U_LVAL_FLD(vm, regs + 2, 3);
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 35, 11);
    regs[0] = locals[2]; // nn
    U_PUSHFLD(vm, regs + 1, 4);
    fun_278(vm, regs + 1); // call: function258
    U_I2F(vm, regs + 1);
    regs[1] = locals[2]; // nn
    U_LVAL_FLD(vm, regs + 2, 1);
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 36, 11);
    regs[0] = locals[3]; // G
    regs[1] = locals[2]; // nn
    U_LVAL_FLD(vm, regs + 2, 0);
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 37, 11);
    regs[0] = locals[3]; // G
    regs[1] = locals[2]; // nn
    U_PUSHFLD(vm, regs + 2, 1);
    U_FADD(vm, regs + 2);
    regs[1] = locals[2]; // nn
    U_LVAL_FLD(vm, regs + 2, 2);
    U_LV_WRITE(vm, regs + 1);
    block32040:;
    block32042:;
    U_RETURNLOCAL(vm, 0, 0);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[2]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function241
static void fun_553(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[5];
    locals[0] = *(psp - 4);
    locals[1] = *(psp - 3);
    locals[2] = *(psp - 2);
    locals[3] = *(psp - 1);
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2268, locals);
    U_STATEMENT(vm, regs + 0, 29, 11);
    regs[0] = locals[3]; // nn
    U_PUSHFLD(vm, regs + 1, 9);
    U_LOGNOT(vm, regs + 1);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block32270;
    U_STATEMENT(vm, regs + 0, 30, 11);
    U_PUSHVARF(vm, regs + 0, 1371); // n
    U_PUSHFLD(vm, regs + 1, 0);
    regs[1] = locals[2]; // cost
    U_I2F(vm, regs + 2);
    U_FADD(vm, regs + 2);
    SetLVal(vm, &locals[4]); // G
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 31, 11);
    regs[0] = locals[3]; // nn
    U_PUSHFLD(vm, regs + 1, 8);
    U_LOGNOT(vm, regs + 1);
    if (!U_JUMPFAILR(vm, regs + 1)) goto block32131;
    U_POP(vm, regs + 1);
    U_PUSHVARF(vm, regs + 0, 1370); // openlist
    regs[1] = locals[3]; // nn
    U_INCREF(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 2, 1);
    U_BCALLRETV(vm, regs + 3, 15, 1); // push
    block32131:;
    U_E2B(vm, regs + 1);
    if (!U_JUMPNOFAILR(vm, regs + 1)) goto block32151;
    U_POP(vm, regs + 1);
    regs[0] = locals[4]; // G
    regs[1] = locals[3]; // nn
    U_PUSHFLD(vm, regs + 2, 0);
    U_FLT(vm, regs + 2);
    block32151:;
    if (!U_JUMPFAIL(vm, regs + 1)) goto block32268;
    U_STATEMENT(vm, regs + 0, 32, 11);
    U_PUSHINT(vm, regs + 0, 1);
    regs[1] = locals[3]; // nn
    U_LVAL_FLD(vm, regs + 2, 8);
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 33, 11);
    regs[0] = locals[0];regs[1] = locals[1]; // delta
    regs[2] = locals[3]; // nn
    U_LVAL_FLD(vm, regs + 3, 6);
    U_LV_WRITEV(vm, regs + 2, 2);
    U_STATEMENT(vm, regs + 0, 34, 11);
    U_PUSHVARF(vm, regs + 0, 1371); // n
    U_INCREF(vm, regs + 1, 0);
    regs[1] = locals[3]; // nn
    U_LVAL_FLD(vm, regs + 2, 3);
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 35, 11);
    regs[0] = locals[3]; // nn
    U_PUSHFLD2V(vm, regs + 1, 4, 2);
    fun_554(vm, regs + 2); // call: function246
    U_I2F(vm, regs + 1);
    regs[1] = locals[3]; // nn
    U_LVAL_FLD(vm, regs + 2, 1);
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 36, 11);
    regs[0] = locals[4]; // G
    regs[1] = locals[3]; // nn
    U_LVAL_FLD(vm, regs + 2, 0);
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 37, 11);
    regs[0] = locals[4]; // G
    regs[1] = locals[3]; // nn
    U_PUSHFLD(vm, regs + 2, 1);
    U_FADD(vm, regs + 2);
    regs[1] = locals[3]; // nn
    U_LVAL_FLD(vm, regs + 2, 2);
    U_LV_WRITE(vm, regs + 1);
    block32268:;
    block32270:;
    U_RETURNLOCAL(vm, 0, 0);
    DecVal(vm, locals[3]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function241
static void fun_261(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[5];
    locals[0] = *(psp - 4);
    locals[1] = *(psp - 3);
    locals[2] = *(psp - 2);
    locals[3] = *(psp - 1);
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2278, locals);
    U_STATEMENT(vm, regs + 0, 29, 11);
    regs[0] = locals[3]; // nn
    U_PUSHFLD(vm, regs + 1, 9);
    U_LOGNOT(vm, regs + 1);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block32496;
    U_STATEMENT(vm, regs + 0, 30, 11);
    U_PUSHVARF(vm, regs + 0, 479); // n
    U_PUSHFLD(vm, regs + 1, 0);
    regs[1] = locals[2]; // cost
    U_I2F(vm, regs + 2);
    U_FADD(vm, regs + 2);
    SetLVal(vm, &locals[4]); // G
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 31, 11);
    regs[0] = locals[3]; // nn
    U_PUSHFLD(vm, regs + 1, 8);
    U_LOGNOT(vm, regs + 1);
    if (!U_JUMPFAILR(vm, regs + 1)) goto block32359;
    U_POP(vm, regs + 1);
    U_PUSHVARF(vm, regs + 0, 478); // openlist
    regs[1] = locals[3]; // nn
    U_INCREF(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 2, 1);
    U_BCALLRETV(vm, regs + 3, 15, 1); // push
    block32359:;
    U_E2B(vm, regs + 1);
    if (!U_JUMPNOFAILR(vm, regs + 1)) goto block32379;
    U_POP(vm, regs + 1);
    regs[0] = locals[4]; // G
    regs[1] = locals[3]; // nn
    U_PUSHFLD(vm, regs + 2, 0);
    U_FLT(vm, regs + 2);
    block32379:;
    if (!U_JUMPFAIL(vm, regs + 1)) goto block32494;
    U_STATEMENT(vm, regs + 0, 32, 11);
    U_PUSHINT(vm, regs + 0, 1);
    regs[1] = locals[3]; // nn
    U_LVAL_FLD(vm, regs + 2, 8);
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 33, 11);
    regs[0] = locals[0];regs[1] = locals[1]; // delta
    regs[2] = locals[3]; // nn
    U_LVAL_FLD(vm, regs + 3, 6);
    U_LV_WRITEV(vm, regs + 2, 2);
    U_STATEMENT(vm, regs + 0, 34, 11);
    U_PUSHVARF(vm, regs + 0, 479); // n
    U_INCREF(vm, regs + 1, 0);
    regs[1] = locals[3]; // nn
    U_LVAL_FLD(vm, regs + 2, 3);
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 35, 11);
    regs[0] = locals[3]; // nn
    U_PUSHFLD2V(vm, regs + 1, 4, 2);
    fun_266(vm, regs + 2); // call: function246
    regs[1] = locals[3]; // nn
    U_LVAL_FLD(vm, regs + 2, 1);
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 36, 11);
    regs[0] = locals[4]; // G
    regs[1] = locals[3]; // nn
    U_LVAL_FLD(vm, regs + 2, 0);
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 37, 11);
    regs[0] = locals[4]; // G
    regs[1] = locals[3]; // nn
    U_PUSHFLD(vm, regs + 2, 1);
    U_FADD(vm, regs + 2);
    regs[1] = locals[3]; // nn
    U_LVAL_FLD(vm, regs + 2, 2);
    U_LV_WRITE(vm, regs + 1);
    block32494:;
    block32496:;
    U_RETURNLOCAL(vm, 0, 0);
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
static void fun_542(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[1];
    locals[0] = *(psp - 5);
    SwapVars(vm, 1362, psp, 4);
    SwapVars(vm, 1363, psp, 3);
    SwapVars(vm, 1364, psp, 2);
    SwapVars(vm, 1365, psp, 1);
    PushFunId(vm, funinfo_table + 2288, locals);
    U_STATEMENT(vm, regs + 0, 63, 11);
    regs[0] = locals[0]; // startnode
    U_INCREF(vm, regs + 1, 0);
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    U_PUSHFUN(vm, regs + 2, 0, fun_10000001);
    U_PUSHFUN(vm, regs + 3, 0, fun_10000001);
    fun_546(vm, regs + 4); // call: astar_generic
    U_RETURNLOCAL(vm, 0, 1);
    psp = PopArg(vm, 1365, psp);
    psp = PopArg(vm, 1364, psp);
    psp = PopArg(vm, 1363, psp);
    psp = PopArg(vm, 1362, psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// astar_graph
static void fun_262(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[1];
    locals[0] = *(psp - 5);
    SwapVars(vm, 488, psp, 4);
    SwapVars(vm, 489, psp, 3);
    SwapVars(vm, 490, psp, 2);
    SwapVars(vm, 491, psp, 1);
    PushFunId(vm, funinfo_table + 2298, locals);
    U_STATEMENT(vm, regs + 0, 63, 11);
    regs[0] = locals[0]; // startnode
    U_INCREF(vm, regs + 1, 0);
    U_PUSHFUN(vm, regs + 1, 0, fun_263);
    U_PUSHFUN(vm, regs + 2, 0, fun_264);
    U_PUSHFUN(vm, regs + 3, 0, fun_266);
    fun_260(vm, regs + 4); // call: astar_generic
    U_RETURNLOCAL(vm, 0, 1);
    psp = PopArg(vm, 491, psp);
    psp = PopArg(vm, 490, psp);
    psp = PopArg(vm, 489, psp);
    psp = PopArg(vm, 488, psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function243
static void fun_547(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2308, locals);
    U_STATEMENT(vm, regs + 0, 56, 11);
    regs[0] = locals[0]; // n
    U_PUSHVARF(vm, regs + 1, 1362); // endnode
    U_AEQ(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function243
static void fun_263(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2314, locals);
    U_STATEMENT(vm, regs + 0, 56, 11);
    regs[0] = locals[0]; // n
    U_PUSHVARF(vm, regs + 1, 488); // endnode
    U_AEQ(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function244
static void fun_549(VMRef vm, StackPtr psp) {
    Value regs[2];
    SwapVars(vm, 1375, psp, 2);
    SwapVars(vm, 1376, psp, 1);
    PushFunId(vm, funinfo_table + 2320, 0);
    U_STATEMENT(vm, regs + 0, 61, 11);
    U_PUSHVARF(vm, regs + 0, 1375); // n
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_551(vm, regs + 2); // call: function249
    U_RETURNLOCAL(vm, 0, 0);
    DecOwned(vm, 1375);
    psp = PopArg(vm, 1376, psp);
    psp = PopArg(vm, 1375, psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function244
static void fun_264(VMRef vm, StackPtr psp) {
    Value regs[2];
    SwapVars(vm, 493, psp, 2);
    SwapVars(vm, 494, psp, 1);
    PushFunId(vm, funinfo_table + 2327, 0);
    U_STATEMENT(vm, regs + 0, 61, 11);
    U_PUSHVARF(vm, regs + 0, 493); // n
    U_PUSHFUN(vm, regs + 1, 0, fun_265);
    fun_269(vm, regs + 2); // call: function249
    U_RETURNLOCAL(vm, 0, 0);
    DecOwned(vm, 493);
    psp = PopArg(vm, 494, psp);
    psp = PopArg(vm, 493, psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function245
static void fun_552(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[2];
    locals[0] = *(psp - 1);
    locals[1] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2334, locals);
    U_STATEMENT(vm, regs + 0, 59, 11);
    U_PUSHVARF(vm, regs + 0, 1375); // n
    regs[1] = locals[0]; // nn
    fun_283(vm, regs + 2); // call: function263
    SetLVal(vm, &locals[1]); // cost
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 60, 11);
    regs[0] = locals[1]; // cost
    U_I2F(vm, regs + 1);
    U_PUSHFLT(vm, regs + 1, 0);
    U_FGT(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block32758;
    U_STATEMENT(vm, regs + 0, 61, 11);
    regs[0] = locals[0]; // nn
    U_PUSHFLD2V(vm, regs + 1, 4, 2);
    U_PUSHVARF(vm, regs + 2, 1375); // n
    U_PUSHFLD2V(vm, regs + 3, 4, 2);
    U_IVVSUB(vm, regs + 4, 2);
    regs[2] = locals[1]; // cost
    regs[3] = locals[0]; // nn
    U_INCREF(vm, regs + 4, 0);
    fun_553(vm, regs + 4); // call: function241
    block32758:;
    U_RETURNLOCAL(vm, 0, 0);
    DecVal(vm, locals[0]);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function245
static void fun_265(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[2];
    locals[0] = *(psp - 1);
    locals[1] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2341, locals);
    U_STATEMENT(vm, regs + 0, 59, 11);
    U_PUSHVARF(vm, regs + 0, 493); // n
    regs[1] = locals[0]; // nn
    fun_283(vm, regs + 2); // call: function263
    SetLVal(vm, &locals[1]); // cost
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 60, 11);
    regs[0] = locals[1]; // cost
    U_I2F(vm, regs + 1);
    U_PUSHFLT(vm, regs + 1, 0);
    U_FGT(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block32842;
    U_STATEMENT(vm, regs + 0, 61, 11);
    regs[0] = locals[0]; // nn
    U_PUSHFLD2V(vm, regs + 1, 4, 2);
    U_PUSHVARF(vm, regs + 2, 493); // n
    U_PUSHFLD2V(vm, regs + 3, 4, 2);
    U_IVVSUB(vm, regs + 4, 2);
    regs[2] = locals[1]; // cost
    regs[3] = locals[0]; // nn
    U_INCREF(vm, regs + 4, 0);
    fun_261(vm, regs + 4); // call: function241
    block32842:;
    U_RETURNLOCAL(vm, 0, 0);
    DecVal(vm, locals[0]);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function246
static void fun_554(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2348, locals);
    U_STATEMENT(vm, regs + 0, 63, 11);
    regs[0] = locals[0];regs[1] = locals[1]; // state
    U_PUSHVARF(vm, regs + 2, 1362); // endnode
    U_PUSHFLD2V(vm, regs + 3, 4, 2);
    U_IVVSUB(vm, regs + 4, 2);
    fun_271(vm, regs + 2); // call: function251
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function246
static void fun_266(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2355, locals);
    U_STATEMENT(vm, regs + 0, 63, 11);
    regs[0] = locals[0];regs[1] = locals[1]; // state
    U_PUSHVARF(vm, regs + 2, 488); // endnode
    U_PUSHFLD2V(vm, regs + 3, 4, 2);
    U_IVVSUB(vm, regs + 4, 2);
    fun_270(vm, regs + 2); // call: function250
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// astar_2dgrid
static void fun_267(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 7);
    SwapVars(vm, 500, psp, 6);
    SwapVars(vm, 501, psp, 5);
    SwapVars(vm, 502, psp, 4);
    SwapVars(vm, 503, psp, 3);
    SwapVars(vm, 504, psp, 2);
    SwapVars(vm, 505, psp, 1);
    BackupVar(vm, 506);
    PushFunId(vm, funinfo_table + 2362, locals);
    U_STATEMENT(vm, regs + 0, 68, 11);
    U_PUSHVARF(vm, regs + 0, 170); // cardinal_directions
    U_INCREF(vm, regs + 1, 0);
    U_LVAL_VARF(vm, regs + 1, 506); // directions
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 69, 11);
    U_STATEMENT(vm, regs + 0, 84, 11);
    regs[0] = locals[0]; // isocta
    if (!U_JUMPFAIL(vm, regs + 1)) goto block32993;
    U_STATEMENT(vm, regs + 0, 76, 11);
    U_PUSHVARF(vm, regs + 0, 506); // directions
    U_PUSHVARF(vm, regs + 1, 171); // diagonal_directions
    U_BCALLRET2(vm, regs + 2, 8, 1); // append
    U_LVAL_VARF(vm, regs + 1, 506); // directions
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 77, 11);
    U_PUSHFUN(vm, regs + 0, 0, fun_270);
    fun_268(vm, regs + 1); // call: astar_distance
    goto block33005;
    block32993:;
    U_STATEMENT(vm, regs + 0, 84, 11);
    U_PUSHFUN(vm, regs + 0, 0, fun_271);
    fun_541(vm, regs + 1); // call: astar_distance
    block33005:;
    U_RETURNLOCAL(vm, 0, 1);
    DecOwned(vm, 506);
    psp = PopArg(vm, 505, psp);
    psp = PopArg(vm, 504, psp);
    psp = PopArg(vm, 503, psp);
    psp = PopArg(vm, 502, psp);
    psp = PopArg(vm, 501, psp);
    psp = PopArg(vm, 500, psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    RestoreBackup(vm, 506);
    PopFunId(vm);
}

// astar_distance
static void fun_541(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2375, locals);
    U_STATEMENT(vm, regs + 0, 74, 11);
    U_PUSHVARF(vm, regs + 0, 502); // startnode
    U_PUSHVARF(vm, regs + 1, 503); // endnode
    U_PUSHVARF(vm, regs + 2, 505); // costf
    regs[3] = locals[0]; // distancef
    U_PUSHFUN(vm, regs + 4, 0, fun_269);
    fun_542(vm, regs + 5); // call: astar_graph
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// astar_distance
static void fun_268(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2381, locals);
    U_STATEMENT(vm, regs + 0, 74, 11);
    U_PUSHVARF(vm, regs + 0, 502); // startnode
    U_PUSHVARF(vm, regs + 1, 503); // endnode
    U_PUSHVARF(vm, regs + 2, 505); // costf
    regs[3] = locals[0]; // distancef
    U_PUSHFUN(vm, regs + 4, 0, fun_269);
    fun_262(vm, regs + 5); // call: astar_graph
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function249
static void fun_551(VMRef vm, StackPtr psp) {
    Value regs[11];
    Value locals[6];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    locals[5] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2387, locals);
    U_STATEMENT(vm, regs + 0, 71, 11);
    U_PUSHINT(vm, regs + 0, -1);
    U_PUSHVARF(vm, regs + 1, 506); // directions
    block33102:;
    if (!U_VFOR(vm, regs + 2)) goto block33196;
    U_STATEMENT(vm, regs + 2, 71, 11);
    U_VFORELEM2S(vm, regs + 2);
    SetLVal(vm, &locals[2]); // delta+0
    U_LV_WRITEV(vm, regs + 4, 2);
    U_STATEMENT(vm, regs + 2, 72, 11);
    regs[2] = locals[0]; // n
    U_PUSHFLD2V(vm, regs + 3, 4, 2);
    regs[4] = locals[2];regs[5] = locals[3]; // delta
    U_IVVADD(vm, regs + 6, 2);
    SetLVal(vm, &locals[4]); // np+0
    U_LV_WRITEV(vm, regs + 4, 2);
    U_STATEMENT(vm, regs + 2, 73, 11);
    regs[2] = locals[4];regs[3] = locals[5]; // np
    U_PUSHINT(vm, regs + 4, 2);
    U_PUSHVARVF(vm, regs + 5, 500, 2); // gridsize+0
    U_PUSHINT(vm, regs + 7, 2);
    U_PUSHINT(vm, regs + 8, 0);
    U_PUSHINT(vm, regs + 9, 0);
    U_PUSHINT(vm, regs + 10, 2);
    U_BCALLRETV(vm, regs + 11, 107, 1); // in_range
    if (!U_JUMPFAIL(vm, regs + 3)) goto block33191;
    U_STATEMENT(vm, regs + 2, 74, 11);
    regs[2] = locals[4];regs[3] = locals[5]; // np
    fun_282(vm, regs + 4); // call: function262
    fun_552(vm, regs + 3); // call: function245
    block33191:;
    goto block33102;
    block33196:;
    U_RETURNLOCAL(vm, 0, 0);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function249
static void fun_269(VMRef vm, StackPtr psp) {
    Value regs[11];
    Value locals[6];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    locals[5] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2398, locals);
    U_STATEMENT(vm, regs + 0, 71, 11);
    U_PUSHINT(vm, regs + 0, -1);
    U_PUSHVARF(vm, regs + 1, 506); // directions
    block33225:;
    if (!U_VFOR(vm, regs + 2)) goto block33319;
    U_STATEMENT(vm, regs + 2, 71, 11);
    U_VFORELEM2S(vm, regs + 2);
    SetLVal(vm, &locals[2]); // delta+0
    U_LV_WRITEV(vm, regs + 4, 2);
    U_STATEMENT(vm, regs + 2, 72, 11);
    regs[2] = locals[0]; // n
    U_PUSHFLD2V(vm, regs + 3, 4, 2);
    regs[4] = locals[2];regs[5] = locals[3]; // delta
    U_IVVADD(vm, regs + 6, 2);
    SetLVal(vm, &locals[4]); // np+0
    U_LV_WRITEV(vm, regs + 4, 2);
    U_STATEMENT(vm, regs + 2, 73, 11);
    regs[2] = locals[4];regs[3] = locals[5]; // np
    U_PUSHINT(vm, regs + 4, 2);
    U_PUSHVARVF(vm, regs + 5, 500, 2); // gridsize+0
    U_PUSHINT(vm, regs + 7, 2);
    U_PUSHINT(vm, regs + 8, 0);
    U_PUSHINT(vm, regs + 9, 0);
    U_PUSHINT(vm, regs + 10, 2);
    U_BCALLRETV(vm, regs + 11, 107, 1); // in_range
    if (!U_JUMPFAIL(vm, regs + 3)) goto block33314;
    U_STATEMENT(vm, regs + 2, 74, 11);
    regs[2] = locals[4];regs[3] = locals[5]; // np
    fun_282(vm, regs + 4); // call: function262
    fun_265(vm, regs + 3); // call: function245
    block33314:;
    goto block33225;
    block33319:;
    U_RETURNLOCAL(vm, 0, 0);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function250
static void fun_270(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[6];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    locals[5] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2409, locals);
    U_STATEMENT(vm, regs + 0, 78, 11);
    regs[0] = locals[0]; // v+0
    U_BCALLRET1(vm, regs + 1, 111, 1); // abs
    SetLVal(vm, &locals[2]); // x
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 79, 11);
    regs[0] = locals[1]; // v+1
    U_BCALLRET1(vm, regs + 1, 111, 1); // abs
    SetLVal(vm, &locals[3]); // y
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 80, 11);
    regs[0] = locals[2]; // x
    regs[1] = locals[3]; // y
    U_BCALLRET2(vm, regs + 2, 127, 1); // max
    SetLVal(vm, &locals[4]); // big
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 81, 11);
    regs[0] = locals[2]; // x
    regs[1] = locals[3]; // y
    U_BCALLRET2(vm, regs + 2, 119, 1); // min
    SetLVal(vm, &locals[5]); // small
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 82, 11);
    U_PUSHFLT(vm, regs + 0, 1073741824);
    U_BCALLRET1(vm, regs + 1, 56, 1); // sqrt
    regs[1] = locals[5]; // small
    U_I2F(vm, regs + 2);
    U_FMUL(vm, regs + 2);
    regs[1] = locals[4]; // big
    U_I2F(vm, regs + 2);
    U_FADD(vm, regs + 2);
    regs[1] = locals[5]; // small
    U_I2F(vm, regs + 2);
    U_FSUB(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function251
static void fun_271(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2420, locals);
    U_STATEMENT(vm, regs + 0, 84, 11);
    regs[0] = locals[0];regs[1] = locals[1]; // _
    U_PUSHINT(vm, regs + 2, 2);
    U_BCALLRETV(vm, regs + 3, 89, 1); // manhattan
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// astar_goap
static void fun_274(VMRef vm, StackPtr psp) {
    Value regs[8];
    Value locals[2];
    SwapVars(vm, 522, psp, 4);
    locals[0] = *(psp - 3);
    SwapVars(vm, 524, psp, 2);
    SwapVars(vm, 525, psp, 1);
    locals[1] = lobster::NilVal();
    BackupVar(vm, 527);
    PushFunId(vm, funinfo_table + 2427, locals);
    U_STATEMENT(vm, regs + 0, 97, 11);
    regs[0] = locals[0]; // initstate
    fun_299(vm, regs + 1); // call: function269
    SetLVal(vm, &locals[1]); // H
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 98, 11);
    U_PUSHFLT(vm, regs + 0, 0);
    regs[1] = locals[1]; // H
    U_I2F(vm, regs + 2);
    regs[2] = locals[1]; // H
    U_I2F(vm, regs + 3);
    U_PUSHNIL(vm, regs + 3);
    regs[4] = locals[0]; // initstate
    U_INCREF(vm, regs + 5, 0);
    U_PUSHNIL(vm, regs + 5);
    U_PUSHINT(vm, regs + 6, 0);
    U_PUSHINT(vm, regs + 7, 0);
    U_NEWOBJECT(vm, regs + 8, 49); // spec_node
    U_NEWVEC(vm, regs + 1, 148, 1);
    U_LVAL_VARF(vm, regs + 1, 527); // existingnodes
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 111, 11);
    U_PUSHVARF(vm, regs + 0, 527); // existingnodes
    U_PUSHINT(vm, regs + 1, 0);
    U_VPUSHIDXI(vm, regs + 2);
    U_INCREF(vm, regs + 1, 0);
    U_PUSHFUN(vm, regs + 1, 0, fun_275);
    U_PUSHFUN(vm, regs + 2, 0, fun_276);
    U_PUSHFUN(vm, regs + 3, 0, fun_278);
    fun_557(vm, regs + 4); // call: astar_generic
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    DecOwned(vm, 527);
    psp = PopArg(vm, 525, psp);
    psp = PopArg(vm, 524, psp);
    Pop(psp);
    psp = PopArg(vm, 522, psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    RestoreBackup(vm, 527);
    PopFunId(vm);
}

// function255
static void fun_275(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2438, locals);
    U_STATEMENT(vm, regs + 0, 100, 11);
    regs[0] = locals[0]; // _
    U_PUSHFLD(vm, regs + 1, 4);
    fun_300(vm, regs + 1); // call: function270
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function256
static void fun_276(VMRef vm, StackPtr psp) {
    Value regs[11];
    Value locals[4];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    locals[2] = lobster::NilVal();
    BackupVar(vm, 532);
    locals[3] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2444, locals);
    U_STATEMENT(vm, regs + 0, 102, 11);
    U_PUSHINT(vm, regs + 0, -1);
    U_PUSHVARF(vm, regs + 1, 522); // goapactions
    block33628:;
    if (!U_VFOR(vm, regs + 2)) goto block33836;
    U_STATEMENT(vm, regs + 2, 102, 11);
    U_VFORELEMREF(vm, regs + 2);
    SetLVal(vm, &locals[2]); // act
    U_LV_WRITEREF(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 103, 11);
    regs[2] = locals[2]; // act
    U_INCREF(vm, regs + 3, 0);
    regs[3] = locals[0]; // n
    U_PUSHFLD(vm, regs + 4, 4);
    U_INCREF(vm, regs + 4, 0);
    U_DDCALL(vm, regs + 4, 0, 1); vm.next_call_target(vm, regs + 4);
    if (!U_JUMPFAIL(vm, regs + 3)) goto block33831;
    U_STATEMENT(vm, regs + 2, 104, 11);
    regs[2] = locals[0]; // n
    U_PUSHFLD(vm, regs + 3, 4);
    U_BCALLRET1(vm, regs + 3, 29, 1); // copy
    U_LVAL_VARF(vm, regs + 3, 532); // nstate
    U_LV_WRITEREF(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 105, 11);
    regs[2] = locals[2]; // act
    U_INCREF(vm, regs + 3, 0);
    U_PUSHVARF(vm, regs + 3, 532); // nstate
    U_INCREF(vm, regs + 4, 0);
    U_DDCALL(vm, regs + 4, 1, 1); vm.next_call_target(vm, regs + 4);
    U_STATEMENT(vm, regs + 2, 106, 11);
    U_PUSHVARF(vm, regs + 2, 527); // existingnodes
    U_PUSHFUN(vm, regs + 3, 0, fun_277);
    fun_23(vm, regs + 4); // call: find
    SetLVal(vm, &locals[3]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 107, 11);
    regs[2] = locals[3]; // i
    U_PUSHINT(vm, regs + 3, 0);
    U_ILT(vm, regs + 4);
    if (!U_JUMPFAIL(vm, regs + 3)) goto block33802;
    U_STATEMENT(vm, regs + 2, 108, 11);
    U_PUSHVARF(vm, regs + 2, 527); // existingnodes
    U_BCALLRET1(vm, regs + 3, 13, 1); // length
    SetLVal(vm, &locals[3]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 109, 11);
    U_PUSHVARF(vm, regs + 2, 527); // existingnodes
    U_PUSHFLT(vm, regs + 3, 0);
    U_PUSHFLT(vm, regs + 4, 0);
    U_PUSHFLT(vm, regs + 5, 0);
    U_PUSHNIL(vm, regs + 6);
    U_PUSHVARF(vm, regs + 7, 532); // nstate
    U_INCREF(vm, regs + 8, 0);
    U_PUSHNIL(vm, regs + 8);
    U_PUSHINT(vm, regs + 9, 0);
    U_PUSHINT(vm, regs + 10, 0);
    U_NEWOBJECT(vm, regs + 11, 49); // spec_node
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    block33802:;
    U_STATEMENT(vm, regs + 2, 110, 11);
    regs[2] = locals[2]; // act
    U_INCREF(vm, regs + 3, 0);
    U_PUSHINT(vm, regs + 3, 1);
    U_PUSHVARF(vm, regs + 4, 527); // existingnodes
    regs[5] = locals[3]; // i
    U_VPUSHIDXI(vm, regs + 6);
    U_INCREF(vm, regs + 5, 0);
    fun_558(vm, regs + 5); // call: function241
    block33831:;
    goto block33628;
    block33836:;
    U_RETURNLOCAL(vm, 0, 0);
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

// function257
static void fun_277(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2454, locals);
    U_STATEMENT(vm, regs + 0, 106, 11);
    regs[0] = locals[0]; // _
    U_PUSHFLD(vm, regs + 1, 4);
    U_PUSHVARF(vm, regs + 1, 532); // nstate
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function258
static void fun_278(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2460, locals);
    U_STATEMENT(vm, regs + 0, 111, 11);
    regs[0] = locals[0]; // _
    fun_299(vm, regs + 1); // call: function269
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function259
static void fun_279(VMRef vm, StackPtr psp) {
    Value regs[12];
    Value locals[4];
    BackupVar(vm, 536);
    locals[0] = lobster::NilVal();
    BackupVar(vm, 538);
    BackupVar(vm, 539);
    BackupVar(vm, 540);
    BackupVar(vm, 541);
    BackupVar(vm, 542);
    BackupVar(vm, 543);
    BackupVar(vm, 544);
    locals[1] = lobster::NilVal();
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2466, locals);
    U_STATEMENT(vm, regs + 0, 6, 10);
    U_PUSHSTR(vm, regs + 0, 447); // "................................."
    U_INCREF(vm, regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 448); // "................S................"
    U_INCREF(vm, regs + 2, 0);
    U_PUSHSTR(vm, regs + 2, 449); // "........#..............#........."
    U_INCREF(vm, regs + 3, 0);
    U_PUSHSTR(vm, regs + 3, 450); // "........#..............#/////////"
    U_INCREF(vm, regs + 4, 0);
    U_PUSHSTR(vm, regs + 4, 451); // "........#..............#........."
    U_INCREF(vm, regs + 5, 0);
    U_PUSHSTR(vm, regs + 5, 452); // "........#..............#........."
    U_INCREF(vm, regs + 6, 0);
    U_PUSHSTR(vm, regs + 6, 453); // "........################........."
    U_INCREF(vm, regs + 7, 0);
    U_PUSHSTR(vm, regs + 7, 454); // "................................."
    U_INCREF(vm, regs + 8, 0);
    U_PUSHSTR(vm, regs + 8, 455); // "...............///..............."
    U_INCREF(vm, regs + 9, 0);
    U_PUSHSTR(vm, regs + 9, 456); // ".............../E/..............."
    U_INCREF(vm, regs + 10, 0);
    U_PUSHSTR(vm, regs + 10, 457); // "...............///..............."
    U_INCREF(vm, regs + 11, 0);
    U_PUSHSTR(vm, regs + 11, 458); // "................................."
    U_INCREF(vm, regs + 12, 0);
    U_NEWVEC(vm, regs + 12, 11, 12);
    U_LVAL_VARF(vm, regs + 1, 536); // initworld
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 21, 10);
    U_PUSHSTR(vm, regs + 0, 459); // "......OXXXXXXXXXXXXXXXXXXXO......"
    U_INCREF(vm, regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 460); // ".....OXPPPPPPPPPPXXXXXXXXXXO....."
    U_INCREF(vm, regs + 2, 0);
    U_PUSHSTR(vm, regs + 2, 461); // ".....OXP#XXXXXXXXXXXXXX#XXXO....."
    U_INCREF(vm, regs + 3, 0);
    U_PUSHSTR(vm, regs + 3, 462); // ".....OXP#XXXXXXXXXXXXXX#XOO//////"
    U_INCREF(vm, regs + 4, 0);
    U_PUSHSTR(vm, regs + 4, 463); // ".....OXP#XXXXXXXXXXXXXX#XO......."
    U_INCREF(vm, regs + 5, 0);
    U_PUSHSTR(vm, regs + 5, 464); // ".....OXP#XXXXXXXXXXXXXX#XO......."
    U_INCREF(vm, regs + 6, 0);
    U_PUSHSTR(vm, regs + 6, 465); // ".....OXP################XO......."
    U_INCREF(vm, regs + 7, 0);
    U_PUSHSTR(vm, regs + 7, 466); // ".....OXPPPPPPPPPPXXXXXXXXO......."
    U_INCREF(vm, regs + 8, 0);
    U_PUSHSTR(vm, regs + 8, 467); // ".....OXXXXXXXXXOPOXXXXXXXO......."
    U_INCREF(vm, regs + 9, 0);
    U_PUSHSTR(vm, regs + 9, 468); // ".....OXXXXXXXXXOPOXXXXXXXO......."
    U_INCREF(vm, regs + 10, 0);
    U_PUSHSTR(vm, regs + 10, 469); // "......OXXXXXXXXO//OOOOOOO........"
    U_INCREF(vm, regs + 11, 0);
    U_PUSHSTR(vm, regs + 11, 470); // ".......OOOOOOOO.................."
    U_INCREF(vm, regs + 12, 0);
    U_NEWVEC(vm, regs + 12, 11, 12);
    SetLVal(vm, &locals[0]); // expected_result
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 36, 10);
    U_STATEMENT(vm, regs + 0, 36, 10);
    U_STATEMENT(vm, regs + 0, 40, 10);
    U_PUSHVARF(vm, regs + 0, 536); // initworld
    U_PUSHINT(vm, regs + 1, 0);
    U_VPUSHIDXI(vm, regs + 2);
    U_BCALLRET1(vm, regs + 1, 12, 1); // length
    U_PUSHVARF(vm, regs + 1, 536); // initworld
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    U_LVAL_VARF(vm, regs + 2, 538); // worldsize+0
    U_LV_WRITEV(vm, regs + 2, 2);
    U_STATEMENT(vm, regs + 0, 42, 10);
    U_PUSHVARVF(vm, regs + 0, 123, 2); // int2_0+0
    U_LVAL_VARF(vm, regs + 2, 540); // startpos+0
    U_LV_WRITEV(vm, regs + 2, 2);
    U_STATEMENT(vm, regs + 0, 43, 10);
    U_PUSHVARVF(vm, regs + 0, 123, 2); // int2_0+0
    U_LVAL_VARF(vm, regs + 2, 542); // endpos+0
    U_LV_WRITEV(vm, regs + 2, 2);
    U_STATEMENT(vm, regs + 0, 45, 10);
    U_PUSHVARF(vm, regs + 0, 539); // worldsize+1
    U_PUSHFUN(vm, regs + 1, 0, fun_280);
    fun_539(vm, regs + 2); // call: map
    U_LVAL_VARF(vm, regs + 1, 544); // world
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 51, 10);
    U_PUSHINT(vm, regs + 0, 0);
    U_PUSHVARVF(vm, regs + 1, 538, 2); // worldsize+0
    U_PUSHVARF(vm, regs + 3, 544); // world
    U_PUSHVARVF(vm, regs + 4, 540, 2); // startpos+0
    U_VPUSHIDXV(vm, regs + 6, 2);
    U_PUSHVARF(vm, regs + 4, 544); // world
    U_PUSHVARVF(vm, regs + 5, 542, 2); // endpos+0
    U_VPUSHIDXV(vm, regs + 7, 2);
    U_PUSHFUN(vm, regs + 5, 0, fun_282);
    U_PUSHFUN(vm, regs + 6, 0, fun_283);
    fun_267(vm, regs + 7); // call: astar_2dgrid
    SetLVal(vm, &locals[1]); // path
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 55, 10);
    regs[0] = locals[1]; // path
    U_BCALLRET1(vm, regs + 1, 13, 1); // length
    U_PUSHINT(vm, regs + 1, 27);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 55, 10, 471);
    U_STATEMENT(vm, regs + 0, 57, 10);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[1]; // path
    block34247:;
    if (!U_VFOR(vm, regs + 2)) goto block34281;
    U_STATEMENT(vm, regs + 2, 57, 10);
    U_VFORELEMREF(vm, regs + 2);
    SetLVal(vm, &locals[2]); // n
    U_LV_WRITEREF(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 58, 10);
    U_PUSHINT(vm, regs + 2, 1);
    regs[3] = locals[2]; // n
    U_LVAL_FLD(vm, regs + 4, 10);
    U_LV_WRITE(vm, regs + 3);
    goto block34247;
    block34281:;
    U_STATEMENT(vm, regs + 0, 60, 10);
    U_PUSHVARF(vm, regs + 0, 544); // world
    U_PUSHFUN(vm, regs + 1, 0, fun_284);
    fun_555(vm, regs + 2); // call: map
    SetLVal(vm, &locals[3]); // astar_result
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 66, 10);
    regs[0] = locals[3]; // astar_result
    regs[1] = locals[0]; // expected_result
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 66, 10, 472);
    U_RETURNLOCAL(vm, 0, 0);
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

// function260
static void fun_280(VMRef vm, StackPtr psp) {
    Value regs[2];
    SwapVars(vm, 545, psp, 1);
    PushFunId(vm, funinfo_table + 2483, 0);
    U_STATEMENT(vm, regs + 0, 45, 10);
    U_PUSHVARF(vm, regs + 0, 538); // worldsize+0
    U_PUSHFUN(vm, regs + 1, 0, fun_281);
    fun_540(vm, regs + 2); // call: map
    U_RETURNLOCAL(vm, 0, 1);
    psp = PopArg(vm, 545, psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function261
static void fun_281(VMRef vm, StackPtr psp) {
    Value regs[12];
    Value locals[2];
    locals[0] = *(psp - 1);
    locals[1] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2489, locals);
    U_STATEMENT(vm, regs + 0, 46, 10);
    U_PUSHVARF(vm, regs + 0, 536); // initworld
    U_PUSHVARF(vm, regs + 1, 545); // y
    U_VPUSHIDXI(vm, regs + 2);
    regs[1] = locals[0]; // x
    U_SPUSHIDXI(vm, regs + 2);
    SetLVal(vm, &locals[1]); // c
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 47, 10);
    regs[0] = locals[1]; // c
    U_PUSHINT(vm, regs + 1, 83);
    U_IEQ(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block34411;
    U_STATEMENT(vm, regs + 0, 47, 10);
    regs[0] = locals[0]; // x
    U_PUSHVARF(vm, regs + 1, 545); // y
    U_LVAL_VARF(vm, regs + 2, 540); // startpos+0
    U_LV_WRITEV(vm, regs + 2, 2);
    block34411:;
    U_STATEMENT(vm, regs + 0, 48, 10);
    regs[0] = locals[1]; // c
    U_PUSHINT(vm, regs + 1, 69);
    U_IEQ(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block34444;
    U_STATEMENT(vm, regs + 0, 48, 10);
    regs[0] = locals[0]; // x
    U_PUSHVARF(vm, regs + 1, 545); // y
    U_LVAL_VARF(vm, regs + 2, 542); // endpos+0
    U_LV_WRITEV(vm, regs + 2, 2);
    block34444:;
    U_STATEMENT(vm, regs + 0, 49, 10);
    U_PUSHFLT(vm, regs + 0, 0);
    U_PUSHFLT(vm, regs + 1, 0);
    U_PUSHFLT(vm, regs + 2, 0);
    U_PUSHNIL(vm, regs + 3);
    regs[4] = locals[0]; // x
    U_PUSHVARF(vm, regs + 5, 545); // y
    U_PUSHVARVF(vm, regs + 6, 123, 2); // int2_0+0
    U_PUSHINT(vm, regs + 8, 0);
    U_PUSHINT(vm, regs + 9, 0);
    U_PUSHINT(vm, regs + 10, 0);
    regs[11] = locals[1]; // c
    U_NEWOBJECT(vm, regs + 12, 919); // pathingcell
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function262
static void fun_282(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2496, locals);
    U_STATEMENT(vm, regs + 0, 52, 10);
    U_PUSHVARF(vm, regs + 0, 544); // world
    regs[1] = locals[0];regs[2] = locals[1]; // _
    U_VPUSHIDXV(vm, regs + 3, 2);
    U_INCREF(vm, regs + 1, 0);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function263
static void fun_283(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2503, locals);
    U_STATEMENT(vm, regs + 0, 53, 10);
    regs[0] = locals[1]; // nn
    U_PUSHFLD(vm, regs + 1, 11);
    U_PUSHINT(vm, regs + 1, 35);
    U_IEQ(vm, regs + 2);
    if (!U_JUMPFAILR(vm, regs + 1)) goto block34552;
    U_POP(vm, regs + 1);
    U_PUSHINT(vm, regs + 0, -1);
    block34552:;
    if (!U_JUMPNOFAILR(vm, regs + 1)) goto block34580;
    U_POP(vm, regs + 1);
    regs[0] = locals[1]; // nn
    U_PUSHFLD(vm, regs + 1, 11);
    U_PUSHINT(vm, regs + 1, 47);
    U_IEQ(vm, regs + 2);
    if (!U_JUMPFAILR(vm, regs + 1)) goto block34578;
    U_POP(vm, regs + 1);
    U_PUSHINT(vm, regs + 0, 5);
    block34578:;
    block34580:;
    if (!U_JUMPNOFAILR(vm, regs + 1)) goto block34590;
    U_POP(vm, regs + 1);
    U_PUSHINT(vm, regs + 0, 1);
    block34590:;
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function264
static void fun_284(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2510, locals);
    U_STATEMENT(vm, regs + 0, 60, 10);
    regs[0] = locals[0]; // row
    U_PUSHSTR(vm, regs + 1, 473); // ""
    U_INCREF(vm, regs + 2, 0);
    U_PUSHFUN(vm, regs + 2, 0, fun_285);
    fun_556(vm, regs + 3); // call: fold
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function265
static void fun_285(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value keepvar[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2516, locals);
    keepvar[0] = lobster::NilVal();
    keepvar[1] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 64, 10);
    regs[0] = locals[0]; // accum
    regs[1] = locals[1]; // n
    U_PUSHFLD(vm, regs + 2, 10);
    if (!U_JUMPFAILR(vm, regs + 2)) goto block34657;
    U_POP(vm, regs + 2);
    U_PUSHSTR(vm, regs + 1, 474); // "P"
    block34657:;
    if (!U_JUMPNOFAILR(vm, regs + 2)) goto block34680;
    U_POP(vm, regs + 2);
    regs[1] = locals[1]; // n
    U_PUSHFLD(vm, regs + 2, 9);
    if (!U_JUMPFAILR(vm, regs + 2)) goto block34678;
    U_POP(vm, regs + 2);
    U_PUSHSTR(vm, regs + 1, 475); // "X"
    block34678:;
    block34680:;
    if (!U_JUMPNOFAILR(vm, regs + 2)) goto block34703;
    U_POP(vm, regs + 2);
    regs[1] = locals[1]; // n
    U_PUSHFLD(vm, regs + 2, 8);
    if (!U_JUMPFAILR(vm, regs + 2)) goto block34701;
    U_POP(vm, regs + 2);
    U_PUSHSTR(vm, regs + 1, 476); // "O"
    block34701:;
    block34703:;
    U_INCREF(vm, regs + 2, 0);
    if (!U_JUMPNOFAILR(vm, regs + 2)) goto block34731;
    U_POP(vm, regs + 2);
    regs[1] = locals[1]; // n
    U_PUSHFLD(vm, regs + 2, 11);
    U_NEWVEC(vm, regs + 2, 7, 1);
    keepvar[0] = TopM(regs + 2, 0);
    U_BCALLRET1(vm, regs + 2, 43, 1); // unicode_to_string
    block34731:;
    keepvar[1] = TopM(regs + 2, 0);
    U_SADD(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    DecVal(vm, keepvar[1]);
    PopFunId(vm);
}

// function266
static void fun_286(VMRef vm, StackPtr psp) {
    Value regs[9];
    Value keepvar[2];
    Value locals[2];
    locals[0] = lobster::NilVal();
    locals[1] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2523, locals);
    keepvar[0] = lobster::NilVal();
    keepvar[1] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 6, 12);
    U_STATEMENT(vm, regs + 0, 6, 12);
    U_STATEMENT(vm, regs + 0, 16, 12);
    U_STATEMENT(vm, regs + 0, 18, 12);
    U_STATEMENT(vm, regs + 0, 15, 12);
    U_STATEMENT(vm, regs + 0, 15, 12);
    U_STATEMENT(vm, regs + 0, 23, 12);
    U_STATEMENT(vm, regs + 0, 25, 12);
    U_STATEMENT(vm, regs + 0, 22, 12);
    U_STATEMENT(vm, regs + 0, 22, 12);
    U_STATEMENT(vm, regs + 0, 30, 12);
    U_STATEMENT(vm, regs + 0, 32, 12);
    U_STATEMENT(vm, regs + 0, 29, 12);
    U_STATEMENT(vm, regs + 0, 29, 12);
    U_STATEMENT(vm, regs + 0, 37, 12);
    U_STATEMENT(vm, regs + 0, 39, 12);
    U_STATEMENT(vm, regs + 0, 36, 12);
    U_STATEMENT(vm, regs + 0, 36, 12);
    U_STATEMENT(vm, regs + 0, 44, 12);
    U_STATEMENT(vm, regs + 0, 46, 12);
    U_STATEMENT(vm, regs + 0, 43, 12);
    U_STATEMENT(vm, regs + 0, 43, 12);
    U_STATEMENT(vm, regs + 0, 51, 12);
    U_STATEMENT(vm, regs + 0, 53, 12);
    U_STATEMENT(vm, regs + 0, 50, 12);
    U_STATEMENT(vm, regs + 0, 50, 12);
    U_STATEMENT(vm, regs + 0, 60, 12);
    U_NEWOBJECT(vm, regs + 0, 1029); // KillWolf
    U_NEWOBJECT(vm, regs + 1, 1035); // SellSkin
    U_NEWOBJECT(vm, regs + 2, 1041); // BuyPizza
    U_NEWOBJECT(vm, regs + 3, 1047); // BuyFlour
    U_NEWOBJECT(vm, regs + 4, 1053); // BakeBread
    U_NEWOBJECT(vm, regs + 5, 1059); // Eat
    U_NEWVEC(vm, regs + 6, 1019, 6);
    SetLVal(vm, &locals[0]); // goapactions
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 69, 12);
    U_STATEMENT(vm, regs + 0, 71, 12);
    regs[0] = locals[0]; // goapactions
    U_PUSHINT(vm, regs + 1, 3);
    U_PUSHINT(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 3, 0);
    U_PUSHINT(vm, regs + 4, 0);
    U_PUSHINT(vm, regs + 5, 0);
    U_PUSHINT(vm, regs + 6, 0);
    U_PUSHINT(vm, regs + 7, 2);
    U_NEWOBJECT(vm, regs + 8, 111); // resources
    U_PUSHFUN(vm, regs + 2, 0, fun_299);
    U_PUSHFUN(vm, regs + 3, 0, fun_300);
    fun_274(vm, regs + 4); // call: astar_goap
    SetLVal(vm, &locals[1]); // goap_path
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 77, 12);
    regs[0] = locals[1]; // goap_path
    U_BCALLRET1(vm, regs + 1, 13, 1); // length
    U_PUSHINT(vm, regs + 1, 9);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 77, 12, 477);
    U_STATEMENT(vm, regs + 0, 78, 12);
    regs[0] = locals[1]; // goap_path
    U_BCALLRETV(vm, regs + 1, 16, 1); // pop
    U_POPREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 79, 12);
    regs[0] = locals[1]; // goap_path
    U_PUSHFUN(vm, regs + 1, 0, fun_301);
    fun_559(vm, regs + 2); // call: map
    keepvar[0] = TopM(regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 478); // "Eat"
    U_INCREF(vm, regs + 2, 0);
    U_PUSHSTR(vm, regs + 2, 479); // "BuyPizza"
    U_INCREF(vm, regs + 3, 0);
    U_PUSHSTR(vm, regs + 3, 480); // "SellSkin"
    U_INCREF(vm, regs + 4, 0);
    U_PUSHSTR(vm, regs + 4, 481); // "KillWolf"
    U_INCREF(vm, regs + 5, 0);
    U_PUSHSTR(vm, regs + 5, 482); // "Eat"
    U_INCREF(vm, regs + 6, 0);
    U_PUSHSTR(vm, regs + 6, 483); // "BuyPizza"
    U_INCREF(vm, regs + 7, 0);
    U_PUSHSTR(vm, regs + 7, 484); // "SellSkin"
    U_INCREF(vm, regs + 8, 0);
    U_PUSHSTR(vm, regs + 8, 485); // "KillWolf"
    U_INCREF(vm, regs + 9, 0);
    U_NEWVEC(vm, regs + 9, 11, 8);
    keepvar[1] = TopM(regs + 2, 0);
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 79, 12, 486);
    U_RETURNLOCAL(vm, 0, 0);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    DecVal(vm, keepvar[1]);
    PopFunId(vm);
}

// precondition
static void fun_287(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2530, locals);
    U_STATEMENT(vm, regs + 0, 17, 12);
    regs[0] = locals[1]; // s
    U_PUSHFLD(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 1, 1);
    U_IGE(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// precondition
static void fun_289(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2537, locals);
    U_STATEMENT(vm, regs + 0, 24, 12);
    regs[0] = locals[1]; // s
    U_PUSHFLD(vm, regs + 1, 1);
    U_PUSHINT(vm, regs + 1, 1);
    U_IGE(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// precondition
static void fun_291(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2544, locals);
    U_STATEMENT(vm, regs + 0, 31, 12);
    regs[0] = locals[1]; // s
    U_PUSHFLD(vm, regs + 1, 2);
    U_PUSHINT(vm, regs + 1, 2);
    U_IGE(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// precondition
static void fun_293(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2551, locals);
    U_STATEMENT(vm, regs + 0, 38, 12);
    regs[0] = locals[1]; // s
    U_PUSHFLD(vm, regs + 1, 2);
    U_PUSHINT(vm, regs + 1, 1);
    U_IGE(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// precondition
static void fun_295(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2558, locals);
    U_STATEMENT(vm, regs + 0, 45, 12);
    regs[0] = locals[1]; // s
    U_PUSHFLD(vm, regs + 1, 3);
    U_PUSHINT(vm, regs + 1, 1);
    U_IGE(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// precondition
static void fun_297(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2565, locals);
    U_STATEMENT(vm, regs + 0, 52, 12);
    regs[0] = locals[1]; // s
    U_PUSHFLD(vm, regs + 1, 6);
    U_PUSHINT(vm, regs + 1, 0);
    U_IGT(vm, regs + 2);
    if (!U_JUMPFAILR(vm, regs + 1)) goto block35268;
    U_POP(vm, regs + 1);
    regs[0] = locals[1]; // s
    U_PUSHFLD(vm, regs + 1, 4);
    U_PUSHINT(vm, regs + 1, 0);
    U_IGT(vm, regs + 2);
    if (!U_JUMPNOFAILR(vm, regs + 1)) goto block35266;
    U_POP(vm, regs + 1);
    regs[0] = locals[1]; // s
    U_PUSHFLD(vm, regs + 1, 5);
    U_PUSHINT(vm, regs + 1, 0);
    U_IGT(vm, regs + 2);
    block35266:;
    block35268:;
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// effect
static void fun_288(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2572, locals);
    U_STATEMENT(vm, regs + 0, 19, 12);
    regs[0] = locals[1]; // s
    U_LVAL_FLD(vm, regs + 1, 0);
    U_LV_IMM(vm, regs + 0);
    U_STATEMENT(vm, regs + 0, 20, 12);
    regs[0] = locals[1]; // s
    U_LVAL_FLD(vm, regs + 1, 1);
    U_LV_IPP(vm, regs + 0);
    U_RETURNLOCAL(vm, 0, 0);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// effect
static void fun_290(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2579, locals);
    U_STATEMENT(vm, regs + 0, 26, 12);
    U_PUSHINT(vm, regs + 0, 2);
    regs[1] = locals[1]; // s
    U_LVAL_FLD(vm, regs + 2, 2);
    U_LV_IADD(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 27, 12);
    regs[0] = locals[1]; // s
    U_LVAL_FLD(vm, regs + 1, 1);
    U_LV_IMM(vm, regs + 0);
    U_RETURNLOCAL(vm, 0, 0);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// effect
static void fun_292(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2586, locals);
    U_STATEMENT(vm, regs + 0, 33, 12);
    U_PUSHINT(vm, regs + 0, 2);
    regs[1] = locals[1]; // s
    U_LVAL_FLD(vm, regs + 2, 2);
    U_LV_ISUB(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 34, 12);
    regs[0] = locals[1]; // s
    U_LVAL_FLD(vm, regs + 1, 5);
    U_LV_IPP(vm, regs + 0);
    U_RETURNLOCAL(vm, 0, 0);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// effect
static void fun_294(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2593, locals);
    U_STATEMENT(vm, regs + 0, 40, 12);
    regs[0] = locals[1]; // s
    U_LVAL_FLD(vm, regs + 1, 2);
    U_LV_IMM(vm, regs + 0);
    U_STATEMENT(vm, regs + 0, 41, 12);
    regs[0] = locals[1]; // s
    U_LVAL_FLD(vm, regs + 1, 3);
    U_LV_IPP(vm, regs + 0);
    U_RETURNLOCAL(vm, 0, 0);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// effect
static void fun_296(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2600, locals);
    U_STATEMENT(vm, regs + 0, 47, 12);
    regs[0] = locals[1]; // s
    U_LVAL_FLD(vm, regs + 1, 3);
    U_LV_IMM(vm, regs + 0);
    U_STATEMENT(vm, regs + 0, 48, 12);
    regs[0] = locals[1]; // s
    U_LVAL_FLD(vm, regs + 1, 4);
    U_LV_IPP(vm, regs + 0);
    U_RETURNLOCAL(vm, 0, 0);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// effect
static void fun_298(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2607, locals);
    U_STATEMENT(vm, regs + 0, 54, 12);
    regs[0] = locals[1]; // s
    U_PUSHFLD(vm, regs + 1, 4);
    U_PUSHINT(vm, regs + 1, 0);
    U_IGT(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block35519;
    U_STATEMENT(vm, regs + 0, 55, 12);
    regs[0] = locals[1]; // s
    U_LVAL_FLD(vm, regs + 1, 4);
    U_LV_IMM(vm, regs + 0);
    goto block35533;
    block35519:;
    U_STATEMENT(vm, regs + 0, 57, 12);
    regs[0] = locals[1]; // s
    U_LVAL_FLD(vm, regs + 1, 5);
    U_LV_IMM(vm, regs + 0);
    block35533:;
    U_STATEMENT(vm, regs + 0, 58, 12);
    regs[0] = locals[1]; // s
    U_LVAL_FLD(vm, regs + 1, 6);
    U_LV_IMM(vm, regs + 0);
    U_RETURNLOCAL(vm, 0, 0);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function269
static void fun_299(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2614, locals);
    U_STATEMENT(vm, regs + 0, 73, 12);
    regs[0] = locals[0]; // state
    U_PUSHFLD(vm, regs + 1, 6);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function270
static void fun_300(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2620, locals);
    U_STATEMENT(vm, regs + 0, 75, 12);
    regs[0] = locals[0]; // s
    U_PUSHFLD(vm, regs + 1, 6);
    U_PUSHINT(vm, regs + 1, 0);
    U_ILE(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function271
static void fun_301(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2626, locals);
    U_STATEMENT(vm, regs + 0, 79, 12);
    regs[0] = locals[0]; // n
    U_PUSHFLD(vm, regs + 1, 5);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block35638;
    U_STATEMENT(vm, regs + 0, 79, 12);
    regs[0] = locals[0]; // n
    U_PUSHFLD(vm, regs + 1, 5);
    U_BCALLRETV(vm, regs + 1, 155, 1); // type_string
    goto block35650;
    block35638:;
    U_STATEMENT(vm, regs + 0, 79, 12);
    U_PUSHSTR(vm, regs + 0, 487); // ""
    U_INCREF(vm, regs + 1, 0);
    block35650:;
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function272
static void fun_302(VMRef vm, StackPtr psp) {
    Value regs[16];
    Value keepvar[1];
    BackupVar(vm, 587);
    PushFunId(vm, funinfo_table + 2632, 0);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 6, 13);
    U_PUSHINT(vm, regs + 0, 1);
    U_PUSHINT(vm, regs + 1, 2);
    U_PUSHINT(vm, regs + 2, 1);
    U_PUSHINT(vm, regs + 3, -2);
    U_PUSHINT(vm, regs + 4, -1);
    U_PUSHINT(vm, regs + 5, 2);
    U_PUSHINT(vm, regs + 6, -1);
    U_PUSHINT(vm, regs + 7, -2);
    U_PUSHINT(vm, regs + 8, 2);
    U_PUSHINT(vm, regs + 9, 1);
    U_PUSHINT(vm, regs + 10, 2);
    U_PUSHINT(vm, regs + 11, -1);
    U_PUSHINT(vm, regs + 12, -2);
    U_PUSHINT(vm, regs + 13, 1);
    U_PUSHINT(vm, regs + 14, -2);
    U_PUSHINT(vm, regs + 15, -1);
    U_NEWVEC(vm, regs + 16, 37, 8);
    U_LVAL_VARF(vm, regs + 1, 587); // knight_moves
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 9, 13);
    U_STATEMENT(vm, regs + 0, 23, 13);
    U_PUSHINT(vm, regs + 0, 0);
    U_BCALLRET1(vm, regs + 1, 97, 0); // rnd_seed
    U_STATEMENT(vm, regs + 0, 24, 13);
    U_PUSHINT(vm, regs + 0, 8);
    U_PUSHINT(vm, regs + 1, 8);
    fun_303(vm, regs + 2); // call: knights_tour
    keepvar[0] = TopM(regs + 1, 0);
    U_ASSERT(vm, regs + 1, 24, 13, 488);
    U_RETURNLOCAL(vm, 0, 0);
    DecOwned(vm, 587);
    goto epilogue;
    epilogue:;
    RestoreBackup(vm, 587);
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// knights_tour
static void fun_303(VMRef vm, StackPtr psp) {
    Value regs[6];
    Value locals[3];
    SwapVars(vm, 588, psp, 2);
    SwapVars(vm, 589, psp, 1);
    BackupVar(vm, 590);
    BackupVar(vm, 591);
    BackupVar(vm, 592);
    locals[0] = lobster::NilVal();
    locals[1] = lobster::NilVal();
    locals[2] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2638, locals);
    U_STATEMENT(vm, regs + 0, 10, 13);
    U_PUSHVARVF(vm, regs + 0, 588, 2); // dim+0
    U_PUSHFUN(vm, regs + 2, 0, fun_10000001);
    fun_100(vm, regs + 3); // call: mapxy
    U_LVAL_VARF(vm, regs + 1, 590); // board
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 11, 13);
    U_PUSHVARVF(vm, regs + 0, 588, 2); // dim+0
    fun_95(vm, regs + 2); // call: rnd_int2
    U_LVAL_VARF(vm, regs + 2, 591); // cur+0
    U_LV_WRITEV(vm, regs + 2, 2);
    U_STATEMENT(vm, regs + 0, 12, 13);
    U_PUSHINT(vm, regs + 0, 0);
    U_PUSHVARF(vm, regs + 1, 590); // board
    U_PUSHVARVF(vm, regs + 2, 591, 2); // cur+0
    U_LVAL_IDXVV(vm, regs + 4, 0, 2);
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 13, 13);
    U_PUSHINT(vm, regs + 0, -1);
    U_PUSHVARF(vm, regs + 1, 588); // dim+0
    U_PUSHVARF(vm, regs + 2, 589); // dim+1
    U_IMUL(vm, regs + 3);
    U_PUSHINT(vm, regs + 2, 1);
    U_ISUB(vm, regs + 3);
    block35861:;
    if (!U_IFOR(vm, regs + 2)) goto block36027;
    U_STATEMENT(vm, regs + 2, 13, 13);
    U_IFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[0]); // step
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 14, 13);
    U_PUSHVARF(vm, regs + 2, 587); // knight_moves
    U_PUSHFUN(vm, regs + 3, 0, fun_305);
    fun_562(vm, regs + 4); // call: map
    SetLVal(vm, &locals[1]); // candidates
    U_LV_WRITEREF(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 15, 13);
    U_STATEMENT(vm, regs + 2, 16, 13);
    regs[2] = locals[1]; // candidates
    U_INCREF(vm, regs + 3, 0);
    U_PUSHFUN(vm, regs + 3, 0, fun_307);
    fun_10(vm, regs + 4); // call: filter
    SetLVal(vm, &locals[1]); // candidates
    U_LV_WRITEREF(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 17, 13);
    regs[2] = locals[1]; // candidates
    U_BCALLRET1(vm, regs + 3, 13, 1); // length
    U_LOGNOT(vm, regs + 3);
    if (!U_JUMPFAIL(vm, regs + 3)) goto block35949;
    U_STATEMENT(vm, regs + 2, 17, 13);
    U_POP(vm, regs + 2);
    U_POP(vm, regs + 1);
    U_PUSHNIL(vm, regs + 0);
    U_RETURNLOCAL(vm, 0, 1);
    DecOwned(vm, 590);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[2]);
    psp = PopArg(vm, 589, psp);
    psp = PopArg(vm, 588, psp);
    Push(psp, regs[0]);
    goto epilogue;
    block35949:;
    U_STATEMENT(vm, regs + 2, 18, 13);
    regs[2] = locals[1]; // candidates
    U_INCREF(vm, regs + 3, 0);
    U_PUSHFUN(vm, regs + 3, 0, fun_308);
    fun_563(vm, regs + 4); // call: map
    SetLVal(vm, &locals[2]); // degrees
    U_LV_WRITEREF(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 19, 13);
    regs[2] = locals[1]; // candidates
    regs[3] = locals[2]; // degrees
    U_INCREF(vm, regs + 4, 0);
    U_PUSHFUN(vm, regs + 4, 0, fun_310);
    fun_26(vm, regs + 5); // call: find_best
    U_VPUSHIDXI2V(vm, regs + 4);
    U_LVAL_VARF(vm, regs + 4, 591); // cur+0
    U_LV_WRITEV(vm, regs + 4, 2);
    U_STATEMENT(vm, regs + 2, 20, 13);
    regs[2] = locals[0]; // step
    U_PUSHINT(vm, regs + 3, 1);
    U_IADD(vm, regs + 4);
    U_PUSHVARF(vm, regs + 3, 590); // board
    U_PUSHVARVF(vm, regs + 4, 591, 2); // cur+0
    U_LVAL_IDXVV(vm, regs + 6, 0, 2);
    U_LV_WRITE(vm, regs + 3);
    goto block35861;
    block36027:;
    U_STATEMENT(vm, regs + 0, 21, 13);
    U_PUSHVARF(vm, regs + 0, 590); // board
    U_INCREF(vm, regs + 1, 0);
    U_RETURNLOCAL(vm, 0, 1);
    DecOwned(vm, 590);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[2]);
    psp = PopArg(vm, 589, psp);
    psp = PopArg(vm, 588, psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    RestoreBackup(vm, 592);
    RestoreBackup(vm, 591);
    RestoreBackup(vm, 590);
    PopFunId(vm);
}

// function275
static void fun_305(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2651, locals);
    U_STATEMENT(vm, regs + 0, 14, 13);
    U_PUSHVARVF(vm, regs + 0, 591, 2); // cur+0
    regs[2] = locals[0];regs[3] = locals[1]; // m
    U_IVVADD(vm, regs + 4, 2);
    U_RETURNLOCAL(vm, 0, 2);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// empty
static void fun_306(VMRef vm, StackPtr psp) {
    Value regs[9];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2658, locals);
    U_STATEMENT(vm, regs + 0, 15, 13);
    regs[0] = locals[0];regs[1] = locals[1]; // v
    U_PUSHINT(vm, regs + 2, 2);
    U_PUSHVARVF(vm, regs + 3, 588, 2); // dim+0
    U_PUSHINT(vm, regs + 5, 2);
    U_PUSHINT(vm, regs + 6, 0);
    U_PUSHINT(vm, regs + 7, 0);
    U_PUSHINT(vm, regs + 8, 2);
    U_BCALLRETV(vm, regs + 9, 107, 1); // in_range
    if (!U_JUMPFAILR(vm, regs + 1)) goto block36131;
    U_POP(vm, regs + 1);
    U_PUSHVARF(vm, regs + 0, 590); // board
    regs[1] = locals[0];regs[2] = locals[1]; // v
    U_VPUSHIDXV(vm, regs + 3, 2);
    U_PUSHINT(vm, regs + 1, 0);
    U_ILT(vm, regs + 2);
    block36131:;
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function277
static void fun_307(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2665, locals);
    U_STATEMENT(vm, regs + 0, 16, 13);
    regs[0] = locals[0];regs[1] = locals[1]; // c
    fun_306(vm, regs + 2); // call: empty
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function278
static void fun_308(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value keepvar[1];
    SwapVars(vm, 602, psp, 2);
    SwapVars(vm, 603, psp, 1);
    PushFunId(vm, funinfo_table + 2672, 0);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 18, 13);
    U_PUSHVARF(vm, regs + 0, 587); // knight_moves
    U_PUSHFUN(vm, regs + 1, 0, fun_309);
    fun_564(vm, regs + 2); // call: filter
    keepvar[0] = TopM(regs + 1, 0);
    U_BCALLRET1(vm, regs + 1, 13, 1); // length
    U_RETURNLOCAL(vm, 0, 1);
    psp = PopArg(vm, 603, psp);
    psp = PopArg(vm, 602, psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// function279
static void fun_309(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2679, locals);
    U_STATEMENT(vm, regs + 0, 18, 13);
    U_PUSHVARVF(vm, regs + 0, 602, 2); // c+0
    regs[2] = locals[0];regs[3] = locals[1]; // m
    U_IVVADD(vm, regs + 4, 2);
    fun_306(vm, regs + 2); // call: empty
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function280
static void fun_310(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2686, locals);
    U_STATEMENT(vm, regs + 0, 19, 13);
    regs[0] = locals[0]; // d
    U_IUMINUS(vm, regs + 1);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function281
static void fun_311(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value keepvar[1];
    Value locals[4];
    locals[0] = lobster::NilVal();
    locals[1] = lobster::NilVal();
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2692, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 8, 14);
    U_STATEMENT(vm, regs + 0, 8, 14);
    U_STATEMENT(vm, regs + 0, 13, 14);
    U_STATEMENT(vm, regs + 0, 14, 14);
    U_STATEMENT(vm, regs + 0, 15, 14);
    U_STATEMENT(vm, regs + 0, 10, 14);
    U_STATEMENT(vm, regs + 0, 10, 14);
    U_STATEMENT(vm, regs + 0, 21, 14);
    U_STATEMENT(vm, regs + 0, 25, 14);
    U_STATEMENT(vm, regs + 0, 34, 14);
    U_STATEMENT(vm, regs + 0, 17, 14);
    U_STATEMENT(vm, regs + 0, 17, 14);
    U_STATEMENT(vm, regs + 0, 41, 14);
    U_STATEMENT(vm, regs + 0, 116, 14);
    U_PUSHSTR(vm, regs + 0, 489); // "(+\n(* 2 3 4)  // Comment.\n10)\n"
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[0]); // test_code
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 122, 14);
    regs[0] = locals[0]; // test_code
    fun_323(vm, regs + 1); // call: parse
    SetLVal(vm, &locals[2]); // err
    U_LV_WRITEREF(vm, regs + 2);
    SetLVal(vm, &locals[1]); // ast
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 123, 14);
    regs[0] = locals[1]; // ast
    U_E2B(vm, regs + 1);
    if (!U_JUMPFAILR(vm, regs + 1)) goto block36371;
    U_POP(vm, regs + 1);
    regs[0] = locals[2]; // err
    U_PUSHSTR(vm, regs + 1, 490); // ""
    U_SEQ(vm, regs + 2);
    block36371:;
    U_ASSERT(vm, regs + 1, 123, 14, 491);
    U_STATEMENT(vm, regs + 0, 124, 14);
    regs[0] = locals[1]; // ast
    U_INCREF(vm, regs + 1, 0);
    U_DDCALL(vm, regs + 1, 0, 0); vm.next_call_target(vm, regs + 1);
    keepvar[0] = TopM(regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 492); // "(+ (* 2 3 4) 10)"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 124, 14, 493);
    U_STATEMENT(vm, regs + 0, 125, 14);
    regs[0] = locals[1]; // ast
    U_INCREF(vm, regs + 1, 0);
    U_DDCALL(vm, regs + 1, 2, 0); vm.next_call_target(vm, regs + 1);
    U_PUSHINT(vm, regs + 1, 34);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 125, 14, 494);
    U_STATEMENT(vm, regs + 0, 126, 14);
    regs[0] = locals[1]; // ast
    U_INCREF(vm, regs + 1, 0);
    U_DDCALL(vm, regs + 1, 5, 0); vm.next_call_target(vm, regs + 1);
    SetLVal(vm, &locals[3]); // code
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 127, 14);
    regs[0] = locals[3]; // code
    U_PUSHSTR(vm, regs + 1, 495); // "(((2*3)*4)+10)"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 127, 14, 496);
    U_RETURNLOCAL(vm, 0, 0);
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
static void fun_312(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2701, locals);
    U_STATEMENT(vm, regs + 0, 13, 14);
    regs[0] = locals[0]; // this
    U_PUSHFLD(vm, regs + 1, 0);
    U_A2S(vm, regs + 1, 0);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// pretty
static void fun_315(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    locals[0] = *(psp - 1);
    locals[1] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2707, locals);
    U_STATEMENT(vm, regs + 0, 22, 14);
    regs[0] = locals[0]; // this
    U_PUSHFLD(vm, regs + 1, 1);
    regs[1] = locals[0]; // this
    U_PUSHFLD(vm, regs + 2, 0);
    U_INCREF(vm, regs + 2, 0);
    U_PUSHFUN(vm, regs + 2, 0, fun_316);
    fun_567(vm, regs + 3); // call: fold
    SetLVal(vm, &locals[1]); // elems
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 23, 14);
    U_PUSHSTR(vm, regs + 0, 497); // "("
    regs[1] = locals[1]; // elems
    U_PUSHSTR(vm, regs + 2, 498); // ")"
    U_SADDN(vm, regs + 3, 3);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// eval
static void fun_313(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2714, locals);
    U_STATEMENT(vm, regs + 0, 14, 14);
    regs[0] = locals[0]; // this
    U_PUSHFLD(vm, regs + 1, 0);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// eval
static void fun_317(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2720, locals);
    U_STATEMENT(vm, regs + 0, 32, 14);
    regs[0] = locals[0]; // this
    U_PUSHFLD(vm, regs + 1, 0);
    U_DUP(vm, regs + 1);
    U_PUSHSTR(vm, regs + 2, 499); // "+"
    U_SEQ(vm, regs + 3);
    if (!U_JUMPFAIL(vm, regs + 2)) goto block36633;
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 28, 14);
    regs[0] = locals[0]; // this
    U_PUSHFLD(vm, regs + 1, 1);
    U_PUSHINT(vm, regs + 1, 0);
    U_PUSHFUN(vm, regs + 2, 0, fun_318);
    fun_568(vm, regs + 3); // call: fold
    goto block36685;
    block36633:;
    U_DUP(vm, regs + 1);
    U_PUSHSTR(vm, regs + 2, 500); // "*"
    U_SEQ(vm, regs + 3);
    if (!U_JUMPFAIL(vm, regs + 2)) goto block36669;
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 30, 14);
    regs[0] = locals[0]; // this
    U_PUSHFLD(vm, regs + 1, 1);
    U_PUSHINT(vm, regs + 1, 1);
    U_PUSHFUN(vm, regs + 2, 0, fun_319);
    fun_569(vm, regs + 3); // call: fold
    goto block36685;
    block36669:;
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 32, 14);
    U_PUSHINT(vm, regs + 0, 0);
    U_ASSERTR(vm, regs + 1, 32, 14, 501);
    block36685:;
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// compile
static void fun_314(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2726, locals);
    U_STATEMENT(vm, regs + 0, 15, 14);
    regs[0] = locals[0]; // this
    U_PUSHFLD(vm, regs + 1, 0);
    U_A2S(vm, regs + 1, 0);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// compile
static void fun_320(VMRef vm, StackPtr psp) {
    Value regs[3];
    SwapVars(vm, 619, psp, 1);
    PushFunId(vm, funinfo_table + 2732, 0);
    U_STATEMENT(vm, regs + 0, 39, 14);
    U_PUSHVARF(vm, regs + 0, 619); // this
    U_PUSHFLD(vm, regs + 1, 0);
    U_DUP(vm, regs + 1);
    U_PUSHSTR(vm, regs + 2, 502); // "+"
    U_SEQ(vm, regs + 3);
    if (!U_JUMPNOFAIL(vm, regs + 2)) goto block36756;
    U_DUP(vm, regs + 1);
    U_PUSHSTR(vm, regs + 2, 503); // "*"
    U_SEQ(vm, regs + 3);
    if (!U_JUMPFAIL(vm, regs + 2)) goto block36785;
    block36756:;
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 37, 14);
    U_PUSHVARF(vm, regs + 0, 619); // this
    U_PUSHFLD(vm, regs + 1, 1);
    U_PUSHFUN(vm, regs + 1, 0, fun_321);
    fun_570(vm, regs + 2); // call: map
    U_PUSHFUN(vm, regs + 1, 0, fun_322);
    fun_19(vm, regs + 2); // call: reduce
    goto block36801;
    block36785:;
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 39, 14);
    U_PUSHINT(vm, regs + 0, 0);
    U_ASSERTR(vm, regs + 1, 39, 14, 504);
    block36801:;
    U_RETURNLOCAL(vm, 0, 1);
    DecOwned(vm, 619);
    psp = PopArg(vm, 619, psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function285
static void fun_316(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value keepvar[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2738, locals);
    keepvar[0] = lobster::NilVal();
    keepvar[1] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 22, 14);
    regs[0] = locals[0]; // _a
    U_INCREF(vm, regs + 1, 0);
    keepvar[0] = TopM(regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 505); // " "
    regs[2] = locals[1]; // _e
    U_INCREF(vm, regs + 3, 0);
    U_DDCALL(vm, regs + 3, 1, 0); vm.next_call_target(vm, regs + 3);
    keepvar[1] = TopM(regs + 3, 0);
    U_SADDN(vm, regs + 3, 3);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    DecVal(vm, keepvar[1]);
    PopFunId(vm);
}

// function286
static void fun_318(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2745, locals);
    U_STATEMENT(vm, regs + 0, 28, 14);
    regs[0] = locals[0]; // _a
    regs[1] = locals[1]; // _e
    U_INCREF(vm, regs + 2, 0);
    U_DDCALL(vm, regs + 2, 3, 0); vm.next_call_target(vm, regs + 2);
    U_IADD(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function287
static void fun_319(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2752, locals);
    U_STATEMENT(vm, regs + 0, 30, 14);
    regs[0] = locals[0]; // _a
    regs[1] = locals[1]; // _e
    U_INCREF(vm, regs + 2, 0);
    U_DDCALL(vm, regs + 2, 4, 0); vm.next_call_target(vm, regs + 2);
    U_IMUL(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function288
static void fun_321(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2759, locals);
    U_STATEMENT(vm, regs + 0, 37, 14);
    regs[0] = locals[0]; // _
    U_INCREF(vm, regs + 1, 0);
    U_DDCALL(vm, regs + 1, 6, 0); vm.next_call_target(vm, regs + 1);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function289
static void fun_322(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2765, locals);
    U_STATEMENT(vm, regs + 0, 37, 14);
    U_PUSHSTR(vm, regs + 0, 506); // "("
    regs[1] = locals[0]; // _a
    U_PUSHVARF(vm, regs + 2, 619); // this
    U_PUSHFLD(vm, regs + 3, 0);
    regs[3] = locals[1]; // _e
    U_PUSHSTR(vm, regs + 4, 507); // ")"
    U_SADDN(vm, regs + 5, 5);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// parse
static void fun_323(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    SwapVars(vm, 623, psp, 1);
    BackupVar(vm, 624);
    BackupVar(vm, 625);
    BackupVar(vm, 626);
    BackupVar(vm, 627);
    BackupVar(vm, 628);
    locals[0] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2772, locals);
    U_STATEMENT(vm, regs + 0, 42, 14);
    U_PUSHSTR(vm, regs + 0, 508); // ""
    U_INCREF(vm, regs + 1, 0);
    U_LVAL_VARF(vm, regs + 1, 624); // token
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 43, 14);
    U_PUSHSTR(vm, regs + 0, 509); // ""
    U_INCREF(vm, regs + 1, 0);
    U_LVAL_VARF(vm, regs + 1, 625); // atom
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 44, 14);
    U_PUSHINT(vm, regs + 0, 0);
    U_LVAL_VARF(vm, regs + 1, 626); // i
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 45, 14);
    U_PUSHINT(vm, regs + 0, 1);
    U_LVAL_VARF(vm, regs + 1, 627); // line
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 46, 14);
    U_PUSHINT(vm, regs + 0, 0);
    U_LVAL_VARF(vm, regs + 1, 628); // ival
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 47, 14);
    U_STATEMENT(vm, regs + 0, 51, 14);
    U_STATEMENT(vm, regs + 0, 61, 14);
    U_STATEMENT(vm, regs + 0, 87, 14);
    fun_327(vm, regs + 0); // call: lex_next
    if (!U_JUMPIFUNWOUND(vm, regs + 2, 294)) goto block37093;
    U_RETURNANY(vm, 0, 0);
    DecOwned(vm, 624);
    DecOwned(vm, 625);
    DecVal(vm, locals[0]);
    psp = PopArg(vm, 623, psp);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block37093:;
    U_STATEMENT(vm, regs + 0, 88, 14);
    U_STATEMENT(vm, regs + 0, 94, 14);
    U_STATEMENT(vm, regs + 0, 112, 14);
    fun_329(vm, regs + 0); // call: parse_exp
    if (!U_JUMPIFUNWOUND(vm, regs + 2, 296)) goto block37119;
    U_RETURNANY(vm, 0, 1);
    DecOwned(vm, 624);
    DecOwned(vm, 625);
    DecVal(vm, locals[0]);
    psp = PopArg(vm, 623, psp);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block37119:;
    SetLVal(vm, &locals[0]); // root
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 113, 14);
    U_PUSHSTR(vm, regs + 0, 510); // "eof"
    fun_566(vm, regs + 1); // call: expect
    if (!U_JUMPIFUNWOUND(vm, regs + 2, 295)) goto block37145;
    U_RETURNANY(vm, 0, 0);
    DecOwned(vm, 624);
    DecOwned(vm, 625);
    DecVal(vm, locals[0]);
    psp = PopArg(vm, 623, psp);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block37145:;
    U_STATEMENT(vm, regs + 0, 114, 14);
    regs[0] = locals[0]; // root
    U_PUSHSTR(vm, regs + 1, 511); // ""
    U_INCREF(vm, regs + 2, 0);
    U_INCREF(vm, regs + 2, 1);
    U_RETURNLOCAL(vm, 0, 2);
    DecOwned(vm, 624);
    DecOwned(vm, 625);
    DecVal(vm, locals[0]);
    psp = PopArg(vm, 623, psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
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
static void fun_324(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2784, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 50, 14);
    U_PUSHNIL(vm, regs + 0);
    U_PUSHSTR(vm, regs + 1, 512); // "error: line "
    U_PUSHVARF(vm, regs + 2, 627); // line
    U_A2S(vm, regs + 3, 0);
    keepvar[0] = TopM(regs + 3, 0);
    U_PUSHSTR(vm, regs + 3, 513); // ": "
    regs[4] = locals[0]; // err
    U_SADDN(vm, regs + 5, 4);
    U_RETURNNONLOCAL(vm, 0, 2, 290);
    DecVal(vm, locals[0]);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// lex_atom
static void fun_325(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value keepvar[1];
    Value locals[2];
    locals[0] = lobster::NilVal();
    locals[1] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2790, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 52, 14);
    U_LVAL_VARF(vm, regs + 0, 626); // i
    U_LV_IMM(vm, regs + 0);
    U_STATEMENT(vm, regs + 0, 53, 14);
    U_PUSHVARF(vm, regs + 0, 626); // i
    SetLVal(vm, &locals[0]); // start
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 54, 14);
    block37243:;
    U_PUSHVARF(vm, regs + 0, 623); // s
    U_PUSHVARF(vm, regs + 1, 626); // i
    U_SPUSHIDXI(vm, regs + 2);
    U_PUSHINT(vm, regs + 1, 32);
    U_IGT(vm, regs + 2);
    if (!U_JUMPFAILR(vm, regs + 1)) goto block37274;
    U_POP(vm, regs + 1);
    U_PUSHSTR(vm, regs + 0, 514); // "()\x7F"
    U_PUSHFUN(vm, regs + 1, 0, fun_326);
    fun_15(vm, regs + 2); // call: exists
    U_LOGNOT(vm, regs + 1);
    block37274:;
    if (!U_JUMPFAIL(vm, regs + 1)) goto block37291;
    U_STATEMENT(vm, regs + 0, 54, 14);
    U_LVAL_VARF(vm, regs + 0, 626); // i
    U_LV_IPP(vm, regs + 0);
    goto block37243;
    block37291:;
    U_STATEMENT(vm, regs + 0, 55, 14);
    regs[0] = locals[0]; // start
    U_PUSHVARF(vm, regs + 1, 626); // i
    U_IEQ(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block37346;
    U_STATEMENT(vm, regs + 0, 56, 14);
    U_PUSHSTR(vm, regs + 0, 515); // "unprintable character: "
    U_PUSHVARF(vm, regs + 1, 623); // s
    U_PUSHVARF(vm, regs + 2, 626); // i
    U_SPUSHIDXI(vm, regs + 3);
    U_A2S(vm, regs + 2, 0);
    keepvar[0] = TopM(regs + 2, 0);
    U_SADD(vm, regs + 2);
    fun_324(vm, regs + 1); // call: error
    if (!U_JUMPIFUNWOUND(vm, regs + 2, 291)) goto block37344;
    U_RETURNANY(vm, 0, 0);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block37344:;
    block37346:;
    U_STATEMENT(vm, regs + 0, 57, 14);
    U_PUSHVARF(vm, regs + 0, 623); // s
    regs[1] = locals[0]; // start
    U_PUSHVARF(vm, regs + 2, 626); // i
    regs[3] = locals[0]; // start
    U_ISUB(vm, regs + 4);
    U_BCALLRET3(vm, regs + 3, 36, 1); // substring
    U_LVAL_VARF(vm, regs + 1, 625); // atom
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 58, 14);
    U_PUSHINT(vm, regs + 0, 0);
    SetLVal(vm, &locals[1]); // all
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 59, 14);
    U_PUSHVARF(vm, regs + 0, 625); // atom
    U_PUSHINT(vm, regs + 1, 0);
    U_BCALLRET2(vm, regs + 2, 40, 1); // string_to_int
    SetLVal(vm, &locals[1]); // all
    U_LV_WRITE(vm, regs + 2);
    U_LVAL_VARF(vm, regs + 1, 628); // ival
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 60, 14);
    regs[0] = locals[1]; // all
    if (!U_JUMPFAIL(vm, regs + 1)) goto block37431;
    U_STATEMENT(vm, regs + 0, 60, 14);
    U_PUSHSTR(vm, regs + 0, 516); // "int"
    goto block37440;
    block37431:;
    U_STATEMENT(vm, regs + 0, 60, 14);
    U_PUSHSTR(vm, regs + 0, 517); // "atom"
    block37440:;
    U_INCREF(vm, regs + 1, 0);
    U_LVAL_VARF(vm, regs + 1, 624); // token
    U_LV_WRITEREF(vm, regs + 1);
    U_RETURNNONLOCAL(vm, 0, 0, 292);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// function293
static void fun_326(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2797, locals);
    U_STATEMENT(vm, regs + 0, 54, 14);
    regs[0] = locals[0]; // _
    U_PUSHVARF(vm, regs + 1, 623); // s
    U_PUSHVARF(vm, regs + 2, 626); // i
    U_SPUSHIDXI(vm, regs + 3);
    U_IEQ(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// lex_next
static void fun_327(VMRef vm, StackPtr psp) {
    Value regs[3];
    PushFunId(vm, funinfo_table + 2803, 0);
    U_STATEMENT(vm, regs + 0, 62, 14);
    block37495:;
    U_PUSHINT(vm, regs + 0, 1);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block37871;
    U_STATEMENT(vm, regs + 0, 63, 14);
    U_PUSHVARF(vm, regs + 0, 623); // s
    U_LVAL_VARF(vm, regs + 1, 626); // i
    U_LV_DUP(vm, regs + 1);
    U_LV_IPP(vm, regs + 2);
    U_SPUSHIDXI(vm, regs + 2);
    U_DUP(vm, regs + 1);
    U_PUSHINT(vm, regs + 2, 0);
    U_IEQ(vm, regs + 3);
    if (!U_JUMPFAIL(vm, regs + 2)) goto block37566;
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 65, 14);
    U_LVAL_VARF(vm, regs + 0, 626); // i
    U_LV_IMM(vm, regs + 0);
    U_STATEMENT(vm, regs + 0, 66, 14);
    U_PUSHSTR(vm, regs + 0, 518); // "eof"
    U_INCREF(vm, regs + 1, 0);
    U_LVAL_VARF(vm, regs + 1, 624); // token
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 67, 14);
    U_RETURNNONLOCAL(vm, 0, 0, 294);
    goto epilogue;
    goto block37866;
    block37566:;
    U_DUP(vm, regs + 1);
    U_PUSHINT(vm, regs + 2, 10);
    U_IEQ(vm, regs + 3);
    if (!U_JUMPFAIL(vm, regs + 2)) goto block37592;
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 69, 14);
    U_LVAL_VARF(vm, regs + 0, 627); // line
    U_LV_IPP(vm, regs + 0);
    goto block37866;
    block37592:;
    U_DUP(vm, regs + 1);
    U_PUSHINT(vm, regs + 2, 13);
    U_IEQ(vm, regs + 3);
    if (!U_JUMPNOFAIL(vm, regs + 2)) goto block37624;
    U_DUP(vm, regs + 1);
    U_PUSHINT(vm, regs + 2, 9);
    U_IEQ(vm, regs + 3);
    if (!U_JUMPNOFAIL(vm, regs + 2)) goto block37624;
    U_DUP(vm, regs + 1);
    U_PUSHINT(vm, regs + 2, 32);
    U_IEQ(vm, regs + 3);
    if (!U_JUMPFAIL(vm, regs + 2)) goto block37635;
    block37624:;
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 71, 14);
    goto block37866;
    block37635:;
    U_DUP(vm, regs + 1);
    U_PUSHINT(vm, regs + 2, 40);
    U_IEQ(vm, regs + 3);
    if (!U_JUMPFAIL(vm, regs + 2)) goto block37675;
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 73, 14);
    U_PUSHSTR(vm, regs + 0, 519); // "("
    U_INCREF(vm, regs + 1, 0);
    U_LVAL_VARF(vm, regs + 1, 624); // token
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 74, 14);
    U_RETURNNONLOCAL(vm, 0, 0, 294);
    goto epilogue;
    goto block37866;
    block37675:;
    U_DUP(vm, regs + 1);
    U_PUSHINT(vm, regs + 2, 41);
    U_IEQ(vm, regs + 3);
    if (!U_JUMPFAIL(vm, regs + 2)) goto block37715;
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 76, 14);
    U_PUSHSTR(vm, regs + 0, 520); // ")"
    U_INCREF(vm, regs + 1, 0);
    U_LVAL_VARF(vm, regs + 1, 624); // token
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 77, 14);
    U_RETURNNONLOCAL(vm, 0, 0, 294);
    goto epilogue;
    goto block37866;
    block37715:;
    U_DUP(vm, regs + 1);
    U_PUSHINT(vm, regs + 2, 47);
    U_IEQ(vm, regs + 3);
    if (!U_JUMPFAIL(vm, regs + 2)) goto block37836;
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 79, 14);
    U_PUSHVARF(vm, regs + 0, 623); // s
    U_PUSHVARF(vm, regs + 1, 626); // i
    U_SPUSHIDXI(vm, regs + 2);
    U_PUSHINT(vm, regs + 1, 47);
    U_IEQ(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block37803;
    U_STATEMENT(vm, regs + 0, 80, 14);
    block37753:;
    U_PUSHVARF(vm, regs + 0, 623); // s
    U_PUSHVARF(vm, regs + 1, 626); // i
    U_SPUSHIDXI(vm, regs + 2);
    if (!U_JUMPFAILR(vm, regs + 1)) goto block37781;
    U_POP(vm, regs + 1);
    U_PUSHVARF(vm, regs + 0, 623); // s
    U_PUSHVARF(vm, regs + 1, 626); // i
    U_SPUSHIDXI(vm, regs + 2);
    U_PUSHINT(vm, regs + 1, 10);
    U_INE(vm, regs + 2);
    block37781:;
    if (!U_JUMPFAIL(vm, regs + 1)) goto block37798;
    U_STATEMENT(vm, regs + 0, 80, 14);
    U_LVAL_VARF(vm, regs + 0, 626); // i
    U_LV_IPP(vm, regs + 0);
    goto block37753;
    block37798:;
    goto block37831;
    block37803:;
    U_STATEMENT(vm, regs + 0, 82, 14);
    fun_325(vm, regs + 0); // call: lex_atom
    if (!U_JUMPIFUNWOUND(vm, regs + 2, 292)) goto block37821;
    U_RETURNANY(vm, 0, 0);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block37821:;
    U_STATEMENT(vm, regs + 0, 83, 14);
    U_RETURNNONLOCAL(vm, 0, 0, 294);
    goto epilogue;
    block37831:;
    goto block37866;
    block37836:;
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 85, 14);
    fun_325(vm, regs + 0); // call: lex_atom
    if (!U_JUMPIFUNWOUND(vm, regs + 2, 292)) goto block37856;
    U_RETURNANY(vm, 0, 0);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block37856:;
    U_STATEMENT(vm, regs + 0, 86, 14);
    U_RETURNNONLOCAL(vm, 0, 0, 294);
    goto epilogue;
    block37866:;
    goto block37495;
    block37871:;
    U_RETURNNONLOCAL(vm, 0, 0, 294);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// expect
static void fun_566(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[2];
    locals[0] = *(psp - 1);
    locals[1] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2808, locals);
    U_STATEMENT(vm, regs + 0, 89, 14);
    regs[0] = locals[0]; // tok
    U_PUSHVARF(vm, regs + 1, 624); // token
    U_SNE(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block37936;
    U_STATEMENT(vm, regs + 0, 90, 14);
    U_PUSHSTR(vm, regs + 0, 521); // "expected: "
    regs[1] = locals[0]; // tok
    U_PUSHSTR(vm, regs + 2, 522); // ", found: "
    U_PUSHVARF(vm, regs + 3, 624); // token
    U_SADDN(vm, regs + 4, 4);
    fun_324(vm, regs + 1); // call: error
    if (!U_JUMPIFUNWOUND(vm, regs + 2, 291)) goto block37934;
    U_RETURNANY(vm, 0, 0);
    DecVal(vm, locals[1]);
    Pop(psp);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block37934:;
    block37936:;
    U_STATEMENT(vm, regs + 0, 91, 14);
    U_PUSHVARF(vm, regs + 0, 625); // atom
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[1]); // a
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 92, 14);
    fun_327(vm, regs + 0); // call: lex_next
    if (!U_JUMPIFUNWOUND(vm, regs + 2, 294)) goto block37969;
    U_RETURNANY(vm, 0, 0);
    DecVal(vm, locals[1]);
    Pop(psp);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block37969:;
    U_STATEMENT(vm, regs + 0, 93, 14);
    U_RETURNNONLOCAL(vm, 0, 0, 295);
    DecVal(vm, locals[1]);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// expect
static void fun_328(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[2];
    locals[0] = *(psp - 1);
    locals[1] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2815, locals);
    U_STATEMENT(vm, regs + 0, 89, 14);
    regs[0] = locals[0]; // tok
    U_PUSHVARF(vm, regs + 1, 624); // token
    U_SNE(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block38038;
    U_STATEMENT(vm, regs + 0, 90, 14);
    U_PUSHSTR(vm, regs + 0, 523); // "expected: "
    regs[1] = locals[0]; // tok
    U_PUSHSTR(vm, regs + 2, 524); // ", found: "
    U_PUSHVARF(vm, regs + 3, 624); // token
    U_SADDN(vm, regs + 4, 4);
    fun_324(vm, regs + 1); // call: error
    if (!U_JUMPIFUNWOUND(vm, regs + 2, 291)) goto block38036;
    U_RETURNANY(vm, 0, 0);
    DecVal(vm, locals[1]);
    Pop(psp);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block38036:;
    block38038:;
    U_STATEMENT(vm, regs + 0, 91, 14);
    U_PUSHVARF(vm, regs + 0, 625); // atom
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[1]); // a
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 92, 14);
    fun_327(vm, regs + 0); // call: lex_next
    if (!U_JUMPIFUNWOUND(vm, regs + 2, 294)) goto block38071;
    U_RETURNANY(vm, 0, 0);
    DecVal(vm, locals[1]);
    Pop(psp);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block38071:;
    U_STATEMENT(vm, regs + 0, 93, 14);
    regs[0] = locals[1]; // a
    U_INCREF(vm, regs + 1, 0);
    U_RETURNNONLOCAL(vm, 0, 1, 295);
    DecVal(vm, locals[1]);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// parse_exp
static void fun_329(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    BackupVar(vm, 635);
    locals[0] = lobster::NilVal();
    locals[1] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2822, locals);
    U_STATEMENT(vm, regs + 0, 95, 14);
    U_PUSHVARF(vm, regs + 0, 624); // token
    U_DUP(vm, regs + 1);
    U_PUSHSTR(vm, regs + 2, 525); // "("
    U_SEQ(vm, regs + 3);
    if (!U_JUMPFAIL(vm, regs + 2)) goto block38337;
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 97, 14);
    fun_327(vm, regs + 0); // call: lex_next
    if (!U_JUMPIFUNWOUND(vm, regs + 2, 294)) goto block38136;
    U_RETURNANY(vm, 0, 0);
    DecOwned(vm, 635);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block38136:;
    U_STATEMENT(vm, regs + 0, 98, 14);
    U_PUSHSTR(vm, regs + 0, 526); // "atom"
    fun_328(vm, regs + 1); // call: expect
    if (!U_JUMPIFUNWOUND(vm, regs + 2, 295)) goto block38157;
    U_RETURNANY(vm, 0, 1);
    DecOwned(vm, 635);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block38157:;
    U_LVAL_VARF(vm, regs + 1, 635); // name
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 99, 14);
    U_PUSHSTR(vm, regs + 0, 527); // "+"
    U_INCREF(vm, regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 528); // "*"
    U_INCREF(vm, regs + 2, 0);
    U_NEWVEC(vm, regs + 2, 11, 2);
    U_PUSHFUN(vm, regs + 1, 0, fun_330);
    fun_565(vm, regs + 2); // call: exists
    U_LOGNOT(vm, regs + 1);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block38221;
    U_STATEMENT(vm, regs + 0, 100, 14);
    U_PUSHSTR(vm, regs + 0, 529); // "unknown op: "
    U_PUSHVARF(vm, regs + 1, 635); // name
    U_SADD(vm, regs + 2);
    fun_324(vm, regs + 1); // call: error
    if (!U_JUMPIFUNWOUND(vm, regs + 2, 291)) goto block38219;
    U_RETURNANY(vm, 0, 0);
    DecOwned(vm, 635);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block38219:;
    block38221:;
    U_STATEMENT(vm, regs + 0, 101, 14);
    U_PUSHVARF(vm, regs + 0, 635); // name
    U_INCREF(vm, regs + 1, 0);
    U_NEWVEC(vm, regs + 1, 1092, 0);
    U_NEWOBJECT(vm, regs + 2, 1080); // inst
    SetLVal(vm, &locals[0]); // n
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 102, 14);
    block38249:;
    U_PUSHVARF(vm, regs + 0, 624); // token
    U_PUSHSTR(vm, regs + 1, 530); // ")"
    U_SNE(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block38300;
    U_STATEMENT(vm, regs + 0, 103, 14);
    regs[0] = locals[0]; // n
    U_PUSHFLD(vm, regs + 1, 1);
    fun_329(vm, regs + 1); // call: parse_exp
    if (!U_JUMPIFUNWOUND(vm, regs + 3, 296)) goto block38286;
    U_RETURNANY(vm, 0, 1);
    DecOwned(vm, 635);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 1]); }
    U_POP(vm, regs + 1);
    goto epilogue;
    block38286:;
    U_PUSHINT(vm, regs + 2, 1);
    U_BCALLRETV(vm, regs + 3, 15, 1); // push
    U_POP(vm, regs + 1);
    goto block38249;
    block38300:;
    U_STATEMENT(vm, regs + 0, 104, 14);
    fun_327(vm, regs + 0); // call: lex_next
    if (!U_JUMPIFUNWOUND(vm, regs + 2, 294)) goto block38318;
    U_RETURNANY(vm, 0, 0);
    DecOwned(vm, 635);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block38318:;
    U_STATEMENT(vm, regs + 0, 105, 14);
    regs[0] = locals[0]; // n
    U_INCREF(vm, regs + 1, 0);
    U_RETURNNONLOCAL(vm, 0, 1, 296);
    DecOwned(vm, 635);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    Push(psp, regs[0]);
    goto epilogue;
    goto block38431;
    block38337:;
    U_DUP(vm, regs + 1);
    U_PUSHSTR(vm, regs + 2, 531); // "int"
    U_SEQ(vm, regs + 3);
    if (!U_JUMPFAIL(vm, regs + 2)) goto block38401;
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 107, 14);
    U_PUSHVARF(vm, regs + 0, 628); // ival
    U_NEWOBJECT(vm, regs + 1, 1065); // integer
    SetLVal(vm, &locals[1]); // n
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 108, 14);
    fun_327(vm, regs + 0); // call: lex_next
    if (!U_JUMPIFUNWOUND(vm, regs + 2, 294)) goto block38382;
    U_RETURNANY(vm, 0, 0);
    DecOwned(vm, 635);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block38382:;
    U_STATEMENT(vm, regs + 0, 109, 14);
    regs[0] = locals[1]; // n
    U_INCREF(vm, regs + 1, 0);
    U_RETURNNONLOCAL(vm, 0, 1, 296);
    DecOwned(vm, 635);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    Push(psp, regs[0]);
    goto epilogue;
    goto block38431;
    block38401:;
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 111, 14);
    U_PUSHSTR(vm, regs + 0, 532); // "cannot parse expression starting with: "
    U_PUSHVARF(vm, regs + 1, 624); // token
    U_SADD(vm, regs + 2);
    fun_324(vm, regs + 1); // call: error
    if (!U_JUMPIFUNWOUND(vm, regs + 2, 291)) goto block38429;
    U_RETURNANY(vm, 0, 0);
    DecOwned(vm, 635);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) Push(psp, regs[i + 0]); }
    goto epilogue;
    block38429:;
    block38431:;
    U_RETURNNONLOCAL(vm, 0, 0, 296);
    DecOwned(vm, 635);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    goto epilogue;
    epilogue:;
    RestoreBackup(vm, 635);
    PopFunId(vm);
}

// function297
static void fun_330(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2830, locals);
    U_STATEMENT(vm, regs + 0, 99, 14);
    U_PUSHVARF(vm, regs + 0, 635); // name
    regs[1] = locals[0]; // _
    U_SEQ(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// floodfillpath
static void fun_331(VMRef vm, StackPtr psp) {
    Value regs[7];
    Value locals[6];
    SwapVars(vm, 644, psp, 5);
    SwapVars(vm, 645, psp, 4);
    locals[0] = *(psp - 3);
    locals[1] = *(psp - 2);
    SwapVars(vm, 648, psp, 1);
    BackupVar(vm, 649);
    locals[2] = lobster::NilVal();
    BackupVar(vm, 653);
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    locals[5] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2836, locals);
    U_STATEMENT(vm, regs + 0, 14, 16);
    U_PUSHVARVF(vm, regs + 0, 644, 2); // sz+0
    U_PUSHFUN(vm, regs + 2, 0, fun_332);
    fun_571(vm, regs + 3); // call: mapxy
    U_LVAL_VARF(vm, regs + 1, 649); // m
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 15, 16);
    U_PUSHVARF(vm, regs + 0, 649); // m
    regs[1] = locals[0];regs[2] = locals[1]; // target
    U_VPUSHIDXV(vm, regs + 3, 2);
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[2]); // end
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 16, 16);
    U_PUSHINT(vm, regs + 0, 0);
    regs[1] = locals[2]; // end
    U_LVAL_FLD(vm, regs + 2, 0);
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 17, 16);
    regs[0] = locals[2]; // end
    U_INCREF(vm, regs + 1, 0);
    U_NEWVEC(vm, regs + 1, 1119, 1);
    U_LVAL_VARF(vm, regs + 1, 653); // pathq
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 18, 16);
    U_STATEMENT(vm, regs + 0, 28, 16);
    block38566:;
    U_PUSHVARF(vm, regs + 0, 653); // pathq
    U_BCALLRET1(vm, regs + 1, 13, 1); // length
    if (!U_JUMPFAIL(vm, regs + 1)) goto block38693;
    U_STATEMENT(vm, regs + 0, 29, 16);
    U_PUSHVARF(vm, regs + 0, 653); // pathq
    U_BCALLRETV(vm, regs + 1, 16, 1); // pop
    SetLVal(vm, &locals[3]); // c
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 30, 16);
    U_PUSHINT(vm, regs + 0, -1);
    U_PUSHINT(vm, regs + 1, 2);
    block38604:;
    if (!U_IFOR(vm, regs + 2)) goto block38688;
    U_STATEMENT(vm, regs + 2, 30, 16);
    U_IFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[4]); // x
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 30, 16);
    U_PUSHINT(vm, regs + 2, -1);
    U_PUSHINT(vm, regs + 3, 2);
    block38630:;
    if (!U_IFOR(vm, regs + 4)) goto block38683;
    U_STATEMENT(vm, regs + 4, 30, 16);
    U_IFORELEM(vm, regs + 4);
    SetLVal(vm, &locals[5]); // y
    U_LV_WRITE(vm, regs + 5);
    U_STATEMENT(vm, regs + 4, 31, 16);
    U_PUSHINT(vm, regs + 4, 1);
    regs[5] = locals[5]; // y
    U_ISUB(vm, regs + 6);
    regs[5] = locals[4]; // x
    U_ISUB(vm, regs + 6);
    regs[5] = locals[5]; // y
    regs[6] = locals[4]; // x
    U_ISUB(vm, regs + 7);
    regs[6] = locals[3]; // c
    U_INCREF(vm, regs + 7, 0);
    fun_333(vm, regs + 7); // call: flood
    goto block38630;
    block38683:;
    goto block38604;
    block38688:;
    goto block38566;
    block38693:;
    U_STATEMENT(vm, regs + 0, 32, 16);
    U_PUSHVARF(vm, regs + 0, 649); // m
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[2]);
    DecOwned(vm, 653);
    DecVal(vm, locals[3]);
    psp = PopArg(vm, 648, psp);
    Pop(psp);
    Pop(psp);
    psp = PopArg(vm, 645, psp);
    psp = PopArg(vm, 644, psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    RestoreBackup(vm, 653);
    RestoreBackup(vm, 649);
    PopFunId(vm);
}

// function299
static void fun_332(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2852, locals);
    U_STATEMENT(vm, regs + 0, 14, 16);
    U_PUSHINT(vm, regs + 0, -1);
    U_PUSHVARVF(vm, regs + 1, 123, 2); // int2_0+0
    regs[3] = locals[0];regs[4] = locals[1]; // v
    U_NEWOBJECT(vm, regs + 5, 1098); // pathmap
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// flood
static void fun_333(VMRef vm, StackPtr psp) {
    Value regs[9];
    Value locals[9];
    locals[0] = *(psp - 3);
    locals[1] = *(psp - 2);
    locals[2] = *(psp - 1);
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    locals[5] = lobster::NilVal();
    locals[6] = lobster::NilVal();
    locals[7] = lobster::NilVal();
    locals[8] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2859, locals);
    U_STATEMENT(vm, regs + 0, 19, 16);
    regs[0] = locals[2]; // parent
    U_PUSHFLD2V(vm, regs + 1, 3, 2);
    regs[2] = locals[0];regs[3] = locals[1]; // dir
    U_IVVADD(vm, regs + 4, 2);
    SetLVal(vm, &locals[3]); // pos+0
    U_LV_WRITEV(vm, regs + 2, 2);
    U_STATEMENT(vm, regs + 0, 20, 16);
    regs[0] = locals[3];regs[1] = locals[4]; // pos
    U_PUSHINT(vm, regs + 2, 2);
    U_PUSHVARVF(vm, regs + 3, 644, 2); // sz+0
    U_PUSHINT(vm, regs + 5, 2);
    U_PUSHINT(vm, regs + 6, 0);
    U_PUSHINT(vm, regs + 7, 0);
    U_PUSHINT(vm, regs + 8, 2);
    U_BCALLRETV(vm, regs + 9, 107, 1); // in_range
    if (!U_JUMPFAIL(vm, regs + 1)) goto block39029;
    U_STATEMENT(vm, regs + 0, 21, 16);
    U_PUSHVARF(vm, regs + 0, 649); // m
    regs[1] = locals[3];regs[2] = locals[4]; // pos
    U_VPUSHIDXV(vm, regs + 3, 2);
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[5]); // c
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 22, 16);
    U_STATEMENT(vm, regs + 0, 22, 16);
    regs[0] = locals[3];regs[1] = locals[4]; // pos
    SetLVal(vm, &locals[7]); // v+0
    U_LV_WRITEV(vm, regs + 2, 2);
    U_STATEMENT(vm, regs + 0, 6, 15);
    U_PUSHINT(vm, regs + 0, 1);
    if (!U_JUMPFAILR(vm, regs + 1)) goto block38876;
    U_POP(vm, regs + 1);
    regs[0] = locals[5]; // c
    U_PUSHFLD(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 1, 0);
    U_ILT(vm, regs + 2);
    block38876:;
    if (!U_JUMPFAIL(vm, regs + 1)) goto block39027;
    U_STATEMENT(vm, regs + 0, 23, 16);
    regs[0] = locals[2]; // parent
    U_PUSHFLD(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 1, 1);
    U_IADD(vm, regs + 2);
    regs[1] = locals[5]; // c
    U_LVAL_FLD(vm, regs + 2, 0);
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 24, 16);
    regs[0] = locals[0];regs[1] = locals[1]; // dir
    U_IVUMINUS(vm, regs + 2, 2);
    regs[2] = locals[5]; // c
    U_LVAL_FLD(vm, regs + 3, 1);
    U_LV_WRITEV(vm, regs + 2, 2);
    U_STATEMENT(vm, regs + 0, 25, 16);
    U_PUSHVARF(vm, regs + 0, 653); // pathq
    U_BCALLRET1(vm, regs + 1, 13, 1); // length
    SetLVal(vm, &locals[6]); // i
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 26, 16);
    block38944:;
    regs[0] = locals[6]; // i
    U_PUSHINT(vm, regs + 1, 0);
    U_IGT(vm, regs + 2);
    if (!U_JUMPFAILR(vm, regs + 1)) goto block38983;
    U_POP(vm, regs + 1);
    U_PUSHVARF(vm, regs + 0, 653); // pathq
    regs[1] = locals[6]; // i
    U_PUSHINT(vm, regs + 2, 1);
    U_ISUB(vm, regs + 3);
    U_VPUSHIDXI(vm, regs + 2);
    U_PUSHFLD(vm, regs + 1, 0);
    regs[1] = locals[5]; // c
    U_PUSHFLD(vm, regs + 2, 0);
    U_ILT(vm, regs + 2);
    block38983:;
    if (!U_JUMPFAIL(vm, regs + 1)) goto block39000;
    U_STATEMENT(vm, regs + 0, 26, 16);
    SetLVal(vm, &locals[6]); // i
    U_LV_IMM(vm, regs + 0);
    goto block38944;
    block39000:;
    U_STATEMENT(vm, regs + 0, 27, 16);
    U_PUSHVARF(vm, regs + 0, 653); // pathq
    regs[1] = locals[6]; // i
    regs[2] = locals[5]; // c
    U_INCREF(vm, regs + 3, 0);
    U_PUSHINT(vm, regs + 3, 1);
    U_BCALLRETV(vm, regs + 4, 18, 1); // insert
    U_POP(vm, regs + 1);
    block39027:;
    block39029:;
    U_RETURNLOCAL(vm, 0, 0);
    DecVal(vm, locals[2]);
    DecVal(vm, locals[5]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function301
static void fun_334(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value keepvar[1];
    PushFunId(vm, funinfo_table + 2873, 0);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 6, 15);
    U_PUSHVARVF(vm, regs + 0, 125, 2); // int2_1+0
    U_PUSHINT(vm, regs + 2, 20);
    U_IVSMUL(vm, regs + 3, 2);
    U_PUSHVARVF(vm, regs + 2, 125, 2); // int2_1+0
    U_PUSHINT(vm, regs + 4, 5);
    U_IVSMUL(vm, regs + 5, 2);
    U_PUSHFUN(vm, regs + 4, 0, fun_10000001);
    fun_331(vm, regs + 5); // call: floodfillpath
    keepvar[0] = TopM(regs + 1, 0);
    U_BCALLRET1(vm, regs + 1, 13, 1); // length
    U_PUSHINT(vm, regs + 1, 20);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 6, 15, 533);
    U_RETURNLOCAL(vm, 0, 0);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// function303
static void fun_336(VMRef vm, StackPtr psp) {
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
    locals[0] = lobster::NilVal();
    BackupVar(vm, 691);
    BackupVar(vm, 692);
    BackupVar(vm, 693);
    PushFunId(vm, funinfo_table + 2878, locals);
    U_STATEMENT(vm, regs + 0, 9, 17);
    U_PUSHINT(vm, regs + 0, 0);
    U_BCALLRET1(vm, regs + 1, 97, 0); // rnd_seed
    U_STATEMENT(vm, regs + 0, 11, 17);
    U_STATEMENT(vm, regs + 0, 11, 17);
    U_STATEMENT(vm, regs + 0, 16, 17);
    U_PUSHINT(vm, regs + 0, 100);
    U_LVAL_VARF(vm, regs + 1, 666); // lssize
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 17, 17);
    U_PUSHFLT64(vm, regs + 0, -1717986918, 1070176665);
    U_LVAL_VARF(vm, regs + 1, 667); // hmargin
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 19, 17);
    U_PUSHVARVF(vm, regs + 0, 123, 2); // int2_0+0
    U_LVAL_VARF(vm, regs + 2, 668); // highest+0
    U_LV_WRITEV(vm, regs + 2, 2);
    U_STATEMENT(vm, regs + 0, 20, 17);
    U_PUSHVARVF(vm, regs + 0, 123, 2); // int2_0+0
    U_LVAL_VARF(vm, regs + 2, 670); // lowest+0
    U_LV_WRITEV(vm, regs + 2, 2);
    U_STATEMENT(vm, regs + 0, 21, 17);
    U_PUSHFLT(vm, regs + 0, -998637568);
    U_LVAL_VARF(vm, regs + 1, 672); // highh
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 22, 17);
    U_PUSHFLT(vm, regs + 0, 1148846080);
    U_LVAL_VARF(vm, regs + 1, 673); // lowh
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 24, 17);
    U_PUSHVARF(vm, regs + 0, 666); // lssize
    U_PUSHFUN(vm, regs + 1, 0, fun_337);
    fun_578(vm, regs + 2); // call: map
    U_LVAL_VARF(vm, regs + 1, 674); // cells
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 38, 17);
    U_STATEMENT(vm, regs + 0, 43, 17);
    U_STATEMENT(vm, regs + 0, 47, 17);
    U_PUSHVARVF(vm, regs + 0, 127, 2); // int2_x+0
    U_PUSHVARVF(vm, regs + 2, 129, 2); // int2_y+0
    U_PUSHVARVF(vm, regs + 4, 127, 2); // int2_x+0
    U_IVUMINUS(vm, regs + 6, 2);
    U_PUSHVARVF(vm, regs + 6, 129, 2); // int2_y+0
    U_IVUMINUS(vm, regs + 8, 2);
    U_NEWVEC(vm, regs + 8, 37, 4);
    U_LVAL_VARF(vm, regs + 1, 684); // neighbours
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 49, 17);
    U_STATEMENT(vm, regs + 0, 54, 17);
    U_PUSHFLT(vm, regs + 0, 0);
    SetLVal(vm, &locals[0]); // starting_extra_water
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 55, 17);
    U_PUSHFLT(vm, regs + 0, 1082130432);
    U_LVAL_VARF(vm, regs + 1, 691); // water_per_second
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 56, 17);
    U_PUSHFLT(vm, regs + 0, 1109393408);
    U_LVAL_VARF(vm, regs + 1, 692); // give_water_divider
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 57, 17);
    U_PUSHFLT64(vm, regs + 0, -1717986918, 1070176665);
    U_LVAL_VARF(vm, regs + 1, 693); // minimum_water_accumulation
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 59, 17);
    regs[0] = locals[0]; // starting_extra_water
    U_PUSHVARF(vm, regs + 1, 674); // cells
    U_PUSHVARVF(vm, regs + 2, 668, 2); // highest+0
    U_VPUSHIDXV(vm, regs + 4, 2);
    U_LVAL_FLD(vm, regs + 2, 1);
    U_LV_FADD(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 61, 17);
    U_STATEMENT(vm, regs + 0, 84, 17);
    U_PUSHINT(vm, regs + 0, -1);
    U_PUSHINT(vm, regs + 1, 5);
    block39363:;
    if (!U_IFOR(vm, regs + 2)) goto block39378;
    U_STATEMENT(vm, regs + 2, 84, 17);
    fun_343(vm, regs + 2); // call: sim
    goto block39363;
    block39378:;
    U_RETURNLOCAL(vm, 0, 0);
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

// function304
static void fun_337(VMRef vm, StackPtr psp) {
    Value regs[2];
    SwapVars(vm, 675, psp, 1);
    PushFunId(vm, funinfo_table + 2897, 0);
    U_STATEMENT(vm, regs + 0, 25, 17);
    U_PUSHVARF(vm, regs + 0, 666); // lssize
    U_PUSHFUN(vm, regs + 1, 0, fun_338);
    fun_579(vm, regs + 2); // call: map
    U_RETURNLOCAL(vm, 0, 1);
    psp = PopArg(vm, 675, psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function305
static void fun_338(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    locals[0] = *(psp - 1);
    locals[1] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2903, locals);
    U_STATEMENT(vm, regs + 0, 26, 17);
    U_BCALLRET0(vm, regs + 0, 95, 1); // rnd_float
    SetLVal(vm, &locals[1]); // h
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 27, 17);
    U_PUSHFLT(vm, regs + 0, 1065353216);
    SetLVal(vm, &locals[1]); // h
    U_LV_FADD(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 28, 17);
    regs[0] = locals[1]; // h
    U_PUSHFLT(vm, regs + 1, 1077936128);
    U_BCALLRET2(vm, regs + 2, 52, 1); // pow
    SetLVal(vm, &locals[1]); // h
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 29, 17);
    regs[0] = locals[1]; // h
    U_PUSHFLT(vm, regs + 1, 1092616192);
    U_FMUL(vm, regs + 2);
    SetLVal(vm, &locals[1]); // h
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 30, 17);
    regs[0] = locals[1]; // h
    U_PUSHVARF(vm, regs + 1, 672); // highh
    U_FGT(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block39522;
    U_STATEMENT(vm, regs + 0, 31, 17);
    regs[0] = locals[1]; // h
    U_LVAL_VARF(vm, regs + 1, 672); // highh
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 32, 17);
    regs[0] = locals[0]; // x
    U_PUSHVARF(vm, regs + 1, 675); // y
    U_LVAL_VARF(vm, regs + 2, 668); // highest+0
    U_LV_WRITEV(vm, regs + 2, 2);
    block39522:;
    U_STATEMENT(vm, regs + 0, 33, 17);
    regs[0] = locals[1]; // h
    U_PUSHVARF(vm, regs + 1, 673); // lowh
    U_FLT(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block39567;
    U_STATEMENT(vm, regs + 0, 34, 17);
    regs[0] = locals[1]; // h
    U_LVAL_VARF(vm, regs + 1, 673); // lowh
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 35, 17);
    regs[0] = locals[0]; // x
    U_PUSHVARF(vm, regs + 1, 675); // y
    U_LVAL_VARF(vm, regs + 2, 670); // lowest+0
    U_LV_WRITEV(vm, regs + 2, 2);
    block39567:;
    U_STATEMENT(vm, regs + 0, 36, 17);
    regs[0] = locals[1]; // h
    regs[1] = locals[1]; // h
    U_PUSHFLT(vm, regs + 2, 0);
    U_NEWOBJECT(vm, regs + 3, 1123); // cell
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// loopworld
static void fun_580(VMRef vm, StackPtr psp) {
    Value regs[8];
    Value locals[3];
    locals[0] = *(psp - 1);
    locals[1] = lobster::NilVal();
    locals[2] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2910, locals);
    U_STATEMENT(vm, regs + 0, 39, 17);
    U_PUSHINT(vm, regs + 0, -1);
    U_PUSHVARF(vm, regs + 1, 666); // lssize
    block39609:;
    if (!U_IFOR(vm, regs + 2)) goto block39685;
    U_STATEMENT(vm, regs + 2, 39, 17);
    U_IFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[1]); // y
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 40, 17);
    U_PUSHINT(vm, regs + 2, -1);
    U_PUSHVARF(vm, regs + 3, 666); // lssize
    block39635:;
    if (!U_IFOR(vm, regs + 4)) goto block39680;
    U_STATEMENT(vm, regs + 4, 40, 17);
    U_IFORELEM(vm, regs + 4);
    SetLVal(vm, &locals[2]); // x
    U_LV_WRITE(vm, regs + 5);
    U_STATEMENT(vm, regs + 4, 41, 17);
    regs[4] = locals[2]; // x
    regs[5] = locals[1]; // y
    U_PUSHVARF(vm, regs + 6, 674); // cells
    regs[7] = locals[1]; // y
    U_VPUSHIDXI(vm, regs + 8);
    regs[7] = locals[2]; // x
    U_VPUSHIDXI(vm, regs + 8);
    fun_346(vm, regs + 7); // call: function313
    goto block39635;
    block39680:;
    goto block39609;
    block39685:;
    U_RETURNLOCAL(vm, 0, 0);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// loopworld
static void fun_339(VMRef vm, StackPtr psp) {
    Value regs[8];
    Value locals[3];
    locals[0] = *(psp - 1);
    locals[1] = lobster::NilVal();
    locals[2] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2918, locals);
    U_STATEMENT(vm, regs + 0, 39, 17);
    U_PUSHINT(vm, regs + 0, -1);
    U_PUSHVARF(vm, regs + 1, 666); // lssize
    block39711:;
    if (!U_IFOR(vm, regs + 2)) goto block39790;
    U_STATEMENT(vm, regs + 2, 39, 17);
    U_IFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[1]); // y
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 40, 17);
    U_PUSHINT(vm, regs + 2, -1);
    U_PUSHVARF(vm, regs + 3, 666); // lssize
    block39737:;
    if (!U_IFOR(vm, regs + 4)) goto block39785;
    U_STATEMENT(vm, regs + 4, 40, 17);
    U_IFORELEM(vm, regs + 4);
    SetLVal(vm, &locals[2]); // x
    U_LV_WRITE(vm, regs + 5);
    U_STATEMENT(vm, regs + 4, 41, 17);
    regs[4] = locals[2]; // x
    regs[5] = locals[1]; // y
    U_PUSHVARF(vm, regs + 6, 674); // cells
    regs[7] = locals[1]; // y
    U_VPUSHIDXI(vm, regs + 8);
    regs[7] = locals[2]; // x
    U_VPUSHIDXI(vm, regs + 8);
    U_INCREF(vm, regs + 7, 0);
    fun_344(vm, regs + 7); // call: function311
    goto block39737;
    block39785:;
    goto block39711;
    block39790:;
    U_RETURNLOCAL(vm, 0, 0);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// ifinworld
static void fun_340(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[3];
    locals[0] = *(psp - 3);
    locals[1] = *(psp - 2);
    locals[2] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2926, locals);
    U_STATEMENT(vm, regs + 0, 44, 17);
    regs[0] = locals[0]; // pos+0
    U_PUSHINT(vm, regs + 1, 0);
    U_IGE(vm, regs + 2);
    if (!U_JUMPFAILR(vm, regs + 1)) goto block39831;
    U_POP(vm, regs + 1);
    regs[0] = locals[1]; // pos+1
    U_PUSHINT(vm, regs + 1, 0);
    U_IGE(vm, regs + 2);
    block39831:;
    if (!U_JUMPFAILR(vm, regs + 1)) goto block39846;
    U_POP(vm, regs + 1);
    regs[0] = locals[0]; // pos+0
    U_PUSHVARF(vm, regs + 1, 666); // lssize
    U_ILT(vm, regs + 2);
    block39846:;
    if (!U_JUMPFAILR(vm, regs + 1)) goto block39861;
    U_POP(vm, regs + 1);
    regs[0] = locals[1]; // pos+1
    U_PUSHVARF(vm, regs + 1, 666); // lssize
    U_ILT(vm, regs + 2);
    block39861:;
    if (!U_JUMPFAIL(vm, regs + 1)) goto block39873;
    U_STATEMENT(vm, regs + 0, 45, 17);
    fun_342(vm, regs + 0); // call: function309
    block39873:;
    U_RETURNLOCAL(vm, 0, 0);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// forneighboursinworld
static void fun_341(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value locals[2];
    locals[0] = *(psp - 3);
    locals[1] = *(psp - 2);
    SwapVars(vm, 687, psp, 1);
    BackupVar(vm, 688);
    BackupVar(vm, 689);
    PushFunId(vm, funinfo_table + 2934, locals);
    U_STATEMENT(vm, regs + 0, 50, 17);
    U_PUSHINT(vm, regs + 0, -1);
    U_PUSHVARF(vm, regs + 1, 684); // neighbours
    block39901:;
    if (!U_VFOR(vm, regs + 2)) goto block39951;
    U_STATEMENT(vm, regs + 2, 50, 17);
    U_VFORELEM2S(vm, regs + 2);
    U_LVAL_VARF(vm, regs + 4, 688); // n+0
    U_LV_WRITEV(vm, regs + 4, 2);
    U_STATEMENT(vm, regs + 2, 51, 17);
    regs[2] = locals[0]; // x
    regs[3] = locals[1]; // y
    U_LVAL_VARF(vm, regs + 4, 688); // n+0
    U_LV_IVVADD(vm, regs + 4, 2);
    U_STATEMENT(vm, regs + 2, 52, 17);
    U_PUSHVARVF(vm, regs + 2, 688, 2); // n+0
    U_PUSHFUN(vm, regs + 4, 0, fun_342);
    fun_340(vm, regs + 5); // call: ifinworld
    goto block39901;
    block39951:;
    U_RETURNLOCAL(vm, 0, 0);
    psp = PopArg(vm, 687, psp);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    RestoreBackup(vm, 689);
    RestoreBackup(vm, 688);
    PopFunId(vm);
}

// function309
static void fun_342(VMRef vm, StackPtr psp) {
    Value regs[2];
    PushFunId(vm, funinfo_table + 2944, 0);
    U_STATEMENT(vm, regs + 0, 52, 17);
    U_PUSHVARF(vm, regs + 0, 674); // cells
    U_PUSHVARF(vm, regs + 1, 689); // n+1
    U_VPUSHIDXI(vm, regs + 2);
    U_PUSHVARF(vm, regs + 1, 688); // n+0
    U_VPUSHIDXI(vm, regs + 2);
    fun_345(vm, regs + 1); // call: function312
    U_RETURNLOCAL(vm, 0, 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// sim
static void fun_343(VMRef vm, StackPtr psp) {
    Value regs[4];
    PushFunId(vm, funinfo_table + 2949, 0);
    U_STATEMENT(vm, regs + 0, 62, 17);
    U_PUSHFLT64(vm, regs + 0, 286331153, 1066471697);
    U_PUSHVARF(vm, regs + 1, 691); // water_per_second
    U_FMUL(vm, regs + 2);
    U_PUSHVARF(vm, regs + 1, 674); // cells
    U_PUSHVARVF(vm, regs + 2, 668, 2); // highest+0
    U_VPUSHIDXV(vm, regs + 4, 2);
    U_LVAL_FLD(vm, regs + 2, 1);
    U_LV_FADD(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 63, 17);
    U_PUSHFUN(vm, regs + 0, 0, fun_344);
    fun_339(vm, regs + 1); // call: loopworld
    U_STATEMENT(vm, regs + 0, 79, 17);
    U_PUSHFUN(vm, regs + 0, 0, fun_346);
    fun_580(vm, regs + 1); // call: loopworld
    U_RETURNLOCAL(vm, 0, 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function311
static void fun_344(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[5];
    locals[0] = *(psp - 3);
    locals[1] = *(psp - 2);
    SwapVars(vm, 696, psp, 1);
    BackupVar(vm, 697);
    BackupVar(vm, 698);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2954, locals);
    U_STATEMENT(vm, regs + 0, 64, 17);
    U_PUSHVARF(vm, regs + 0, 696); // c
    U_PUSHFLD(vm, regs + 1, 1);
    U_PUSHVARF(vm, regs + 1, 696); // c
    U_PUSHFLD(vm, regs + 2, 0);
    U_FSUB(vm, regs + 2);
    U_BCALLRET1(vm, regs + 1, 112, 1); // abs
    U_PUSHVARF(vm, regs + 1, 693); // minimum_water_accumulation
    U_FGT(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block40270;
    U_STATEMENT(vm, regs + 0, 65, 17);
    U_PUSHFLT(vm, regs + 0, 0);
    U_LVAL_VARF(vm, regs + 1, 697); // totald
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 66, 17);
    U_NEWVEC(vm, regs + 0, 1138, 0);
    U_LVAL_VARF(vm, regs + 1, 698); // lower
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 69, 17);
    regs[0] = locals[0]; // x
    regs[1] = locals[1]; // y
    U_PUSHFUN(vm, regs + 2, 0, fun_345);
    fun_341(vm, regs + 3); // call: forneighboursinworld
    U_STATEMENT(vm, regs + 0, 74, 17);
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
    U_BCALLRET2(vm, regs + 2, 120, 1); // min
    SetLVal(vm, &locals[2]); // w
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 75, 17);
    U_PUSHINT(vm, regs + 0, -1);
    U_PUSHVARF(vm, regs + 1, 698); // lower
    block40186:;
    if (!U_VFOR(vm, regs + 2)) goto block40268;
    U_STATEMENT(vm, regs + 2, 75, 17);
    U_VFORELEMREF(vm, regs + 2);
    SetLVal(vm, &locals[3]); // nc
    U_LV_WRITEREF(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 76, 17);
    U_PUSHVARF(vm, regs + 2, 696); // c
    U_PUSHFLD(vm, regs + 3, 1);
    regs[3] = locals[3]; // nc
    U_PUSHFLD(vm, regs + 4, 1);
    U_FSUB(vm, regs + 4);
    U_PUSHVARF(vm, regs + 3, 697); // totald
    U_FDIV(vm, regs + 4);
    regs[3] = locals[2]; // w
    U_FMUL(vm, regs + 4);
    SetLVal(vm, &locals[4]); // give
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 77, 17);
    regs[2] = locals[4]; // give
    regs[3] = locals[3]; // nc
    U_LVAL_FLD(vm, regs + 4, 2);
    U_LV_FADD(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 78, 17);
    regs[2] = locals[4]; // give
    U_PUSHVARF(vm, regs + 3, 696); // c
    U_LVAL_FLD(vm, regs + 4, 2);
    U_LV_FSUB(vm, regs + 3);
    goto block40186;
    block40268:;
    block40270:;
    U_RETURNLOCAL(vm, 0, 0);
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

// function312
static void fun_345(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[2];
    locals[0] = *(psp - 1);
    locals[1] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2967, locals);
    U_STATEMENT(vm, regs + 0, 70, 17);
    U_PUSHVARF(vm, regs + 0, 696); // c
    U_PUSHFLD(vm, regs + 1, 1);
    regs[1] = locals[0]; // nc
    U_PUSHFLD(vm, regs + 2, 1);
    U_FSUB(vm, regs + 2);
    SetLVal(vm, &locals[1]); // d
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 71, 17);
    regs[0] = locals[1]; // d
    U_PUSHVARF(vm, regs + 1, 667); // hmargin
    U_FGT(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block40357;
    U_STATEMENT(vm, regs + 0, 72, 17);
    regs[0] = locals[1]; // d
    U_LVAL_VARF(vm, regs + 1, 697); // totald
    U_LV_FADD(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 73, 17);
    U_PUSHVARF(vm, regs + 0, 698); // lower
    regs[1] = locals[0]; // nc
    U_INCREF(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 2, 1);
    U_BCALLRETV(vm, regs + 3, 15, 1); // push
    U_POP(vm, regs + 1);
    block40357:;
    U_RETURNLOCAL(vm, 0, 0);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function313
static void fun_346(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[3];
    locals[0] = *(psp - 3);
    locals[1] = *(psp - 2);
    locals[2] = *(psp - 1);
    PushFunId(vm, funinfo_table + 2974, locals);
    U_STATEMENT(vm, regs + 0, 80, 17);
    regs[0] = locals[2]; // c
    U_PUSHFLD(vm, regs + 1, 2);
    U_PUSHFLT(vm, regs + 1, 0);
    U_FNE(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block40424;
    U_STATEMENT(vm, regs + 0, 81, 17);
    regs[0] = locals[2]; // c
    U_PUSHFLD(vm, regs + 1, 2);
    regs[1] = locals[2]; // c
    U_LVAL_FLD(vm, regs + 2, 1);
    U_LV_FADD(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 82, 17);
    U_PUSHFLT(vm, regs + 0, 0);
    regs[1] = locals[2]; // c
    U_LVAL_FLD(vm, regs + 2, 2);
    U_LV_WRITE(vm, regs + 1);
    block40424:;
    U_RETURNLOCAL(vm, 0, 0);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function314
static void fun_347(VMRef vm, StackPtr psp) {
    Value regs[6];
    Value keepvar[1];
    Value locals[6];
    BackupVar(vm, 707);
    BackupVar(vm, 708);
    locals[0] = lobster::NilVal();
    locals[1] = lobster::NilVal();
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    locals[5] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2982, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 7, 18);
    U_PUSHINT(vm, regs + 0, 0);
    U_BCALLRET1(vm, regs + 1, 97, 0); // rnd_seed
    U_STATEMENT(vm, regs + 0, 9, 18);
    U_PUSHINT(vm, regs + 0, 100);
    U_LVAL_VARF(vm, regs + 1, 707); // N
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 10, 18);
    U_PUSHFLT(vm, regs + 0, 1073741824);
    U_PUSHVARF(vm, regs + 1, 707); // N
    U_I2F(vm, regs + 2);
    U_FDIV(vm, regs + 2);
    U_LVAL_VARF(vm, regs + 1, 708); // f
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 12, 18);
    U_STATEMENT(vm, regs + 0, 29, 18);
    U_PUSHFLT64(vm, regs + 0, -1717986918, 1069128089);
    SetLVal(vm, &locals[0]); // sigma
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 30, 18);
    U_PUSHFLT64(vm, regs + 0, -755914244, 1062232653);
    SetLVal(vm, &locals[1]); // mu
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 31, 18);
    U_PUSHINT(vm, regs + 0, 1000);
    SetLVal(vm, &locals[2]); // n_epochs
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 33, 18);
    U_PUSHVARF(vm, regs + 0, 707); // N
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_581(vm, regs + 2); // call: map
    SetLVal(vm, &locals[3]); // x
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 34, 18);
    U_PUSHVARF(vm, regs + 0, 707); // N
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_582(vm, regs + 2); // call: map
    SetLVal(vm, &locals[4]); // d
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 36, 18);
    U_PUSHINT(vm, regs + 0, -1);
    U_PUSHVARF(vm, regs + 1, 707); // N
    block40577:;
    if (!U_IFOR(vm, regs + 2)) goto block40667;
    U_STATEMENT(vm, regs + 2, 36, 18);
    U_IFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[5]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 37, 18);
    U_PUSHVARF(vm, regs + 2, 708); // f
    regs[3] = locals[5]; // i
    U_BCALLRET1(vm, regs + 4, 67, 1); // float
    U_FMUL(vm, regs + 4);
    regs[3] = locals[3]; // x
    regs[4] = locals[5]; // i
    U_LVAL_IDXVI(vm, regs + 5, 0);
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 38, 18);
    U_PUSHFLT(vm, regs + 2, 1077936128);
    U_PUSHFLT(vm, regs + 3, 1073741824);
    regs[4] = locals[3]; // x
    regs[5] = locals[5]; // i
    U_VPUSHIDXI(vm, regs + 6);
    U_FMUL(vm, regs + 5);
    U_FADD(vm, regs + 4);
    regs[3] = locals[0]; // sigma
    U_BCALLRET0(vm, regs + 4, 96, 1); // rnd_gaussian
    U_FMUL(vm, regs + 5);
    U_FADD(vm, regs + 4);
    regs[3] = locals[4]; // d
    regs[4] = locals[5]; // i
    U_LVAL_IDXVI(vm, regs + 5, 0);
    U_LV_WRITE(vm, regs + 3);
    goto block40577;
    block40667:;
    U_STATEMENT(vm, regs + 0, 40, 18);
    regs[0] = locals[3]; // x
    U_INCREF(vm, regs + 1, 0);
    regs[1] = locals[4]; // d
    U_INCREF(vm, regs + 2, 0);
    regs[2] = locals[1]; // mu
    regs[3] = locals[2]; // n_epochs
    fun_348(vm, regs + 4); // call: gradient_descent
    keepvar[0] = TopM(regs + 1, 0);
    U_PUSHINT(vm, regs + 1, 0);
    U_VPUSHIDXI(vm, regs + 2);
    U_PUSHFLT64(vm, regs + 1, -1941428633, 1073966704);
    U_FSUB(vm, regs + 2);
    U_BCALLRET1(vm, regs + 1, 112, 1); // abs
    U_PUSHFLT64(vm, regs + 1, 1202590843, 1065646817);
    U_FLT(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 40, 18, 534);
    U_RETURNLOCAL(vm, 0, 0);
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
static void fun_348(VMRef vm, StackPtr psp) {
    Value regs[8];
    Value locals[12];
    locals[0] = *(psp - 4);
    locals[1] = *(psp - 3);
    locals[2] = *(psp - 2);
    locals[3] = *(psp - 1);
    locals[4] = lobster::NilVal();
    locals[5] = lobster::NilVal();
    locals[6] = lobster::NilVal();
    locals[7] = lobster::NilVal();
    locals[8] = lobster::NilVal();
    locals[9] = lobster::NilVal();
    locals[10] = lobster::NilVal();
    locals[11] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 2995, locals);
    U_STATEMENT(vm, regs + 0, 13, 18);
    U_PUSHVARF(vm, regs + 0, 707); // N
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_583(vm, regs + 2); // call: map
    SetLVal(vm, &locals[4]); // y
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 14, 18);
    U_PUSHFLT(vm, regs + 0, 0);
    SetLVal(vm, &locals[5]); // w0
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 15, 18);
    U_PUSHFLT(vm, regs + 0, 0);
    SetLVal(vm, &locals[6]); // w1
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 16, 18);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[3]; // n_epochs
    block40802:;
    if (!U_IFOR(vm, regs + 2)) goto block41029;
    U_STATEMENT(vm, regs + 2, 17, 18);
    U_PUSHFLT(vm, regs + 2, 0);
    SetLVal(vm, &locals[7]); // grad0
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 18, 18);
    U_PUSHFLT(vm, regs + 2, 0);
    SetLVal(vm, &locals[8]); // grad1
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 19, 18);
    U_PUSHINT(vm, regs + 2, -1);
    U_PUSHVARF(vm, regs + 3, 707); // N
    block40841:;
    if (!U_IFOR(vm, regs + 4)) goto block40926;
    U_STATEMENT(vm, regs + 4, 19, 18);
    U_IFORELEM(vm, regs + 4);
    SetLVal(vm, &locals[9]); // i
    U_LV_WRITE(vm, regs + 5);
    U_STATEMENT(vm, regs + 4, 20, 18);
    U_PUSHVARF(vm, regs + 4, 708); // f
    regs[5] = locals[1]; // d
    regs[6] = locals[9]; // i
    U_VPUSHIDXI(vm, regs + 7);
    regs[6] = locals[4]; // y
    regs[7] = locals[9]; // i
    U_VPUSHIDXI(vm, regs + 8);
    U_FSUB(vm, regs + 7);
    U_FMUL(vm, regs + 6);
    SetLVal(vm, &locals[10]); // err
    U_LV_WRITE(vm, regs + 5);
    U_STATEMENT(vm, regs + 4, 21, 18);
    regs[4] = locals[10]; // err
    SetLVal(vm, &locals[7]); // grad0
    U_LV_FADD(vm, regs + 5);
    U_STATEMENT(vm, regs + 4, 22, 18);
    regs[4] = locals[10]; // err
    regs[5] = locals[0]; // x
    regs[6] = locals[9]; // i
    U_VPUSHIDXI(vm, regs + 7);
    U_FMUL(vm, regs + 6);
    SetLVal(vm, &locals[8]); // grad1
    U_LV_FADD(vm, regs + 5);
    goto block40841;
    block40926:;
    U_STATEMENT(vm, regs + 2, 23, 18);
    regs[2] = locals[2]; // mu
    regs[3] = locals[7]; // grad0
    U_FMUL(vm, regs + 4);
    SetLVal(vm, &locals[5]); // w0
    U_LV_FADD(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 24, 18);
    regs[2] = locals[2]; // mu
    regs[3] = locals[8]; // grad1
    U_FMUL(vm, regs + 4);
    SetLVal(vm, &locals[6]); // w1
    U_LV_FADD(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 25, 18);
    U_PUSHINT(vm, regs + 2, -1);
    U_PUSHVARF(vm, regs + 3, 707); // N
    block40972:;
    if (!U_IFOR(vm, regs + 4)) goto block41024;
    U_STATEMENT(vm, regs + 4, 25, 18);
    U_IFORELEM(vm, regs + 4);
    SetLVal(vm, &locals[11]); // i
    U_LV_WRITE(vm, regs + 5);
    U_STATEMENT(vm, regs + 4, 26, 18);
    regs[4] = locals[5]; // w0
    regs[5] = locals[6]; // w1
    regs[6] = locals[0]; // x
    regs[7] = locals[11]; // i
    U_VPUSHIDXI(vm, regs + 8);
    U_FMUL(vm, regs + 7);
    U_FADD(vm, regs + 6);
    regs[5] = locals[4]; // y
    regs[6] = locals[11]; // i
    U_LVAL_IDXVI(vm, regs + 7, 0);
    U_LV_WRITE(vm, regs + 5);
    goto block40972;
    block41024:;
    goto block40802;
    block41029:;
    U_STATEMENT(vm, regs + 0, 27, 18);
    regs[0] = locals[5]; // w0
    regs[1] = locals[6]; // w1
    U_NEWVEC(vm, regs + 2, 9, 2);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[4]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function319
static void fun_352(VMRef vm, StackPtr psp) {
    Value regs[10];
    Value locals[36];
    BackupVar(vm, 727);
    locals[0] = lobster::NilVal();
    locals[1] = lobster::NilVal();
    BackupVar(vm, 743);
    BackupVar(vm, 744);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    locals[5] = lobster::NilVal();
    locals[6] = lobster::NilVal();
    locals[7] = lobster::NilVal();
    locals[8] = lobster::NilVal();
    locals[9] = lobster::NilVal();
    locals[10] = lobster::NilVal();
    locals[11] = lobster::NilVal();
    locals[12] = lobster::NilVal();
    locals[13] = lobster::NilVal();
    locals[14] = lobster::NilVal();
    locals[15] = lobster::NilVal();
    locals[16] = lobster::NilVal();
    locals[17] = lobster::NilVal();
    BackupVar(vm, 773);
    BackupVar(vm, 774);
    locals[18] = lobster::NilVal();
    locals[19] = lobster::NilVal();
    locals[20] = lobster::NilVal();
    locals[21] = lobster::NilVal();
    locals[22] = lobster::NilVal();
    locals[23] = lobster::NilVal();
    locals[24] = lobster::NilVal();
    locals[25] = lobster::NilVal();
    locals[26] = lobster::NilVal();
    locals[27] = lobster::NilVal();
    locals[28] = lobster::NilVal();
    locals[29] = lobster::NilVal();
    locals[30] = lobster::NilVal();
    locals[31] = lobster::NilVal();
    locals[32] = lobster::NilVal();
    locals[33] = lobster::NilVal();
    locals[34] = lobster::NilVal();
    locals[35] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 3012, locals);
    U_STATEMENT(vm, regs + 0, 8, 19);
    U_PUSHINT(vm, regs + 0, 0);
    U_BCALLRET1(vm, regs + 1, 97, 0); // rnd_seed
    U_STATEMENT(vm, regs + 0, 10, 19);
    U_STATEMENT(vm, regs + 0, 10, 19);
    U_STATEMENT(vm, regs + 0, 14, 19);
    U_STATEMENT(vm, regs + 0, 14, 19);
    U_STATEMENT(vm, regs + 0, 18, 19);
    U_NEWVEC(vm, regs + 0, 1148, 0);
    U_LVAL_VARF(vm, regs + 1, 727); // verts
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 19, 19);
    U_NEWVEC(vm, regs + 0, 13, 0);
    SetLVal(vm, &locals[0]); // edges
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 20, 19);
    U_NEWVEC(vm, regs + 0, 13, 0);
    SetLVal(vm, &locals[1]); // quads
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 22, 19);
    U_STATEMENT(vm, regs + 0, 36, 19);
    U_PUSHVARVF(vm, regs + 0, 125, 2); // int2_1+0
    U_PUSHINT(vm, regs + 2, 9);
    U_IVSMUL(vm, regs + 3, 2);
    U_LVAL_VARF(vm, regs + 2, 743); // wsize+0
    U_LV_WRITEV(vm, regs + 2, 2);
    U_STATEMENT(vm, regs + 0, 37, 19);
    U_NEWVEC(vm, regs + 0, 1148, 0);
    U_LVAL_VARF(vm, regs + 1, 727); // verts
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 38, 19);
    U_NEWVEC(vm, regs + 0, 13, 0);
    SetLVal(vm, &locals[0]); // edges
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 39, 19);
    U_PUSHVARVF(vm, regs + 0, 743, 2); // wsize+0
    U_PUSHFUN(vm, regs + 2, 0, fun_355);
    fun_98(vm, regs + 3); // call: forxy
    U_STATEMENT(vm, regs + 0, 41, 19);
    block41243:;
    fun_353(vm, regs + 0); // call: wiggleverts
    U_PUSHFLT(vm, regs + 1, 1065353216);
    U_FGT(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block41263;
    U_STATEMENT(vm, regs + 0, 41, 19);
    goto block41243;
    block41263:;
    U_STATEMENT(vm, regs + 0, 42, 19);
    U_STATEMENT(vm, regs + 0, 42, 19);
    U_STATEMENT(vm, regs + 0, 46, 19);
    U_NEWVEC(vm, regs + 0, 1165, 0);
    SetLVal(vm, &locals[2]); // pedges
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 47, 19);
    U_PUSHINT(vm, regs + 0, -1);
    U_PUSHVARF(vm, regs + 1, 727); // verts
    block41296:;
    if (!U_VFOR(vm, regs + 2)) goto block41456;
    U_STATEMENT(vm, regs + 2, 47, 19);
    U_VFORELEM2S(vm, regs + 2);
    SetLVal(vm, &locals[3]); // v+0
    U_LV_WRITEV(vm, regs + 4, 2);
    U_STATEMENT(vm, regs + 2, 47, 19);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[5]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 48, 19);
    U_PUSHINT(vm, regs + 2, -1);
    U_PUSHVARF(vm, regs + 3, 727); // verts
    block41334:;
    if (!U_VFOR(vm, regs + 4)) goto block41451;
    U_STATEMENT(vm, regs + 4, 48, 19);
    U_VFORELEM2S(vm, regs + 4);
    SetLVal(vm, &locals[6]); // w+0
    U_LV_WRITEV(vm, regs + 6, 2);
    U_STATEMENT(vm, regs + 4, 48, 19);
    U_FORLOOPI(vm, regs + 4);
    SetLVal(vm, &locals[8]); // j
    U_LV_WRITE(vm, regs + 5);
    U_STATEMENT(vm, regs + 4, 49, 19);
    regs[4] = locals[5]; // i
    regs[5] = locals[8]; // j
    U_ILT(vm, regs + 6);
    if (!U_JUMPFAIL(vm, regs + 5)) goto block41446;
    U_STATEMENT(vm, regs + 4, 50, 19);
    regs[4] = locals[6];regs[5] = locals[7]; // w
    regs[6] = locals[3];regs[7] = locals[4]; // v
    U_FVVSUB(vm, regs + 8, 2);
    U_PUSHINT(vm, regs + 6, 2);
    U_BCALLRETV(vm, regs + 7, 86, 1); // magnitude
    SetLVal(vm, &locals[9]); // dist
    U_LV_WRITE(vm, regs + 5);
    U_STATEMENT(vm, regs + 4, 51, 19);
    regs[4] = locals[9]; // dist
    U_PUSHFLT(vm, regs + 5, 1073741824);
    U_FLT(vm, regs + 6);
    if (!U_JUMPFAIL(vm, regs + 5)) goto block41444;
    U_STATEMENT(vm, regs + 4, 52, 19);
    regs[4] = locals[2]; // pedges
    regs[5] = locals[5]; // i
    regs[6] = locals[8]; // j
    regs[7] = locals[9]; // dist
    U_PUSHINT(vm, regs + 8, 3);
    U_BCALLRETV(vm, regs + 9, 15, 1); // push
    U_POP(vm, regs + 5);
    block41444:;
    block41446:;
    goto block41334;
    block41451:;
    goto block41296;
    block41456:;
    U_STATEMENT(vm, regs + 0, 53, 19);
    regs[0] = locals[2]; // pedges
    U_INCREF(vm, regs + 1, 0);
    U_PUSHFUN(vm, regs + 1, 0, fun_356);
    fun_585(vm, regs + 2); // call: qsort
    SetLVal(vm, &locals[2]); // pedges
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 54, 19);
    regs[0] = locals[2]; // pedges
    U_PUSHINT(vm, regs + 1, 0);
    U_VPUSHIDXIS2V(vm, regs + 2, 1, 2);
    U_PUSHFLT(vm, regs + 1, 1073741824);
    U_BCALLRET1(vm, regs + 2, 56, 1); // sqrt
    U_FMUL(vm, regs + 2);
    SetLVal(vm, &locals[10]); // edgedistnocross
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 55, 19);
    U_PUSHVARF(vm, regs + 0, 727); // verts
    U_INCREF(vm, regs + 1, 0);
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_589(vm, regs + 2); // call: map
    SetLVal(vm, &locals[11]); // vnbs
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 56, 19);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[2]; // pedges
    block41538:;
    if (!U_VFOR(vm, regs + 2)) goto block41649;
    U_STATEMENT(vm, regs + 2, 56, 19);
    U_VFORELEM2S(vm, regs + 2);
    SetLVal(vm, &locals[12]); // pe+0
    U_LV_WRITEV(vm, regs + 5, 3);
    U_STATEMENT(vm, regs + 2, 57, 19);
    regs[2] = locals[14]; // pe+2
    regs[3] = locals[10]; // edgedistnocross
    U_FLT(vm, regs + 4);
    if (!U_JUMPFAIL(vm, regs + 3)) goto block41644;
    U_STATEMENT(vm, regs + 2, 58, 19);
    regs[2] = locals[0]; // edges
    regs[3] = locals[12]; // pe+0
    regs[4] = locals[13]; // pe+1
    U_NEWVEC(vm, regs + 5, 7, 2);
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 59, 19);
    regs[2] = locals[11]; // vnbs
    regs[3] = locals[12]; // pe+0
    U_VPUSHIDXI(vm, regs + 4);
    regs[3] = locals[13]; // pe+1
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 60, 19);
    regs[2] = locals[11]; // vnbs
    regs[3] = locals[13]; // pe+1
    U_VPUSHIDXI(vm, regs + 4);
    regs[3] = locals[12]; // pe+0
    U_PUSHINT(vm, regs + 4, 1);
    U_BCALLRETV(vm, regs + 5, 15, 1); // push
    U_POP(vm, regs + 3);
    block41644:;
    goto block41538;
    block41649:;
    U_STATEMENT(vm, regs + 0, 61, 19);
    U_NEWVEC(vm, regs + 0, 1165, 0);
    SetLVal(vm, &locals[2]); // pedges
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 64, 19);
    regs[0] = locals[11]; // vnbs
    U_INCREF(vm, regs + 1, 0);
    U_PUSHFUN(vm, regs + 1, 0, fun_358);
    fun_590(vm, regs + 2); // call: map
    SetLVal(vm, &locals[11]); // vnbs
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 65, 19);
    U_NEWVEC(vm, regs + 0, 13, 0);
    SetLVal(vm, &locals[15]); // tris
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 66, 19);
    U_NEWVEC(vm, regs + 0, 13, 0);
    SetLVal(vm, &locals[1]); // quads
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 67, 19);
    U_PUSHINT(vm, regs + 0, -1);
    U_PUSHVARF(vm, regs + 1, 727); // verts
    U_BCALLRET1(vm, regs + 2, 13, 1); // length
    block41725:;
    if (!U_IFOR(vm, regs + 2)) goto block42202;
    U_STATEMENT(vm, regs + 2, 67, 19);
    U_IFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[16]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 68, 19);
    U_PUSHINT(vm, regs + 2, -1);
    regs[3] = locals[11]; // vnbs
    regs[4] = locals[16]; // i
    U_VPUSHIDXI(vm, regs + 5);
    block41756:;
    if (!U_VFOR(vm, regs + 4)) goto block42197;
    U_STATEMENT(vm, regs + 4, 68, 19);
    U_VFORELEM(vm, regs + 4);
    SetLVal(vm, &locals[17]); // j
    U_LV_WRITE(vm, regs + 5);
    U_STATEMENT(vm, regs + 4, 69, 19);
    regs[4] = locals[16]; // i
    regs[5] = locals[17]; // j
    U_NEWVEC(vm, regs + 6, 7, 2);
    U_LVAL_VARF(vm, regs + 5, 773); // shape
    U_LV_WRITEREF(vm, regs + 5);
    U_STATEMENT(vm, regs + 4, 70, 19);
    regs[4] = locals[16]; // i
    U_LVAL_VARF(vm, regs + 5, 774); // p
    U_LV_WRITE(vm, regs + 5);
    U_STATEMENT(vm, regs + 4, 71, 19);
    block41807:;
    U_PUSHVARF(vm, regs + 4, 773); // shape
    U_BCALLRETV(vm, regs + 5, 16, 1); // pop
    regs[5] = locals[16]; // i
    U_INE(vm, regs + 6);
    if (!U_JUMPFAIL(vm, regs + 5)) goto block41963;
    U_STATEMENT(vm, regs + 4, 72, 19);
    U_PUSHVARF(vm, regs + 4, 773); // shape
    regs[5] = locals[17]; // j
    U_PUSHINT(vm, regs + 6, 1);
    U_BCALLRETV(vm, regs + 7, 15, 1); // push
    U_POP(vm, regs + 5);
    U_STATEMENT(vm, regs + 4, 73, 19);
    regs[4] = locals[11]; // vnbs
    regs[5] = locals[17]; // j
    U_VPUSHIDXI(vm, regs + 6);
    U_INCREF(vm, regs + 5, 0);
    SetLVal(vm, &locals[18]); // nbl
    U_LV_WRITEREF(vm, regs + 5);
    U_STATEMENT(vm, regs + 4, 74, 19);
    regs[4] = locals[18]; // nbl
    U_PUSHFUN(vm, regs + 5, 0, fun_360);
    fun_594(vm, regs + 6); // call: find
    SetLVal(vm, &locals[19]); // pidx
    U_LV_WRITE(vm, regs + 5);
    U_STATEMENT(vm, regs + 4, 75, 19);
    regs[4] = locals[19]; // pidx
    U_PUSHINT(vm, regs + 5, 0);
    U_IGE(vm, regs + 6);
    U_ASSERT(vm, regs + 5, 75, 19, 535);
    U_STATEMENT(vm, regs + 4, 76, 19);
    regs[4] = locals[17]; // j
    U_LVAL_VARF(vm, regs + 5, 774); // p
    U_LV_WRITE(vm, regs + 5);
    U_STATEMENT(vm, regs + 4, 77, 19);
    regs[4] = locals[18]; // nbl
    regs[5] = locals[19]; // pidx
    U_PUSHINT(vm, regs + 6, 1);
    U_IADD(vm, regs + 7);
    regs[6] = locals[18]; // nbl
    U_BCALLRET1(vm, regs + 7, 13, 1); // length
    U_IMOD(vm, regs + 7);
    U_VPUSHIDXI(vm, regs + 6);
    SetLVal(vm, &locals[17]); // j
    U_LV_WRITE(vm, regs + 5);
    U_STATEMENT(vm, regs + 4, 78, 19);
    U_PUSHVARF(vm, regs + 4, 773); // shape
    regs[5] = locals[17]; // j
    U_PUSHINT(vm, regs + 6, 1);
    U_BCALLRETV(vm, regs + 7, 15, 1); // push
    U_POP(vm, regs + 5);
    goto block41807;
    block41963:;
    U_STATEMENT(vm, regs + 4, 79, 19);
    U_PUSHVARF(vm, regs + 4, 773); // shape
    U_BCALLRET1(vm, regs + 5, 13, 1); // length
    U_PUSHINT(vm, regs + 5, 4);
    U_ILE(vm, regs + 6);
    if (!U_JUMPFAIL(vm, regs + 5)) goto block42192;
    U_STATEMENT(vm, regs + 4, 80, 19);
    U_PUSHVARF(vm, regs + 4, 773); // shape
    U_INCREF(vm, regs + 5, 0);
    U_PUSHFUN(vm, regs + 5, 0, fun_361);
    fun_595(vm, regs + 6); // call: reduce
    SetLVal(vm, &locals[20]); // start
    U_LV_WRITE(vm, regs + 5);
    U_STATEMENT(vm, regs + 4, 81, 19);
    block42009:;
    U_PUSHVARF(vm, regs + 4, 773); // shape
    U_PUSHINT(vm, regs + 5, 0);
    U_VPUSHIDXI(vm, regs + 6);
    regs[5] = locals[20]; // start
    U_INE(vm, regs + 6);
    if (!U_JUMPFAIL(vm, regs + 5)) goto block42056;
    U_STATEMENT(vm, regs + 4, 82, 19);
    U_PUSHVARF(vm, regs + 4, 773); // shape
    U_PUSHINT(vm, regs + 5, 0);
    U_PUSHVARF(vm, regs + 6, 773); // shape
    U_BCALLRETV(vm, regs + 7, 16, 1); // pop
    U_PUSHINT(vm, regs + 7, 1);
    U_BCALLRETV(vm, regs + 8, 18, 1); // insert
    U_POP(vm, regs + 5);
    goto block42009;
    block42056:;
    U_STATEMENT(vm, regs + 4, 83, 19);
    U_PUSHVARF(vm, regs + 4, 773); // shape
    U_BCALLRET1(vm, regs + 5, 13, 1); // length
    U_PUSHINT(vm, regs + 5, 4);
    U_IEQ(vm, regs + 6);
    if (!U_JUMPFAIL(vm, regs + 5)) goto block42125;
    U_STATEMENT(vm, regs + 4, 84, 19);
    regs[4] = locals[1]; // quads
    U_INCREF(vm, regs + 5, 0);
    U_PUSHFUN(vm, regs + 5, 0, fun_362);
    fun_596(vm, regs + 6); // call: exists
    U_LOGNOT(vm, regs + 5);
    if (!U_JUMPFAIL(vm, regs + 5)) goto block42120;
    U_STATEMENT(vm, regs + 4, 85, 19);
    regs[4] = locals[1]; // quads
    U_PUSHVARF(vm, regs + 5, 773); // shape
    U_INCREF(vm, regs + 6, 0);
    U_PUSHINT(vm, regs + 6, 1);
    U_BCALLRETV(vm, regs + 7, 15, 1); // push
    U_POP(vm, regs + 5);
    block42120:;
    goto block42190;
    block42125:;
    U_STATEMENT(vm, regs + 4, 86, 19);
    U_PUSHVARF(vm, regs + 4, 773); // shape
    U_BCALLRET1(vm, regs + 5, 13, 1); // length
    U_PUSHINT(vm, regs + 5, 3);
    U_IEQ(vm, regs + 6);
    if (!U_JUMPFAIL(vm, regs + 5)) goto block42188;
    U_STATEMENT(vm, regs + 4, 87, 19);
    regs[4] = locals[15]; // tris
    U_PUSHFUN(vm, regs + 5, 0, fun_363);
    fun_597(vm, regs + 6); // call: exists
    U_LOGNOT(vm, regs + 5);
    if (!U_JUMPFAIL(vm, regs + 5)) goto block42186;
    U_STATEMENT(vm, regs + 4, 88, 19);
    regs[4] = locals[15]; // tris
    U_PUSHVARF(vm, regs + 5, 773); // shape
    U_INCREF(vm, regs + 6, 0);
    U_PUSHINT(vm, regs + 6, 1);
    U_BCALLRETV(vm, regs + 7, 15, 1); // push
    U_POP(vm, regs + 5);
    block42186:;
    block42188:;
    block42190:;
    block42192:;
    goto block41756;
    block42197:;
    goto block41725;
    block42202:;
    U_STATEMENT(vm, regs + 0, 89, 19);
    block42208:;
    regs[0] = locals[15]; // tris
    U_BCALLRET1(vm, regs + 1, 13, 1); // length
    if (!U_JUMPFAIL(vm, regs + 1)) goto block42951;
    U_STATEMENT(vm, regs + 0, 90, 19);
    U_PUSHINT(vm, regs + 0, -1);
    SetLVal(vm, &locals[21]); // besti
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 91, 19);
    U_PUSHINT(vm, regs + 0, -1);
    SetLVal(vm, &locals[22]); // bestqc
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 92, 19);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[15]; // tris
    block42254:;
    if (!U_VFOR(vm, regs + 2)) goto block42456;
    U_STATEMENT(vm, regs + 2, 92, 19);
    U_VFORELEMREF(vm, regs + 2);
    SetLVal(vm, &locals[23]); // t
    U_LV_WRITEREF(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 92, 19);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[24]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 93, 19);
    U_PUSHINT(vm, regs + 2, 0);
    SetLVal(vm, &locals[25]); // qc
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 94, 19);
    U_PUSHINT(vm, regs + 2, -1);
    regs[3] = locals[23]; // t
    block42303:;
    if (!U_VFOR(vm, regs + 4)) goto block42410;
    U_STATEMENT(vm, regs + 4, 94, 19);
    U_VFORELEM(vm, regs + 4);
    SetLVal(vm, &locals[26]); // vi1
    U_LV_WRITE(vm, regs + 5);
    U_STATEMENT(vm, regs + 4, 95, 19);
    U_PUSHINT(vm, regs + 4, -1);
    regs[5] = locals[1]; // quads
    block42329:;
    if (!U_VFOR(vm, regs + 6)) goto block42405;
    U_STATEMENT(vm, regs + 6, 95, 19);
    U_VFORELEMREF(vm, regs + 6);
    SetLVal(vm, &locals[27]); // q
    U_LV_WRITEREF(vm, regs + 7);
    U_STATEMENT(vm, regs + 6, 96, 19);
    U_PUSHINT(vm, regs + 6, -1);
    regs[7] = locals[27]; // q
    block42355:;
    if (!U_VFOR(vm, regs + 8)) goto block42400;
    U_STATEMENT(vm, regs + 8, 96, 19);
    U_VFORELEM(vm, regs + 8);
    SetLVal(vm, &locals[28]); // vi2
    U_LV_WRITE(vm, regs + 9);
    U_STATEMENT(vm, regs + 8, 97, 19);
    regs[8] = locals[26]; // vi1
    regs[9] = locals[28]; // vi2
    U_IEQ(vm, regs + 10);
    if (!U_JUMPFAIL(vm, regs + 9)) goto block42395;
    U_STATEMENT(vm, regs + 8, 98, 19);
    SetLVal(vm, &locals[25]); // qc
    U_LV_IPP(vm, regs + 8);
    block42395:;
    goto block42355;
    block42400:;
    goto block42329;
    block42405:;
    goto block42303;
    block42410:;
    U_STATEMENT(vm, regs + 2, 99, 19);
    regs[2] = locals[25]; // qc
    regs[3] = locals[22]; // bestqc
    U_IGT(vm, regs + 4);
    if (!U_JUMPFAIL(vm, regs + 3)) goto block42451;
    U_STATEMENT(vm, regs + 2, 100, 19);
    regs[2] = locals[25]; // qc
    SetLVal(vm, &locals[22]); // bestqc
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 101, 19);
    regs[2] = locals[24]; // i
    SetLVal(vm, &locals[21]); // besti
    U_LV_WRITE(vm, regs + 3);
    block42451:;
    goto block42254;
    block42456:;
    U_STATEMENT(vm, regs + 0, 102, 19);
    regs[0] = locals[15]; // tris
    regs[1] = locals[21]; // besti
    U_BCALLRETV(vm, regs + 2, 19, 1); // remove
    SetLVal(vm, &locals[29]); // t1
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 103, 19);
    U_PUSHINT(vm, regs + 0, 0);
    SetLVal(vm, &locals[30]); // found
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 104, 19);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[29]; // t1
    block42499:;
    if (!U_VFOR(vm, regs + 2)) goto block42946;
    U_STATEMENT(vm, regs + 2, 104, 19);
    U_VFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[31]); // vi
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 104, 19);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[32]); // vii
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 105, 19);
    regs[2] = locals[30]; // found
    U_LOGNOT(vm, regs + 3);
    if (!U_JUMPFAIL(vm, regs + 3)) goto block42941;
    U_STATEMENT(vm, regs + 2, 106, 19);
    regs[2] = locals[29]; // t1
    regs[3] = locals[32]; // vii
    U_PUSHINT(vm, regs + 4, 1);
    U_IADD(vm, regs + 5);
    U_PUSHINT(vm, regs + 4, 3);
    U_IMOD(vm, regs + 5);
    U_VPUSHIDXI(vm, regs + 4);
    SetLVal(vm, &locals[33]); // ni
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 107, 19);
    regs[2] = locals[29]; // t1
    regs[3] = locals[32]; // vii
    U_PUSHINT(vm, regs + 4, 2);
    U_IADD(vm, regs + 5);
    U_PUSHINT(vm, regs + 4, 3);
    U_IMOD(vm, regs + 5);
    U_VPUSHIDXI(vm, regs + 4);
    SetLVal(vm, &locals[34]); // li
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 108, 19);
    U_PUSHINT(vm, regs + 2, -1);
    regs[3] = locals[15]; // tris
    block42602:;
    if (!U_VFOR(vm, regs + 4)) goto block42939;
    U_STATEMENT(vm, regs + 4, 108, 19);
    U_VFORELEMREF(vm, regs + 4);
    SetLVal(vm, &locals[35]); // t
    U_LV_WRITEREF(vm, regs + 5);
    U_STATEMENT(vm, regs + 4, 109, 19);
    regs[4] = locals[30]; // found
    U_LOGNOT(vm, regs + 5);
    if (!U_JUMPFAIL(vm, regs + 5)) goto block42934;
    U_STATEMENT(vm, regs + 4, 110, 19);
    U_PUSHINT(vm, regs + 4, 1);
    SetLVal(vm, &locals[30]); // found
    U_LV_WRITE(vm, regs + 5);
    U_STATEMENT(vm, regs + 4, 111, 19);
    regs[4] = locals[35]; // t
    U_PUSHINT(vm, regs + 5, 0);
    U_VPUSHIDXI(vm, regs + 6);
    regs[5] = locals[33]; // ni
    U_IEQ(vm, regs + 6);
    if (!U_JUMPFAILR(vm, regs + 5)) goto block42677;
    U_POP(vm, regs + 5);
    regs[4] = locals[35]; // t
    U_PUSHINT(vm, regs + 5, 1);
    U_VPUSHIDXI(vm, regs + 6);
    regs[5] = locals[31]; // vi
    U_IEQ(vm, regs + 6);
    block42677:;
    if (!U_JUMPFAIL(vm, regs + 5)) goto block42722;
    U_STATEMENT(vm, regs + 4, 112, 19);
    regs[4] = locals[1]; // quads
    regs[5] = locals[31]; // vi
    regs[6] = locals[35]; // t
    U_PUSHINT(vm, regs + 7, 2);
    U_VPUSHIDXI(vm, regs + 8);
    regs[7] = locals[33]; // ni
    regs[8] = locals[34]; // li
    U_NEWVEC(vm, regs + 9, 7, 4);
    U_PUSHINT(vm, regs + 6, 1);
    U_BCALLRETV(vm, regs + 7, 15, 1); // push
    U_POP(vm, regs + 5);
    goto block42904;
    block42722:;
    U_STATEMENT(vm, regs + 4, 113, 19);
    regs[4] = locals[35]; // t
    U_PUSHINT(vm, regs + 5, 1);
    U_VPUSHIDXI(vm, regs + 6);
    regs[5] = locals[33]; // ni
    U_IEQ(vm, regs + 6);
    if (!U_JUMPFAILR(vm, regs + 5)) goto block42759;
    U_POP(vm, regs + 5);
    regs[4] = locals[35]; // t
    U_PUSHINT(vm, regs + 5, 2);
    U_VPUSHIDXI(vm, regs + 6);
    regs[5] = locals[31]; // vi
    U_IEQ(vm, regs + 6);
    block42759:;
    if (!U_JUMPFAIL(vm, regs + 5)) goto block42804;
    U_STATEMENT(vm, regs + 4, 114, 19);
    regs[4] = locals[1]; // quads
    regs[5] = locals[31]; // vi
    regs[6] = locals[35]; // t
    U_PUSHINT(vm, regs + 7, 0);
    U_VPUSHIDXI(vm, regs + 8);
    regs[7] = locals[33]; // ni
    regs[8] = locals[34]; // li
    U_NEWVEC(vm, regs + 9, 7, 4);
    U_PUSHINT(vm, regs + 6, 1);
    U_BCALLRETV(vm, regs + 7, 15, 1); // push
    U_POP(vm, regs + 5);
    goto block42902;
    block42804:;
    U_STATEMENT(vm, regs + 4, 115, 19);
    regs[4] = locals[35]; // t
    U_PUSHINT(vm, regs + 5, 2);
    U_VPUSHIDXI(vm, regs + 6);
    regs[5] = locals[33]; // ni
    U_IEQ(vm, regs + 6);
    if (!U_JUMPFAILR(vm, regs + 5)) goto block42841;
    U_POP(vm, regs + 5);
    regs[4] = locals[35]; // t
    U_PUSHINT(vm, regs + 5, 0);
    U_VPUSHIDXI(vm, regs + 6);
    regs[5] = locals[31]; // vi
    U_IEQ(vm, regs + 6);
    block42841:;
    if (!U_JUMPFAIL(vm, regs + 5)) goto block42886;
    U_STATEMENT(vm, regs + 4, 116, 19);
    regs[4] = locals[1]; // quads
    regs[5] = locals[31]; // vi
    regs[6] = locals[35]; // t
    U_PUSHINT(vm, regs + 7, 1);
    U_VPUSHIDXI(vm, regs + 8);
    regs[7] = locals[33]; // ni
    regs[8] = locals[34]; // li
    U_NEWVEC(vm, regs + 9, 7, 4);
    U_PUSHINT(vm, regs + 6, 1);
    U_BCALLRETV(vm, regs + 7, 15, 1); // push
    U_POP(vm, regs + 5);
    goto block42900;
    block42886:;
    U_STATEMENT(vm, regs + 4, 118, 19);
    U_PUSHINT(vm, regs + 4, 0);
    SetLVal(vm, &locals[30]); // found
    U_LV_WRITE(vm, regs + 5);
    block42900:;
    block42902:;
    block42904:;
    U_STATEMENT(vm, regs + 4, 119, 19);
    regs[4] = locals[30]; // found
    if (!U_JUMPFAIL(vm, regs + 5)) goto block42932;
    U_STATEMENT(vm, regs + 4, 120, 19);
    regs[4] = locals[15]; // tris
    regs[5] = locals[35]; // t
    U_BCALLRET2(vm, regs + 6, 21, 1); // remove_obj
    U_POP(vm, regs + 5);
    block42932:;
    block42934:;
    goto block42602;
    block42939:;
    block42941:;
    goto block42499;
    block42946:;
    goto block42208;
    block42951:;
    U_RETURNLOCAL(vm, 0, 0);
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
static void fun_353(VMRef vm, StackPtr psp) {
    Value regs[8];
    Value locals[13];
    locals[0] = lobster::NilVal();
    locals[1] = lobster::NilVal();
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    locals[5] = lobster::NilVal();
    locals[6] = lobster::NilVal();
    locals[7] = lobster::NilVal();
    locals[8] = lobster::NilVal();
    locals[9] = lobster::NilVal();
    locals[10] = lobster::NilVal();
    locals[11] = lobster::NilVal();
    locals[12] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 3058, locals);
    U_STATEMENT(vm, regs + 0, 23, 19);
    U_PUSHVARF(vm, regs + 0, 727); // verts
    U_INCREF(vm, regs + 1, 0);
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_584(vm, regs + 2); // call: map
    SetLVal(vm, &locals[0]); // disps
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 24, 19);
    U_PUSHINT(vm, regs + 0, -1);
    U_PUSHVARF(vm, regs + 1, 727); // verts
    block43009:;
    if (!U_VFOR(vm, regs + 2)) goto block43190;
    U_STATEMENT(vm, regs + 2, 24, 19);
    U_VFORELEM2S(vm, regs + 2);
    SetLVal(vm, &locals[1]); // v+0
    U_LV_WRITEV(vm, regs + 4, 2);
    U_STATEMENT(vm, regs + 2, 25, 19);
    U_PUSHINT(vm, regs + 2, -1);
    U_PUSHVARF(vm, regs + 3, 727); // verts
    block43036:;
    if (!U_VFOR(vm, regs + 4)) goto block43185;
    U_STATEMENT(vm, regs + 4, 25, 19);
    U_VFORELEM2S(vm, regs + 4);
    SetLVal(vm, &locals[3]); // w+0
    U_LV_WRITEV(vm, regs + 6, 2);
    U_STATEMENT(vm, regs + 4, 25, 19);
    U_FORLOOPI(vm, regs + 4);
    SetLVal(vm, &locals[5]); // i
    U_LV_WRITE(vm, regs + 5);
    U_STATEMENT(vm, regs + 4, 26, 19);
    regs[4] = locals[3];regs[5] = locals[4]; // w
    regs[6] = locals[1];regs[7] = locals[2]; // v
    U_FVVSUB(vm, regs + 8, 2);
    SetLVal(vm, &locals[6]); // tow+0
    U_LV_WRITEV(vm, regs + 6, 2);
    U_STATEMENT(vm, regs + 4, 27, 19);
    regs[4] = locals[6];regs[5] = locals[7]; // tow
    U_PUSHINT(vm, regs + 6, 2);
    U_BCALLRETV(vm, regs + 7, 86, 1); // magnitude
    SetLVal(vm, &locals[8]); // dist
    U_LV_WRITE(vm, regs + 5);
    U_STATEMENT(vm, regs + 4, 28, 19);
    regs[4] = locals[8]; // dist
    U_PUSHFLT(vm, regs + 5, 1065353216);
    U_FLT(vm, regs + 6);
    if (!U_JUMPFAILR(vm, regs + 5)) goto block43133;
    U_POP(vm, regs + 5);
    regs[4] = locals[1];regs[5] = locals[2]; // v
    regs[6] = locals[3];regs[7] = locals[4]; // w
    U_STNE(vm, regs + 8, 2);
    block43133:;
    if (!U_JUMPFAIL(vm, regs + 5)) goto block43180;
    U_STATEMENT(vm, regs + 4, 29, 19);
    regs[4] = locals[6];regs[5] = locals[7]; // tow
    U_PUSHFLT(vm, regs + 6, 1065353216);
    regs[7] = locals[8]; // dist
    U_FDIV(vm, regs + 8);
    U_FVSMUL(vm, regs + 7, 2);
    U_PUSHFLT(vm, regs + 6, 1065353216);
    regs[7] = locals[8]; // dist
    U_FSUB(vm, regs + 8);
    U_FVSMUL(vm, regs + 7, 2);
    regs[6] = locals[0]; // disps
    regs[7] = locals[5]; // i
    U_LVAL_IDXVI(vm, regs + 8, 0);
    U_LV_FVVADD(vm, regs + 6, 2);
    block43180:;
    goto block43036;
    block43185:;
    goto block43009;
    block43190:;
    U_STATEMENT(vm, regs + 0, 30, 19);
    U_PUSHFLT(vm, regs + 0, 0);
    SetLVal(vm, &locals[9]); // totaldisp
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 31, 19);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[0]; // disps
    block43214:;
    if (!U_VFOR(vm, regs + 2)) goto block43285;
    U_STATEMENT(vm, regs + 2, 31, 19);
    U_VFORELEM2S(vm, regs + 2);
    SetLVal(vm, &locals[10]); // d+0
    U_LV_WRITEV(vm, regs + 4, 2);
    U_STATEMENT(vm, regs + 2, 31, 19);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[12]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 32, 19);
    regs[2] = locals[10];regs[3] = locals[11]; // d
    U_PUSHVARF(vm, regs + 4, 727); // verts
    regs[5] = locals[12]; // i
    U_LVAL_IDXVI(vm, regs + 6, 0);
    U_LV_FVVADD(vm, regs + 4, 2);
    U_STATEMENT(vm, regs + 2, 33, 19);
    regs[2] = locals[10];regs[3] = locals[11]; // d
    U_PUSHINT(vm, regs + 4, 2);
    U_BCALLRETV(vm, regs + 5, 86, 1); // magnitude
    SetLVal(vm, &locals[9]); // totaldisp
    U_LV_FADD(vm, regs + 3);
    goto block43214;
    block43285:;
    U_STATEMENT(vm, regs + 0, 34, 19);
    regs[0] = locals[9]; // totaldisp
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function322
static void fun_355(VMRef vm, StackPtr psp) {
    Value regs[6];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3076, locals);
    U_STATEMENT(vm, regs + 0, 40, 19);
    U_PUSHVARF(vm, regs + 0, 727); // verts
    regs[1] = locals[0];regs[2] = locals[1]; // p
    U_PUSHINT(vm, regs + 3, 2);
    U_BCALLRETV(vm, regs + 4, 68, 1); // float
    fun_89(vm, regs + 3); // call: rnd_float2
    U_FVVADD(vm, regs + 5, 2);
    U_PUSHVARVF(vm, regs + 3, 743, 2); // wsize+0
    U_PUSHINT(vm, regs + 5, 2);
    U_BCALLRETV(vm, regs + 6, 68, 1); // float
    U_PUSHFLT(vm, regs + 5, 1073741824);
    U_FVSDIV(vm, regs + 6, 2);
    U_FVVSUB(vm, regs + 5, 2);
    U_PUSHINT(vm, regs + 3, 2);
    U_BCALLRETV(vm, regs + 4, 15, 1); // push
    U_POP(vm, regs + 1);
    U_RETURNLOCAL(vm, 0, 0);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function323
static void fun_356(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[6];
    locals[0] = *(psp - 6);
    locals[1] = *(psp - 5);
    locals[2] = *(psp - 4);
    locals[3] = *(psp - 3);
    locals[4] = *(psp - 2);
    locals[5] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3083, locals);
    U_STATEMENT(vm, regs + 0, 53, 19);
    regs[0] = locals[2]; // a+2
    regs[1] = locals[5]; // b+2
    U_FLT(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function325
static void fun_358(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 2);
    SwapVars(vm, 767, psp, 1);
    PushFunId(vm, funinfo_table + 3094, locals);
    U_STATEMENT(vm, regs + 0, 63, 19);
    regs[0] = locals[0]; // nbl
    U_INCREF(vm, regs + 1, 0);
    U_PUSHFUN(vm, regs + 1, 0, fun_359);
    fun_591(vm, regs + 2); // call: qsort
    U_RETURNLOCAL(vm, 0, 1);
    psp = PopArg(vm, 767, psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function326
static void fun_359(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3101, locals);
    U_STATEMENT(vm, regs + 0, 64, 19);
    U_PUSHVARF(vm, regs + 0, 727); // verts
    regs[1] = locals[0]; // a
    U_VPUSHIDXI2V(vm, regs + 2);
    U_PUSHVARF(vm, regs + 2, 727); // verts
    U_PUSHVARF(vm, regs + 3, 767); // i
    U_VPUSHIDXI2V(vm, regs + 4);
    U_FVVSUB(vm, regs + 4, 2);
    U_PUSHINT(vm, regs + 2, 2);
    U_BCALLRETV(vm, regs + 3, 81, 1); // atan2
    U_PUSHVARF(vm, regs + 1, 727); // verts
    regs[2] = locals[1]; // b
    U_VPUSHIDXI2V(vm, regs + 3);
    U_PUSHVARF(vm, regs + 3, 727); // verts
    U_PUSHVARF(vm, regs + 4, 767); // i
    U_VPUSHIDXI2V(vm, regs + 5);
    U_FVVSUB(vm, regs + 5, 2);
    U_PUSHINT(vm, regs + 3, 2);
    U_BCALLRETV(vm, regs + 4, 81, 1); // atan2
    U_FLT(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function327
static void fun_360(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3108, locals);
    U_STATEMENT(vm, regs + 0, 74, 19);
    regs[0] = locals[0]; // _
    U_PUSHVARF(vm, regs + 1, 774); // p
    U_IEQ(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function328
static void fun_361(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3114, locals);
    U_STATEMENT(vm, regs + 0, 80, 19);
    regs[0] = locals[0]; // _a
    regs[1] = locals[1]; // _b
    U_BCALLRET2(vm, regs + 2, 119, 1); // min
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function329
static void fun_362(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3121, locals);
    U_STATEMENT(vm, regs + 0, 84, 19);
    regs[0] = locals[0]; // _
    U_PUSHVARF(vm, regs + 1, 773); // shape
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function330
static void fun_363(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3127, locals);
    U_STATEMENT(vm, regs + 0, 87, 19);
    regs[0] = locals[0]; // _
    U_PUSHVARF(vm, regs + 1, 773); // shape
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function334
static void fun_367(VMRef vm, StackPtr psp) {
    Value regs[19];
    Value locals[29];
    locals[0] = lobster::NilVal();
    locals[1] = lobster::NilVal();
    BackupVar(vm, 946);
    BackupVar(vm, 1015);
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    locals[5] = lobster::NilVal();
    locals[6] = lobster::NilVal();
    locals[7] = lobster::NilVal();
    locals[8] = lobster::NilVal();
    locals[9] = lobster::NilVal();
    locals[10] = lobster::NilVal();
    locals[11] = lobster::NilVal();
    locals[12] = lobster::NilVal();
    locals[13] = lobster::NilVal();
    locals[14] = lobster::NilVal();
    locals[15] = lobster::NilVal();
    BackupVar(vm, 1030);
    locals[16] = lobster::NilVal();
    locals[17] = lobster::NilVal();
    locals[18] = lobster::NilVal();
    locals[19] = lobster::NilVal();
    locals[20] = lobster::NilVal();
    locals[21] = lobster::NilVal();
    locals[22] = lobster::NilVal();
    locals[23] = lobster::NilVal();
    locals[24] = lobster::NilVal();
    locals[25] = lobster::NilVal();
    locals[26] = lobster::NilVal();
    locals[27] = lobster::NilVal();
    locals[28] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 3133, locals);
    U_STATEMENT(vm, regs + 0, 9, 20);
    U_STATEMENT(vm, regs + 0, 9, 20);
    U_STATEMENT(vm, regs + 0, 13, 20);
    U_STATEMENT(vm, regs + 0, 18, 20);
    U_STATEMENT(vm, regs + 0, 18, 20);
    U_STATEMENT(vm, regs + 0, 25, 20);
    U_STATEMENT(vm, regs + 0, 41, 20);
    U_PUSHFLT(vm, regs + 0, 1148846080);
    SetLVal(vm, &locals[0]); // bigrad
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 42, 20);
    U_PUSHFLT(vm, regs + 0, 1120403456);
    SetLVal(vm, &locals[1]); // lrad
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 44, 20);
    regs[0] = locals[1]; // lrad
    U_PUSHFLT(vm, regs + 1, 1112014848);
    regs[2] = locals[1]; // lrad
    U_PUSHFLT64(vm, regs + 3, 1717986918, 1079273062);
    U_FADD(vm, regs + 4);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    U_FSUB(vm, regs + 4);
    U_PUSHFLT64(vm, regs + 3, 1717986918, 1079273062);
    U_PUSHVARVF(vm, regs + 4, 87, 3); // float3_1+0
    U_PUSHFLT(vm, regs + 7, 1094713344);
    U_FVSMUL(vm, regs + 8, 3);
    U_PUSHVARVF(vm, regs + 7, 84, 3); // float3_0+0
    U_PUSHINT(vm, regs + 10, 0);
    U_NEWOBJECT(vm, regs + 11, 1185); // Sphere
    U_PUSHFLT(vm, regs + 1, 1099169792);
    U_PUSHFLT(vm, regs + 2, 1116864512);
    U_PUSHFLT(vm, regs + 3, 1099169792);
    U_PUSHFLT(vm, regs + 4, 1117519872);
    U_PUSHVARVF(vm, regs + 5, 84, 3); // float3_0+0
    U_PUSHVARVF(vm, regs + 8, 87, 3); // float3_1+0
    U_PUSHFLT64(vm, regs + 11, -652835029, 1072691150);
    U_FVSMUL(vm, regs + 12, 3);
    U_PUSHINT(vm, regs + 11, 2);
    U_NEWOBJECT(vm, regs + 12, 1185); // Sphere
    U_PUSHFLT(vm, regs + 2, 1099169792);
    U_PUSHFLT(vm, regs + 3, 1104674816);
    U_PUSHFLT(vm, regs + 4, 1099169792);
    U_PUSHFLT(vm, regs + 5, 1111228416);
    U_PUSHVARVF(vm, regs + 6, 84, 3); // float3_0+0
    U_PUSHVARVF(vm, regs + 9, 87, 3); // float3_1+0
    U_PUSHFLT64(vm, regs + 12, -652835029, 1072691150);
    U_FVSMUL(vm, regs + 13, 3);
    U_PUSHINT(vm, regs + 12, 1);
    U_NEWOBJECT(vm, regs + 13, 1185); // Sphere
    regs[3] = locals[0]; // bigrad
    U_PUSHFLT(vm, regs + 4, 1112014848);
    regs[5] = locals[0]; // bigrad
    U_FUMINUS(vm, regs + 6);
    U_PUSHFLT64(vm, regs + 6, 1717986918, 1079273062);
    U_FADD(vm, regs + 7);
    U_PUSHFLT64(vm, regs + 6, 1717986918, 1079273062);
    U_PUSHVARVF(vm, regs + 7, 84, 3); // float3_0+0
    U_PUSHVARVF(vm, regs + 10, 87, 3); // float3_1+0
    U_PUSHFLT(vm, regs + 13, 1061158912);
    U_FVSMUL(vm, regs + 14, 3);
    U_PUSHINT(vm, regs + 13, 0);
    U_NEWOBJECT(vm, regs + 14, 1185); // Sphere
    regs[4] = locals[0]; // bigrad
    U_PUSHFLT(vm, regs + 5, 1112014848);
    regs[6] = locals[0]; // bigrad
    U_PUSHFLT64(vm, regs + 7, 1717986918, 1079273062);
    U_PUSHVARVF(vm, regs + 8, 84, 3); // float3_0+0
    U_PUSHVARVF(vm, regs + 11, 87, 3); // float3_1+0
    U_PUSHFLT(vm, regs + 14, 1061158912);
    U_FVSMUL(vm, regs + 15, 3);
    U_PUSHINT(vm, regs + 14, 0);
    U_NEWOBJECT(vm, regs + 15, 1185); // Sphere
    regs[5] = locals[0]; // bigrad
    U_PUSHFLT(vm, regs + 6, 1112014848);
    U_PUSHFLT64(vm, regs + 7, 1717986918, 1078224486);
    regs[8] = locals[0]; // bigrad
    U_FUMINUS(vm, regs + 9);
    U_PUSHFLT(vm, regs + 9, 1126825984);
    U_FADD(vm, regs + 10);
    U_PUSHVARVF(vm, regs + 9, 84, 3); // float3_0+0
    U_PUSHVARVF(vm, regs + 12, 84, 3); // float3_0+0
    U_PUSHINT(vm, regs + 15, 0);
    U_NEWOBJECT(vm, regs + 16, 1185); // Sphere
    regs[6] = locals[0]; // bigrad
    U_PUSHFLT(vm, regs + 7, 1112014848);
    U_PUSHFLT64(vm, regs + 8, 1717986918, 1078224486);
    regs[9] = locals[0]; // bigrad
    U_PUSHVARVF(vm, regs + 10, 84, 3); // float3_0+0
    U_PUSHVARVF(vm, regs + 13, 87, 3); // float3_1+0
    U_PUSHFLT(vm, regs + 16, 1061158912);
    U_FVSMUL(vm, regs + 17, 3);
    U_PUSHINT(vm, regs + 16, 0);
    U_NEWOBJECT(vm, regs + 17, 1185); // Sphere
    regs[7] = locals[0]; // bigrad
    regs[8] = locals[0]; // bigrad
    U_FUMINUS(vm, regs + 9);
    U_PUSHFLT(vm, regs + 9, 1120272384);
    U_FADD(vm, regs + 10);
    U_PUSHFLT64(vm, regs + 9, 1717986918, 1078224486);
    U_PUSHFLT64(vm, regs + 10, 1717986918, 1079273062);
    U_PUSHVARVF(vm, regs + 11, 84, 3); // float3_0+0
    U_PUSHFLT(vm, regs + 14, 1048576000);
    U_PUSHFLT(vm, regs + 15, 1048576000);
    U_PUSHFLT(vm, regs + 16, 1061158912);
    U_PUSHINT(vm, regs + 17, 0);
    U_NEWOBJECT(vm, regs + 18, 1185); // Sphere
    regs[8] = locals[0]; // bigrad
    regs[9] = locals[0]; // bigrad
    U_PUSHFLT(vm, regs + 10, 1065353216);
    U_FADD(vm, regs + 11);
    U_PUSHFLT64(vm, regs + 10, 1717986918, 1078224486);
    U_PUSHFLT64(vm, regs + 11, 1717986918, 1079273062);
    U_PUSHVARVF(vm, regs + 12, 84, 3); // float3_0+0
    U_PUSHFLT(vm, regs + 15, 1061158912);
    U_PUSHFLT(vm, regs + 16, 1048576000);
    U_PUSHFLT(vm, regs + 17, 1048576000);
    U_PUSHINT(vm, regs + 18, 0);
    U_NEWOBJECT(vm, regs + 19, 1185); // Sphere
    U_NEWVEC(vm, regs + 9, 1250, 9);
    U_LVAL_VARF(vm, regs + 1, 946); // spheres
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 56, 20);
    U_STATEMENT(vm, regs + 0, 112, 20);
    U_PUSHINT(vm, regs + 0, 32);
    U_LVAL_VARF(vm, regs + 1, 1015); // w
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 113, 20);
    U_PUSHINT(vm, regs + 0, 24);
    SetLVal(vm, &locals[2]); // h
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 115, 20);
    U_PUSHFLT(vm, regs + 0, 1112014848);
    U_PUSHFLT(vm, regs + 1, 1112014848);
    U_PUSHFLT(vm, regs + 2, 1133576192);
    U_PUSHFLT(vm, regs + 3, 0);
    U_PUSHFLT64(vm, regs + 4, 1960154354, -1079652035);
    U_PUSHFLT(vm, regs + 5, -1082130432);
    U_PUSHINT(vm, regs + 6, 3);
    U_BCALLRETV(vm, regs + 7, 84, 1); // normalize
    SetLVal(vm, &locals[3]); // cam+0
    U_LV_WRITEV(vm, regs + 6, 6);
    U_STATEMENT(vm, regs + 0, 116, 20);
    U_PUSHVARVF(vm, regs + 0, 90, 3); // float3_x+0
    U_PUSHVARF(vm, regs + 3, 1015); // w
    U_I2F(vm, regs + 4);
    U_PUSHFLT64(vm, regs + 4, -1924145349, 1071672983);
    U_FMUL(vm, regs + 5);
    regs[4] = locals[2]; // h
    U_I2F(vm, regs + 5);
    U_FDIV(vm, regs + 5);
    U_FVSMUL(vm, regs + 4, 3);
    SetLVal(vm, &locals[9]); // cx+0
    U_LV_WRITEV(vm, regs + 3, 3);
    U_STATEMENT(vm, regs + 0, 117, 20);
    regs[0] = locals[9];regs[1] = locals[10];regs[2] = locals[11]; // cx
    U_PUSHINT(vm, regs + 3, 3);
    regs[4] = locals[6];regs[5] = locals[7];regs[6] = locals[8]; // cam
    U_PUSHINT(vm, regs + 7, 3);
    U_BCALLRETV(vm, regs + 8, 90, 1); // cross
    U_PUSHINT(vm, regs + 3, 3);
    U_BCALLRETV(vm, regs + 4, 84, 1); // normalize
    U_PUSHFLT64(vm, regs + 3, -1924145349, 1071672983);
    U_FVSMUL(vm, regs + 4, 3);
    SetLVal(vm, &locals[12]); // cy+0
    U_LV_WRITEV(vm, regs + 3, 3);
    U_STATEMENT(vm, regs + 0, 119, 20);
    regs[0] = locals[2]; // h
    U_PUSHFUN(vm, regs + 1, 0, fun_370);
    fun_598(vm, regs + 2); // call: map
    SetLVal(vm, &locals[15]); // c
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 121, 20);
    U_PUSHINT(vm, regs + 0, 0);
    U_LVAL_VARF(vm, regs + 1, 1030); // samples
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 123, 20);
    U_PUSHINT(vm, regs + 0, 0);
    U_BCALLRET1(vm, regs + 1, 97, 0); // rnd_seed
    U_STATEMENT(vm, regs + 0, 125, 20);
    U_PUSHINT(vm, regs + 0, -1);
    regs[1] = locals[2]; // h
    block44220:;
    if (!U_IFOR(vm, regs + 2)) goto block44625;
    U_STATEMENT(vm, regs + 2, 125, 20);
    U_IFORELEM(vm, regs + 2);
    SetLVal(vm, &locals[16]); // y
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 126, 20);
    U_PUSHINT(vm, regs + 2, -1);
    U_PUSHVARF(vm, regs + 3, 1015); // w
    block44246:;
    if (!U_IFOR(vm, regs + 4)) goto block44620;
    U_STATEMENT(vm, regs + 4, 126, 20);
    U_IFORELEM(vm, regs + 4);
    SetLVal(vm, &locals[17]); // x
    U_LV_WRITE(vm, regs + 5);
    U_STATEMENT(vm, regs + 4, 127, 20);
    U_PUSHFLT(vm, regs + 4, 1073741824);
    U_BCALLRET0(vm, regs + 5, 95, 1); // rnd_float
    U_FMUL(vm, regs + 6);
    SetLVal(vm, &locals[18]); // r1
    U_LV_WRITE(vm, regs + 5);
    U_STATEMENT(vm, regs + 4, 128, 20);
    regs[4] = locals[18]; // r1
    U_PUSHFLT(vm, regs + 5, 1065353216);
    U_FLT(vm, regs + 6);
    if (!U_JUMPFAIL(vm, regs + 5)) goto block44314;
    U_STATEMENT(vm, regs + 4, 128, 20);
    regs[4] = locals[18]; // r1
    U_BCALLRET1(vm, regs + 5, 56, 1); // sqrt
    U_PUSHFLT(vm, regs + 5, 1065353216);
    U_FSUB(vm, regs + 6);
    goto block44337;
    block44314:;
    U_STATEMENT(vm, regs + 4, 128, 20);
    U_PUSHFLT(vm, regs + 4, 1065353216);
    U_PUSHFLT(vm, regs + 5, 1073741824);
    regs[6] = locals[18]; // r1
    U_FSUB(vm, regs + 7);
    U_BCALLRET1(vm, regs + 6, 56, 1); // sqrt
    U_FSUB(vm, regs + 6);
    block44337:;
    SetLVal(vm, &locals[19]); // dx
    U_LV_WRITE(vm, regs + 5);
    U_STATEMENT(vm, regs + 4, 129, 20);
    U_PUSHFLT(vm, regs + 4, 1073741824);
    U_BCALLRET0(vm, regs + 5, 95, 1); // rnd_float
    U_FMUL(vm, regs + 6);
    SetLVal(vm, &locals[20]); // r2
    U_LV_WRITE(vm, regs + 5);
    U_STATEMENT(vm, regs + 4, 130, 20);
    regs[4] = locals[20]; // r2
    U_PUSHFLT(vm, regs + 5, 1065353216);
    U_FLT(vm, regs + 6);
    if (!U_JUMPFAIL(vm, regs + 5)) goto block44396;
    U_STATEMENT(vm, regs + 4, 130, 20);
    regs[4] = locals[20]; // r2
    U_BCALLRET1(vm, regs + 5, 56, 1); // sqrt
    U_PUSHFLT(vm, regs + 5, 1065353216);
    U_FSUB(vm, regs + 6);
    goto block44419;
    block44396:;
    U_STATEMENT(vm, regs + 4, 130, 20);
    U_PUSHFLT(vm, regs + 4, 1065353216);
    U_PUSHFLT(vm, regs + 5, 1073741824);
    regs[6] = locals[20]; // r2
    U_FSUB(vm, regs + 7);
    U_BCALLRET1(vm, regs + 6, 56, 1); // sqrt
    U_FSUB(vm, regs + 6);
    block44419:;
    SetLVal(vm, &locals[21]); // dy
    U_LV_WRITE(vm, regs + 5);
    U_STATEMENT(vm, regs + 4, 131, 20);
    regs[4] = locals[9];regs[5] = locals[10];regs[6] = locals[11]; // cx
    U_PUSHFLT(vm, regs + 7, 1065353216);
    regs[8] = locals[19]; // dx
    U_FADD(vm, regs + 9);
    U_PUSHFLT(vm, regs + 8, 1073741824);
    U_FDIV(vm, regs + 9);
    regs[8] = locals[17]; // x
    U_I2F(vm, regs + 9);
    U_FADD(vm, regs + 9);
    U_PUSHVARF(vm, regs + 8, 1015); // w
    U_I2F(vm, regs + 9);
    U_FDIV(vm, regs + 9);
    U_PUSHFLT(vm, regs + 8, 1056964608);
    U_FSUB(vm, regs + 9);
    U_FVSMUL(vm, regs + 8, 3);
    regs[7] = locals[12];regs[8] = locals[13];regs[9] = locals[14]; // cy
    U_PUSHFLT(vm, regs + 10, 1065353216);
    regs[11] = locals[21]; // dy
    U_FADD(vm, regs + 12);
    U_PUSHFLT(vm, regs + 11, 1073741824);
    U_FDIV(vm, regs + 12);
    regs[11] = locals[16]; // y
    U_I2F(vm, regs + 12);
    U_FADD(vm, regs + 12);
    regs[11] = locals[2]; // h
    U_I2F(vm, regs + 12);
    U_FDIV(vm, regs + 12);
    U_PUSHFLT(vm, regs + 11, 1056964608);
    U_FSUB(vm, regs + 12);
    U_FVSMUL(vm, regs + 11, 3);
    U_FVVADD(vm, regs + 10, 3);
    regs[7] = locals[6];regs[8] = locals[7];regs[9] = locals[8]; // cam
    U_FVVADD(vm, regs + 10, 3);
    SetLVal(vm, &locals[22]); // d+0
    U_LV_WRITEV(vm, regs + 7, 3);
    U_STATEMENT(vm, regs + 4, 133, 20);
    regs[4] = locals[22];regs[5] = locals[23];regs[6] = locals[24]; // d
    U_PUSHINT(vm, regs + 7, 3);
    U_BCALLRETV(vm, regs + 8, 84, 1); // normalize
    SetLVal(vm, &locals[22]); // d+0
    U_LV_WRITEV(vm, regs + 7, 3);
    U_STATEMENT(vm, regs + 4, 134, 20);
    regs[4] = locals[3];regs[5] = locals[4];regs[6] = locals[5]; // cam
    regs[7] = locals[22];regs[8] = locals[23];regs[9] = locals[24]; // d
    U_PUSHFLT(vm, regs + 10, 1124859904);
    U_FVSMUL(vm, regs + 11, 3);
    U_FVVADD(vm, regs + 10, 3);
    regs[7] = locals[22];regs[8] = locals[23];regs[9] = locals[24]; // d
    U_PUSHINT(vm, regs + 10, 0);
    fun_369(vm, regs + 11); // call: radiance
    SetLVal(vm, &locals[25]); // r+0
    U_LV_WRITEV(vm, regs + 7, 3);
    U_STATEMENT(vm, regs + 4, 136, 20);
    regs[4] = locals[25];regs[5] = locals[26];regs[6] = locals[27]; // r
    regs[7] = locals[15]; // c
    regs[8] = locals[2]; // h
    regs[9] = locals[16]; // y
    U_ISUB(vm, regs + 10);
    U_PUSHINT(vm, regs + 9, 1);
    U_ISUB(vm, regs + 10);
    U_VPUSHIDXI(vm, regs + 9);
    regs[8] = locals[17]; // x
    U_LVAL_IDXVI(vm, regs + 9, 0);
    U_LV_FVVADD(vm, regs + 7, 3);
    goto block44246;
    block44620:;
    goto block44220;
    block44625:;
    U_STATEMENT(vm, regs + 0, 137, 20);
    U_LVAL_VARF(vm, regs + 0, 1030); // samples
    U_LV_IPP(vm, regs + 0);
    U_STATEMENT(vm, regs + 0, 138, 20);
    regs[0] = locals[15]; // c
    U_PUSHFUN(vm, regs + 1, 0, fun_372);
    fun_600(vm, regs + 2); // call: map
    SetLVal(vm, &locals[28]); // r
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 139, 20);
    regs[0] = locals[28]; // r
    U_BCALLRET1(vm, regs + 1, 13, 1); // length
    regs[1] = locals[2]; // h
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 139, 20, 536);
    U_RETURNLOCAL(vm, 0, 0);
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
static void fun_368(VMRef vm, StackPtr psp) {
    Value regs[9];
    Value locals[14];
    locals[0] = *(psp - 7);
    locals[1] = *(psp - 6);
    locals[2] = *(psp - 5);
    locals[3] = *(psp - 4);
    locals[4] = *(psp - 3);
    locals[5] = *(psp - 2);
    locals[6] = *(psp - 1);
    locals[7] = lobster::NilVal();
    locals[8] = lobster::NilVal();
    locals[9] = lobster::NilVal();
    locals[10] = lobster::NilVal();
    locals[11] = lobster::NilVal();
    locals[12] = lobster::NilVal();
    locals[13] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 3170, locals);
    U_STATEMENT(vm, regs + 0, 26, 20);
    regs[0] = locals[0]; // sphere
    U_PUSHFLD2V(vm, regs + 1, 1, 3);
    regs[3] = locals[1];regs[4] = locals[2];regs[5] = locals[3]; // r
    U_FVVSUB(vm, regs + 6, 3);
    SetLVal(vm, &locals[7]); // op+0
    U_LV_WRITEV(vm, regs + 3, 3);
    U_STATEMENT(vm, regs + 0, 27, 20);
    U_PUSHFLT64(vm, regs + 0, -350469331, 1058682594);
    SetLVal(vm, &locals[10]); // eps
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 28, 20);
    regs[0] = locals[7];regs[1] = locals[8];regs[2] = locals[9]; // op
    U_PUSHINT(vm, regs + 3, 3);
    regs[4] = locals[4];regs[5] = locals[5];regs[6] = locals[6]; // r
    U_PUSHINT(vm, regs + 7, 3);
    U_BCALLRETV(vm, regs + 8, 85, 1); // dot
    SetLVal(vm, &locals[11]); // b
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 29, 20);
    regs[0] = locals[11]; // b
    regs[1] = locals[11]; // b
    U_FMUL(vm, regs + 2);
    regs[1] = locals[7];regs[2] = locals[8];regs[3] = locals[9]; // op
    U_PUSHINT(vm, regs + 4, 3);
    regs[5] = locals[7];regs[6] = locals[8];regs[7] = locals[9]; // op
    U_PUSHINT(vm, regs + 8, 3);
    U_BCALLRETV(vm, regs + 9, 85, 1); // dot
    U_FSUB(vm, regs + 2);
    regs[1] = locals[0]; // sphere
    U_PUSHFLD(vm, regs + 2, 0);
    regs[2] = locals[0]; // sphere
    U_PUSHFLD(vm, regs + 3, 0);
    U_FMUL(vm, regs + 3);
    U_FADD(vm, regs + 2);
    SetLVal(vm, &locals[12]); // det
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 30, 20);
    regs[0] = locals[12]; // det
    U_PUSHFLT(vm, regs + 1, 0);
    U_FLT(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block44843;
    U_STATEMENT(vm, regs + 0, 30, 20);
    U_PUSHFLT(vm, regs + 0, 0);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    block44843:;
    U_STATEMENT(vm, regs + 0, 31, 20);
    regs[0] = locals[12]; // det
    U_BCALLRET1(vm, regs + 1, 56, 1); // sqrt
    SetLVal(vm, &locals[12]); // det
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 32, 20);
    regs[0] = locals[11]; // b
    regs[1] = locals[12]; // det
    U_FSUB(vm, regs + 2);
    SetLVal(vm, &locals[13]); // t
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 37, 20);
    regs[0] = locals[13]; // t
    regs[1] = locals[10]; // eps
    U_FGT(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block44903;
    U_STATEMENT(vm, regs + 0, 34, 20);
    regs[0] = locals[13]; // t
    goto block44958;
    block44903:;
    U_STATEMENT(vm, regs + 0, 36, 20);
    regs[0] = locals[11]; // b
    regs[1] = locals[12]; // det
    U_FADD(vm, regs + 2);
    SetLVal(vm, &locals[13]); // t
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 37, 20);
    regs[0] = locals[13]; // t
    regs[1] = locals[10]; // eps
    U_FGT(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block44947;
    U_STATEMENT(vm, regs + 0, 37, 20);
    regs[0] = locals[13]; // t
    goto block44956;
    block44947:;
    U_STATEMENT(vm, regs + 0, 37, 20);
    U_PUSHFLT(vm, regs + 0, 0);
    block44956:;
    block44958:;
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// radiance
static void fun_369(VMRef vm, StackPtr psp) {
    Value regs[23];
    Value locals[68];
    locals[0] = *(psp - 7);
    locals[1] = *(psp - 6);
    locals[2] = *(psp - 5);
    locals[3] = *(psp - 4);
    locals[4] = *(psp - 3);
    locals[5] = *(psp - 2);
    locals[6] = *(psp - 1);
    locals[7] = lobster::NilVal();
    locals[8] = lobster::NilVal();
    locals[9] = lobster::NilVal();
    locals[10] = lobster::NilVal();
    locals[11] = lobster::NilVal();
    locals[12] = lobster::NilVal();
    locals[13] = lobster::NilVal();
    locals[14] = lobster::NilVal();
    locals[15] = lobster::NilVal();
    locals[16] = lobster::NilVal();
    locals[17] = lobster::NilVal();
    locals[18] = lobster::NilVal();
    locals[19] = lobster::NilVal();
    locals[20] = lobster::NilVal();
    locals[21] = lobster::NilVal();
    locals[22] = lobster::NilVal();
    locals[23] = lobster::NilVal();
    locals[24] = lobster::NilVal();
    locals[25] = lobster::NilVal();
    locals[26] = lobster::NilVal();
    locals[27] = lobster::NilVal();
    locals[28] = lobster::NilVal();
    locals[29] = lobster::NilVal();
    locals[30] = lobster::NilVal();
    locals[31] = lobster::NilVal();
    locals[32] = lobster::NilVal();
    locals[33] = lobster::NilVal();
    locals[34] = lobster::NilVal();
    locals[35] = lobster::NilVal();
    locals[36] = lobster::NilVal();
    locals[37] = lobster::NilVal();
    locals[38] = lobster::NilVal();
    locals[39] = lobster::NilVal();
    locals[40] = lobster::NilVal();
    locals[41] = lobster::NilVal();
    locals[42] = lobster::NilVal();
    locals[43] = lobster::NilVal();
    locals[44] = lobster::NilVal();
    locals[45] = lobster::NilVal();
    locals[46] = lobster::NilVal();
    locals[47] = lobster::NilVal();
    locals[48] = lobster::NilVal();
    locals[49] = lobster::NilVal();
    locals[50] = lobster::NilVal();
    locals[51] = lobster::NilVal();
    locals[52] = lobster::NilVal();
    locals[53] = lobster::NilVal();
    locals[54] = lobster::NilVal();
    locals[55] = lobster::NilVal();
    locals[56] = lobster::NilVal();
    locals[57] = lobster::NilVal();
    locals[58] = lobster::NilVal();
    locals[59] = lobster::NilVal();
    locals[60] = lobster::NilVal();
    locals[61] = lobster::NilVal();
    locals[62] = lobster::NilVal();
    locals[63] = lobster::NilVal();
    locals[64] = lobster::NilVal();
    locals[65] = lobster::NilVal();
    locals[66] = lobster::NilVal();
    locals[67] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 3189, locals);
    U_STATEMENT(vm, regs + 0, 57, 20);
    U_PUSHFLT64(vm, regs + 0, -1577058304, 1114446484);
    SetLVal(vm, &locals[7]); // t
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 58, 20);
    U_PUSHINT(vm, regs + 0, -1);
    SetLVal(vm, &locals[8]); // id
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 59, 20);
    U_PUSHINT(vm, regs + 0, -1);
    U_PUSHVARF(vm, regs + 1, 946); // spheres
    block45076:;
    if (!U_VFOR(vm, regs + 2)) goto block45184;
    U_STATEMENT(vm, regs + 2, 59, 20);
    U_VFORELEMREF(vm, regs + 2);
    SetLVal(vm, &locals[9]); // s
    U_LV_WRITEREF(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 59, 20);
    U_FORLOOPI(vm, regs + 2);
    SetLVal(vm, &locals[10]); // i
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 60, 20);
    regs[2] = locals[9]; // s
    U_INCREF(vm, regs + 3, 0);
    regs[3] = locals[0];regs[4] = locals[1];regs[5] = locals[2];regs[6] = locals[3];regs[7] = locals[4];regs[8] = locals[5]; // r
    fun_368(vm, regs + 9); // call: intersect
    SetLVal(vm, &locals[11]); // d
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 61, 20);
    regs[2] = locals[11]; // d
    U_PUSHFLT(vm, regs + 3, 0);
    U_FNE(vm, regs + 4);
    if (!U_JUMPFAILR(vm, regs + 3)) goto block45150;
    U_POP(vm, regs + 3);
    regs[2] = locals[11]; // d
    regs[3] = locals[7]; // t
    U_FLT(vm, regs + 4);
    block45150:;
    if (!U_JUMPFAIL(vm, regs + 3)) goto block45179;
    U_STATEMENT(vm, regs + 2, 62, 20);
    regs[2] = locals[11]; // d
    SetLVal(vm, &locals[7]); // t
    U_LV_WRITE(vm, regs + 3);
    U_STATEMENT(vm, regs + 2, 63, 20);
    regs[2] = locals[10]; // i
    SetLVal(vm, &locals[8]); // id
    U_LV_WRITE(vm, regs + 3);
    block45179:;
    goto block45076;
    block45184:;
    U_STATEMENT(vm, regs + 0, 64, 20);
    regs[0] = locals[8]; // id
    U_PUSHINT(vm, regs + 1, 0);
    U_ILT(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block45212;
    U_STATEMENT(vm, regs + 0, 64, 20);
    U_PUSHVARVF(vm, regs + 0, 84, 3); // float3_0+0
    U_RETURNLOCAL(vm, 0, 3);
    DecVal(vm, locals[9]);
    DecVal(vm, locals[12]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    Push(psp, regs[2]);
    goto epilogue;
    block45212:;
    U_STATEMENT(vm, regs + 0, 65, 20);
    U_PUSHVARF(vm, regs + 0, 946); // spheres
    regs[1] = locals[8]; // id
    U_VPUSHIDXI(vm, regs + 2);
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[12]); // obj
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 66, 20);
    regs[0] = locals[0];regs[1] = locals[1];regs[2] = locals[2]; // r
    regs[3] = locals[3];regs[4] = locals[4];regs[5] = locals[5]; // r
    regs[6] = locals[7]; // t
    U_FVSMUL(vm, regs + 7, 3);
    U_FVVADD(vm, regs + 6, 3);
    SetLVal(vm, &locals[13]); // x+0
    U_LV_WRITEV(vm, regs + 3, 3);
    U_STATEMENT(vm, regs + 0, 67, 20);
    regs[0] = locals[13];regs[1] = locals[14];regs[2] = locals[15]; // x
    regs[3] = locals[12]; // obj
    U_PUSHFLD2V(vm, regs + 4, 1, 3);
    U_FVVSUB(vm, regs + 6, 3);
    U_PUSHINT(vm, regs + 3, 3);
    U_BCALLRETV(vm, regs + 4, 84, 1); // normalize
    SetLVal(vm, &locals[16]); // n+0
    U_LV_WRITEV(vm, regs + 3, 3);
    U_STATEMENT(vm, regs + 0, 68, 20);
    regs[0] = locals[16];regs[1] = locals[17];regs[2] = locals[18]; // n
    U_PUSHINT(vm, regs + 3, 3);
    regs[4] = locals[3];regs[5] = locals[4];regs[6] = locals[5]; // r
    U_PUSHINT(vm, regs + 7, 3);
    U_BCALLRETV(vm, regs + 8, 85, 1); // dot
    U_PUSHFLT(vm, regs + 1, 0);
    U_FLT(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block45333;
    U_STATEMENT(vm, regs + 0, 68, 20);
    regs[0] = locals[16];regs[1] = locals[17];regs[2] = locals[18]; // n
    goto block45349;
    block45333:;
    U_STATEMENT(vm, regs + 0, 68, 20);
    regs[0] = locals[16];regs[1] = locals[17];regs[2] = locals[18]; // n
    U_PUSHFLT(vm, regs + 3, -1082130432);
    U_FVSMUL(vm, regs + 4, 3);
    block45349:;
    SetLVal(vm, &locals[19]); // nl+0
    U_LV_WRITEV(vm, regs + 3, 3);
    U_STATEMENT(vm, regs + 0, 69, 20);
    regs[0] = locals[12]; // obj
    U_PUSHFLD2V(vm, regs + 1, 7, 3);
    SetLVal(vm, &locals[22]); // f+0
    U_LV_WRITEV(vm, regs + 3, 3);
    U_STATEMENT(vm, regs + 0, 70, 20);
    regs[0] = locals[22]; // f+0
    regs[1] = locals[23]; // f+1
    U_FGT(vm, regs + 2);
    if (!U_JUMPFAILR(vm, regs + 1)) goto block45399;
    U_POP(vm, regs + 1);
    regs[0] = locals[22]; // f+0
    regs[1] = locals[24]; // f+2
    U_FGT(vm, regs + 2);
    block45399:;
    if (!U_JUMPFAIL(vm, regs + 1)) goto block45414;
    U_STATEMENT(vm, regs + 0, 70, 20);
    regs[0] = locals[22]; // f+0
    goto block45452;
    block45414:;
    U_STATEMENT(vm, regs + 0, 70, 20);
    regs[0] = locals[23]; // f+1
    regs[1] = locals[24]; // f+2
    U_FGT(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block45441;
    U_STATEMENT(vm, regs + 0, 70, 20);
    regs[0] = locals[23]; // f+1
    goto block45450;
    block45441:;
    U_STATEMENT(vm, regs + 0, 70, 20);
    regs[0] = locals[24]; // f+2
    block45450:;
    block45452:;
    SetLVal(vm, &locals[25]); // p
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 71, 20);
    SetLVal(vm, &locals[6]); // depth
    U_LV_IPP(vm, regs + 0);
    U_LV_DUP(vm, regs + 0);
    U_I2F(vm, regs + 1);
    U_PUSHFLT(vm, regs + 1, 1084227584);
    U_FGT(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block45542;
    U_STATEMENT(vm, regs + 0, 71, 20);
    U_BCALLRET0(vm, regs + 0, 95, 1); // rnd_float
    regs[1] = locals[25]; // p
    U_FLT(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block45524;
    U_STATEMENT(vm, regs + 0, 71, 20);
    regs[0] = locals[22];regs[1] = locals[23];regs[2] = locals[24]; // f
    U_PUSHFLT(vm, regs + 3, 1065353216);
    regs[4] = locals[25]; // p
    U_FDIV(vm, regs + 5);
    U_FVSMUL(vm, regs + 4, 3);
    SetLVal(vm, &locals[22]); // f+0
    U_LV_WRITEV(vm, regs + 3, 3);
    goto block45540;
    block45524:;
    U_STATEMENT(vm, regs + 0, 71, 20);
    regs[0] = locals[12]; // obj
    U_PUSHFLD2V(vm, regs + 1, 4, 3);
    U_RETURNLOCAL(vm, 0, 3);
    DecVal(vm, locals[9]);
    DecVal(vm, locals[12]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    Push(psp, regs[2]);
    goto epilogue;
    block45540:;
    block45542:;
    U_STATEMENT(vm, regs + 0, 72, 20);
    regs[0] = locals[12]; // obj
    U_PUSHFLD(vm, regs + 1, 10);
    U_PUSHINT(vm, regs + 1, 0);
    U_IEQ(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block45844;
    U_STATEMENT(vm, regs + 0, 73, 20);
    U_PUSHFLT(vm, regs + 0, 1135869952);
    U_BCALLRET0(vm, regs + 1, 95, 1); // rnd_float
    U_FMUL(vm, regs + 2);
    SetLVal(vm, &locals[26]); // r1
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 74, 20);
    U_BCALLRET0(vm, regs + 0, 95, 1); // rnd_float
    SetLVal(vm, &locals[27]); // r2
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 75, 20);
    regs[0] = locals[27]; // r2
    U_BCALLRET1(vm, regs + 1, 56, 1); // sqrt
    SetLVal(vm, &locals[28]); // r2s
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 76, 20);
    regs[0] = locals[19];regs[1] = locals[20];regs[2] = locals[21]; // nl
    SetLVal(vm, &locals[29]); // w+0
    U_LV_WRITEV(vm, regs + 3, 3);
    U_STATEMENT(vm, regs + 0, 77, 20);
    regs[0] = locals[29]; // w+0
    U_BCALLRET1(vm, regs + 1, 112, 1); // abs
    U_PUSHFLT64(vm, regs + 1, -1717986918, 1069128089);
    U_FGT(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block45654;
    U_STATEMENT(vm, regs + 0, 77, 20);
    U_PUSHVARVF(vm, regs + 0, 93, 3); // float3_y+0
    goto block45664;
    block45654:;
    U_STATEMENT(vm, regs + 0, 77, 20);
    U_PUSHVARVF(vm, regs + 0, 90, 3); // float3_x+0
    block45664:;
    U_PUSHINT(vm, regs + 3, 3);
    regs[4] = locals[29];regs[5] = locals[30];regs[6] = locals[31]; // w
    U_PUSHINT(vm, regs + 7, 3);
    U_BCALLRETV(vm, regs + 8, 90, 1); // cross
    U_PUSHINT(vm, regs + 3, 3);
    U_BCALLRETV(vm, regs + 4, 84, 1); // normalize
    SetLVal(vm, &locals[32]); // u+0
    U_LV_WRITEV(vm, regs + 3, 3);
    U_STATEMENT(vm, regs + 0, 78, 20);
    regs[0] = locals[29];regs[1] = locals[30];regs[2] = locals[31]; // w
    U_PUSHINT(vm, regs + 3, 3);
    regs[4] = locals[32];regs[5] = locals[33];regs[6] = locals[34]; // u
    U_PUSHINT(vm, regs + 7, 3);
    U_BCALLRETV(vm, regs + 8, 90, 1); // cross
    SetLVal(vm, &locals[35]); // v+0
    U_LV_WRITEV(vm, regs + 3, 3);
    U_STATEMENT(vm, regs + 0, 79, 20);
    regs[0] = locals[32];regs[1] = locals[33];regs[2] = locals[34]; // u
    regs[3] = locals[26]; // r1
    U_BCALLRET1(vm, regs + 4, 71, 1); // cos
    U_FVSMUL(vm, regs + 4, 3);
    regs[3] = locals[28]; // r2s
    U_FVSMUL(vm, regs + 4, 3);
    regs[3] = locals[35];regs[4] = locals[36];regs[5] = locals[37]; // v
    regs[6] = locals[26]; // r1
    U_BCALLRET1(vm, regs + 7, 69, 1); // sin
    U_FVSMUL(vm, regs + 7, 3);
    regs[6] = locals[28]; // r2s
    U_FVSMUL(vm, regs + 7, 3);
    U_FVVADD(vm, regs + 6, 3);
    regs[3] = locals[29];regs[4] = locals[30];regs[5] = locals[31]; // w
    U_PUSHFLT(vm, regs + 6, 1065353216);
    regs[7] = locals[27]; // r2
    U_FSUB(vm, regs + 8);
    U_BCALLRET1(vm, regs + 7, 56, 1); // sqrt
    U_FVSMUL(vm, regs + 7, 3);
    U_FVVADD(vm, regs + 6, 3);
    U_PUSHINT(vm, regs + 3, 3);
    U_BCALLRETV(vm, regs + 4, 84, 1); // normalize
    SetLVal(vm, &locals[38]); // d+0
    U_LV_WRITEV(vm, regs + 3, 3);
    U_STATEMENT(vm, regs + 0, 80, 20);
    regs[0] = locals[12]; // obj
    U_PUSHFLD2V(vm, regs + 1, 4, 3);
    regs[3] = locals[22];regs[4] = locals[23];regs[5] = locals[24]; // f
    regs[6] = locals[13];regs[7] = locals[14];regs[8] = locals[15]; // x
    regs[9] = locals[38];regs[10] = locals[39];regs[11] = locals[40]; // d
    regs[12] = locals[6]; // depth
    fun_369(vm, regs + 13); // call: radiance
    U_FVVMUL(vm, regs + 9, 3);
    U_FVVADD(vm, regs + 6, 3);
    U_RETURNLOCAL(vm, 0, 3);
    DecVal(vm, locals[9]);
    DecVal(vm, locals[12]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    Push(psp, regs[2]);
    goto epilogue;
    goto block45938;
    block45844:;
    U_STATEMENT(vm, regs + 0, 81, 20);
    regs[0] = locals[12]; // obj
    U_PUSHFLD(vm, regs + 1, 10);
    U_PUSHINT(vm, regs + 1, 1);
    U_IEQ(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block45936;
    U_STATEMENT(vm, regs + 0, 82, 20);
    regs[0] = locals[12]; // obj
    U_PUSHFLD2V(vm, regs + 1, 4, 3);
    regs[3] = locals[22];regs[4] = locals[23];regs[5] = locals[24]; // f
    regs[6] = locals[13];regs[7] = locals[14];regs[8] = locals[15]; // x
    regs[9] = locals[3];regs[10] = locals[4];regs[11] = locals[5]; // r
    regs[12] = locals[16];regs[13] = locals[17];regs[14] = locals[18]; // n
    U_PUSHFLT(vm, regs + 15, 1073741824);
    U_FVSMUL(vm, regs + 16, 3);
    regs[15] = locals[16];regs[16] = locals[17];regs[17] = locals[18]; // n
    U_PUSHINT(vm, regs + 18, 3);
    regs[19] = locals[3];regs[20] = locals[4];regs[21] = locals[5]; // r
    U_PUSHINT(vm, regs + 22, 3);
    U_BCALLRETV(vm, regs + 23, 85, 1); // dot
    U_FVSMUL(vm, regs + 16, 3);
    U_FVVSUB(vm, regs + 15, 3);
    regs[12] = locals[6]; // depth
    fun_369(vm, regs + 13); // call: radiance
    U_FVVMUL(vm, regs + 9, 3);
    U_FVVADD(vm, regs + 6, 3);
    U_RETURNLOCAL(vm, 0, 3);
    DecVal(vm, locals[9]);
    DecVal(vm, locals[12]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    Push(psp, regs[2]);
    goto epilogue;
    block45936:;
    block45938:;
    U_STATEMENT(vm, regs + 0, 83, 20);
    regs[0] = locals[13];regs[1] = locals[14];regs[2] = locals[15]; // x
    regs[3] = locals[3];regs[4] = locals[4];regs[5] = locals[5]; // r
    regs[6] = locals[16];regs[7] = locals[17];regs[8] = locals[18]; // n
    U_PUSHFLT(vm, regs + 9, 1073741824);
    U_FVSMUL(vm, regs + 10, 3);
    regs[9] = locals[16];regs[10] = locals[17];regs[11] = locals[18]; // n
    U_PUSHINT(vm, regs + 12, 3);
    regs[13] = locals[3];regs[14] = locals[4];regs[15] = locals[5]; // r
    U_PUSHINT(vm, regs + 16, 3);
    U_BCALLRETV(vm, regs + 17, 85, 1); // dot
    U_FVSMUL(vm, regs + 10, 3);
    U_FVVSUB(vm, regs + 9, 3);
    SetLVal(vm, &locals[41]); // reflRay+0
    U_LV_WRITEV(vm, regs + 6, 6);
    U_STATEMENT(vm, regs + 0, 84, 20);
    regs[0] = locals[16];regs[1] = locals[17];regs[2] = locals[18]; // n
    U_PUSHINT(vm, regs + 3, 3);
    regs[4] = locals[19];regs[5] = locals[20];regs[6] = locals[21]; // nl
    U_PUSHINT(vm, regs + 7, 3);
    U_BCALLRETV(vm, regs + 8, 85, 1); // dot
    U_PUSHFLT(vm, regs + 1, 0);
    U_FGT(vm, regs + 2);
    SetLVal(vm, &locals[47]); // into
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 85, 20);
    U_PUSHFLT(vm, regs + 0, 1065353216);
    SetLVal(vm, &locals[48]); // nc
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 86, 20);
    U_PUSHFLT(vm, regs + 0, 1069547520);
    SetLVal(vm, &locals[49]); // nt
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 87, 20);
    regs[0] = locals[47]; // into
    if (!U_JUMPFAIL(vm, regs + 1)) goto block46073;
    U_STATEMENT(vm, regs + 0, 87, 20);
    regs[0] = locals[48]; // nc
    regs[1] = locals[49]; // nt
    U_FDIV(vm, regs + 2);
    goto block46087;
    block46073:;
    U_STATEMENT(vm, regs + 0, 87, 20);
    regs[0] = locals[49]; // nt
    regs[1] = locals[48]; // nc
    U_FDIV(vm, regs + 2);
    block46087:;
    SetLVal(vm, &locals[50]); // nnt
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 88, 20);
    regs[0] = locals[3];regs[1] = locals[4];regs[2] = locals[5]; // r
    U_PUSHINT(vm, regs + 3, 3);
    regs[4] = locals[19];regs[5] = locals[20];regs[6] = locals[21]; // nl
    U_PUSHINT(vm, regs + 7, 3);
    U_BCALLRETV(vm, regs + 8, 85, 1); // dot
    SetLVal(vm, &locals[51]); // ddn
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 89, 20);
    U_PUSHFLT(vm, regs + 0, 1065353216);
    regs[1] = locals[50]; // nnt
    regs[2] = locals[50]; // nnt
    U_FMUL(vm, regs + 3);
    U_PUSHFLT(vm, regs + 2, 1065353216);
    regs[3] = locals[51]; // ddn
    regs[4] = locals[51]; // ddn
    U_FMUL(vm, regs + 5);
    U_FSUB(vm, regs + 4);
    U_FMUL(vm, regs + 3);
    U_FSUB(vm, regs + 2);
    SetLVal(vm, &locals[52]); // cos2t
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 90, 20);
    regs[0] = locals[52]; // cos2t
    U_PUSHFLT(vm, regs + 1, 0);
    U_FLT(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block46207;
    U_STATEMENT(vm, regs + 0, 91, 20);
    regs[0] = locals[12]; // obj
    U_PUSHFLD2V(vm, regs + 1, 4, 3);
    regs[3] = locals[22];regs[4] = locals[23];regs[5] = locals[24]; // f
    regs[6] = locals[41];regs[7] = locals[42];regs[8] = locals[43];regs[9] = locals[44];regs[10] = locals[45];regs[11] = locals[46]; // reflRay
    regs[12] = locals[6]; // depth
    fun_369(vm, regs + 13); // call: radiance
    U_FVVMUL(vm, regs + 9, 3);
    U_FVVADD(vm, regs + 6, 3);
    U_RETURNLOCAL(vm, 0, 3);
    DecVal(vm, locals[9]);
    DecVal(vm, locals[12]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    Push(psp, regs[2]);
    goto epilogue;
    block46207:;
    U_STATEMENT(vm, regs + 0, 92, 20);
    regs[0] = locals[3];regs[1] = locals[4];regs[2] = locals[5]; // r
    regs[3] = locals[50]; // nnt
    U_FVSMUL(vm, regs + 4, 3);
    regs[3] = locals[16];regs[4] = locals[17];regs[5] = locals[18]; // n
    regs[6] = locals[47]; // into
    if (!U_JUMPFAIL(vm, regs + 7)) goto block46243;
    U_STATEMENT(vm, regs + 6, 92, 20);
    U_PUSHINT(vm, regs + 6, 1);
    goto block46252;
    block46243:;
    U_STATEMENT(vm, regs + 6, 92, 20);
    U_PUSHINT(vm, regs + 6, -1);
    block46252:;
    U_I2F(vm, regs + 7);
    regs[7] = locals[51]; // ddn
    regs[8] = locals[50]; // nnt
    U_FMUL(vm, regs + 9);
    regs[8] = locals[52]; // cos2t
    U_BCALLRET1(vm, regs + 9, 56, 1); // sqrt
    U_FADD(vm, regs + 9);
    U_FMUL(vm, regs + 8);
    U_FVSMUL(vm, regs + 7, 3);
    U_FVVSUB(vm, regs + 6, 3);
    U_PUSHINT(vm, regs + 3, 3);
    U_BCALLRETV(vm, regs + 4, 84, 1); // normalize
    SetLVal(vm, &locals[53]); // tdir+0
    U_LV_WRITEV(vm, regs + 3, 3);
    U_STATEMENT(vm, regs + 0, 93, 20);
    regs[0] = locals[49]; // nt
    regs[1] = locals[48]; // nc
    U_FSUB(vm, regs + 2);
    SetLVal(vm, &locals[56]); // a
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 94, 20);
    regs[0] = locals[49]; // nt
    regs[1] = locals[48]; // nc
    U_FADD(vm, regs + 2);
    SetLVal(vm, &locals[57]); // b
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 95, 20);
    regs[0] = locals[56]; // a
    regs[1] = locals[56]; // a
    U_FMUL(vm, regs + 2);
    regs[1] = locals[57]; // b
    regs[2] = locals[57]; // b
    U_FMUL(vm, regs + 3);
    U_FDIV(vm, regs + 2);
    SetLVal(vm, &locals[58]); // R0
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 96, 20);
    U_PUSHFLT(vm, regs + 0, 1065353216);
    regs[1] = locals[47]; // into
    if (!U_JUMPFAIL(vm, regs + 2)) goto block46380;
    U_STATEMENT(vm, regs + 1, 96, 20);
    regs[1] = locals[51]; // ddn
    U_FUMINUS(vm, regs + 2);
    goto block46404;
    block46380:;
    U_STATEMENT(vm, regs + 1, 96, 20);
    regs[1] = locals[53];regs[2] = locals[54];regs[3] = locals[55]; // tdir
    U_PUSHINT(vm, regs + 4, 3);
    regs[5] = locals[16];regs[6] = locals[17];regs[7] = locals[18]; // n
    U_PUSHINT(vm, regs + 8, 3);
    U_BCALLRETV(vm, regs + 9, 85, 1); // dot
    block46404:;
    U_FSUB(vm, regs + 2);
    SetLVal(vm, &locals[59]); // c
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 97, 20);
    regs[0] = locals[58]; // R0
    U_PUSHFLT(vm, regs + 1, 1065353216);
    regs[2] = locals[58]; // R0
    U_FSUB(vm, regs + 3);
    regs[2] = locals[59]; // c
    U_FMUL(vm, regs + 3);
    regs[2] = locals[59]; // c
    U_FMUL(vm, regs + 3);
    regs[2] = locals[59]; // c
    U_FMUL(vm, regs + 3);
    regs[2] = locals[59]; // c
    U_FMUL(vm, regs + 3);
    regs[2] = locals[59]; // c
    U_FMUL(vm, regs + 3);
    U_FADD(vm, regs + 2);
    SetLVal(vm, &locals[60]); // Re
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 98, 20);
    U_PUSHFLT(vm, regs + 0, 1065353216);
    regs[1] = locals[60]; // Re
    U_FSUB(vm, regs + 2);
    SetLVal(vm, &locals[61]); // Tr
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 99, 20);
    U_PUSHFLT(vm, regs + 0, 1048576000);
    U_PUSHFLT(vm, regs + 1, 1056964608);
    regs[2] = locals[60]; // Re
    U_FMUL(vm, regs + 3);
    U_FADD(vm, regs + 2);
    SetLVal(vm, &locals[62]); // P
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 100, 20);
    regs[0] = locals[60]; // Re
    regs[1] = locals[62]; // P
    U_FDIV(vm, regs + 2);
    SetLVal(vm, &locals[63]); // RP
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 101, 20);
    regs[0] = locals[61]; // Tr
    U_PUSHFLT(vm, regs + 1, 1065353216);
    regs[2] = locals[62]; // P
    U_FSUB(vm, regs + 3);
    U_FDIV(vm, regs + 2);
    SetLVal(vm, &locals[64]); // TP
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 102, 20);
    regs[0] = locals[6]; // depth
    U_PUSHINT(vm, regs + 1, 2);
    U_IGT(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block46623;
    U_STATEMENT(vm, regs + 0, 104, 20);
    U_BCALLRET0(vm, regs + 0, 95, 1); // rnd_float
    regs[1] = locals[62]; // P
    U_FLT(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block46592;
    U_STATEMENT(vm, regs + 0, 105, 20);
    regs[0] = locals[41];regs[1] = locals[42];regs[2] = locals[43];regs[3] = locals[44];regs[4] = locals[45];regs[5] = locals[46]; // reflRay
    regs[6] = locals[6]; // depth
    fun_369(vm, regs + 7); // call: radiance
    regs[3] = locals[63]; // RP
    U_FVSMUL(vm, regs + 4, 3);
    goto block46618;
    block46592:;
    U_STATEMENT(vm, regs + 0, 107, 20);
    regs[0] = locals[13];regs[1] = locals[14];regs[2] = locals[15]; // x
    regs[3] = locals[53];regs[4] = locals[54];regs[5] = locals[55]; // tdir
    regs[6] = locals[6]; // depth
    fun_369(vm, regs + 7); // call: radiance
    regs[3] = locals[64]; // TP
    U_FVSMUL(vm, regs + 4, 3);
    block46618:;
    goto block46668;
    block46623:;
    U_STATEMENT(vm, regs + 0, 109, 20);
    regs[0] = locals[41];regs[1] = locals[42];regs[2] = locals[43];regs[3] = locals[44];regs[4] = locals[45];regs[5] = locals[46]; // reflRay
    regs[6] = locals[6]; // depth
    fun_369(vm, regs + 7); // call: radiance
    regs[3] = locals[60]; // Re
    U_FVSMUL(vm, regs + 4, 3);
    regs[3] = locals[13];regs[4] = locals[14];regs[5] = locals[15]; // x
    regs[6] = locals[53];regs[7] = locals[54];regs[8] = locals[55]; // tdir
    regs[9] = locals[6]; // depth
    fun_369(vm, regs + 10); // call: radiance
    regs[6] = locals[61]; // Tr
    U_FVSMUL(vm, regs + 7, 3);
    U_FVVADD(vm, regs + 6, 3);
    block46668:;
    SetLVal(vm, &locals[65]); // temp+0
    U_LV_WRITEV(vm, regs + 3, 3);
    U_STATEMENT(vm, regs + 0, 110, 20);
    regs[0] = locals[12]; // obj
    U_PUSHFLD2V(vm, regs + 1, 4, 3);
    regs[3] = locals[22];regs[4] = locals[23];regs[5] = locals[24]; // f
    regs[6] = locals[65];regs[7] = locals[66];regs[8] = locals[67]; // temp
    U_FVVMUL(vm, regs + 9, 3);
    U_FVVADD(vm, regs + 6, 3);
    U_RETURNLOCAL(vm, 0, 3);
    DecVal(vm, locals[9]);
    DecVal(vm, locals[12]);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    Push(psp, regs[2]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function337
static void fun_370(VMRef vm, StackPtr psp) {
    Value regs[2];
    PushFunId(vm, funinfo_table + 3262, 0);
    U_STATEMENT(vm, regs + 0, 119, 20);
    U_PUSHVARF(vm, regs + 0, 1015); // w
    U_PUSHFUN(vm, regs + 1, 0, fun_10000001);
    fun_599(vm, regs + 2); // call: map
    U_RETURNLOCAL(vm, 0, 1);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function339
static void fun_372(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3267, locals);
    U_STATEMENT(vm, regs + 0, 138, 20);
    regs[0] = locals[0]; // row
    U_PUSHFUN(vm, regs + 1, 0, fun_373);
    fun_601(vm, regs + 2); // call: map
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function340
static void fun_373(VMRef vm, StackPtr psp) {
    Value regs[12];
    Value locals[3];
    locals[0] = *(psp - 3);
    locals[1] = *(psp - 2);
    locals[2] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3273, locals);
    U_STATEMENT(vm, regs + 0, 138, 20);
    regs[0] = locals[0];regs[1] = locals[1];regs[2] = locals[2]; // col
    U_PUSHVARF(vm, regs + 3, 1030); // samples
    U_I2F(vm, regs + 4);
    U_FVSDIV(vm, regs + 4, 3);
    U_PUSHINT(vm, regs + 3, 3);
    U_PUSHVARVF(vm, regs + 4, 84, 3); // float3_0+0
    U_PUSHINT(vm, regs + 7, 3);
    U_PUSHVARVF(vm, regs + 8, 87, 3); // float3_1+0
    U_PUSHINT(vm, regs + 11, 3);
    U_BCALLRETV(vm, regs + 12, 104, 1); // clamp
    U_PUSHINT(vm, regs + 3, 3);
    U_PUSHFLT64(vm, regs + 4, -780903145, 1071454021);
    U_BCALLRETV(vm, regs + 5, 53, 1); // pow
    U_PUSHFLT(vm, regs + 3, 1065353216);
    fun_79(vm, regs + 4); // call: vec4
    U_RETURNLOCAL(vm, 0, 4);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    Push(psp, regs[2]);
    Push(psp, regs[3]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function341
static void fun_374(VMRef vm, StackPtr psp) {
    Value regs[6];
    Value keepvar[32];
    Value locals[2];
    locals[0] = lobster::NilVal();
    locals[1] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 3281, locals);
    keepvar[0] = lobster::NilVal();
    keepvar[1] = lobster::NilVal();
    keepvar[2] = lobster::NilVal();
    keepvar[3] = lobster::NilVal();
    keepvar[4] = lobster::NilVal();
    keepvar[5] = lobster::NilVal();
    keepvar[6] = lobster::NilVal();
    keepvar[7] = lobster::NilVal();
    keepvar[8] = lobster::NilVal();
    keepvar[9] = lobster::NilVal();
    keepvar[10] = lobster::NilVal();
    keepvar[11] = lobster::NilVal();
    keepvar[12] = lobster::NilVal();
    keepvar[13] = lobster::NilVal();
    keepvar[14] = lobster::NilVal();
    keepvar[15] = lobster::NilVal();
    keepvar[16] = lobster::NilVal();
    keepvar[17] = lobster::NilVal();
    keepvar[18] = lobster::NilVal();
    keepvar[19] = lobster::NilVal();
    keepvar[20] = lobster::NilVal();
    keepvar[21] = lobster::NilVal();
    keepvar[22] = lobster::NilVal();
    keepvar[23] = lobster::NilVal();
    keepvar[24] = lobster::NilVal();
    keepvar[25] = lobster::NilVal();
    keepvar[26] = lobster::NilVal();
    keepvar[27] = lobster::NilVal();
    keepvar[28] = lobster::NilVal();
    keepvar[29] = lobster::NilVal();
    keepvar[30] = lobster::NilVal();
    keepvar[31] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 4, 22);
    U_PUSHINT(vm, regs + 0, 42);
    SetLVal(vm, &locals[0]); // a
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 5, 22);
    U_PUSHFLT64(vm, regs + 0, 1374389535, 1074339512);
    U_NEWVEC(vm, regs + 1, 9, 1);
    SetLVal(vm, &locals[1]); // b
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 8, 22);
    U_PUSHSTR(vm, regs + 0, 537); // "a = "
    regs[1] = locals[0]; // a
    U_A2S(vm, regs + 2, 0);
    keepvar[0] = TopM(regs + 2, 0);
    U_PUSHSTR(vm, regs + 2, 538); // "!"
    U_SADDN(vm, regs + 3, 3);
    keepvar[1] = TopM(regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 539); // "a = "
    regs[2] = locals[0]; // a
    U_A2S(vm, regs + 3, 0);
    keepvar[2] = TopM(regs + 3, 0);
    U_PUSHSTR(vm, regs + 3, 540); // "!"
    U_SADDN(vm, regs + 4, 3);
    keepvar[3] = TopM(regs + 2, 0);
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 9, 22, 541);
    U_STATEMENT(vm, regs + 0, 10, 22);
    U_PUSHSTR(vm, regs + 0, 542); // "a = "
    regs[1] = locals[0]; // a
    U_A2S(vm, regs + 2, 0);
    keepvar[4] = TopM(regs + 2, 0);
    U_PUSHSTR(vm, regs + 2, 543); // " and b = "
    regs[3] = locals[1]; // b
    U_A2S(vm, regs + 4, 9);
    keepvar[5] = TopM(regs + 4, 0);
    U_PUSHSTR(vm, regs + 4, 544); // "!"
    U_SADDN(vm, regs + 5, 5);
    keepvar[6] = TopM(regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 545); // "a = "
    regs[2] = locals[0]; // a
    U_A2S(vm, regs + 3, 0);
    keepvar[7] = TopM(regs + 3, 0);
    U_PUSHSTR(vm, regs + 3, 546); // " and b = "
    regs[4] = locals[1]; // b
    U_A2S(vm, regs + 5, 9);
    keepvar[8] = TopM(regs + 5, 0);
    U_PUSHSTR(vm, regs + 5, 547); // "!"
    U_SADDN(vm, regs + 6, 5);
    keepvar[9] = TopM(regs + 2, 0);
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 11, 22, 548);
    U_STATEMENT(vm, regs + 0, 12, 22);
    U_PUSHINT(vm, regs + 0, 3);
    regs[1] = locals[0]; // a
    U_IADD(vm, regs + 2);
    U_A2S(vm, regs + 1, 0);
    keepvar[10] = TopM(regs + 1, 0);
    U_PUSHINT(vm, regs + 1, 3);
    regs[2] = locals[0]; // a
    U_IADD(vm, regs + 3);
    U_A2S(vm, regs + 2, 0);
    keepvar[11] = TopM(regs + 2, 0);
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 13, 22, 549);
    U_STATEMENT(vm, regs + 0, 14, 22);
    regs[0] = locals[0]; // a
    U_A2S(vm, regs + 1, 0);
    keepvar[12] = TopM(regs + 1, 0);
    regs[1] = locals[1]; // b
    U_A2S(vm, regs + 2, 9);
    keepvar[13] = TopM(regs + 2, 0);
    U_SADD(vm, regs + 2);
    keepvar[14] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // a
    U_A2S(vm, regs + 2, 0);
    keepvar[15] = TopM(regs + 2, 0);
    regs[2] = locals[1]; // b
    U_A2S(vm, regs + 3, 9);
    keepvar[16] = TopM(regs + 3, 0);
    U_SADD(vm, regs + 3);
    keepvar[17] = TopM(regs + 2, 0);
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 15, 22, 550);
    U_STATEMENT(vm, regs + 0, 16, 22);
    U_PUSHSTR(vm, regs + 0, 551); // "{ "
    regs[1] = locals[0]; // a
    U_A2S(vm, regs + 2, 0);
    keepvar[18] = TopM(regs + 2, 0);
    U_PUSHSTR(vm, regs + 2, 552); // " }"
    U_SADDN(vm, regs + 3, 3);
    keepvar[19] = TopM(regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 553); // "{ "
    regs[2] = locals[0]; // a
    U_A2S(vm, regs + 3, 0);
    keepvar[20] = TopM(regs + 3, 0);
    U_PUSHSTR(vm, regs + 3, 554); // " }"
    U_SADDN(vm, regs + 4, 3);
    keepvar[21] = TopM(regs + 2, 0);
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 17, 22, 555);
    U_STATEMENT(vm, regs + 0, 18, 22);
    U_PUSHSTR(vm, regs + 0, 556); // "{ "
    regs[1] = locals[0]; // a
    U_A2S(vm, regs + 2, 0);
    keepvar[22] = TopM(regs + 2, 0);
    U_PUSHSTR(vm, regs + 2, 557); // " }"
    U_SADDN(vm, regs + 3, 3);
    keepvar[23] = TopM(regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 558); // "{ "
    regs[2] = locals[0]; // a
    U_A2S(vm, regs + 3, 0);
    keepvar[24] = TopM(regs + 3, 0);
    U_PUSHSTR(vm, regs + 3, 559); // " }"
    U_SADDN(vm, regs + 4, 3);
    keepvar[25] = TopM(regs + 2, 0);
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 19, 22, 560);
    U_STATEMENT(vm, regs + 0, 20, 22);
    U_PUSHSTR(vm, regs + 0, 561); // "{"
    U_PUSHINT(vm, regs + 1, 3);
    regs[2] = locals[0]; // a
    U_IADD(vm, regs + 3);
    U_A2S(vm, regs + 2, 0);
    keepvar[26] = TopM(regs + 2, 0);
    U_PUSHSTR(vm, regs + 2, 562); // "}"
    U_SADDN(vm, regs + 3, 3);
    keepvar[27] = TopM(regs + 1, 0);
    U_PUSHSTR(vm, regs + 1, 563); // "{"
    U_PUSHINT(vm, regs + 2, 3);
    regs[3] = locals[0]; // a
    U_IADD(vm, regs + 4);
    U_A2S(vm, regs + 3, 0);
    keepvar[28] = TopM(regs + 3, 0);
    U_PUSHSTR(vm, regs + 3, 564); // "}"
    U_SADDN(vm, regs + 4, 3);
    keepvar[29] = TopM(regs + 2, 0);
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 21, 22, 565);
    U_STATEMENT(vm, regs + 0, 22, 22);
    regs[0] = locals[0]; // a
    U_A2S(vm, regs + 1, 0);
    keepvar[30] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // a
    U_A2S(vm, regs + 2, 0);
    keepvar[31] = TopM(regs + 2, 0);
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 24, 22, 566);
    U_RETURNLOCAL(vm, 0, 0);
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

// function342
static void fun_375(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value keepvar[2];
    Value locals[4];
    locals[0] = lobster::NilVal();
    locals[1] = lobster::NilVal();
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 3288, locals);
    keepvar[0] = lobster::NilVal();
    keepvar[1] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 9, 23);
    U_BCALLRET0(vm, regs + 0, 162, 1); // vm_compiled_mode
    U_LOGNOT(vm, regs + 1);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block47487;
    U_STATEMENT(vm, regs + 0, 10, 23);
    U_PUSHSTR(vm, regs + 0, 567); // "return 1 + 2"
    U_NEWVEC(vm, regs + 1, 11, 0);
    keepvar[0] = TopM(regs + 2, 0);
    U_BCALLRET2(vm, regs + 2, 186, 1); // compile_run_code
    SetLVal(vm, &locals[1]); // comperr1
    U_LV_WRITEREF(vm, regs + 2);
    SetLVal(vm, &locals[0]); // compres1
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 11, 23);
    regs[0] = locals[1]; // comperr1
    if (!U_JUMPFAIL(vm, regs + 1)) goto block47399;
    U_STATEMENT(vm, regs + 0, 12, 23);
    regs[0] = locals[1]; // comperr1
    U_BCALLRET1(vm, regs + 1, 0, 0); // print
    block47399:;
    U_STATEMENT(vm, regs + 0, 13, 23);
    regs[0] = locals[0]; // compres1
    U_PUSHSTR(vm, regs + 1, 568); // "3"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 13, 23, 569);
    U_STATEMENT(vm, regs + 0, 15, 23);
    U_PUSHSTR(vm, regs + 0, 570); // "plugintest.lobster"
    U_NEWVEC(vm, regs + 1, 11, 0);
    keepvar[1] = TopM(regs + 2, 0);
    U_BCALLRET2(vm, regs + 2, 187, 1); // compile_run_file
    SetLVal(vm, &locals[3]); // comperr2
    U_LV_WRITEREF(vm, regs + 2);
    SetLVal(vm, &locals[2]); // compres2
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 16, 23);
    regs[0] = locals[3]; // comperr2
    if (!U_JUMPFAIL(vm, regs + 1)) goto block47468;
    U_STATEMENT(vm, regs + 0, 17, 23);
    regs[0] = locals[3]; // comperr2
    U_BCALLRET1(vm, regs + 1, 0, 0); // print
    block47468:;
    U_STATEMENT(vm, regs + 0, 18, 23);
    regs[0] = locals[2]; // compres2
    U_PUSHSTR(vm, regs + 1, 571); // "plugintest return value"
    U_SEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 18, 23, 572);
    block47487:;
    U_RETURNLOCAL(vm, 0, 0);
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

// function343
static void fun_376(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[2];
    locals[0] = lobster::NilVal();
    locals[1] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 3297, locals);
    U_STATEMENT(vm, regs + 0, 8, 24);
    U_STATEMENT(vm, regs + 0, 8, 24);
    U_PUSHFUN(vm, regs + 0, 0, fun_377);
    SetLVal(vm, &locals[0]); // f
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 253, 5);
    fun_377(vm, regs + 0); // call: function344
    U_STATEMENT(vm, regs + 0, 13, 24);
    U_STATEMENT(vm, regs + 0, 13, 24);
    U_PUSHFUN(vm, regs + 0, 0, fun_378);
    SetLVal(vm, &locals[1]); // f
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 253, 5);
    fun_378(vm, regs + 0); // call: function345
    U_RETURNLOCAL(vm, 0, 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function344
static void fun_377(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = lobster::NilVal();
    locals[1] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 3304, locals);
    U_STATEMENT(vm, regs + 0, 9, 24);
    U_PUSHSTR(vm, regs + 0, 573); // ""
    U_INCREF(vm, regs + 1, 0);
    U_NEWVEC(vm, regs + 1, 11, 1);
    SetLVal(vm, &locals[0]); // v
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 10, 24);
    regs[0] = locals[0]; // v
    U_PUSHINT(vm, regs + 1, 0);
    U_VPUSHIDXI(vm, regs + 2);
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[1]); // k
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 11, 24);
    regs[0] = locals[0]; // v
    U_PUSHINT(vm, regs + 1, 0);
    U_VPUSHIDXI(vm, regs + 2);
    U_INCREF(vm, regs + 1, 0);
    SetLVal(vm, &locals[1]); // k
    U_LV_WRITEREF(vm, regs + 1);
    U_RETURNLOCAL(vm, 0, 0);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function345
static void fun_378(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = lobster::NilVal();
    locals[1] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 3311, locals);
    U_STATEMENT(vm, regs + 0, 14, 24);
    U_PUSHINT(vm, regs + 0, 1);
    U_NEWVEC(vm, regs + 1, 7, 1);
    SetLVal(vm, &locals[0]); // v
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 15, 24);
    regs[0] = locals[0]; // v
    U_PUSHINT(vm, regs + 1, 0);
    U_VPUSHIDXI(vm, regs + 2);
    SetLVal(vm, &locals[1]); // k
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 16, 24);
    regs[0] = locals[0]; // v
    U_PUSHINT(vm, regs + 1, 0);
    U_VPUSHIDXI(vm, regs + 2);
    SetLVal(vm, &locals[1]); // k
    U_LV_WRITE(vm, regs + 1);
    U_RETURNLOCAL(vm, 0, 0);
    DecVal(vm, locals[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function346
static void fun_379(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    locals[0] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 3318, locals);
    U_STATEMENT(vm, regs + 0, 7, 25);
    U_STATEMENT(vm, regs + 0, 7, 25);
    U_PUSHFUN(vm, regs + 0, 0, fun_380);
    SetLVal(vm, &locals[0]); // f
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 253, 5);
    fun_380(vm, regs + 0); // call: function347
    U_RETURNLOCAL(vm, 0, 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function347
static void fun_380(VMRef vm, StackPtr psp) {
    Value regs[9];
    Value keepvar[1];
    Value locals[2];
    locals[0] = lobster::NilVal();
    locals[1] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 3324, locals);
    keepvar[0] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 8, 25);
    U_STATEMENT(vm, regs + 0, 8, 25);
    U_STATEMENT(vm, regs + 0, 15, 25);
    U_NEWVEC(vm, regs + 0, 1287, 0);
    U_PUSHSTR(vm, regs + 1, 574); // "a"
    U_INCREF(vm, regs + 2, 0);
    U_PUSHFLT(vm, regs + 2, 1065353216);
    U_PUSHFLT(vm, regs + 3, 1073741824);
    U_PUSHNIL(vm, regs + 4);
    U_PUSHSTR(vm, regs + 5, 575); // "b"
    U_INCREF(vm, regs + 6, 0);
    U_PUSHSTR(vm, regs + 6, 576); // "c"
    U_INCREF(vm, regs + 7, 0);
    U_NEWVEC(vm, regs + 7, 11, 2);
    U_PUSHINT(vm, regs + 6, 1);
    U_PUSHINT(vm, regs + 7, 2);
    U_NEWVEC(vm, regs + 8, 7, 2);
    U_PUSHINT(vm, regs + 7, 3);
    U_PUSHINT(vm, regs + 8, 4);
    U_NEWVEC(vm, regs + 9, 7, 2);
    U_NEWVEC(vm, regs + 8, 13, 2);
    U_NEWOBJECT(vm, regs + 7, 1260); // Nest
    SetLVal(vm, &locals[0]); // nested
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 16, 25);
    regs[0] = locals[0]; // nested
    U_PUSHINT(vm, regs + 1, 10);
    U_BCALLRET2(vm, regs + 2, 30, 1); // deepcopy
    SetLVal(vm, &locals[1]); // sub
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 17, 25);
    regs[0] = locals[1]; // sub
    U_INCREF(vm, regs + 1, 0);
    regs[1] = locals[0]; // nested
    U_LVAL_FLD(vm, regs + 2, 4);
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 18, 25);
    regs[0] = locals[0]; // nested
    U_PUSHFLD(vm, regs + 1, 0);
    regs[1] = locals[1]; // sub
    U_INCREF(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 2, 1);
    U_BCALLRETV(vm, regs + 3, 15, 1); // push
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 19, 25);
    regs[0] = locals[0]; // nested
    U_PUSHFLD(vm, regs + 1, 0);
    regs[1] = locals[1]; // sub
    U_INCREF(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 2, 1);
    U_BCALLRETV(vm, regs + 3, 15, 1); // push
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 20, 25);
    regs[0] = locals[0]; // nested
    U_PUSHINT(vm, regs + 1, 10);
    U_BCALLRET2(vm, regs + 2, 30, 1); // deepcopy
    keepvar[0] = TopM(regs + 1, 0);
    regs[1] = locals[0]; // nested
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 20, 25, 577);
    U_RETURNLOCAL(vm, 0, 0);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    goto epilogue;
    epilogue:;
    DecVal(vm, keepvar[0]);
    PopFunId(vm);
}

// operator*
static void fun_381(VMRef vm, StackPtr psp) {
    Value regs[6];
    Value locals[8];
    locals[0] = *(psp - 8);
    locals[1] = *(psp - 7);
    locals[2] = *(psp - 6);
    locals[3] = *(psp - 5);
    locals[4] = *(psp - 4);
    locals[5] = *(psp - 3);
    locals[6] = *(psp - 2);
    locals[7] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3331, locals);
    U_STATEMENT(vm, regs + 0, 13, 27);
    regs[0] = locals[3]; // this+3
    regs[1] = locals[4]; // o+0
    U_FMUL(vm, regs + 2);
    regs[1] = locals[0]; // this+0
    regs[2] = locals[7]; // o+3
    U_FMUL(vm, regs + 3);
    U_FADD(vm, regs + 2);
    regs[1] = locals[1]; // this+1
    regs[2] = locals[6]; // o+2
    U_FMUL(vm, regs + 3);
    U_FADD(vm, regs + 2);
    regs[1] = locals[2]; // this+2
    regs[2] = locals[5]; // o+1
    U_FMUL(vm, regs + 3);
    U_FSUB(vm, regs + 2);
    regs[1] = locals[3]; // this+3
    regs[2] = locals[5]; // o+1
    U_FMUL(vm, regs + 3);
    regs[2] = locals[0]; // this+0
    regs[3] = locals[6]; // o+2
    U_FMUL(vm, regs + 4);
    U_FSUB(vm, regs + 3);
    regs[2] = locals[1]; // this+1
    regs[3] = locals[7]; // o+3
    U_FMUL(vm, regs + 4);
    U_FADD(vm, regs + 3);
    regs[2] = locals[2]; // this+2
    regs[3] = locals[4]; // o+0
    U_FMUL(vm, regs + 4);
    U_FADD(vm, regs + 3);
    regs[2] = locals[3]; // this+3
    regs[3] = locals[6]; // o+2
    U_FMUL(vm, regs + 4);
    regs[3] = locals[0]; // this+0
    regs[4] = locals[5]; // o+1
    U_FMUL(vm, regs + 5);
    U_FADD(vm, regs + 4);
    regs[3] = locals[1]; // this+1
    regs[4] = locals[4]; // o+0
    U_FMUL(vm, regs + 5);
    U_FSUB(vm, regs + 4);
    regs[3] = locals[2]; // this+2
    regs[4] = locals[7]; // o+3
    U_FMUL(vm, regs + 5);
    U_FADD(vm, regs + 4);
    regs[3] = locals[3]; // this+3
    regs[4] = locals[7]; // o+3
    U_FMUL(vm, regs + 5);
    regs[4] = locals[0]; // this+0
    regs[5] = locals[4]; // o+0
    U_FMUL(vm, regs + 6);
    U_FSUB(vm, regs + 5);
    regs[4] = locals[1]; // this+1
    regs[5] = locals[5]; // o+1
    U_FMUL(vm, regs + 6);
    U_FSUB(vm, regs + 5);
    regs[4] = locals[2]; // this+2
    regs[5] = locals[6]; // o+2
    U_FMUL(vm, regs + 6);
    U_FSUB(vm, regs + 5);
    U_RETURNLOCAL(vm, 0, 4);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    Push(psp, regs[2]);
    Push(psp, regs[3]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator*
static void fun_382(VMRef vm, StackPtr psp) {
    Value regs[5];
    Value locals[5];
    locals[0] = *(psp - 5);
    locals[1] = *(psp - 4);
    locals[2] = *(psp - 3);
    locals[3] = *(psp - 2);
    locals[4] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3344, locals);
    U_STATEMENT(vm, regs + 0, 16, 27);
    regs[0] = locals[0]; // this+0
    regs[1] = locals[4]; // f
    U_FMUL(vm, regs + 2);
    regs[1] = locals[1]; // this+1
    regs[2] = locals[4]; // f
    U_FMUL(vm, regs + 3);
    regs[2] = locals[2]; // this+2
    regs[3] = locals[4]; // f
    U_FMUL(vm, regs + 4);
    regs[3] = locals[3]; // this+3
    regs[4] = locals[4]; // f
    U_FMUL(vm, regs + 5);
    U_RETURNLOCAL(vm, 0, 4);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    Push(psp, regs[2]);
    Push(psp, regs[3]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator*
static void fun_393(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3354, locals);
    U_STATEMENT(vm, regs + 0, 12, 26);
    regs[0] = locals[0]; // this
    U_PUSHFLD(vm, regs + 1, 0);
    regs[1] = locals[1]; // o
    U_PUSHFLD(vm, regs + 2, 0);
    U_IMUL(vm, regs + 2);
    U_NEWOBJECT(vm, regs + 1, 1309); // OC
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator*
static void fun_425(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3361, locals);
    U_STATEMENT(vm, regs + 0, 56, 26);
    regs[0] = locals[0]; // this+0
    regs[1] = locals[1]; // o+0
    U_IMUL(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator*
static void fun_443(VMRef vm, StackPtr psp) {
    Value regs[10];
    Value locals[8];
    locals[0] = *(psp - 8);
    locals[1] = *(psp - 7);
    locals[2] = *(psp - 6);
    locals[3] = *(psp - 5);
    locals[4] = *(psp - 4);
    locals[5] = *(psp - 3);
    locals[6] = *(psp - 2);
    locals[7] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3368, locals);
    U_STATEMENT(vm, regs + 0, 11, 29);
    regs[0] = locals[4];regs[1] = locals[5];regs[2] = locals[6];regs[3] = locals[7]; // o
    fun_70(vm, regs + 4); // call: xz
    U_PUSHINT(vm, regs + 2, 2);
    regs[3] = locals[0];regs[4] = locals[1];regs[5] = locals[2];regs[6] = locals[3]; // this
    U_POP(vm, regs + 7);
    U_POP(vm, regs + 6);
    U_PUSHINT(vm, regs + 5, 2);
    U_BCALLRETV(vm, regs + 6, 85, 1); // dot
    regs[1] = locals[4];regs[2] = locals[5];regs[3] = locals[6];regs[4] = locals[7]; // o
    fun_71(vm, regs + 5); // call: yw
    U_PUSHINT(vm, regs + 3, 2);
    regs[4] = locals[0];regs[5] = locals[1];regs[6] = locals[2];regs[7] = locals[3]; // this
    U_POP(vm, regs + 8);
    U_POP(vm, regs + 7);
    U_PUSHINT(vm, regs + 6, 2);
    U_BCALLRETV(vm, regs + 7, 85, 1); // dot
    regs[2] = locals[4];regs[3] = locals[5];regs[4] = locals[6];regs[5] = locals[7]; // o
    fun_70(vm, regs + 6); // call: xz
    U_PUSHINT(vm, regs + 4, 2);
    regs[5] = locals[0];regs[6] = locals[1];regs[7] = locals[2];regs[8] = locals[3]; // this
    fun_72(vm, regs + 9); // call: zw
    U_PUSHINT(vm, regs + 7, 2);
    U_BCALLRETV(vm, regs + 8, 85, 1); // dot
    regs[3] = locals[4];regs[4] = locals[5];regs[5] = locals[6];regs[6] = locals[7]; // o
    fun_71(vm, regs + 7); // call: yw
    U_PUSHINT(vm, regs + 5, 2);
    regs[6] = locals[0];regs[7] = locals[1];regs[8] = locals[2];regs[9] = locals[3]; // this
    fun_72(vm, regs + 10); // call: zw
    U_PUSHINT(vm, regs + 8, 2);
    U_BCALLRETV(vm, regs + 9, 85, 1); // dot
    U_RETURNLOCAL(vm, 0, 4);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    Push(psp, regs[2]);
    Push(psp, regs[3]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator*
static void fun_444(VMRef vm, StackPtr psp) {
    Value regs[8];
    Value locals[6];
    locals[0] = *(psp - 6);
    locals[1] = *(psp - 5);
    locals[2] = *(psp - 4);
    locals[3] = *(psp - 3);
    locals[4] = *(psp - 2);
    locals[5] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3381, locals);
    U_STATEMENT(vm, regs + 0, 15, 29);
    regs[0] = locals[4];regs[1] = locals[5]; // o
    U_PUSHINT(vm, regs + 2, 2);
    regs[3] = locals[0];regs[4] = locals[1];regs[5] = locals[2];regs[6] = locals[3]; // this
    U_POP(vm, regs + 7);
    U_POP(vm, regs + 6);
    U_PUSHINT(vm, regs + 5, 2);
    U_BCALLRETV(vm, regs + 6, 85, 1); // dot
    regs[1] = locals[4];regs[2] = locals[5]; // o
    U_PUSHINT(vm, regs + 3, 2);
    regs[4] = locals[0];regs[5] = locals[1];regs[6] = locals[2];regs[7] = locals[3]; // this
    fun_72(vm, regs + 8); // call: zw
    U_PUSHINT(vm, regs + 6, 2);
    U_BCALLRETV(vm, regs + 7, 85, 1); // dot
    U_RETURNLOCAL(vm, 0, 2);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator-
static void fun_383(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[4];
    locals[0] = *(psp - 4);
    locals[1] = *(psp - 3);
    locals[2] = *(psp - 2);
    locals[3] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3392, locals);
    U_STATEMENT(vm, regs + 0, 19, 27);
    regs[0] = locals[0]; // this+0
    U_FUMINUS(vm, regs + 1);
    regs[1] = locals[1]; // this+1
    U_FUMINUS(vm, regs + 2);
    regs[2] = locals[2]; // this+2
    U_FUMINUS(vm, regs + 3);
    regs[3] = locals[3]; // this+3
    U_RETURNLOCAL(vm, 0, 4);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    Push(psp, regs[2]);
    Push(psp, regs[3]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator-
static void fun_606(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3401, locals);
    U_STATEMENT(vm, regs + 0, 40, 26);
    regs[0] = locals[0]; // this
    U_PUSHFLD(vm, regs + 1, 0);
    U_IUMINUS(vm, regs + 1);
    U_NEWOBJECT(vm, regs + 1, 1309); // OC
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator-
static void fun_418(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3407, locals);
    U_STATEMENT(vm, regs + 0, 40, 26);
    regs[0] = locals[0]; // this
    U_PUSHFLD(vm, regs + 1, 0);
    U_IUMINUS(vm, regs + 1);
    U_NEWOBJECT(vm, regs + 1, 1309); // OC
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator+
static void fun_391(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3413, locals);
    U_STATEMENT(vm, regs + 0, 10, 26);
    regs[0] = locals[0]; // this
    U_PUSHFLD(vm, regs + 1, 0);
    regs[1] = locals[1]; // o
    U_PUSHFLD(vm, regs + 2, 0);
    U_IADD(vm, regs + 2);
    U_NEWOBJECT(vm, regs + 1, 1309); // OC
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator+
static void fun_423(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3420, locals);
    U_STATEMENT(vm, regs + 0, 54, 26);
    regs[0] = locals[0]; // this+0
    regs[1] = locals[1]; // o+0
    U_IADD(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator-
static void fun_392(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3427, locals);
    U_STATEMENT(vm, regs + 0, 11, 26);
    regs[0] = locals[0]; // this
    U_PUSHFLD(vm, regs + 1, 0);
    regs[1] = locals[1]; // o
    U_PUSHFLD(vm, regs + 2, 0);
    U_ISUB(vm, regs + 2);
    U_NEWOBJECT(vm, regs + 1, 1309); // OC
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator-
static void fun_424(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3434, locals);
    U_STATEMENT(vm, regs + 0, 55, 26);
    regs[0] = locals[0]; // this+0
    regs[1] = locals[1]; // o+0
    U_ISUB(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator/
static void fun_394(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3441, locals);
    U_STATEMENT(vm, regs + 0, 13, 26);
    regs[0] = locals[0]; // this
    U_PUSHFLD(vm, regs + 1, 0);
    regs[1] = locals[1]; // o
    U_PUSHFLD(vm, regs + 2, 0);
    U_IDIV(vm, regs + 2);
    U_NEWOBJECT(vm, regs + 1, 1309); // OC
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator/
static void fun_426(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3448, locals);
    U_STATEMENT(vm, regs + 0, 57, 26);
    regs[0] = locals[0]; // this+0
    regs[1] = locals[1]; // o+0
    U_IDIV(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator+=
static void fun_401(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3455, locals);
    U_STATEMENT(vm, regs + 0, 21, 26);
    regs[0] = locals[1]; // o
    U_PUSHFLD(vm, regs + 1, 0);
    regs[1] = locals[0]; // this
    U_LVAL_FLD(vm, regs + 2, 0);
    U_LV_IADD(vm, regs + 1);
    U_RETURNLOCAL(vm, 0, 0);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator==
static void fun_605(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3462, locals);
    U_STATEMENT(vm, regs + 0, 32, 26);
    regs[0] = locals[0]; // this
    U_PUSHFLD(vm, regs + 1, 0);
    regs[1] = locals[1]; // o
    U_PUSHFLD(vm, regs + 2, 0);
    U_IEQ(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator==
static void fun_411(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3469, locals);
    U_STATEMENT(vm, regs + 0, 32, 26);
    regs[0] = locals[0]; // this
    U_PUSHFLD(vm, regs + 1, 0);
    regs[1] = locals[1]; // o
    U_PUSHFLD(vm, regs + 2, 0);
    U_IEQ(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator==
static void fun_433(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3476, locals);
    U_STATEMENT(vm, regs + 0, 65, 26);
    regs[0] = locals[0]; // this+0
    regs[1] = locals[1]; // o+0
    U_IEQ(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator!=
static void fun_412(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3483, locals);
    U_STATEMENT(vm, regs + 0, 33, 26);
    regs[0] = locals[0]; // this
    U_PUSHFLD(vm, regs + 1, 0);
    regs[1] = locals[1]; // o
    U_PUSHFLD(vm, regs + 2, 0);
    U_INE(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator++
static void fun_419(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3490, locals);
    U_STATEMENT(vm, regs + 0, 41, 26);
    regs[0] = locals[0]; // this
    U_LVAL_FLD(vm, regs + 1, 0);
    U_LV_DUP(vm, regs + 0);
    U_LV_IPP(vm, regs + 1);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator--
static void fun_420(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3496, locals);
    U_STATEMENT(vm, regs + 0, 42, 26);
    regs[0] = locals[0]; // this
    U_LVAL_FLD(vm, regs + 1, 0);
    U_LV_DUP(vm, regs + 0);
    U_LV_IMM(vm, regs + 1);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator=
static void fun_421(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3502, locals);
    U_STATEMENT(vm, regs + 0, 44, 26);
    regs[0] = locals[1]; // o
    U_PUSHFLD(vm, regs + 1, 0);
    regs[1] = locals[0]; // this
    U_LVAL_FLD(vm, regs + 2, 0);
    U_LV_WRITE(vm, regs + 1);
    U_RETURNLOCAL(vm, 0, 0);
    Pop(psp);
    Pop(psp);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// operator[]
static void fun_422(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[2];
    locals[0] = *(psp - 2);
    locals[1] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3509, locals);
    U_STATEMENT(vm, regs + 0, 46, 26);
    regs[0] = locals[1]; // i
    U_PUSHINT(vm, regs + 1, 0);
    U_IEQ(vm, regs + 2);
    if (!U_JUMPFAIL(vm, regs + 1)) goto block48948;
    U_STATEMENT(vm, regs + 0, 46, 26);
    regs[0] = locals[0]; // this
    U_PUSHFLD(vm, regs + 1, 0);
    goto block48957;
    block48948:;
    U_STATEMENT(vm, regs + 0, 46, 26);
    U_PUSHINT(vm, regs + 0, 0);
    block48957:;
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function387
static void fun_442(VMRef vm, StackPtr psp) {
    Value regs[8];
    Value locals[8];
    locals[0] = lobster::NilVal();
    locals[1] = lobster::NilVal();
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    locals[5] = lobster::NilVal();
    locals[6] = lobster::NilVal();
    locals[7] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 3516, locals);
    U_STATEMENT(vm, regs + 0, 80, 26);
    U_PUSHINT(vm, regs + 0, 1);
    U_NEWOBJECT(vm, regs + 1, 1309); // OC
    SetLVal(vm, &locals[0]); // c1
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 81, 26);
    U_PUSHINT(vm, regs + 0, 2);
    U_NEWOBJECT(vm, regs + 1, 1309); // OC
    SetLVal(vm, &locals[1]); // c2
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 82, 26);
    U_PUSHINT(vm, regs + 0, 3);
    U_NEWOBJECT(vm, regs + 1, 1309); // OC
    SetLVal(vm, &locals[2]); // c3
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 83, 26);
    U_PUSHINT(vm, regs + 0, 4);
    U_NEWOBJECT(vm, regs + 1, 1309); // OC
    SetLVal(vm, &locals[3]); // c4
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 84, 26);
    regs[0] = locals[0]; // c1
    regs[1] = locals[1]; // c2
    fun_391(vm, regs + 2); // call: operator+
    regs[1] = locals[0]; // c1
    fun_392(vm, regs + 2); // call: operator-
    regs[1] = locals[1]; // c2
    fun_393(vm, regs + 2); // call: operator*
    regs[1] = locals[0]; // c1
    fun_394(vm, regs + 2); // call: operator/
    regs[1] = locals[3]; // c4
    fun_411(vm, regs + 2); // call: operator==
    U_ASSERT(vm, regs + 1, 84, 26, 578);
    U_STATEMENT(vm, regs + 0, 85, 26);
    regs[0] = locals[2]; // c3
    regs[1] = locals[0]; // c1
    fun_401(vm, regs + 2); // call: operator+=
    U_STATEMENT(vm, regs + 0, 86, 26);
    regs[0] = locals[2]; // c3
    regs[1] = locals[3]; // c4
    fun_605(vm, regs + 2); // call: operator==
    U_ASSERT(vm, regs + 1, 86, 26, 579);
    U_STATEMENT(vm, regs + 0, 87, 26);
    regs[0] = locals[2]; // c3
    fun_420(vm, regs + 1); // call: operator--
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 88, 26);
    regs[0] = locals[3]; // c4
    regs[1] = locals[2]; // c3
    fun_421(vm, regs + 2); // call: operator=
    U_STATEMENT(vm, regs + 0, 89, 26);
    regs[0] = locals[3]; // c4
    regs[1] = locals[2]; // c3
    fun_605(vm, regs + 2); // call: operator==
    U_ASSERT(vm, regs + 1, 89, 26, 580);
    U_STATEMENT(vm, regs + 0, 90, 26);
    regs[0] = locals[3]; // c4
    fun_419(vm, regs + 1); // call: operator++
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 91, 26);
    regs[0] = locals[3]; // c4
    fun_418(vm, regs + 1); // call: operator-
    fun_606(vm, regs + 1); // call: operator-
    regs[1] = locals[3]; // c4
    fun_411(vm, regs + 2); // call: operator==
    U_ASSERT(vm, regs + 1, 91, 26, 581);
    U_STATEMENT(vm, regs + 0, 92, 26);
    regs[0] = locals[3]; // c4
    regs[1] = locals[3]; // c4
    fun_412(vm, regs + 2); // call: operator!=
    U_PUSHINT(vm, regs + 1, 0);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 92, 26, 582);
    U_STATEMENT(vm, regs + 0, 93, 26);
    regs[0] = locals[3]; // c4
    U_PUSHINT(vm, regs + 1, 0);
    fun_422(vm, regs + 2); // call: operator[]
    U_PUSHINT(vm, regs + 1, 4);
    U_IEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 93, 26, 583);
    U_STATEMENT(vm, regs + 0, 96, 26);
    U_PUSHINT(vm, regs + 0, 1);
    SetLVal(vm, &locals[4]); // s1+0
    U_LV_WRITEV(vm, regs + 1, 1);
    U_STATEMENT(vm, regs + 0, 97, 26);
    U_PUSHINT(vm, regs + 0, 2);
    SetLVal(vm, &locals[5]); // s2+0
    U_LV_WRITEV(vm, regs + 1, 1);
    U_STATEMENT(vm, regs + 0, 98, 26);
    U_PUSHINT(vm, regs + 0, 3);
    SetLVal(vm, &locals[6]); // s3+0
    U_LV_WRITEV(vm, regs + 1, 1);
    U_STATEMENT(vm, regs + 0, 99, 26);
    U_PUSHINT(vm, regs + 0, 4);
    SetLVal(vm, &locals[7]); // s4+0
    U_LV_WRITEV(vm, regs + 1, 1);
    U_STATEMENT(vm, regs + 0, 100, 26);
    regs[0] = locals[4]; // s1
    regs[1] = locals[5]; // s2
    fun_423(vm, regs + 2); // call: operator+
    regs[1] = locals[4]; // s1
    fun_424(vm, regs + 2); // call: operator-
    regs[1] = locals[5]; // s2
    fun_425(vm, regs + 2); // call: operator*
    regs[1] = locals[4]; // s1
    fun_426(vm, regs + 2); // call: operator/
    regs[1] = locals[7]; // s4
    fun_433(vm, regs + 2); // call: operator==
    U_ASSERT(vm, regs + 1, 100, 26, 584);
    U_STATEMENT(vm, regs + 0, 103, 26);
    U_PUSHFLT(vm, regs + 0, 1065353216);
    U_PUSHFLT(vm, regs + 1, 1065353216);
    U_PUSHFLT(vm, regs + 2, 1065353216);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    fun_383(vm, regs + 4); // call: operator-
    U_PUSHFLT(vm, regs + 4, -1082130432);
    U_PUSHFLT(vm, regs + 5, -1082130432);
    U_PUSHFLT(vm, regs + 6, -1082130432);
    U_PUSHFLT(vm, regs + 7, 1065353216);
    U_STEQ(vm, regs + 8, 4);
    U_ASSERT(vm, regs + 1, 103, 26, 585);
    U_STATEMENT(vm, regs + 0, 104, 26);
    U_PUSHFLT(vm, regs + 0, 1065353216);
    U_PUSHFLT(vm, regs + 1, 1065353216);
    U_PUSHFLT(vm, regs + 2, 1065353216);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    U_PUSHFLT(vm, regs + 4, 1073741824);
    U_PUSHFLT(vm, regs + 5, 1073741824);
    U_PUSHFLT(vm, regs + 6, 1073741824);
    U_PUSHFLT(vm, regs + 7, 1065353216);
    fun_381(vm, regs + 8); // call: operator*
    U_PUSHFLT(vm, regs + 4, 1077936128);
    U_PUSHFLT(vm, regs + 5, 1077936128);
    U_PUSHFLT(vm, regs + 6, 1077936128);
    U_PUSHFLT(vm, regs + 7, -1063256064);
    U_STEQ(vm, regs + 8, 4);
    U_ASSERT(vm, regs + 1, 104, 26, 586);
    U_STATEMENT(vm, regs + 0, 105, 26);
    U_PUSHFLT(vm, regs + 0, 1065353216);
    U_PUSHFLT(vm, regs + 1, 1065353216);
    U_PUSHFLT(vm, regs + 2, 1065353216);
    U_PUSHFLT(vm, regs + 3, 1065353216);
    U_PUSHFLT(vm, regs + 4, 1073741824);
    fun_382(vm, regs + 5); // call: operator*
    U_PUSHFLT(vm, regs + 4, 1073741824);
    U_PUSHFLT(vm, regs + 5, 1073741824);
    U_PUSHFLT(vm, regs + 6, 1073741824);
    U_PUSHFLT(vm, regs + 7, 1073741824);
    U_STEQ(vm, regs + 8, 4);
    U_ASSERT(vm, regs + 1, 105, 26, 587);
    U_RETURNLOCAL(vm, 0, 0);
    DecVal(vm, locals[0]);
    DecVal(vm, locals[1]);
    DecVal(vm, locals[2]);
    DecVal(vm, locals[3]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// determinant
static void fun_445(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[4];
    locals[0] = *(psp - 4);
    locals[1] = *(psp - 3);
    locals[2] = *(psp - 2);
    locals[3] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3529, locals);
    U_STATEMENT(vm, regs + 0, 18, 29);
    regs[0] = locals[0]; // this+0
    regs[1] = locals[3]; // this+3
    U_FMUL(vm, regs + 2);
    regs[1] = locals[1]; // this+1
    regs[2] = locals[2]; // this+2
    U_FMUL(vm, regs + 3);
    U_FSUB(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// adjugate
static void fun_446(VMRef vm, StackPtr psp) {
    Value regs[4];
    Value locals[4];
    locals[0] = *(psp - 4);
    locals[1] = *(psp - 3);
    locals[2] = *(psp - 2);
    locals[3] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3538, locals);
    U_STATEMENT(vm, regs + 0, 21, 29);
    regs[0] = locals[3]; // this+3
    regs[1] = locals[1]; // this+1
    U_FUMINUS(vm, regs + 2);
    regs[2] = locals[2]; // this+2
    U_FUMINUS(vm, regs + 3);
    regs[3] = locals[0]; // this+0
    U_RETURNLOCAL(vm, 0, 4);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    Push(psp, regs[2]);
    Push(psp, regs[3]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// inverse
static void fun_447(VMRef vm, StackPtr psp) {
    Value regs[8];
    Value locals[4];
    locals[0] = *(psp - 4);
    locals[1] = *(psp - 3);
    locals[2] = *(psp - 2);
    locals[3] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3547, locals);
    U_STATEMENT(vm, regs + 0, 24, 29);
    regs[0] = locals[0];regs[1] = locals[1];regs[2] = locals[2];regs[3] = locals[3]; // this
    fun_446(vm, regs + 4); // call: adjugate
    regs[4] = locals[0];regs[5] = locals[1];regs[6] = locals[2];regs[7] = locals[3]; // this
    fun_445(vm, regs + 8); // call: determinant
    U_FVSDIV(vm, regs + 5, 4);
    U_RETURNLOCAL(vm, 0, 4);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    Push(psp, regs[1]);
    Push(psp, regs[2]);
    Push(psp, regs[3]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// trace
static void fun_448(VMRef vm, StackPtr psp) {
    Value regs[2];
    Value locals[4];
    locals[0] = *(psp - 4);
    locals[1] = *(psp - 3);
    locals[2] = *(psp - 2);
    locals[3] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3556, locals);
    U_STATEMENT(vm, regs + 0, 27, 29);
    regs[0] = locals[0]; // this+0
    regs[1] = locals[3]; // this+3
    U_FMUL(vm, regs + 2);
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function392
static void fun_449(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    locals[0] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 3565, locals);
    U_STATEMENT(vm, regs + 0, 6, 28);
    U_STATEMENT(vm, regs + 0, 6, 28);
    U_PUSHFUN(vm, regs + 0, 0, fun_450);
    SetLVal(vm, &locals[0]); // f
    U_LV_WRITE(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 253, 5);
    fun_450(vm, regs + 0); // call: function393
    U_RETURNLOCAL(vm, 0, 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function393
static void fun_450(VMRef vm, StackPtr psp) {
    Value regs[8];
    Value locals[10];
    locals[0] = lobster::NilVal();
    locals[1] = lobster::NilVal();
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    locals[5] = lobster::NilVal();
    locals[6] = lobster::NilVal();
    locals[7] = lobster::NilVal();
    locals[8] = lobster::NilVal();
    locals[9] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 3571, locals);
    U_STATEMENT(vm, regs + 0, 7, 28);
    U_PUSHFLT(vm, regs + 0, 1065353216);
    U_PUSHFLT(vm, regs + 1, 1073741824);
    U_PUSHFLT(vm, regs + 2, 1077936128);
    U_PUSHFLT(vm, regs + 3, 1082130432);
    SetLVal(vm, &locals[0]); // a+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 8, 28);
    U_PUSHFLT(vm, regs + 0, 1084227584);
    U_PUSHFLT(vm, regs + 1, 1086324736);
    U_PUSHFLT(vm, regs + 2, 1088421888);
    U_PUSHFLT(vm, regs + 3, 1090519040);
    SetLVal(vm, &locals[4]); // b+0
    U_LV_WRITEV(vm, regs + 4, 4);
    U_STATEMENT(vm, regs + 0, 10, 28);
    regs[0] = locals[0];regs[1] = locals[1];regs[2] = locals[2];regs[3] = locals[3]; // a
    regs[4] = locals[0];regs[5] = locals[1];regs[6] = locals[2];regs[7] = locals[3]; // a
    U_STEQ(vm, regs + 8, 4);
    U_ASSERT(vm, regs + 1, 10, 28, 588);
    U_STATEMENT(vm, regs + 0, 11, 28);
    regs[0] = locals[0];regs[1] = locals[1];regs[2] = locals[2];regs[3] = locals[3]; // a
    U_PUSHFLT(vm, regs + 4, 1065353216);
    U_PUSHFLT(vm, regs + 5, 1073741824);
    U_PUSHFLT(vm, regs + 6, 1077936128);
    U_PUSHFLT(vm, regs + 7, 1082130432);
    U_STEQ(vm, regs + 8, 4);
    U_ASSERT(vm, regs + 1, 11, 28, 589);
    U_STATEMENT(vm, regs + 0, 12, 28);
    regs[0] = locals[0];regs[1] = locals[1];regs[2] = locals[2];regs[3] = locals[3]; // a
    regs[4] = locals[4];regs[5] = locals[5];regs[6] = locals[6];regs[7] = locals[7]; // b
    U_STNE(vm, regs + 8, 4);
    U_ASSERT(vm, regs + 1, 12, 28, 590);
    U_STATEMENT(vm, regs + 0, 14, 28);
    U_PUSHFLT(vm, regs + 0, 1084227584);
    regs[1] = locals[0];regs[2] = locals[1];regs[3] = locals[2];regs[4] = locals[3]; // a
    U_SFVADD(vm, regs + 5, 4);
    U_PUSHFLT(vm, regs + 4, 1086324736);
    U_PUSHFLT(vm, regs + 5, 1088421888);
    U_PUSHFLT(vm, regs + 6, 1090519040);
    U_PUSHFLT(vm, regs + 7, 1091567616);
    U_STEQ(vm, regs + 8, 4);
    U_ASSERT(vm, regs + 1, 14, 28, 591);
    U_STATEMENT(vm, regs + 0, 15, 28);
    regs[0] = locals[0];regs[1] = locals[1];regs[2] = locals[2];regs[3] = locals[3]; // a
    U_PUSHFLT(vm, regs + 4, 1084227584);
    U_FVSADD(vm, regs + 5, 4);
    U_PUSHFLT(vm, regs + 4, 1086324736);
    U_PUSHFLT(vm, regs + 5, 1088421888);
    U_PUSHFLT(vm, regs + 6, 1090519040);
    U_PUSHFLT(vm, regs + 7, 1091567616);
    U_STEQ(vm, regs + 8, 4);
    U_ASSERT(vm, regs + 1, 15, 28, 592);
    U_STATEMENT(vm, regs + 0, 16, 28);
    regs[0] = locals[0];regs[1] = locals[1];regs[2] = locals[2];regs[3] = locals[3]; // a
    regs[4] = locals[4];regs[5] = locals[5];regs[6] = locals[6];regs[7] = locals[7]; // b
    U_FVVADD(vm, regs + 8, 4);
    U_PUSHFLT(vm, regs + 4, 1086324736);
    U_PUSHFLT(vm, regs + 5, 1090519040);
    U_PUSHFLT(vm, regs + 6, 1092616192);
    U_PUSHFLT(vm, regs + 7, 1094713344);
    U_STEQ(vm, regs + 8, 4);
    U_ASSERT(vm, regs + 1, 16, 28, 593);
    U_STATEMENT(vm, regs + 0, 18, 28);
    U_PUSHFLT(vm, regs + 0, 1084227584);
    regs[1] = locals[0];regs[2] = locals[1];regs[3] = locals[2];regs[4] = locals[3]; // a
    U_SFVSUB(vm, regs + 5, 4);
    U_PUSHFLT(vm, regs + 4, 1082130432);
    U_PUSHFLT(vm, regs + 5, 1077936128);
    U_PUSHFLT(vm, regs + 6, 1073741824);
    U_PUSHFLT(vm, regs + 7, 1065353216);
    U_STEQ(vm, regs + 8, 4);
    U_ASSERT(vm, regs + 1, 18, 28, 594);
    U_STATEMENT(vm, regs + 0, 19, 28);
    regs[0] = locals[0];regs[1] = locals[1];regs[2] = locals[2];regs[3] = locals[3]; // a
    U_PUSHFLT(vm, regs + 4, 1084227584);
    U_FVSSUB(vm, regs + 5, 4);
    U_PUSHFLT(vm, regs + 4, -1065353216);
    U_PUSHFLT(vm, regs + 5, -1069547520);
    U_PUSHFLT(vm, regs + 6, -1073741824);
    U_PUSHFLT(vm, regs + 7, -1082130432);
    U_STEQ(vm, regs + 8, 4);
    U_ASSERT(vm, regs + 1, 19, 28, 595);
    U_STATEMENT(vm, regs + 0, 20, 28);
    regs[0] = locals[4];regs[1] = locals[5];regs[2] = locals[6];regs[3] = locals[7]; // b
    regs[4] = locals[0];regs[5] = locals[1];regs[6] = locals[2];regs[7] = locals[3]; // a
    U_FVVSUB(vm, regs + 8, 4);
    U_PUSHFLT(vm, regs + 4, 1082130432);
    U_PUSHFLT(vm, regs + 5, 1082130432);
    U_PUSHFLT(vm, regs + 6, 1082130432);
    U_PUSHFLT(vm, regs + 7, 1082130432);
    U_STEQ(vm, regs + 8, 4);
    U_ASSERT(vm, regs + 1, 20, 28, 596);
    U_STATEMENT(vm, regs + 0, 22, 28);
    U_PUSHFLT(vm, regs + 0, 1084227584);
    regs[1] = locals[0];regs[2] = locals[1];regs[3] = locals[2];regs[4] = locals[3]; // a
    U_SFVMUL(vm, regs + 5, 4);
    U_PUSHFLT(vm, regs + 4, 1084227584);
    U_PUSHFLT(vm, regs + 5, 1092616192);
    U_PUSHFLT(vm, regs + 6, 1097859072);
    U_PUSHFLT(vm, regs + 7, 1101004800);
    U_STEQ(vm, regs + 8, 4);
    U_ASSERT(vm, regs + 1, 22, 28, 597);
    U_STATEMENT(vm, regs + 0, 26, 28);
    regs[0] = locals[0];regs[1] = locals[1];regs[2] = locals[2];regs[3] = locals[3]; // a
    regs[4] = locals[4];regs[5] = locals[5];regs[6] = locals[6];regs[7] = locals[7]; // b
    fun_443(vm, regs + 8); // call: operator*
    U_PUSHFLT(vm, regs + 4, 1100480512);
    U_PUSHFLT(vm, regs + 5, 1102053376);
    U_PUSHFLT(vm, regs + 6, 1110179840);
    U_PUSHFLT(vm, regs + 7, 1112014848);
    U_STEQ(vm, regs + 8, 4);
    U_ASSERT(vm, regs + 1, 26, 28, 598);
    U_STATEMENT(vm, regs + 0, 27, 28);
    regs[0] = locals[4];regs[1] = locals[5];regs[2] = locals[6];regs[3] = locals[7]; // b
    regs[4] = locals[0];regs[5] = locals[1];regs[6] = locals[2];regs[7] = locals[3]; // a
    fun_443(vm, regs + 8); // call: operator*
    U_PUSHFLT(vm, regs + 4, 1102577664);
    U_PUSHFLT(vm, regs + 5, 1107820544);
    U_PUSHFLT(vm, regs + 6, 1106771968);
    U_PUSHFLT(vm, regs + 7, 1110966272);
    U_STEQ(vm, regs + 8, 4);
    U_ASSERT(vm, regs + 1, 27, 28, 599);
    U_STATEMENT(vm, regs + 0, 29, 28);
    regs[0] = locals[0];regs[1] = locals[1];regs[2] = locals[2];regs[3] = locals[3]; // a
    fun_445(vm, regs + 4); // call: determinant
    U_PUSHFLT(vm, regs + 1, -1073741824);
    U_FEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 29, 28, 600);
    U_STATEMENT(vm, regs + 0, 30, 28);
    regs[0] = locals[0];regs[1] = locals[1];regs[2] = locals[2];regs[3] = locals[3]; // a
    fun_448(vm, regs + 4); // call: trace
    U_PUSHFLT(vm, regs + 1, 1082130432);
    U_FEQ(vm, regs + 2);
    U_ASSERT(vm, regs + 1, 30, 28, 601);
    U_STATEMENT(vm, regs + 0, 31, 28);
    regs[0] = locals[0];regs[1] = locals[1];regs[2] = locals[2];regs[3] = locals[3]; // a
    fun_446(vm, regs + 4); // call: adjugate
    U_PUSHFLT(vm, regs + 4, 1082130432);
    U_PUSHFLT(vm, regs + 5, -1073741824);
    U_PUSHFLT(vm, regs + 6, -1069547520);
    U_PUSHFLT(vm, regs + 7, 1065353216);
    U_STEQ(vm, regs + 8, 4);
    U_ASSERT(vm, regs + 1, 31, 28, 602);
    U_STATEMENT(vm, regs + 0, 32, 28);
    regs[0] = locals[0];regs[1] = locals[1];regs[2] = locals[2];regs[3] = locals[3]; // a
    fun_447(vm, regs + 4); // call: inverse
    U_PUSHFLT(vm, regs + 4, -1073741824);
    U_PUSHFLT(vm, regs + 5, 1065353216);
    U_PUSHFLT(vm, regs + 6, 1069547520);
    U_PUSHFLT(vm, regs + 7, -1090519040);
    U_STEQ(vm, regs + 8, 4);
    U_ASSERT(vm, regs + 1, 32, 28, 603);
    U_STATEMENT(vm, regs + 0, 34, 28);
    regs[0] = locals[0];regs[1] = locals[1];regs[2] = locals[2];regs[3] = locals[3]; // a
    regs[4] = locals[0];regs[5] = locals[1];regs[6] = locals[2];regs[7] = locals[3]; // a
    fun_447(vm, regs + 8); // call: inverse
    fun_443(vm, regs + 8); // call: operator*
    U_PUSHVARVF(vm, regs + 4, 1150, 4); // mat2x2_identity+0
    U_STEQ(vm, regs + 8, 4);
    U_ASSERT(vm, regs + 1, 34, 28, 604);
    U_STATEMENT(vm, regs + 0, 38, 28);
    U_PUSHFLT(vm, regs + 0, 1073741824);
    U_PUSHFLT(vm, regs + 1, 1077936128);
    SetLVal(vm, &locals[8]); // c+0
    U_LV_WRITEV(vm, regs + 2, 2);
    U_STATEMENT(vm, regs + 0, 40, 28);
    regs[0] = locals[0];regs[1] = locals[1];regs[2] = locals[2];regs[3] = locals[3]; // a
    regs[4] = locals[8];regs[5] = locals[9]; // c
    fun_444(vm, regs + 6); // call: operator*
    U_PUSHFLT(vm, regs + 2, 1090519040);
    U_PUSHFLT(vm, regs + 3, 1099956224);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 40, 28, 605);
    U_STATEMENT(vm, regs + 0, 41, 28);
    regs[0] = locals[4];regs[1] = locals[5];regs[2] = locals[6];regs[3] = locals[7]; // b
    regs[4] = locals[8];regs[5] = locals[9]; // c
    fun_444(vm, regs + 6); // call: operator*
    U_PUSHFLT(vm, regs + 2, 1105199104);
    U_PUSHFLT(vm, regs + 3, 1108869120);
    U_STEQ(vm, regs + 4, 2);
    U_ASSERT(vm, regs + 1, 41, 28, 606);
    U_RETURNLOCAL(vm, 0, 0);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function394
static void fun_451(VMRef vm, StackPtr psp) {
    Value regs[7];
    Value keepvar[6];
    Value locals[9];
    locals[0] = lobster::NilVal();
    locals[1] = lobster::NilVal();
    locals[2] = lobster::NilVal();
    locals[3] = lobster::NilVal();
    locals[4] = lobster::NilVal();
    locals[5] = lobster::NilVal();
    locals[6] = lobster::NilVal();
    locals[7] = lobster::NilVal();
    locals[8] = lobster::NilVal();
    PushFunId(vm, funinfo_table + 3586, locals);
    keepvar[0] = lobster::NilVal();
    keepvar[1] = lobster::NilVal();
    keepvar[2] = lobster::NilVal();
    keepvar[3] = lobster::NilVal();
    keepvar[4] = lobster::NilVal();
    keepvar[5] = lobster::NilVal();
    U_STATEMENT(vm, regs + 0, 6, 30);
    U_STATEMENT(vm, regs + 0, 6, 30);
    U_STATEMENT(vm, regs + 0, 9, 30);
    U_STATEMENT(vm, regs + 0, 9, 30);
    U_STATEMENT(vm, regs + 0, 12, 30);
    U_STATEMENT(vm, regs + 0, 12, 30);
    U_STATEMENT(vm, regs + 0, 15, 30);
    U_STATEMENT(vm, regs + 0, 15, 30);
    U_STATEMENT(vm, regs + 0, 18, 30);
    U_STATEMENT(vm, regs + 0, 18, 30);
    U_STATEMENT(vm, regs + 0, 21, 30);
    U_STATEMENT(vm, regs + 0, 21, 30);
    U_STATEMENT(vm, regs + 0, 39, 30);
    U_PUSHINT(vm, regs + 0, 0);
    U_PUSHINT(vm, regs + 1, 1);
    U_NEWOBJECT(vm, regs + 2, 1623); // B
    U_PUSHINT(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 2, 2);
    U_PUSHINT(vm, regs + 3, 3);
    U_NEWOBJECT(vm, regs + 4, 1635); // D
    U_PUSHINT(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 3, 2);
    U_PUSHINT(vm, regs + 4, 4);
    U_NEWOBJECT(vm, regs + 5, 1662); // E
    U_PUSHINT(vm, regs + 3, 0);
    U_PUSHINT(vm, regs + 4, 2);
    U_PUSHINT(vm, regs + 5, 4);
    U_PUSHINT(vm, regs + 6, 5);
    U_NEWOBJECT(vm, regs + 7, 1677); // F
    U_NEWVEC(vm, regs + 4, 1336, 4);
    SetLVal(vm, &locals[0]); // tests
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 42, 30);
    regs[0] = locals[0]; // tests
    U_PUSHFUN(vm, regs + 1, 0, fun_452);
    fun_608(vm, regs + 2); // call: map
    SetLVal(vm, &locals[1]); // results1
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 50, 30);
    regs[0] = locals[1]; // results1
    U_PUSHINT(vm, regs + 1, 1);
    U_PUSHINT(vm, regs + 2, 3);
    U_PUSHINT(vm, regs + 3, 4);
    U_PUSHINT(vm, regs + 4, 5);
    U_NEWVEC(vm, regs + 5, 7, 4);
    keepvar[0] = TopM(regs + 2, 0);
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 50, 30, 607);
    U_STATEMENT(vm, regs + 0, 53, 30);
    regs[0] = locals[0]; // tests
    U_PUSHFUN(vm, regs + 1, 0, fun_453);
    fun_609(vm, regs + 2); // call: map
    SetLVal(vm, &locals[2]); // results2
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 60, 30);
    regs[0] = locals[2]; // results2
    U_PUSHINT(vm, regs + 1, 1);
    U_PUSHINT(vm, regs + 2, 3);
    U_PUSHINT(vm, regs + 3, 4);
    U_PUSHINT(vm, regs + 4, 4);
    U_NEWVEC(vm, regs + 5, 7, 4);
    keepvar[1] = TopM(regs + 2, 0);
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 60, 30, 608);
    U_STATEMENT(vm, regs + 0, 63, 30);
    regs[0] = locals[0]; // tests
    U_PUSHFUN(vm, regs + 1, 0, fun_454);
    fun_610(vm, regs + 2); // call: map
    SetLVal(vm, &locals[3]); // results3
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 69, 30);
    regs[0] = locals[3]; // results3
    U_PUSHINT(vm, regs + 1, 1);
    U_PUSHINT(vm, regs + 2, 2);
    U_PUSHINT(vm, regs + 3, 2);
    U_PUSHINT(vm, regs + 4, 2);
    U_NEWVEC(vm, regs + 5, 7, 4);
    keepvar[2] = TopM(regs + 2, 0);
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 69, 30, 609);
    U_STATEMENT(vm, regs + 0, 72, 30);
    regs[0] = locals[0]; // tests
    U_PUSHFUN(vm, regs + 1, 0, fun_455);
    fun_611(vm, regs + 2); // call: map
    SetLVal(vm, &locals[4]); // results4
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 77, 30);
    regs[0] = locals[4]; // results4
    U_PUSHINT(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 2, 0);
    U_PUSHINT(vm, regs + 3, 0);
    U_PUSHINT(vm, regs + 4, 5);
    U_NEWVEC(vm, regs + 5, 7, 4);
    keepvar[3] = TopM(regs + 2, 0);
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 77, 30, 610);
    U_STATEMENT(vm, regs + 0, 82, 30);
    U_PUSHINT(vm, regs + 0, 1);
    U_PUSHINT(vm, regs + 1, 0);
    U_NEWVEC(vm, regs + 2, 1338, 2);
    SetLVal(vm, &locals[5]); // btests
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 84, 30);
    regs[0] = locals[5]; // btests
    U_PUSHFUN(vm, regs + 1, 0, fun_456);
    fun_612(vm, regs + 2); // call: map
    SetLVal(vm, &locals[6]); // results5
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 91, 30);
    regs[0] = locals[6]; // results5
    U_PUSHSTR(vm, regs + 1, 611); // "t"
    U_INCREF(vm, regs + 2, 0);
    U_PUSHSTR(vm, regs + 2, 612); // "f"
    U_INCREF(vm, regs + 3, 0);
    U_NEWVEC(vm, regs + 3, 11, 2);
    keepvar[4] = TopM(regs + 2, 0);
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 91, 30, 613);
    U_STATEMENT(vm, regs + 0, 93, 30);
    U_PUSHINT(vm, regs + 0, 1);
    U_PUSHINT(vm, regs + 1, 0);
    U_PUSHINT(vm, regs + 2, 2);
    U_NEWVEC(vm, regs + 3, 1338, 3);
    SetLVal(vm, &locals[7]); // btests2
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 95, 30);
    regs[0] = locals[7]; // btests2
    U_PUSHFUN(vm, regs + 1, 0, fun_457);
    fun_613(vm, regs + 2); // call: map
    SetLVal(vm, &locals[8]); // results6
    U_LV_WRITEREF(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 110, 30);
    regs[0] = locals[8]; // results6
    U_PUSHSTR(vm, regs + 1, 614); // "t"
    U_INCREF(vm, regs + 2, 0);
    U_PUSHSTR(vm, regs + 2, 615); // "f"
    U_INCREF(vm, regs + 3, 0);
    U_PUSHSTR(vm, regs + 3, 616); // "?"
    U_INCREF(vm, regs + 4, 0);
    U_NEWVEC(vm, regs + 4, 11, 3);
    keepvar[5] = TopM(regs + 2, 0);
    U_BCALLRET2(vm, regs + 2, 14, 1); // equal
    U_ASSERT(vm, regs + 1, 110, 30, 617);
    U_RETURNLOCAL(vm, 0, 0);
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

// function395
static void fun_452(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3600, locals);
    U_STATEMENT(vm, regs + 0, 43, 30);
    regs[0] = locals[0]; // t
    switch (GetTypeSwitchID(vm, regs[0], 0)) {
    case 0:
    U_STATEMENT(vm, regs + 0, 44, 30);
    regs[0] = locals[0]; // t
    U_PUSHFLD(vm, regs + 1, 1);
    goto block50891;
    case 1:
    U_STATEMENT(vm, regs + 0, 45, 30);
    regs[0] = locals[0]; // t
    U_PUSHFLD(vm, regs + 1, 2);
    goto block50891;
    case 2:
    U_STATEMENT(vm, regs + 0, 46, 30);
    regs[0] = locals[0]; // t
    U_PUSHFLD(vm, regs + 1, 2);
    goto block50891;
    case 3:
    U_STATEMENT(vm, regs + 0, 47, 30);
    regs[0] = locals[0]; // t
    U_PUSHFLD(vm, regs + 1, 3);
    } // switch
    block50891:;
    U_RETURNLOCAL(vm, 0, 1);
    DecVal(vm, locals[0]);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function396
static void fun_453(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3606, locals);
    U_STATEMENT(vm, regs + 0, 54, 30);
    regs[0] = locals[0]; // t
    switch (GetTypeSwitchID(vm, regs[0], 1)) {
    case 0:
    U_STATEMENT(vm, regs + 0, 55, 30);
    regs[0] = locals[0]; // t
    U_PUSHFLD(vm, regs + 1, 1);
    goto block50965;
    case 1:
    U_STATEMENT(vm, regs + 0, 56, 30);
    regs[0] = locals[0]; // t
    U_PUSHFLD(vm, regs + 1, 2);
    goto block50965;
    case 2:
    U_STATEMENT(vm, regs + 0, 57, 30);
    regs[0] = locals[0]; // t
    U_PUSHFLD(vm, regs + 1, 2);
    } // switch
    block50965:;
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function397
static void fun_454(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3612, locals);
    U_STATEMENT(vm, regs + 0, 64, 30);
    regs[0] = locals[0]; // t
    switch (GetTypeSwitchID(vm, regs[0], 2)) {
    case 0:
    U_STATEMENT(vm, regs + 0, 65, 30);
    regs[0] = locals[0]; // t
    U_PUSHFLD(vm, regs + 1, 1);
    goto block51023;
    case 1:
    U_STATEMENT(vm, regs + 0, 66, 30);
    regs[0] = locals[0]; // t
    U_PUSHFLD(vm, regs + 1, 1);
    } // switch
    block51023:;
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function398
static void fun_455(VMRef vm, StackPtr psp) {
    Value regs[1];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3618, locals);
    U_STATEMENT(vm, regs + 0, 73, 30);
    regs[0] = locals[0]; // t
    switch (GetTypeSwitchID(vm, regs[0], 3)) {
    case 0:
    U_STATEMENT(vm, regs + 0, 75, 30);
    regs[0] = locals[0]; // t
    U_PUSHFLD(vm, regs + 1, 3);
    goto block51081;
    case 1:default:
    U_STATEMENT(vm, regs + 0, 74, 30);
    regs[0] = locals[0]; // t
    U_PUSHFLD(vm, regs + 1, 0);
    } // switch
    block51081:;
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function399
static void fun_456(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3624, locals);
    U_STATEMENT(vm, regs + 0, 85, 30);
    regs[0] = locals[0]; // t
    U_DUP(vm, regs + 1);
    U_PUSHINT(vm, regs + 2, 1);
    U_IEQ(vm, regs + 3);
    if (!U_JUMPFAIL(vm, regs + 2)) goto block51127;
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 86, 30);
    U_PUSHSTR(vm, regs + 0, 618); // "t"
    U_INCREF(vm, regs + 1, 0);
    goto block51160;
    block51127:;
    U_DUP(vm, regs + 1);
    U_PUSHINT(vm, regs + 2, 0);
    U_IEQ(vm, regs + 3);
    if (!U_JUMPFAIL(vm, regs + 2)) goto block51154;
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 87, 30);
    U_PUSHSTR(vm, regs + 0, 619); // "f"
    U_INCREF(vm, regs + 1, 0);
    goto block51160;
    block51154:;
    U_POP(vm, regs + 1);
    U_ENUM_RANGE_ERR(vm, regs + 0);
    block51160:;
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
    goto epilogue;
    epilogue:;
    PopFunId(vm);
}

// function400
static void fun_457(VMRef vm, StackPtr psp) {
    Value regs[3];
    Value locals[1];
    locals[0] = *(psp - 1);
    PushFunId(vm, funinfo_table + 3630, locals);
    U_STATEMENT(vm, regs + 0, 96, 30);
    regs[0] = locals[0]; // t
    U_DUP(vm, regs + 1);
    U_PUSHINT(vm, regs + 2, 1);
    U_IEQ(vm, regs + 3);
    if (!U_JUMPFAIL(vm, regs + 2)) goto block51206;
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 97, 30);
    U_PUSHSTR(vm, regs + 0, 620); // "t"
    U_INCREF(vm, regs + 1, 0);
    goto block51247;
    block51206:;
    U_DUP(vm, regs + 1);
    U_PUSHINT(vm, regs + 2, 0);
    U_IEQ(vm, regs + 3);
    if (!U_JUMPFAIL(vm, regs + 2)) goto block51233;
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 98, 30);
    U_PUSHSTR(vm, regs + 0, 621); // "f"
    U_INCREF(vm, regs + 1, 0);
    goto block51247;
    block51233:;
    U_POP(vm, regs + 1);
    U_STATEMENT(vm, regs + 0, 99, 30);
    U_PUSHSTR(vm, regs + 0, 622); // "?"
    U_INCREF(vm, regs + 1, 0);
    block51247:;
    U_RETURNLOCAL(vm, 0, 1);
    Pop(psp);
    Push(psp, regs[0]);
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
    fun_156,
    fun_157,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    fun_216,
    fun_217,
    fun_516,
    fun_218,
    fun_220,
    fun_215,
    fun_221,
    fun_219,
    fun_222,
    fun_223,
    fun_517,
    fun_224,
    fun_226,
    fun_227,
    fun_221,
    fun_225,
    0,
    0,
    fun_287,
    fun_288,
    fun_289,
    fun_290,
    fun_291,
    fun_292,
    fun_293,
    fun_294,
    fun_295,
    fun_296,
    fun_297,
    fun_298,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    fun_312,
    fun_312,
    fun_313,
    fun_313,
    fun_313,
    fun_314,
    fun_314,
    fun_315,
    fun_315,
    fun_317,
    fun_317,
    fun_317,
    fun_320,
    fun_320,
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
    fun_228,
    fun_217,
    fun_516,
    fun_218,
    fun_220,
    fun_215,
    fun_221,
    fun_219,
    0
};

const int funinfo_table[] = {
    0, 24, 0, 241, 0, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 
    85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 
    101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 
    117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 
    133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 
    149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 
    165, 166, 167, 168, 169, 170, 171, 172, 173, 798, 799, 800, 801, 802, 803, 804, 
    805, 806, 807, 808, 809, 810, 811, 812, 813, 814, 815, 816, 817, 818, 819, 820, 
    821, 822, 823, 824, 825, 826, 827, 828, 829, 830, 831, 832, 833, 834, 835, 836, 
    837, 838, 839, 840, 841, 842, 843, 844, 845, 846, 847, 848, 849, 850, 851, 852, 
    853, 854, 855, 856, 857, 858, 859, 860, 861, 862, 863, 864, 865, 866, 867, 868, 
    869, 870, 871, 872, 873, 874, 875, 876, 877, 878, 879, 880, 881, 882, 883, 884, 
    885, 886, 887, 888, 889, 890, 891, 892, 893, 894, 895, 896, 897, 898, 899, 900, 
    901, 902, 903, 904, 905, 906, 907, 908, 909, 910, 911, 912, 913, 914, 915, 916, 
    917, 918, 919, 920, 921, 922, 923, 924, 925, 926, 927, 928, 929, 1077, 1078, 1079, 
    1080, 1150, 1151, 1152, 1153, 0, 3, 1, 2, 1, 2, 0, 0, 4, 3, 2, 
    3, 4, 0, 0, 5, 2, 0, 0, 1, 6, 2, 2, 5, 6, 0, 1, 
    613, 5, 2, 1631, 1632, 3, 1633, 1634, 1635, 1, 612, 5, 2, 1626, 1627, 3, 
    1628, 1629, 1630, 1, 611, 5, 2, 1621, 1622, 3, 1623, 1624, 1625, 1, 610, 5, 
    2, 1616, 1617, 3, 1618, 1619, 1620, 1, 609, 5, 2, 1611, 1612, 3, 1613, 1614, 
    1615, 1, 608, 5, 2, 1606, 1607, 3, 1608, 1609, 1610, 1, 601, 8, 2, 1592, 
    1593, 5, 1594, 1595, 1596, 1597, 1598, 1, 600, 5, 2, 1587, 1588, 3, 1589, 1590, 
    1591, 1, 599, 7, 2, 1582, 1583, 3, 1584, 1585, 1586, 1, 598, 5, 2, 1577, 
    1578, 3, 1579, 1580, 1581, 1, 590, 5, 2, 1543, 1544, 3, 1545, 1546, 1547, 1, 
    589, 5, 2, 1537, 1538, 4, 1539, 1540, 1541, 1542, 1, 584, 6, 2, 1512, 1513, 
    4, 1514, 1515, 1516, 1517, 1, 583, 5, 2, 1507, 1508, 3, 1509, 1510, 1511, 1, 
    582, 5, 2, 1502, 1503, 3, 1504, 1505, 1506, 1, 581, 5, 2, 1497, 1498, 3, 
    1499, 1500, 1501, 1, 579, 5, 2, 1489, 1490, 3, 1491, 1492, 1493, 1, 578, 5, 
    2, 1484, 1485, 3, 1486, 1487, 1488, 1, 576, 5, 2, 1478, 1479, 3, 1480, 1481, 
    1482, 1, 573, 5, 2, 1472, 1473, 3, 1474, 1475, 1476, 1, 570, 5, 2, 1464, 
    1465, 3, 1466, 1467, 1468, 1, 563, 5, 2, 1434, 1435, 4, 1436, 1437, 1438, 1439, 
    1, 562, 6, 2, 1428, 1429, 4, 1430, 1431, 1432, 1433, 1, 561, 5, 2, 1423, 
    1424, 3, 1425, 1426, 1427, 1, 560, 5, 2, 1418, 1419, 3, 1420, 1421, 1422, 1, 
    559, 5, 2, 1413, 1414, 3, 1415, 1416, 1417, 1, 555, 5, 2, 1392, 1393, 3, 
    1394, 1395, 1396, 1, 540, 5, 2, 1355, 1356, 3, 1357, 1358, 1359, 1, 539, 5, 
    2, 1350, 1351, 3, 1352, 1353, 1354, 1, 529, 5, 2, 1324, 1325, 3, 1326, 1327, 
    1328, 1, 502, 5, 2, 1286, 1287, 3, 1288, 1289, 1290, 1, 499, 5, 2, 1279, 
    1280, 3, 1281, 1282, 1283, 1, 489, 5, 2, 1241, 1242, 3, 1243, 1244, 1245, 1, 
    482, 5, 2, 1218, 1219, 3, 1220, 1221, 1222, 1, 479, 5, 2, 1208, 1209, 3, 
    1210, 1211, 1212, 1, 468, 5, 2, 1189, 1190, 3, 1191, 1192, 1193, 1, 7, 5, 
    2, 7, 8, 3, 9, 10, 11, 1, 564, 6, 2, 1440, 1441, 4, 1442, 1443, 
    1444, 1445, 0, 10, 6, 2, 12, 13, 4, 14, 15, 16, 17, 0, 592, 5, 
    2, 1554, 1555, 4, 1556, 1557, 1558, 1559, 0, 587, 7, 2, 1526, 1527, 6, 1528, 
    1529, 1530, 1531, 1532, 1533, 0, 13, 5, 2, 18, 19, 4, 20, 21, 22, 23, 
    0, 597, 3, 2, 1573, 1574, 2, 1575, 1576, 0, 596, 3, 2, 1569, 1570, 2, 
    1571, 1572, 0, 565, 3, 2, 1446, 1447, 2, 1448, 1449, 0, 15, 3, 2, 24, 
    25, 2, 26, 27, 0, 569, 4, 3, 1460, 1461, 1462, 1, 1463, 0, 568, 4, 
    3, 1456, 1457, 1458, 1, 1459, 0, 567, 4, 3, 1452, 1453, 1454, 1, 1455, 0, 
    556, 4, 3, 1397, 1398, 1399, 1, 1400, 0, 478, 4, 3, 1204, 1205, 1206, 1, 
    1207, 0, 17, 4, 3, 28, 29, 30, 1, 31, 0, 595, 6, 2, 1565, 1566, 
    2, 1567, 1568, 0, 19, 6, 2, 32, 33, 2, 34, 35, 0, 594, 3, 2, 
    1561, 1562, 2, 1563, 1564, 0, 23, 3, 2, 36, 37, 2, 38, 39, 0, 26, 
    4, 2, 40, 41, 6, 42, 43, 44, 45, 46, 47, 0, 469, 3, 1, 1194, 
    1, 1195, 0, 29, 3, 1, 48, 1, 49, 0, 30, 2, 2, 50, 51, 0, 
    0, 591, 3, 2, 1548, 1549, 4, 1550, 1551, 1552, 1553, 4, 585, 4, 2, 1518, 
    1519, 6, 1520, 1521, 1522, 1523, 1524, 1525, 4, 48, 3, 2, 52, 53, 4, 54, 
    55, 56, 57, 4, 593, 2, 1, 1560, 0, 0, 588, 6, 3, 1534, 1535, 1536, 
    0, 0, 49, 2, 1, 58, 0, 0, 50, 2, 2, 59, 60, 0, 0, 51, 
    4, 2, 61, 62, 5, 63, 64, 65, 66, 67, 0, 52, 6, 2, 68, 69, 
    3, 70, 71, 72, 0, 473, 0, 1, 1196, 0, 0, 70, 2, 4, 174, 175, 
    176, 177, 0, 0, 71, 2, 4, 178, 179, 180, 181, 0, 0, 72, 2, 4, 
    182, 183, 184, 185, 0, 0, 79, 4, 4, 186, 187, 188, 189, 0, 0, 89, 
    2, 0, 0, 0, 95, 2, 2, 190, 191, 0, 0, 98, 6, 3, 192, 193, 
    194, 2, 195, 196, 0, 571, 2, 3, 1469, 1470, 1471, 0, 0, 100, 2, 3, 
    197, 198, 199, 0, 0, 574, 2, 1, 1477, 0, 0, 101, 2, 1, 200, 0, 
    0, 577, 2, 1, 1483, 0, 0, 102, 1, 1, 201, 0, 0, 110, 1, 0, 
    10, 73, 1179, 1180, 1181, 1182, 1183, 1184, 1185, 1186, 1187, 0, 111, 5, 0, 14, 
    202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 0, 112, 
    5, 0, 11, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 0, 113, 
    5, 0, 4, 227, 228, 229, 230, 3, 114, 4, 0, 4, 236, 237, 238, 239, 
    0, 115, 2, 4, 231, 232, 233, 234, 0, 0, 116, 4, 1, 235, 0, 0, 
    117, 3, 0, 0, 0, 118, 2, 3, 240, 241, 242, 0, 0, 119, 2, 3, 
    243, 244, 245, 0, 0, 120, 6, 0, 12, 246, 247, 250, 251, 252, 253, 254, 
    255, 259, 260, 261, 262, 0, 121, 3, 2, 248, 249, 0, 0, 122, 4, 3, 
    256, 257, 258, 0, 0, 123, 2, 0, 2, 263, 264, 1, 124, 3, 2, 265, 
    266, 0, 0, 125, 18, 0, 10, 267, 268, 269, 270, 271, 272, 273, 274, 275, 
    276, 0, 126, 10, 0, 0, 0, 127, 3, 0, 0, 2, 474, 2, 2, 1197, 
    1198, 2, 1199, 1200, 0, 128, 2, 2, 277, 278, 3, 279, 280, 284, 0, 475, 
    2, 1, 1201, 0, 0, 129, 2, 1, 281, 0, 0, 476, 2, 1, 1202, 0, 
    0, 130, 2, 1, 282, 0, 0, 133, 2, 1, 283, 0, 0, 534, 2, 2, 
    1342, 1343, 2, 1344, 1345, 0, 532, 2, 2, 1334, 1335, 2, 1336, 1337, 0, 504, 
    2, 2, 1291, 1292, 2, 1293, 1294, 0, 494, 6, 4, 1269, 1270, 1271, 1272, 2, 
    1273, 1274, 0, 492, 6, 4, 1255, 1256, 1257, 1258, 2, 1259, 1260, 0, 487, 2, 
    2, 1236, 1237, 2, 1238, 1239, 0, 485, 2, 2, 1228, 1229, 2, 1230, 1231, 0, 
    480, 2, 2, 1213, 1214, 2, 1215, 1216, 0, 137, 2, 2, 285, 286, 2, 287, 
    288, 0, 531, 3, 3, 1329, 1330, 1331, 2, 1332, 1333, 0, 491, 7, 7, 1246, 
    1247, 1248, 1249, 1250, 1251, 1252, 2, 1253, 1254, 0, 484, 3, 3, 1223, 1224, 1225, 
    2, 1226, 1227, 0, 138, 3, 3, 289, 290, 291, 2, 292, 293, 0, 143, 2, 
    2, 294, 295, 1, 296, 0, 533, 2, 3, 1338, 1339, 1340, 1, 1341, 0, 493, 
    4, 7, 1261, 1262, 1263, 1264, 1265, 1266, 1267, 1, 1268, 0, 486, 2, 3, 1232, 
    1233, 1234, 1, 1235, 0, 144, 2, 3, 297, 298, 299, 1, 300, 0, 528, 2, 
    1, 1323, 0, 0, 501, 2, 1, 1285, 0, 0, 488, 2, 1, 1240, 0, 0, 
    481, 2, 1, 1217, 0, 0, 148, 2, 1, 301, 0, 0, 150, 1, 0, 0, 
    0, 153, 12, 0, 72, 303, 304, 314, 315, 316, 317, 318, 319, 320, 321, 322, 
    323, 324, 325, 326, 327, 328, 329, 330, 335, 336, 339, 342, 345, 346, 349, 352, 
    353, 354, 355, 356, 357, 358, 359, 360, 361, 362, 363, 364, 365, 366, 367, 368, 
    369, 378, 379, 380, 381, 382, 383, 384, 385, 386, 387, 391, 396, 397, 398, 399, 
    302, 305, 306, 307, 308, 313, 1188, 395, 1275, 1276, 1277, 1278, 1284, 24, 156, 1, 
    2, 309, 310, 0, 0, 157, 1, 2, 311, 312, 0, 0, 159, 2, 1, 331, 
    1, 332, 1, 160, 3, 0, 2, 333, 334, 0, 161, 2, 2, 337, 338, 0, 
    0, 162, 2, 2, 340, 341, 0, 0, 163, 2, 2, 343, 344, 0, 0, 164, 
    2, 1, 347, 0, 0, 477, 2, 1, 1203, 0, 0, 165, 2, 1, 348, 0, 
    0, 166, 1, 1, 350, 0, 0, 167, 2, 1, 351, 0, 0, 470, 1, 0, 
    0, 0, 168, 2, 0, 0, 0, 169, 2, 0, 0, 0, 170, 2, 0, 0, 
    0, 171, 3, 1, 370, 0, 0, 172, 4, 1, 371, 1, 372, 0, 471, 0, 
    0, 0, 0, 173, 1, 0, 0, 0, 472, 0, 0, 0, 0, 174, 1, 0, 
    0, 0, 175, 3, 0, 1, 373, 0, 176, 2, 1, 374, 0, 0, 177, 2, 
    0, 0, 0, 178, 2, 1, 375, 0, 0, 179, 2, 0, 0, 0, 180, 2, 
    2, 376, 377, 0, 0, 181, 3, 0, 2, 388, 389, 0, 182, 1, 0, 0, 
    0, 183, 2, 1, 390, 0, 1, 184, 2, 0, 0, 0, 185, 3, 0, 0, 
    0, 187, 2, 3, 392, 393, 394, 0, 0, 190, 2, 0, 0, 0, 191, 1, 
    0, 0, 1, 192, 4, 0, 1, 400, 0, 193, 4, 0, 1, 401, 6, 194, 
    2, 0, 2, 402, 403, 0, 195, 1, 0, 0, 0, 196, 5, 0, 1, 404, 
    1, 197, 2, 1, 405, 0, 0, 198, 2, 0, 0, 1, 199, 2, 0, 12, 
    1295, 1298, 1300, 1302, 1303, 1312, 1313, 1322, 1346, 1347, 1348, 1349, 0, 200, 2, 0, 
    0, 1, 508, 1, 1, 1297, 0, 0, 506, 1, 1, 1296, 0, 0, 202, 1, 
    1, 407, 1, 406, 0, 203, 1, 0, 0, 0, 205, 1, 0, 0, 0, 206, 
    1, 0, 0, 0, 511, 1, 1, 1299, 0, 0, 207, 2, 1, 408, 0, 0, 
    208, 1, 0, 0, 2, 513, 2, 1, 1301, 0, 0, 209, 2, 1, 409, 0, 
    0, 210, 1, 0, 0, 0, 211, 3, 0, 1, 410, 2, 212, 2, 0, 9, 
    435, 436, 437, 438, 439, 411, 412, 413, 441, 4, 215, 1, 1, 414, 0, 0, 
    227, 1, 1, 432, 0, 0, 216, 2, 2, 415, 416, 0, 0, 222, 2, 2, 
    424, 425, 0, 0, 228, 2, 2, 433, 434, 0, 0, 516, 2, 2, 1304, 1305, 
    0, 0, 217, 2, 2, 417, 418, 0, 1, 517, 2, 2, 1306, 1307, 0, 0, 
    223, 2, 2, 426, 427, 0, 1, 218, 1, 2, 419, 420, 0, 0, 518, 3, 
    2, 1308, 1309, 0, 0, 224, 3, 2, 428, 429, 0, 0, 219, 2, 1, 421, 
    0, 0, 225, 2, 1, 430, 0, 0, 220, 1, 1, 422, 0, 0, 226, 2, 
    1, 431, 1, 1310, 0, 221, 1, 1, 423, 0, 0, 520, 1, 1, 1311, 0, 
    0, 229, 1, 1, 440, 0, 0, 231, 1, 1, 442, 0, 0, 232, 2, 1, 
    443, 0, 0, 233, 2, 0, 2, 444, 445, 0, 234, 2, 1, 446, 0, 0, 
    235, 5, 0, 8, 455, 456, 460, 461, 457, 458, 1320, 1321, 6, 523, 3, 2, 
    1314, 1315, 0, 0, 236, 3, 2, 447, 448, 0, 0, 524, 3, 2, 1316, 1317, 
    0, 0, 237, 3, 2, 449, 450, 0, 0, 238, 1, 1, 451, 1, 452, 0, 
    525, 3, 2, 1318, 1319, 0, 0, 239, 3, 2, 453, 454, 0, 0, 241, 1, 
    1, 459, 0, 0, 242, 2, 0, 0, 0, 243, 5, 1, 462, 0, 0, 247, 
    1, 0, 0, 0, 248, 5, 1, 463, 0, 0, 249, 5, 1, 464, 0, 0, 
    250, 4, 0, 2, 465, 466, 0, 251, 3, 0, 1, 467, 0, 254, 2, 0, 
    3, 468, 469, 470, 0, 255, 0, 1, 471, 0, 0, 256, 0, 0, 0, 0, 
    257, 6, 0, 1, 472, 0, 258, 2, 1, 473, 0, 0, 557, 4, 4, 1401, 
    1402, 1403, 1404, 4, 1405, 1406, 1407, 1408, 0, 546, 4, 4, 1366, 1367, 1368, 1369, 
    4, 1370, 1371, 1372, 1373, 0, 260, 4, 4, 474, 475, 476, 477, 4, 478, 479, 
    485, 486, 0, 558, 3, 3, 1409, 1410, 1411, 1, 1412, 0, 553, 3, 4, 1385, 
    1386, 1387, 1388, 1, 1389, 0, 261, 3, 4, 480, 481, 482, 483, 1, 484, 0, 
    542, 4, 5, 1361, 1362, 1363, 1364, 1365, 0, 0, 262, 4, 5, 487, 488, 489, 
    490, 491, 0, 0, 547, 2, 1, 1374, 0, 0, 263, 2, 1, 492, 0, 0, 
    549, 2, 2, 1375, 1376, 0, 0, 264, 2, 2, 493, 494, 0, 0, 552, 4, 
    1, 1383, 1, 1384, 0, 265, 4, 1, 495, 1, 496, 0, 554, 4, 2, 1390, 
    1391, 0, 0, 266, 4, 2, 497, 498, 0, 0, 267, 2, 7, 499, 500, 501, 
    502, 503, 504, 505, 1, 506, 0, 541, 5, 1, 1360, 0, 0, 268, 5, 1, 
    507, 0, 0, 551, 11, 2, 1377, 1378, 4, 1379, 1380, 1381, 1382, 0, 269, 11, 
    2, 508, 509, 4, 510, 511, 512, 513, 0, 270, 2, 2, 514, 515, 4, 516, 
    517, 518, 519, 0, 271, 3, 2, 520, 521, 0, 0, 274, 8, 4, 522, 523, 
    524, 525, 2, 526, 527, 0, 275, 1, 1, 528, 0, 0, 276, 11, 2, 529, 
    530, 3, 531, 532, 533, 0, 277, 2, 1, 534, 0, 0, 278, 1, 1, 535, 
    0, 0, 279, 12, 0, 12, 536, 537, 538, 539, 540, 541, 542, 543, 544, 548, 
    553, 554, 0, 280, 2, 1, 545, 0, 0, 281, 12, 1, 546, 1, 547, 0, 
    282, 3, 2, 549, 550, 0, 0, 283, 2, 2, 551, 552, 0, 0, 284, 3, 
    1, 555, 0, 0, 285, 2, 2, 556, 557, 0, 2, 286, 9, 0, 2, 582, 
    583, 2, 287, 2, 2, 558, 559, 0, 0, 289, 2, 2, 562, 563, 0, 0, 
    291, 2, 2, 566, 567, 0, 0, 293, 2, 2, 570, 571, 0, 0, 295, 2, 
    2, 574, 575, 0, 0, 297, 2, 2, 578, 579, 0, 0, 288, 1, 2, 560, 
    561, 0, 0, 290, 2, 2, 564, 565, 0, 0, 292, 2, 2, 568, 569, 0, 
    0, 294, 1, 2, 572, 573, 0, 0, 296, 1, 2, 576, 577, 0, 0, 298, 
    2, 2, 580, 581, 0, 0, 299, 1, 1, 584, 0, 0, 300, 2, 1, 585, 
    0, 0, 301, 1, 1, 586, 0, 0, 302, 16, 0, 1, 587, 1, 303, 6, 
    2, 588, 589, 6, 590, 591, 592, 593, 594, 601, 0, 305, 4, 2, 595, 596, 
    0, 0, 306, 9, 2, 597, 598, 0, 0, 307, 2, 2, 599, 600, 0, 0, 
    308, 2, 2, 602, 603, 0, 1, 309, 4, 2, 604, 605, 0, 0, 310, 1, 
    1, 606, 0, 0, 311, 2, 0, 4, 640, 641, 642, 643, 1, 312, 1, 1, 
    607, 0, 0, 315, 3, 1, 610, 1, 611, 0, 313, 1, 1, 608, 0, 0, 
    317, 3, 1, 614, 0, 0, 314, 1, 1, 609, 0, 0, 320, 3, 1, 619, 
    0, 0, 316, 3, 2, 612, 613, 0, 2, 318, 2, 2, 615, 616, 0, 0, 
    319, 2, 2, 617, 618, 0, 0, 321, 1, 1, 620, 0, 0, 322, 5, 2, 
    621, 622, 0, 0, 323, 2, 1, 623, 6, 624, 625, 626, 627, 628, 639, 0, 
    324, 5, 1, 629, 0, 1, 325, 4, 0, 2, 630, 632, 1, 326, 3, 1, 
    631, 0, 0, 327, 3, 0, 0, 0, 566, 4, 1, 1450, 1, 1451, 0, 328, 
    4, 1, 633, 1, 634, 0, 329, 3, 0, 3, 635, 637, 638, 0, 330, 2, 
    1, 636, 0, 0, 331, 7, 5, 644, 645, 646, 647, 648, 6, 649, 652, 653, 
    661, 662, 663, 0, 332, 5, 2, 650, 651, 0, 0, 333, 9, 3, 654, 655, 
    656, 6, 657, 658, 659, 660, 664, 665, 0, 334, 5, 0, 0, 1, 336, 8, 
    0, 14, 666, 667, 668, 669, 670, 671, 672, 673, 674, 684, 690, 691, 692, 693, 
    0, 337, 2, 1, 675, 0, 0, 338, 3, 1, 676, 1, 677, 0, 580, 8, 
    1, 1494, 2, 1495, 1496, 0, 339, 8, 1, 678, 2, 679, 680, 0, 340, 2, 
    3, 681, 682, 683, 0, 0, 341, 5, 3, 685, 686, 687, 2, 688, 689, 0, 
    342, 2, 0, 0, 0, 343, 4, 0, 0, 0, 344, 4, 3, 694, 695, 696, 
    5, 697, 698, 701, 702, 703, 0, 345, 3, 1, 699, 1, 700, 0, 346, 2, 
    3, 704, 705, 706, 0, 0, 347, 6, 0, 8, 707, 708, 721, 722, 723, 724, 
    725, 726, 1, 348, 8, 4, 709, 710, 711, 712, 8, 713, 714, 715, 716, 717, 
    718, 719, 720, 0, 352, 10, 0, 41, 727, 728, 729, 743, 744, 747, 748, 749, 
    750, 751, 752, 753, 754, 761, 762, 763, 764, 765, 770, 771, 772, 773, 774, 775, 
    776, 778, 783, 784, 785, 786, 787, 788, 789, 790, 791, 792, 793, 794, 795, 796, 
    797, 0, 353, 8, 0, 13, 730, 731, 732, 733, 734, 735, 736, 737, 738, 739, 
    740, 741, 742, 0, 355, 6, 2, 745, 746, 0, 0, 356, 2, 6, 755, 756, 
    757, 758, 759, 760, 0, 0, 358, 2, 2, 766, 767, 0, 0, 359, 5, 2, 
    768, 769, 0, 0, 360, 2, 1, 777, 0, 0, 361, 2, 2, 779, 780, 0, 
    0, 362, 2, 1, 781, 0, 0, 363, 2, 1, 782, 0, 0, 367, 19, 0, 
    32, 944, 945, 946, 1015, 1016, 1017, 1018, 1019, 1020, 1021, 1022, 1023, 1024, 1025, 1026, 
    1027, 1028, 1029, 1030, 1031, 1032, 1033, 1034, 1035, 1036, 1037, 1038, 1039, 1040, 1041, 1042, 
    1043, 0, 368, 9, 7, 930, 931, 932, 933, 934, 935, 936, 7, 937, 938, 939, 
    940, 941, 942, 943, 0, 369, 23, 7, 947, 948, 949, 950, 951, 952, 953, 61, 
    954, 955, 956, 957, 958, 959, 960, 961, 962, 963, 964, 965, 966, 967, 968, 969, 
    970, 971, 972, 973, 974, 975, 976, 977, 978, 979, 980, 981, 982, 983, 984, 985, 
    986, 987, 988, 989, 990, 991, 992, 993, 994, 995, 996, 997, 998, 999, 1000, 1001, 
    1002, 1003, 1004, 1005, 1006, 1007, 1008, 1009, 1010, 1011, 1012, 1013, 1014, 0, 370, 2, 
    0, 0, 0, 372, 2, 1, 1044, 0, 0, 373, 12, 3, 1045, 1046, 1047, 0, 
    0, 374, 6, 0, 2, 1048, 1049, 32, 375, 2, 0, 4, 1050, 1051, 1052, 1053, 
    2, 376, 1, 0, 2, 1599, 1600, 0, 377, 2, 0, 2, 1054, 1055, 0, 378, 
    2, 0, 2, 1056, 1057, 0, 379, 1, 0, 1, 1601, 0, 380, 9, 0, 2, 
    1058, 1059, 1, 381, 6, 8, 1060, 1061, 1062, 1063, 1064, 1065, 1066, 1067, 0, 0, 
    382, 5, 5, 1068, 1069, 1070, 1071, 1072, 0, 0, 393, 2, 2, 1085, 1086, 0, 
    0, 425, 2, 2, 1106, 1107, 0, 0, 443, 10, 8, 1120, 1121, 1122, 1123, 1124, 
    1125, 1126, 1127, 0, 0, 444, 8, 6, 1128, 1129, 1130, 1131, 1132, 1133, 0, 0, 
    383, 4, 4, 1073, 1074, 1075, 1076, 0, 0, 606, 1, 1, 1604, 0, 0, 418, 
    1, 1, 1095, 0, 0, 391, 2, 2, 1081, 1082, 0, 0, 423, 2, 2, 1102, 
    1103, 0, 0, 392, 2, 2, 1083, 1084, 0, 0, 424, 2, 2, 1104, 1105, 0, 
    0, 394, 2, 2, 1087, 1088, 0, 0, 426, 2, 2, 1108, 1109, 0, 0, 401, 
    2, 2, 1089, 1090, 0, 0, 605, 2, 2, 1602, 1603, 0, 0, 411, 2, 2, 
    1091, 1092, 0, 0, 433, 2, 2, 1110, 1111, 0, 0, 412, 2, 2, 1093, 1094, 
    0, 0, 419, 1, 1, 1096, 0, 0, 420, 1, 1, 1097, 0, 0, 421, 2, 
    2, 1098, 1099, 0, 0, 422, 2, 2, 1100, 1101, 0, 0, 442, 8, 0, 8, 
    1112, 1113, 1114, 1115, 1116, 1117, 1118, 1119, 0, 445, 3, 4, 1134, 1135, 1136, 1137, 
    0, 0, 446, 4, 4, 1138, 1139, 1140, 1141, 0, 0, 447, 8, 4, 1142, 1143, 
    1144, 1145, 0, 0, 448, 2, 4, 1146, 1147, 1148, 1149, 0, 0, 449, 1, 0, 
    1, 1605, 0, 450, 8, 0, 10, 1154, 1155, 1156, 1157, 1158, 1159, 1160, 1161, 1162, 
    1163, 0, 451, 7, 0, 9, 1164, 1165, 1167, 1169, 1171, 1173, 1174, 1176, 1177, 6, 
    452, 1, 1, 1166, 0, 0, 453, 1, 1, 1168, 0, 0, 454, 1, 1, 1170, 
    0, 0, 455, 1, 1, 1172, 0, 0, 456, 3, 1, 1175, 0, 0, 457, 3, 
    1, 1178, 0, 0,     0
};

static const int type_table[] = {
  1, -1, 2, -3, -4, -7, -6, -1, 0, -1, 2, -1, 3, -1, 7, -1,
  9, -1, 4, 3, 2, 3, 1, 3, 159, 4, 1, 2, 0, -1, -1, 0,
  -1, 0, 0, -1, 0, -1, 25, 3, 307, 3, 49, 3, 326, 3, 30, 3,
  322, -2, 50, 8, 41, 79, -1, 2, -1, 0, 2, -1, 0, 2, -1, 0,
  109, -1, 0, 111, -1, 0, 144, -1, 0, 146, -1, 0, 146, -1, 0, -2,
  51, 8, 41, -1, -1, 2, -1, 0, 2, -1, 0, 2, -1, 0, 109, -1,
  0, 111, -1, 0, 144, -1, 0, 146, -1, 0, 146, -1, 0, 0, 49, -2,
  43, 7, 29, -1, -1, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1,
  0, 0, -1, 0, 0, -1, 0, 0, -1, 0, -2, 40, 0, 27, -1, -1,
  0, 138, 1, 0, -1, 49, 3, 277, 3, 310, 3, 161, 3, 163, 3, 162,
  3, 111, 4, 0, 2, 0, -1, -1, 2, -1, 0, 2, -1, 0, 4, 2,
  3, 0, 162, -1, 2, -1, 0, 2, -1, 0, 2, -1, 0, 4, 4, 4,
  0, 174, -1, 2, -1, 0, 2, -1, 0, 2, -1, 0, 2, -1, 0, 4,
  3, 3, 0, 25, -1, 0, -1, 0, 0, -1, 0, 0, -1, 0, 4, 5,
  4, 0, 207, -1, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0,
  -1, 207, 4, 67, 4, 0, 189, -1, 2, -1, 0, 2, -1, 0, 2, -1,
  0, 2, -1, 0, 3, 355, 3, 304, 4, 7, 3, 0, -1, -1, 0, 279,
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
  2, -1, 0, 2, -1, 0, 3, 179, 3, 134, 0, 3, -2, 74, 1, 82,
  -1, -1, 486, -1, 0, -2, 75, 3, 82, -1, -1, 484, -1, 0, 2, -1,
  0, 2, -1, 0, 0, 469, -1, 484, -2, 82, 1, 82, -1, -1, 542, -1,
  0, -2, 83, 3, 82, -1, -1, 512, -1, 0, 3, -1, 0, 514, -1, 0,
  0, 497, -2, 80, 1, 82, -1, -1, 540, -1, 0, -2, 81, 3, 82, -1,
  -1, 538, -1, 0, 3, -1, 0, 0, -1, 0, 0, 523, -1, 538, -1, 512,
  -2, 20, 1, 2, -1, -1, 0, -1, 1, -2, 21, 1, 2, -1, -1, 0,
  -1, 0, -2, 22, 2, 3, 553, 0, 0, -1, 0, 0, -1, 0, -2, 23,
  11, 4, -1, 123, 146, -1, 0, 0, -1, 0, 2, -1, 0, 2, 174, 0,
  2, 174, 0, 2, 174, 0, 3, -1, 0, 7, -1, 0, 458, -1, 0, 0,
  -1, 0, 553, -1, 0, 0, 574, 3, 166, 3, 167, -2, 24, 2, 4, -1,
  -1, 3, -1, 0, 0, -1, 0, -2, 76, 1, 82, -1, -1, 657, -1, 0,
  -2, 77, 3, 82, -1, -1, 655, -1, 0, 3, -1, 0, 458, -1, 0, 0,
  640, -1, 655, -2, 78, 1, 82, -1, -1, 697, -1, 0, -2, 79, 7, 82,
  -1, -1, 695, -1, 0, 0, 207, 0, 0, 207, 0, 0, 207, 0, 0, 207,
  0, 0, 207, 0, 0, 207, 0, 0, 668, -1, 695, -5, 25, 2, 3, -1,
  -1, 3, -1, 0, 11, -1, 0, -2, 26, 4, 4, -1, -1, 2, 162, 0,
  2, 162, 0, 3, -1, 0, 7, -1, 0, 3, 195, 3, 203, 3, 204, 4,
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
  -1, 0, 146, -1, 0, 0, 919, 3, 263, 3, 264, 3, 266, -1, 919, 3,
  283, 3, 270, 3, 269, 3, 261, 3, 282, 3, 265, -1, 138, 3, 299, 3,
  300, 3, 548, -1, 1005, -2, 44, 0, 29, 138, -1, -2, 45, 0, 31, 138,
  -1, -2, 46, 0, 33, 138, -1, -2, 47, 0, 35, 138, -1, -2, 48, 0,
  37, 138, -1, -2, 49, 0, 39, 138, -1, -2, 53, 1, 48, 1074, -1, 0,
  -1, 0, -2, 52, 0, 41, -1, -1, -2, 54, 2, 55, 1074, -1, 3, -1,
  0, 1092, -1, 0, -1, 1074, 0, 1074, 3, 335, -2, 55, 5, 62, -1, -1,
  0, -1, 0, 0, 25, 0, 0, 25, 0, 0, 25, 0, 0, 25, 0, -1,
  1098, -1, 1119, -2, 56, 3, 62, -1, -1, 2, -1, 0, 2, -1, 0, 2,
  -1, 0, -1, 1123, -1, 1138, 3, 344, 3, 342, 3, 345, -1, 162, 4, 59,
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
  3, 114, 3, 117, 3, 120, 3, 123, 3, 125, 3, 126, 3, 127, 3, 160,
  3, 164, 3, 175, 3, 177, 3, 178, 3, 180, 3, 149, 3, 483, 3, 192,
  3, 193, 3, 194, 3, 196, 3, 197, 3, 198, 3, 490, 3, 200, 3, 205,
  3, 507, 3, 206, 3, 208, 3, 211, 3, 212, -2, 86, 1, 90, -1, -1,
  3, -1, 0, 3, 233, 3, 235, 3, 250, 3, 503, 3, 251, 3, 254, 3,
  256, 3, 257, 3, 280, 3, 281, 3, 271, 3, 543, 3, 544, 3, 545, 3,
  550, 3, 284, 3, 285, 3, 275, 3, 276, 3, 278, 3, 301, 3, 101, 3,
  102, 3, 305, 3, 308, 3, 309, 3, 330, 3, 316, 3, 318, 3, 319, 3,
  321, 3, 332, 3, 572, 3, 575, 3, 337, 3, 338, 3, 346, 3, 350, 3,
  351, 3, 349, 3, 354, 3, 356, 3, 586, 3, 357, 3, 358, 3, 359, 3,
  360, 3, 361, 3, 362, 3, 363, 3, 370, 3, 371, 3, 372, 3, 373, 3,
  377, 3, 378, 3, 380, 3, 450, 3, 452, 3, 453, 3, 454, 3, 455, 3,
  456, 3, 457, 4, 17, 1, 0, -1, -1, 0, -1, 0, 4, 19, 5, 0,
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
    string_view("(== test_namespace.f:int 1:int)", 31),
    string_view("(== (.x (lifetime change<0|1> (object_constructor 1:int):test_namespace.g):test_namespace.g):int 1:int)", 103),
    string_view("(== (block (this var (object_constructor 1:int):test_namespace.g):void 2:int):int 2:int)", 88),
    string_view("(== (block (x var 1:int):void (y var 0:int):void 1:int):int 1:int)", 66),
    string_view("", 0),
    string_view("(== (block (x var \"\":string):void (y var 0:int):void 2:int):int 2:int)", 70),
    string_view("(== (tf (object_constructor 0:int 0:int):testb 4:int):int 4:int)", 64),
    string_view("(== (tf (block (a var (object_constructor 0:int 0:int):testb):void (lifetime change<1|0> a:testa):testa):testa 1:int):int 4:int)", 128),
    string_view("hello, world!\n\"\'\r\t\\\xC0", 20),
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
    string_view("(== 264:int (sum (map testvector:[int] (def function157):function157(_:int) -> int):[int]):int)", 95),
    string_view("(== (factorial 7:int):int 5040:int)", 35),
    string_view("(== 16:int (function159 (def function160):function160(x:int) -> int):int)", 73),
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
    string_view("\x03\x04\x00\x00", 4),
    string_view("(and (== ns:string \"\\x03\\x04\\x00\\x00\":string):bool (== ni:int 2:int):bool)", 74),
    string_view("(== (read_int32_le ns:string 0:int):int 1027:int)", 49),
    string_view("", 0),
    string_view("\x00\x00\x02\x08", 4),
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
    string_view("\xE2\xBA\x86\xE3\x8B\x87\xE4\xAC\x88", 9),
    string_view("\xE2\xBA\x86\xE3\x8B\x87\xE4\xAC\x88", 9),
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
    string_view("(== 1:int (g (def function196):function196() -> int):int)", 57),
    string_view("a", 1),
    string_view("(== \"a\":string (lifetime change<0|1> (g (def function197):function197() -> string):string):string)", 98),
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
    string_view("(equal (lifetime change<0|1> (map goap_path:[spec_node] (def function271):function271(n:spec_node) -> string):[string]):[string] (lifetime change<0|1> (vector_constructor (lifetime change<1|0> \"Eat\":string):string (lifetime change<1|0> \"BuyPizza\":string):string (lifetime change<1|0> \"SellSkin\":string):string (lifetime change<1|0> \"KillWolf\":string):string (lifetime change<1|0> \"Eat\":string):string (lifetime change<1|0> \"BuyPizza\":string):string (lifetime change<1|0> \"SellSkin\":string):string (lifetime change<1|0> \"KillWolf\":string):string):[string]):[string])", 565),
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
    string_view("()\x7F", 3),
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
    string_view("(== (length (lifetime change<0|1> (floodfillpath (* int2_1:int2 20:int):int2 (* int2_1:int2 5:int):int2 (def function302):function302(v:int2) -> bool):[[pathmap]]):[[pathmap]]):int 20:int)", 188),
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
    string_view("rethrow", 7),
    string_view("throw_if", 8),
    string_view("try", 3),
    string_view("function130", 11),
    string_view("protect", 7),
    string_view("function132", 11),
    string_view("lookup", 6),
    string_view("set", 3),
    string_view("del", 3),
    string_view("delete", 6),
    string_view("clear", 5),
    string_view("add", 3),
    string_view("get", 3),
    string_view("get", 3),
    string_view("get_keys", 8),
    string_view("get_values", 10),
    string_view("forkv", 5),
    string_view("dictionary", 10),
    string_view("function145", 11),
    string_view("test_namespace.f", 16),
    string_view("test_namespace.f2", 17),
    string_view("m", 1),
    string_view("function149", 11),
    string_view("tf", 2),
    string_view("to_a", 4),
    string_view("function152", 11),
    string_view("function153", 11),
    string_view("function154", 11),
    string_view("function155", 11),
    string_view("function156", 11),
    string_view("function157", 11),
    string_view("factorial", 9),
    string_view("function159", 11),
    string_view("function160", 11),
    string_view("multiret", 8),
    string_view("multiretpassthru", 16),
    string_view("multirettyped", 13),
    string_view("write", 5),
    string_view("union_return", 12),
    string_view("nlr1", 4),
    string_view("nlr2", 4),
    string_view("function168", 11),
    string_view("recursive_exceptions", 20),
    string_view("function170", 11),
    string_view("function171", 11),
    string_view("function172", 11),
    string_view("function173", 11),
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
    string_view("function184", 11),
    string_view("function185", 11),
    string_view("function186", 11),
    string_view("function187", 11),
    string_view("function188", 11),
    string_view("function189", 11),
    string_view("function190", 11),
    string_view("function191", 11),
    string_view("function192", 11),
    string_view("h", 1),
    string_view("g", 1),
    string_view("function195", 11),
    string_view("function196", 11),
    string_view("function197", 11),
    string_view("function198", 11),
    string_view("compile_time_if", 15),
    string_view("function200", 11),
    string_view("return_from", 11),
    string_view("return_from2", 12),
    string_view("function203", 11),
    string_view("function204", 11),
    string_view("f", 1),
    string_view("ad", 2),
    string_view("f", 1),
    string_view("g", 1),
    string_view("r", 1),
    string_view("mr", 2),
    string_view("sup", 3),
    string_view("sni", 3),
    string_view("foo", 3),
    string_view("function214", 11),
    string_view("f", 1),
    string_view("function216", 11),
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
    string_view("function231", 11),
    string_view("function232", 11),
    string_view("make_set", 8),
    string_view("function234", 11),
    string_view("f", 1),
    string_view("function236", 11),
    string_view("function237", 11),
    string_view("foo", 3),
    string_view("astar_clear", 11),
    string_view("astar_generic", 13),
    string_view("function241", 11),
    string_view("astar_graph", 11),
    string_view("function243", 11),
    string_view("function244", 11),
    string_view("function245", 11),
    string_view("function246", 11),
    string_view("astar_2dgrid", 12),
    string_view("astar_distance", 14),
    string_view("function249", 11),
    string_view("function250", 11),
    string_view("function251", 11),
    string_view("precondition", 12),
    string_view("effect", 6),
    string_view("astar_goap", 10),
    string_view("function255", 11),
    string_view("function256", 11),
    string_view("function257", 11),
    string_view("function258", 11),
    string_view("function259", 11),
    string_view("function260", 11),
    string_view("function261", 11),
    string_view("function262", 11),
    string_view("function263", 11),
    string_view("function264", 11),
    string_view("function265", 11),
    string_view("function266", 11),
    string_view("precondition", 12),
    string_view("effect", 6),
    string_view("function269", 11),
    string_view("function270", 11),
    string_view("function271", 11),
    string_view("function272", 11),
    string_view("knights_tour", 12),
    string_view("function274", 11),
    string_view("function275", 11),
    string_view("empty", 5),
    string_view("function277", 11),
    string_view("function278", 11),
    string_view("function279", 11),
    string_view("function280", 11),
    string_view("function281", 11),
    string_view("pretty", 6),
    string_view("eval", 4),
    string_view("compile", 7),
    string_view("function285", 11),
    string_view("function286", 11),
    string_view("function287", 11),
    string_view("function288", 11),
    string_view("function289", 11),
    string_view("parse", 5),
    string_view("error", 5),
    string_view("lex_atom", 8),
    string_view("function293", 11),
    string_view("lex_next", 8),
    string_view("expect", 6),
    string_view("parse_exp", 9),
    string_view("function297", 11),
    string_view("floodfillpath", 13),
    string_view("function299", 11),
    string_view("flood", 5),
    string_view("function301", 11),
    string_view("function302", 11),
    string_view("function303", 11),
    string_view("function304", 11),
    string_view("function305", 11),
    string_view("loopworld", 9),
    string_view("ifinworld", 9),
    string_view("forneighboursinworld", 20),
    string_view("function309", 11),
    string_view("sim", 3),
    string_view("function311", 11),
    string_view("function312", 11),
    string_view("function313", 11),
    string_view("function314", 11),
    string_view("gradient_descent", 16),
    string_view("function316", 11),
    string_view("function317", 11),
    string_view("function318", 11),
    string_view("function319", 11),
    string_view("wiggleverts", 11),
    string_view("function321", 11),
    string_view("function322", 11),
    string_view("function323", 11),
    string_view("function324", 11),
    string_view("function325", 11),
    string_view("function326", 11),
    string_view("function327", 11),
    string_view("function328", 11),
    string_view("function329", 11),
    string_view("function330", 11),
    string_view("from_srgb", 9),
    string_view("to_srgb", 7),
    string_view("luminance", 9),
    string_view("function334", 11),
    string_view("intersect", 9),
    string_view("radiance", 8),
    string_view("function337", 11),
    string_view("function338", 11),
    string_view("function339", 11),
    string_view("function340", 11),
    string_view("function341", 11),
    string_view("function342", 11),
    string_view("function343", 11),
    string_view("function344", 11),
    string_view("function345", 11),
    string_view("function346", 11),
    string_view("function347", 11),
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
    string_view("function387", 11),
    string_view("determinant", 11),
    string_view("adjugate", 8),
    string_view("inverse", 7),
    string_view("trace", 5),
    string_view("function392", 11),
    string_view("function393", 11),
    string_view("function394", 11),
    string_view("function395", 11),
    string_view("function396", 11),
    string_view("function397", 11),
    string_view("function398", 11),
    string_view("function399", 11),
    string_view("function400", 11),
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
    { string_view("float2", 6), 0, 2, -1, 162, make_span(float2_fields0) },
    { string_view("int2", 4), 1, 2, -1, 25, make_span(int2_fields1) },
    { string_view("float3", 6), 2, 3, 0, 174, make_span(float3_fields2) },
    { string_view("int3", 4), 3, 3, 1, 207, make_span(int3_fields3) },
    { string_view("float4", 6), 4, 4, 2, 189, make_span(float4_fields4) },
    { string_view("int4", 4), 5, 4, 3, 222, make_span(int4_fields5) },
    { string_view("A", 1), 6, 2, -1, 279, make_span(A_fields6) },
    { string_view("B", 1), 7, 3, -1, 264, make_span(B_fields7) },
    { string_view("C", 1), 8, 3, -1, 291, make_span(C_fields8) },
    { string_view("C", 1), 9, 5, -1, 318, make_span(C_fields9) },
    { string_view("S", 1), 10, 5, -1, 339, make_span(S_fields10) },
    { string_view("xy_s", 4), 11, 2, -1, 360, make_span(xy_s_fields11) },
    { string_view("mm1", 3), 12, 1, -1, 372, make_span(mm1_fields12) },
    { string_view("mm2", 3), 13, 1, 12, 381, make_span(mm2_fields13) },
    { string_view("R", 1), 14, 2, -1, 390, make_span(R_fields14) },
    { string_view("RN", 2), 15, 3, -1, 404, make_span(RN_fields15) },
    { string_view("RS", 2), 16, 2, -1, 421, make_span(RS_fields16) },
    { string_view("V1", 2), 17, 1, -1, 1539, make_span(V1_fields17) },
    { string_view("V5", 2), 18, 5, -1, 433, make_span(V5_fields18) },
    { string_view("A", 1), 19, 5, -1, 1548, make_span(A_fields19) },
    { string_view("test_namespace.g", 16), 20, 1, -1, 544, make_span(test_namespace_g_fields20) },
    { string_view("testa", 5), 21, 1, -1, 553, make_span(testa_fields21) },
    { string_view("testb", 5), 22, 2, 21, 562, make_span(testb_fields22) },
    { string_view("parsetest", 9), 23, 11, -1, 574, make_span(parsetest_fields23) },
    { string_view("mretfields", 10), 24, 2, -1, 619, make_span(mretfields_fields24) },
    { string_view("STLV", 4), 25, 2, -1, 699, make_span(STLV_fields25) },
    { string_view("Reflect", 7), 26, 4, -1, 711, make_span(Reflect_fields26) },
    { string_view("testa", 5), 27, 1, -1, 1569, make_span(testa_fields27) },
    { string_view("testb", 5), 28, 2, 27, 1578, make_span(testb_fields28) },
    { string_view("S1", 2), 29, 1, -1, 735, make_span(S1_fields29) },
    { string_view("S2", 2), 30, 2, 29, 744, make_span(S2_fields30) },
    { string_view("C0", 2), 31, 0, -1, 756, {} },
    { string_view("C1", 2), 32, 1, 31, 762, make_span(C1_fields32) },
    { string_view("C2", 2), 33, 2, 32, 771, make_span(C2_fields33) },
    { string_view("Ai", 2), 34, 5, -1, 804, make_span(Ai_fields34) },
    { string_view("A", 1), 35, 5, -1, 829, make_span(A_fields35) },
    { string_view("A", 1), 36, 1, -1, 872, make_span(A_fields36) },
    { string_view("B", 1), 37, 2, 36, 883, make_span(B_fields37) },
    { string_view("A", 1), 38, 1, -1, 1590, make_span(A_fields38) },
    { string_view("A", 1), 39, 4, -1, 897, make_span(A_fields39) },
    { string_view("goapaction", 10), 40, 0, -1, 138, {} },
    { string_view("pathingcell", 11), 41, 12, 42, 919, make_span(pathingcell_fields41) },
    { string_view("astar_node", 10), 42, 10, -1, 961, make_span(astar_node_fields42) },
    { string_view("resources", 9), 43, 7, -1, 111, make_span(resources_fields43) },
    { string_view("KillWolf", 8), 44, 0, 40, 1029, {} },
    { string_view("SellSkin", 8), 45, 0, 40, 1035, {} },
    { string_view("BuyPizza", 8), 46, 0, 40, 1041, {} },
    { string_view("BuyFlour", 8), 47, 0, 40, 1047, {} },
    { string_view("BakeBread", 9), 48, 0, 40, 1053, {} },
    { string_view("Eat", 3), 49, 0, 40, 1059, {} },
    { string_view("spec_node", 9), 50, 8, 51, 49, make_span(spec_node_fields50) },
    { string_view("astar_node", 10), 51, 8, -1, 79, make_span(astar_node_fields51) },
    { string_view("node", 4), 52, 0, -1, 1074, {} },
    { string_view("integer", 7), 53, 1, 52, 1065, make_span(integer_fields53) },
    { string_view("inst", 4), 54, 2, 52, 1080, make_span(inst_fields54) },
    { string_view("pathmap", 7), 55, 5, -1, 1098, make_span(pathmap_fields55) },
    { string_view("cell", 4), 56, 3, -1, 1123, make_span(cell_fields56) },
    { string_view("triangle", 8), 57, 2, -1, 1599, make_span(triangle_fields57) },
    { string_view("node", 4), 58, 2, -1, 1611, make_span(node_fields58) },
    { string_view("pedge", 5), 59, 3, -1, 1150, make_span(pedge_fields59) },
    { string_view("color", 5), 60, 4, -1, 1167, make_span(color_fields60) },
    { string_view("Ray", 3), 61, 6, -1, 1226, make_span(Ray_fields61) },
    { string_view("Sphere", 6), 62, 11, -1, 1185, make_span(Sphere_fields62) },
    { string_view("Nest", 4), 63, 7, -1, 1260, make_span(Nest_fields63) },
    { string_view("quat", 4), 64, 4, 4, 1291, make_span(quat_fields64) },
    { string_view("OC", 2), 65, 1, -1, 1309, make_span(OC_fields65) },
    { string_view("OS", 2), 66, 1, -1, 1318, make_span(OS_fields66) },
    { string_view("mat2x2", 6), 67, 4, 4, 242, make_span(mat2x2_fields67) },
    { string_view("A", 1), 68, 1, -1, 1327, make_span(A_fields68) },
    { string_view("B", 1), 69, 2, 68, 1623, make_span(B_fields69) },
    { string_view("C", 1), 70, 2, 68, 1650, make_span(C_fields70) },
    { string_view("D", 1), 71, 3, 70, 1635, make_span(D_fields71) },
    { string_view("E", 1), 72, 3, 70, 1662, make_span(E_fields72) },
    { string_view("F", 1), 73, 4, 72, 1677, make_span(F_fields73) },
    { string_view("dictionary", 10), 74, 1, -1, 460, make_span(dictionary_fields74) },
    { string_view("chain", 5), 75, 3, -1, 469, make_span(chain_fields75) },
    { string_view("dictionary", 10), 76, 1, -1, 631, make_span(dictionary_fields76) },
    { string_view("chain", 5), 77, 3, -1, 640, make_span(chain_fields77) },
    { string_view("dictionary", 10), 78, 1, -1, 659, make_span(dictionary_fields78) },
    { string_view("chain", 5), 79, 7, -1, 668, make_span(chain_fields79) },
    { string_view("dictionary", 10), 80, 1, -1, 514, make_span(dictionary_fields80) },
    { string_view("chain", 5), 81, 3, -1, 523, make_span(chain_fields81) },
    { string_view("dictionary", 10), 82, 1, -1, 488, make_span(dictionary_fields82) },
    { string_view("chain", 5), 83, 3, -1, 497, make_span(chain_fields83) },
    { string_view("C3", 2), 84, 2, 32, 783, make_span(C3_fields84) },
    { string_view("D", 1), 85, 1, -1, 795, make_span(D_fields85) },
    { string_view("D", 1), 86, 1, -1, 1402, make_span(D_fields86) },
    { string_view("set1", 4), 87, 1, -1, 854, make_span(set1_fields87) },
    { string_view("set2", 4), 88, 1, -1, 863, make_span(set2_fields88) },
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
    { string_view("body", 4), 347, 454, 0, 0, 0 },
    { string_view("_", 1), 348, 3, 0, 0, 0 },
    { string_view("this", 4), 352, 460, 0, 0, 0 },
    { string_view("key", 3), 353, 2, 0, 0, 0 },
    { string_view("h", 1), 354, 0, 0, 1, 0 },
    { string_view("c", 1), 355, 484, 0, 0, 0 },
    { string_view("this", 4), 356, 460, 0, 0, 0 },
    { string_view("key", 3), 357, 2, 0, 0, 0 },
    { string_view("value", 5), 358, 2, 0, 0, 0 },
    { string_view("c", 1), 359, 484, 0, 1, 0 },
    { string_view("h", 1), 360, 0, 0, 1, 0 },
    { string_view("this", 4), 372, 488, 0, 0, 0 },
    { string_view("key", 3), 373, 3, 0, 0, 0 },
    { string_view("c", 1), 374, 512, 0, 1, 0 },
    { string_view("this", 4), 375, 460, 0, 0, 0 },
    { string_view("key", 3), 376, 2, 0, 0, 0 },
    { string_view("default_val", 11), 377, 2, 0, 0, 0 },
    { string_view("c", 1), 378, 484, 0, 1, 0 },
    { string_view("size", 4), 388, 0, 0, 0, 0 },
    { string_view("this", 4), 389, 544, 0, 0, 0 },
    { string_view("vardef", 6), 390, 0, 0, 0, 0 },
    { string_view("constdef", 8), 391, 0, 0, 1, 0 },
    { string_view("x", 1), 392, 0, 0, 0, 0 },
    { string_view("y", 1), 393, 0, 0, 0, 0 },
    { string_view("x", 1), 394, 3, 0, 0, 0 },
    { string_view("y", 1), 395, 0, 0, 0, 0 },
    { string_view("x", 1), 396, 553, 0, 0, 0 },
    { string_view("y", 1), 397, 0, 0, 0, 0 },
    { string_view("x", 1), 398, 562, 0, 0, 0 },
    { string_view("y", 1), 399, 0, 0, 0, 0 },
    { string_view("a", 1), 400, 553, 0, 0, 0 },
    { string_view("direct", 6), 401, 574, 0, 1, 0 },
    { string_view("parsed", 6), 402, 613, 0, 0, 0 },
    { string_view("err", 3), 403, 458, 0, 0, 0 },
    { string_view("groundv", 7), 404, 574, 0, 1, 0 },
    { string_view("flex", 4), 405, 3, 0, 1, 0 },
    { string_view("json", 4), 406, 458, 0, 1, 0 },
    { string_view("groundj", 7), 407, 3, 0, 1, 0 },
    { string_view("flex2", 5), 408, 3, 0, 1, 0 },
    { string_view("ferr", 4), 409, 458, 0, 1, 0 },
    { string_view("json2", 5), 410, 458, 0, 1, 0 },
    { string_view("fval", 4), 411, 613, 0, 1, 0 },
    { string_view("fverr", 5), 412, 458, 0, 1, 0 },
    { string_view("lb", 2), 413, 3, 0, 1, 0 },
    { string_view("lbval", 5), 414, 613, 0, 1, 0 },
    { string_view("lberr", 5), 415, 458, 0, 1, 0 },
    { string_view("unicodetests", 12), 416, 7, 0, 1, 0 },
    { string_view("st", 2), 417, 11, 0, 1, 0 },
    { string_view("i", 1), 418, 0, 0, 0, 0 },
    { string_view("x", 1), 419, 0, 0, 1, 0 },
    { string_view("a", 1), 420, 0, 0, 0, 0 },
    { string_view("b", 1), 421, 7, 0, 1, 0 },
    { string_view("testvector", 10), 422, 7, 0, 1, 0 },
    { string_view("sorted1", 7), 423, 7, 0, 1, 0 },
    { string_view("_a", 2), 424, 0, 0, 0, 0 },
    { string_view("_b", 2), 425, 0, 0, 0, 0 },
    { string_view("sorted2", 7), 426, 7, 0, 1, 0 },
    { string_view("_a", 2), 427, 0, 0, 0, 0 },
    { string_view("_b", 2), 428, 0, 0, 0, 0 },
    { string_view("sorted3", 7), 429, 7, 0, 1, 0 },
    { string_view("_a", 2), 430, 0, 0, 0, 0 },
    { string_view("_b", 2), 431, 0, 0, 0, 0 },
    { string_view("found", 5), 432, 0, 0, 0, 0 },
    { string_view("findex", 6), 433, 0, 0, 0, 0 },
    { string_view("_", 1), 434, 0, 0, 0, 0 },
    { string_view("n", 1), 435, 0, 0, 0, 0 },
    { string_view("applyto4", 8), 436, 615, 0, 1, 0 },
    { string_view("f", 1), 437, 617, 0, 0, 0 },
    { string_view("x", 1), 438, 0, 0, 0, 0 },
    { string_view("ns", 2), 439, 3, 0, 0, 0 },
    { string_view("ni", 2), 440, 0, 0, 0, 0 },
    { string_view("s1", 2), 441, 3, 0, 0, 0 },
    { string_view("s2", 2), 442, 3, 0, 0, 0 },
    { string_view("aa", 2), 443, 0, 0, 0, 0 },
    { string_view("bb", 2), 444, 0, 0, 1, 0 },
    { string_view("cc", 2), 445, 0, 0, 1, 0 },
    { string_view("dd", 2), 446, 0, 0, 1, 0 },
    { string_view("ee", 2), 447, 0, 0, 1, 0 },
    { string_view("ff", 2), 448, 0, 0, 0, 0 },
    { string_view("gg", 2), 449, 0, 0, 0, 0 },
    { string_view("hh", 2), 450, 0, 0, 1, 0 },
    { string_view("ii", 2), 451, 0, 0, 1, 0 },
    { string_view("ll", 2), 452, 0, 0, 1, 0 },
    { string_view("jj", 2), 453, 0, 0, 1, 0 },
    { string_view("kk", 2), 454, 0, 0, 1, 0 },
    { string_view("nn", 2), 455, 2, 0, 1, 0 },
    { string_view("mm", 2), 456, 3, 0, 1, 0 },
    { string_view("this", 4), 457, 619, 0, 0, 0 },
    { string_view("n", 1), 458, 0, 0, 0, 0 },
    { string_view("a", 1), 459, 0, 0, 0, 0 },
    { string_view("i", 1), 460, 0, 0, 0, 0 },
    { string_view("n", 1), 461, 0, 1, 0, 0 },
    { string_view("v", 1), 462, 3, 0, 0, 0 },
    { string_view("_x", 2), 463, 0, 0, 0, 0 },
    { string_view("_y", 2), 464, 0, 0, 0, 0 },
    { string_view("bit", 3), 465, 0, 0, 0, 0 },
    { string_view("dict", 4), 466, 460, 0, 1, 0 },
    { string_view("key", 3), 467, 2, 0, 1, 0 },
    { string_view("key", 3), 468, 2, 0, 1, 0 },
    { string_view("value", 5), 469, 2, 0, 1, 0 },
    { string_view("dict2", 5), 470, 631, 0, 1, 0 },
    { string_view("val", 3), 471, 458, 0, 1, 0 },
    { string_view("dict3", 5), 472, 659, 0, 1, 0 },
    { string_view("lc", 2), 473, 0, 0, 0, 0 },
    { string_view("i", 1), 474, 0, 0, 0, 0 },
    { string_view("a", 1), 475, 0, 0, 1, 0 },
    { string_view("b", 1), 476, 0, 0, 1, 0 },
    { string_view("mode", 4), 477, 0, 1, 0, 0 },
    { string_view("dav", 3), 478, 0, 0, 1, 0 },
    { string_view("a", 1), 479, 0, 0, 0, 0 },
    { string_view("b", 1), 480, 0, 0, 0, 0 },
    { string_view("c", 1), 481, 0, 0, 0, 0 },
    { string_view("a", 1), 485, 0, 0, 0, 0 },
    { string_view("slv", 3), 486, 3, 0, 0, 0 },
    { string_view("vlv", 3), 487, 7, 0, 0, 0 },
    { string_view("stlv", 4), 488, 699, 0, 0, 0 },
    { string_view("stlv", 4), 488, 699, 0, 0, 0 },
    { string_view("divisor", 7), 489, 2, 0, 1, 0 },
    { string_view("r", 1), 490, 711, 0, 1, 0 },
    { string_view("state", 5), 491, 0, 1, 0, 0 },
    { string_view("fv", 2), 492, 729, 0, 1, 0 },
    { string_view("gs", 2), 493, 7, 0, 1, 0 },
    { string_view("i", 1), 494, 0, 0, 0, 0 },
    { string_view("i", 1), 495, 731, 0, 0, 0 },
    { string_view("f", 1), 496, 733, 1, 0, 0 },
    { string_view("x", 1), 497, 0, 0, 0, 0 },
    { string_view("a", 1), 498, 0, 0, 0, 0 },
    { string_view("b", 1), 499, 146, 0, 0, 0 },
    { string_view("this", 4), 500, 735, 0, 0, 0 },
    { string_view("this", 4), 501, 744, 0, 0, 0 },
    { string_view("this", 4), 501, 744, 0, 0, 0 },
    { string_view("this", 4), 502, 756, 0, 0, 0 },
    { string_view("this", 4), 503, 762, 0, 0, 0 },
    { string_view("c", 1), 504, 0, 0, 0, 0 },
    { string_view("this", 4), 505, 762, 0, 0, 0 },
    { string_view("c", 1), 506, 3, 0, 0, 0 },
    { string_view("this", 4), 507, 762, 0, 0, 0 },
    { string_view("c", 1), 508, 0, 0, 0, 0 },
    { string_view("this", 4), 509, 762, 0, 0, 0 },
    { string_view("this", 4), 510, 762, 0, 0, 0 },
    { string_view("this", 4), 511, 762, 0, 0, 0 },
    { string_view("this", 4), 512, 771, 0, 0, 0 },
    { string_view("c", 1), 513, 0, 0, 0, 0 },
    { string_view("this", 4), 514, 771, 0, 0, 0 },
    { string_view("c", 1), 515, 3, 0, 0, 0 },
    { string_view("this", 4), 516, 771, 0, 0, 0 },
    { string_view("c", 1), 517, 0, 0, 0, 0 },
    { string_view("this", 4), 518, 771, 0, 0, 0 },
    { string_view("this", 4), 519, 771, 0, 0, 0 },
    { string_view("this", 4), 520, 771, 0, 0, 0 },
    { string_view("this", 4), 521, 783, 0, 0, 0 },
    { string_view("c", 1), 522, 0, 0, 0, 0 },
    { string_view("x", 1), 523, 762, 0, 1, 0 },
    { string_view("x2", 2), 524, 762, 0, 1, 0 },
    { string_view("x3", 2), 525, 756, 0, 1, 0 },
    { string_view("mr1", 3), 526, 0, 0, 1, 0 },
    { string_view("mr2", 3), 527, 3, 0, 1, 0 },
    { string_view("d", 1), 528, 795, 0, 0, 0 },
    { string_view("i", 1), 529, 0, 0, 0, 0 },
    { string_view("s", 1), 530, 3, 0, 0, 0 },
    { string_view("t", 1), 531, 2, 0, 0, 0 },
    { string_view("a", 1), 532, 458, 0, 1, 0 },
    { string_view("b", 1), 533, 458, 0, 1, 0 },
    { string_view("x", 1), 534, 3, 0, 0, 0 },
    { string_view("this", 4), 535, 804, 0, 0, 0 },
    { string_view("t", 1), 536, 0, 0, 0, 0 },
    { string_view("this", 4), 537, 804, 0, 0, 0 },
    { string_view("u", 1), 538, 0, 0, 0, 0 },
    { string_view("this", 4), 539, 804, 0, 0, 0 },
    { string_view("x", 1), 540, 0, 0, 1, 0 },
    { string_view("a", 1), 541, 804, 0, 0, 0 },
    { string_view("t", 1), 542, 0, 0, 0, 0 },
    { string_view("a", 1), 543, 804, 0, 1, 0 },
    { string_view("b", 1), 544, 829, 0, 1, 0 },
    { string_view("x", 1), 545, 2, 0, 0, 0 },
    { string_view("y", 1), 546, 2, 0, 0, 0 },
    { string_view("t", 1), 547, 2, 0, 0, 0 },
    { string_view("r", 1), 548, 2, 0, 1, 0 },
    { string_view("s", 1), 549, 2, 0, 1, 0 },
    { string_view("aa", 2), 550, 825, 0, 0, 0 },
    { string_view("aa", 2), 551, 804, 0, 0, 0 },
    { string_view("aa", 2), 552, 825, 0, 0, 0 },
    { string_view("a1", 2), 553, 854, 0, 1, 0 },
    { string_view("a2", 2), 554, 863, 0, 1, 0 },
    { string_view("xs", 2), 556, 7, 0, 0, 0 },
    { string_view("as", 2), 557, 881, 0, 1, 0 },
    { string_view("bs", 2), 558, 895, 0, 1, 0 },
    { string_view("xs", 2), 559, 881, 0, 1, 0 },
    { string_view("ys", 2), 560, 881, 0, 0, 0 },
    { string_view("as", 2), 561, 917, 0, 1, 0 },
    { string_view("a", 1), 562, 897, 0, 0, 0 },
    { string_view("startnode", 9), 564, 919, 0, 0, 0 },
    { string_view("endcondition", 12), 565, 999, 0, 0, 0 },
    { string_view("generatenewstates", 17), 566, 1001, 0, 0, 0 },
    { string_view("heuristic", 9), 567, 1003, 1, 0, 0 },
    { string_view("openlist", 8), 568, 1005, 1, 1, 0 },
    { string_view("n", 1), 569, 997, 1, 0, 0 },
    { string_view("delta", 5), 570, 25, 0, 0, 0 },
    { string_view("delta", 5), 570, 25, 0, 0, 0 },
    { string_view("cost", 4), 571, 0, 0, 0, 0 },
    { string_view("nn", 2), 572, 919, 0, 0, 0 },
    { string_view("G", 1), 573, 2, 0, 1, 0 },
    { string_view("c", 1), 574, 919, 0, 0, 0 },
    { string_view("path", 4), 575, 1005, 0, 1, 0 },
    { string_view("startnode", 9), 576, 919, 0, 0, 0 },
    { string_view("endnode", 7), 577, 919, 1, 0, 0 },
    { string_view("costf", 5), 578, 1007, 1, 0, 0 },
    { string_view("distancef", 9), 579, 1009, 1, 0, 0 },
    { string_view("neighbors", 9), 580, 1011, 1, 0, 0 },
    { string_view("n", 1), 581, 919, 0, 0, 0 },
    { string_view("n", 1), 582, 919, 1, 0, 0 },
    { string_view("f", 1), 583, 1013, 1, 0, 0 },
    { string_view("nn", 2), 584, 919, 0, 0, 0 },
    { string_view("cost", 4), 585, 0, 0, 1, 0 },
    { string_view("state", 5), 586, 25, 0, 0, 0 },
    { string_view("state", 5), 586, 25, 0, 0, 0 },
    { string_view("isocta", 6), 587, 146, 0, 0, 0 },
    { string_view("gridsize", 8), 588, 25, 1, 0, 0 },
    { string_view("gridsize", 8), 588, 25, 1, 0, 0 },
    { string_view("startnode", 9), 589, 919, 1, 0, 0 },
    { string_view("endnode", 7), 590, 919, 1, 0, 0 },
    { string_view("getnode", 7), 591, 1015, 1, 0, 0 },
    { string_view("costf", 5), 592, 1007, 1, 0, 0 },
    { string_view("directions", 10), 593, 37, 1, 0, 0 },
    { string_view("distancef", 9), 594, 1009, 0, 0, 0 },
    { string_view("n", 1), 595, 919, 0, 0, 0 },
    { string_view("f", 1), 596, 1017, 0, 0, 0 },
    { string_view("delta", 5), 597, 25, 0, 0, 0 },
    { string_view("delta", 5), 597, 25, 0, 0, 0 },
    { string_view("np", 2), 598, 25, 0, 1, 0 },
    { string_view("np", 2), 598, 25, 0, 1, 0 },
    { string_view("v", 1), 599, 25, 0, 0, 0 },
    { string_view("v", 1), 599, 25, 0, 0, 0 },
    { string_view("x", 1), 600, 0, 0, 1, 0 },
    { string_view("y", 1), 601, 0, 0, 1, 0 },
    { string_view("big", 3), 602, 0, 0, 1, 0 },
    { string_view("small", 5), 603, 0, 0, 1, 0 },
    { string_view("_", 1), 604, 25, 0, 0, 0 },
    { string_view("_", 1), 604, 25, 0, 0, 0 },
    { string_view("goapactions", 11), 609, 1019, 1, 0, 0 },
    { string_view("initstate", 9), 610, 111, 0, 0, 0 },
    { string_view("heuristic", 9), 611, 1021, 1, 0, 0 },
    { string_view("endcondition", 12), 612, 1023, 1, 0, 0 },
    { string_view("H", 1), 613, 0, 0, 1, 0 },
    { string_view("existingnodes", 13), 614, 148, 1, 1, 0 },
    { string_view("_", 1), 615, 49, 0, 0, 0 },
    { string_view("n", 1), 616, 49, 0, 0, 0 },
    { string_view("f", 1), 617, 1025, 0, 0, 0 },
    { string_view("act", 3), 618, 138, 0, 0, 0 },
    { string_view("nstate", 6), 619, 111, 1, 1, 0 },
    { string_view("i", 1), 620, 0, 0, 0, 0 },
    { string_view("_", 1), 621, 49, 0, 0, 0 },
    { string_view("_", 1), 622, 111, 0, 0, 0 },
    { string_view("initworld", 9), 623, 11, 1, 1, 0 },
    { string_view("expected_result", 15), 624, 11, 0, 1, 0 },
    { string_view("worldsize", 9), 625, 25, 1, 1, 0 },
    { string_view("worldsize", 9), 625, 25, 1, 1, 0 },
    { string_view("startpos", 8), 626, 25, 1, 0, 0 },
    { string_view("startpos", 8), 626, 25, 1, 0, 0 },
    { string_view("endpos", 6), 627, 25, 1, 0, 0 },
    { string_view("endpos", 6), 627, 25, 1, 0, 0 },
    { string_view("world", 5), 628, 1027, 1, 1, 0 },
    { string_view("y", 1), 629, 0, 1, 0, 0 },
    { string_view("x", 1), 630, 0, 0, 0, 0 },
    { string_view("c", 1), 631, 0, 0, 1, 0 },
    { string_view("path", 4), 632, 1005, 0, 1, 0 },
    { string_view("_", 1), 633, 25, 0, 0, 0 },
    { string_view("_", 1), 633, 25, 0, 0, 0 },
    { string_view("n", 1), 634, 919, 0, 0, 0 },
    { string_view("nn", 2), 635, 919, 0, 0, 0 },
    { string_view("n", 1), 636, 919, 0, 0, 0 },
    { string_view("astar_result", 12), 637, 11, 0, 1, 0 },
    { string_view("row", 3), 638, 1005, 0, 0, 0 },
    { string_view("accum", 5), 639, 3, 0, 0, 0 },
    { string_view("n", 1), 640, 919, 0, 0, 0 },
    { string_view("this", 4), 641, 1029, 0, 0, 0 },
    { string_view("s", 1), 642, 111, 0, 0, 0 },
    { string_view("this", 4), 643, 1029, 0, 0, 0 },
    { string_view("s", 1), 644, 111, 0, 0, 0 },
    { string_view("this", 4), 645, 1035, 0, 0, 0 },
    { string_view("s", 1), 646, 111, 0, 0, 0 },
    { string_view("this", 4), 647, 1035, 0, 0, 0 },
    { string_view("s", 1), 648, 111, 0, 0, 0 },
    { string_view("this", 4), 649, 1041, 0, 0, 0 },
    { string_view("s", 1), 650, 111, 0, 0, 0 },
    { string_view("this", 4), 651, 1041, 0, 0, 0 },
    { string_view("s", 1), 652, 111, 0, 0, 0 },
    { string_view("this", 4), 653, 1047, 0, 0, 0 },
    { string_view("s", 1), 654, 111, 0, 0, 0 },
    { string_view("this", 4), 655, 1047, 0, 0, 0 },
    { string_view("s", 1), 656, 111, 0, 0, 0 },
    { string_view("this", 4), 657, 1053, 0, 0, 0 },
    { string_view("s", 1), 658, 111, 0, 0, 0 },
    { string_view("this", 4), 659, 1053, 0, 0, 0 },
    { string_view("s", 1), 660, 111, 0, 0, 0 },
    { string_view("this", 4), 661, 1059, 0, 0, 0 },
    { string_view("s", 1), 662, 111, 0, 0, 0 },
    { string_view("this", 4), 663, 1059, 0, 0, 0 },
    { string_view("s", 1), 664, 111, 0, 0, 0 },
    { string_view("goapactions", 11), 665, 1019, 0, 1, 0 },
    { string_view("goap_path", 9), 666, 148, 0, 1, 0 },
    { string_view("state", 5), 667, 111, 0, 0, 0 },
    { string_view("s", 1), 668, 111, 0, 0, 0 },
    { string_view("n", 1), 669, 49, 0, 0, 0 },
    { string_view("knight_moves", 12), 670, 37, 1, 1, 0 },
    { string_view("dim", 3), 671, 25, 1, 0, 0 },
    { string_view("dim", 3), 671, 25, 1, 0, 0 },
    { string_view("board", 5), 672, 13, 1, 1, 0 },
    { string_view("cur", 3), 673, 25, 1, 0, 0 },
    { string_view("cur", 3), 673, 25, 1, 0, 0 },
    { string_view("step", 4), 674, 0, 0, 0, 0 },
    { string_view("candidates", 10), 675, 37, 0, 0, 0 },
    { string_view("m", 1), 676, 25, 0, 0, 0 },
    { string_view("m", 1), 676, 25, 0, 0, 0 },
    { string_view("v", 1), 677, 25, 0, 0, 0 },
    { string_view("v", 1), 677, 25, 0, 0, 0 },
    { string_view("c", 1), 678, 25, 0, 0, 0 },
    { string_view("c", 1), 678, 25, 0, 0, 0 },
    { string_view("degrees", 7), 679, 7, 0, 1, 0 },
    { string_view("c", 1), 680, 25, 1, 0, 0 },
    { string_view("c", 1), 680, 25, 1, 0, 0 },
    { string_view("m", 1), 681, 25, 0, 0, 0 },
    { string_view("m", 1), 681, 25, 0, 0, 0 },
    { string_view("d", 1), 682, 0, 0, 0, 0 },
    { string_view("this", 4), 683, 1065, 0, 0, 0 },
    { string_view("this", 4), 684, 1065, 0, 0, 0 },
    { string_view("this", 4), 685, 1065, 0, 0, 0 },
    { string_view("this", 4), 686, 1080, 0, 0, 0 },
    { string_view("elems", 5), 687, 3, 0, 1, 0 },
    { string_view("_a", 2), 688, 3, 0, 0, 0 },
    { string_view("_e", 2), 689, 1074, 0, 0, 0 },
    { string_view("this", 4), 690, 1080, 0, 0, 0 },
    { string_view("_a", 2), 691, 0, 0, 0, 0 },
    { string_view("_e", 2), 692, 1074, 0, 0, 0 },
    { string_view("_a", 2), 693, 0, 0, 0, 0 },
    { string_view("_e", 2), 694, 1074, 0, 0, 0 },
    { string_view("this", 4), 695, 1080, 1, 0, 0 },
    { string_view("_", 1), 696, 1074, 0, 0, 0 },
    { string_view("_a", 2), 697, 3, 0, 0, 0 },
    { string_view("_e", 2), 698, 3, 0, 0, 0 },
    { string_view("s", 1), 699, 3, 1, 0, 0 },
    { string_view("token", 5), 700, 3, 1, 0, 0 },
    { string_view("atom", 4), 701, 3, 1, 0, 0 },
    { string_view("i", 1), 702, 0, 1, 0, 0 },
    { string_view("line", 4), 703, 0, 1, 0, 0 },
    { string_view("ival", 4), 704, 0, 1, 0, 0 },
    { string_view("err", 3), 705, 3, 0, 0, 0 },
    { string_view("start", 5), 706, 0, 0, 1, 0 },
    { string_view("_", 1), 707, 0, 0, 0, 0 },
    { string_view("all", 3), 708, 146, 0, 0, 0 },
    { string_view("tok", 3), 709, 3, 0, 0, 0 },
    { string_view("a", 1), 710, 3, 0, 1, 0 },
    { string_view("name", 4), 711, 3, 1, 1, 0 },
    { string_view("_", 1), 712, 3, 0, 0, 0 },
    { string_view("n", 1), 713, 1080, 0, 1, 0 },
    { string_view("n", 1), 714, 1065, 0, 1, 0 },
    { string_view("root", 4), 715, 1074, 0, 1, 0 },
    { string_view("test_code", 9), 716, 3, 0, 1, 0 },
    { string_view("ast", 3), 717, 1094, 0, 1, 0 },
    { string_view("err", 3), 718, 3, 0, 1, 0 },
    { string_view("code", 4), 719, 3, 0, 1, 0 },
    { string_view("sz", 2), 720, 25, 1, 0, 0 },
    { string_view("sz", 2), 720, 25, 1, 0, 0 },
    { string_view("target", 6), 721, 25, 0, 0, 0 },
    { string_view("target", 6), 721, 25, 0, 0, 0 },
    { string_view("traversablef", 12), 722, 1096, 1, 0, 0 },
    { string_view("m", 1), 723, 1121, 1, 1, 0 },
    { string_view("v", 1), 724, 25, 0, 0, 0 },
    { string_view("v", 1), 724, 25, 0, 0, 0 },
    { string_view("end", 3), 725, 1098, 0, 1, 0 },
    { string_view("pathq", 5), 726, 1119, 1, 1, 0 },
    { string_view("dir", 3), 727, 25, 0, 0, 0 },
    { string_view("dir", 3), 727, 25, 0, 0, 0 },
    { string_view("parent", 6), 728, 1098, 0, 0, 0 },
    { string_view("pos", 3), 729, 25, 0, 1, 0 },
    { string_view("pos", 3), 729, 25, 0, 1, 0 },
    { string_view("c", 1), 730, 1098, 0, 1, 0 },
    { string_view("i", 1), 731, 0, 0, 0, 0 },
    { string_view("c", 1), 732, 1098, 0, 1, 0 },
    { string_view("x", 1), 733, 0, 0, 0, 0 },
    { string_view("y", 1), 734, 0, 0, 0, 0 },
    { string_view("v", 1), 735, 25, 0, 0, 0 },
    { string_view("v", 1), 735, 25, 0, 0, 0 },
    { string_view("lssize", 6), 736, 0, 1, 1, 0 },
    { string_view("hmargin", 7), 737, 2, 1, 1, 0 },
    { string_view("highest", 7), 738, 25, 1, 0, 0 },
    { string_view("highest", 7), 738, 25, 1, 0, 0 },
    { string_view("lowest", 6), 739, 25, 1, 0, 0 },
    { string_view("lowest", 6), 739, 25, 1, 0, 0 },
    { string_view("highh", 5), 740, 2, 1, 0, 0 },
    { string_view("lowh", 4), 741, 2, 1, 0, 0 },
    { string_view("cells", 5), 742, 1140, 1, 1, 0 },
    { string_view("y", 1), 743, 0, 1, 0, 0 },
    { string_view("x", 1), 744, 0, 0, 0, 0 },
    { string_view("h", 1), 745, 2, 0, 0, 0 },
    { string_view("f", 1), 746, 1142, 0, 0, 0 },
    { string_view("y", 1), 747, 0, 0, 0, 0 },
    { string_view("x", 1), 748, 0, 0, 0, 0 },
    { string_view("pos", 3), 749, 25, 0, 0, 0 },
    { string_view("pos", 3), 749, 25, 0, 0, 0 },
    { string_view("f", 1), 750, 1144, 0, 0, 0 },
    { string_view("neighbours", 10), 751, 37, 1, 1, 0 },
    { string_view("x", 1), 752, 0, 0, 0, 0 },
    { string_view("y", 1), 753, 0, 0, 0, 0 },
    { string_view("f", 1), 754, 1146, 1, 0, 0 },
    { string_view("n", 1), 755, 25, 1, 0, 0 },
    { string_view("n", 1), 755, 25, 1, 0, 0 },
    { string_view("starting_extra_water", 20), 756, 2, 0, 1, 0 },
    { string_view("water_per_second", 16), 757, 2, 1, 1, 0 },
    { string_view("give_water_divider", 18), 758, 2, 1, 1, 0 },
    { string_view("minimum_water_accumulation", 26), 759, 2, 1, 1, 0 },
    { string_view("x", 1), 760, 0, 0, 0, 0 },
    { string_view("y", 1), 761, 0, 0, 0, 0 },
    { string_view("c", 1), 762, 1123, 1, 0, 0 },
    { string_view("totald", 6), 763, 2, 1, 0, 0 },
    { string_view("lower", 5), 764, 1138, 1, 1, 0 },
    { string_view("nc", 2), 765, 1123, 0, 0, 0 },
    { string_view("d", 1), 766, 2, 0, 1, 0 },
    { string_view("w", 1), 767, 2, 0, 1, 0 },
    { string_view("nc", 2), 768, 1123, 0, 0, 0 },
    { string_view("give", 4), 769, 2, 0, 1, 0 },
    { string_view("x", 1), 770, 0, 0, 0, 0 },
    { string_view("y", 1), 771, 0, 0, 0, 0 },
    { string_view("c", 1), 772, 1123, 0, 0, 0 },
    { string_view("N", 1), 773, 0, 1, 1, 0 },
    { string_view("f", 1), 774, 2, 1, 1, 0 },
    { string_view("x", 1), 775, 9, 0, 0, 0 },
    { string_view("d", 1), 776, 9, 0, 0, 0 },
    { string_view("mu", 2), 777, 2, 0, 0, 0 },
    { string_view("n_epochs", 8), 778, 0, 0, 0, 0 },
    { string_view("y", 1), 779, 9, 0, 1, 0 },
    { string_view("w0", 2), 780, 2, 0, 0, 0 },
    { string_view("w1", 2), 781, 2, 0, 0, 0 },
    { string_view("grad0", 5), 782, 2, 0, 0, 0 },
    { string_view("grad1", 5), 783, 2, 0, 0, 0 },
    { string_view("i", 1), 784, 0, 0, 0, 0 },
    { string_view("err", 3), 785, 2, 0, 1, 0 },
    { string_view("i", 1), 786, 0, 0, 0, 0 },
    { string_view("sigma", 5), 787, 2, 0, 1, 0 },
    { string_view("mu", 2), 788, 2, 0, 1, 0 },
    { string_view("n_epochs", 8), 789, 0, 0, 1, 0 },
    { string_view("x", 1), 790, 9, 0, 1, 0 },
    { string_view("d", 1), 791, 9, 0, 1, 0 },
    { string_view("i", 1), 792, 0, 0, 0, 0 },
    { string_view("verts", 5), 793, 1148, 1, 0, 0 },
    { string_view("edges", 5), 794, 13, 0, 0, 0 },
    { string_view("quads", 5), 795, 13, 0, 0, 0 },
    { string_view("disps", 5), 796, 1148, 0, 1, 0 },
    { string_view("v", 1), 797, 162, 0, 0, 0 },
    { string_view("v", 1), 797, 162, 0, 0, 0 },
    { string_view("w", 1), 798, 162, 0, 0, 0 },
    { string_view("w", 1), 798, 162, 0, 0, 0 },
    { string_view("i", 1), 799, 0, 0, 0, 0 },
    { string_view("tow", 3), 800, 162, 0, 1, 0 },
    { string_view("tow", 3), 800, 162, 0, 1, 0 },
    { string_view("dist", 4), 801, 2, 0, 1, 0 },
    { string_view("totaldisp", 9), 802, 2, 0, 0, 0 },
    { string_view("d", 1), 803, 162, 0, 0, 0 },
    { string_view("d", 1), 803, 162, 0, 0, 0 },
    { string_view("i", 1), 804, 0, 0, 0, 0 },
    { string_view("wsize", 5), 805, 25, 1, 1, 0 },
    { string_view("wsize", 5), 805, 25, 1, 1, 0 },
    { string_view("p", 1), 806, 25, 0, 0, 0 },
    { string_view("p", 1), 806, 25, 0, 0, 0 },
    { string_view("pedges", 6), 807, 1165, 0, 0, 0 },
    { string_view("v", 1), 808, 162, 0, 0, 0 },
    { string_view("v", 1), 808, 162, 0, 0, 0 },
    { string_view("i", 1), 809, 0, 0, 0, 0 },
    { string_view("w", 1), 810, 162, 0, 0, 0 },
    { string_view("w", 1), 810, 162, 0, 0, 0 },
    { string_view("j", 1), 811, 0, 0, 0, 0 },
    { string_view("dist", 4), 812, 2, 0, 1, 0 },
    { string_view("a", 1), 813, 1150, 0, 0, 0 },
    { string_view("a", 1), 813, 1150, 0, 0, 0 },
    { string_view("a", 1), 813, 1150, 0, 0, 0 },
    { string_view("b", 1), 814, 1150, 0, 0, 0 },
    { string_view("b", 1), 814, 1150, 0, 0, 0 },
    { string_view("b", 1), 814, 1150, 0, 0, 0 },
    { string_view("edgedistnocross", 15), 815, 2, 0, 1, 0 },
    { string_view("vnbs", 4), 816, 13, 0, 0, 0 },
    { string_view("pe", 2), 817, 1150, 0, 0, 0 },
    { string_view("pe", 2), 817, 1150, 0, 0, 0 },
    { string_view("pe", 2), 817, 1150, 0, 0, 0 },
    { string_view("nbl", 3), 818, 7, 0, 0, 0 },
    { string_view("i", 1), 819, 0, 1, 0, 0 },
    { string_view("a", 1), 820, 0, 0, 0, 0 },
    { string_view("b", 1), 821, 0, 0, 0, 0 },
    { string_view("tris", 4), 822, 13, 0, 1, 0 },
    { string_view("i", 1), 823, 0, 0, 0, 0 },
    { string_view("j", 1), 824, 0, 0, 0, 0 },
    { string_view("shape", 5), 825, 7, 1, 1, 0 },
    { string_view("p", 1), 826, 0, 1, 0, 0 },
    { string_view("nbl", 3), 827, 7, 0, 1, 0 },
    { string_view("pidx", 4), 828, 0, 0, 1, 0 },
    { string_view("_", 1), 829, 0, 0, 0, 0 },
    { string_view("start", 5), 830, 0, 0, 1, 0 },
    { string_view("_a", 2), 831, 0, 0, 0, 0 },
    { string_view("_b", 2), 832, 0, 0, 0, 0 },
    { string_view("_", 1), 833, 7, 0, 0, 0 },
    { string_view("_", 1), 834, 7, 0, 0, 0 },
    { string_view("besti", 5), 835, 0, 0, 0, 0 },
    { string_view("bestqc", 6), 836, 0, 0, 0, 0 },
    { string_view("t", 1), 837, 7, 0, 0, 0 },
    { string_view("i", 1), 838, 0, 0, 0, 0 },
    { string_view("qc", 2), 839, 0, 0, 0, 0 },
    { string_view("vi1", 3), 840, 0, 0, 0, 0 },
    { string_view("q", 1), 841, 7, 0, 0, 0 },
    { string_view("vi2", 3), 842, 0, 0, 0, 0 },
    { string_view("t1", 2), 843, 7, 0, 1, 0 },
    { string_view("found", 5), 844, 146, 0, 0, 0 },
    { string_view("vi", 2), 845, 0, 0, 0, 0 },
    { string_view("vii", 3), 846, 0, 0, 0, 0 },
    { string_view("ni", 2), 847, 0, 0, 1, 0 },
    { string_view("li", 2), 848, 0, 0, 1, 0 },
    { string_view("t", 1), 849, 7, 0, 0, 0 },
    { string_view("color_clear", 11), 850, 1167, 0, 1, 1 },
    { string_view("color_clear", 11), 850, 1167, 0, 1, 1 },
    { string_view("color_clear", 11), 850, 1167, 0, 1, 1 },
    { string_view("color_clear", 11), 850, 1167, 0, 1, 1 },
    { string_view("color_black", 11), 851, 1167, 0, 1, 1 },
    { string_view("color_black", 11), 851, 1167, 0, 1, 1 },
    { string_view("color_black", 11), 851, 1167, 0, 1, 1 },
    { string_view("color_black", 11), 851, 1167, 0, 1, 1 },
    { string_view("color_white", 11), 852, 1167, 0, 1, 1 },
    { string_view("color_white", 11), 852, 1167, 0, 1, 1 },
    { string_view("color_white", 11), 852, 1167, 0, 1, 1 },
    { string_view("color_white", 11), 852, 1167, 0, 1, 1 },
    { string_view("color_red", 9), 853, 1167, 0, 1, 1 },
    { string_view("color_red", 9), 853, 1167, 0, 1, 1 },
    { string_view("color_red", 9), 853, 1167, 0, 1, 1 },
    { string_view("color_red", 9), 853, 1167, 0, 1, 1 },
    { string_view("color_green", 11), 854, 1167, 0, 1, 1 },
    { string_view("color_green", 11), 854, 1167, 0, 1, 1 },
    { string_view("color_green", 11), 854, 1167, 0, 1, 1 },
    { string_view("color_green", 11), 854, 1167, 0, 1, 1 },
    { string_view("color_blue", 10), 855, 1167, 0, 1, 1 },
    { string_view("color_blue", 10), 855, 1167, 0, 1, 1 },
    { string_view("color_blue", 10), 855, 1167, 0, 1, 1 },
    { string_view("color_blue", 10), 855, 1167, 0, 1, 1 },
    { string_view("color_yellow", 12), 856, 1167, 0, 1, 1 },
    { string_view("color_yellow", 12), 856, 1167, 0, 1, 1 },
    { string_view("color_yellow", 12), 856, 1167, 0, 1, 1 },
    { string_view("color_yellow", 12), 856, 1167, 0, 1, 1 },
    { string_view("color_cyan", 10), 857, 1167, 0, 1, 1 },
    { string_view("color_cyan", 10), 857, 1167, 0, 1, 1 },
    { string_view("color_cyan", 10), 857, 1167, 0, 1, 1 },
    { string_view("color_cyan", 10), 857, 1167, 0, 1, 1 },
    { string_view("color_pink", 10), 858, 1167, 0, 1, 1 },
    { string_view("color_pink", 10), 858, 1167, 0, 1, 1 },
    { string_view("color_pink", 10), 858, 1167, 0, 1, 1 },
    { string_view("color_pink", 10), 858, 1167, 0, 1, 1 },
    { string_view("color_grey", 10), 859, 1167, 0, 1, 1 },
    { string_view("color_grey", 10), 859, 1167, 0, 1, 1 },
    { string_view("color_grey", 10), 859, 1167, 0, 1, 1 },
    { string_view("color_grey", 10), 859, 1167, 0, 1, 1 },
    { string_view("color_dark_grey", 15), 860, 1167, 0, 1, 1 },
    { string_view("color_dark_grey", 15), 860, 1167, 0, 1, 1 },
    { string_view("color_dark_grey", 15), 860, 1167, 0, 1, 1 },
    { string_view("color_dark_grey", 15), 860, 1167, 0, 1, 1 },
    { string_view("color_light_grey", 16), 861, 1167, 0, 1, 1 },
    { string_view("color_light_grey", 16), 861, 1167, 0, 1, 1 },
    { string_view("color_light_grey", 16), 861, 1167, 0, 1, 1 },
    { string_view("color_light_grey", 16), 861, 1167, 0, 1, 1 },
    { string_view("color_lightest_grey", 19), 862, 1167, 0, 1, 1 },
    { string_view("color_lightest_grey", 19), 862, 1167, 0, 1, 1 },
    { string_view("color_lightest_grey", 19), 862, 1167, 0, 1, 1 },
    { string_view("color_lightest_grey", 19), 862, 1167, 0, 1, 1 },
    { string_view("color_darkest_grey", 18), 863, 1167, 0, 1, 1 },
    { string_view("color_darkest_grey", 18), 863, 1167, 0, 1, 1 },
    { string_view("color_darkest_grey", 18), 863, 1167, 0, 1, 1 },
    { string_view("color_darkest_grey", 18), 863, 1167, 0, 1, 1 },
    { string_view("color_light_red", 15), 864, 1167, 0, 1, 1 },
    { string_view("color_light_red", 15), 864, 1167, 0, 1, 1 },
    { string_view("color_light_red", 15), 864, 1167, 0, 1, 1 },
    { string_view("color_light_red", 15), 864, 1167, 0, 1, 1 },
    { string_view("color_light_green", 17), 865, 1167, 0, 1, 1 },
    { string_view("color_light_green", 17), 865, 1167, 0, 1, 1 },
    { string_view("color_light_green", 17), 865, 1167, 0, 1, 1 },
    { string_view("color_light_green", 17), 865, 1167, 0, 1, 1 },
    { string_view("color_light_blue", 16), 866, 1167, 0, 1, 1 },
    { string_view("color_light_blue", 16), 866, 1167, 0, 1, 1 },
    { string_view("color_light_blue", 16), 866, 1167, 0, 1, 1 },
    { string_view("color_light_blue", 16), 866, 1167, 0, 1, 1 },
    { string_view("color_light_yellow", 18), 867, 1167, 0, 1, 1 },
    { string_view("color_light_yellow", 18), 867, 1167, 0, 1, 1 },
    { string_view("color_light_yellow", 18), 867, 1167, 0, 1, 1 },
    { string_view("color_light_yellow", 18), 867, 1167, 0, 1, 1 },
    { string_view("color_light_cyan", 16), 868, 1167, 0, 1, 1 },
    { string_view("color_light_cyan", 16), 868, 1167, 0, 1, 1 },
    { string_view("color_light_cyan", 16), 868, 1167, 0, 1, 1 },
    { string_view("color_light_cyan", 16), 868, 1167, 0, 1, 1 },
    { string_view("color_light_pink", 16), 869, 1167, 0, 1, 1 },
    { string_view("color_light_pink", 16), 869, 1167, 0, 1, 1 },
    { string_view("color_light_pink", 16), 869, 1167, 0, 1, 1 },
    { string_view("color_light_pink", 16), 869, 1167, 0, 1, 1 },
    { string_view("color_dark_red", 14), 870, 1167, 0, 1, 1 },
    { string_view("color_dark_red", 14), 870, 1167, 0, 1, 1 },
    { string_view("color_dark_red", 14), 870, 1167, 0, 1, 1 },
    { string_view("color_dark_red", 14), 870, 1167, 0, 1, 1 },
    { string_view("color_dark_green", 16), 871, 1167, 0, 1, 1 },
    { string_view("color_dark_green", 16), 871, 1167, 0, 1, 1 },
    { string_view("color_dark_green", 16), 871, 1167, 0, 1, 1 },
    { string_view("color_dark_green", 16), 871, 1167, 0, 1, 1 },
    { string_view("color_dark_blue", 15), 872, 1167, 0, 1, 1 },
    { string_view("color_dark_blue", 15), 872, 1167, 0, 1, 1 },
    { string_view("color_dark_blue", 15), 872, 1167, 0, 1, 1 },
    { string_view("color_dark_blue", 15), 872, 1167, 0, 1, 1 },
    { string_view("color_olive", 11), 873, 1167, 0, 1, 1 },
    { string_view("color_olive", 11), 873, 1167, 0, 1, 1 },
    { string_view("color_olive", 11), 873, 1167, 0, 1, 1 },
    { string_view("color_olive", 11), 873, 1167, 0, 1, 1 },
    { string_view("color_teal", 10), 874, 1167, 0, 1, 1 },
    { string_view("color_teal", 10), 874, 1167, 0, 1, 1 },
    { string_view("color_teal", 10), 874, 1167, 0, 1, 1 },
    { string_view("color_teal", 10), 874, 1167, 0, 1, 1 },
    { string_view("color_purple", 12), 875, 1167, 0, 1, 1 },
    { string_view("color_purple", 12), 875, 1167, 0, 1, 1 },
    { string_view("color_purple", 12), 875, 1167, 0, 1, 1 },
    { string_view("color_purple", 12), 875, 1167, 0, 1, 1 },
    { string_view("color_orange", 12), 876, 1167, 0, 1, 1 },
    { string_view("color_orange", 12), 876, 1167, 0, 1, 1 },
    { string_view("color_orange", 12), 876, 1167, 0, 1, 1 },
    { string_view("color_orange", 12), 876, 1167, 0, 1, 1 },
    { string_view("color_chartreuse", 16), 877, 1167, 0, 1, 1 },
    { string_view("color_chartreuse", 16), 877, 1167, 0, 1, 1 },
    { string_view("color_chartreuse", 16), 877, 1167, 0, 1, 1 },
    { string_view("color_chartreuse", 16), 877, 1167, 0, 1, 1 },
    { string_view("color_springgreen", 17), 878, 1167, 0, 1, 1 },
    { string_view("color_springgreen", 17), 878, 1167, 0, 1, 1 },
    { string_view("color_springgreen", 17), 878, 1167, 0, 1, 1 },
    { string_view("color_springgreen", 17), 878, 1167, 0, 1, 1 },
    { string_view("color_dodgerblue", 16), 879, 1167, 0, 1, 1 },
    { string_view("color_dodgerblue", 16), 879, 1167, 0, 1, 1 },
    { string_view("color_dodgerblue", 16), 879, 1167, 0, 1, 1 },
    { string_view("color_dodgerblue", 16), 879, 1167, 0, 1, 1 },
    { string_view("color_pink_violet", 17), 880, 1167, 0, 1, 1 },
    { string_view("color_pink_violet", 17), 880, 1167, 0, 1, 1 },
    { string_view("color_pink_violet", 17), 880, 1167, 0, 1, 1 },
    { string_view("color_pink_violet", 17), 880, 1167, 0, 1, 1 },
    { string_view("color_blue_violet", 17), 881, 1167, 0, 1, 1 },
    { string_view("color_blue_violet", 17), 881, 1167, 0, 1, 1 },
    { string_view("color_blue_violet", 17), 881, 1167, 0, 1, 1 },
    { string_view("color_blue_violet", 17), 881, 1167, 0, 1, 1 },
    { string_view("color_luminance_weights", 23), 884, 1167, 0, 1, 1 },
    { string_view("color_luminance_weights", 23), 884, 1167, 0, 1, 1 },
    { string_view("color_luminance_weights", 23), 884, 1167, 0, 1, 1 },
    { string_view("color_luminance_weights", 23), 884, 1167, 0, 1, 1 },
    { string_view("sphere", 6), 886, 1185, 0, 0, 0 },
    { string_view("r", 1), 887, 1226, 0, 0, 0 },
    { string_view("r", 1), 887, 1226, 0, 0, 0 },
    { string_view("r", 1), 887, 1226, 0, 0, 0 },
    { string_view("r", 1), 887, 1226, 0, 0, 0 },
    { string_view("r", 1), 887, 1226, 0, 0, 0 },
    { string_view("r", 1), 887, 1226, 0, 0, 0 },
    { string_view("op", 2), 888, 174, 0, 1, 0 },
    { string_view("op", 2), 888, 174, 0, 1, 0 },
    { string_view("op", 2), 888, 174, 0, 1, 0 },
    { string_view("eps", 3), 889, 2, 0, 1, 0 },
    { string_view("b", 1), 890, 2, 0, 1, 0 },
    { string_view("det", 3), 891, 2, 0, 0, 0 },
    { string_view("t", 1), 892, 2, 0, 0, 0 },
    { string_view("bigrad", 6), 893, 2, 0, 1, 0 },
    { string_view("lrad", 4), 894, 2, 0, 1, 0 },
    { string_view("spheres", 7), 895, 1250, 1, 1, 0 },
    { string_view("r", 1), 896, 1226, 0, 0, 0 },
    { string_view("r", 1), 896, 1226, 0, 0, 0 },
    { string_view("r", 1), 896, 1226, 0, 0, 0 },
    { string_view("r", 1), 896, 1226, 0, 0, 0 },
    { string_view("r", 1), 896, 1226, 0, 0, 0 },
    { string_view("r", 1), 896, 1226, 0, 0, 0 },
    { string_view("depth", 5), 897, 0, 0, 0, 0 },
    { string_view("t", 1), 898, 2, 0, 0, 0 },
    { string_view("id", 2), 899, 0, 0, 0, 0 },
    { string_view("s", 1), 900, 1185, 0, 0, 0 },
    { string_view("i", 1), 901, 0, 0, 0, 0 },
    { string_view("d", 1), 902, 2, 0, 1, 0 },
    { string_view("obj", 3), 903, 1185, 0, 1, 0 },
    { string_view("x", 1), 904, 174, 0, 1, 0 },
    { string_view("x", 1), 904, 174, 0, 1, 0 },
    { string_view("x", 1), 904, 174, 0, 1, 0 },
    { string_view("n", 1), 905, 174, 0, 1, 0 },
    { string_view("n", 1), 905, 174, 0, 1, 0 },
    { string_view("n", 1), 905, 174, 0, 1, 0 },
    { string_view("nl", 2), 906, 174, 0, 1, 0 },
    { string_view("nl", 2), 906, 174, 0, 1, 0 },
    { string_view("nl", 2), 906, 174, 0, 1, 0 },
    { string_view("f", 1), 907, 174, 0, 0, 0 },
    { string_view("f", 1), 907, 174, 0, 0, 0 },
    { string_view("f", 1), 907, 174, 0, 0, 0 },
    { string_view("p", 1), 908, 2, 0, 1, 0 },
    { string_view("r1", 2), 909, 2, 0, 1, 0 },
    { string_view("r2", 2), 910, 2, 0, 1, 0 },
    { string_view("r2s", 3), 911, 2, 0, 1, 0 },
    { string_view("w", 1), 912, 174, 0, 1, 0 },
    { string_view("w", 1), 912, 174, 0, 1, 0 },
    { string_view("w", 1), 912, 174, 0, 1, 0 },
    { string_view("u", 1), 913, 174, 0, 1, 0 },
    { string_view("u", 1), 913, 174, 0, 1, 0 },
    { string_view("u", 1), 913, 174, 0, 1, 0 },
    { string_view("v", 1), 914, 174, 0, 1, 0 },
    { string_view("v", 1), 914, 174, 0, 1, 0 },
    { string_view("v", 1), 914, 174, 0, 1, 0 },
    { string_view("d", 1), 915, 174, 0, 1, 0 },
    { string_view("d", 1), 915, 174, 0, 1, 0 },
    { string_view("d", 1), 915, 174, 0, 1, 0 },
    { string_view("reflRay", 7), 916, 1226, 0, 1, 0 },
    { string_view("reflRay", 7), 916, 1226, 0, 1, 0 },
    { string_view("reflRay", 7), 916, 1226, 0, 1, 0 },
    { string_view("reflRay", 7), 916, 1226, 0, 1, 0 },
    { string_view("reflRay", 7), 916, 1226, 0, 1, 0 },
    { string_view("reflRay", 7), 916, 1226, 0, 1, 0 },
    { string_view("into", 4), 917, 146, 0, 1, 0 },
    { string_view("nc", 2), 918, 2, 0, 1, 0 },
    { string_view("nt", 2), 919, 2, 0, 1, 0 },
    { string_view("nnt", 3), 920, 2, 0, 1, 0 },
    { string_view("ddn", 3), 921, 2, 0, 1, 0 },
    { string_view("cos2t", 5), 922, 2, 0, 1, 0 },
    { string_view("tdir", 4), 923, 174, 0, 1, 0 },
    { string_view("tdir", 4), 923, 174, 0, 1, 0 },
    { string_view("tdir", 4), 923, 174, 0, 1, 0 },
    { string_view("a", 1), 924, 2, 0, 1, 0 },
    { string_view("b", 1), 925, 2, 0, 1, 0 },
    { string_view("R0", 2), 926, 2, 0, 1, 0 },
    { string_view("c", 1), 927, 2, 0, 1, 0 },
    { string_view("Re", 2), 928, 2, 0, 1, 0 },
    { string_view("Tr", 2), 929, 2, 0, 1, 0 },
    { string_view("P", 1), 930, 2, 0, 1, 0 },
    { string_view("RP", 2), 931, 2, 0, 1, 0 },
    { string_view("TP", 2), 932, 2, 0, 1, 0 },
    { string_view("temp", 4), 933, 174, 0, 1, 0 },
    { string_view("temp", 4), 933, 174, 0, 1, 0 },
    { string_view("temp", 4), 933, 174, 0, 1, 0 },
    { string_view("w", 1), 934, 0, 1, 1, 0 },
    { string_view("h", 1), 935, 0, 0, 1, 0 },
    { string_view("cam", 3), 936, 1226, 0, 1, 0 },
    { string_view("cam", 3), 936, 1226, 0, 1, 0 },
    { string_view("cam", 3), 936, 1226, 0, 1, 0 },
    { string_view("cam", 3), 936, 1226, 0, 1, 0 },
    { string_view("cam", 3), 936, 1226, 0, 1, 0 },
    { string_view("cam", 3), 936, 1226, 0, 1, 0 },
    { string_view("cx", 2), 937, 174, 0, 1, 0 },
    { string_view("cx", 2), 937, 174, 0, 1, 0 },
    { string_view("cx", 2), 937, 174, 0, 1, 0 },
    { string_view("cy", 2), 938, 174, 0, 1, 0 },
    { string_view("cy", 2), 938, 174, 0, 1, 0 },
    { string_view("cy", 2), 938, 174, 0, 1, 0 },
    { string_view("c", 1), 939, 1254, 0, 1, 0 },
    { string_view("samples", 7), 940, 0, 1, 0, 0 },
    { string_view("y", 1), 941, 0, 0, 0, 0 },
    { string_view("x", 1), 942, 0, 0, 0, 0 },
    { string_view("r1", 2), 943, 2, 0, 1, 0 },
    { string_view("dx", 2), 944, 2, 0, 1, 0 },
    { string_view("r2", 2), 945, 2, 0, 1, 0 },
    { string_view("dy", 2), 946, 2, 0, 1, 0 },
    { string_view("d", 1), 947, 174, 0, 0, 0 },
    { string_view("d", 1), 947, 174, 0, 0, 0 },
    { string_view("d", 1), 947, 174, 0, 0, 0 },
    { string_view("r", 1), 948, 174, 0, 1, 0 },
    { string_view("r", 1), 948, 174, 0, 1, 0 },
    { string_view("r", 1), 948, 174, 0, 1, 0 },
    { string_view("r", 1), 949, 1258, 0, 1, 0 },
    { string_view("row", 3), 950, 1252, 0, 0, 0 },
    { string_view("col", 3), 951, 174, 0, 0, 0 },
    { string_view("col", 3), 951, 174, 0, 0, 0 },
    { string_view("col", 3), 951, 174, 0, 0, 0 },
    { string_view("a", 1), 952, 0, 0, 1, 0 },
    { string_view("b", 1), 953, 9, 0, 1, 0 },
    { string_view("compres1", 8), 954, 3, 0, 1, 0 },
    { string_view("comperr1", 8), 955, 458, 0, 1, 0 },
    { string_view("compres2", 8), 956, 3, 0, 1, 0 },
    { string_view("comperr2", 8), 957, 458, 0, 1, 0 },
    { string_view("v", 1), 958, 11, 0, 1, 0 },
    { string_view("k", 1), 959, 3, 0, 0, 0 },
    { string_view("v", 1), 960, 7, 0, 1, 0 },
    { string_view("k", 1), 961, 0, 0, 0, 0 },
    { string_view("nested", 6), 962, 1260, 0, 1, 0 },
    { string_view("sub", 3), 963, 1260, 0, 1, 0 },
    { string_view("this", 4), 964, 1291, 0, 0, 0 },
    { string_view("this", 4), 964, 1291, 0, 0, 0 },
    { string_view("this", 4), 964, 1291, 0, 0, 0 },
    { string_view("this", 4), 964, 1291, 0, 0, 0 },
    { string_view("o", 1), 965, 1291, 0, 0, 0 },
    { string_view("o", 1), 965, 1291, 0, 0, 0 },
    { string_view("o", 1), 965, 1291, 0, 0, 0 },
    { string_view("o", 1), 965, 1291, 0, 0, 0 },
    { string_view("this", 4), 966, 1291, 0, 0, 0 },
    { string_view("this", 4), 966, 1291, 0, 0, 0 },
    { string_view("this", 4), 966, 1291, 0, 0, 0 },
    { string_view("this", 4), 966, 1291, 0, 0, 0 },
    { string_view("f", 1), 967, 2, 0, 0, 0 },
    { string_view("this", 4), 968, 1291, 0, 0, 0 },
    { string_view("this", 4), 968, 1291, 0, 0, 0 },
    { string_view("this", 4), 968, 1291, 0, 0, 0 },
    { string_view("this", 4), 968, 1291, 0, 0, 0 },
    { string_view("quat_unit", 9), 992, 1291, 0, 1, 1 },
    { string_view("quat_unit", 9), 992, 1291, 0, 1, 1 },
    { string_view("quat_unit", 9), 992, 1291, 0, 1, 1 },
    { string_view("quat_unit", 9), 992, 1291, 0, 1, 1 },
    { string_view("this", 4), 993, 1309, 0, 0, 0 },
    { string_view("o", 1), 994, 1309, 0, 0, 0 },
    { string_view("this", 4), 995, 1309, 0, 0, 0 },
    { string_view("o", 1), 996, 1309, 0, 0, 0 },
    { string_view("this", 4), 997, 1309, 0, 0, 0 },
    { string_view("o", 1), 998, 1309, 0, 0, 0 },
    { string_view("this", 4), 999, 1309, 0, 0, 0 },
    { string_view("o", 1), 1000, 1309, 0, 0, 0 },
    { string_view("this", 4), 1013, 1309, 0, 0, 0 },
    { string_view("o", 1), 1014, 1309, 0, 0, 0 },
    { string_view("this", 4), 1033, 1309, 0, 0, 0 },
    { string_view("o", 1), 1034, 1309, 0, 0, 0 },
    { string_view("this", 4), 1035, 1309, 0, 0, 0 },
    { string_view("o", 1), 1036, 1309, 0, 0, 0 },
    { string_view("this", 4), 1046, 1309, 0, 0, 0 },
    { string_view("this", 4), 1047, 1309, 0, 0, 0 },
    { string_view("this", 4), 1048, 1309, 0, 0, 0 },
    { string_view("this", 4), 1049, 1309, 0, 0, 0 },
    { string_view("o", 1), 1050, 1309, 0, 0, 0 },
    { string_view("this", 4), 1051, 1309, 0, 0, 0 },
    { string_view("i", 1), 1052, 0, 0, 0, 0 },
    { string_view("this", 4), 1053, 1318, 0, 0, 0 },
    { string_view("o", 1), 1054, 1318, 0, 0, 0 },
    { string_view("this", 4), 1055, 1318, 0, 0, 0 },
    { string_view("o", 1), 1056, 1318, 0, 0, 0 },
    { string_view("this", 4), 1057, 1318, 0, 0, 0 },
    { string_view("o", 1), 1058, 1318, 0, 0, 0 },
    { string_view("this", 4), 1059, 1318, 0, 0, 0 },
    { string_view("o", 1), 1060, 1318, 0, 0, 0 },
    { string_view("this", 4), 1073, 1318, 0, 0, 0 },
    { string_view("o", 1), 1074, 1318, 0, 0, 0 },
    { string_view("c1", 2), 1089, 1309, 0, 1, 0 },
    { string_view("c2", 2), 1090, 1309, 0, 1, 0 },
    { string_view("c3", 2), 1091, 1309, 0, 0, 0 },
    { string_view("c4", 2), 1092, 1309, 0, 0, 0 },
    { string_view("s1", 2), 1093, 1318, 0, 1, 0 },
    { string_view("s2", 2), 1094, 1318, 0, 1, 0 },
    { string_view("s3", 2), 1095, 1318, 0, 1, 0 },
    { string_view("s4", 2), 1096, 1318, 0, 1, 0 },
    { string_view("this", 4), 1097, 242, 0, 0, 0 },
    { string_view("this", 4), 1097, 242, 0, 0, 0 },
    { string_view("this", 4), 1097, 242, 0, 0, 0 },
    { string_view("this", 4), 1097, 242, 0, 0, 0 },
    { string_view("o", 1), 1098, 242, 0, 0, 0 },
    { string_view("o", 1), 1098, 242, 0, 0, 0 },
    { string_view("o", 1), 1098, 242, 0, 0, 0 },
    { string_view("o", 1), 1098, 242, 0, 0, 0 },
    { string_view("this", 4), 1099, 242, 0, 0, 0 },
    { string_view("this", 4), 1099, 242, 0, 0, 0 },
    { string_view("this", 4), 1099, 242, 0, 0, 0 },
    { string_view("this", 4), 1099, 242, 0, 0, 0 },
    { string_view("o", 1), 1100, 162, 0, 0, 0 },
    { string_view("o", 1), 1100, 162, 0, 0, 0 },
    { string_view("this", 4), 1101, 242, 0, 0, 0 },
    { string_view("this", 4), 1101, 242, 0, 0, 0 },
    { string_view("this", 4), 1101, 242, 0, 0, 0 },
    { string_view("this", 4), 1101, 242, 0, 0, 0 },
    { string_view("this", 4), 1102, 242, 0, 0, 0 },
    { string_view("this", 4), 1102, 242, 0, 0, 0 },
    { string_view("this", 4), 1102, 242, 0, 0, 0 },
    { string_view("this", 4), 1102, 242, 0, 0, 0 },
    { string_view("this", 4), 1103, 242, 0, 0, 0 },
    { string_view("this", 4), 1103, 242, 0, 0, 0 },
    { string_view("this", 4), 1103, 242, 0, 0, 0 },
    { string_view("this", 4), 1103, 242, 0, 0, 0 },
    { string_view("this", 4), 1104, 242, 0, 0, 0 },
    { string_view("this", 4), 1104, 242, 0, 0, 0 },
    { string_view("this", 4), 1104, 242, 0, 0, 0 },
    { string_view("this", 4), 1104, 242, 0, 0, 0 },
    { string_view("mat2x2_identity", 15), 1105, 242, 1, 1, 1 },
    { string_view("mat2x2_identity", 15), 1105, 242, 1, 1, 1 },
    { string_view("mat2x2_identity", 15), 1105, 242, 1, 1, 1 },
    { string_view("mat2x2_identity", 15), 1105, 242, 1, 1, 1 },
    { string_view("a", 1), 1106, 242, 0, 1, 0 },
    { string_view("a", 1), 1106, 242, 0, 1, 0 },
    { string_view("a", 1), 1106, 242, 0, 1, 0 },
    { string_view("a", 1), 1106, 242, 0, 1, 0 },
    { string_view("b", 1), 1107, 242, 0, 1, 0 },
    { string_view("b", 1), 1107, 242, 0, 1, 0 },
    { string_view("b", 1), 1107, 242, 0, 1, 0 },
    { string_view("b", 1), 1107, 242, 0, 1, 0 },
    { string_view("c", 1), 1108, 162, 0, 1, 0 },
    { string_view("c", 1), 1108, 162, 0, 1, 0 },
    { string_view("tests", 5), 1109, 1336, 0, 1, 0 },
    { string_view("results1", 8), 1110, 7, 0, 1, 0 },
    { string_view("t", 1), 1111, 1327, 0, 0, 0 },
    { string_view("results2", 8), 1112, 7, 0, 1, 0 },
    { string_view("t", 1), 1113, 1327, 0, 0, 0 },
    { string_view("results3", 8), 1114, 7, 0, 1, 0 },
    { string_view("t", 1), 1115, 1327, 0, 0, 0 },
    { string_view("results4", 8), 1116, 7, 0, 1, 0 },
    { string_view("t", 1), 1117, 1327, 0, 0, 0 },
    { string_view("btests", 6), 1118, 1338, 0, 1, 0 },
    { string_view("results5", 8), 1119, 11, 0, 1, 0 },
    { string_view("t", 1), 1120, 146, 0, 0, 0 },
    { string_view("btests2", 7), 1121, 1338, 0, 1, 0 },
    { string_view("results6", 8), 1122, 11, 0, 1, 0 },
    { string_view("t", 1), 1123, 146, 0, 0, 0 },
    { string_view("f", 1), 1124, 1340, 0, 0, 0 },
    { string_view("f", 1), 1125, 1342, 0, 0, 0 },
    { string_view("f", 1), 1126, 1344, 0, 0, 0 },
    { string_view("f", 1), 1127, 1346, 0, 0, 0 },
    { string_view("f", 1), 1128, 1348, 0, 0, 0 },
    { string_view("f", 1), 1129, 1350, 0, 0, 0 },
    { string_view("f", 1), 1130, 1352, 0, 0, 0 },
    { string_view("f", 1), 1131, 1354, 0, 0, 0 },
    { string_view("f", 1), 1132, 1356, 0, 0, 0 },
    { string_view("f", 1), 1133, 1358, 0, 0, 0 },
    { string_view("xs", 2), 1134, 7, 0, 0, 0 },
    { string_view("fun", 3), 1135, 1360, 0, 0, 0 },
    { string_view("r", 1), 1136, 7, 0, 1, 0 },
    { string_view("x", 1), 1137, 0, 0, 0, 0 },
    { string_view("i", 1), 1138, 0, 0, 0, 0 },
    { string_view("xs", 2), 1139, 7, 0, 0, 0 },
    { string_view("init", 4), 1140, 0, 0, 1, 0 },
    { string_view("f", 1), 1141, 1362, 0, 0, 0 },
    { string_view("body", 4), 1142, 1364, 0, 0, 0 },
    { string_view("catch", 5), 1143, 1366, 0, 0, 0 },
    { string_view("err", 3), 1144, 458, 0, 1, 0 },
    { string_view("stack_trace", 11), 1145, 3, 0, 1, 0 },
    { string_view("body", 4), 1146, 1364, 0, 0, 0 },
    { string_view("v", 1), 1147, 3, 0, 0, 0 },
    { string_view("n", 1), 1148, 0, 0, 0, 0 },
    { string_view("xs", 2), 1149, 7, 0, 0, 0 },
    { string_view("acc", 3), 1150, 0, 0, 0, 0 },
    { string_view("fun", 3), 1151, 1368, 0, 0, 0 },
    { string_view("_", 1), 1152, 0, 0, 1, 0 },
    { string_view("xs", 2), 1153, 0, 0, 0, 0 },
    { string_view("fun", 3), 1154, 1370, 0, 0, 0 },
    { string_view("r", 1), 1155, 486, 0, 1, 0 },
    { string_view("x", 1), 1156, 0, 0, 0, 0 },
    { string_view("i", 1), 1157, 0, 0, 0, 0 },
    { string_view("this", 4), 1158, 460, 0, 0, 0 },
    { string_view("key", 3), 1159, 2, 0, 0, 0 },
    { string_view("h", 1), 1160, 0, 0, 1, 0 },
    { string_view("c", 1), 1161, 484, 0, 0, 0 },
    { string_view("size", 4), 1162, 0, 0, 0, 0 },
    { string_view("xs", 2), 1163, 0, 0, 0, 0 },
    { string_view("fun", 3), 1164, 1370, 0, 0, 0 },
    { string_view("r", 1), 1165, 657, 0, 1, 0 },
    { string_view("x", 1), 1166, 0, 0, 0, 0 },
    { string_view("i", 1), 1167, 0, 0, 0, 0 },
    { string_view("this", 4), 1168, 631, 0, 0, 0 },
    { string_view("key", 3), 1169, 3, 0, 0, 0 },
    { string_view("value", 5), 1170, 458, 0, 0, 0 },
    { string_view("c", 1), 1171, 655, 0, 1, 0 },
    { string_view("h", 1), 1172, 0, 0, 1, 0 },
    { string_view("this", 4), 1173, 631, 0, 0, 0 },
    { string_view("key", 3), 1174, 3, 0, 0, 0 },
    { string_view("h", 1), 1175, 0, 0, 1, 0 },
    { string_view("c", 1), 1176, 655, 0, 0, 0 },
    { string_view("this", 4), 1177, 631, 0, 0, 0 },
    { string_view("key", 3), 1178, 3, 0, 0, 0 },
    { string_view("default_val", 11), 1179, 458, 0, 0, 0 },
    { string_view("c", 1), 1180, 655, 0, 1, 0 },
    { string_view("this", 4), 1181, 631, 0, 0, 0 },
    { string_view("key", 3), 1182, 3, 0, 0, 0 },
    { string_view("h", 1), 1183, 0, 0, 1, 0 },
    { string_view("c", 1), 1184, 655, 0, 0, 0 },
    { string_view("size", 4), 1185, 0, 0, 0, 0 },
    { string_view("xs", 2), 1186, 0, 0, 0, 0 },
    { string_view("fun", 3), 1187, 1372, 0, 0, 0 },
    { string_view("r", 1), 1188, 697, 0, 1, 0 },
    { string_view("x", 1), 1189, 0, 0, 0, 0 },
    { string_view("i", 1), 1190, 0, 0, 0, 0 },
    { string_view("this", 4), 1191, 659, 0, 0, 0 },
    { string_view("key", 3), 1192, 207, 0, 0, 0 },
    { string_view("key", 3), 1192, 207, 0, 0, 0 },
    { string_view("key", 3), 1192, 207, 0, 0, 0 },
    { string_view("value", 5), 1193, 207, 0, 0, 0 },
    { string_view("value", 5), 1193, 207, 0, 0, 0 },
    { string_view("value", 5), 1193, 207, 0, 0, 0 },
    { string_view("c", 1), 1194, 695, 0, 1, 0 },
    { string_view("h", 1), 1195, 0, 0, 1, 0 },
    { string_view("this", 4), 1196, 659, 0, 0, 0 },
    { string_view("key", 3), 1197, 207, 0, 0, 0 },
    { string_view("key", 3), 1197, 207, 0, 0, 0 },
    { string_view("key", 3), 1197, 207, 0, 0, 0 },
    { string_view("h", 1), 1198, 0, 0, 1, 0 },
    { string_view("c", 1), 1199, 695, 0, 0, 0 },
    { string_view("this", 4), 1200, 659, 0, 0, 0 },
    { string_view("key", 3), 1201, 207, 0, 0, 0 },
    { string_view("key", 3), 1201, 207, 0, 0, 0 },
    { string_view("key", 3), 1201, 207, 0, 0, 0 },
    { string_view("default_val", 11), 1202, 207, 0, 0, 0 },
    { string_view("default_val", 11), 1202, 207, 0, 0, 0 },
    { string_view("default_val", 11), 1202, 207, 0, 0, 0 },
    { string_view("c", 1), 1203, 695, 0, 1, 0 },
    { string_view("this", 4), 1204, 659, 0, 0, 0 },
    { string_view("key", 3), 1205, 207, 0, 0, 0 },
    { string_view("key", 3), 1205, 207, 0, 0, 0 },
    { string_view("key", 3), 1205, 207, 0, 0, 0 },
    { string_view("h", 1), 1206, 0, 0, 1, 0 },
    { string_view("c", 1), 1207, 695, 0, 0, 0 },
    { string_view("f", 1), 1208, 1374, 0, 0, 0 },
    { string_view("f", 1), 1209, 1376, 0, 0, 0 },
    { string_view("f", 1), 1210, 1378, 0, 0, 0 },
    { string_view("f", 1), 1211, 1380, 0, 0, 0 },
    { string_view("xs", 2), 1212, 0, 0, 0, 0 },
    { string_view("fun", 3), 1213, 1382, 0, 0, 0 },
    { string_view("r", 1), 1214, 7, 0, 1, 0 },
    { string_view("x", 1), 1215, 0, 0, 0, 0 },
    { string_view("i", 1), 1216, 0, 0, 0, 0 },
    { string_view("f", 1), 1217, 1384, 0, 0, 0 },
    { string_view("size", 4), 1218, 0, 0, 0, 0 },
    { string_view("xs", 2), 1219, 0, 0, 0, 0 },
    { string_view("fun", 3), 1220, 1386, 0, 0, 0 },
    { string_view("r", 1), 1221, 542, 0, 1, 0 },
    { string_view("x", 1), 1222, 0, 0, 0, 0 },
    { string_view("i", 1), 1223, 0, 0, 0, 0 },
    { string_view("this", 4), 1224, 488, 0, 0, 0 },
    { string_view("key", 3), 1225, 3, 0, 0, 0 },
    { string_view("h", 1), 1226, 0, 0, 1, 0 },
    { string_view("c", 1), 1227, 512, 0, 0, 0 },
    { string_view("f", 1), 1228, 1388, 0, 0, 0 },
    { string_view("f", 1), 1229, 1390, 1, 0, 0 },
    { string_view("i", 1), 1230, 1392, 0, 0, 0 },
    { string_view("f", 1), 1231, 1394, 0, 0, 0 },
    { string_view("x", 1), 1232, 3, 0, 0, 0 },
    { string_view("f", 1), 1233, 1396, 0, 0, 0 },
    { string_view("a", 1), 1234, 3, 0, 0, 0 },
    { string_view("f", 1), 1235, 1398, 0, 0, 0 },
    { string_view("f", 1), 1236, 1400, 0, 0, 0 },
    { string_view("this", 4), 1237, 762, 0, 0, 0 },
    { string_view("c", 1), 1238, 0, 0, 0, 0 },
    { string_view("this", 4), 1239, 771, 0, 0, 0 },
    { string_view("c", 1), 1240, 0, 0, 0, 0 },
    { string_view("this", 4), 1241, 771, 0, 0, 0 },
    { string_view("c", 1), 1242, 0, 0, 0, 0 },
    { string_view("this", 4), 1243, 762, 0, 0, 0 },
    { string_view("d", 1), 1244, 1402, 0, 0, 0 },
    { string_view("f", 1), 1245, 1411, 0, 0, 0 },
    { string_view("f", 1), 1246, 1413, 0, 0, 0 },
    { string_view("this", 4), 1247, 829, 0, 0, 0 },
    { string_view("t", 1), 1248, 3, 0, 0, 0 },
    { string_view("this", 4), 1249, 829, 0, 0, 0 },
    { string_view("u", 1), 1250, 3, 0, 0, 0 },
    { string_view("a", 1), 1251, 829, 0, 0, 0 },
    { string_view("t", 1), 1252, 3, 0, 0, 0 },
    { string_view("x", 1), 1253, 2, 0, 0, 0 },
    { string_view("y", 1), 1254, 2, 0, 0, 0 },
    { string_view("f", 1), 1255, 1415, 0, 0, 0 },
    { string_view("size", 4), 1256, 0, 0, 0, 0 },
    { string_view("xs", 2), 1257, 0, 0, 0, 0 },
    { string_view("fun", 3), 1258, 1417, 0, 0, 0 },
    { string_view("r", 1), 1259, 540, 0, 1, 0 },
    { string_view("x", 1), 1260, 0, 0, 0, 0 },
    { string_view("i", 1), 1261, 0, 0, 0, 0 },
    { string_view("this", 4), 1262, 514, 0, 0, 0 },
    { string_view("key", 3), 1263, 3, 0, 0, 0 },
    { string_view("value", 5), 1264, 0, 0, 0, 0 },
    { string_view("c", 1), 1265, 538, 0, 1, 0 },
    { string_view("h", 1), 1266, 0, 0, 1, 0 },
    { string_view("this", 4), 1267, 514, 0, 0, 0 },
    { string_view("key", 3), 1268, 3, 0, 0, 0 },
    { string_view("h", 1), 1269, 0, 0, 1, 0 },
    { string_view("c", 1), 1270, 538, 0, 0, 0 },
    { string_view("this", 4), 1271, 514, 0, 0, 0 },
    { string_view("key", 3), 1272, 3, 0, 0, 0 },
    { string_view("default_val", 11), 1273, 0, 0, 0, 0 },
    { string_view("c", 1), 1274, 538, 0, 1, 0 },
    { string_view("this", 4), 1275, 514, 0, 0, 0 },
    { string_view("key", 3), 1276, 3, 0, 0, 0 },
    { string_view("h", 1), 1277, 0, 0, 1, 0 },
    { string_view("c", 1), 1278, 538, 0, 0, 0 },
    { string_view("f", 1), 1279, 1419, 0, 0, 0 },
    { string_view("f", 1), 1280, 1421, 0, 0, 0 },
    { string_view("f", 1), 1281, 1423, 0, 0, 0 },
    { string_view("f", 1), 1282, 1425, 0, 0, 0 },
    { string_view("xs", 2), 1283, 0, 0, 0, 0 },
    { string_view("fun", 3), 1284, 1427, 0, 0, 0 },
    { string_view("r", 1), 1285, 1027, 0, 1, 0 },
    { string_view("x", 1), 1286, 0, 0, 0, 0 },
    { string_view("i", 1), 1287, 0, 0, 0, 0 },
    { string_view("xs", 2), 1288, 0, 0, 0, 0 },
    { string_view("fun", 3), 1289, 1429, 0, 0, 0 },
    { string_view("r", 1), 1290, 1005, 0, 1, 0 },
    { string_view("x", 1), 1291, 0, 0, 0, 0 },
    { string_view("i", 1), 1292, 0, 0, 0, 0 },
    { string_view("distancef", 9), 1293, 1431, 0, 0, 0 },
    { string_view("startnode", 9), 1294, 919, 0, 0, 0 },
    { string_view("endnode", 7), 1295, 919, 1, 0, 0 },
    { string_view("costf", 5), 1296, 1007, 1, 0, 0 },
    { string_view("distancef", 9), 1297, 1431, 1, 0, 0 },
    { string_view("neighbors", 9), 1298, 1011, 1, 0, 0 },
    { string_view("startnode", 9), 1303, 919, 0, 0, 0 },
    { string_view("endcondition", 12), 1304, 1433, 0, 0, 0 },
    { string_view("generatenewstates", 17), 1305, 1435, 0, 0, 0 },
    { string_view("heuristic", 9), 1306, 1437, 1, 0, 0 },
    { string_view("openlist", 8), 1307, 1005, 1, 1, 0 },
    { string_view("n", 1), 1308, 997, 1, 0, 0 },
    { string_view("c", 1), 1309, 919, 0, 0, 0 },
    { string_view("path", 4), 1310, 1005, 0, 1, 0 },
    { string_view("n", 1), 1311, 919, 0, 0, 0 },
    { string_view("n", 1), 1316, 919, 1, 0, 0 },
    { string_view("f", 1), 1317, 1025, 1, 0, 0 },
    { string_view("n", 1), 1320, 919, 0, 0, 0 },
    { string_view("f", 1), 1321, 1439, 0, 0, 0 },
    { string_view("delta", 5), 1322, 25, 0, 0, 0 },
    { string_view("delta", 5), 1322, 25, 0, 0, 0 },
    { string_view("np", 2), 1323, 25, 0, 1, 0 },
    { string_view("np", 2), 1323, 25, 0, 1, 0 },
    { string_view("nn", 2), 1324, 919, 0, 0, 0 },
    { string_view("cost", 4), 1325, 0, 0, 1, 0 },
    { string_view("delta", 5), 1326, 25, 0, 0, 0 },
    { string_view("delta", 5), 1326, 25, 0, 0, 0 },
    { string_view("cost", 4), 1327, 0, 0, 0, 0 },
    { string_view("nn", 2), 1328, 919, 0, 0, 0 },
    { string_view("G", 1), 1329, 2, 0, 1, 0 },
    { string_view("state", 5), 1330, 25, 0, 0, 0 },
    { string_view("state", 5), 1330, 25, 0, 0, 0 },
    { string_view("xs", 2), 1331, 1027, 0, 0, 0 },
    { string_view("fun", 3), 1332, 1441, 0, 0, 0 },
    { string_view("r", 1), 1333, 11, 0, 1, 0 },
    { string_view("x", 1), 1334, 1005, 0, 0, 0 },
    { string_view("i", 1), 1335, 0, 0, 0, 0 },
    { string_view("xs", 2), 1336, 1005, 0, 0, 0 },
    { string_view("acc", 3), 1337, 3, 0, 0, 0 },
    { string_view("fun", 3), 1338, 1443, 0, 0, 0 },
    { string_view("_", 1), 1339, 919, 0, 1, 0 },
    { string_view("startnode", 9), 1340, 49, 0, 0, 0 },
    { string_view("endcondition", 12), 1341, 1445, 0, 0, 0 },
    { string_view("generatenewstates", 17), 1342, 1447, 0, 0, 0 },
    { string_view("heuristic", 9), 1343, 1449, 1, 0, 0 },
    { string_view("openlist", 8), 1344, 148, 1, 1, 0 },
    { string_view("n", 1), 1345, 109, 1, 0, 0 },
    { string_view("c", 1), 1346, 49, 0, 0, 0 },
    { string_view("path", 4), 1347, 148, 0, 1, 0 },
    { string_view("delta", 5), 1348, 138, 0, 0, 0 },
    { string_view("cost", 4), 1349, 0, 0, 0, 0 },
    { string_view("nn", 2), 1350, 49, 0, 0, 0 },
    { string_view("G", 1), 1351, 2, 0, 1, 0 },
    { string_view("xs", 2), 1352, 148, 0, 0, 0 },
    { string_view("fun", 3), 1353, 1451, 0, 0, 0 },
    { string_view("r", 1), 1354, 11, 0, 1, 0 },
    { string_view("x", 1), 1355, 49, 0, 0, 0 },
    { string_view("i", 1), 1356, 0, 0, 0, 0 },
    { string_view("xs", 2), 1357, 0, 0, 0, 0 },
    { string_view("fun", 3), 1358, 1453, 0, 0, 0 },
    { string_view("r", 1), 1359, 13, 0, 1, 0 },
    { string_view("x", 1), 1360, 0, 0, 0, 0 },
    { string_view("i", 1), 1361, 0, 0, 0, 0 },
    { string_view("xs", 2), 1362, 0, 0, 0, 0 },
    { string_view("fun", 3), 1363, 1455, 0, 0, 0 },
    { string_view("r", 1), 1364, 7, 0, 1, 0 },
    { string_view("x", 1), 1365, 0, 0, 0, 0 },
    { string_view("i", 1), 1366, 0, 0, 0, 0 },
    { string_view("xs", 2), 1367, 37, 0, 0, 0 },
    { string_view("fun", 3), 1368, 1457, 0, 0, 0 },
    { string_view("r", 1), 1369, 37, 0, 1, 0 },
    { string_view("x", 1), 1370, 25, 0, 0, 0 },
    { string_view("x", 1), 1370, 25, 0, 0, 0 },
    { string_view("i", 1), 1371, 0, 0, 0, 0 },
    { string_view("xs", 2), 1372, 37, 0, 0, 0 },
    { string_view("fun", 3), 1373, 1459, 0, 0, 0 },
    { string_view("r", 1), 1374, 7, 0, 1, 0 },
    { string_view("x", 1), 1375, 25, 0, 0, 0 },
    { string_view("x", 1), 1375, 25, 0, 0, 0 },
    { string_view("i", 1), 1376, 0, 0, 0, 0 },
    { string_view("xs", 2), 1377, 37, 0, 0, 0 },
    { string_view("fun", 3), 1378, 1461, 0, 0, 0 },
    { string_view("r", 1), 1379, 37, 0, 1, 0 },
    { string_view("x", 1), 1380, 25, 0, 0, 0 },
    { string_view("x", 1), 1380, 25, 0, 0, 0 },
    { string_view("i", 1), 1381, 0, 0, 0, 0 },
    { string_view("xs", 2), 1382, 11, 0, 0, 0 },
    { string_view("fun", 3), 1383, 1463, 0, 0, 0 },
    { string_view("x", 1), 1384, 3, 0, 0, 0 },
    { string_view("i", 1), 1385, 0, 0, 0, 0 },
    { string_view("tok", 3), 1386, 3, 0, 0, 0 },
    { string_view("a", 1), 1387, 3, 0, 1, 0 },
    { string_view("xs", 2), 1388, 1092, 0, 0, 0 },
    { string_view("acc", 3), 1389, 3, 0, 0, 0 },
    { string_view("fun", 3), 1390, 1465, 0, 0, 0 },
    { string_view("_", 1), 1391, 1074, 0, 1, 0 },
    { string_view("xs", 2), 1392, 1092, 0, 0, 0 },
    { string_view("acc", 3), 1393, 0, 0, 0, 0 },
    { string_view("fun", 3), 1394, 1467, 0, 0, 0 },
    { string_view("_", 1), 1395, 1074, 0, 1, 0 },
    { string_view("xs", 2), 1396, 1092, 0, 0, 0 },
    { string_view("acc", 3), 1397, 0, 0, 0, 0 },
    { string_view("fun", 3), 1398, 1469, 0, 0, 0 },
    { string_view("_", 1), 1399, 1074, 0, 1, 0 },
    { string_view("xs", 2), 1400, 1092, 0, 0, 0 },
    { string_view("fun", 3), 1401, 1471, 0, 0, 0 },
    { string_view("r", 1), 1402, 11, 0, 1, 0 },
    { string_view("x", 1), 1403, 1074, 0, 0, 0 },
    { string_view("i", 1), 1404, 0, 0, 0, 0 },
    { string_view("v", 1), 1405, 25, 1, 0, 0 },
    { string_view("v", 1), 1405, 25, 1, 0, 0 },
    { string_view("fun", 3), 1406, 1473, 1, 0, 0 },
    { string_view("xs", 2), 1408, 0, 0, 0, 0 },
    { string_view("fun", 3), 1409, 1475, 0, 0, 0 },
    { string_view("r", 1), 1410, 1121, 0, 1, 0 },
    { string_view("x", 1), 1411, 0, 0, 0, 0 },
    { string_view("i", 1), 1412, 0, 0, 0, 0 },
    { string_view("y", 1), 1413, 0, 1, 0, 0 },
    { string_view("xs", 2), 1415, 0, 0, 0, 0 },
    { string_view("fun", 3), 1416, 1477, 0, 0, 0 },
    { string_view("r", 1), 1417, 1119, 0, 1, 0 },
    { string_view("x", 1), 1418, 0, 0, 0, 0 },
    { string_view("i", 1), 1419, 0, 0, 0, 0 },
    { string_view("x", 1), 1420, 0, 0, 0, 0 },
    { string_view("xs", 2), 1421, 0, 0, 0, 0 },
    { string_view("fun", 3), 1422, 1479, 0, 0, 0 },
    { string_view("r", 1), 1423, 1140, 0, 1, 0 },
    { string_view("x", 1), 1424, 0, 0, 0, 0 },
    { string_view("i", 1), 1425, 0, 0, 0, 0 },
    { string_view("xs", 2), 1426, 0, 0, 0, 0 },
    { string_view("fun", 3), 1427, 1481, 0, 0, 0 },
    { string_view("r", 1), 1428, 1138, 0, 1, 0 },
    { string_view("x", 1), 1429, 0, 0, 0, 0 },
    { string_view("i", 1), 1430, 0, 0, 0, 0 },
    { string_view("f", 1), 1431, 1483, 0, 0, 0 },
    { string_view("y", 1), 1432, 0, 0, 0, 0 },
    { string_view("x", 1), 1433, 0, 0, 0, 0 },
    { string_view("xs", 2), 1434, 0, 0, 0, 0 },
    { string_view("fun", 3), 1435, 1485, 0, 0, 0 },
    { string_view("r", 1), 1436, 9, 0, 1, 0 },
    { string_view("x", 1), 1437, 0, 0, 0, 0 },
    { string_view("i", 1), 1438, 0, 0, 0, 0 },
    { string_view("xs", 2), 1439, 0, 0, 0, 0 },
    { string_view("fun", 3), 1440, 1487, 0, 0, 0 },
    { string_view("r", 1), 1441, 9, 0, 1, 0 },
    { string_view("x", 1), 1442, 0, 0, 0, 0 },
    { string_view("i", 1), 1443, 0, 0, 0, 0 },
    { string_view("xs", 2), 1444, 0, 0, 0, 0 },
    { string_view("fun", 3), 1445, 1489, 0, 0, 0 },
    { string_view("r", 1), 1446, 9, 0, 1, 0 },
    { string_view("x", 1), 1447, 0, 0, 0, 0 },
    { string_view("i", 1), 1448, 0, 0, 0, 0 },
    { string_view("xs", 2), 1449, 1148, 0, 0, 0 },
    { string_view("fun", 3), 1450, 1491, 0, 0, 0 },
    { string_view("r", 1), 1451, 1148, 0, 1, 0 },
    { string_view("x", 1), 1452, 162, 0, 0, 0 },
    { string_view("x", 1), 1452, 162, 0, 0, 0 },
    { string_view("i", 1), 1453, 0, 0, 0, 0 },
    { string_view("xs", 2), 1454, 1165, 0, 0, 0 },
    { string_view("lt", 2), 1455, 1493, 1, 0, 0 },
    { string_view("pivot", 5), 1456, 1150, 1, 1, 0 },
    { string_view("pivot", 5), 1456, 1150, 1, 1, 0 },
    { string_view("pivot", 5), 1456, 1150, 1, 1, 0 },
    { string_view("tail", 4), 1457, 1165, 0, 1, 0 },
    { string_view("f1", 2), 1458, 1165, 0, 1, 0 },
    { string_view("f2", 2), 1459, 1165, 0, 1, 0 },
    { string_view("xs", 2), 1461, 1165, 0, 0, 0 },
    { string_view("fun", 3), 1462, 1495, 0, 0, 0 },
    { string_view("t", 1), 1463, 1165, 0, 1, 0 },
    { string_view("f", 1), 1464, 1165, 0, 1, 0 },
    { string_view("x", 1), 1465, 1150, 0, 0, 0 },
    { string_view("x", 1), 1465, 1150, 0, 0, 0 },
    { string_view("x", 1), 1465, 1150, 0, 0, 0 },
    { string_view("i", 1), 1466, 0, 0, 0, 0 },
    { string_view("_", 1), 1467, 1150, 0, 0, 0 },
    { string_view("_", 1), 1467, 1150, 0, 0, 0 },
    { string_view("_", 1), 1467, 1150, 0, 0, 0 },
    { string_view("xs", 2), 1468, 1148, 0, 0, 0 },
    { string_view("fun", 3), 1469, 1497, 0, 0, 0 },
    { string_view("r", 1), 1470, 13, 0, 1, 0 },
    { string_view("x", 1), 1471, 162, 0, 0, 0 },
    { string_view("x", 1), 1471, 162, 0, 0, 0 },
    { string_view("i", 1), 1472, 0, 0, 0, 0 },
    { string_view("xs", 2), 1473, 13, 0, 0, 0 },
    { string_view("fun", 3), 1474, 1499, 0, 0, 0 },
    { string_view("r", 1), 1475, 13, 0, 1, 0 },
    { string_view("x", 1), 1476, 7, 0, 0, 0 },
    { string_view("i", 1), 1477, 0, 0, 0, 0 },
    { string_view("xs", 2), 1478, 7, 0, 0, 0 },
    { string_view("lt", 2), 1479, 1501, 1, 0, 0 },
    { string_view("pivot", 5), 1480, 0, 1, 1, 0 },
    { string_view("tail", 4), 1481, 7, 0, 1, 0 },
    { string_view("f1", 2), 1482, 7, 0, 1, 0 },
    { string_view("f2", 2), 1483, 7, 0, 1, 0 },
    { string_view("xs", 2), 1484, 7, 0, 0, 0 },
    { string_view("fun", 3), 1485, 1495, 0, 0, 0 },
    { string_view("t", 1), 1486, 7, 0, 1, 0 },
    { string_view("f", 1), 1487, 7, 0, 1, 0 },
    { string_view("x", 1), 1488, 0, 0, 0, 0 },
    { string_view("i", 1), 1489, 0, 0, 0, 0 },
    { string_view("_", 1), 1490, 0, 0, 0, 0 },
    { string_view("xs", 2), 1491, 7, 0, 0, 0 },
    { string_view("fun", 3), 1492, 1503, 0, 0, 0 },
    { string_view("x", 1), 1493, 0, 0, 0, 0 },
    { string_view("i", 1), 1494, 0, 0, 0, 0 },
    { string_view("xs", 2), 1495, 7, 0, 0, 0 },
    { string_view("fun", 3), 1496, 1505, 0, 0, 0 },
    { string_view("acc", 3), 1497, 0, 0, 0, 0 },
    { string_view("i", 1), 1498, 0, 0, 0, 0 },
    { string_view("xs", 2), 1499, 13, 0, 0, 0 },
    { string_view("fun", 3), 1500, 1507, 0, 0, 0 },
    { string_view("x", 1), 1501, 7, 0, 0, 0 },
    { string_view("i", 1), 1502, 0, 0, 0, 0 },
    { string_view("xs", 2), 1503, 13, 0, 0, 0 },
    { string_view("fun", 3), 1504, 1509, 0, 0, 0 },
    { string_view("x", 1), 1505, 7, 0, 0, 0 },
    { string_view("i", 1), 1506, 0, 0, 0, 0 },
    { string_view("xs", 2), 1507, 0, 0, 0, 0 },
    { string_view("fun", 3), 1508, 1511, 0, 0, 0 },
    { string_view("r", 1), 1509, 1254, 0, 1, 0 },
    { string_view("x", 1), 1510, 0, 0, 0, 0 },
    { string_view("i", 1), 1511, 0, 0, 0, 0 },
    { string_view("xs", 2), 1512, 0, 0, 0, 0 },
    { string_view("fun", 3), 1513, 1513, 0, 0, 0 },
    { string_view("r", 1), 1514, 1252, 0, 1, 0 },
    { string_view("x", 1), 1515, 0, 0, 0, 0 },
    { string_view("i", 1), 1516, 0, 0, 0, 0 },
    { string_view("xs", 2), 1517, 1254, 0, 0, 0 },
    { string_view("fun", 3), 1518, 1515, 0, 0, 0 },
    { string_view("r", 1), 1519, 1258, 0, 1, 0 },
    { string_view("x", 1), 1520, 1252, 0, 0, 0 },
    { string_view("i", 1), 1521, 0, 0, 0, 0 },
    { string_view("xs", 2), 1522, 1252, 0, 0, 0 },
    { string_view("fun", 3), 1523, 1517, 0, 0, 0 },
    { string_view("r", 1), 1524, 1256, 0, 1, 0 },
    { string_view("x", 1), 1525, 174, 0, 0, 0 },
    { string_view("x", 1), 1525, 174, 0, 0, 0 },
    { string_view("x", 1), 1525, 174, 0, 0, 0 },
    { string_view("i", 1), 1526, 0, 0, 0, 0 },
    { string_view("f", 1), 1527, 1519, 0, 0, 0 },
    { string_view("f", 1), 1528, 1521, 0, 0, 0 },
    { string_view("f", 1), 1529, 1523, 0, 0, 0 },
    { string_view("this", 4), 1530, 1309, 0, 0, 0 },
    { string_view("o", 1), 1531, 1309, 0, 0, 0 },
    { string_view("this", 4), 1532, 1309, 0, 0, 0 },
    { string_view("f", 1), 1533, 1525, 0, 0, 0 },
    { string_view("xs", 2), 1534, 1336, 0, 0, 0 },
    { string_view("fun", 3), 1535, 1527, 0, 0, 0 },
    { string_view("r", 1), 1536, 7, 0, 1, 0 },
    { string_view("x", 1), 1537, 1327, 0, 0, 0 },
    { string_view("i", 1), 1538, 0, 0, 0, 0 },
    { string_view("xs", 2), 1539, 1336, 0, 0, 0 },
    { string_view("fun", 3), 1540, 1529, 0, 0, 0 },
    { string_view("r", 1), 1541, 7, 0, 1, 0 },
    { string_view("x", 1), 1542, 1327, 0, 0, 0 },
    { string_view("i", 1), 1543, 0, 0, 0, 0 },
    { string_view("xs", 2), 1544, 1336, 0, 0, 0 },
    { string_view("fun", 3), 1545, 1531, 0, 0, 0 },
    { string_view("r", 1), 1546, 7, 0, 1, 0 },
    { string_view("x", 1), 1547, 1327, 0, 0, 0 },
    { string_view("i", 1), 1548, 0, 0, 0, 0 },
    { string_view("xs", 2), 1549, 1336, 0, 0, 0 },
    { string_view("fun", 3), 1550, 1533, 0, 0, 0 },
    { string_view("r", 1), 1551, 7, 0, 1, 0 },
    { string_view("x", 1), 1552, 1327, 0, 0, 0 },
    { string_view("i", 1), 1553, 0, 0, 0, 0 },
    { string_view("xs", 2), 1554, 1338, 0, 0, 0 },
    { string_view("fun", 3), 1555, 1535, 0, 0, 0 },
    { string_view("r", 1), 1556, 11, 0, 1, 0 },
    { string_view("x", 1), 1557, 146, 0, 0, 0 },
    { string_view("i", 1), 1558, 0, 0, 0, 0 },
    { string_view("xs", 2), 1559, 1338, 0, 0, 0 },
    { string_view("fun", 3), 1560, 1537, 0, 0, 0 },
    { string_view("r", 1), 1561, 11, 0, 1, 0 },
    { string_view("x", 1), 1562, 146, 0, 0, 0 },
    { string_view("i", 1), 1563, 0, 0, 0, 0 },
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
    { string_view("bool", 4), make_span(bool_vals0), 0 },
    { string_view("Material", 8), make_span(Material_vals1), 0 },
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
  140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 150, 150, 150, 151, 152,
  153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168,
  169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 180, 181, 182, 183,
  184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199,
  200, 201, 202, 203, 204, 205, 205, 206, 207, 208, 209, 210, 211, 212, 207, 208,
  209, 210, 211, 206, 207, 213, 213, 213, 213, 214, 215, 216, 217, 218, 219, 220,
  221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 233, 234, 235,
  236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251,
  252, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267,
  268, 267, 268, 267, 268, 267, 268, 267, 268, 267, 268, 269, 270, 271, 272, 273,
  274, 275, 276, 277, 278, 279, 280, 281, 282, 283, 284, 282, 285, 283, 286, 287,
  284, 288, 289, 290, 291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 302,
  303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318,
  319, 320, 321, 322, 323, 324, 325, 326, 327, 328, 329, 330, 331, 332, 333, 334,
  335, 336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 346, 347, 348, 348, 349,
  350, 351, 352, 353, 354, 355, 356, 357, 358, 348, 359, 360, 361, 362, 363, 364,
  365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 380,
  381, 382, 349, 383, 384, 385, 386, 357, 358, 348, 359, 360, 361, 362, 363, 364,
  365, 376, 377, 378, 379, 380, 381, 382, 349, 386, 387, 348, 348, 388, 389, 390,
  391, 392, 393, 394, 395, 396, 397, 398, 399, 400, 57, 57, 57, 57, 57, 57,
  57, 57, 57, 57, 7, 29, 161, 166, 167, 57, 124, 125, 126, 158, 17, 7,
  133, 144, 7, 145, 134, 133, 140, 133, 144, 7, 145, 134, 133, 140, 133, 57,
  57, 57, 57, 7, 57, 144, 7, 145, 133, 57, 194, 195, 193, 195, 57, 199,
  57, 201, 57, 57, 208, 208, 209, 211, 213, 57, 57, 217, 218, 220, 221, 57,
  144, 7, 145, 134, 133, 140, 133, 57, 57, 57, 57, 7, 7, 248, 242, 243,
  244, 246, 240, 243, 241, 244, 245, 249, 245, 241, 246, 7, 17, 240, 241, 7,
  7, 7, 7, 7, 10, 15, 295, 17, 17, 17, 7, 97, 98, 7, 98, 99,
  7, 99, 7, 7, 306, 7, 7, 7, 7, 48, 49, 13, 49, 7, 7, 48,
  13, 49, 23, 19, 15, 15, 7, 7, 7, 7, 57, 57, 57, 376, 349, 57,
  7, 7, 7, 7, 7, 7,
};

extern "C" void compiled_entry_point(VMRef vm, StackPtr sp) {
    if (vm.nfr.HashAll() != 7286824047428956706ULL) vm.BuiltinError("code compiled with mismatching builtin function library");
    fun_10000002(vm, sp);
}

int main(int argc, char *argv[]) {
    // This is hard-coded to call compiled_entry_point()
    lobster::VMMetaData vmmeta = {
        23,
        make_span((const lobster::type_elem_t *)&type_table, sizeof(type_table) / sizeof(int)),
        make_span(stringtable),
        make_span(file_names),
        make_span(function_names),
        make_span(udts),
        make_span(specidents),
        make_span(enums),
        make_span(ser_ids),
        string_view("Mon May 26 14:17:41 2025", 24),
        0x33793c4a18e2f8e9,
        make_span(subfunctions_to_function),
    };
    return RunCompiledCodeMain(argc, argv, &vmmeta, vtables, nullptr, "");
}
