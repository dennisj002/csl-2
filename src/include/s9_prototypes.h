/* src/primitives/s9.c */
void rerror(char *s, ptrdiff_t x);
void error(char *s, ptrdiff_t x);
void expect(char *who, char *what, ptrdiff_t got);
ptrdiff_t closure(ptrdiff_t i, ptrdiff_t e);
ptrdiff_t catch(void);
int throw(ptrdiff_t ct, ptrdiff_t v);
ptrdiff_t capture_cont(void);
int call_cont(ptrdiff_t c, ptrdiff_t v);
void rehash(void);
void addhash(ptrdiff_t x);
int lookup(ptrdiff_t x);
int strcmp_ci(char *s1, char *s2);
ptrdiff_t reverse(ptrdiff_t n);
ptrdiff_t nreverse(ptrdiff_t n);
ptrdiff_t conc(ptrdiff_t a, ptrdiff_t b);
ptrdiff_t nconc(ptrdiff_t a, ptrdiff_t b);
int memq(ptrdiff_t x, ptrdiff_t a);
int assq(ptrdiff_t x, ptrdiff_t a);
int posq(ptrdiff_t x, ptrdiff_t a);
int hashq(ptrdiff_t x, ptrdiff_t e);
ptrdiff_t set_union(ptrdiff_t a, ptrdiff_t b);
ptrdiff_t flatargs(ptrdiff_t a);
ptrdiff_t dotted_p(ptrdiff_t x);
ptrdiff_t carof(ptrdiff_t a);
ptrdiff_t zip(ptrdiff_t a, ptrdiff_t b);
ptrdiff_t lastpair(ptrdiff_t x);
ptrdiff_t exists_p(char *s);
ptrdiff_t subvector(ptrdiff_t x, int k0, int k1);
ptrdiff_t list_to_vector(ptrdiff_t m, char *msg, int flags);
ptrdiff_t list_to_string(ptrdiff_t x);
ptrdiff_t string_to_list(ptrdiff_t x);
ptrdiff_t vector_to_list(ptrdiff_t x);
void envbind(ptrdiff_t v, ptrdiff_t a);
void setbind(ptrdiff_t v, ptrdiff_t a);
ptrdiff_t getbind(ptrdiff_t v);
void clear_trace(void);
ptrdiff_t make_library_path(void);
void add_primitives(char *name, struct S9_primitive *p);
void init_extensions(void);
void init_rts(void);
void init(void);
void load_initial_image(char *image);
ptrdiff_t read_list(int flags, int delim);
ptrdiff_t quote(ptrdiff_t n, ptrdiff_t quotation);
ptrdiff_t read_character(void);
ptrdiff_t read_string(void);
void funny_char(char *msg, int c);
ptrdiff_t read_symbol_or_number(int c);
ptrdiff_t read_vector(void);
ptrdiff_t meta_command(void);
int block_comment(void);
int closing_paren(void);
ptrdiff_t bignum_read(char *pre, int radix);
ptrdiff_t read_real_number(int inexact);
ptrdiff_t unreadable(void);
ptrdiff_t read_form(int flags);
ptrdiff_t xread(void);
ptrdiff_t xsread(char *s);
int print_integer(ptrdiff_t n);
int print_realnum(ptrdiff_t n);
int print_quoted(ptrdiff_t n);
int print_procedure(ptrdiff_t n);
int print_catch_tag(ptrdiff_t n);
int print_continuation(ptrdiff_t n);
int print_char(ptrdiff_t n);
int print_string(ptrdiff_t n);
int print_symbol(ptrdiff_t n);
int print_primitive(ptrdiff_t n);
int print_vector(ptrdiff_t n);
int print_port(ptrdiff_t n);
int print_fixnum(ptrdiff_t n);
void print_special(ptrdiff_t n);
void x_print_form(ptrdiff_t n, int depth);
void print_form(ptrdiff_t n);
void ckargs(ptrdiff_t x, char *who, int min, int max);
int ckseq(ptrdiff_t x, int top);
int ckapply(ptrdiff_t x);
int ckbegin(ptrdiff_t x, int top);
int ckdefine(ptrdiff_t x, int top);
int ckif(ptrdiff_t x);
int ckifstar(ptrdiff_t x);
int symlistp(ptrdiff_t x);
int uniqlist(ptrdiff_t x);
int cklambda(ptrdiff_t x);
int ckmacro(ptrdiff_t x, int top);
int ckquote(ptrdiff_t x);
int cksetb(ptrdiff_t x);
int syncheck(ptrdiff_t x, int top);
ptrdiff_t free_vars(ptrdiff_t x, ptrdiff_t e);
ptrdiff_t mapconv(ptrdiff_t x, ptrdiff_t e, ptrdiff_t a);
ptrdiff_t initmap(ptrdiff_t fv, ptrdiff_t e, ptrdiff_t a);
void newvar(ptrdiff_t x);
void newvars(ptrdiff_t x);
ptrdiff_t lamconv(ptrdiff_t x, ptrdiff_t e, ptrdiff_t a);
int contains(ptrdiff_t a, ptrdiff_t x);
int liftable(ptrdiff_t x);
ptrdiff_t liftargs(ptrdiff_t m);
ptrdiff_t liftnames(ptrdiff_t m);
ptrdiff_t appconv(ptrdiff_t x, ptrdiff_t e, ptrdiff_t a);
ptrdiff_t defconv(ptrdiff_t x, ptrdiff_t e, ptrdiff_t a);
ptrdiff_t cconv(ptrdiff_t x, ptrdiff_t e, ptrdiff_t a);
ptrdiff_t zipenv(ptrdiff_t vs, ptrdiff_t oe);
ptrdiff_t clsconv(ptrdiff_t x);
void emit(ptrdiff_t x);
void emitop(ptrdiff_t op);
void emitq(ptrdiff_t x);
void patch(int a, ptrdiff_t x);
ptrdiff_t cpop(void);
void swap(void);
int subr0p(ptrdiff_t x);
int subr1p(ptrdiff_t x);
int subr2p(ptrdiff_t x);
int subr3p(ptrdiff_t x);
int osubr0p(ptrdiff_t x);
int osubr1p(ptrdiff_t x);
int osubr4p(ptrdiff_t x);
int lsubr0p(ptrdiff_t x);
int lsubr1p(ptrdiff_t x);
int lsubr2p(ptrdiff_t x);
int subrp(ptrdiff_t x);
void compbegin(ptrdiff_t x, int t);
void compsetb(ptrdiff_t x);
void compif(ptrdiff_t x, int t, int star);
void setupenv(ptrdiff_t m);
void compcls(ptrdiff_t x);
void compapply(ptrdiff_t x, int t);
void compapp(ptrdiff_t x, int t);
void compsubr0(ptrdiff_t x, int op);
void compsubr1(ptrdiff_t x, int op);
void compsubr2(ptrdiff_t x, int op);
void compsubr3(ptrdiff_t x, int op);
void composubr0(ptrdiff_t x, int op);
void composubr1(ptrdiff_t x, int op);
void composubr4(ptrdiff_t x, int op);
void complsubr0(ptrdiff_t x, int op);
void complsubr1(ptrdiff_t x, int op);
void complsubr2(ptrdiff_t x, int op);
void compexpr(ptrdiff_t x, int t);
ptrdiff_t compile(ptrdiff_t x);
ptrdiff_t mapexp(ptrdiff_t x, int all);
ptrdiff_t expanddef(ptrdiff_t x);
ptrdiff_t expandbody(ptrdiff_t x);
ptrdiff_t expand(ptrdiff_t x, int all);
void stkalloc(int k);
void push(ptrdiff_t x);
ptrdiff_t apply_extproc(ptrdiff_t pfn);
int apply(int tail);
int applis(int tail);
int ret(void);
void entcol(int fix);
void newmacro(ptrdiff_t name, ptrdiff_t fn);
ptrdiff_t integer_value(char *who, ptrdiff_t x);
ptrdiff_t integer_argument(char *who, ptrdiff_t x);
ptrdiff_t gensym(void);
char *rev_cxr_name(char *s);
ptrdiff_t cxr(char *op, ptrdiff_t x);
ptrdiff_t append(ptrdiff_t a, ptrdiff_t b);
ptrdiff_t list_length(ptrdiff_t a);
int eqv_p(ptrdiff_t a, ptrdiff_t b);
int assqv(char *who, int v, ptrdiff_t x, ptrdiff_t a);
int memqv(char *who, int v, ptrdiff_t x, ptrdiff_t a);
int nth(char *who, int ref, ptrdiff_t a, ptrdiff_t n);
ptrdiff_t bit_op(ptrdiff_t op, ptrdiff_t x, ptrdiff_t y);
ptrdiff_t add(ptrdiff_t x, ptrdiff_t y);
ptrdiff_t xsub(ptrdiff_t x, ptrdiff_t y);
ptrdiff_t mul(ptrdiff_t x, ptrdiff_t y);
ptrdiff_t xdiv(ptrdiff_t x, ptrdiff_t y);
ptrdiff_t intdiv(ptrdiff_t x, ptrdiff_t y);
ptrdiff_t intrem(ptrdiff_t x, ptrdiff_t y);
ptrdiff_t exact_to_inexact(ptrdiff_t x);
ptrdiff_t inexact_to_exact(ptrdiff_t x);
void grtr(ptrdiff_t x, ptrdiff_t y);
void gteq(ptrdiff_t x, ptrdiff_t y);
void less(ptrdiff_t x, ptrdiff_t y);
void lteq(ptrdiff_t x, ptrdiff_t y);
void equal(ptrdiff_t x, ptrdiff_t y);
void cless(ptrdiff_t x, ptrdiff_t y);
void clteq(ptrdiff_t x, ptrdiff_t y);
void cequal(ptrdiff_t x, ptrdiff_t y);
void cgrtr(ptrdiff_t x, ptrdiff_t y);
void cgteq(ptrdiff_t x, ptrdiff_t y);
void ciless(ptrdiff_t x, ptrdiff_t y);
void cilteq(ptrdiff_t x, ptrdiff_t y);
void ciequal(ptrdiff_t x, ptrdiff_t y);
void cigrtr(ptrdiff_t x, ptrdiff_t y);
void cigteq(ptrdiff_t x, ptrdiff_t y);
void sless(ptrdiff_t x, ptrdiff_t y);
void slteq(ptrdiff_t x, ptrdiff_t y);
void sequal(ptrdiff_t x, ptrdiff_t y);
void sgrtr(ptrdiff_t x, ptrdiff_t y);
void sgteq(ptrdiff_t x, ptrdiff_t y);
void siless(ptrdiff_t x, ptrdiff_t y);
void silteq(ptrdiff_t x, ptrdiff_t y);
void siequal(ptrdiff_t x, ptrdiff_t y);
void sigrtr(ptrdiff_t x, ptrdiff_t y);
void sigteq(ptrdiff_t x, ptrdiff_t y);
ptrdiff_t makestr(ptrdiff_t z, ptrdiff_t a);
ptrdiff_t sref(ptrdiff_t s, ptrdiff_t n);
void sset(ptrdiff_t s, ptrdiff_t n, ptrdiff_t r);
ptrdiff_t substring(ptrdiff_t s, ptrdiff_t n0, ptrdiff_t n1);
void sfill(ptrdiff_t a, ptrdiff_t n);
ptrdiff_t sconc(ptrdiff_t x);
ptrdiff_t makevec(ptrdiff_t z, ptrdiff_t a);
ptrdiff_t vref(ptrdiff_t s, ptrdiff_t n);
ptrdiff_t vconc(ptrdiff_t x);
ptrdiff_t vcopy(ptrdiff_t v, ptrdiff_t n0, ptrdiff_t nn, ptrdiff_t fill);
void vfill(ptrdiff_t a, ptrdiff_t n);
void vset(ptrdiff_t s, ptrdiff_t n, ptrdiff_t r);
ptrdiff_t openfile(ptrdiff_t x, int mode);
ptrdiff_t readchar(ptrdiff_t p, int rej);
ptrdiff_t read_obj(ptrdiff_t p);
void write_obj(ptrdiff_t x, int p, int disp);
void writechar(int c, ptrdiff_t p);
void dump_image_file(ptrdiff_t s);
void loadfile(char *s);
void load(ptrdiff_t x);
ptrdiff_t stats(ptrdiff_t x);
ptrdiff_t getenvvar(char *s);
ptrdiff_t cvt_bytecode(ptrdiff_t x);
void run(ptrdiff_t x);
ptrdiff_t interpret(ptrdiff_t x);
void begin_rec(void);
void end_rec(void);
ptrdiff_t eval(ptrdiff_t x, int r);
void keyboard_interrupt(int sig);
void keyboard_quit(int sig);
void mem_error(int src);
void reset_tty(void);
void repl(void);
void evalstr(char *s, int echo);
void usage(void);
void longusage(void);
long get_size_k(char *s);
char *cmdarg(char *s);
int s9_main(int argc, char **argv);
void CSL_S9_Init(void);
/* src/primitives/s9core.c */
void s9_run_stats(int x);
void s9_cons_stats(int x);
void s9_reset_counter(struct S9_counter *c);
void s9_count(struct S9_counter *c);
void s9_countn(struct S9_counter *c, int n);
ptrdiff_t s9_read_counter(struct S9_counter *c);
void s9_get_counters(struct S9_counter **nc, struct S9_counter **cc, struct S9_counter **vc, struct S9_counter **gc);
int s9_inport_open_p(void);
int s9_outport_open_p(void);
int s9_readc(void);
void s9_rejectc(int c);
void s9_writec(int c);
char *s9_open_input_string(char *s);
void s9_close_input_string(void);
void s9_flush(void);
void s9_set_printer_limit(int k);
int s9_printer_limit(void);
int s9_fwrite(char *s, int k);
void s9_blockwrite(char *s, int k);
int s9_blockread(char *s, int k);
void s9_prints(char *s);
int s9_io_status(void);
void s9_io_reset(void);
void s9_fatal(char *msg);
void s9_abort(void);
void s9_reset(void);
int s9_aborted(void);
void s9_set_node_limit(int n);
void s9_set_vector_limit(int n);
void s9_gc_verbosity(int n);
void s9_mem_error_handler(void (*h)(int src));
int s9_gc(void);
ptrdiff_t s9_cons3(ptrdiff_t pcar, ptrdiff_t pcdr, int ptag);
int s9_gcv(void);
ptrdiff_t s9_new_vec(ptrdiff_t type, int size);
ptrdiff_t s9_unsave(int k);
unsigned hash(char *s);
int hash_size(int n);
void add_symhash(ptrdiff_t x);
ptrdiff_t s9_find_symbol(char *s);
ptrdiff_t s9_make_symbol(char *s, int k);
ptrdiff_t s9_intern_symbol(ptrdiff_t y);
ptrdiff_t s9_symbol_table(void);
ptrdiff_t s9_symbol_ref(char *s);
ptrdiff_t s9_make_string(char *s, int k);
ptrdiff_t s9_make_vector(int k);
ptrdiff_t s9_mkfix(int v);
ptrdiff_t s9_make_integer(ptrdiff_t i);
ptrdiff_t s9_make_char(int x);
ptrdiff_t S9_make_real(int flags, ptrdiff_t exp, ptrdiff_t mant);
ptrdiff_t s9_make_real(int sign, ptrdiff_t exp, ptrdiff_t mant);
ptrdiff_t s9_make_primitive(struct S9_primitive *p);
ptrdiff_t s9_make_port(int portno, ptrdiff_t type);
ptrdiff_t s9_string_to_symbol(ptrdiff_t x);
ptrdiff_t s9_symbol_to_string(ptrdiff_t x);
ptrdiff_t s9_copy_string(ptrdiff_t x);
int s9_length(ptrdiff_t n);
int s9_conses(ptrdiff_t n);
ptrdiff_t s9_flat_copy(ptrdiff_t n, ptrdiff_t *lastp);
long s9_asctol(char *s);
char *ntoa(char *b, ptrdiff_t x, int w);
ptrdiff_t s9_argv_to_list(char **argv);
ptrdiff_t s9_bignum_abs(ptrdiff_t a);
ptrdiff_t s9_bignum_negate(ptrdiff_t a);
int s9_bignum_even_p(ptrdiff_t a);
ptrdiff_t s9_bignum_add(ptrdiff_t a, ptrdiff_t b);
int s9_bignum_less_p(ptrdiff_t a, ptrdiff_t b);
int s9_bignum_equal_p(ptrdiff_t a, ptrdiff_t b);
ptrdiff_t s9_bignum_subtract(ptrdiff_t a, ptrdiff_t b);
ptrdiff_t s9_bignum_shift_left(ptrdiff_t a, int fill);
ptrdiff_t s9_bignum_shift_right(ptrdiff_t a);
ptrdiff_t s9_bignum_multiply(ptrdiff_t a, ptrdiff_t b);
ptrdiff_t s9_bignum_divide(ptrdiff_t a, ptrdiff_t b);
ptrdiff_t s9_real_exponent(ptrdiff_t x);
ptrdiff_t s9_real_mantissa(ptrdiff_t x);
ptrdiff_t s9_bignum_to_real(ptrdiff_t a);
ptrdiff_t s9_real_negate(ptrdiff_t a);
ptrdiff_t s9_real_negative_p(ptrdiff_t a);
ptrdiff_t s9_real_positive_p(ptrdiff_t a);
ptrdiff_t s9_real_zero_p(ptrdiff_t a);
ptrdiff_t s9_real_abs(ptrdiff_t a);
ptrdiff_t shift_mantissa(ptrdiff_t m);
int s9_real_equal_p(ptrdiff_t a, ptrdiff_t b);
int s9_real_approx_p(ptrdiff_t a, ptrdiff_t b);
int s9_real_less_p(ptrdiff_t a, ptrdiff_t b);
ptrdiff_t s9_real_add(ptrdiff_t a, ptrdiff_t b);
ptrdiff_t s9_real_subtract(ptrdiff_t a, ptrdiff_t b);
ptrdiff_t s9_real_multiply(ptrdiff_t a, ptrdiff_t b);
ptrdiff_t s9_real_divide(ptrdiff_t a, ptrdiff_t b);
ptrdiff_t s9_real_sqrt(ptrdiff_t x);
ptrdiff_t s9_real_power(ptrdiff_t x, ptrdiff_t y);
ptrdiff_t s9_real_trunc(ptrdiff_t x);
ptrdiff_t s9_real_floor(ptrdiff_t x);
ptrdiff_t s9_real_ceil(ptrdiff_t x);
ptrdiff_t s9_real_to_bignum(ptrdiff_t r);
ptrdiff_t s9_real_integer_p(ptrdiff_t x);
int s9_integer_string_p(char *s);
int s9_string_numeric_p(char *s);
ptrdiff_t s9_string_to_bignum(char *s);
ptrdiff_t s9_string_to_real(char *s);
ptrdiff_t s9_string_to_number(char *s);
void s9_print_bignum(ptrdiff_t n);
void s9_print_expanded_real(ptrdiff_t n);
void s9_print_sci_real(ptrdiff_t n);
void s9_print_real(ptrdiff_t n);
ptrdiff_t s9_bignum_to_int(ptrdiff_t x, int *of);
ptrdiff_t s9_int_to_bignum(int v);
ptrdiff_t s9_bignum_to_string(ptrdiff_t x);
ptrdiff_t s9_real_to_string(ptrdiff_t x, int mode);
void s9_close_port(int port);
int s9_new_port(void);
int s9_open_input_port(char *path);
int s9_open_output_port(char *path, int append);
int s9_port_eof(int p);
int s9_error_port(void);
int s9_input_port(void);
int s9_output_port(void);
ptrdiff_t s9_set_input_port(ptrdiff_t port);
ptrdiff_t s9_set_output_port(ptrdiff_t port);
void s9_reset_std_ports(void);
int s9_lock_port(int port);
int s9_unlock_port(int port);
char *s9_typecheck(ptrdiff_t f);
ptrdiff_t s9_apply_prim(ptrdiff_t f);
char *s9_dump_image(char *path, char *magic);
char *s9_load_image(char *path, char *magic);
void s9_exponent_chars(char *s);
void s9_image_vars(ptrdiff_t **v);
void s9_add_image_vars(ptrdiff_t **v);
void CSL_S9Core_Init(void);
void s9_init(ptrdiff_t **extroots, ptrdiff_t *stack, int *stkptr);
void s9_fini(void);
