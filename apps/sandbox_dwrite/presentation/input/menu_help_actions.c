#include <stddef.h>
#include "menu_help_actions.h"

int menu_help_execute(int menu_id, int item_idx, const MenuHelpOps* ops) {
    if (ops == NULL) return 0;
    if (menu_id != 7) return 0;
    if (ops->show_help_modal != NULL) *ops->show_help_modal = 1;
    if (ops->show_config_modal != NULL) *ops->show_config_modal = 0;
    if (ops->help_modal_page != NULL) *ops->help_modal_page = item_idx;
    return 1;
}
