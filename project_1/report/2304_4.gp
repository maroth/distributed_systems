
set term epslatex
# some line types with different colors, you can use them by using line styles in the plot command afterwards (linestyle X)
set style line 1 lt 1 lc rgb "#FF0000" lw 7 # red
set style line 2 lt 1 lc rgb "#00FF00" lw 7 # green
set style line 3 lt 1 lc rgb "#0000FF" lw 7 # blue
set style line 4 lt 1 lc rgb "#000000" lw 7 # black

set output "2304_4.eps"
set title "N=2304 Cores=4"

# indicates the labels
set xlabel "Workers"
set ylabel "Total time (s)"

set size 1.0, 1.0

# set the grid on
set grid x,y

# set the key, options are top/bottom and left/right
set key top left

# indicates the ranges
set yrange [0:] # example of a closed range (points outside will not be displayed)
set xrange [1:] # example of a range closed on one side only, the max will determined automatically

plot "2304_4.aggregated" u ($1):($5) with lines linestyle 1 title "total time",      "2304_4.aggregated" u ($1):($2) with lines linestyle 2 title "init time",      "2304_4.aggregated" u ($1):($3) with lines linestyle 3 title "send time",      "2304_4.aggregated" u ($1):($4) with lines linestyle 4 title "compute time", 