
void show_time()
{
   tprint(time_x, time_y, "time delta:", W_OTHEROFF);
   char text[32]; sprintf(text, "%14I64d", (__int64)(comp.t_states + cpu.t - debug_last_t));
   tprint(time_x+11, time_y, text, W_OTHER);
   tprint(time_x+25, time_y, "t", W_OTHEROFF);
   frame(time_x, time_y, 26, 1, FRAME);
}

void wtline(char *name, unsigned ptr, unsigned y)
{
   char line[40]; sprintf(line, name?"%3s: ":"%04X ", name?name:(char*)ptr);
   for (unsigned dx = 0; dx < 8; dx++) {
      unsigned char c = rmdbg(ptr++);
      sprintf(line+5+3*dx, "%02X", c); line[7+3*dx] = ' ';
      line[29+dx] = c ? c : '.';
   }
   line[37] = 0; tprint(wat_x, wat_y+y, line, W_OTHER);
}

void showwatch()
{
   if (show_scrshot) {
      for (unsigned y = 0; y < wat_sz; y++)
         for (unsigned x = 0; x < 37; x++)
            txtscr[80*30 +  (wat_y+y)*80 + (wat_x+x)] = 0xFF;
   } else {
      wtline("PC", cpu.pc, 0);
      wtline("SP", cpu.sp, 1);
      wtline("BC", cpu.bc, 2);
      wtline("DE", cpu.de, 3);
      wtline("HL", cpu.hl, 4);
      wtline("IX", cpu.ix, 5);
      wtline("IY", cpu.iy, 6);
      wtline("BC'", cpu.alt.bc, 7);
      wtline("DE'", cpu.alt.de, 8);
      wtline("HL'", cpu.alt.hl, 9);
      wtline(0, user_watches[0], 10);
      wtline(0, user_watches[1], 11);
      wtline(0, user_watches[2], 12);
   }
   char *text = "watches";
   if (show_scrshot == 1) text = "screen memory";
   if (show_scrshot == 2) text = "ray-painted";
   tprint(wat_x, wat_y-1, text, W_TITLE);
   frame(wat_x,wat_y,37,wat_sz,FRAME);
}

void mon_setwatch()
{
   if (show_scrshot) show_scrshot = 0;
   for (unsigned i = 0; i < 3; i++) {
      debugscr();
      unsigned addr = input4(wat_x, wat_y+wat_sz-3+i, user_watches[i]);
      if (addr == -1) return;
      user_watches[i] = addr;
   }
}

void showstack()
{
   for (unsigned i = 0; i < stack_size; i++) {
      char xx[10]; //-2:1234
                   //SP:1234
                   //+2:
      if (!i) *(unsigned*)xx = WORD2('-','2');
      else if (i==1) *(unsigned*)xx = WORD2('S','P');
      else sprintf(xx, (i > 8) ? "%X" : "+%X", (i-1)*2);
      sprintf(xx+2, ":%02X%02X", rmdbg(cpu.sp+(i-1)*2+1), rmdbg(cpu.sp+(i-1)*2));
      tprint(stack_x, stack_y+i, xx, W_OTHER);
   }
   tprint(stack_x, stack_y-1, "stack", W_TITLE);
   frame(stack_x, stack_y, 7, stack_size, FRAME);
}

void show_ay()
{
   if (!conf.sound.ay_scheme) return;
   char *ayn = comp.active_ay? "AY1" : "AY0";
   if (conf.sound.ay_scheme < AY::SCHEME_QUADRO) ayn = "AY:", comp.active_ay = 0;
   tprint(ay_x-3, ay_y, ayn, W_TITLE);
   AY *ay = comp.ay + comp.active_ay;
   char line[32];
   for (int i = 0; i < 16; i++) {
      line[0] = "0123456789ABCDEF"[i]; line[1] = 0;
      tprint(ay_x + i*3, ay_y, line, W_AYNUM);
      sprintf(line, "%02X", ay->reg[i]);
      tprint(ay_x + i*3 + 1, ay_y, line, i == ay->activereg ? W_AYON : W_AYOFF);
   }
   frame(ay_x, ay_y, 48, 1, FRAME);
}

void mon_switchay()
{
   comp.active_ay ^= 1;
}

void showbanks()
{
   for (int i = 0; i < 4; i++) {
      char ln[64]; sprintf(ln, "%d:", i);
      tprint(banks_x, banks_y+i+1, ln, W_OTHEROFF);
      strcpy(ln, "?????");
      if (bankr[i] < RAM_BASE_M+MAX_RAM_PAGES*PAGE) sprintf(ln, "RAM%2X", (bankr[i] - RAM_BASE_M)/PAGE);
      if ((unsigned)(bankr[i] - ROM_BASE_M) < PAGE*MAX_ROM_PAGES) sprintf(ln, "ROM%2X", (bankr[i] - ROM_BASE_M)/PAGE);
      if (bankr[i] == base_sos_rom) strcpy(ln, "BASIC");
      if (bankr[i] == base_dos_rom) strcpy(ln, "TRDOS");
      if (bankr[i] == base_128_rom) strcpy(ln, "B128K");
      if (bankr[i] == base_sys_rom) strcpy(ln, "SERVM");
      if (bankr[i] == CACHE_M) strcpy(ln, (conf.cache!=32)?"CACHE":"CACH0");
      if (bankr[i] == CACHE_M+PAGE) strcpy(ln, "CACH1");
      tprint(banks_x+2, banks_y+i+1, ln, bankr[i]!=bankw[i] ? W_BANKRO : W_BANK);
   }
   frame(banks_x, banks_y+1, 7, 4, FRAME);
   tprint(banks_x, banks_y, "pages", W_TITLE);
}

void showports()
{
   char ln[64];
   sprintf(ln, "  FE:%02X", comp.pFE); tprint(ports_x, ports_y, ln, W_OTHER);
   sprintf(ln, "7FFD:%02X", comp.p7FFD); tprint(ports_x, ports_y+1, ln, W_OTHER);

   switch (conf.mem_model) {
      case MM_KAY:
      case MM_SCORP:
      case MM_PROFSCORP:
         dbg_extport = 0x1FFD; dgb_extval = comp.p1FFD; break;
      case MM_PROFI:
         dbg_extport = 0xDFFD; dgb_extval = comp.pDFFD; break;
      case MM_ATM450:
         dbg_extport = 0xFDFD; dgb_extval = comp.pFDFD; break;
      case MM_ATM710:
         dbg_extport = (comp.aFF77 & 0xFFFF); dgb_extval = comp.pFF77; break;
      default:
         dbg_extport = 0;
   }
   if (dbg_extport) sprintf(ln, "%04X:%02X", dbg_extport, dgb_extval);
   else sprintf(ln, "cmos:%02X", comp.cmos_addr);
   tprint(ports_x, ports_y+2, ln, W_OTHER);

   sprintf(ln, "EFF7:%02X", comp.pEFF7); tprint(ports_x, ports_y+3, ln, W_OTHER);
   frame(ports_x, ports_y, 7, 4, FRAME); tprint(ports_x, ports_y-1, "ports", W_TITLE);
}

void showdos()
{
//    CD:802E
//    STAT:24
//    SECT:00
//    T:00/01
//    S:00/00
   if (conf.trdos_present) comp.wd.process();
   char ln[64]; unsigned char atr = conf.trdos_present ? W_OTHER : W_OTHEROFF;
   sprintf(ln, "CD:%02X%02X", comp.wd.cmd, comp.wd.data);
   tprint(dos_x, dos_y, ln, atr);
   sprintf(ln, "STAT:%02X", comp.wd.status);
   tprint(dos_x, dos_y+1, ln, atr);
   sprintf(ln, "SECT:%02X", comp.wd.sector);
   tprint(dos_x, dos_y+2, ln, atr);
   sprintf(ln, "T:%02X/%02X", comp.wd.seldrive->track, comp.wd.track);
   tprint(dos_x, dos_y+3, ln, atr);
   sprintf(ln, "S:%02X/%02X", comp.wd.system, comp.wd.rqs);
   tprint(dos_x, dos_y+4, ln, atr);
   frame(dos_x, dos_y, 7, 5, FRAME);
#if 1
   tprint(dos_x, dos_y-1, "beta128", W_TITLE);
#else
   sprintf(ln, "%X-%X %d", comp.wd.state, comp.wd.state2, comp.wd.seldrive->track);
   tprint(dos_x,dos_y-1, ln, atr);
#endif
/*
//    STAT:00101010
//    CMD:80,STA:2A
//    DAT:22,SYS:EE
//    TRK:31,SEC:01
//    DISK:A,SIDE:0

   char ln[64]; unsigned char atr = conf.trdos_present ? 0x20 : 0x27;
   sprintf(ln, "STAT:00000000"); unsigned char stat = in_trdos(0x1F);
   for (int i = 0; i < 7; i++) ln[i+5] = (stat & (0x80 >> i)) ? '1':'0';
   tprint(dos_x, dos_y+1, ln, atr);
   sprintf(ln, "CMD:%02X,STA:%02X", comp.trdos.cmd, stat);
   tprint(dos_x, dos_y+2, ln, atr);
   sprintf(ln, "DAT:%02X,SYS:%02X", comp.trdos.data, in_trdos(0xFF));
   tprint(dos_x, dos_y+3, ln, atr);
   sprintf(ln, "TRK:%02X,SEC:%02X", comp.trdos.track, comp.trdos.sector);
   tprint(dos_x, dos_y+4, ln, atr);
   sprintf(ln, "DISK:%c,SIDE:%c", 'A'+(comp.trdos.system & 3), (comp.trdos.system & 0x10) ? '0':'1');
   tprint(dos_x, dos_y+5, ln, atr);
   frame(dos_x, dos_y+1, 13, 5, FRAME);
   tprint(dos_x, dos_y, "beta128", 0x83);
*/
}

#ifdef MOD_GSBASS
BOOL CALLBACK gsdlg(HWND dlg, UINT msg, WPARAM wp, LPARAM lp)
{
   char tmp[0x200];
   if (msg == WM_INITDIALOG) {
repaint:
      while (SendDlgItemMessage(dlg, IDC_GSLIST, LB_GETCOUNT, 0, 0))
         SendDlgItemMessage(dlg, IDC_GSLIST, LB_DELETESTRING, 0, 0);
      if (gs.modsize) {
         sprintf(tmp, "%-.20s (%s)", gs.mod, gs.mod_playing ? "playing" : "stopped");
         SendDlgItemMessage(dlg, IDC_GSLIST, LB_ADDSTRING, 0, (LPARAM)tmp);
      }
      for (unsigned i = 1; i < gs.total_fx; i++) {
         sprintf(tmp, "%csmp %d: v=%d, n=%d, %d%s",
            gs.cur_fx == i ? '*':' ', i,
            gs.sample[i].volume, gs.sample[i].note, gs.sample[i].end,
            gs.sample[i].loop < gs.sample[i].end ? " (L)":nil);
         SendDlgItemMessage(dlg, IDC_GSLIST, LB_ADDSTRING, 0, (LPARAM)tmp);
      }
      *tmp = 0; for (i = 0; i < 0x100; i++) {
         if (gs.badgs[i]) sprintf(tmp+strlen(tmp), "%02X ", i);
      }
      SendDlgItemMessage(dlg, IDE_GS, WM_SETTEXT, 0, (LPARAM)tmp);
      return 1;
   }
   if (msg == WM_SYSCOMMAND && wp == SC_CLOSE) EndDialog(dlg, 0);
   if (msg != WM_COMMAND) return 0;
   unsigned id = LOWORD(wp);
   if (id == IDCANCEL || id == IDOK) EndDialog(dlg, 0);
   if (id == IDB_GS_CLEAR) { memset(gs.badgs, 0, sizeof gs.badgs); SendDlgItemMessage(dlg, IDE_GS, WM_SETTEXT, 0, 0); }
   if (id == IDB_GS_RESET) { gs.reset(); goto repaint; }
   if (id == IDB_GS_PLAY || (id == IDC_GSLIST && HIWORD(wp) == LBN_DBLCLK)) {
      unsigned i = SendDlgItemMessage(dlg, IDC_GSLIST, LB_GETCURSEL, 0, 0);
      if (i > 0x100) return 1;
      if (!i && gs.modsize) {
         gs.mod_playing ^= 1;
         if (gs.mod_playing) gs.restart_mod(0); else gs.stop_mod();
         goto repaint;
      }
      if (!gs.modsize) i++;
      GSHLE::CHANNEL ch = { 0 };
      ch.volume = 64; ch.ptr = 0;
      ch.start = gs.sample[i].start;
      ch.loop = gs.sample[i].loop;
      ch.end = gs.sample[i].end;
      ch.delta = gs.note2rate[gs.sample[i].note];
      unsigned t0 = GetTickCount();
      sound_play();
      while (ch.start && (gs.sample[i].loop > gs.sample[i].end || GetTickCount() < t0+5000)) {
         gs.mixchannel(&ch, 0);
         unsigned save = min(ch.sound_state.ready_samples(), temp.snd_frame_ticks);
         SOUND_STREAM::copy_to_sndplaybuf(save);
         do_sound();
      }
      memset(sndplaybuf, 0, spbsize = temp.snd_frame_samples*4);
      for (i = 0; i <= 10; i++) do_sound();
      sound_stop();
   }
   return 0;
}

void mon_gsdialog() {
   if (conf.gs_type == 2)
      DialogBox(hIn, MAKEINTRESOURCE(IDD_GS), wnd, gsdlg);
   else MessageBox(wnd, "high-level GS emulation\nis not initialized", 0, MB_OK | MB_ICONERROR);
}
#else
void mon_gsdialog() {}
#endif
