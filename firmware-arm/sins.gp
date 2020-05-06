V = 220
A120 = 2 * pi / 3;
M_sqrt_2_3 = sqrt(2.0/3)
M_sqrt3_2 = sqrt(3)/2
vab(x) = sqrt(2) * V * sin (x)
vbc(x) = sqrt(2) * V * sin (x - A120)
vca(x) = -vbc(x) - vab(x)
valpha(a,b,c) = M_sqrt_2_3 * (a - 0.5 * b - 0.5 * c);
vbeta(a,b,c) = M_sqrt_2_3 * (M_sqrt3_2 * b - M_sqrt3_2 * c);
mod(a,b,c) = sqrt(valpha(a,b,c)**2 + vbeta(a,b,c)**2)
set xrange[0:2*pi]
plot vab(x), vbc(x), vca(x), valpha(vab(x), vbc(x), vca(x)), vbeta(vab(x), vbc(x), vca(x)), mod(vab(x), vbc(x), vca(x))/sqrt(3)