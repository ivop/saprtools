#pragma once
#include <vector>
#include "fltk.h"

class MyTable : public Fl_Table_Row {
public:
    MyTable(int x, int y, int w, int h, const char *l = nullptr);

    std::vector<std::vector<uint8_t>> values;

    int handle(int event);
    void selrow(int row);

    bool audible[4];

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
    void copy_cells(void);
    void paste_cells(void);
};

class SaprEditWindow : public Fl_Double_Window {
public:
    SaprEditWindow(const char *filename);
    ~SaprEditWindow(void);
    MyTable *table = nullptr;
    char *filename = nullptr;
    Fl_Check_Button *audible[4];
private:
};

class MainWindow : public Fl_Double_Window {
public:
    MainWindow(const char *l = nullptr);

    void update_paste_buffer_stats(void);
private:
    Fl_Box *wvalue;
    Fl_Box *hvalue;
};
