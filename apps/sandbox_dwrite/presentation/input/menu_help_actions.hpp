#ifndef MENU_HELP_ACTIONS_H
#define MENU_HELP_ACTIONS_H

typedef struct {
    int* show_help_modal;
    int* show_config_modal;
    int* help_modal_page;
} MenuHelpOps;

int menu_help_execute(int menu_id, int item_idx, const MenuHelpOps* ops);

#endif
