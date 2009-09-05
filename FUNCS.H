void __declspec(noreturn) exit();
void correct_exit();

void do_screenshot();
void qsave(char*);
void qload(char*);
void savesnap(int);

void setpal(char);
void set_vidmode();
void set_video();

void trdos_traps();
void tape_traps();
void fast_tape();
unsigned char tape_bit();

void out(unsigned port, unsigned char val);
unsigned char in(unsigned port);
void set_banks();

void applyconfig();
void apply_video();
void apply_gs();
void setup_dlg();
void savesnddialog();
void load_labels(char *filename, unsigned char *base, unsigned size);
unsigned char isbrk();

void rend_double(unsigned char *dst, unsigned pitch);
void prepare_chunks();
void prepare_chunks32();

void init_gs_frame();
void flush_gs_frame();
void reset_gs();
void reset_gs_sound();

void load_ay_stereo();
void load_ay_vols();
void load_ula_preset();
void create_font_tables();
void restart_sound();

void do_sound();
void flip();
void spectrum_frame();

void reset(ROM_MODE mode);
