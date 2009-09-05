
#define MAX_CBP 16
unsigned cbp[MAX_CBP][128];
unsigned cbpn;

enum {
   DB_STOP = 0,
   DB_CHAR,
   DB_SHORT,
   DB_PCHAR,
   DB_PSHORT,
   DB_PINT
};

unsigned calcerr;
unsigned calc(unsigned *script)
{
   unsigned stack[64];
   unsigned *sp = stack-1, x;
   while (*script) {
      switch (*script++) {
         case 'M':             *sp = rmdbg(*sp);   break;
         case '!':             *sp = !*sp;     break;
         case '~':             *sp = ~*sp;     break;
         case '+':             *(sp-1) += *sp; goto arith;
         case '-':             *(sp-1) -= *sp; goto arith;
         case '*':             *(sp-1) *= *sp; goto arith;
         case '/':             if (*sp) *(sp-1) /= *sp; goto arith;
         case '%':             if (*sp) *(sp-1) %= *sp; goto arith;
         case '&':             *(sp-1) &= *sp; goto arith;
         case '|':             *(sp-1) |= *sp; goto arith;
         case '^':             *(sp-1) ^= *sp; goto arith;
         case WORD2('-','>'):  *(sp-1) = rmdbg(*sp + sp[-1]); goto arith;
         case WORD2('>','>'):  *(sp-1) >>= *sp;goto arith;
         case WORD2('<','<'):  *(sp-1) <<= *sp;goto arith;
         case WORD2('!','='):  *(sp-1) = (sp[-1]!=*sp);goto arith;
         case '=':
         case WORD2('=','='):  *(sp-1) = (sp[-1]==*sp);goto arith;
         case WORD2('<','='):  *(sp-1) = (sp[-1]<=*sp);goto arith;
         case WORD2('>','='):  *(sp-1) = (sp[-1]>=*sp);goto arith;
         case WORD2('|','|'):  *(sp-1) = (sp[-1]||*sp);goto arith;
         case WORD2('&','&'):  *(sp-1) = (sp[-1]&&*sp);goto arith;
         case '<':             *(sp-1) = (sp[-1]<*sp);goto arith;
         case '>':             *(sp-1) = (sp[-1]>*sp);goto arith;
         arith:         sp--;  break;
         case DB_CHAR:
         case DB_SHORT:        x = *script++; goto push;
         case DB_PCHAR:        x = *(unsigned char*)*script++; goto push;
         case DB_PSHORT:       x = 0xFFFF & *(unsigned*)*script++; goto push;
         case DB_PINT:         x = *(unsigned*)*script++; goto push;
         push:                 *++sp = x; break;
      } // switch (*script)
   } // while
   if (sp != stack) calcerr = 1;
   return *sp;
}

static struct {
   unsigned reg;
   void *ptr;
   unsigned char size;
} regs[] = {

   { WORD4('O','U','T',0), &brk_port_out, 4 },
   { WORD2('I','N'), &brk_port_in, 4 },
   { WORD4('V','A','L',0), &brk_port_val, 1 },
   { WORD2('F','D'), &comp.p7FFD, 1 },

   { WORD4('A','F','\'',0), &cpu.alt.af, 2 },
   { WORD4('B','C','\'',0), &cpu.alt.bc, 2 },
   { WORD4('D','E','\'',0), &cpu.alt.de, 2 },
   { WORD4('H','L','\'',0), &cpu.alt.hl, 2 },
   { WORD2('A','\''), &cpu.alt.a, 1 },
   { WORD2('F','\''), &cpu.alt.f, 1 },
   { WORD2('B','\''), &cpu.alt.b, 1 },
   { WORD2('C','\''), &cpu.alt.c, 1 },
   { WORD2('D','\''), &cpu.alt.d, 1 },
   { WORD2('E','\''), &cpu.alt.e, 1 },
   { WORD2('H','\''), &cpu.alt.h, 1 },
   { WORD2('L','\''), &cpu.alt.l, 1 },

   { WORD2('A','F'), &cpu.af, 2 },
   { WORD2('B','C'), &cpu.bc, 2 },
   { WORD2('D','E'), &cpu.de, 2 },
   { WORD2('H','L'), &cpu.hl, 2 },
   { 'A', &cpu.a, 1 },
   { 'F', &cpu.f, 1 },
   { 'B', &cpu.b, 1 },
   { 'C', &cpu.c, 1 },
   { 'D', &cpu.d, 1 },
   { 'E', &cpu.e, 1 },
   { 'H', &cpu.h, 1 },
   { 'L', &cpu.l, 1 },

   { WORD2('P','C'), &cpu.pc, 2 },
   { WORD2('S','P'), &cpu.sp, 2 },
   { WORD2('I','X'), &cpu.ix, 2 },
   { WORD2('I','Y'), &cpu.iy, 2 },

   { 'I', &cpu.i, 1 },
   { 'R', &cpu.r_low, 1 },
};

unsigned char toscript(char *script, unsigned *dst)
{
   unsigned *d1 = dst;
   static struct {
      unsigned short op;
      unsigned char prior;
   } prio[] = {
      { '(', 10 },
      { ')', 0 },
      { '!', 1 },
      { '~', 1 },
      { 'M', 1 },
      { WORD2('-','>'), 1 },
      { '*', 2 },
      { '%', 2 },
      { '/', 2 },
      { '+', 3 },
      { '-', 3 },
      { WORD2('>','>'), 4 },
      { WORD2('<','<'), 4 },
      { '>', 5 },
      { '<', 5 },
      { '=', 5 },
      { WORD2('>','='), 5 },
      { WORD2('<','='), 5 },
      { WORD2('=','='), 5 },
      { WORD2('!','='), 5 },
      { '&', 6 },
      { '^', 7 },
      { '|', 8 },
      { WORD2('&','&'), 9 },
      { WORD2('|','|'), 10 }
   };

   unsigned sp = 0;
   unsigned stack[128];
   for (char *p = script; *p; p++) if (p[1] != 0x27) *p = toupper(*p);
   while (*script) {
      if (*(unsigned char*)script <= ' ') { script++; continue; }
      if (*script == '\'') { // char
         *dst++ = DB_CHAR;
         *dst++ = script[1];
         if (script[2] != '\'') return 0;
         script += 3; continue;
      }
      if (isalnum(*script) && *script != 'M') {
         unsigned r = -1, p = *(unsigned*)script;
         unsigned ln = 0;
         for (int i = 0; i < sizeof regs / sizeof *regs; i++) {
            unsigned mask = 0xFF; ln = 1;
            if (regs[i].reg & 0xFF00) mask = 0xFFFF, ln = 2;
            if (regs[i].reg & 0xFF0000) mask = 0xFFFFFF, ln = 3;
            if (regs[i].reg == (p & mask)) { r = i; break; }
         }
         if (r != -1) {
            script += ln;
            switch (regs[r].size) {
               case 1: *dst++ = DB_PCHAR; break;
               case 2: *dst++ = DB_PSHORT; break;
               case 4: *dst++ = DB_PINT; break;
               default: printf("BUG01"); exit();
            }
            *dst++ = (unsigned)regs[r].ptr;
         } else { // number
            if (*script > 'F') return 0;
            for (r = 0; isalnum(*script) && *script <= 'F'; script++)
               r = r*0x10 + ((*script >= 'A') ? *script-'A'+10 : *script-'0');
            *dst++ = DB_SHORT;
            *dst++ = r;
         }
         continue;
      }
      // find operation
      unsigned char pr = 0xFF;
      unsigned r = *script++;
      if (strchr("<>=&|-!", (char)r) && strchr("<>=&|", *script))
         r = r + 0x100 * (*script++);
      for (int i = 0; i < sizeof(prio)/sizeof(*prio); i++)
         if (prio[i].op == r) { pr = prio[i].prior; break; }
      if (pr == 0xFF) return 0;
      if (r != '(')
      while (sp && ((stack[sp] >> 16 <= pr) || (r == ')' && (stack[sp] & 0xFF) != '('))) { // get from stack
         *dst++ = stack[sp--] & 0xFFFF;
      }
      if (r == ')') sp--; // del opening bracket
      else stack[++sp] = r+0x10000*pr; // put to stack
      if ((int)sp < 0) return 0; // no opening bracket
   }
   // empty stack
   while (sp) {
      if ((stack[sp] & 0xFF) == '(') return 0; // no closing bracket
      *dst++ = stack[sp--] & 0xFFFF;
   }
   *dst = DB_STOP;
   calcerr = 0; calc(d1);
   return (1-calcerr);
}

void script2text(char *dst, unsigned *src)
{
   char stack[64][0x200], tmp[0x200]; unsigned sp = 0, r;
   while (r = *src++) {
      if (r == DB_CHAR) {
         sprintf(stack[sp++], "'%c'", *src++);
         continue;
      }
      if (r == DB_SHORT) {
         sprintf(stack[sp], "0%X", *src++);
         if (isdigit(stack[sp][1])) strcpy(stack[sp], stack[sp]+1);
         sp++;
         continue;
      }
      if (r >= DB_PCHAR && r <= DB_PINT) {
         for (int i = 0; i < sizeof regs / sizeof *regs; i++)
            if (*src == (unsigned)regs[i].ptr) break;
         *(unsigned*)&(stack[sp++]) = regs[i].reg;
         src++; continue;
      }
      if (r == 'M' || r == '~' || r == '!') { // unary operators
         sprintf(tmp, "%c(%s)", r, stack[sp-1]);
         strcpy(stack[sp-1], tmp);
         continue;
      }
      // else binary operators
      sprintf(tmp, "(%s%s%s)", stack[sp-2], (char*)&r, stack[sp-1]);
      sp--; strcpy(stack[sp-1], tmp);
   }
   strcpy(dst, stack[sp-1]);
}

void SetBpxButtons(HWND dlg)
{
   int focus = -1, text = 0, box = 0;
   HWND focusedWnd = GetFocus();
   if (focusedWnd == GetDlgItem(dlg, IDE_CBP) || focusedWnd == GetDlgItem(dlg, IDC_CBP))
      focus = 0, text = IDE_CBP, box = IDC_CBP;
   if (focusedWnd == GetDlgItem(dlg, IDE_BPX) || focusedWnd == GetDlgItem(dlg, IDC_BPX))
      focus = 1, text = IDE_BPX, box = IDC_BPX;
   if (focusedWnd == GetDlgItem(dlg, IDE_MEM) || focusedWnd == GetDlgItem(dlg, IDC_MEM) ||
       focusedWnd == GetDlgItem(dlg, IDC_MEM_R) || focusedWnd == GetDlgItem(dlg, IDC_MEM_W))
      focus = 2, text = IDE_MEM, box = IDC_MEM;

   SendDlgItemMessage(dlg, IDE_CBP, EM_SETREADONLY, (BOOL)(focus != 0), 0);
   SendDlgItemMessage(dlg, IDE_BPX, EM_SETREADONLY, (BOOL)(focus != 1), 0);
   SendDlgItemMessage(dlg, IDE_MEM, EM_SETREADONLY, (BOOL)(focus != 2), 0);

   int del0 = 0, add0 = 0, del1 = 0, add1 = 0, del2 = 0, add2 = 0;
   unsigned max = SendDlgItemMessage(dlg, box, LB_GETCOUNT, 0, 0),
            cur = SendDlgItemMessage(dlg, box, LB_GETCURSEL, 0, 0),
            len = SendDlgItemMessage(dlg, text, WM_GETTEXTLENGTH, 0, 0);

   if (max && cur >= max) SendDlgItemMessage(dlg, box, LB_SETCURSEL, cur = 0, 0);

   if (focus == 0) { if (len && max < MAX_CBP) add0 = 1; if (cur < max) del0 = 1; }
   if (focus == 1) { if (len) add1 = 1; if (cur < max) del1 = 1; }
   if (focus == 2) {
      if (IsDlgButtonChecked(dlg, IDC_MEM_R) == BST_UNCHECKED && IsDlgButtonChecked(dlg, IDC_MEM_W) == BST_UNCHECKED) len = 0;
      if (len) add2 = 1; if (cur < max) del2 = 1;
   }

   EnableWindow(GetDlgItem(dlg, IDB_CBP_ADD), add0);
   EnableWindow(GetDlgItem(dlg, IDB_CBP_DEL), del0);
   EnableWindow(GetDlgItem(dlg, IDB_BPX_ADD), add1);
   EnableWindow(GetDlgItem(dlg, IDB_BPX_DEL), del1);
   EnableWindow(GetDlgItem(dlg, IDB_MEM_ADD), add2);
   EnableWindow(GetDlgItem(dlg, IDB_MEM_DEL), del2);

   unsigned defid = 0;
   if (add0) defid = IDB_CBP_ADD;
   if (add1) defid = IDB_BPX_ADD;
   if (add2) defid = IDB_MEM_ADD;
   if (defid) SendMessage(dlg, DM_SETDEFID, defid, 0);
}

void ClearListBox(HWND box)
{
   while (SendMessage(box, LB_GETCOUNT, 0, 0))
      SendMessage(box, LB_DELETESTRING, 0, 0);
}

void FillCondBox(HWND dlg, unsigned cursor)
{
   HWND box = GetDlgItem(dlg, IDC_CBP);
   ClearListBox(box);

   for (unsigned i = 0; i < cbpn; i++) {
      char tmp[0x200]; script2text(tmp, cbp[i]);
      SendMessage(box, LB_ADDSTRING, 0, (LPARAM)tmp);
   }
   SendMessage(box, LB_SETCURSEL, cursor, 0);
}

void FillBpxBox(HWND dlg, unsigned address)
{
   HWND box = GetDlgItem(dlg, IDC_BPX);
   ClearListBox(box);
   unsigned selection = 0;

   for (unsigned start = 0; start < 0x10000; ) {
      if (!(membits[start] & MEMBITS_BPX)) { start++; continue; }
      for (unsigned end = start; end < 0xFFFF && (membits[end+1] & MEMBITS_BPX); end++);
      char tmp[16];
      if (start == end) sprintf(tmp, "%04X", start);
      else sprintf(tmp, "%04X-%04X", start, end);
      SendMessage(box, LB_ADDSTRING, 0, (LPARAM)tmp);
      if (start <= address && address <= end)
         selection = SendMessage(box, LB_GETCOUNT, 0, 0);
      start = end+1;
   }
   if (selection) SendMessage(box, LB_SETCURSEL, selection-1, 0);
}

void FillMemBox(HWND dlg, unsigned address)
{
   HWND box = GetDlgItem(dlg, IDC_MEM);
   ClearListBox(box);
   unsigned selection = 0;

   for (unsigned start = 0; start < 0x10000; ) {
      const unsigned char mask = MEMBITS_BPR | MEMBITS_BPW;
      if (!(membits[start] & mask)) { start++; continue; }
      unsigned active = membits[start];
      for (unsigned end = start; end < 0xFFFF && !((active ^ membits[end+1]) & mask); end++);
      char tmp[16];
      if (start == end) sprintf(tmp, "%04X ", start);
      else sprintf(tmp, "%04X-%04X ", start, end);
      if (active & MEMBITS_BPR) strcat(tmp, "R");
      if (active & MEMBITS_BPW) strcat(tmp, "W");
      SendMessage(box, LB_ADDSTRING, 0, (LPARAM)tmp);
      if (start <= address && address <= end)
         selection = SendMessage(box, LB_GETCOUNT, 0, 0);
      start = end+1;
   }
   if (selection) SendMessage(box, LB_SETCURSEL, selection-1, 0);
}

char MoveBpxFromBoxToEdit(HWND dlg, unsigned box, unsigned edit)
{
   HWND hBox = GetDlgItem(dlg, box);
   unsigned max = SendDlgItemMessage(dlg, box, LB_GETCOUNT, 0, 0),
            cur = SendDlgItemMessage(dlg, box, LB_GETCURSEL, 0, 0);
   if (cur >= max) return 0;
   char tmp[0x200];
   SendMessage(hBox, LB_GETTEXT, cur, (LPARAM)tmp);
   if (box == IDC_MEM && *tmp) {
      char *last = tmp + strlen(tmp);
      unsigned r = BST_UNCHECKED, w = BST_UNCHECKED;
      if (last[-1] == 'W') w = BST_CHECKED, last--;
      if (last[-1] == 'R') r = BST_CHECKED, last--;
      if (last[-1] == ' ') last--;
      *last = 0;
      CheckDlgButton(dlg, IDC_MEM_R, r);
      CheckDlgButton(dlg, IDC_MEM_W, w);
   }
   SetDlgItemText(dlg, edit, tmp);
   return 1;
}

struct MEM_RANGE { unsigned start, end; };

int GetMemRamge(char *str, MEM_RANGE &range)
{
   while (*str == ' ') str++;
   for (range.start = 0; ishex(*str); str++)
      range.start = range.start*0x10 + hex(*str);
   if (*str == '-') {
      for (range.end = 0, str++; ishex(*str); str++)
         range.end = range.end*0x10 + hex(*str);
   } else range.end = range.start;
   while (*str == ' ') str++;
   if (*str) return 0;

   if (range.start > 0xFFFF || range.end > 0xFFFF || range.start > range.end) return 0;
   return 1;
}


BOOL CALLBACK conddlg(HWND dlg, UINT msg, WPARAM wp, LPARAM lp)
{
   if (msg == WM_INITDIALOG)
   {
      FillCondBox(dlg, 0);
      FillBpxBox(dlg, 0);
      FillMemBox(dlg, 0);
      SetFocus(GetDlgItem(dlg, IDE_CBP));

set_buttons_and_return:
      SetBpxButtons(dlg);
      return 1;
   }
   if (msg == WM_SYSCOMMAND && wp == SC_CLOSE) EndDialog(dlg, 0);
   if (msg != WM_COMMAND) return 0;

   unsigned id = LOWORD(wp), code = HIWORD(wp); unsigned char mask = 0;
   if (id == IDCANCEL || id == IDOK) EndDialog(dlg, 0);
   char tmp[0x200];

   if (((id == IDE_BPX || id == IDE_CBP || id == IDE_MEM) && (code == EN_SETFOCUS || code == EN_CHANGE)) ||
       ((id == IDC_BPX || id == IDC_CBP || id == IDC_MEM) && code == LBN_SETFOCUS)) goto set_buttons_and_return;

   if (id == IDC_MEM_R || id == IDC_MEM_W) goto set_buttons_and_return;

   if (code == LBN_DBLCLK) {
      if (id == IDC_CBP && MoveBpxFromBoxToEdit(dlg, IDC_CBP, IDE_CBP)) goto del_cond;
      if (id == IDC_BPX && MoveBpxFromBoxToEdit(dlg, IDC_BPX, IDE_BPX)) goto del_bpx;
      if (id == IDC_MEM && MoveBpxFromBoxToEdit(dlg, IDC_MEM, IDE_MEM)) goto del_mem;
   }

   if (id == IDB_CBP_ADD) {
      SendDlgItemMessage(dlg, IDE_CBP, WM_GETTEXT, sizeof tmp, (LPARAM)tmp);
      SetFocus(GetDlgItem(dlg, IDE_CBP));
      if (!toscript(tmp, cbp[cbpn])) {
         MessageBox(dlg, "Error in expression\nPlease do RTFM", 0, MB_ICONERROR);
         return 1;
      }
      SendDlgItemMessage(dlg, IDE_CBP, WM_SETTEXT, 0, 0);
      FillCondBox(dlg, cbpn++);
      goto set_buttons_and_return;
   }

   if (id == IDB_BPX_ADD) {
      SendDlgItemMessage(dlg, IDE_BPX, WM_GETTEXT, sizeof tmp, (LPARAM)tmp);
      SetFocus(GetDlgItem(dlg, IDE_BPX));
      MEM_RANGE range;
      if (!GetMemRamge(tmp, range)) {
         MessageBox(dlg, "Invalid breakpoint address / range", 0, MB_ICONERROR);
         return 1;
      }
      for (unsigned i = range.start; i <= range.end; i++) membits[i] |= MEMBITS_BPX;
      SendDlgItemMessage(dlg, IDE_BPX, WM_SETTEXT, 0, 0);
      FillBpxBox(dlg, range.start);
      goto set_buttons_and_return;
   }

   if (id == IDB_MEM_ADD) {
      SendDlgItemMessage(dlg, IDE_MEM, WM_GETTEXT, sizeof tmp, (LPARAM)tmp);
      SetFocus(GetDlgItem(dlg, IDE_MEM));
      MEM_RANGE range;
      if (!GetMemRamge(tmp, range)) {
         MessageBox(dlg, "Invalid watch address / range", 0, MB_ICONERROR);
         return 1;
      }
      unsigned char mask = 0;
      if (IsDlgButtonChecked(dlg, IDC_MEM_R) == BST_CHECKED) mask |= MEMBITS_BPR;
      if (IsDlgButtonChecked(dlg, IDC_MEM_W) == BST_CHECKED) mask |= MEMBITS_BPW;
      for (unsigned i = range.start; i <= range.end; i++) membits[i] |= mask;
      SendDlgItemMessage(dlg, IDE_MEM, WM_SETTEXT, 0, 0);
      CheckDlgButton(dlg, IDC_MEM_R, BST_UNCHECKED);
      CheckDlgButton(dlg, IDC_MEM_W, BST_UNCHECKED);
      FillMemBox(dlg, range.start);
      goto set_buttons_and_return;
   }

   if (id == IDB_CBP_DEL) {
del_cond:
      SetFocus(GetDlgItem(dlg, IDE_CBP));
      unsigned cur = SendDlgItemMessage(dlg, IDC_CBP, LB_GETCURSEL, 0, 0);
      if (cur >= cbpn) return 0;
      cbpn--; memcpy(cbp[cur], cbp[cur+1], sizeof(cbp[0])*(cbpn-cur));

      if (cur && cur == cbpn) cur--;
      FillCondBox(dlg, cur);
      goto set_buttons_and_return;
   }

   if (id == IDB_BPX_DEL) {
del_bpx:
      SetFocus(GetDlgItem(dlg, IDE_BPX));
      id = IDC_BPX; mask = ~MEMBITS_BPX;
del_range:
      unsigned cur = SendDlgItemMessage(dlg, id, LB_GETCURSEL, 0, 0),
               max = SendDlgItemMessage(dlg, id, LB_GETCOUNT, 0, 0);
      if (cur >= max) return 0;
      SendDlgItemMessage(dlg, id, LB_GETTEXT, cur, (LPARAM)tmp);
      unsigned start, end;
      sscanf(tmp, "%X", &start);
      if (tmp[4] == '-') sscanf(tmp+5, "%X", &end); else end = start;
      for (unsigned i = start; i <= end; i++) membits[i] &= mask;
      if (id == IDC_BPX) FillBpxBox(dlg, 0); else FillMemBox(dlg, 0);
      if (cur && cur == max) cur--;
      SendDlgItemMessage(dlg, id, LB_SETCURSEL, cur, 0);
      goto set_buttons_and_return;
   }

   if (id == IDB_MEM_DEL) {
del_mem:
      SetFocus(GetDlgItem(dlg, IDE_MEM));
      id = IDC_MEM; mask = ~(MEMBITS_BPR | MEMBITS_BPW);
      goto del_range;
   }

   return 0;
}

void mon_bpdialog()
{
   DialogBox(hIn, MAKEINTRESOURCE(IDD_COND), wnd, conddlg);
}

unsigned char isbrk() // is there breakpoints active ?
{
   if (cbpn || conf.mem_model == MM_PROFSCORP) return 1; // breakpoint on read ROM switches ROM bank
   unsigned char res = 0;
   for (int i = 0; i < 0x10000; i++) res |= membits[i];
   return (res & (MEMBITS_BPR | MEMBITS_BPW | MEMBITS_BPX));
}

BOOL CALLBACK watchdlg(HWND dlg, UINT msg, WPARAM wp, LPARAM lp)
{
   char tmp[0x200]; unsigned i;
   static const ids1[] = { IDC_W1_ON, IDC_W2_ON, IDC_W3_ON, IDC_W4_ON };
   static const ids2[] = { IDE_W1, IDE_W2, IDE_W3, IDE_W4 };
   if (msg == WM_INITDIALOG) {
      for (i = 0; i < 4; i++) {
         CheckDlgButton(dlg, ids1[i], watch_enabled[i] ? BST_CHECKED : BST_UNCHECKED);
         script2text(tmp, watch_script[i]); SetWindowText(GetDlgItem(dlg, ids2[i]), tmp);
      }
      CheckDlgButton(dlg, IDC_TR_RAM, trace_ram ? BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(dlg, IDC_TR_ROM, trace_ram ? BST_CHECKED : BST_UNCHECKED);
reinit:
      for (i = 0; i < 4; i++)
         EnableWindow(GetDlgItem(dlg, ids2[i]), watch_enabled[i]);
      return 1;
   }
   if (msg == WM_COMMAND && (LOWORD(wp)==ids1[0] || LOWORD(wp)==ids1[1] || LOWORD(wp)==ids1[2] || LOWORD(wp)==ids1[3])) {
      for (i = 0; i < 4; i++)
         watch_enabled[i] = IsDlgButtonChecked(dlg, ids1[i]) == BST_CHECKED;
      goto reinit;
   }
   if ((msg == WM_SYSCOMMAND && wp == SC_CLOSE) || (msg == WM_COMMAND && LOWORD(wp) == IDCANCEL)) {
      trace_ram = IsDlgButtonChecked(dlg, IDC_TR_RAM) == BST_CHECKED;
      trace_rom = IsDlgButtonChecked(dlg, IDC_TR_ROM) == BST_CHECKED;
      for (i = 0; i < 4; i++)
         if (watch_enabled[i]) {
            SendDlgItemMessage(dlg, ids2[i], WM_GETTEXT, sizeof tmp, (LPARAM)tmp);
            if (!toscript(tmp, watch_script[i])) {
               sprintf(tmp, "Watch %d: error in expression\nPlease do RTFM", i+1);
               MessageBox(dlg, tmp, 0, MB_ICONERROR); watch_enabled[i] = 0;
               SetFocus(GetDlgItem(dlg, ids2[i]));
               return 0;
            }
         }
      EndDialog(dlg, 0);
   }
   return 0;
}

void mon_watchdialog()
{
   DialogBox(hIn, MAKEINTRESOURCE(IDD_OSW), wnd, watchdlg);
}
