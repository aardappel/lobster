/* This file is a part of MIR project.
   Copyright (C) 2018-2024 Vladimir Makarov <vmakarov.gcc@gmail.com>.
   aarch64 call ABI target specific code.
*/

typedef int target_arg_info_t;

static void target_init_arg_vars (c2m_ctx_t c2m_ctx MIR_UNUSED,
                                  target_arg_info_t *arg_info MIR_UNUSED) {}

/* Backported from upstream MIR: https://github.com/vnmakarov/mir/commit/079a47da (vnmakarov/mir#477).
   Homogeneous floating-point aggregates (HFAs): structs/unions/arrays of 1-4
   members, all of the same float, double or long double type, with no
   padding.  They are passed in FP registers (MIR_T_BLK + 1..3 block args,
   see mir-aarch64.h) and returned in v0-v3.  */
static int hfa_walk (c2m_ctx_t c2m_ctx, struct type *type, struct type **el, mir_size_t *count) {
  switch (type->mode) {
  case TM_BASIC:
    if (type->u.basic_type != TP_FLOAT && type->u.basic_type != TP_DOUBLE
        && type->u.basic_type != TP_LDOUBLE)
      return FALSE;
    if (*el != NULL && (*el)->u.basic_type != type->u.basic_type) return FALSE;
    *el = type;
    (*count)++;
    return TRUE;
  case TM_ARR: {
    mir_size_t n = 0, el_size = type_size (c2m_ctx, type->u.arr_type->el_type);

    if (!hfa_walk (c2m_ctx, type->u.arr_type->el_type, el, &n) || el_size == 0) return FALSE;
    *count += n * (type_size (c2m_ctx, type) / el_size);
    return TRUE;
  }
  case TM_STRUCT:
  case TM_UNION: {
    mir_size_t max = 0;

    for (node_t member = NL_HEAD (NL_EL (type->u.tag_type->u.ops, 1)->u.ops); member != NULL;
         member = NL_NEXT (member))
      if (member->code == N_MEMBER) {
        decl_t decl = member->attr;
        mir_size_t n = 0;

        if (decl->bit_offset >= 0 || !hfa_walk (c2m_ctx, decl->decl_spec.type, el, &n))
          return FALSE;
        if (type->mode == TM_STRUCT)
          *count += n;
        else if (max < n)
          max = n;
      }
    *count += max;
    return TRUE;
  }
  default: return FALSE;
  }
}

/* Return the member count of HFA TYPE and set up its member MIR type, or return 0.  */
static int hfa_members (c2m_ctx_t c2m_ctx, struct type *type, MIR_type_t *el_type) {
  struct type *el = NULL;
  mir_size_t count = 0;

  if (type->mode != TM_STRUCT && type->mode != TM_UNION) return 0;
  if (!hfa_walk (c2m_ctx, type, &el, &count) || count < 1 || count > 4
      || type_size (c2m_ctx, type) != count * type_size (c2m_ctx, el))
    return 0;
  *el_type = get_mir_type (c2m_ctx, el);
  return (int) count;
}

static int target_return_by_addr_p (c2m_ctx_t c2m_ctx, struct type *ret_type) {
  MIR_type_t el_type;

  return ((ret_type->mode == TM_STRUCT || ret_type->mode == TM_UNION)
          && type_size (c2m_ctx, ret_type) > 2 * 8 && hfa_members (c2m_ctx, ret_type, &el_type) == 0);
}

static int reg_aggregate_size (c2m_ctx_t c2m_ctx, struct type *type) {
  size_t size;

  if (type->mode != TM_STRUCT && type->mode != TM_UNION) return -1;
  return (size = type_size (c2m_ctx, type)) <= 2 * 8 ? (int) size : -1;
}

static void target_add_res_proto (c2m_ctx_t c2m_ctx, struct type *ret_type,
                                  target_arg_info_t *arg_info, VARR (MIR_type_t) * res_types,
                                  VARR (MIR_var_t) * arg_vars) {
  MIR_type_t el_type;
  int size, n;

  if ((n = hfa_members (c2m_ctx, ret_type, &el_type)) != 0) {
    for (int i = 0; i < n; i++) VARR_PUSH (MIR_type_t, res_types, el_type);
    return;
  }
  if ((size = reg_aggregate_size (c2m_ctx, ret_type)) < 0) {
    simple_add_res_proto (c2m_ctx, ret_type, arg_info, res_types, arg_vars);
    return;
  }
  if (size == 0) return;
  VARR_PUSH (MIR_type_t, res_types, MIR_T_I64);
  if (size > 8) VARR_PUSH (MIR_type_t, res_types, MIR_T_I64);
}

static int target_add_call_res_op (c2m_ctx_t c2m_ctx, struct type *ret_type,
                                   target_arg_info_t *arg_info, size_t call_arg_area_offset) {
  gen_ctx_t gen_ctx = c2m_ctx->gen_ctx;
  MIR_context_t ctx = c2m_ctx->ctx;
  MIR_type_t el_type;
  int size, n;

  if ((n = hfa_members (c2m_ctx, ret_type, &el_type)) != 0) {
    for (int i = 0; i < n; i++)
      VARR_PUSH (MIR_op_t, call_ops, get_new_temp (c2m_ctx, el_type).mir_op);
    return n;
  }
  if ((size = reg_aggregate_size (c2m_ctx, ret_type)) < 0)
    return simple_add_call_res_op (c2m_ctx, ret_type, arg_info, call_arg_area_offset);
  if (size == 0) return -1;
  VARR_PUSH (MIR_op_t, call_ops,
             MIR_new_reg_op (ctx, get_new_temp (c2m_ctx, MIR_T_I64).mir_op.u.reg));
  if (size > 8)
    VARR_PUSH (MIR_op_t, call_ops,
               MIR_new_reg_op (ctx, get_new_temp (c2m_ctx, MIR_T_I64).mir_op.u.reg));
  return size <= 8 ? 1 : 2;
}

/* Move N HFA members of type EL_TYPE between REGS and the aggregate at MEM.  */
static void hfa_load_store (c2m_ctx_t c2m_ctx, MIR_type_t el_type, int n, MIR_op_t *regs,
                            MIR_op_t mem, int load_p) {
  gen_ctx_t gen_ctx = c2m_ctx->gen_ctx;
  MIR_context_t ctx = c2m_ctx->ctx;
  mir_size_t el_size = _MIR_type_size (ctx, el_type);

  for (int i = 0; i < n; i++) {
    MIR_op_t mem_op = MIR_new_mem_op (ctx, el_type, mem.u.mem.disp + i * el_size, mem.u.mem.base,
                                      mem.u.mem.index, mem.u.mem.scale);
    MIR_append_insn (ctx, curr_func,
                     MIR_new_insn (ctx, tp_mov (el_type), load_p ? regs[i] : mem_op,
                                   load_p ? mem_op : regs[i]));
  }
}

static op_t target_gen_post_call_res_code (c2m_ctx_t c2m_ctx, struct type *ret_type, op_t res,
                                           MIR_insn_t call, size_t call_ops_start) {
  gen_ctx_t gen_ctx = c2m_ctx->gen_ctx;
  MIR_type_t el_type;
  int size, n;

  if ((n = hfa_members (c2m_ctx, ret_type, &el_type)) != 0) {
    hfa_load_store (c2m_ctx, el_type, n, &VARR_ADDR (MIR_op_t, call_ops)[call_ops_start + 2],
                    res.mir_op, FALSE);
    return res;
  }
  if ((size = reg_aggregate_size (c2m_ctx, ret_type)) < 0)
    return simple_gen_post_call_res_code (c2m_ctx, ret_type, res, call, call_ops_start);
  if (size != 0)
    gen_multiple_load_store (c2m_ctx, ret_type, &VARR_ADDR (MIR_op_t, call_ops)[call_ops_start + 2],
                             res.mir_op, FALSE);
  return res;
}

static void target_add_ret_ops (c2m_ctx_t c2m_ctx, struct type *ret_type, op_t res) {
  gen_ctx_t gen_ctx = c2m_ctx->gen_ctx;
  MIR_type_t el_type;
  int i, size, n;

  if ((n = hfa_members (c2m_ctx, ret_type, &el_type)) != 0) {
    assert (res.mir_op.mode == MIR_OP_MEM && VARR_LENGTH (MIR_op_t, ret_ops) == 0);
    for (i = 0; i < n; i++) VARR_PUSH (MIR_op_t, ret_ops, get_new_temp (c2m_ctx, el_type).mir_op);
    hfa_load_store (c2m_ctx, el_type, n, VARR_ADDR (MIR_op_t, ret_ops), res.mir_op, TRUE);
    return;
  }
  if ((size = reg_aggregate_size (c2m_ctx, ret_type)) < 0) {
    simple_add_ret_ops (c2m_ctx, ret_type, res);
    return;
  }
  assert (res.mir_op.mode == MIR_OP_MEM && VARR_LENGTH (MIR_op_t, ret_ops) == 0 && size <= 2 * 8);
  for (i = 0; size > 0; size -= 8, i++)
    VARR_PUSH (MIR_op_t, ret_ops, get_new_temp (c2m_ctx, MIR_T_I64).mir_op);
  gen_multiple_load_store (c2m_ctx, ret_type, VARR_ADDR (MIR_op_t, ret_ops), res.mir_op, TRUE);
}

/* Block type for an aggregate passed by value: an HFA block type (MIR_T_BLK +
   1, 2, 3 for float, double, long double members) or MIR_T_BLK.  */
static MIR_type_t target_get_blk_type (c2m_ctx_t c2m_ctx, struct type *arg_type) {
  MIR_type_t el_type;

  if (hfa_members (c2m_ctx, arg_type, &el_type) == 0) return MIR_T_BLK;
  return el_type == MIR_T_F ? MIR_T_BLK + 1 : el_type == MIR_T_D ? MIR_T_BLK + 2 : MIR_T_BLK + 3;
}

static void target_add_arg_proto (c2m_ctx_t c2m_ctx, const char *name, struct type *arg_type,
                                  target_arg_info_t *arg_info, VARR (MIR_var_t) * arg_vars) {
  MIR_var_t var;

  if (arg_type->mode != TM_STRUCT && arg_type->mode != TM_UNION) {
    simple_add_arg_proto (c2m_ctx, name, arg_type, arg_info, arg_vars);
    return;
  }
  var.name = name;
  var.type = target_get_blk_type (c2m_ctx, arg_type);
  var.size = type_size (c2m_ctx, arg_type);
  VARR_PUSH (MIR_var_t, arg_vars, var);
}

static void target_add_call_arg_op (c2m_ctx_t c2m_ctx, struct type *arg_type,
                                    target_arg_info_t *arg_info, op_t arg) {
  gen_ctx_t gen_ctx = c2m_ctx->gen_ctx;

  if (arg_type->mode != TM_STRUCT && arg_type->mode != TM_UNION) {
    simple_add_call_arg_op (c2m_ctx, arg_type, arg_info, arg);
    return;
  }
  assert (arg.mir_op.mode == MIR_OP_MEM);
  arg = mem_to_address (c2m_ctx, arg, TRUE);
  VARR_PUSH (MIR_op_t, call_ops,
             MIR_new_mem_op (c2m_ctx->ctx, target_get_blk_type (c2m_ctx, arg_type),
                             type_size (c2m_ctx, arg_type), arg.mir_op.u.reg, 0, 1));
}

static int target_gen_gather_arg (c2m_ctx_t c2m_ctx MIR_UNUSED, const char *name MIR_UNUSED,
                                  struct type *arg_type MIR_UNUSED, decl_t param_decl MIR_UNUSED,
                                  target_arg_info_t *arg_info MIR_UNUSED) {
  return FALSE;
}
