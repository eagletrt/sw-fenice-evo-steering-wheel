# Fenice Evo Steering Wheel

Using [this project](https://github.com/lvgl/lv_port_pc_vscode) as template.

## Contributors

- Giacomo Mazzucchi
- Francesco Olivieri
- Federico Adami

## TO MAKE CAN SIMULATOR WORK:

go to project main folder and run following commands:

```
./can.sh
canplayer vcan0=primary vcan1=secondary -I candump.log -l i

```
in a new tab launch the simulatore of the ui with "./run.sh"
