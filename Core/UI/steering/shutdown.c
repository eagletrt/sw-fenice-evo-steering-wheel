#include "shutdown.h"

char shutdown_labels[SHUTDOWN_COMPONENT_SIZE][20] = {"SD start",
                                                     "SD in",
                                                     "SD out",
                                                     "TSAC interlock",
                                                     "Mushroom L+R",
                                                     "Mushroom cock",
                                                     "Inertial switch",
                                                     "BOTS",
                                                     "INVC Lid",
                                                     "HVD",
                                                     "BSPD",
                                                     "INVC motos interlock",
                                                     "TSMS[LV]",
                                                     "TSMS[HV]",
                                                     "Precharge",
                                                     "AIR P",
                                                     "AIR N"};

bool shutdown_status_array[SHUTDOWN_COMPONENT_SIZE] = {false};

void shutdown_circuit_update() {
  int last_opend_index = -1;
  for (int i = 0; i < SHUTDOWN_COMPONENT_SIZE; i++) {
    if (shutdown_status_array[i]) {
      last_opend_index = i;
      break;
    }
  }
  if (last_opend_index == -1) {
    set_tab_hv_label_text("SHUTDOWN CLOSE", shutdown_status);
  } else {
    set_tab_hv_label_text(shutdown_labels[last_opend_index], shutdown_status);
  }
}

void update_shutdown_circuit_component(shutdown_circuit_indexes_t idx,
                                       float shutdown_circuit_state) {
  shutdown_status_array[idx] =
      !(shutdown_circuit_state <
        12.0f); // 12V is the value for considering the shutdown circuit closed
  shutdown_circuit_update();
}