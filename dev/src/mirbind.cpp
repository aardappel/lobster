#include "lobster/stdafx.h"

#include "lobster/compiler.h"
#include "lobster/tonative.h"

#include <setjmp.h>
#include <stdarg.h>

extern "C" {
#include "mir.h"
#include "mir-gen.h"
#include "c2mir.h"
}

// mir.h defines these unconditionally, and they are too generic to leave around.
#undef TRUE
#undef FALSE

#ifdef _MSC_VER
    // MIRCompileAndLink is deliberately kept free of objects with destructors, see MIRError.
    #pragma warning(disable: 4611)  // interaction between setjmp and C++ object destruction
#endif

namespace lobster {

// MIR reports errors thru a function that is not allowed to return, so we longjmp out of it.
// Everything in between the setjmp and the longjmp must therefore be free of C++ objects with
// destructors, which is why MIRCompileAndLink below is a plain C style function.
struct MIRError {
    jmp_buf jmp;
    char msg[1024];
};

static MIRError *mir_error = nullptr;

static void MIR_NO_RETURN MIRErrorFunc(MIR_error_type_t, const char *format, ...) {
    va_list args;
    va_start(args, format);
    if (mir_error) {
        vsnprintf(mir_error->msg, sizeof(mir_error->msg), format, args);
        va_end(args);
        longjmp(mir_error->jmp, 1);
    }
    vfprintf(stderr, format, args);
    va_end(args);
    abort();
}

struct MIRReader {
    const char *pos;
};

static int MIRGetC(void *data) {
    auto r = (MIRReader *)data;
    return *r->pos ? (int)(unsigned char)*r->pos++ : EOF;
}

// MIR_link's resolver takes no user data, so the symbols the generated code may import have to
// live here.
static const void **mir_imports = nullptr;

static void *MIRImportResolver(const char *name) {
    if (mir_imports) {
        for (auto p = mir_imports; *p; p += 2) {
            if (strcmp((const char *)p[0], name) == 0) return (void *)p[1];
        }
    }
    // C code generators like to call these even when the source doesn't mention them.
    if (strcmp(name, "memcpy") == 0) return (void *)memcpy;
    if (strcmp(name, "memmove") == 0) return (void *)memmove;
    if (strcmp(name, "memset") == 0) return (void *)memset;
    if (strcmp(name, "memcmp") == 0) return (void *)memcmp;
    if (strcmp(name, "strlen") == 0) return (void *)strlen;
    return nullptr;
}

static void *MIRFindItem(MIR_context_t ctx, const char *name) {
    for (auto m = DLIST_HEAD(MIR_module_t, *MIR_get_module_list(ctx)); m;
         m = DLIST_NEXT(MIR_module_t, m)) {
        for (auto item = DLIST_HEAD(MIR_item_t, m->items); item;
             item = DLIST_NEXT(MIR_item_t, item)) {
            // Imports/exports/forwards are aliases for a definition we'd find anyway, and they
            // are the only items whose name may repeat, so skip them.
            if (item->item_type == MIR_import_item || item->item_type == MIR_export_item ||
                item->item_type == MIR_forward_item)
                continue;
            auto iname = MIR_item_name(ctx, item);
            if (iname && strcmp(iname, name) == 0) return item->addr;
        }
    }
    return nullptr;
}

struct MIRJob {
    MIR_context_t ctx;
    int optimize_level;
    const char *source;
    const char *source_name;
    const char *object_name;
    const void **imports;
    const char **export_names;
    void **exports;
    FILE *msgs;
    int c2mir_inited;
    int gen_inited;
};

// Kept free of anything the longjmp in MIRErrorFunc could skip over, see MIRError.
static bool MIRCompileAndLink(MIRJob *job, MIRError *err) {
    struct c2mir_options ops;
    struct MIRReader reader;
    MIR_module_t m;
    FILE *f;
    const void **imp;
    size_t i;
    int ok = 0;
    err->msg[0] = 0;
    mir_error = err;
    mir_imports = job->imports;
    if (setjmp(err->jmp)) goto done;
    MIR_set_error_func(job->ctx, MIRErrorFunc);
    c2mir_init(job->ctx);
    job->c2mir_inited = 1;
    memset(&ops, 0, sizeof(ops));
    ops.message_file = job->msgs;
    reader.pos = job->source;
    if (!c2mir_compile(job->ctx, &ops, MIRGetC, &reader, job->source_name, NULL)) goto done;
    if (job->object_name) {
        // MIR has no object file writer, so this produces a binary MIR module instead, which
        // the m2b/b2m/c2m tools from the MIR distribution can read.
        if ((f = fopen(job->object_name, "wb")) == NULL) {
            snprintf(err->msg, sizeof(err->msg), "cannot write %s", job->object_name);
            goto done;
        }
        MIR_write(job->ctx, f);
        fclose(f);
        ok = 1;
        goto done;
    }
    for (m = DLIST_HEAD(MIR_module_t, *MIR_get_module_list(job->ctx)); m != NULL;
         m = DLIST_NEXT(MIR_module_t, m))
        MIR_load_module(job->ctx, m);
    for (imp = job->imports; *imp != NULL; imp += 2)
        MIR_load_external(job->ctx, (const char *)imp[0], (void *)imp[1]);
    MIR_gen_init(job->ctx);
    job->gen_inited = 1;
    if (job->optimize_level >= 0)
        MIR_gen_set_optimize_level(job->ctx, (unsigned)job->optimize_level);
    MIR_link(job->ctx, MIR_set_gen_interface, MIRImportResolver);
    for (i = 0; job->export_names[i] != NULL; i++)
        job->exports[i] = MIRFindItem(job->ctx, job->export_names[i]);
    ok = 1;
done:
    mir_error = NULL;
    mir_imports = NULL;
    return ok != 0;
}

// c2mir wants a FILE to write diagnostics to, so give it a temporary one we can read back.
static FILE *MIROpenMessageFile() {
    auto f = tmpfile();
    if (!f) f = fopen("mir_messages.tmp", "w+");
    return f;
}

static void MIRReadMessages(FILE *f, string &error) {
    if (!f) return;
    fflush(f);
    if (fseek(f, 0, SEEK_SET) == 0) {
        char buf[1024];
        size_t n;
        while ((n = fread(buf, 1, sizeof(buf), f)) > 0) error.append(buf, n);
    }
    fclose(f);
}

bool RunMIR(const char *source,
            const char *object_name,
            string &error,
            const void **imports,
            const char **export_names,
            const JitOptions &jit_options,
            function<bool (void **)> runf) {
    size_t num_exports = 0;
    while (export_names[num_exports]) num_exports++;
    vector<void *> exports(num_exports, nullptr);
    MIRJob job;
    memset(&job, 0, sizeof(job));
    job.source = source;
    job.source_name = "lobster_jit.c";
    job.object_name = object_name;
    job.imports = imports;
    job.export_names = export_names;
    job.exports = exports.data();
    // MIR's own default is 2, but from that level on it miscompiles what we generate: c2mir
    // gives each member of the anonymous union in Value its own alias, so a store thru .ival and
    // a load thru .ref of the same slot are taken to be independent. Unless asked otherwise,
    // stay one below.
    job.optimize_level = jit_options.optimize_level >= 0 ? jit_options.optimize_level : 1;
    job.msgs = MIROpenMessageFile();
    job.ctx = MIR_init();
    // The generated code may still throw an exception, so tear down thru a destructor.
    struct Cleanup {
        MIRJob *job;
        ~Cleanup() {
            if (job->gen_inited) MIR_gen_finish(job->ctx);
            if (job->c2mir_inited) c2mir_finish(job->ctx);
            MIR_finish(job->ctx);
        }
    } cleanup { &job };
    MIRError err;
    auto ok = MIRCompileAndLink(&job, &err);
    MIRReadMessages(job.msgs, error);
    if (!ok) {
        if (err.msg[0]) {
            error += err.msg;
            error += "\n";
        }
        return false;
    }
    if (object_name) return true;
    for (size_t i = 0; i < num_exports; i++) {
        if (!exports[i]) {
            error += cat("cannot find export \"", export_names[i], "\"\n");
            return false;
        }
    }
    return runf(exports.data());
}

}  // namespace lobster
