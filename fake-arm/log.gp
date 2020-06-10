fn = 'log.dat'
set yrange [-17:1.25]
set ytics('0' 0, '1' 0.25, '2' 0.5, '3' 0.75, '4' 1, 'V1' -1, 'V2' -2, 'V3' -3, 'V4' -4, 'V5' -5, 'V6' -6, '10 cm' xe0+2, '5 cm' xe0+1, '0' xe0, '300 rd/s' omega0+3, '200 rd/s' omega0+2, '100 rd/s' omega0+1, '0' omega0, '-400 rd/s' omega0-4, '1 N.m' Tel0+1, '0' Tel0, '-1 N.m' Tel0-1)
xe0 = -9
omega0 = xe0 - 4
Tel0 = omega0 - 2

plot fn u 1:($2/4) with points pt 'o' title 'phase', \
     fn u 1:($3*.5-1) w lines notitle, \
     fn u 1:($4*.5-2) w lines notitle, \
     fn u 1:($5*.5-3) w lines notitle, \
     fn u 1:($6*.5-4) w lines notitle, \
     fn u 1:($7*.5-5) w lines notitle, \
     fn u 1:($8*.5-6) w lines notitle, \
     fn u 1:($11*20 + xe0) w lines title 'x_e', \
     fn u 1:($12*0.01 + omega0) w lines title 'omega_m', \
     fn u 1:($13 + Tel0) w lines title 'Tel'
pause 1
reread
