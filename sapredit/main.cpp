#include <cstdio>
#include <cstdint>
#include <climits>
#include <vector>
#include <memory>
#include <SDL.h>
#include "mzpokey.h"
#include "fltk.h"

static struct mzpokey_context *mzp;

using namespace std;

// ****************************************************************************
// SAP-R TABLE CLASS
//
class MyTable : public Fl_Table_Row {
public:
    MyTable(int x, int y, int w, int h, const char *l = nullptr);

    vector<vector<uint8_t>> values;

    int handle(int event);
    void selrow(int row);

protected:
    void draw_cell(TableContext context, int R=0, int C=0, int X=0, int Y=0,
                                                           int W=0, int H=0);

private:
    bool shift_down;
    bool ctrl_down;
    void determine_selection(int &left, int &right, int &top, int &bottom);
    void clear_cells(void);
    void delete_lines(void);
    void insert_line(void);
    void insert_line_after(void);
    void edit_cells(int key);
};

// ****************************************************************************
// SAP-R TABLE CONSTRUCTOR
//
MyTable::MyTable(int x, int y, int w, int h, const char *l)
    : Fl_Table_Row(x, y, w, h, l) {

    shift_down = false;
    ctrl_down = false;

    row_header(1);
    row_header_width(64);
    rows(0);

    col_header(1);
    cols(9);
    col_width_all(32);
    end();
}

// DRAW SAP-R EDITOR WINDOW ***************************************************

void MyTable::draw_cell(TableContext context, int R, int C, int X, int Y,
                                                            int W, int H) {
    static char s[32];
    static const char *coltxt[9] = {
        "F1", "C1", "F2", "C2", "F3", "C3", "F4", "C4", "CTL"
    };
    switch (context) {
    case CONTEXT_COL_HEADER:
    case CONTEXT_ROW_HEADER:
        fl_push_clip(X,Y,W,H);
        {
            Fl_Color c = (context == CONTEXT_COL_HEADER) ?
                            col_header_color() :
                            row_header_color();
            fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, c);
            fl_color(FL_BLACK);
            if (context == CONTEXT_COL_HEADER) {
                fl_draw(coltxt[C], X, Y, W, H, FL_ALIGN_CENTER);
            } else {
                snprintf(s, 32, "%06d",  R);
                fl_draw(s, X, Y, W, H, FL_ALIGN_CENTER);
            }
        }
        fl_pop_clip();
        break;
    case CONTEXT_CELL:
        fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, is_selected(R,C) ?
                fl_rgb_color(0x38,0x70,0xe0) : FL_WHITE);
        fl_push_clip(X+3, Y+3, W-6, H-6);
        {
            fl_color(FL_BLACK);
            snprintf(s, 32, "%02x", values[R][C]);
            fl_draw(s, X+3, Y+3, W-6, H-6, FL_ALIGN_CENTER);
        }
        fl_pop_clip();
        break;
    default:
        break;
    }
    return;
}

// SELECT ROW *****************************************************************

void MyTable::selrow(int row) {
    if (row < rows()) {
        set_selection(row,0, row, cols()-1);
    }
}

void MyTable::determine_selection(int &left, int &right, int &top, int &bottom){
    left = top = INT_MAX;
    right = bottom = -INT_MAX;
    for (int r=0; r<rows(); r++) {
        for (int c=0; c<9; c++) {
            if (is_selected(r,c)) {
                if (c < left)   left = c;
                if (c > right)  right = c;
                if (r < top)    top = r;
                if (r > bottom) bottom = r;
            }
        }
    }
}

// CLEAR CELLS ****************************************************************

void MyTable::clear_cells(void) {
    int left, right, top, bottom;
    determine_selection(left, right, top, bottom);
    if (right < 0) return;
    for (int r=top; r<=bottom; r++) {
        for (int c=left; c<=right; c++) {
            values[r][c] = 0;
        }
    }
    redraw();
}

// DELETE LINES ***************************************************************

void MyTable::delete_lines(void) {
    int left, right, top, bottom;
    determine_selection(left, right, top, bottom);
    if (right < 0) return;
    values.erase(values.begin() + top, values.begin() + bottom + 1);
    rows(values.size());
    set_selection(top,0,top,0);
    redraw();
}

// INSERT EMPTY LINE **********************************************************

void MyTable::insert_line(void) {
    int left, right, top, bottom;
    determine_selection(left, right, top, bottom);
    if (right < 0) {
        if (values.size() != 0) {
            return;
        }
        values.push_back({ 0,0,0,0,0,0,0,0,0 });
    } else {
        values.insert(values.begin() + top, { 0,0,0,0,0,0,0,0,0 });
    }
    rows(values.size());
    set_selection(top,0,top,0);
    redraw();
}

// INSERT EMPTY LINE AFTER ****************************************************

void MyTable::insert_line_after(void) {
    int left, right, top, bottom;
    determine_selection(left, right, top, bottom);
    if (right < 0) return;
    if (bottom + 1 >= (int) values.size()) {
        values.push_back({ 0,0,0,0,0,0,0,0,0 });
    } else {
        values.insert(values.begin() + bottom + 1, { 0,0,0,0,0,0,0,0,0 });
    }
    rows(values.size());
    set_selection(bottom+1, 0, bottom+1, 0);
    redraw();
}

// EDIT CELL(S) ***************************************************************

void MyTable::edit_cells(int key) {
    int left, right, top, bottom;
    determine_selection(left, right, top, bottom);
    if (right < 0) return;
    for (int r=top; r<=bottom; r++) {
        for (int c=left; c<=right; c++) {
            values[r][c] <<= 4;
            values[r][c] += key <= '9' ? key - '0' : key - 'a' + 10;
            values[r][c] &= 0xff;
        }
    }
    redraw();
}

// HANDLE EVENTS **************************************************************

int MyTable::handle(int event) {
    int ret = Fl_Table_Row::handle(event);
    int key = Fl::event_key();
    switch (event) {
    case FL_ENTER:
        take_focus();
        return 1;
        break;
    case FL_KEYDOWN:
        if (key == FL_Shift_L || key == FL_Shift_R) {
            shift_down = true;
            return 1;
        } else if (key == FL_Control_L || key == FL_Control_R) {
            ctrl_down = true;
            return 1;
        } else if (key == FL_Delete) {
            delete_lines();
            return 1;
        } else if (key == FL_BackSpace) {
            clear_cells();
            return 1;
        } else if (key == FL_Insert) {
            if (shift_down) {
                insert_line_after();
                return 1;
            } else {
                insert_line();
                return 1;
            }
        } else if ((key >= '0' && key <= '9') ||
                   (key >= 'a' && key <= 'f')) {
            edit_cells(key);
            return 1;
        }
        break;
    case FL_KEYUP:
        if (key == FL_Shift_L || key == FL_Shift_R) {
            shift_down = false;
            return 1;
        } else if (key == FL_Control_L || key == FL_Control_R) {
            ctrl_down = false;
            return 1;
        }
        break;
    case FL_FOCUS:
    case FL_UNFOCUS:
        return 1;
        break;
    default:
        break;
    }
    return ret;
}

// ****************************************************************************
// EDITOR WINDOW CLASS
//
class SaprEditWindow : public Fl_Double_Window {
public:
    SaprEditWindow(const char *filename);
    ~SaprEditWindow(void);
    MyTable *table = nullptr;
    char *filename = nullptr;
private:
};

// ****************************************************************************
// EDITOR CALLBACKS
//
static void CloseSaprEditWindow(Fl_Widget *w, void *data) {
    delete w;
}

static MyTable *playtable;
static int playpos = -1;

// PLAY BUTTON ****************************************************************

static void PlayButtonCallback(Fl_Widget *w, void *data) {
    auto sew = (SaprEditWindow *) data;
    auto tbl = sew->table;
    int start_at = -1;
    for (int r=0; r<tbl->rows() && start_at == -1; r++) {
        for (int c=0; c<9 && start_at == -1; c++) {
            if (tbl->is_selected(r, c)) {
                start_at = r;
            }
        }
    }
    if (start_at == -1) start_at = 0;

    playtable = tbl;
    playpos = start_at;
}

// REWIND BUTTON **************************************************************

static void RewButtonCallback(Fl_Widget *w, void *data) {
    auto sew = (SaprEditWindow *) data;
    if (sew->table == playtable) {
        playpos = 0;
        playtable->selrow(playpos);
        playtable->top_row(playpos);
    } else {
        sew->table->selrow(0);
        sew->table->top_row(0);
    }
}

// FORWARD BUTTON *************************************************************

static void FwdButtonCallback(Fl_Widget *w, void *data) {
    auto sew = (SaprEditWindow *) data;
    if (sew->table == playtable) {
        playpos = playtable->values.size() - 1;
        playtable->selrow(playpos);
        playtable->top_row(playpos);
    } else {
        int pos = sew->table->values.size() - 1;
        sew->table->selrow(pos);
        sew->table->top_row(pos);
    }
}

// STOP BUTTON ****************************************************************

static void StopButtonCallback(Fl_Widget *w, void *data) {
    auto sew = (SaprEditWindow *) data;
    if (sew->table == playtable) {
        playtable = nullptr;
    }
}

// PAUSE BUTTON ***************************************************************

static void PauseButtonCallback(Fl_Widget *w, void *data) {
    auto sew = (SaprEditWindow *) data;
    if (sew->table == playtable) {
        int stoppos = playpos;
        playtable->selrow(stoppos);
        playtable->top_row(stoppos);
        playtable = nullptr;
    }
}

// SAVE AS BUTTON *************************************************************

#define SAPR_HEADER "SAP\r\nAUTHOR \"\"\r\nNAME \"\"\r\nDATE \"\"\r\nTYPE R\r\n\r\n"

static void SaveAsButtonCallback(Fl_Widget *w, void *data) {
    auto sew = (SaprEditWindow *) data;
    char *filename = fl_file_chooser("Save as...", "*.sapr", sew->filename);

    if (!filename) return;

    FILE *output = fopen(filename, "wb");
    if (!output) goto err_out;

    if (fwrite(SAPR_HEADER, strlen(SAPR_HEADER), 1, output) != 1) goto err_out;

    for (unsigned int r=0; r<sew->table->values.size(); r++) {
        for (int c=0; c<9; c++) {
            if (fputc(sew->table->values[r][c], output) < 0) goto err_out;
        }
    }
    fclose(output);

    fl_alert("File saved.");
    if (sew->filename) {
        free(sew->filename);
    }
    sew->filename = strdup(filename);
    sew->label(filename);
    return;

err_out:
    if (output) fclose(output);
    fl_alert("Error saving file!");
}

// ****************************************************************************
// EDITOR WINDOW
//

#define EDITOR_HEIGHT 768

SaprEditWindow::SaprEditWindow(const char *filename)
    : Fl_Double_Window(400+256, EDITOR_HEIGHT) {

    callback(CloseSaprEditWindow, this);
    label(filename);

    FILE *input = fopen(filename, "rb");
    if (!input) {
        fl_alert("Failed to open '%s'\n", filename);
        throw(0);
    }

    fseek(input, 0, SEEK_END);
    int filesize = ftell(input);
    fseek(input, 0, SEEK_SET);

    uint8_t *data = (uint8_t *) malloc(filesize);

    if (!data) {
        fl_alert("Out of memory!");
        throw(0);
    }

    if (fread(data, filesize, 1, input) != 1) {
        fl_alert("Error reading '%s'\n", filename);
        free(data);
        throw(0);
    }
    fclose(input);

    uint8_t *endheader = (uint8_t *) strstr((char *) data, "\r\n\r\n");
    uint8_t *saprdata = nullptr;
    int saprdatasize = 1;
    if (endheader) {
        saprdata = endheader + 4;
        saprdatasize = filesize - (saprdata - data);
    }

    if (memcmp(data, "SAP", 3) || !saprdata || (saprdatasize % 9 != 0)) {
        fl_alert("Not a SAP-R file!\n");
        free(data);
        throw(0);
    }

    table = new MyTable(8, 8, 384, 768-16);

    for (int i=0; i<saprdatasize; i += 9) {
        table->values.push_back({saprdata[i+0], saprdata[i+1], saprdata[i+2],
                                 saprdata[i+3], saprdata[i+4], saprdata[i+5],
                                 saprdata[i+6], saprdata[i+7], saprdata[i+8]});
        table->rows(table->values.size());
    }
    table->selrow(0);

    free(data);
    this->filename = strdup(filename);

    int curx = 400;
    int cury = 8;

    auto rew_img = new Fl_PNG_Image("png/rew.png");
    auto play_img = new Fl_PNG_Image("png/play.png");
    auto stop_img = new Fl_PNG_Image("png/stop.png");
    auto pause_img = new Fl_PNG_Image("png/pause.png");
    auto fwd_img = new Fl_PNG_Image("png/fwd.png");

    auto rew = new Fl_Button(curx, cury, 30, 30);
    rew->callback(RewButtonCallback, this);
    rew->image(rew_img);
    curx += 32;

    auto play = new Fl_Button(curx, cury, 30, 30);
    play->callback(PlayButtonCallback, this);
    play->image(play_img);
    curx += 32;

    auto stop = new Fl_Button(curx, cury, 30, 30);
    stop->callback(StopButtonCallback, this);
    stop->image(stop_img);
    curx += 32;

    auto pause = new Fl_Button(curx, cury, 30, 30);
    pause->callback(PauseButtonCallback, this);
    pause->image(pause_img);
    curx += 32;

    auto fwd = new Fl_Button(curx, cury, 30, 30);
    fwd->callback(FwdButtonCallback, this);
    fwd->image(fwd_img);

    curx = 400;
    cury = 128;

    auto save_as = new Fl_Button(curx, cury, 94, 24, "Save as...");
    save_as->callback(SaveAsButtonCallback, this);

    Fl_Box *tb;

    const char *t[] = {
        "Cursors - Movement",
        "Shift + Cursors - Select",
        "[0-9][a-z][A-Z] - edit cell",
        "BackSpace - clear cell(s)",
        "DEL - delete line(s)",
        "INS - insert line",
        "Shift+INS - insert line after"
    };
    curx = 400;
    cury = EDITOR_HEIGHT - 4 - 7*20;
    for (auto txt : t) {
        tb = new Fl_Box(curx, cury, 256, 20, txt);
        tb->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        cury += 20;
    }

    end();
    show();
}

SaprEditWindow::~SaprEditWindow(void) {
    if (filename) {
        free(filename);
    }
    if (playtable == table) {
        playtable = nullptr;
    }
}

// ****************************************************************************
// SPAWN NEW EDITOR WINDOW
//
void LoadFileButton(Fl_Widget *w, void *data) {
    char *filename = fl_file_chooser("Load SAP-R File", "*.sapr", nullptr);

    if (filename) {
        try {
            new SaprEditWindow(filename);
        }
        catch(int num) {
        }
    }
}

// ****************************************************************************

static void fill_audio(void *udata, Uint8 *stream, int len) {
    if (playtable && playpos < playtable->rows()) {
        for (int i=0; i<9; i++) {
            mzpokey_write_register(mzp, (pokey_register) i,
                                        playtable->values[playpos][i], 0);
        }
        playpos++;
    } else if (playtable) {
        playtable = nullptr;
    } else {
        for (int i=0; i<9; i++) {
            mzpokey_write_register(mzp, (pokey_register) i, 0, 0);
        }
    }
    mzpokey_process_float(mzp, stream, len/sizeof(float));
}

// ****************************************************************************
// MAIN WINDOW
//
#define WIDTH 400
#define HEIGHT 128

int main(int argc, char **argv) {
    mzp= mzpokey_create(1773447, 44100, 1, 0);
    if (!mzp) {
        fprintf(stderr, "Error: unable to create mzpokey context!\n");
        return 1;
    }
    mzpokey_write_register(mzp, SKCTL, 3, 0);

    SDL_AudioSpec wanted;
    wanted.freq = 44100;
    wanted.format = AUDIO_F32SYS;
    wanted.channels = 1;
    wanted.samples = 882;               // one 50Hz frame @ 44100
    wanted.callback = fill_audio;
    wanted.userdata = NULL;

    if (SDL_OpenAudio(&wanted, nullptr) < 0) {
        fprintf(stderr, "Error: couldn't open audio: %s\n", SDL_GetError());
        return 1;
    }
    SDL_PauseAudio(0);

    auto window = new Fl_Double_Window(WIDTH, HEIGHT, "SAP-R Editor");
    int cury = 16;

    auto title = new Fl_Box(WIDTH/2-128, cury, 256, 48, "SAP-R Editor");
    title->labelsize(48);
    title->labelfont(FL_ITALIC | FL_BOLD);
    title->labelcolor(fl_rgb_color(0x20,0x40,0x80));
    cury += 48;

    auto copyright = new Fl_Box(WIDTH/2-128, cury,
                        256, 16, "Copyright © 2025 by Ivo van Poorten");
    copyright->labelsize(12);
    copyright->labelfont(FL_ITALIC);
    cury += 32;

    auto load = new Fl_Button(WIDTH/2-64, cury, 128, 20, "Load File");
    load->callback(LoadFileButton, window);

    window->end();
    window->show();

    return Fl::run();
}
