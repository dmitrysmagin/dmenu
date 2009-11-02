enum MenuSound { MENU_MOVE = 0, MENUITEM_MOVE = 1, DECIDE = 2, CANCEL = 3, OUT = 4, TEST = 5};

void SE_Init();
void SE_out(enum MenuSound seNum); 
void SE_deInit();
