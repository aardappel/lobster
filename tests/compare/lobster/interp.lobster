// AST interpreter, see ../SPEC.md section 3. Dispatch is a switch over class cases, which
// Lobster turns into a dense jump table and checks for exhaustiveness.

import std
import .rnd

private let DEPTH = 14
private let ROUNDS = 1500
private let NVARS = 8

private abstract class Expr
private class Const : Expr
    v:float
private class Var : Expr
    i:int
private class Neg : Expr
    a:Expr
private class Add : Expr
    a:Expr
    b:Expr
private class Sub : Expr
    a:Expr
    b:Expr
private class Mul : Expr
    a:Expr
    b:Expr

private def build(rnd:Rnd, d:int) -> Expr:
    if d == 0:
        return switch rnd.below(3):
            case 0: Const { rnd.range(-1.0, 1.0) }
            case 1: Var { rnd.below(NVARS) }
            default: Neg { build(rnd, 0) }
    // Multiplication only near the leaves, so the value of the tree stays finite.
    let k = rnd.below(if d <= 3: 3 else: 2)
    let a = build(rnd, d - 1)
    let b = build(rnd, d - 1)
    return switch k:
        case 0: Add { a, b }
        case 1: Sub { a, b }
        default: Mul { a, b }

private def eval(e:Expr, env:[float]) -> float:
    switch e:
        case Const: return e.v
        case Var: return env[e.i]
        case Neg: return -eval(e.a, env)
        case Add: return eval(e.a, env) + eval(e.b, env)
        case Sub: return eval(e.a, env) - eval(e.b, env)
        case Mul: return eval(e.a, env) * eval(e.b, env)

def bench_interp() -> float:
    let rnd = Rnd { 4242 }
    let tree = build(rnd, DEPTH)
    let env = map(NVARS): 0.0
    var total = 0.0
    for(ROUNDS) r:
        for(NVARS) k:
            env[k] = float((r * 7 + k * 13) % 101) / 50.0 - 1.0
        total += eval(tree, env)
    return total
