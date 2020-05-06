target remote | openocd -c "gdb_port pipe; log_output openocd.log" -f openocd.cfg
#target extended :3333

set confirm off

#monitor mt_flash main.elf

file main.elf
monitor reset halt
#set $pc=0x8002080

# define hook-quit
#     set confirm off
# end

define reconnect
  target extended :3333
end

define reset
  monitor reset halt
end

define reprogram
  monitor mt_flash main.elf
  file main.elf
end

set print elements 0

define dump_acq
  set logging file data.txt
  set logging on
  p speed_acq
  p I_ref_acq
  p Ik_acq
  p Ik1_acq
  p dc_acq
  set logging off
end

