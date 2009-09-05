char load_errors;

void loadkeys(action*);
void loadzxkeys(CONFIG*);
void load_arch(char*);

unsigned load_rom(char *path, unsigned char *bank, unsigned max_banks = 1)
{
   if (!*path) { norom: memset(bank, 0xFF, max_banks*PAGE); return 0; }
   char tmp[0x200]; strcpy(tmp, path);
   char *x = strrchr(tmp+2, ':');
   unsigned page = 0;
   if (x) { *x = 0; page = atoi(x+1); }
   if (max_banks == 16) page *= 16; // bank for scorp prof.rom

   FILE *ff = fopen(tmp, "rb");
   if (!ff) { printf("error: ROM file %s not found\n", tmp); err: load_errors = 1; goto norom; }
   if (fseek(ff, page*PAGE, SEEK_SET)) { badrom: fclose(ff); printf("Incorrect ROM file: %s\n", path); goto err; }

   unsigned size = fread(bank, 1, max_banks*PAGE, ff);
   if (!size || (size & (PAGE-1))) goto badrom;

   fclose(ff);
   return size / 1024;
}

void load_atm_font()
{
   FILE *ff = fopen("SGEN.ROM", "rb");
   if (!ff) return;
   unsigned char font[0x800];
   unsigned sz = fread(font, 1, 0x800, ff);
   if (sz == 0x800) {
      printf("using ATM font from external SGEN.ROM\n");
      for (unsigned chr = 0; chr < 0x100; chr++)
         for (unsigned l = 0; l < 8; l++)
            fontatm2[chr+l*0x100] = font[chr*8+l];
   }
   fclose(ff);
}

void addpath(char *dst, char *fname = 0)
{
   if (!fname) fname = dst;
   else strcpy(dst, fname);
   if (!*fname) return; // empty filenames have special meaning
   if (fname[1] == ':') return; // already full name
   char tmp[0x200];
   GetModuleFileName(0, tmp, sizeof tmp);
   char *xx = strrchr(tmp, '\\');
   if (*fname == '?') *xx = 0; // "?" to get exe directory
   else strcpy(xx+1, fname);
   strcpy(dst, tmp);
}

void save_nv()
{
   char line[0x200]; addpath(line, "CMOS");
   FILE *f0 = fopen(line, "wb");
   if (f0) fwrite(cmos, 1, sizeof cmos, f0), fclose(f0);

   addpath(line, "NVRAM");
   if (f0 = fopen(line, "wb")) fwrite(nvram, 1, sizeof nvram, f0), fclose(f0);
}

void load_ay_stereo()
{
   char line[128], name[64]; sprintf(name, "STEREO.%s", aystereo[conf.sound.ay_stereo]);
   GetPrivateProfileString("AY", name, "100,10,66,66,10,100", line, sizeof line, ininame);
   sscanf(line, "%d,%d,%d,%d,%d,%d", &stereo[0], &stereo[1], &stereo[2], &stereo[3], &stereo[4], &stereo[5]);
}

void load_romset(CONFIG *conf, char *romset)
{
   char sec[0x200];
   sprintf(sec, "ROM.%s", romset);
   GetPrivateProfileString(sec, "sos", nil, conf->sos_rom_path, sizeof conf->sos_rom_path, ininame);
   GetPrivateProfileString(sec, "dos", nil, conf->dos_rom_path, sizeof conf->dos_rom_path, ininame);
   GetPrivateProfileString(sec, "128", nil, conf->zx128_rom_path, sizeof conf->zx128_rom_path, ininame);
   GetPrivateProfileString(sec, "sys", nil, conf->sys_rom_path, sizeof conf->sys_rom_path, ininame);
   addpath(conf->sos_rom_path);
   addpath(conf->dos_rom_path);
   addpath(conf->zx128_rom_path);
   addpath(conf->sys_rom_path);
}

void add_presets(char *section, char *prefix0, unsigned *num, char **tab, unsigned char *curr)
{
   *num = 0; char buf[0x7F00], defval[64];
   GetPrivateProfileSection(section, buf, sizeof buf, ininame);
   GetPrivateProfileString(section, prefix0, "none", defval, sizeof defval, ininame);
   char *p = strchr(defval, ';'); if (p) *p = 0;
   for (p = defval+strlen(defval)-1; p>=defval && *p == ' '; *p-- = 0);
   char prefix[0x200]; strcpy(prefix, prefix0);
   strcat(prefix, "."); unsigned plen = strlen(prefix);
   for (char *ptr = buf; *ptr; ) {
      if (!strnicmp(ptr, prefix, plen)) {
         ptr += plen; tab[*num] = setptr;
         while (*ptr && *ptr != '=') *setptr++ = *ptr++; *setptr++ = 0;
         if (!stricmp(tab[*num], defval)) *curr = (unsigned char)*num;
         (*num)++;
      }
      while (*ptr) ptr++; ptr++;
   }
}

void load_ula_preset()
{
   if (conf.ula_preset >= num_ula) return;
   char line[128], name[64];
   sprintf(name, "PRESET.%s", ulapreset[conf.ula_preset]);
   static char defaults[] = "71680,17989,224,50,0,0";
   GetPrivateProfileString("ULA", name, defaults, line, sizeof line, ininame);
   unsigned t1, t2, t3, t4;
   sscanf(line, "%d,%d,%d,%d,%d,%d,%d,%d", &conf.frame, &conf.paper, &conf.t_line, &conf.intfq, &t1, &t2, &t3, &t4);
   conf.even_M1 = (unsigned char)t1; conf.border_4T = (unsigned char)t2;
   conf.floatbus = (unsigned char)t3; conf.floatdos = (unsigned char)t4;
}

void load_ay_vols()
{
   char line[512] = { 0 };
   static char defaults[] = "0000,0340,04C0,06F2,0A44,0F13,1510,227E,289F,414E,5B21,7258,905E,B550,D7A0,FFFF";
   char name[64]; sprintf(name, "VOLTAB.%s", ayvols[conf.sound.ay_vols]);
   GetPrivateProfileString("AY", name, defaults, line, sizeof line, ininame);
   if (line[74] != ',') strcpy(line, defaults);
   if (line[79] == ',') { // YM
      for (int i = 0; i < 32; i++)
         sscanf(line+i*5, "%X", &voltab[i]);
   } else { // AY
      for (int i = 0; i < 16; i++)
         sscanf(line+i*5, "%X", &voltab[2*i]), voltab[2*i+1] = voltab[2*i];
   }
}

void load_config(char *fname)
{
   char line[0x200];
   load_errors = 0;

   GetModuleFileName(0, ininame, sizeof ininame);
   strlwr(ininame); *(unsigned*)(strstr(ininame, ".exe")+1) = WORD4('i','n','i',0);

   if (fname && *fname) {
      char *dst = strrchr(ininame, '\\');
      if (strchr(fname, '/') || strchr(fname, '\\')) dst = ininame; else dst++;
      strcpy(dst, fname);
   }
   printf("ini: %s\n", ininame);

   if (GetFileAttributes(ininame) == -1)
      printf("config not found, using defaults\n"), load_errors = 1;
   static char misc[] = "MISC";
   static char video[] = "VIDEO";
   static char ula[] = "ULA";
   static char beta128[] = "Beta128";
   static char leds[] = "LEDS";
   static char sound[] = "SOUND";
   static char input[] = "INPUT";
   static char colors[] = "COLORS";
   static char ay[] = "AY";
   static char atm[] = "ATM";
   static char hdd[] = "HDD";
   static char rom[] = "ROM";

   #ifdef MOD_MONITOR
   addpath(conf.sos_labels_path, "sos.l");
   #endif

   GetPrivateProfileString("*", "UNREAL", nil, line, sizeof line, ininame);
   int a,b,c; sscanf(line, "%u.%u.%u", &a, &b, &c);
   if ((a*0x100+b-VER_HL) | (c-(VER_A & 0x7F))) { printf("wrong ini-file version"); exit(); }

   GetPrivateProfileString(misc, "Help",  "help_eng.html", helpname, sizeof helpname, ininame);
   addpath(helpname);

   if (GetPrivateProfileInt(misc, "HideConsole", 0, ininame)) FreeConsole(), nowait=1;

   conf.sleepidle = GetPrivateProfileInt(misc, "ShareCPU", 0, ininame);
   conf.tape_autostart = GetPrivateProfileInt(misc, "TapeAutoStart", 1, ininame);
   conf.EFF7_mask = GetPrivateProfileInt(misc, "EFF7mask", 0, ininame);

   GetPrivateProfileString(rom, "ATM", nil, conf.atm_rom_path, sizeof conf.atm_rom_path, ininame);
   GetPrivateProfileString(rom, "SCORP", nil, conf.scorp_rom_path, sizeof conf.scorp_rom_path, ininame);
   GetPrivateProfileString(rom, "PROFROM", nil, conf.prof_rom_path, sizeof conf.prof_rom_path, ininame);
   GetPrivateProfileString(rom, "PROFI", nil, conf.profi_rom_path, sizeof conf.profi_rom_path, ininame);
   #ifdef MOD_GSZ80
   GetPrivateProfileString(rom, "GS", nil, conf.gs_rom_path, sizeof conf.gs_rom_path, ininame);
   addpath(conf.gs_rom_path);
   #endif
   addpath(conf.atm_rom_path);
   addpath(conf.scorp_rom_path);
   addpath(conf.prof_rom_path);
   addpath(conf.profi_rom_path);

   GetPrivateProfileString(rom, "ROMSET", "default", line, sizeof line, ininame);
   if (*line) load_romset(&conf, line), conf.use_romset = 1; else conf.use_romset = 0;

   conf.smuc = GetPrivateProfileInt(misc, "SMUC", 0, ininame);
   GetPrivateProfileString(misc, "CMOS", nil, line, sizeof line, ininame);
   conf.cmos = 0;
   if (!strnicmp(line, "DALLAS", 6)) conf.cmos=1;
   if (!strnicmp(line, "512Bu1", 6)) conf.cmos=2;
   conf.cache = GetPrivateProfileInt(misc, "Cache", 0, ininame);
   if (conf.cache && conf.cache!=16 && conf.cache!=32) conf.cache = 0;
   GetPrivateProfileString(misc, "HIMEM", nil, line, sizeof line, ininame);
   conf.mem_model = MM_PENTAGON;
   for (unsigned i = 0; i < N_MM_MODELS; i++)
      if (!strnicmp(line, mem_model[i].shortname, strlen(mem_model[i].shortname)))
         conf.mem_model = (MEM_MODEL)i;
   conf.ramsize = GetPrivateProfileInt(misc, "RamSize", 128, ininame);

   if ((conf.mem_model == MM_PENTAGON || conf.mem_model == MM_MYPENT) &&
        conf.ramsize != 128 && conf.ramsize != 256 && conf.ramsize != 512 && conf.ramsize != 1024)
      conf.ramsize = 128, printf("invalid RAM size for PENTAGON model\n");
   if ((conf.mem_model == MM_SCORP || conf.mem_model == MM_PROFSCORP) && conf.ramsize != 256 && conf.ramsize != 1024)
      conf.ramsize = 256, printf("invalid RAM size for SCORPION model\n");
   if (conf.mem_model == MM_PROFI && conf.ramsize != 1024)
      conf.ramsize = 1024, printf("invalid RAM size for PROFI model\n");
   if (conf.mem_model == MM_ATM450 && conf.ramsize != 512 && conf.ramsize != 1024)
      conf.ramsize = 512, printf("invalid RAM size for ATM 4.50 model\n");
   if (conf.mem_model == MM_ATM710 && conf.ramsize != 128 && conf.ramsize != 256 && conf.ramsize != 512 && conf.ramsize != 1024)
      conf.ramsize = 1024, printf("invalid RAM size for ATM 7.10 model\n");

   GetPrivateProfileString(misc, "DIR", nil, conf.workdir, sizeof conf.workdir, ininame);

   conf.reset_rom = RM_SOS;
   GetPrivateProfileString(misc, "RESET", nil, line, sizeof line, ininame);
   if (!strnicmp(line, "DOS", 3)) conf.reset_rom = RM_DOS;
   if (!strnicmp(line, "MENU", 4)) conf.reset_rom = RM_128;
   if (!strnicmp(line, "SYS", 3)) conf.reset_rom = RM_SYS;

   GetPrivateProfileString(misc, "Modem", nil, line, sizeof line, ininame);
   conf.modem_port = 0;
   if (!strnicmp(line, "COM", 3) && ((unsigned char)line[3])-'1' < 9) conf.modem_port = line[3] - '0';

   conf.paper = GetPrivateProfileInt(ula, "Paper", 17989, ininame);
   conf.t_line = GetPrivateProfileInt(ula, "Line", 224, ininame);
   conf.intfq = GetPrivateProfileInt(ula, "int", 50, ininame);
   conf.frame = GetPrivateProfileInt(ula, "Frame", 71680, ininame);
   conf.border_4T = GetPrivateProfileInt(ula, "4TBorder", 0, ininame);
   conf.even_M1 = GetPrivateProfileInt(ula, "EvenM1", 0, ininame);
   conf.floatbus = GetPrivateProfileInt(ula, "FloatBus", 0, ininame);
   conf.floatdos = GetPrivateProfileInt(ula, "FloatDOS", 0, ininame);

   conf.ula_preset=-1;
   add_presets(ula, "preset", &num_ula, ulapreset, &conf.ula_preset);

   conf.atm.mem_swap = GetPrivateProfileInt(ula, "AtmMemSwap", 0, ininame);
   conf.atm.use_pal = GetPrivateProfileInt(ula, "UseAtmPalette", 1, ininame);

   conf.flashcolor = GetPrivateProfileInt(video, "FlashColor", 0, ininame);
   conf.frameskip = GetPrivateProfileInt(video, "SkipFrame", 0, ininame);
   conf.flip = GetPrivateProfileInt(video, "VSync", 0, ininame);
   conf.fullscr = GetPrivateProfileInt(video, "FullScr", 1, ininame);
   conf.refresh = GetPrivateProfileInt(video, "Refresh", 0, ininame);
   conf.frameskipmax = GetPrivateProfileInt(video, "SkipFrameMaxSpeed", 33, ininame);
   conf.updateb = GetPrivateProfileInt(video, "Update", 1, ininame);
   conf.ch_size = GetPrivateProfileInt(video, "ChunkSize", 0, ininame);
   conf.noflic = GetPrivateProfileInt(video, "NoFlic", 0, ininame);
   conf.alt_nf = GetPrivateProfileInt(video, "AltNoFlic", 0, ininame);
   conf.scanbright = GetPrivateProfileInt(video, "ScanIntens", 66, ininame);
   conf.pixelscroll = GetPrivateProfileInt(video, "PixelScroll", 0, ininame);
   conf.detect_video = GetPrivateProfileInt(video, "DetectModel", 1, ininame);
   conf.fontsize = 8;

   conf.rsm.mix_frames = GetPrivateProfileInt(video, "rsm.frames", 8, ininame);
   GetPrivateProfileString(video, "rsm.mode", nil, line, sizeof line, ininame);
   conf.rsm.mode = RSM_FIR0;
   if (!strnicmp(line, "FULL", 4)) conf.rsm.mode = RSM_FIR0;
   if (!strnicmp(line, "2C", 2)) conf.rsm.mode = RSM_FIR1;
   if (!strnicmp(line, "3C", 2)) conf.rsm.mode = RSM_FIR2;
   if (!strnicmp(line, "SIMPLE", 6)) conf.rsm.mode = RSM_SIMPLE;

   GetPrivateProfileString(video, video, nil, line, sizeof line, ininame);
   conf.render = 0;
   for (i = 0; renders[i].func; i++)
      if (!strnicmp(line, renders[i].nick, strlen(renders[i].nick)))
         conf.render = i;

   GetPrivateProfileString(video, "driver", nil, line, sizeof line, ininame);
   conf.driver = DRIVER_DDRAW;
   for (i = 0; drivers[i].nick; i++)
      if (!strnicmp(line, drivers[i].nick, strlen(drivers[i].nick)))
         conf.driver = i;

   conf.fast_sl = GetPrivateProfileInt(video, "fastlines", 0, ininame);

   GetPrivateProfileString(video, "Border", nil, line, sizeof line, ininame);
   conf.bordersize = 1;
   if (!strnicmp(line, "none", 4)) conf.bordersize = 0;
   if (!strnicmp(line, "small", 5)) conf.bordersize = 1;
   if (!strnicmp(line, "wide", 4)) conf.bordersize = 2;
   conf.minres = GetPrivateProfileInt(video, "MinRes", 0, ininame);

   GetPrivateProfileString(video, "Hide", nil, line, sizeof line, ininame);
   char *ptr = strchr(line, ';'); if (ptr) *ptr = 0;
   for (ptr = line;;) {
      unsigned max = (sizeof renders/sizeof *renders)-1;
      for (i = 0; renders[i].func; i++) {
         unsigned sl = strlen(renders[i].nick);
         if (!strnicmp(ptr, renders[i].nick, sl) && !isalnum(ptr[sl])) {
            ptr += sl;
            memcpy(&renders[i], &renders[i+1], (sizeof *renders)*(max-i));
            break;
         }
      }
      if (!*ptr++) break;
   }

   GetPrivateProfileString(video, "ScrShot", nil, line, sizeof line, ininame);
   conf.bmpshot = strnicmp(line, "bmp", 3) ? 0 : 1;

   conf.trdos_present = GetPrivateProfileInt(beta128, "beta128", 1, ininame);
   conf.trdos_traps = GetPrivateProfileInt(beta128, "Traps", 1, ininame);
   conf.wd93_nodelay = GetPrivateProfileInt(beta128, "Fast", 1, ininame);
   conf.trdos_interleave = GetPrivateProfileInt(beta128, "IL", 1, ininame)-1;
   if (conf.trdos_interleave > 2) conf.trdos_interleave = 0;
   GetPrivateProfileString(beta128, "BOOT", nil, conf.appendboot, sizeof conf.appendboot, ininame);
   addpath(conf.appendboot);

   conf.led.enabled = GetPrivateProfileInt(leds, "leds", 1, ininame);
   conf.led.flash_ay_kbd = GetPrivateProfileInt(leds, "KBD_AY", 1, ininame);
   conf.led.perf_t = GetPrivateProfileInt(leds, "PerfShowT", 0, ininame);

   static char nm[] = "AY\0Perf\0LOAD\0Input\0Time\0OSW";
   char *n2 = nm;
   for (i = 0; i < NUM_LEDS; i++) {
      GetPrivateProfileString("LEDS", n2, nil, line, sizeof line, ininame);
      int x, y, z; unsigned r; n2 += strlen(n2)+1;
      if (sscanf(line, "%d:%d,%d", &z, &x, &y) != 3) r = 0;
      else r = (x & 0xFFFF) + ((y << 16) & 0x7FFFFFFF) + z*0x80000000;
      *(&conf.led.ay+i) = r;
   }

   conf.sound.do_sound = do_sound_none;
   GetPrivateProfileString(sound, "SoundDrv", nil, line, sizeof line, ininame);
   if (!strnicmp(line, "wave", 4)) {
      conf.sound.do_sound = do_sound_wave;
      conf.soundbuffer = GetPrivateProfileInt(sound, "SoundBuffer", 0, ininame);
      if (!conf.soundbuffer) conf.soundbuffer = 6;
      if (conf.soundbuffer >= MAXWQSIZE) conf.soundbuffer = MAXWQSIZE-1;
   }
   if (!strnicmp(line, "ds", 2)) {
      conf.sound.do_sound = do_sound_ds;
//      conf.soundbuffer = GetPrivateProfileInt(sound, "DSoundBuffer", 1000, ininame);
//      conf.soundbuffer *= 4; // 16-bit, stereo
   }

   conf.sound.enabled = GetPrivateProfileInt(sound, "Enabled", 1, ininame);
   #ifdef MOD_GS
   conf.sound.gsreset = GetPrivateProfileInt(sound, "GSReset", 0, ininame);
   #endif
   conf.sound.fq = GetPrivateProfileInt(sound, "Fq", 44100, ininame);
   conf.sound.dsprimary = GetPrivateProfileInt(sound, "DSPrimary", 0, ininame);

   conf.gs_type = 0;
#ifdef MOD_GS
   GetPrivateProfileString(sound, "GSTYPE", nil, line, sizeof line, ininame);
   #ifdef MOD_GSZ80
   if (!strnicmp(line, "Z80", 3)) conf.gs_type = 1;
   #endif
   #ifdef MOD_GSBASS
   if (!strnicmp(line, "BASS", 4)) conf.gs_type = 2;
   #endif
#endif

   conf.sound.beeper = GetPrivateProfileInt(sound, "Beeper", 4000, ininame);
   conf.sound.micout = GetPrivateProfileInt(sound, "MicOut", 1000, ininame);
   conf.sound.micin = GetPrivateProfileInt(sound, "MicIn", 1000, ininame);
   conf.sound.ay = GetPrivateProfileInt(sound, "AY", 4000, ininame);
   conf.sound.covoxFB = GetPrivateProfileInt(sound, "CovoxFB", 8192, ininame);
   conf.sound.covoxDD = GetPrivateProfileInt(sound, "CovoxDD", 4000, ininame);
   conf.sound.sd = GetPrivateProfileInt(sound, "SD", 4000, ininame);

   #ifdef MOD_GS
   conf.sound.gs = GetPrivateProfileInt(sound, "GS", 8000, ininame);
   #endif

   #ifdef MOD_GSBASS
   conf.sound.bass = GetPrivateProfileInt(sound, "BASS", 8000, ininame);
   #endif

   add_presets(ay, "VOLTAB", &num_ayvols, ayvols, &conf.sound.ay_vols);
   add_presets(ay, "STEREO", &num_aystereo, aystereo, &conf.sound.ay_stereo);
   conf.sound.ayfq = GetPrivateProfileInt(ay, "Fq", 1774400, ininame);

   GetPrivateProfileString(ay, "Chip", nil, line, sizeof line, ininame);
   conf.sound.ay_chip = AY::CHIP_YM;
   if (!strnicmp(line, "AY", 2)) conf.sound.ay_chip = AY::CHIP_AY;
   if (!strnicmp(line, "YM", 2)) conf.sound.ay_chip = AY::CHIP_YM;

   conf.sound.ay_samples = GetPrivateProfileInt(ay, "UseSamples", 0, ininame);

   GetPrivateProfileString(ay, "Scheme", nil, line, sizeof line, ininame);
   conf.sound.ay_scheme = AY::SCHEME_NONE;
   if (!strnicmp(line, "default", 7)) conf.sound.ay_scheme = AY::SCHEME_SINGLE;
   if (!strnicmp(line, "PSEUDO", 6)) conf.sound.ay_scheme = AY::SCHEME_PSEUDO;
   if (!strnicmp(line, "QUADRO", 6)) conf.sound.ay_scheme = AY::SCHEME_QUADRO;
   if (!strnicmp(line, "POS", 3)) conf.sound.ay_scheme = AY::SCHEME_POS;
   if (!strnicmp(line, "CHRV", 4)) conf.sound.ay_scheme = AY::SCHEME_CHRV;

   GetPrivateProfileString(input, "KeybLayout", "default", conf.keyset, sizeof conf.keyset, ininame);

   GetPrivateProfileString(input, "Mouse", nil, line, sizeof line, ininame);
   conf.input.mouse = 0;
   if (!strnicmp(line, "KEMPSTON", 8)) conf.input.mouse = 1;
   if (!strnicmp(line, "AY", 2)) conf.input.mouse = 2;

   conf.input.joymouse = GetPrivateProfileInt(input, "JoyMouse", 0, ininame);
   conf.input.mousescale = GetPrivateProfileInt(input, "MouseScale", 0, ininame);
   conf.input.mouseswap = GetPrivateProfileInt(input, "SwapMouse", 0, ininame);
   conf.input.kjoy = GetPrivateProfileInt(input, "KJoystick", 1, ininame);
   conf.input.keymatrix = GetPrivateProfileInt(input, "Matrix", 1, ininame);
   conf.input.firedelay = GetPrivateProfileInt(input, "FireRate", 1, ininame);
   conf.input.altlock = GetPrivateProfileInt(input, "AltLock", 1, ininame);
   conf.input.paste_hold = GetPrivateProfileInt(input, "HoldDelay", 2, ininame);
   conf.input.paste_release = GetPrivateProfileInt(input, "ReleaseDelay", 5, ininame);
   conf.input.paste_newline = GetPrivateProfileInt(input, "NewlineDelay", 20, ininame);
   conf.atm.xt_kbd = GetPrivateProfileInt(input, "ATMKBD", 0, ininame);
   GetPrivateProfileString(input, "Fire", "0", line, sizeof line, ininame);
   conf.input.firenum = 0; conf.input.fire = 0;
   for (i = 0; i < sizeof zxk / sizeof *zxk; i++)
      if (!strnicmp(line, zxk[i].name, strlen(zxk[i].name)))
      {  conf.input.firenum = i; break; }

   char buff[0x7000];
   GetPrivateProfileSection(colors, buff, sizeof buff, ininame);
   GetPrivateProfileString(colors, "color", "default", line, sizeof line, ininame);
   conf.pal = 0;

   for (i = 1, ptr = buff; i < sizeof pals / sizeof *pals; ptr += strlen(ptr)+1) {
      if (!*ptr) break;
      if (!isalnum(*ptr) || !strnicmp(ptr, "color=", 6)) continue;
      char *ptr1 = strchr(ptr, '='); if (!ptr1) continue;
      *ptr1 = 0; strcpy(pals[i].name, ptr); ptr = ptr1+1;
      sscanf(ptr, "%02X,%02X,%02X,%02X,%02X,%02X:%02X,%02X,%02X;%02X,%02X,%02X;%02X,%02X,%02X",
         &pals[i].ZZ,  &pals[i].ZN,  &pals[i].NN,
         &pals[i].NB,  &pals[i].BB,  &pals[i].ZB,
         &pals[i].r11, &pals[i].r12, &pals[i].r13,
         &pals[i].r21, &pals[i].r22, &pals[i].r23,
         &pals[i].r31, &pals[i].r32, &pals[i].r33);
      if (!strnicmp(line, pals[i].name, strlen(pals[i].name))) conf.pal = i;
      i++;
   }
   conf.num_pals = i;

   GetPrivateProfileString(hdd, "SCHEME", nil, line, sizeof line, ininame);
   conf.ide_scheme = IDE_NONE;
   if (!strnicmp(line, "ATM", 3)) conf.ide_scheme = IDE_ATM;
   if (!strnicmp(line, "NEMO", 4)) conf.ide_scheme = IDE_NEMO;
   if (!strnicmp(line, "NEMO-A8", 7)) conf.ide_scheme = IDE_NEMO_A8;
   if (!strnicmp(line, "SMUC", 4)) conf.ide_scheme = IDE_SMUC;
   conf.ide_skip_real = GetPrivateProfileInt(hdd, "SkipReal", 0, ininame);
   for (int ide_device = 0; ide_device < 2; ide_device++) {
      char param[32];
      sprintf(param, "LBA%d", ide_device);
      conf.ide[ide_device].lba = GetPrivateProfileInt(hdd, param, 0, ininame);
      sprintf(param, "CHS%d", ide_device);
      GetPrivateProfileString(hdd, param, "0/0/0", line, sizeof line, ininame);
      sscanf(line, "%d/%d/%d", &conf.ide[ide_device].c, &conf.ide[ide_device].h, &conf.ide[ide_device].s);
      sprintf(param, "Image%d", ide_device);
      GetPrivateProfileString(hdd, param, nil, conf.ide[ide_device].image, sizeof conf.ide[ide_device].image, ininame);
      sprintf(param, "HD%dRO", ide_device);
      conf.ide[ide_device].readonly = (BYTE)GetPrivateProfileInt(hdd, param, 0, ininame);
   }

   addpath(line, "CMOS");
   FILE *f0 = fopen(line, "rb");
   if (f0) fread(cmos, 1, sizeof cmos, f0), fclose(f0); else cmos[0x11] = 0xAA;

   addpath(line, "NVRAM");
   if (f0 = fopen(line, "rb")) fread(nvram, 1, sizeof nvram, f0), fclose(f0);

   load_atm_font();
   load_arch(ininame);
   loadkeys(ac_main);
#ifdef MOD_MONITOR
   loadkeys(ac_main_xt);
   loadkeys(ac_regs);
   loadkeys(ac_trace);
   loadkeys(ac_mem);
#endif
}

void apply_memory()
{
   #ifdef MOD_GSZ80
   if (conf.gs_type == 1) { if (load_rom(conf.gs_rom_path, ROM_GS_M, 2) != 32) conf.gs_type = 0; }
   #endif

   if (conf.mem_model == MM_ATM710) {
      base_sos_rom = ROM_BASE_M + 0*PAGE;
      base_dos_rom = ROM_BASE_M + 1*PAGE;
      base_128_rom = ROM_BASE_M + 2*PAGE;
      base_sys_rom = ROM_BASE_M + 3*PAGE;
   } else {
      base_128_rom = ROM_BASE_M + 0*PAGE;
      base_sos_rom = ROM_BASE_M + 1*PAGE;
      base_sys_rom = ROM_BASE_M + 2*PAGE;
      base_dos_rom = ROM_BASE_M + 3*PAGE;
   }

   unsigned romsize;
   if (conf.use_romset) {
      if (!load_rom(conf.sos_rom_path, base_sos_rom)) { printf("fatal error: failed to load BASIC48 ROM\n"); exit(); }
      if (!load_rom(conf.zx128_rom_path, base_128_rom) && conf.reset_rom == RM_128) conf.reset_rom = RM_SOS;
      if (!load_rom(conf.dos_rom_path, base_dos_rom)) conf.trdos_present = 0;
      if (!load_rom(conf.sys_rom_path, base_sys_rom) && conf.reset_rom == RM_SYS) conf.reset_rom = RM_SOS;
      romsize = 64;
   } else {

      if (conf.mem_model == MM_ATM710) {
         romsize = load_rom(conf.atm_rom_path, ROM_BASE_M, 64);
         if (romsize != 64 && romsize != 128 && romsize != 512 && romsize != 1024)
            printf("invalid ROM size for ATM bios\n"), exit();
         unsigned char *lastpage = ROM_BASE_M + (romsize-64)*1024;
         base_sos_rom = lastpage + 0*PAGE;
         base_dos_rom = lastpage + 1*PAGE;
         base_128_rom = lastpage + 2*PAGE;
         base_sys_rom = lastpage + 3*PAGE;
      } else if (conf.mem_model == MM_PROFSCORP) {
         romsize = load_rom(conf.prof_rom_path, ROM_BASE_M, 16);
         if (romsize != 64 && romsize != 128 && romsize != 256)
            printf("invalid PROF-ROM size\n"), exit();
      } else {
         char *romname = 0;
         if (conf.mem_model == MM_PROFI) romname = conf.profi_rom_path;
         if (conf.mem_model == MM_SCORP) romname = conf.scorp_rom_path;
         if (!romname) printf("error: ROMSET should be defined for this memory model\n"), exit();

         romsize = load_rom(romname, ROM_BASE_M, 64);
         if (romsize != 64) printf("invalid ROM size for %s\n", mem_model[conf.mem_model].fullname), exit();
      }
   }

   if (conf.mem_model == MM_PROFSCORP) {
      temp.profrom_mask = 0;
      if (romsize == 128) temp.profrom_mask = 1;
      if (romsize == 256) temp.profrom_mask = 3;
      comp.profrom_bank &= temp.profrom_mask;
      set_scorp_profrom(0);
   }

#ifdef MOD_MONITOR
   load_labels(conf.sos_labels_path, base_sos_rom, 0x4000);
#endif
   temp.ram_mask = (conf.ramsize-1) >> 4;
   temp.rom_mask = (romsize-1) >> 4;
   set_banks();
#ifdef MOD_MONITOR
   dbgchk = isbrk();
#endif
}


void applyconfig()
{
   conf.ticks_frame = (unsigned)(conf.cpufq/conf.intfq);
   loadzxkeys(&conf);
   apply_memory();

   temp.snd_frame_ticks = (conf.sound.fq << TICK_FF) / conf.intfq;
   temp.snd_frame_samples = temp.snd_frame_ticks >> TICK_FF;
   temp.frameskip = conf.sound.enabled? conf.frameskip : conf.frameskipmax;

   input.firedelay = 1; // if conf.input.fire changed
   input.clear_zx();
   modem.open(conf.modem_port);

   apply_video();
   apply_sound();

   hdd.dev[0].configure(conf.ide+0);
   hdd.dev[1].configure(conf.ide+1);
   if (conf.atm.xt_kbd) input.atm51.clear();

   setpal(0);
}

void load_arch(char *fname)
{
   GetPrivateProfileString("ARC", "SkipFiles", nil, skiparc, sizeof skiparc, fname);
   for (char *p = skiparc;;) {
      char *nxt = strchr(p, ';');
      if (!nxt) break;
      *nxt = 0; p = nxt+1;
   }
   p[strlen(p)+1] = 0;

   GetPrivateProfileSection("ARC", arcbuffer, sizeof arcbuffer, fname);
   for (char *x = arcbuffer; *x; ) {
      char *newx = x + strlen(x)+1;
      char *y = strchr(x, '=');
      if (!y) {
ignore_line:
         memcpy(x, newx, sizeof arcbuffer - (newx-arcbuffer));
      } else {
         *y = 0; if (!stricmp(x, "SkipFiles")) goto ignore_line;
         x = newx;
      }
   }
}

void loadkeys(action *table)
{
   unsigned num[0x300], i = 0;
   if (!table->name) return; // empty table (can't sort)
   for (action *p = table; p->name; p++, i++) {
      char line[0x400];
      GetPrivateProfileString("SYSTEM.KEYS", p->name, "`", line, sizeof line, ininame);
      if (*line == '`') {
         printf("warning: keydef for %s not found\n", p->name); load_errors = 1;
bad_key: p->k1 = 0xFE, p->k2 = 0xFF, p->k3 = 0xFD;
         continue;
      }
      char *s = strchr(line, ';'); if (s) *s = 0;
      p->k1 = p->k2 = p->k3 = p->k4 = 0; num[i] = 0;
      for (s = line;;) {
         while (*s == ' ') s++;
         if (!*s) break;
         char *s1 = s; while (isalnum(*s)) s++;
         for (unsigned j = 0; j < sizeof pckeys / sizeof *pckeys; j++)
            if ((int)strlen(pckeys[j].name)==s-s1 && !strnicmp(s1, pckeys[j].name, s-s1)) {
               switch (num[i]) {
                  case 0: p->k1 = pckeys[j].virtkey; break;
                  case 1: p->k2 = pckeys[j].virtkey; break;
                  case 2: p->k3 = pckeys[j].virtkey; break;
                  case 3: p->k4 = pckeys[j].virtkey; break;
                  default:
                     printf("warning: too many keys in %s=%s\n", p->name, line);
                     load_errors = 1;
               }
               num[i]++;
               break;
            }
         if (j == sizeof pckeys / sizeof *pckeys) {
            char x = *s; *s = 0; printf("bad key: %s\n", s1); *s = x;
            load_errors = 1;
         }
      }
      if (!num[i]) goto bad_key;
   }
   // sort keys
   for (unsigned k = 0; k < i-1; k++) {
      unsigned max = k;
      for (unsigned l = k+1; l < i; l++)
         if (num[l] > num[max]) max = l;
      action tmp = table[k]; table[k] = table[max]; table[max] = tmp;
      unsigned tm = num[k]; num[k] = num[max]; num[max] = tm;
   }
}

void loadzxkeys(CONFIG *conf)
{
   char section[0x200];
   sprintf(section, "ZX.KEYS.%s", conf->keyset);
   char line[0x300];
   for (unsigned i = 0; i < VK_MAX; i++) {
      inports[i].port1 = inports[i].port2 = &input.kjoy;
      inports[i].mask1 = inports[i].mask2 = 0xFF;
      for (unsigned j = 0; j < sizeof pckeys / sizeof *pckeys; j++)
         if (pckeys[j].virtkey == i) {
            GetPrivateProfileString(section, pckeys[j].name, "", line, sizeof line, ininame);
            for (char *s = line; *s == ' '; s++);
            for (unsigned k = 0; k < sizeof zxk / sizeof *zxk; k++) {
               if (!strnicmp(s, zxk[k].name, strlen(zxk[k].name))) {
                  inports[i].port1 = zxk[k].port;
                  inports[i].mask1 = zxk[k].mask;
                  break;
               }
            }
            while (isalnum(*s)) s++;
            while (*s == ' ') s++;
            for (k = 0; k < sizeof zxk / sizeof *zxk; k++) {
               if (!strnicmp(s, zxk[k].name, strlen(zxk[k].name))) {
                  inports[i].port2 = zxk[k].port;
                  inports[i].mask2 = zxk[k].mask;
                  break;
               }
            }
            break;
         }
   }
}
