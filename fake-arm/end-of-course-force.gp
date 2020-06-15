set xrange [-1e-3:10e-3]

Fpos = 1e-3
Fneg = 100
xneg = -1e-3
ka = log(Fpos / (Fneg + 1)) / xneg
fa(x) = Fpos*(exp(-x*ka) - 1)

plot fa(x)
