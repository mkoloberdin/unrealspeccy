
void main_pause()
{
   text_i(rbuf+temp.scx/2-8,"pause",0x0F); flip();

   pause = 1;
   sound_stop();
   updatebitmap(); active = 0;
   adjust_mouse_cursor();

   while (!process_msgs());
   eat();

   active = 1; adjust_mouse_cursor();
   sound_play();
   pause = 0;
}

void main_debug() {
   dbgchk = dbgbreak = 1;
}

enum { FIX_FRAME = 0, FIX_LINE, FIX_PAPER, FIX_NOPAPER, FIX_HWNC, FIX_LAST };
const char *fix_titles[FIX_LAST] = {
   "%d t-states / int",
   "%d t-states / line",
   "paper starts at %d",
   "border only: %d",
   "hardware mc: %d"
};


unsigned char whatfix = 0, whatsnd = 0;
unsigned char fixmode = -1;
int mul0 = 100, mul1 = 1000;

void chfix(int dx) {
   if (!fixmode) {
      int value;
      switch (whatfix) {
         case FIX_FRAME: value = (conf.frame += dx); break;
         case FIX_LINE: value = (conf.t_line += dx); break;
         case FIX_PAPER: value = (conf.paper += dx); break;
         case FIX_NOPAPER: value = (conf.nopaper ^= dx?1:0); break;
         case FIX_HWNC: value = (comp.pEFF7 ^= dx?EFF7_HWMC:0)? 1 : 0; break;
      }
      video_timing_tables();
      apply_sound(); // t/frame affects AY engine!
      sprintf(statusline, fix_titles[whatfix], value); statcnt=50;
      if (dx) conf.ula_preset = -1;
      return;
   }
   if (fixmode != 1) return;

   dx = (dx > 0) ? 1 : ((dx < 0) ? -1 : 0);

   *statusline = 0; statcnt = 50;
   switch (whatsnd) {
      case 0:
         conf.sound.ay_stereo = (conf.sound.ay_stereo+dx+num_aystereo) % num_aystereo;
         sprintf(statusline, "Stereo preset: %s", aystereo[conf.sound.ay_stereo]);
         break;
      case 1:
         conf.sound.ay_mix = (conf.sound.ay_mix+dx) & 3;
         sprintf(statusline, "Chip Mixer: %s", ay_mixers[conf.sound.ay_mix]);
         break;
      case 2:
         conf.sound.ay_vols = (conf.sound.ay_vols+num_ayvols+dx) % num_ayvols;
         sprintf(statusline, "Chip Table: %s", ayvols[conf.sound.ay_vols]);
         break;
      case 3:
         conf.pal = (conf.pal+dx);
         if (conf.pal == conf.num_pals) conf.pal = 0;
         if (conf.pal == -1) conf.pal = conf.num_pals-1;
         sprintf(statusline, "Palette: %s", pals[conf.pal].name);
         video_color_tables();
         return;
   }
   apply_sound();
}
void main_selectfix() {
   if (!fixmode) whatfix = (whatfix+1) % FIX_LAST;
   fixmode = 0; mul0 = 1, mul1 = 10;
   if (whatfix == FIX_FRAME) mul0 = 100, mul1 = 1000;
   chfix(0);
}
void main_selectsnd() {
   if (fixmode==1) whatsnd = (whatsnd+1) & 3;
   fixmode = 1;
   chfix(0);
}
void main_incfix() { chfix(mul0); }
void main_decfix() { chfix(-mul0); }
void main_incfix10() { chfix(mul1); }
void main_decfix10() { chfix(-mul1); }

void main_leds() {
   conf.led.enabled ^= 1;
   sprintf(statusline, "leds %s", conf.led.enabled ? "on" : "off"); statcnt = 50;
}

void main_maxspeed()
{
   conf.sound.enabled ^= 1;
   temp.frameskip = conf.sound.enabled? conf.frameskip : conf.frameskipmax;
   if (conf.sound.enabled) sound_play(); else sound_stop();
   sprintf(statusline, "Max speed: %s", conf.sound.enabled ? "NO" : "YES"); statcnt = 50;
}

void selvideo(int dx)
{
   for (unsigned nm = 0; renders[nm].func; nm++);
   conf.render += dx;
   if ((int)conf.render < 0) conf.render = nm-1;
   if (conf.render >= nm) conf.render = 0;
   sprintf(statusline, "Video: %s", renders[conf.render].name); statcnt = 50;
   apply_video();
}
void main_nextvideo() { selvideo(1); }
void main_prevvideo() { selvideo(-1); }
void main_poke() { DialogBox(hIn, MAKEINTRESOURCE(IDD_POKE), wnd, pokedlg), eat(); }
void main_starttape() {
   //if (comp.tape.play_pointer) stop_tape(); else start_tape();
   (comp.tape.play_pointer) ? stop_tape() : start_tape();
}
void main_tapebrowser() { lastpage = "TAPE", setup_dlg(); }

char *getrom(ROM_MODE page)
{
   switch (page) {
      case RM_128: return "Basic 128";
      case RM_SYS: return "Service ROM";
      case RM_DOS: return "TR-DOS";
      case RM_SOS: return "Basic 48";
      case RM_CACHE: return "Cache";
   }
   return "???";
}

void m_reset(ROM_MODE page)
{
   sprintf(statusline, "Reset to %s", getrom(page)); statcnt = 50;
   reset(page);
}
void main_reset128() { m_reset(RM_128); }
void main_resetsys() { m_reset(RM_SYS); }
void main_reset48() { m_reset(RM_SOS); comp.p7FFD = 0x30; }
void main_resetbas() { m_reset(RM_SOS); }
void main_resetdos() { if (conf.trdos_present) m_reset(RM_DOS); }
void main_resetcache() { if (conf.cache) m_reset(RM_CACHE); }
void main_reset() { m_reset((ROM_MODE)conf.reset_rom); }

void m_nmi(ROM_MODE page)
{
   set_mode(page);
   sprintf(statusline, "NMI to %s", getrom(page)); statcnt = 50;
   cpu.sp -= 2; z80dbg::wm(cpu.sp, cpu.pcl); z80dbg::wm(cpu.sp+1, cpu.pch);
   cpu.pc = 0x66; cpu.iff1 = cpu.halted = 0;
}
void main_nmi() { m_nmi(RM_NOCHANGE); }
void main_nmidos() { m_nmi(RM_DOS); }
void main_nmicache() { m_nmi(RM_CACHE); }

void qsave(char *fname) {
   char xx[0x200]; addpath(xx, fname);
   FILE *ff = fopen(xx, "wb");
   if (ff) {
      if (writeSNA(ff)) sprintf(statusline, "Quick save to %s", fname), statcnt = 30;
      fclose(ff);
   }
}
void qsave1() { qsave("qsave1.sna"); }
void qsave2() { qsave("qsave2.sna"); }
void qsave3() { qsave("qsave3.sna"); }

void qload(char *fname) {
   char xx[0x200]; addpath(xx, fname);
   if (loadsnap(xx)) sprintf(statusline, "Quick load from %s", fname), statcnt = 30;
}
void qload1() { qload("qsave1.sna"); }
void qload2() { qload("qsave2.sna"); }
void qload3() { qload("qsave3.sna"); }

void main_keystick()
{
   input.keymode = (input.keymode == K_INPUT::KM_KEYSTICK)? K_INPUT::KM_DEFAULT : K_INPUT::KM_KEYSTICK;
}

void main_autofire()
{
   conf.input.fire ^= 1;
   input.firedelay = 1;
   sprintf(statusline, "autofire %s", conf.input.fire ? "on" : "off"), statcnt = 30;
}

void main_save()
{
   if (conf.cmos) save_nv();
   if (!testdisk(0)) return;
   if (!testdisk(1)) return;
   if (!testdisk(2)) return;
   if (!testdisk(3)) return;

   if (!(disk[0].optype+disk[1].optype+disk[2].optype+disk[3].optype))
      sprintf(statusline, "all saved"), statcnt = 30;
}
void main_fullscr() {
   if (!(temp.rflags & (RF_GDI|RF_OVR|RF_CLIP))) sprintf(statusline, "only for overlay/gdi modes"), statcnt = 30;
   else conf.fullscr ^= 1, apply_video();
}
void main_mouse() {
   conf.lockmouse ^= 1;
   adjust_mouse_cursor();
}
void main_help() { showhelp(); }
void mon_help() { showhelp("monitor_keys"); }

void main_atmkbd()
{
   conf.atm.xt_kbd ^= 1;
   if (conf.atm.xt_kbd) sprintf(statusline, "ATM mode on. emulator hotkeys disabled");
   else sprintf(statusline, "ATM mode off");
   statcnt = 50;
}

void main_pastetext() { input.paste(); }

void correct_exit()
{
   if (!testdisk(0)) return;
   if (!testdisk(1)) return;
   if (!testdisk(2)) return;
   if (!testdisk(3)) return;
   freedisk(0); freedisk(1); freedisk(2); freedisk(3);
   nowait = 1; exit();
}

