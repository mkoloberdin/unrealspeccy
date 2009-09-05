
unsigned char pastekeys[0x80-0x20] =
{
   // s     !     "     #     $     %     &     '     (     )     *     +     ,     -   .       /
   0x71, 0xB1, 0xD1, 0xB3, 0xB4, 0xB5, 0xC5, 0xC4, 0xC3, 0xC2, 0xF5, 0xE3, 0xF4, 0xE4, 0xF3, 0x85,
   // 0     1     2     3     4     5     6     7     8     9     :     ;     <     =     >     ?
   0x41, 0x31, 0x32, 0x33, 0x34, 0x35, 0x45, 0x44, 0x43, 0x42, 0x82, 0xD2, 0xA4, 0xE2, 0xA5, 0x84,
   // @     A     B     C     D     E     F     G     H     I     J     K     L     M     N     O
   0xB2, 0x19, 0x7D, 0x0C, 0x1B, 0x2B, 0x1C, 0x1D, 0x6D, 0x5B, 0x6C, 0x6B, 0x6A, 0x7B, 0x7C, 0x5A,
   // P     Q     R     S     T     U     V     W     X     Y     Z     [     \     ]     ^     _
   0x59, 0x29, 0x2C, 0x1A, 0x2D, 0x5C, 0x0D, 0x2A, 0x0B, 0x5D, 0x0A, 0xD5, 0x93, 0xD4, 0xE5, 0xC1,
   // `     a     b     c     d     e     f     g     h     i     j     k     l     m     n     o
   0x00, 0x11, 0x75, 0x04, 0x13, 0x23, 0x14, 0x15, 0x65, 0x53, 0x64, 0x63, 0x62, 0x73, 0x74, 0x52,
   // p     q     r     s     t     u     v     w     x     y     z     {     |     }     ~
   0x51, 0x21, 0x24, 0x12, 0x25, 0x54, 0x05, 0x22, 0x03, 0x55, 0x02, 0x94, 0x92, 0x95, 0x91, 0x00
};

__inline void K_INPUT::clear_zx()
{
   kbd_x4[0] = kbd_x4[1] = -1;
}

void K_INPUT::make_matrix()
{
   unsigned char altlock = conf.input.altlock? (kbdpc[VK_ALT] | kbdpc[VK_LMENU] | kbdpc[VK_RMENU]) & 0x80 : 0;

   kjoy = 0xFF;
   switch (keymode) {
      case KM_DEFAULT:
         kbd_x4[0] = kbd_x4[1] = -1;
         if (!altlock)
            for (int i = 0; i < VK_MAX; i++)
               if (kbdpc[i] & 0x80)
                  *(inports[i].port1) &= inports[i].mask1,
                  *(inports[i].port2) &= inports[i].mask2;
         if (conf.input.fire) {
            if (!--firedelay)
               firedelay = conf.input.firedelay, firestate ^= 1;
            if (firestate) *(zxk[conf.input.firenum].port) &= zxk[conf.input.firenum].mask;
         }
         break;

      case KM_KEYSTICK:
         kbd_x4[0] = rkbd_x4[0]; kbd_x4[1] = rkbd_x4[1];
         if (stick_delay) stick_delay--, altlock = 1;
         if (!altlock)
            for (int i = 0; i < VK_MAX; i++)
               if (kbdpc[i] & 0x80)
                  *(inports[i].port1) ^= ~inports[i].mask1,
                  *(inports[i].port2) ^= ~inports[i].mask2;
         if ((kbd_x4[0] ^ rkbd_x4[0]) | (kbd_x4[1] ^ rkbd_x4[1])) stick_delay = 10;
         break;

      case KM_PASTE_HOLD:
      {
         kbd_x4[0] = kbd_x4[1] = -1;
         if (tdata & 0x08) kbd[0] &= ~1; // caps
         if (tdata & 0x80) kbd[7] &= ~2; // sym
         if (tdata & 7) kbd[(tdata >> 4) & 7] &= ~(1 << ((tdata & 7) - 1));
         if (tdelay) { tdelay--; break; }
         tdelay = conf.input.paste_release;
         if (tdata == 0x61) tdelay += conf.input.paste_newline;
         keymode = KM_PASTE_RELEASE;
         break;
      }

      case KM_PASTE_RELEASE:
      {
         kbd_x4[0] = kbd_x4[1] = -1;
         if (tdelay) { tdelay--; break; }
         if (textsize == textoffset) {
            keymode = KM_DEFAULT;
            free(textbuffer);
            textbuffer = 0;
            break;
         }
         tdelay = conf.input.paste_hold;
         unsigned char kdata = textbuffer[textoffset++];
         if (kdata == 0x0D) {
            if (textoffset < textsize && textbuffer[textoffset] == 0x0A) textoffset++;
            tdata = 0x61;
         } else {
            if (kdata < 0x20 || kdata >= 0x80) break; // keep release state
            tdata = pastekeys[kdata - 0x20];
            if (!tdata) break; // empty key
         }
         keymode = KM_PASTE_HOLD;
         break;
      }
   }
   kjoy ^= 0xFF;
   if (conf.input.joymouse) kjoy |= mousejoy;

   rkbd_x4[0] = kbd_x4[0]; rkbd_x4[1] = kbd_x4[1];
   if (!conf.input.keymatrix) return;
   for (;;) {
      char done = 1;
      for (int k = 0; k < 7; k++)
         for (int j = k+1; j < 8; j++)
            if (((kbd[k] | kbd[j]) != 0xFF) && (kbd[k] != kbd[j]))
               kbd[k] = kbd[j] = (kbd[k] & kbd[j]), done = 0;
      if (done) return;
   }
}

__inline int sign_pm(int a) { return (a < 0)? -1 : 1; }

char K_INPUT::readdevices()
{
   if (nokb) nokb--;
   if (nomouse) nomouse--;

   kbdpc[VK_JLEFT] = kbdpc[VK_JRIGHT] = kbdpc[VK_JUP] = kbdpc[VK_JDOWN] = kbdpc[VK_JFIRE] = 0;
   if (active && dijoyst) {
      dijoyst->Poll(); DIJOYSTATE js;
      readdevice(&js, sizeof js, (LPDIRECTINPUTDEVICE)dijoyst);
      if ((signed short)js.lX < 0) kbdpc[VK_JLEFT] = 0x80;
      if ((signed short)js.lX > 0) kbdpc[VK_JRIGHT] = 0x80;
      if ((signed short)js.lY < 0) kbdpc[VK_JUP] = 0x80;
      if ((signed short)js.lY > 0) kbdpc[VK_JDOWN] = 0x80;
      if (js.rgbButtons[0] & 0x80) kbdpc[VK_JFIRE] = 0x80;
   }

   mbuttons = 0xFF;
   msx_prev = msx, msy_prev = msy;
   kbdpc[VK_LMB] = kbdpc[VK_RMB] = kbdpc[VK_MMB] = kbdpc[VK_MWU] = kbdpc[VK_MWD] = 0;
   if ((conf.fullscr || conf.lockmouse) && !nomouse)
   {
      unsigned cl1, cl2;
      cl1 = abs(msx - msx_prev) * ay_reset_t / conf.frame;
      cl2 = abs(msx - msx_prev);
      ay_x0 += (cl2-cl1)*sign_pm(msx - msx_prev);
      cl1 = abs(msy - msy_prev) * ay_reset_t / conf.frame;
      cl2 = abs(msy - msy_prev);
      ay_y0 += (cl2-cl1)*sign_pm(msy - msy_prev);
      ay_reset_t = 0;

      DIMOUSESTATE md; readmouse(&md);
      if (conf.input.mouseswap) { unsigned char t = md.rgbButtons[0]; md.rgbButtons[0] = md.rgbButtons[1]; md.rgbButtons[1] = t; }
      msx = md.lX, msy = -md.lY;
      if (conf.input.mousescale >= 0) msx *= (1 << conf.input.mousescale), msy *= (1 << conf.input.mousescale);
      else msx /= (1 << -conf.input.mousescale), msy /= (1 << -conf.input.mousescale);

      if (md.rgbButtons[0]) mbuttons &= ~1, kbdpc[VK_LMB] = 0x80;
      if (md.rgbButtons[1]) mbuttons &= ~2, kbdpc[VK_RMB] = 0x80;
      if (md.rgbButtons[2]) mbuttons &= ~4, kbdpc[VK_MMB] = 0x80;

      int wheel_delta = md.lZ - prev_wheel;
      if (wheel_delta < 0) kbdpc[VK_MWD] = 0x80;
      if (wheel_delta > 0) kbdpc[VK_MWU] = 0x80;
      prev_wheel = md.lZ;
   }
   lastkey = process_msgs();

   if (nokb) memset(kbdpc, 0, sizeof kbdpc);
   else {
      GetKeyboardState(kbdpc);
      if (lastkey) kbdpc[lastkey] = 0x80;
   }

   return lastkey? 1 : 0;
}

void K_INPUT::aymouse_wr(unsigned char val)
{
   // reset by edge bit6: 1->0
   if (ayR14 & ~val & 0x40) ay_x0 = ay_y0 = 8, ay_reset_t = cpu.t;
   ayR14 = val;
}

unsigned char K_INPUT::aymouse_rd()
{
   unsigned coord;
   if (ayR14 & 0x40) {
      unsigned cl1 = abs(msy - msy_prev) * ay_reset_t / conf.frame;
      unsigned cl2 = abs(msy - msy_prev) * cpu.t / conf.frame;
      coord = ay_y0 + (cl2-cl1)*sign_pm(msy - msy_prev);
   } else {
      unsigned cl1 = abs(msx - msx_prev) * ay_reset_t / conf.frame;
      unsigned cl2 = abs(msx - msx_prev) * cpu.t / conf.frame;
      coord = ay_x0 + (cl2-cl1)*sign_pm(msx - msx_prev);
   }
/*
   int coord = (ayR14 & 0x40)?
     ay_y0 + 0x100 * (msy - msy_prev) * (int)(cpu.t - ay_reset_t) / (int)conf.frame:
     ay_x0 + 0x100 * (msx - msx_prev) * (int)(cpu.t - ay_reset_t) / (int)conf.frame;
//   if ((coord & 0x0F)!=8 && !(ayR14 & 0x40)) printf("coord: %X, x0=%4d, frame_dx=%6d, dt=%d\n", (coord & 0x0F), ay_x0, msx-msx_prev, cpu.t-ay_reset_t);
*/
   return 0xC0 | (coord & 0x0F) | (mbuttons << 4);
}

unsigned char K_INPUT::kempston_mx()
{
   int x = (cpu.t*msx + (conf.frame - cpu.t)*msx_prev) / conf.frame;
   return (unsigned char)x;
}

unsigned char K_INPUT::kempston_my()
{
   int y = (cpu.t*msy + (conf.frame - cpu.t)*msy_prev) / conf.frame;
   return (unsigned char)y;
}

unsigned char K_INPUT::read(unsigned char scan)
{
   unsigned char res = 0xBF | (tape_bit() & 0x40);
   kbdled &= scan;
   if (conf.atm.xt_kbd) return input.atm51.read(scan, res);
   for (int i = 0; i < 8; i++)
      if (!(scan & (1<<i))) res &= kbd[i];
   return res;
}

void K_INPUT::paste()
{
   free(textbuffer); textbuffer = 0;
   textsize = textoffset = 0;
   keymode = KM_DEFAULT;
   if (!OpenClipboard(wnd)) return;
   HANDLE hClip = GetClipboardData(CF_TEXT);
   if (hClip) {
      void *ptr = GlobalLock(hClip);
      if (ptr) {
         keymode = KM_PASTE_RELEASE; tdelay = 1;
         textsize = strlen((char*)ptr) + 1;
         memcpy(textbuffer = (unsigned char*)malloc(textsize), ptr, textsize);
         GlobalUnlock(hClip);
      }
   }
   CloseClipboard();
}

unsigned char ATM_KBD::read(unsigned char scan, unsigned char zxdata)
{
   unsigned char t;

   if (R7) {
      if (R7 == 1) cmd = scan;
      switch (cmd & 0x3F) {
         case 1:
         {
            static const unsigned char ver[4] = { 6,0,1,0 };
            R7 = 0; return ver[cmd >> 6];
         }
         case 7: clear(); R7 = 0; return 0xFF; // clear data buffer in mode0
         case 8:
            if (R7 == 2) { mode = scan; kR2 = 0; R7 = 0; return 0xFF; }
            R7++; return 8;
         case 9:
            switch (cmd & 0xC0) {
               case 0x00: t = kR1;
               case 0x40: t = kR2;
               case 0x80: t = kR3;
               case 0xC0: t = kR4;
            }
            R7 = 0;
            return t;
         case 10:
            kR3 |= 0x80; R7 = 0; return 0xFF;
         case 11:
            kR3 &= 0x7F; R7 = 0; return 0xFF;
//         case 12: R7 = 0; return 0xFF; // enter pause mode
         case 13:
            // reset!
            this->reset();
            cpu.int_flags = cpu.ir_ = cpu.pc = 0; cpu.im = 0;
            comp.p7FFD = comp.flags = 0;
            set_atm_FF77(0,0);
            set_banks();
            break;
         case 16:
         case 18:
         {
            SYSTEMTIME time; GetLocalTime(&time);
            R7 = 0;
            if (cmd == 0x10) return (BYTE)time.wSecond;
            if (cmd == 0x40) return (BYTE)time.wMinute;
            if (cmd == 0x80) return (BYTE)time.wHour;
            if (cmd == 0xC0) return (BYTE)time.wDay;
            if (cmd == 0x12) return (BYTE)time.wDay;
            if (cmd == 0x42) return (BYTE)time.wMonth;
            if (cmd == 0x82) return (BYTE)(time.wYear % 100);
            if (cmd == 0xC2) return (BYTE)(time.wYear / 100);
         }
         case 17: // set time
         case 19: // set date
            if (R7 == 2) R7 = 0; else R7++;
            return 0xFF;
      }
      R7 = 0;
      return 0xFF;
   }

   if (scan == 0x55) { R7++; return 0xAA; }

   switch (mode & 3) {
      case 0:
      {
         unsigned char res = zxdata | 0x1F;
         for (unsigned i = 0; i < 8; i++)
            if (!(scan & (1 << i))) res &= zxkeys[i];
         return res;
      }
      case 1: t = kR2; kR2 = 0; return t;
      case 2:
         switch (scan & 0xC0)
         {
            case 0x00: { t = kR2; kR2 = 0; return t; }
            case 0x40: return kR3;
            case 0x80: return kR4;
            case 0xC0: return kR5;
         }
      case 3: t = lastscan; lastscan = 0; return t;
   }
   __assume(0);
}

void ATM_KBD::processzx(unsigned scancode, unsigned char pressed)
{
   static const unsigned char L_4B6[] =
   {
      0x39, 0x31, 0x32, 0x33, 0x34, 0x35, 0x45, 0x44,
      0x43, 0x42, 0x41, 0xE4, 0xE2, 0x49, 0x3B, 0x21,
      0x22, 0x23, 0x24, 0x25, 0x55, 0x54, 0x53, 0x52,
      0x51, 0xD5, 0xD4, 0x61, 0x88, 0x11, 0x12, 0x13,
      0x14, 0x15, 0x65, 0x64, 0x63, 0x62, 0xD2, 0xD1,
      0x91, 0x08, 0x92, 0x02, 0x03, 0x04, 0x05, 0x75,
      0x74, 0x73, 0xF4, 0xF3, 0x85, 0x80, 0xF5, 0x3C,
      0x71, 0x3A, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xC5,
      0xC4, 0xC3, 0xC2, 0xC1, 0x00, 0x00, 0x3C, 0x4C,
      0x3D, 0xE4, 0x3D, 0x35, 0x4B, 0xE3, 0x4A, 0x4D,
      0x4B, 0x84, 0x49, 0x00, 0x00, 0x00, 0xE5, 0x94,
      0x00, 0x00, 0x00
   };
   scancode = (scancode & 0xFF) - 1; if (scancode >= sizeof L_4B6) return;
   unsigned char x = L_4B6[scancode];
   if (x & 0x08) { if (pressed) zxkeys[0] &= ~1; else zxkeys[0] |= 1; }
   if (x & 0x80) { if (pressed) zxkeys[7] &= ~2; else zxkeys[7] |= 2; }
   if (!(x & 7)) return;
   unsigned char data = 1 << ((x & 7) - 1);
   x = (x >> 4) & 7;
   if (pressed) zxkeys[x] &= ~data; else zxkeys[x] |= data;
}

void ATM_KBD::setkey(unsigned scancode, unsigned char pressed)
{
   if (!(mode & 3)) processzx(scancode, pressed);
   lastscan = (unsigned char)scancode;
   if (!pressed) { lastscan |= 0x80; return; }

   kR3 &= 0x80; // keep rus/lat, clear alt,ctrl,shift, num/scroll/caps lock
   if ((kbdpc[VK_SHIFT] | kbdpc[VK_LSHIFT] | kbdpc[VK_RSHIFT]) & 0x80) kR3 |= 1;
   if ((kbdpc[VK_CONTROL] | kbdpc[VK_LCONTROL] | kbdpc[VK_RCONTROL]) & 0x80) kR3 |= 2;
   if ((kbdpc[VK_MENU] | kbdpc[VK_LMENU] | kbdpc[VK_RMENU]) & 0x80) kR3 |= 4;
   if (kbdpc[VK_CAPITAL] & 1) kR3 |= 0x10;
   if (kbdpc[VK_NUMLOCK] & 1) kR3 |= 0x20;
   if (kbdpc[VK_SCROLL] & 1) kR3 |= 0x40;
   kR4 = 0; if (kbdpc[VK_RSHIFT] & 0x80) kR4++;

   static const unsigned char L_400[] =
   {
      0x1B, 0x00, 0x31, 0x00, 0x32, 0x00, 0x33, 0x00, 0x34, 0x00, 0x35, 0x00, 0x36, 0x00, 0x37, 0x00,
      0x38, 0x00, 0x39, 0x00, 0x30, 0x00, 0x2D, 0x00, 0x3D, 0x00, 0x08, 0x00, 0x09, 0x00, 0x51, 0x00,
      0x57, 0x00, 0x45, 0x00, 0x52, 0x00, 0x54, 0x00, 0x59, 0x00, 0x55, 0x00, 0x49, 0x00, 0x4F, 0x00,
      0x50, 0x00, 0x5B, 0x00, 0x5D, 0x00, 0x0D, 0xC0, 0x00, 0x02, 0x41, 0x00, 0x53, 0x00, 0x44, 0x00,
      0x46, 0x00, 0x47, 0x00, 0x48, 0x00, 0x4A, 0x00, 0x4B, 0x00, 0x4C, 0x00, 0x3B, 0x00, 0x27, 0x00,
      0x60, 0x00, 0x00, 0x03, 0x5C, 0x00, 0x5A, 0x00, 0x58, 0x00, 0x43, 0x00, 0x56, 0x00, 0x42, 0x00,
      0x4E, 0x00, 0x4D, 0x00, 0x2C, 0x00, 0x2E, 0x00, 0x2F, 0x40, 0x00, 0x03, 0xAA, 0x00, 0x00, 0x01,
      0x20, 0x00, 0x00, 0x04, 0x61, 0x00, 0x62, 0x00, 0x63, 0x00, 0x64, 0x00, 0x65, 0x00, 0x66, 0x00,
      0x67, 0x00, 0x68, 0x00, 0x69, 0x00, 0x6A, 0x00, 0x00, 0x08, 0x00, 0x0C, 0x37, 0x80, 0x38, 0x80,
      0x39, 0x80, 0x2D, 0x80, 0x34, 0x80, 0x35, 0x80, 0x36, 0x80, 0x2B, 0x80, 0x31, 0x80, 0x32, 0x80,
      0x33, 0x80, 0x30, 0x80, 0x2E, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x6B, 0x00, 0x6C, 0x00
   };
   unsigned index = ((scancode & 0xFF) - 1)*2;
   if (index >= sizeof(L_400)) return;
   kR1 = kR2; kR2 = L_400[index];
   kR5 = L_400[index+1];

   if ((kR5 & 0x30) == 0x30) zxdata[0] = zxdata[1] = 0xFFFFFFFF;
   static const unsigned char L_511[] = { 0x76, 0x70, 0x74, 0xAD, 0x72, 0xB5, 0x73, 0xAB, 0x77, 0x71, 0x75, 0x78, 0x79 };
   if ((scancode & 0x100) && (kR5 & 0x40)) kR2 |= 0x80;
   if (kR5 & 0x80) {
      if (scancode & 0x100) kR2 = L_511[(scancode & 0xFF) - 0x47];
      else kR2 |= 0x80;
   }
   if (kR5 & 0x0C) kR5 = 0;
}

void ATM_KBD::clear()
{
   zxdata[0] = zxdata[1] = 0xFFFFFFFF;
}

void ATM_KBD::reset()
{
   kR1 = kR2 = mode = lastscan = R7 = 0;
   clear();
}
