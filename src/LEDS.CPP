#include "font.cpp"
unsigned led_updtime, pitch;

void text_i(unsigned char *dst, char *text, unsigned char ink)
{
   unsigned off = 0;
   unsigned char mask = (conf.flashcolor)? 0x78 : 0xF0; ink &= ~mask;
   for (unsigned char *x = (unsigned char*)text; *x; x++) {
      unsigned char *d0 = dst;
      for (unsigned y = 0; y < 8; y++) {
         unsigned char byte = font[(*x)*8+y];
         if (!off) d0[0] = byte, d0[1] = (d0[1] & mask) + ink;
         else d0[0] |= (byte >> off),
              d0[2] = byte << (8-off),
              d0[3] = (d0[3] & mask) + ink;
         d0 += pitch;
      }
      off += 6; if (off & 8) off -= 8, dst += 2;
   }
}

void text_16(unsigned char *dst, char *text, unsigned char attr)
{
   for (; *text; text++, dst += 2)
      for (unsigned y = 0; y < 16; y++)
         dst[y*pitch] = font16[16 * *(unsigned char*)text + y],
         dst[y*pitch+1] = attr;
}

unsigned char *aypos;
void paint_led(unsigned level, unsigned char at)
{
   if (level) {
      if (level > 15) level = 15, at = 0x0E;
      unsigned mask = (0xFFFF0000 >> level) & 0xFFFF;
      aypos[0] = mask >> 8;
      aypos[2] = (unsigned char)mask;
      aypos[1] = (aypos[1] & 0xF0) + at, aypos[3] = (aypos[3] & 0xF0) + at;
   }
   aypos += pitch;
}

void ay_led()
{
   aypos = temp.led.ay;
   unsigned char sum=0;

   int max_ay = (conf.sound.ay_scheme > AY::SCHEME_PSEUDO)? 2 : 1;
   for (int ay = 0; ay < max_ay; ay++) {
      for (int i = 0; i < 3; i++) {
         unsigned char tone = (comp.ay[ay].r_mix >> i) & 1,
                       noise = (comp.ay[ay].r_mix >> (i+3)) & 1;
         unsigned char c1 = 0, c2 = 0;
         unsigned char v = comp.ay[ay].reg[i+8];
         if (!tone) c1 = c2 = 0x0F;
         if (!noise) c2 = 0x0E;
         if (v & 0x10) {
            if (comp.ay[ay].r_envT < 0x400) {
               v = (3-(comp.ay[ay].r_envT>>3)) & 0x0F;
               if (!v) v = 6;
            } else v = comp.ay[ay].env/2;
            c1 = 0x0C;
         } else v &= 0x0F;
         if (!c1) c1 = c2;
         if (!c2) c2 = c1;
         if (!c1) v = 0;
         sum |= v;
         paint_led(v, c1);
         paint_led(v, c2);
         paint_led(0, 0);
      }
   }

   #ifdef MOD_GS
   if (sum || !conf.gs_type) return; // else show GS indicators
   aypos = temp.led.ay; // reset y-pos, if nothing above

   #ifdef MOD_GSBASS
   if (conf.gs_type == 2) {
      if (gs.mod_playing) {
         DWORD bass_vols = gs.BASS_ChannelGetLevel(gs.hmod);
         unsigned v = (bass_vols/8) & 0x7F;
         paint_led(v, 0x0D);
         paint_led(v, 0x0D);
         paint_led(0, 0);
         v = (bass_vols >> (16+3));
         paint_led(v, 0x0D);
         paint_led(v, 0x0D);
         paint_led(0, 0);
      }
      for (unsigned c = 0; c < 4; c++) {
         GSHLE::CHANNEL *ch = gs.chan+c;
         // adjust volume with play progress
         unsigned v = (ch->end)? (ch->volume*(ch->end - ch->ptr)/ch->end)/4 : 0;
         paint_led(v, 0x0F);
         paint_led(v, 0x0F);
         paint_led(0, 0);
      }
   }
   #endif

   #ifdef MOD_GSZ80
   if (conf.gs_type == 1) {
      for (unsigned ch = 0; ch < 4; ch++) {
         unsigned v = (z80gs::gsvol[ch] * temp.led.gssum[ch]) / (temp.led.gscnt[ch]*0x100*4+1);
         v = v*z80gs::gsvol[ch] / 0x40;
         temp.led.gssum[ch] = temp.led.gscnt[ch] = 0;
         paint_led(v, 0x0F);
         paint_led(v, 0x0F);
         paint_led(0, 0);
      }
   }
   #endif

   #endif
}

void load_led()
{
   char ln[20]; unsigned char diskcolor = 0;

#ifdef GS_BASS
   if (gs.loadmod) {
      text_i(temp.led.load, "", 0x0D);
      gs.loadmod = 0;
   } else if (gs.loadfx) {
      sprintf(ln, "\x0D%d", gs.loadfx);
      text_i(temp.led.load, ln, 0x0D);
      gs.loadfx = 0;
   } else
#endif
   if (trdos_format) {
      diskcolor = (trdos_format < ROMLED_TIME*3/4) ? 0x06 : 0x0E;
      trdos_format--;
   } else if (trdos_save) {
      diskcolor = (trdos_save < ROMLED_TIME*3/4) ? 0x02 : 0x0A;
      trdos_save--;
   } else if (trdos_load) {
      diskcolor = (trdos_load < ROMLED_TIME*3/4) ? 0x01 : 0x09;
      trdos_load--;
   } else if (comp.tape.play_pointer) {
      static unsigned char tapeled[11*2] = {
         0x7F, 0xFE, 0x80, 0x01, 0x80, 0x01, 0x93, 0xC9, 0xAA, 0x55, 0x93, 0xC9,
         0x80, 0x01, 0x8F, 0xF1, 0x80, 0x01, 0xB5, 0xA9, 0xFF, 0xFF };
      const tapecolor = 0x51; tape_bit();
      for (int i = 0; i < 11; i++)
         temp.led.load[pitch*i+0] = tapeled[2*i],
         temp.led.load[pitch*i+1] = tapecolor,
         temp.led.load[pitch*i+2] = tapeled[2*i+1],
         temp.led.load[pitch*i+3] = tapecolor;
      int time = (int)(temp.led.tape_started + tapeinfo[comp.tape.index].t_size - comp.t_states);
      if (time < 0) {
         find_tape_index(); time = 0;
         temp.led.tape_started = comp.t_states;
         unsigned char *ptr = tape_image + tapeinfo[comp.tape.index].pos;
         if (ptr == comp.tape.play_pointer && comp.tape.index)
            comp.tape.index--, ptr = tape_image + tapeinfo[comp.tape.index].pos;
         for (; ptr < comp.tape.play_pointer; ptr++)
            temp.led.tape_started -= tape_pulse[*ptr];
      }
      time /= (conf.frame * conf.intfq);
      sprintf(ln, "%X:%02d", time/60, time % 60);
      text_i(temp.led.load + pitch*12 - 2, ln, 0x0D);
   }
   if (diskcolor) {
      unsigned *ptr = (unsigned*)temp.led.load;
      for (int i = 0; i < 7; i++, ptr = (unsigned*)((char*)ptr+pitch))
         *ptr = (*ptr & WORD4(0,0xF0,0,0xF0)) | WORD4(0x3F,diskcolor,0xFC,diskcolor);
      static unsigned char disk[] = { 0x38, 0x1C, 0x3B, 0x9C, 0x3B, 0x9C, 0x3B, 0x9C, 0x38,0x1C };
      for (i = 0; i < 5; i++, ptr = (unsigned*)((char*)ptr+pitch))
         *ptr = (*ptr & WORD4(0,0xF0,0,0xF0)) | WORD4(disk[2*i],diskcolor,disk[2*i+1],diskcolor);
      sprintf(ln, "%02X", fdd_track[comp.wd.drive]*2 + comp.wd.side);
      text_i(temp.led.load + pitch - 4, ln, 0x05 + (diskcolor & 8));
   }
}

unsigned p_frames, p_time, p_fps;
__inline void update_perf_led()
{
   unsigned now = led_updtime - p_time;
   if (now > 500) {
      p_fps = (int)((double)p_frames*1000/now + 0.5);
      p_frames = 0; p_time = led_updtime;
   }
   p_frames++;
}

void perf_led()
{
   char bf[0x20]; unsigned PSZ;
   if (conf.led.perf_t)
      sprintf(bf, "%6d*%d", cpu.haltpos ? cpu.haltpos : cpu.t, p_fps), PSZ = 7;
   else sprintf(bf, "%2d fps", p_fps), PSZ = 5;
   text_i(temp.led.perf, bf, 0x0E);
   if (cpu.haltpos) {
      unsigned char *ptr = temp.led.perf + pitch*8;
      for (unsigned xx = 0; xx < PSZ; xx++) *(unsigned short*)(ptr+xx*2) = 0x9A00;
      unsigned mx = cpu.haltpos*PSZ*8/conf.frame;
      for (xx = 1; xx < mx; xx++) ptr[(xx>>2)&0xFE] |= (0x80 >> (xx & 7));
   }
}

void input_led()
{
   if (input.kbdled != 0xFF) {
      unsigned char k0 = 0x99, k1 = 0x9F, k2 = 0x90;
      if (input.keymode == K_INPUT::KM_PASTE_HOLD) k0 = 0xAA, k1 = 0xAF, k2 = 0xA0;
      if (input.keymode == K_INPUT::KM_PASTE_RELEASE) k0 = 0x22, k1 = 0x2F, k2 = 0x20;

      for (int i = 0; i < 5; i++)
         temp.led.input[1+i*2*pitch] = temp.led.input[3+i*2*pitch] = k0;
      for (i = 0; i < 4; i++)
         temp.led.input[pitch*(2*i+1)] = 0x7F,
         temp.led.input[pitch*(2*i+1)+2] = 0xFE;
      temp.led.input[pitch*1+1] = (input.kbdled & 0x08)? k2 : k1;
      temp.led.input[pitch*3+1] = (input.kbdled & 0x04)? k2 : k1;
      temp.led.input[pitch*5+1] = (input.kbdled & 0x02)? k2 : k1;
      temp.led.input[pitch*7+1] = (input.kbdled & 0x01)? k2 : k1;
      temp.led.input[pitch*1+3] = (input.kbdled & 0x10)? k2 : k1;
      temp.led.input[pitch*3+3] = (input.kbdled & 0x20)? k2 : k1;
      temp.led.input[pitch*5+3] = (input.kbdled & 0x40)? k2 : k1;
      temp.led.input[pitch*7+3] = (input.kbdled & 0x80)? k2 : k1;
   }
   static unsigned char joy[] =   { 0x10, 0x38, 0x1C, 0x1C, 0x1C, 0x1C, 0x08, 0x00, 0x7E, 0xFF, 0x00, 0xE7 };
   static unsigned char mouse[] = { 0x0C, 0x12, 0x01, 0x79, 0xB5, 0xB5, 0xB5, 0xFC, 0xFC, 0xFC, 0xFC, 0x78 };
   if (input.mouse_joy_led & 2)
      for (int i = 0; i < sizeof joy; i++)
         temp.led.input[4 + pitch*i] = joy[i],
         temp.led.input[4 + pitch*i+1] = (temp.led.input[4 + pitch*i+1] & 0xF0) + 0x0F;
   if (input.mouse_joy_led & 1)
      for (int i = 0; i < sizeof mouse; i++)
         temp.led.input[6 + pitch*i] = mouse[i],
         temp.led.input[6 + pitch*i+1] = (temp.led.input[6 + pitch*i+1] & 0xF0) + 0x0F;
   input.mouse_joy_led = 0; input.kbdled = 0xFF;
}

#ifdef MOD_MONITOR
void debug_led()
{
   unsigned char *ptr = temp.led.osw;
   if (trace_rom | trace_ram) {
      set_banks();
      if (trace_rom) {
         const unsigned char off = 0x01, on = 0x0C;
         text_i(ptr + 2,           "B48", used_banks[(base_sos_rom - memory) / PAGE] ? on : off);
         text_i(ptr + 8,           "DOS", used_banks[(base_dos_rom - memory) / PAGE] ? on : off);
         text_i(ptr + pitch*8 + 2, "128", used_banks[(base_128_rom - memory) / PAGE] ? on : off);
         text_i(ptr + pitch*8 + 8, "SYS", used_banks[(base_sys_rom - memory) / PAGE] ? on : off);
         ptr += pitch*16;
      }
      if (trace_ram) {
         unsigned num_rows = conf.ramsize/128;
         for (unsigned  i = 0; i < num_rows; i++) {
            char ln[9];
            for (unsigned j = 0; j < 8; j++)
               ln[j] = used_banks[i*8+j]? '*' : '-';
            ln[j] = 0;
            text_i(ptr, ln, 0x0D);
            ptr += pitch*8;
         }
      }
      for (unsigned j = 0; j < MAX_PAGES; j++) used_banks[j] = 0;
   }
   for (unsigned w = 0; w < 4; w++) if (watch_enabled[w]) {
      char bf[12]; sprintf(bf, "%8X", calc(watch_script[w]));
      text_i(ptr,bf,0x0F); ptr += pitch*8;
   }
}
#endif

HANDLE hndKbdDev;

void init_leds()
{
   DefineDosDevice(DDD_RAW_TARGET_PATH, "Kbd_unreal_spec", "\\Device\\KeyboardClass0");
   hndKbdDev = CreateFile("\\\\.\\Kbd_unreal_spec", GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
   if (hndKbdDev == INVALID_HANDLE_VALUE) hndKbdDev = 0, conf.led.flash_ay_kbd = 0;
}

void done_leds()
{
   if (hndKbdDev) {
      DefineDosDevice(DDD_REMOVE_DEFINITION, "Kbd_unreal_spec", 0);
      CloseHandle(hndKbdDev); hndKbdDev = 0;
   }
}




#define IOCTL_KEYBOARD_SET_INDICATORS        CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0002, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KEYBOARD_QUERY_TYPEMATIC       CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0008, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KEYBOARD_QUERY_INDICATORS      CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0010, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct _KEYBOARD_INDICATOR_PARAMETERS {
    USHORT UnitId;              // Unit identifier.
    USHORT LedFlags;            // LED indicator state.
} KEYBOARD_INDICATOR_PARAMETERS, *PKEYBOARD_INDICATOR_PARAMETERS;

#define KEYBOARD_CAPS_LOCK_ON     4
#define KEYBOARD_NUM_LOCK_ON      2
#define KEYBOARD_SCROLL_LOCK_ON   1

void ay_kbd()
{
   static unsigned char pA, pB, pC;
   static unsigned prev_keyled = -1;

   KEYBOARD_INDICATOR_PARAMETERS InputBuffer;
   InputBuffer.LedFlags = InputBuffer.UnitId = 0;

   if (comp.ay->reg[ 8] > pA) InputBuffer.LedFlags |= KEYBOARD_NUM_LOCK_ON;
   if (comp.ay->reg[ 9] > pB) InputBuffer.LedFlags |= KEYBOARD_CAPS_LOCK_ON;
   if (comp.ay->reg[10] > pC) InputBuffer.LedFlags |= KEYBOARD_SCROLL_LOCK_ON;

   pA = comp.ay->reg[8], pB = comp.ay->reg[9], pC = comp.ay->reg[10];

   DWORD xx;
   if (prev_keyled != InputBuffer.LedFlags)
      prev_keyled = InputBuffer.LedFlags,
      DeviceIoControl(hndKbdDev, IOCTL_KEYBOARD_SET_INDICATORS,
               &InputBuffer, sizeof(KEYBOARD_INDICATOR_PARAMETERS), 0, 0, &xx, 0);
}

void key_led()
{
   #define key_x 1
   #define key_y 1
   for (int i = 0; i < 9; i++) text_16(rbuf+(key_y+i)*pitch*16+key_x*2, "                                 ", 0x40);
   static char ks[] = "cZXCVASDFGQWERT1234509876POIUYeLKJHssMNB";
   for (i = 0; i < 8; i++) {
      for (int j = 0; j < 5; j++) {
         unsigned x, y, at;
         if (i < 4) y = 7-2*i+key_y, x = 3*j+2+key_x;
         else y = 2*(i-4)+1+key_y, x = 29-3*j+key_x;
         unsigned a = ks[i*5+j]*0x100+' ';
         at = (input.kbd[i] & (1<<j))? 0x07 : ((input.rkbd[i] & (1<<j)) ? 0xA0:0xD0);
         text_16(rbuf+2*x+y*pitch*16,(char*)&a,at);
      }
   }
}

void time_led()
{
   static unsigned prev_time;
   static char bf[8];
   if (led_updtime - prev_time > 5000) {
      prev_time = led_updtime;
      SYSTEMTIME st; GetLocalTime(&st);
      sprintf(bf, "%2d:%02d", st.wHour, st.wMinute);
   }
   text_i(temp.led.time, bf, 0x0D);
}

/*
void fdd_led()
{
   process_wd();
   unsigned char *ptr = screen + conf.led.fdd;
   unsigned i;
   static double phi = 0;
   phi += 2*PI/(50/5);
   for (unsigned d = 0; d < 4; d++, ptr += 22) {
      unsigned char a = (d == comp.wd.drive) ? 0x45: 0x04;
      if (disk[d].rawdata) {
         for (double t = 0; t < 8; t += 0.1)
            ptr[(int)(cos(t)*6+10) + MAX_WIDTH*(int)(sin(t)*6+10)] = a+1;
      }
      if (d == comp.wd.drive) {
         char xx[8]; sprintf(xx, "%2d", fdd_track[d]);
         printtrraw(ptr+MAX_WIDTH*12+6, xx, attrtab[0x105]);
         *(unsigned*)xx = '0'+comp.wd.side;
         printtrraw(ptr+2*6, xx, attrtab[0x143]);
      }
      static char x[2] = "A"; *x = 'A'+d;
      printtrraw(ptr, x, 0x46);
      a = attrtab[0x100 + a];
      for (i = 0; i < 20; i++) ptr[i] = a;
      for (i = 0; i < 20; i++) ptr[19*MAX_WIDTH+i] = a;
      for (i = 0; i < 20; i++) ptr[i*MAX_WIDTH] = ptr[i*MAX_WIDTH+19] = a;
      if (fdd_motor[d]) {
         ptr[(int)(cos(phi)*8+10) + MAX_WIDTH*(int)(sin(phi)*8+10)] = attrtab[0x142];
         ptr[(int)(cos(phi)*7+10) + MAX_WIDTH*(int)(sin(phi)*7+10)] = attrtab[0x142];
      }
      for (i = 1; i < 10; i++) ptr[10+MAX_WIDTH*i] = attrtab[0x147];
      unsigned char c = attrtab[0x142];
      *(unsigned*)(ptr+8+MAX_WIDTH*(fdd_track[d]*10/80)) = WORD4(0,c,c,c);
   }
}
*/

__inline void showleds()
{
   led_updtime = GetTickCount();
   update_perf_led();

   if (temp.vidblock) return;

   pitch = temp.scx/4;

   if (statcnt) { statcnt--; text_i(rbuf + ((pitch/2-strlen(statusline)*6/8) & 0xFE) + (temp.scy-10)*pitch, statusline, 0x09); }

   if (!conf.led.enabled) return;

   if (temp.led.ay) ay_led();
   if (temp.led.perf) perf_led();
   if (temp.led.load) load_led();
   if (temp.led.input) input_led();
   if (temp.led.time) time_led();
#ifdef MOD_MONITOR
   if (temp.led.osw) debug_led();
#endif
   if (conf.led.flash_ay_kbd && hndKbdDev) ay_kbd();
   if (input.keymode == K_INPUT::KM_KEYSTICK) key_led();
}
