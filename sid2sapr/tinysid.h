
// Externe Daten aus sidengine.c

extern void synth_render (uint16_t *buffer, uint32_t len);
extern void c64Init(void);
extern uint16_t c64SidLoad(char *filename, uint16_t *init_addr, uint16_t *play_addr, uint8_t *sub_song_start, uint8_t *max_sub_songs, uint8_t *speed, char *name, char *author, char *copyright);
extern int cpuJSR(uint16_t npc, uint8_t na);
extern void synth_init(unsigned long mixfrq);

// Externe Daten aus soundcard.c

extern void soundcard_init(void);
extern void start_playing(uint16_t nplay_addr, uint8_t nplay_speed);
extern void stop_playing(void);

extern int gui_init(void);

