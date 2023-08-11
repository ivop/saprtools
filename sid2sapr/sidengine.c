/* Largely TinySID with the sound emulation removed.
 * ADSR and 6502 code Copyright (C) 1999-2006 T. Hinrichs and R. Sinsch.
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "sidengine.h"

#define FLAG_N 128
#define FLAG_V 64
#define FLAG_B 16
#define FLAG_D 8
#define FLAG_I 4
#define FLAG_Z 2
#define FLAG_C 1

static inline int pfloat_ConvertFromInt(int i) {
    return i << 16;
}
static inline int pfloat_ConvertFromFloat(float f) {
    return (int)(f * (1 << 16));
}
static inline int pfloat_Multiply(int a, int b) {
    return (a >> 8) * (b >> 8);
}
static inline int pfloat_ConvertToInt(int i) {
    return i >> 16;
}

enum {
    adc, and, asl, bcc, bcs, beq, bit, bmi, bne, bpl, brk, bvc, bvs, clc,
    cld, cli, clv, cmp, cpx, cpy, dec, dex, dey, eor, inc, inx, iny, jmp,
    jsr, lda, ldx, ldy, lsr, nop, ora, pha, php, pla, plp, rol, ror, rti,
    rts, sbc, sec, sed, sei, sta, stx, sty, tax, tay, tsx, txa, txs, tya,
    xxx
};

// ------------------------------------------------------------- constants

static float attackTimes[16] = {
    0.0022528606, 0.0080099577, 0.0157696042, 0.0237795619, 0.0372963655,
    0.0550684591, 0.0668330845, 0.0783473987, 0.0981219818, 0.244554021,
    0.489108042, 0.782472742, 0.977715461, 2.93364701, 4.88907793, 7.82272493
};

static float decayReleaseTimes[16] = {
    0.00891777693, 0.024594051, 0.0484185907, 0.0730116639, 0.114512475,
    0.169078356, 0.205199432, 0.240551975, 0.301266125, 0.750858245,
    1.50171551, 2.40243682, 3.00189298, 9.00721405, 15.010998, 24.0182111
};

// --------------------------- pseudo-constants (depending on mixing freq)

static int mixing_frequency;
static int freqmul;
static int filtmul;
static int attacks[16];
static int releases[16];

// --------------------------------------------------------------- globals
struct s6581 sid;
struct sidosc osc[3];

// --------------------------------------------------------- some aux stuff

static inline uint8_t get_bit(uint32_t val, uint8_t b) {
    return (uint8_t) ((val >> b) & 1);
}

// ------------------------------------------------------------- synthesis

uint8_t memory[65536];          /* The C64 memory */

// initialize SID and frequency dependant values

void synth_init(uint32_t mixfrq) {
    int i;

    mixing_frequency = mixfrq;
    freqmul = 15872000 / mixfrq;
    filtmul = pfloat_ConvertFromFloat(21.5332031f) / mixfrq;
    for (i = 0; i < 16; i++) {
        attacks[i] = (int)(0x1000000 / (attackTimes[i] * mixfrq));
        releases[i] = (int)(0x1000000 / (decayReleaseTimes[i] * mixfrq));
    }
    memset(&sid, 0, sizeof(sid));
    memset(osc, 0, sizeof(osc));
    osc[0].noiseval = 0xffffff;
    osc[1].noiseval = 0xffffff;
    osc[2].noiseval = 0xffffff;
}

void synth_render(uint32_t len) {
    uint8_t v;
    uint32_t bp;

    for (v = 0; v < 3; v++) {
        osc[v].pulse = (sid.v[v].pulse & 0xfff) << 16;
        osc[v].attack = attacks[sid.v[v].ad >> 4];
        osc[v].decay = releases[sid.v[v].ad & 0xf];
        osc[v].sustain = sid.v[v].sr & 0xf0;
        osc[v].release = releases[sid.v[v].sr & 0xf];
        osc[v].wave = sid.v[v].wave;
        osc[v].freq = ((uint32_t) sid.v[v].freq) * freqmul;
    }

    // now "render"
    for (bp = 0; bp < len; bp++) {
        for (v = 0; v < 3; v++) {
            // now process the envelopes. the first thing about this is testing
            // the gate bit and put the EG into attack or release phase if
            // desired
            if (!(osc[v].wave & 0x01))
                osc[v].envphase = 3;
            else if (osc[v].envphase == 3)
                osc[v].envphase = 0;
            // so now process the volume according to the phase and adsr values         
            switch (osc[v].envphase) {
            case 0:{           // Phase 0 : Attack
                osc[v].envval += osc[v].attack;
                if (osc[v].envval >= 0xFFFFFF) {
                    osc[v].envval = 0xFFFFFF;
                    osc[v].envphase = 1;
                }
                break;
            }
            case 1:{           // Phase 1 : Decay
                osc[v].envval -= osc[v].decay;
                if ((signed int)osc[v].envval <=
                    (signed int)(osc[v].sustain << 16)) {
                    osc[v].envval = osc[v].sustain << 16;
                    osc[v].envphase = 2;
                }
                break;
            }
            case 2:{           // Phase 2 : Sustain
                if ((signed int)osc[v].envval !=
                    (signed int)(osc[v].sustain << 16)) {
                    osc[v].envphase = 1;
                }
                // :) yes, thats exactly how the SID works. and maybe
                // a music routine out there supports this, so better
                // let it in, thanks :)
                break;
            }
            case 3:{           // Phase 3 : Release
                osc[v].envval -= osc[v].release;
                if (osc[v].envval < 0x40000)
                    osc[v].envval = 0x40000;
                // the volume offset is because the SID does not
                // completely silence the voices when it should. most
                // emulators do so though and thats the main reason
                // why the sound of emulators is too, err... emulated :)
                break;
            }
            }
        }
    }
}


void sidPoke(int reg, unsigned char val) {
    int voice = 0;

    if ((reg >= 7) && (reg <= 13)) {
        voice = 1;
        reg -= 7;
    }
    if ((reg >= 14) && (reg <= 20)) {
        voice = 2;
        reg -= 14;
    }

    switch (reg) {
    case 0: sid.v[voice].freq = (sid.v[voice].freq & 0xff00) + val; break;
    case 1: sid.v[voice].freq = (sid.v[voice].freq & 0xff) + (val<<8); break;
    case 2: sid.v[voice].pulse = (sid.v[voice].pulse & 0xff00) + val; break;
    case 3: sid.v[voice].pulse = (sid.v[voice].pulse & 0xff) + (val<<8); break;
    case 4: sid.v[voice].wave = val; break;
    case 5: sid.v[voice].ad   = val; break;
    case 6: sid.v[voice].sr   = val; break;

    case 21: sid.ffreqlo = val; break;
    case 22: sid.ffreqhi = val; break;
    case 23: sid.res_ftv = val; break;
    case 24: sid.ftp_vol = val; break;
    }
    return;
}

uint8_t getmem(uint16_t addr) {
    if (addr == 0xdd0d)
        memory[addr] = 0;
    return memory[addr];
}

int internal_period, internal_order, internal_start, internal_end,
    internal_add, internal_repeat_times, internal_repeat_start;

void setmem(uint16_t addr, uint8_t value) {

    memory[addr] = value;

    //#ifdef TRACE
    //printf("setmem: $%04x <- $%02x\n",addr,memory[addr]);
    //#endif

    if ((addr & 0xfc00) == 0xd400) {
        sidPoke(addr & 0x1f, value);
    }

}

//enum { imp, imm, abs, absx, absy, zp, zpx, zpy, ind, indx, indy, acc, rel};
#define imp 0
#define imm 1
#define abs 2
#define absx 3
#define absy 4
#define zp 6
#define zpx 7
#define zpy 8
#define ind 9
#define indx 10
#define indy 11
#define acc 12
#define rel 13

static int opcodes[256] = {
    brk, ora, xxx, xxx, xxx, ora, asl, xxx, php, ora, asl, xxx, xxx, ora, asl,
        xxx,
    bpl, ora, xxx, xxx, xxx, ora, asl, xxx, clc, ora, xxx, xxx, xxx, ora, asl,
        xxx,
    jsr, and, xxx, xxx, bit, and, rol, xxx, plp, and, rol, xxx, bit, and, rol,
        xxx,
    bmi, and, xxx, xxx, xxx, and, rol, xxx, sec, and, xxx, xxx, xxx, and, rol,
        xxx,
    rti, eor, xxx, xxx, xxx, eor, lsr, xxx, pha, eor, lsr, xxx, jmp, eor, lsr,
        xxx,
    bvc, eor, xxx, xxx, xxx, eor, lsr, xxx, cli, eor, xxx, xxx, xxx, eor, lsr,
        xxx,
    rts, adc, xxx, xxx, xxx, adc, ror, xxx, pla, adc, ror, xxx, jmp, adc, ror,
        xxx,
    bvs, adc, xxx, xxx, xxx, adc, ror, xxx, sei, adc, xxx, xxx, xxx, adc, ror,
        xxx,
    xxx, sta, xxx, xxx, sty, sta, stx, xxx, dey, xxx, txa, xxx, sty, sta, stx,
        xxx,
    bcc, sta, xxx, xxx, sty, sta, stx, xxx, tya, sta, txs, xxx, xxx, sta, xxx,
        xxx,
    ldy, lda, ldx, xxx, ldy, lda, ldx, xxx, tay, lda, tax, xxx, ldy, lda, ldx,
        xxx,
    bcs, lda, xxx, xxx, ldy, lda, ldx, xxx, clv, lda, tsx, xxx, ldy, lda, ldx,
        xxx,
    cpy, cmp, xxx, xxx, cpy, cmp, dec, xxx, iny, cmp, dex, xxx, cpy, cmp, dec,
        xxx,
    bne, cmp, xxx, xxx, xxx, cmp, dec, xxx, cld, cmp, xxx, xxx, xxx, cmp, dec,
        xxx,
    cpx, sbc, xxx, xxx, cpx, sbc, inc, xxx, inx, sbc, nop, xxx, cpx, sbc, inc,
        xxx,
    beq, sbc, xxx, xxx, xxx, sbc, inc, xxx, sed, sbc, xxx, xxx, xxx, sbc, inc,
        xxx
};

static int modes[256] = {
    imp, indx, xxx, xxx, zp, zp, zp, xxx, imp, imm, acc, xxx, abs, abs, abs,
        xxx,
    rel, indy, xxx, xxx, xxx, zpx, zpx, xxx, imp, absy, xxx, xxx, xxx, absx,
        absx, xxx,
    abs, indx, xxx, xxx, zp, zp, zp, xxx, imp, imm, acc, xxx, abs, abs, abs,
        xxx,
    rel, indy, xxx, xxx, xxx, zpx, zpx, xxx, imp, absy, xxx, xxx, xxx, absx,
        absx, xxx,
    imp, indx, xxx, xxx, zp, zp, zp, xxx, imp, imm, acc, xxx, abs, abs, abs,
        xxx,
    rel, indy, xxx, xxx, xxx, zpx, zpx, xxx, imp, absy, xxx, xxx, xxx, absx,
        absx, xxx,
    imp, indx, xxx, xxx, zp, zp, zp, xxx, imp, imm, acc, xxx, ind, abs, abs,
        xxx,
    rel, indy, xxx, xxx, xxx, zpx, zpx, xxx, imp, absy, xxx, xxx, xxx, absx,
        absx, xxx,
    imm, indx, xxx, xxx, zp, zp, zp, xxx, imp, imm, acc, xxx, abs, abs, abs,
        xxx,
    rel, indy, xxx, xxx, zpx, zpx, zpy, xxx, imp, absy, acc, xxx, xxx, absx,
        absx, xxx,
    imm, indx, imm, xxx, zp, zp, zp, xxx, imp, imm, acc, xxx, abs, abs, abs,
        xxx,
    rel, indy, xxx, xxx, zpx, zpx, zpy, xxx, imp, absy, acc, xxx, absx, absx,
        absy, xxx,
    imm, indx, xxx, xxx, zp, zp, zp, xxx, imp, imm, acc, xxx, abs, abs, abs,
        xxx,
    rel, indy, xxx, xxx, zpx, zpx, zpx, xxx, imp, absy, acc, xxx, xxx, absx,
        absx, xxx,
    imm, indx, xxx, xxx, zp, zp, zp, xxx, imp, imm, acc, xxx, abs, abs, abs,
        xxx,
    rel, indy, xxx, xxx, zpx, zpx, zpx, xxx, imp, absy, acc, xxx, xxx, absx,
        absx, xxx
};

// ----------------------------------------------- globale Faulheitsvariablen

static int cycles;
static uint8_t bval;
static uint16_t wval;

// ----------------------------------------------------------------- Register

static uint8_t a, x, y, s, p;
static uint16_t pc;

// ----------------------------------------------------------- DER HARTE KERN

static uint8_t getaddr(int mode) {
    uint16_t ad, ad2;

    switch (mode) {
    case imp:
        cycles += 2;
        return 0;
    case imm:
        cycles += 2;
        return getmem(pc++);
    case abs:
        cycles += 4;
        ad = getmem(pc++);
        ad |= getmem(pc++) << 8;
        return getmem(ad);
    case absx:
        cycles += 4;
        ad = getmem(pc++);
        ad |= 256 * getmem(pc++);
        ad2 = ad + x;
        if ((ad2 & 0xff00) != (ad & 0xff00))
            cycles++;
        return getmem(ad2);
    case absy:
        cycles += 4;
        ad = getmem(pc++);
        ad |= 256 * getmem(pc++);
        ad2 = ad + y;
        if ((ad2 & 0xff00) != (ad & 0xff00))
            cycles++;
        return getmem(ad2);
    case zp:
        cycles += 3;
        ad = getmem(pc++);
        return getmem(ad);
    case zpx:
        cycles += 4;
        ad = getmem(pc++);
        ad += x;
        return getmem(ad & 0xff);
    case zpy:
        cycles += 4;
        ad = getmem(pc++);
        ad += y;
        return getmem(ad & 0xff);
    case indx:
        cycles += 6;
        ad = getmem(pc++);
        ad += x;
        ad2 = getmem(ad & 0xff);
        ad++;
        ad2 |= getmem(ad & 0xff) << 8;
        return getmem(ad2);
    case indy:
        cycles += 5;
        ad = getmem(pc++);
        ad2 = getmem(ad);
        ad2 |= getmem((ad + 1) & 0xff) << 8;
        ad = ad2 + y;
        if ((ad2 & 0xff00) != (ad & 0xff00))
            cycles++;
        return getmem(ad);
    case acc:
        cycles += 2;
        return a;
    }
    return 0;
}


static void setaddr(int mode, uint8_t val) {
    uint16_t ad, ad2;

    switch (mode) {
    case abs:
        cycles += 2;
        ad = getmem(pc - 2);
        ad |= 256 * getmem(pc - 1);
        setmem(ad, val);
        return;
    case absx:
        cycles += 3;
        ad = getmem(pc - 2);
        ad |= 256 * getmem(pc - 1);
        ad2 = ad + x;
        if ((ad2 & 0xff00) != (ad & 0xff00))
            cycles--;
        setmem(ad2, val);
        return;
    case zp:
        cycles += 2;
        ad = getmem(pc - 1);
        setmem(ad, val);
        return;
    case zpx:
        cycles += 2;
        ad = getmem(pc - 1);
        ad += x;
        setmem(ad & 0xff, val);
        return;
    case acc:
        a = val;
        return;
    }
}


static void putaddr(int mode, uint8_t val) {
    uint16_t ad, ad2;

    switch (mode) {
    case abs:
        cycles += 4;
        ad = getmem(pc++);
        ad |= getmem(pc++) << 8;
        setmem(ad, val);
        return;
    case absx:
        cycles += 4;
        ad = getmem(pc++);
        ad |= getmem(pc++) << 8;
        ad2 = ad + x;
        setmem(ad2, val);
        return;
    case absy:
        cycles += 4;
        ad = getmem(pc++);
        ad |= getmem(pc++) << 8;
        ad2 = ad + y;
        if ((ad2 & 0xff00) != (ad & 0xff00))
            cycles++;
        setmem(ad2, val);
        return;
    case zp:
        cycles += 3;
        ad = getmem(pc++);
        setmem(ad, val);
        return;
    case zpx:
        cycles += 4;
        ad = getmem(pc++);
        ad += x;
        setmem(ad & 0xff, val);
        return;
    case zpy:
        cycles += 4;
        ad = getmem(pc++);
        ad += y;
        setmem(ad & 0xff, val);
        return;
    case indx:
        cycles += 6;
        ad = getmem(pc++);
        ad += x;
        ad2 = getmem(ad & 0xff);
        ad++;
        ad2 |= getmem(ad & 0xff) << 8;
        setmem(ad2, val);
        return;
    case indy:
        cycles += 5;
        ad = getmem(pc++);
        ad2 = getmem(ad);
        ad2 |= getmem((ad + 1) & 0xff) << 8;
        ad = ad2 + y;
        setmem(ad, val);
        return;
    case acc:
        cycles += 2;
        a = val;
        return;
    }
}


static inline void setflags(int flag, int cond) {
    // cond?p|=flag:p&=~flag;
    if (cond)
        p |= flag;
    else
        p &= ~flag;
}


static inline void push(uint8_t val) {
    setmem(0x100 + s, val);
    if (s)
        s--;
}

static inline uint8_t pop() {
    if (s < 0xff)
        s++;
    return getmem(0x100 + s);
}

static void branch(int flag) {
    signed char dist;

    dist = (signed char)getaddr(imm);
    wval = pc + dist;
    if (flag) {
        cycles += ((pc & 0x100) != (wval & 0x100)) ? 2 : 1;
        pc = wval;
    }
}

// ----------------------------------------------------- ffentliche Routinen

void cpuReset() {
    a = x = y = 0;
    p = 0;
    s = 255;
    pc = getaddr(0xfffc);
}

void cpuResetTo(uint16_t npc) {
    a = 0;
    x = 0;
    y = 0;
    p = 0;
    s = 255;
    pc = npc;
}
int cpuParse() {
    uint8_t opc;
    int cmd, addr, c;

    cycles = 0;

    //#ifdef TRACE
    //  cpuStatus();
    //  if (opcodes[getmem(pc)]==xxx) getch();
    //#endif

    opc = getmem(pc++);
    cmd = opcodes[opc];
    addr = modes[opc];

    switch (cmd) {
    case adc:
        wval = (uint16_t) a + getaddr(addr) + ((p & FLAG_C) ? 1 : 0);
        setflags(FLAG_C, wval & 0x100);
        a = (uint8_t) wval;
        setflags(FLAG_Z, !a);
        setflags(FLAG_N, a & 0x80);
        setflags(FLAG_V, (!!(p & FLAG_C)) ^ (!!(p & FLAG_N)));
        break;
    case and:
        bval = getaddr(addr);
        a &= bval;
        setflags(FLAG_Z, !a);
        setflags(FLAG_N, a & 0x80);
        break;
    case asl:
        wval = getaddr(addr);
        wval <<= 1;
        setaddr(addr, (uint8_t) wval);
        setflags(FLAG_Z, !wval);
        setflags(FLAG_N, wval & 0x80);
        setflags(FLAG_C, wval & 0x100);
        break;
    case bcc:
        branch(!(p & FLAG_C));
        break;
    case bcs:
        branch(p & FLAG_C);
        break;
    case bne:
        branch(!(p & FLAG_Z));
        break;
    case beq:
        branch(p & FLAG_Z);
        break;
    case bpl:
        branch(!(p & FLAG_N));
        break;
    case bmi:
        branch(p & FLAG_N);
        break;
    case bvc:
        branch(!(p & FLAG_V));
        break;
    case bvs:
        branch(p & FLAG_V);
        break;
    case bit:
        bval = getaddr(addr);
        setflags(FLAG_Z, !(a & bval));
        setflags(FLAG_N, bval & 0x80);
        setflags(FLAG_V, bval & 0x40);
        break;
    case brk:
        push(pc & 0xff);
        push(pc >> 8);
        push(p);
        setflags(FLAG_B, 1);
        pc = getmem(0xfffe);
        cycles += 7;
        break;
    case clc:
        cycles += 2;
        setflags(FLAG_C, 0);
        break;
    case cld:
        cycles += 2;
        setflags(FLAG_D, 0);
        break;
    case cli:
        cycles += 2;
        setflags(FLAG_I, 0);
        break;
    case clv:
        cycles += 2;
        setflags(FLAG_V, 0);
        break;
    case cmp:
        bval = getaddr(addr);
        wval = (uint16_t) a - bval;
        setflags(FLAG_Z, !wval);
        setflags(FLAG_N, wval & 0x80);
        setflags(FLAG_C, a >= bval);
        break;
    case cpx:
        bval = getaddr(addr);
        wval = (uint16_t) x - bval;
        setflags(FLAG_Z, !wval);
        setflags(FLAG_N, wval & 0x80);
        setflags(FLAG_C, a >= bval);
        break;
    case cpy:
        bval = getaddr(addr);
        wval = (uint16_t) y - bval;
        setflags(FLAG_Z, !wval);
        setflags(FLAG_N, wval & 0x80);
        setflags(FLAG_C, a >= bval);
        break;
    case dec:
        bval = getaddr(addr);
        bval--;
        setaddr(addr, bval);
        setflags(FLAG_Z, !bval);
        setflags(FLAG_N, bval & 0x80);
        break;
    case dex:
        cycles += 2;
        x--;
        setflags(FLAG_Z, !x);
        setflags(FLAG_N, x & 0x80);
        break;
    case dey:
        cycles += 2;
        y--;
        setflags(FLAG_Z, !y);
        setflags(FLAG_N, y & 0x80);
        break;
    case eor:
        bval = getaddr(addr);
        a ^= bval;
        setflags(FLAG_Z, !a);
        setflags(FLAG_N, a & 0x80);
        break;
    case inc:
        bval = getaddr(addr);
        bval++;
        setaddr(addr, bval);
        setflags(FLAG_Z, !bval);
        setflags(FLAG_N, bval & 0x80);
        break;
    case inx:
        cycles += 2;
        x++;
        setflags(FLAG_Z, !x);
        setflags(FLAG_N, x & 0x80);
        break;
    case iny:
        cycles += 2;
        y++;
        setflags(FLAG_Z, !y);
        setflags(FLAG_N, y & 0x80);
        break;
    case jmp:
        cycles += 3;
        wval = getmem(pc++);
        wval |= 256 * getmem(pc++);
        switch (addr) {
        case abs:
            pc = wval;
            break;
        case ind:
            pc = getmem(wval);
            pc |= 256 * getmem(wval + 1);
            cycles += 2;
            break;
        }
        break;
    case jsr:
        cycles += 6;
        push((pc + 2));
        push((pc + 2) >> 8);
        wval = getmem(pc++);
        wval |= 256 * getmem(pc++);
        pc = wval;
        break;
    case lda:
        a = getaddr(addr);
        setflags(FLAG_Z, !a);
        setflags(FLAG_N, a & 0x80);
        break;
    case ldx:
        x = getaddr(addr);
        setflags(FLAG_Z, !x);
        setflags(FLAG_N, x & 0x80);
        break;
    case ldy:
        y = getaddr(addr);
        setflags(FLAG_Z, !y);
        setflags(FLAG_N, y & 0x80);
        break;
    case lsr:
        //bval=wval=getaddr(addr);
        bval = getaddr(addr);
        wval = (uint8_t) bval;
        wval >>= 1;
        setaddr(addr, (uint8_t) wval);
        setflags(FLAG_Z, !wval);
        setflags(FLAG_N, wval & 0x80);
        setflags(FLAG_C, bval & 1);
        break;
    case nop:
        cycles += 2;
        break;
    case ora:
        bval = getaddr(addr);
        a |= bval;
        setflags(FLAG_Z, !a);
        setflags(FLAG_N, a & 0x80);
        break;
    case pha:
        push(a);
        cycles += 3;
        break;
    case php:
        push(p);
        cycles += 3;
        break;
    case pla:
        a = pop();
        setflags(FLAG_Z, !a);
        setflags(FLAG_N, a & 0x80);
        cycles += 4;
        break;
    case plp:
        p = pop();
        cycles += 4;
        break;
    case rol:
        bval = getaddr(addr);
        c = !!(p & FLAG_C);
        setflags(FLAG_C, bval & 0x80);
        bval <<= 1;
        bval |= c;
        setaddr(addr, bval);
        setflags(FLAG_N, bval & 0x80);
        setflags(FLAG_Z, !bval);
        break;
    case ror:
        bval = getaddr(addr);
        c = !!(p & FLAG_C);
        setflags(FLAG_C, bval & 1);
        bval >>= 1;
        bval |= 128 * c;
        setaddr(addr, bval);
        setflags(FLAG_N, bval & 0x80);
        setflags(FLAG_Z, !bval);
        break;
    case rti:
        // NEU, rti wie rts, au�r das alle register wieder vom stack kommen
        //p=pop();                
        p = pop();
        y = pop();
        x = pop();
        a = pop();
        // in_nmi = false;
        //write_console("NMI EXIT!");
    case rts:
        wval = 256 * pop();
        wval |= pop();
        pc = wval;
        cycles += 6;
        break;
    case sbc:
        bval = getaddr(addr) ^ 0xff;
        wval = (uint16_t) a + bval + ((p & FLAG_C) ? 1 : 0);
        setflags(FLAG_C, wval & 0x100);
        a = (uint8_t) wval;
        setflags(FLAG_Z, !a);
        setflags(FLAG_N, a > 127);
        setflags(FLAG_V, (!!(p & FLAG_C)) ^ (!!(p & FLAG_N)));
        break;
    case sec:
        cycles += 2;
        setflags(FLAG_C, 1);
        break;
    case sed:
        cycles += 2;
        setflags(FLAG_D, 1);
        break;
    case sei:
        cycles += 2;
        setflags(FLAG_I, 1);
        break;
    case sta:
        putaddr(addr, a);
        break;
    case stx:
        putaddr(addr, x);
        break;
    case sty:
        putaddr(addr, y);
        break;
    case tax:
        cycles += 2;
        x = a;
        setflags(FLAG_Z, !x);
        setflags(FLAG_N, x & 0x80);
        break;
    case tay:
        cycles += 2;
        y = a;
        setflags(FLAG_Z, !y);
        setflags(FLAG_N, y & 0x80);
        break;
    case tsx:
        cycles += 2;
        x = s;
        setflags(FLAG_Z, !x);
        setflags(FLAG_N, x & 0x80);
        break;
    case txa:
        cycles += 2;
        a = x;
        setflags(FLAG_Z, !a);
        setflags(FLAG_N, a & 0x80);
        break;
    case txs:
        cycles += 2;
        s = x;
        break;
    case tya:
        cycles += 2;
        a = y;
        setflags(FLAG_Z, !a);
        setflags(FLAG_N, a & 0x80);
        break;
    }

    return cycles;
}

int cpuJSR(uint16_t npc, uint8_t na) {
    int ccl;

    a = na;
    x = 0;
    y = 0;
    p = 0;
    s = 255;
    pc = npc;
    push(0);
    push(0);
    ccl = 0;
    while (pc) {
        ccl += cpuParse();
    }
    return ccl;
}

void c64Init(void) {
    int i;

    synth_init(44100);

    for (i = 0; i < 65536; i++)
        memory[i] = 0;
    cpuReset();
}

unsigned short LoadSIDFromMemory(void *pSidData, unsigned short *load_addr,
                                 unsigned short *init_addr,
                                 unsigned short *play_addr,
                                 unsigned char *subsongs,
                                 unsigned char *startsong,
                                 unsigned char *speed, unsigned short size) {
    unsigned char *pData;
    unsigned char data_file_offset;

    pData = (unsigned char *)pSidData;
    data_file_offset = pData[7];

    *load_addr = pData[8] << 8;
    *load_addr |= pData[9];

    *init_addr = pData[10] << 8;
    *init_addr |= pData[11];

    *play_addr = pData[12] << 8;
    *play_addr |= pData[13];

    *subsongs = pData[0xf] - 1;
    *startsong = pData[0x11] - 1;

    *load_addr = pData[data_file_offset];
    *load_addr |= pData[data_file_offset + 1] << 8;

    *speed = pData[0x15];

    memset(memory, 0, sizeof(memory));
    memcpy(&memory[*load_addr], &pData[data_file_offset + 2],
           size - (data_file_offset + 2));

    if (*play_addr == 0) {
        cpuJSR(*init_addr, 0);
        *play_addr = (memory[0x0315] << 8) + memory[0x0314];
    }

    return *load_addr;
}

uint16_t c64SidLoad(char *filename, uint16_t * init_addr, uint16_t * play_addr,
                    uint8_t * sub_song_start, uint8_t * max_sub_songs,
                    uint8_t * speed, char *name, char *author,
                    char *copyright) {
    FILE *f;
    int i;

    if ((f = fopen(filename, "rb")) == NULL)
        return (0);
    // Name holen
    fseek(f, 0x16, 0);
    for (i = 0; i < 32; i++)
        name[i] = fgetc(f);

    // Author holen
    fseek(f, 0x36, 0);
    for (i = 0; i < 32; i++)
        author[i] = fgetc(f);

    // Copyright holen
    fseek(f, 0x56, 0);
    for (i = 0; i < 32; i++)
        copyright[i] = fgetc(f);
    fclose(f);


    unsigned char sidmem[65536];
    unsigned char *p = sidmem;
    unsigned short load_addr;

    if ((f = fopen(filename, "rb")) == NULL)
        return (0);
    while (!feof(f))
        *p++ = fgetc(f);
    fclose(f);

    LoadSIDFromMemory(sidmem, &load_addr,
                      init_addr, play_addr, max_sub_songs, sub_song_start,
                      speed, p - sidmem);
    name = NULL;
    author = NULL;
    copyright = NULL;


    return load_addr;
}
