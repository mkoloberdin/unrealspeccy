#define gettime(timer)              \
   __asm rdtsc                      \
   __asm mov dword ptr [timer], eax \
   __asm mov dword ptr [timer+4],edx\

__declspec(naked) unsigned __int64 __fastcall rdtsc()
{
   __asm rdtsc
   __asm retn
}


#define VK_ALT VK_MENU

#define WORD4(a,b,c,d) (((unsigned)(a))+((unsigned)(b))*0x100+((unsigned)(c))*0x10000+((unsigned)(d))*0x1000000)
#define WORD2(a,b) ((a)+(b)*0x100)
#define align(a,b) (((unsigned)(a) + ((b)-1)) & ~((b)-1))
const char clrline[] = "\r\t\t\t\t\t\t\t\t\t       \r";

#define savetab(x) {FILE *ff=fopen("tab","wb");fwrite(x,sizeof(x),1,ff);fclose(ff);}

#define hexdigit(a) ((a) < 'A' ? (a)-'0' : toupper(a)-'A'+10)
#define tohex(a) ((a) < 10 ? (a)+'0' : (a)-10+'A')

char nop = 0;
#define nil (&nop)

int ishex(char c) {
   return (isdigit(c) || (tolower(c) >= 'a' && tolower(c) <= 'f'));
}
unsigned char hex(char p) {
   p = tolower(p);
   return (p < 'a') ? p-'0' : p-'a'+10;
}
unsigned char hex(char *p) {
   return 0x10*hex(p[0]) + hex(p[1]);
}

unsigned process_msgs()
{
   MSG msg; unsigned key = 0;
   while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_KEYDOWN || msg.message == WM_SYSKEYDOWN) {
         if (conf.atm.xt_kbd) input.atm51.setkey(msg.lParam >> 16, 1);
         key = msg.wParam;
      }
      if (msg.message == WM_KEYUP || msg.message == WM_SYSKEYUP) {
         if (conf.atm.xt_kbd) input.atm51.setkey(msg.lParam >> 16, 0);
      }
      else if (msg.message == WM_LBUTTONDOWN) mousepos = msg.lParam, key = VK_LMB;
      else if (msg.message == WM_RBUTTONDOWN) mousepos = msg.lParam | 0x80000000, key = VK_RMB;
      else if (msg.message == WM_MOUSEWHEEL) key = (msg.wParam & 0x80000000)? VK_MWD : VK_MWU;
      else DispatchMessage(&msg);
   }
   return key;
}

void eat() // eat messages
{
   Sleep(20); while (process_msgs()) Sleep(10);
}

char dispatch_more(action *table)
{
   if (!table) return -1;
   kbdpc[0] = 0x80; // nil button is always pressed
   while (table->name) {
      if (kbdpc[table->k1] & kbdpc[table->k2] &
          kbdpc[table->k3] & kbdpc[table->k4] & 0x80)
      {
         table->func();
         return 1;
      }
      table++;
   }
   return -1;
}

char dispatch(action *table)
{
   if (!input.readdevices()) return 0;
   dispatch_more(table);
   return 1;
}

bool wcmatch(char *string, char *wc)
{
   for (;;wc++, string++) {
      if (!*string && !*wc) return 1;
      if (*wc == '?') { if (*string) continue; else return 0; }
      if (*wc == '*') {
         for (wc++; *string; string++)
            if (wcmatch(string, wc)) return 1;
         return 0;
      }
      if (tolower(*string) != tolower(*wc)) return 0;
   }
}
