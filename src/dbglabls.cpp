
struct MON_LABEL { unsigned char *address; unsigned name_offs; };

struct MON_LABELS
{
   MON_LABEL *pairs;
   unsigned n_pairs;
   char *names;
   unsigned names_size;

   MON_LABELS() { pairs = 0, names = 0, n_pairs = names_size = 0; hNewUserLabels = 0; }
   ~MON_LABELS() { free(pairs), free(names); stop_watching_labels(); }

   unsigned add_name(char *name);
   void clear(unsigned char *start, unsigned size);
   void clear_ram() { clear(RAM_BASE_M, MAX_RAM_PAGES*PAGE); }
   void sort();

   char *find(unsigned char *address);
   void add(unsigned char *address, char *name);
   unsigned load(char *filename, unsigned char *base, unsigned size);

   void import_alasm();
   void import_xas();
   void import_file();

   HANDLE hNewUserLabels;
   char userfile[0x200];
   void stop_watching_labels();
   void start_watching_labels();
   void notify_user_labels();

} mon_labels;

void MON_LABELS::start_watching_labels()
{
   addpath(userfile, "?");
   hNewUserLabels = FindFirstChangeNotification(userfile, 0, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE);
   addpath(userfile, "user.l");
}

void MON_LABELS::stop_watching_labels()
{
   if (!hNewUserLabels || hNewUserLabels == INVALID_HANDLE_VALUE) return;
   CloseHandle(hNewUserLabels);
   hNewUserLabels = INVALID_HANDLE_VALUE;
}

unsigned MON_LABELS::add_name(char *name)
{
   unsigned len = strlen(name)+1, new_size = names_size + len;
   if (new_size > align_by(names_size, 4096))
      names = (char*)realloc(names, align_by(new_size, 4096));
   unsigned result = names_size;
   memcpy(names + result, name, len);
   names_size = new_size;
   return result;
}

void MON_LABELS::clear(unsigned char *start, unsigned size)
{
   unsigned dst = 0;
   for (unsigned src = 0; src < n_pairs; src++)
      if ((unsigned)(pairs[src].address - start) > size)
         pairs[dst++] = pairs[src];
   n_pairs = dst;
   // pack `names'
   char *pnames = names; names = 0; names_size = 0;
   for (unsigned l = 0; l < n_pairs; l++)
      pairs[l].name_offs = add_name(pnames + pairs[l].name_offs);
   free(pnames);
}

int __cdecl labels_sort_func(const void *e1, const void *e2)
{
   const MON_LABEL *a = (MON_LABEL*)e1, *b = (MON_LABEL*)e2;
   return a->address - b->address;
}

void MON_LABELS::sort()
{
   qsort(pairs, n_pairs, sizeof(MON_LABEL), labels_sort_func);
}

void MON_LABELS::add(unsigned char *address, char *name)
{
   if (n_pairs >= align_by(n_pairs, 1024))
      pairs = (MON_LABEL*)realloc(pairs, sizeof(MON_LABEL) * align_by(n_pairs+1, 1024));
   pairs[n_pairs].address = address;
   pairs[n_pairs].name_offs = add_name(name);
   n_pairs++;
}

char *MON_LABELS::find(unsigned char *address)
{
   unsigned l = 0, r = n_pairs;
   for (;;) {
      if (l >= r) return 0;
      unsigned m = (l+r)/2;
      if (pairs[m].address == address) return names + pairs[m].name_offs;
      if (pairs[m].address < address) l = m+1; else r = m;
   }
}

unsigned MON_LABELS::load(char *filename, unsigned char *base, unsigned size)
{
   FILE *in = fopen(filename, "rt");
   if (!in) { errmsg("can't find label file %s", filename); return 0; }
   clear(base, size);
   unsigned l_counter = 0, loaded = 0; char *txt = 0;
   while (!feof(in)) {
      char line[64];
      if (!fgets(line, sizeof(line), in)) break;
      l_counter++;
      for (int l = strlen(line); l && line[l-1] <= ' '; l--); line[l] = 0;
      if (!l) continue;
      unsigned val = 0, offset = 0;
      if (l >= 6 && line[4] == ' ') {
         for (l = 0; l < 4; l++) {
            if (!ishex(line[l])) goto ll_err;
            val = (val * 0x10) + hex(line[l]);
         }
         txt = line+5;
      } else if (l >= 9 && line[2] == ':' && line[7] == ' ') {
         for (l = 0; l < 2; l++) {
            if (!ishex(line[l])) goto ll_err;
            val = (val * 0x10) + hex(line[l]);
         }
         for (l = 3; l < 7; l++) {
            if (!ishex(line[l])) goto ll_err;
            offset = (offset * 0x10) + hex(line[l]);
         }
         val = val*PAGE + offset;
         txt = line+8;
      } else {
   ll_err:
         color(CONSCLR_ERROR);
         printf("error in %s, line %d\n", filename, l_counter);
         continue;
      }

      if (val < size) add(base+val, txt), loaded++;
   }
   fclose(in);
   sort();
   return loaded;
}

void MON_LABELS::import_alasm()
{
   static const char caption[] = "Alasm labels import";
   const char *err = "STS 5.7 not found in bank #07";
   unsigned sts = 0;
   if ((conf.mem_model == MM_PENTAGON || conf.mem_model == MM_MYPENT) && conf.ramsize > 128) {
      err = "STS 5.7 not found in banks #07,#47";
      if (*(unsigned*)(RAM_BASE_M+PAGE*15+0x3E84) == WORD4(0x7D,0xE6,0x18,0xF6) && RAM_BASE_M[PAGE*15+0x3E7B] == 0x21) sts = PAGE*15;
   }
   if (!sts && *(unsigned*)(RAM_BASE_M+PAGE*7+0x3E84) == WORD4(0x7D,0xE6,0x18,0xF6) && RAM_BASE_M[PAGE*7+0x3E7B] == 0x21) sts = PAGE*7;
   if (!sts) { MessageBox(GetForegroundWindow(), err, caption, MB_OK | MB_ICONERROR); return; }
   unsigned offset = *(unsigned short*)(RAM_BASE_M+sts+0x3E7C);
   if (offset < 0xC000) { MessageBox(GetForegroundWindow(), "start of labels not found", caption, MB_OK | MB_ICONERROR); return; }
   clear_ram();
   unsigned char bank = RAM_BASE_M[sts+0x3E88]; bank = ((bank & 7) + ((bank & 0xC0) >> 3) + (bank & 0x20)) & temp.ram_mask;
   unsigned char *base = RAM_BASE_M + PAGE*bank; offset &= 0x3FFF;
   unsigned count = 0;
   while (offset < 0x3FFC) { // #FE00/FF00/FFFC - end of labels?
      unsigned char sz = base[offset]; if (!sz) break;
      if ((sz & 0x3F) < 6) { MessageBox(GetForegroundWindow(), "unexpected end of labels", caption, MB_OK | MB_ICONERROR); sts = 0; break; }
      unsigned end = offset + (sz & 0x3F);
      if (!(sz & 0xC0)) {
         char lbl[64]; unsigned ptr = 0;
         for (unsigned k = sz; k > 5;) k--, lbl[ptr++] = base[offset+k]; lbl[ptr] = 0;
         unsigned val = *(unsigned short*)(base+offset+1);
         unsigned char *bs;
         switch (val & 0xC000) {
            case 0x4000: bs = RAM_BASE_M+5*PAGE; break;
            case 0x8000: bs = RAM_BASE_M+2*PAGE; break;
            case 0xC000: bs = RAM_BASE_M+0*PAGE; break;
            default: bs = 0;
         }
         if (bs) add(bs+(val & 0x3FFF), lbl), count++;
      }
      offset = end;
   }
   if (sts) {
      char ln[64]; sprintf(ln, "found %d labels in page #%02X", count, RAM_BASE_M[sts+0x3E88]);
      MessageBox(GetForegroundWindow(), ln, caption, MB_OK | MB_ICONINFORMATION);
   }
   sort();
}

void MON_LABELS::import_xas()
{
   static const char caption[] = "XAS7 labels import";
   const char *err = "labels not found in bank #06";
   unsigned sts = 0;
   if ((conf.mem_model == MM_PENTAGON || conf.mem_model == MM_MYPENT) && conf.ramsize > 128) {
      err = "labels not found in banks #06,#46";
      if (RAM_BASE_M[PAGE*14+0x3FFF] == 5 && RAM_BASE_M[PAGE*14+0x1FFF] == 5) sts = PAGE*14;
   }
   if (!sts && RAM_BASE_M[PAGE*6+0x3FFF] == 5 && RAM_BASE_M[PAGE*6+0x1FFF] == 5) sts = PAGE*6;
   if (!sts) { MessageBox(GetForegroundWindow(), err, caption, MB_OK | MB_ICONERROR); return; }

   clear_ram(); unsigned count = 0;
   for (int k = 0; k < 2; k++) {
      unsigned char *ptr = RAM_BASE_M + sts + (k? 0x3FFD : 0x1FFD);
      for (;;) {
         if (ptr[2] < 5 || (ptr[2] & 0x80)) break;
         char lbl[16]; for (int i = 0; i < 7; i++) lbl[i] = ptr[i-7];
         for (i = 7; i && lbl[i-1]==' '; i--); lbl[i] = 0;
         unsigned val = *(unsigned short*)ptr;
         unsigned char *bs;
         switch (val & 0xC000) {
            case 0x4000: bs = RAM_BASE_M+5*PAGE; break;
            case 0x8000: bs = RAM_BASE_M+2*PAGE; break;
            case 0xC000: bs = RAM_BASE_M+0*PAGE; break;
            default: bs = 0;
         }
         if (bs) add(bs+(val & 0x3FFF), lbl), count++;
         ptr -= 9; if (ptr < RAM_BASE_M+sts+9) break;
      }
   }
   if (sts) {
      char ln[64]; sprintf(ln, "found %d labels in page #%02X", count, sts==PAGE*6? 0x06 : 0x46);
      MessageBox(GetForegroundWindow(), ln, caption, MB_OK | MB_ICONINFORMATION);
   }
   sort();
}

void MON_LABELS::import_file()
{
   FILE *ff = fopen(userfile, "rb"); if (!ff) return; fclose(ff);
   unsigned count = load(userfile, RAM_BASE_M, conf.ramsize * 1024);
   if (!count) return;
   char tmp[0x200]; sprintf(tmp, "loaded %d labels from\r\n%s", count, userfile);
   MessageBox(GetForegroundWindow(), tmp, "unreal discovered changes in user labels", MB_OK | MB_ICONINFORMATION);
}

void MON_LABELS::notify_user_labels()
{
   if (hNewUserLabels == INVALID_HANDLE_VALUE) return;
   // load labels at first check
   if (hNewUserLabels == NULL) { start_watching_labels(); import_file(); return; }

   if (WaitForSingleObject(hNewUserLabels, 0) != WAIT_OBJECT_0) return;

   import_file();
   FindNextChangeNotification(hNewUserLabels);
}

void load_labels(char *filename, unsigned char *base, unsigned size)
{
   mon_labels.load(filename, base, size);
}

char curlabel[64]; unsigned lcount;

void ShowLabels()
{
   SetDlgItemText(dlg, IDC_LABEL_TEXT, curlabel);
   HWND list = GetDlgItem(dlg, IDC_LABELS);

   while (SendMessage(list, LB_GETCOUNT, 0, 0))
      SendMessage(list, LB_DELETESTRING, 0, 0);

   unsigned ln = strlen(curlabel); lcount = 0;
   for (unsigned p = 0; p < 4; p++) {
      unsigned char *base = am_r(p*0x4000);
      for (unsigned i = 0; i < mon_labels.n_pairs; i++) {
         unsigned char *label = mon_labels.pairs[i].address;
         if ((unsigned)(label - base) >= 0x4000) continue;
         char *name = mon_labels.pairs[i].name_offs + mon_labels.names;
         if (ln) {
            // unfortunately, strstr() is case sensitive, use loop
            for (char *s = name; *s; s++)
               if (!strnicmp(s, curlabel, ln)) break;
            if (!*s) continue;
         }
         char zz[0x400]; sprintf(zz, "%04X %s", (label - base) + (p * 0x4000), name);
         SendMessage(list, LB_ADDSTRING, 0, (LPARAM)zz); lcount++;
      }
   }
   SendMessage(list, LB_SETCURSEL, 0, 0);
   SetFocus(list);
}

BOOL CALLBACK LabelsDlg(HWND dlg, UINT msg, WPARAM wp, LPARAM lp)
{
   ::dlg = dlg;
   if (msg == WM_INITDIALOG)
   {
      *curlabel = 0;
      ShowLabels();
      return 1;
   }

   if (msg == WM_SYSCOMMAND && wp == SC_CLOSE) EndDialog(dlg, 0);

   if (msg == WM_VKEYTOITEM)
   {
      unsigned sz = strlen(curlabel);
      wp = LOWORD(wp);
      if (wp == VK_BACK) {
         if (sz) curlabel[sz-1] = 0, ShowLabels();
         else { deadkey: Beep(300, 100); }
      } else if ((unsigned)(wp-'0') < 10 || (unsigned)(wp-'A') < 26 || wp == '_') {
         if (sz == sizeof(curlabel)-1) goto deadkey;
         curlabel[sz] = wp, curlabel[sz+1] = 0, ShowLabels();
         if (!lcount) { curlabel[sz] = 0, ShowLabels(); goto deadkey; }
      } else return -1;
      return -2;
   }

   if (msg != WM_COMMAND) return 0;

   unsigned id = LOWORD(wp), code = HIWORD(wp);
   if (id == IDCANCEL || id == IDOK) EndDialog(dlg, 0);

   if (id == IDOK || (id == IDC_LABELS && code == LBN_DBLCLK))
   {
      HWND list = GetDlgItem(dlg, IDC_LABELS);
      unsigned n = SendMessage(list, LB_GETCURSEL, 0, 0);
      if (n >= lcount) return 0;
      char zz[0x400]; SendMessage(list, LB_GETTEXT, n, (LPARAM)zz);
      unsigned address; sscanf(zz, "%X", &address);

      void push_pos(); push_pos();
      trace_curs = trace_top = address; activedbg = WNDTRACE;

      EndDialog(dlg, 1);
      return 1;
   }

   return 0;
}

void mon_show_labels()
{
   DialogBox(hIn, MAKEINTRESOURCE(IDD_LABELS), wnd, LabelsDlg);
}
