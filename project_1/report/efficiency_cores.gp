set term epslatex 
set style line 1 lt 1 lc rgb "#FF0000" lw 7 # red
set style line 2 lt 1 lc rgb "#00FF00" lw 7 # green
set style line 3 lt 1 lc rgb "#0000FF" lw 7 # blue
set style line 4 lt 1 lc rgb "#000000" lw 7 # black
set style line 5 lt 1 lc rgb "#CD00CD" lw 7 # purple
set style line 7 lt 3 lc rgb "#000000" lw 7 # black, dashed line

set output "efficiency_cores.eps"
set title "Efficiency Comparison (4 Core per Machine)"

# indicates the labels
set xlabel "Workers (4 Cores per Worker)"
set ylabel "Efficiency"

set size 1.0, 1.0

# set the grid on
set grid x,y

# set the key, options are top/bottom and left/right
set key top right

# indicates the ranges
set yrange [0:] 
set xrange [0:] 

plot "efficiency_cores.aggregated" u ($1):($2) with lines linestyle 1 title "N=1152", \
     "efficiency_cores.aggregated" u ($1):($3) with lines linestyle 2 title "N=1440", \
     "efficiency_cores.aggregated" u ($1):($4) with lines linestyle 3 title "N=2304", \
     "efficiency_cores.aggregated" u ($1):($5) with lines linestyle 4 title "N=3600", \
     "efficiency_cores.aggregated" u ($1):($6) with lines linestyle 5 title "N=5040"
