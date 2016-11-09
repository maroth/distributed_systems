set term epslatex 
set style line 1 lt 1 lc rgb "#FF0000" lw 7 # red
set style line 2 lt 1 lc rgb "#00FF00" lw 7 # green
set style line 3 lt 1 lc rgb "#0000FF" lw 7 # blue
set style line 4 lt 1 lc rgb "#000000" lw 7 # black
set style line 5 lt 1 lc rgb "#CD00CD" lw 7 # purple
set style line 7 lt 3 lc rgb "#000000" lw 7 # black, dashed line

# indicates the labels
set xlabel "Matrix Size"
set ylabel "Seconds"

set size 1.0, 1.0

# set the grid on
set grid x,y

# set the key, options are top/bottom and left/right
set key top left

# indicates the ranges
set yrange [0:] 
set xrange [0:] 

set output "seq_init.eps"
set title "Sequential Initialisation Times"

plot "seq.aggregated" u ($1):($2) with lines linestyle 1 title ""

set output "seq_send.eps"
set title "Sequential Send Times"
plot "seq.aggregated" u ($1):($3) with lines linestyle 1 title ""

set output "seq_compute.eps"
set title "Sequential Compute Times"
plot "seq.aggregated" u ($1):($4) with lines linestyle 1 title ""

set output "seq_total.eps"
set title "Sequential Total Times"
plot "seq.aggregated" u ($1):($5) with lines linestyle 1 title ""
