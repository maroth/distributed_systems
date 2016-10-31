import csv
import sys
import os
import collections
import itertools

plot_template= """
set term epslatex
# some line types with different colors, you can use them by using line styles in the plot command afterwards (linestyle X)
set style line 1 lt 1 lc rgb "#FF0000" lw 7 # red
set style line 2 lt 1 lc rgb "#00FF00" lw 7 # green
set style line 3 lt 1 lc rgb "#0000FF" lw 7 # blue
set style line 4 lt 1 lc rgb "#000000" lw 7 # black

set output "{matrix_size}_{cores}.eps"
set title "N={matrix_size} Cores={cores}"

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

plot "{matrix_size}_{cores}.aggregated" u ($1):($5) with lines linestyle 1 title "total time", \
     "{matrix_size}_{cores}.aggregated" u ($1):($2) with lines linestyle 2 title "init time", \
     "{matrix_size}_{cores}.aggregated" u ($1):($3) with lines linestyle 3 title "send time", \
     "{matrix_size}_{cores}.aggregated" u ($1):($4) with lines linestyle 4 title "compute time", \
"""

table_template = """\\begin{table}[]
\centering
\\begin{tabular}{|l|l|l|l|}
\hline
Workers & Initialisation (s) & Sending (s) & Computing (s)\\\\ \hline
1 & {1_init} & {1_send} & {1_comp} \\\\ \hline
2 & {2_init} & {2_send} & {2_comp} \\\\ \hline
4 & {3_init} & {3_send} & {3_comp} \\\\ \hline
8 & {4_init} & {4_send} & {4_comp} \\\\ \hline
16 & {5_init} & {5_send} & {5_comp} \\\\ \hline
\end{tabular}
\caption{N={matrix_size} Cores={cores}}
\label{my-label}
\end{table}
"""

result = {}

for outputfile in os.listdir("../results/"):
    if outputfile.endswith(".txt"):
        with open("../results/" + outputfile) as values:
            matrix_size = None
            cores = None
            count = 0
            workers = None
            init_time_sum = float(0)
            send_time_sum = float(0)
            compute_time_sum = float(0)
            for line in csv.reader(values, delimiter="\t"):
                if len(line) > 1:
                    count += 1
                    matrix_size = int(line[0] )
                    cores = int(line[6])
                    blocks_a = line[1]
                    blocks_b = line[2]
                    workers = int(blocks_a) * int(blocks_b)
                    init_time_sum += float(line[3])
                    send_time_sum += float(line[4])
                    compute_time_sum += float(line[5])
                    entry = (init_time_sum / count, send_time_sum / count, compute_time_sum / count)
                    result[(matrix_size, cores, workers)] = entry


sorted_result = collections.OrderedDict(sorted(result.items()))

for matrix_size, cores_and_workers in itertools.groupby(sorted_result, lambda x: x[0]):
    for cores, worker in itertools.groupby(cores_and_workers, lambda x: x[1]):
        data_file= "{}_{}.aggregated".format(matrix_size, cores)
        table_file= "{}_{}.table".format(matrix_size, cores)
        print("Generating for N={} and Cores={}".format(matrix_size, cores))
        with open(data_file, "w") as outputfile:
            with open(table_file, "w") as tablefile:
                table = table_template.replace("{matrix_size}", str(matrix_size))
                table = table.replace("{cores}", str(cores))
                table = table.replace("my-label", "{}-{}".format(matrix_size, cores))
                count = 0
                for item in worker:
                    count += 1
                    times = result[item]
                    init_time = times[0]
                    send_time = times[1]
                    compute_time = times[2]
                    total_time = init_time + send_time + compute_time
                    outputfile.write("{} {} {} {} {}\n".format(item[2], init_time, send_time, compute_time, total_time))
                    table = table.replace("{" + str(count) + "_init}", str(init_time))
                    table = table.replace("{" + str(count) + "_send}", str(send_time))
                    table = table.replace("{" + str(count) + "_comp}", str(compute_time))
                tablefile.write(table)

        plot_file = "{}_{}.gp".format(matrix_size, cores)
        with open(plot_file, "w") as plotfile:
            plot = plot_template.replace("{matrix_size}", str(matrix_size))
            plot = plot.replace("{cores}", str(cores))
            plotfile.write(plot)
        os.system("gnuplot {}".format(plot_file))

