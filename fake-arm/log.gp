fn = 'log.dat'

phase0 = 6
V10 = 5.5
V20 = 5
V30 = 4.5
V40 = 4
V50 = 3.5
V60 = 3
xe0 = 0
omega0 = xe0 - 4
Tel0 = omega0 - 2
rho0 = Tel0 - 3
P0 = rho0 - 2

set yrange [P0-1.5:phase0 + 1.25]
set autoscale x

set ytics('0' phase0, '1' phase0 + 0.25, '2' phase0 + 0.5, '3' phase0 + 0.75, '4' phase0 + 1, 'V1' V10, 'V2' V20, 'V3' V30, 'V4' V40, 'V5' V50, 'V6' V60, '10 cm' xe0+2, '5 cm' xe0+1, 'x 0' xe0, '300 rd/s' omega0+3, '200 rd/s' omega0+2, '100 rd/s' omega0+1, 'omega_m 0' omega0, '1 N.m' Tel0+1, 'T_{el} 0' Tel0, '-1 N.m' Tel0-1, '100kPa' rho0+1, 'rho_e 0' rho0, '100 W' P0+1, 'P_m 0' P0, '-100 W' P0-1)
set pointsize 1

plot fn u 1:($2/4 + phase0) with points title 'phase', \
     fn u 1:($3*.25 + V10) w lines notitle, \
     fn u 1:($4*.25 + V20) w lines notitle, \
     fn u 1:($5*.25 + V30) w lines notitle, \
     fn u 1:($6*.25 + V40) w lines notitle, \
     fn u 1:($7*.25 + V50) w lines notitle, \
     fn u 1:($8*.25 + V60) w lines notitle, \
     fn u 1:($9*20 + xe0) w lines title 'x_{ref}', \
     fn u 1:($10*20 + xe0) w lines title 'x_e', \
     fn u 1:($11*0.01 + omega0) w lines notitle, \
     fn u 1:($12*0.01 + omega0) w lines title 'omega_{ref}', \
     fn u 1:($13 + Tel0) w lines notitle, \
     fn u 1:($14*1e-5 + rho0) w lines notitle, \
     fn u 1:($15*1e-2 + P0) w lines notitle
