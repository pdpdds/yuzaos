//	Gamectrl Header

extern int16_t dx1, dy1, fire1, fire2, fire1off, fire2off;
extern int16_t joyxl, joyxc, joyxr, joyyu, joyyc, joyyd;
extern int16_t joyflag;
extern int16_t key;
extern int16_t dx1hold, dy1hold, flow1;
extern int16_t joyxsense, joyysense;
extern int32_t systime;

void readspeed (void);
int16_t joypresent (void);
void gc_init (void);
int16_t  gc_config (void);
void gc_exit (void);
void checkctrl (int16_t pollflag);
void checkctrl0 (int16_t pollflag);
int16_t  getctrlmode (void);
void getkey (void);

void recordmac (char *fname);
void playmac (char *fname);
void stopmac (void);
void macrecend (void);

extern int16_t macrecord;
extern int16_t macplay;
extern int16_t macabort;
extern int16_t macaborted;