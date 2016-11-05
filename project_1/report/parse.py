import csv
import sys
import os
import collections
import itertools

plot_template= """
set term epslatex 
set style line 1 lt 1 lc rgb "#FF0000" lw 7 # red
set style line 2 lt 3 lc rgb "#00FF00" lw 5 # green
set style line 3 lt 3 lc rgb "#0000FF" lw 5 # blue
set style line 4 lt 3 lc rgb "#000000" lw 5 # black
set style line 5 lt 3 lc rgb "#CD00CD" lw 3 # purple
set style line 7 lt 3 lc rgb "#000000" lw 3 # black, dashed line

set output "{matrix_size}_{cores}.eps"
set title "N={matrix_size} {cores_label}"

# indicates the labels
set xlabel "{x-axis-label}"
set ylabel "Seconds"

set size 1.0, 1.0

# set the grid on
set grid x,y

# set the key, options are top/bottom and left/right
set key top left

# indicates the ranges
set yrange [0:] 
set xrange [1:] 

plot "{matrix_size}_{cores}.aggregated" u ($1):($5) with lines linestyle 1 title "total time", \
     "{matrix_size}_{cores}.aggregated" u ($1):($2) with lines linestyle 2 title "init time", \
     "{matrix_size}_{cores}.aggregated" u ($1):($3) with lines linestyle 3 title "send time", \
     "{matrix_size}_{cores}.aggregated" u ($1):($4) with lines linestyle 4 title "compute time", \
     "{matrix_size}_{cores}.aggregated" u ($1):($6) with lines linestyle 5 title "worker wait avg.", \
     "{matrix_size}_{cores}.aggregated" u ($1):($7) with lines linestyle 6 title "worker compute avg.", \
"""

table_template = """\\begin{table}[]
\centering
\\begin{tabular}{|l|l|l|l|l|l|l|l|l|l|}
\hline
Workers & Threads & Init (s) & Send (s) & Comp (s) & Total(s) & wwa (s) & wca (s) & Speedup & Efficiency \\\\ \hline
{lines}
\end{tabular}
\caption{N={matrix_size} {cores}}
\label{my-label}
\end{table}
"""

seq_table_template = """
\\begin{table}[]
\centering
\\begin{tabular}{|l|l|l|l|l|l|l|}
\hline
Matrix Size & Init & Send (s) & Comp (s) & Total(s) & Worker Waiting (s) & Worker Computing (s) \\\\ \hline
{lines}
\end{tabular}
\caption{Sequential Performance Summary}
\label{seq-table}
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
            worker_waiting_avg_sum = float(0)
            worker_computing_avg_sum = float(0)
            for line in csv.reader(values, delimiter="\t"):
                if len(line) > 1:
                    count += 1
                    matrix_size = int(line[0])
                    blocks_a = line[1]
                    blocks_b = line[2]
                    workers = int(blocks_a) * int(blocks_b)
                    init_time_sum += float(line[3])
                    send_time_sum += float(line[4])
                    compute_time_sum += float(line[5])
                    cores = int(line[6])

                    worker_waiting_sum = 0
                    worker_computing_sum = 0
                    for worker in range(0, workers):
                        worker_waiting_sum += float(line[7 + (worker * 3)])
                        worker_computing_sum += float(line[8 + (worker * 3)])

                    worker_waiting_avg_sum += worker_waiting_sum / workers
                    worker_computing_avg_sum += worker_computing_sum / workers


                    entry = (init_time_sum / count, 
                        send_time_sum / count, 
                        compute_time_sum / count, 
                        worker_waiting_avg_sum / count, 
                        worker_computing_avg_sum / count)

                    result[(matrix_size, cores, workers)] = entry

                    # add the sequential measurement in both cases (4 cores per machine and 1 core per machine)
                    if workers == 1 and cores == 1:
                      result[(matrix_size, 4, workers)] = entry


sorted_result = collections.OrderedDict(sorted(result.items()))
speedup_table_cores = collections.defaultdict(list)
speedup_table_no_cores = collections.defaultdict(list)

efficiency_table_cores = collections.defaultdict(list)
efficiency_table_no_cores = collections.defaultdict(list)



seq_plot_file = "seq.aggregated"
seq_table_lines = ""
with open(seq_plot_file, "w") as seqplotfile:
    seq_table_file = "seq.table"
    for matrix_size, cores_and_workers in itertools.groupby(sorted_result, lambda x: x[0]):
        for cores, worker in itertools.groupby(cores_and_workers, lambda x: x[1]):
            plot_file = "{}_{}.aggregated".format(matrix_size, cores)
            table_file= "{}_{}.table".format(matrix_size, cores)
            print("Generating for N={} and Cores={}".format(matrix_size, cores))
            with open(plot_file, "w") as plotfile:
                with open(table_file, "w") as tablefile:
                    table = table_template.replace("{matrix_size}", str(matrix_size))
                    cores_string = ""
                    if cores == 1:
                      cores_string = "(16 Machines x 1 Core)"
                    elif cores == 4:
                      cores_string = "(4 Machines x 4 Cores)"
                    table = table.replace("{cores}", cores_string)
                    table = table.replace("my-label", "{}-{}".format(matrix_size, cores))
                    count = 0
                    table_lines = ""
                    sequential_time = 0
                    for item in worker:
                        table_line_template = "{workers} & {cores} & {init} & {send} & {comp} & {total} & {avg_worker_waiting} & {avg_worker_computing} & {speedup} & {efficiency} \\\\ \hline"
                        size, cores, workers = item
                        count += 1
                        times = result[item]
                        init_time = times[0]
                        send_time = times[1]
                        compute_time = times[2]
                        avg_worker_waiting_time = times[3]
                        avg_worker_computing_time = times[4]

                        total_time = init_time + send_time + compute_time

                        if cores == 1:
                            number_of_cores = workers
                        elif cores == 4:
                            number_of_cores = workers * 4

                        if workers == 1:
                            sequential_time = total_time
                            number_of_cores = 1

                        speedup = float(sequential_time) / total_time
                        efficiency = speedup / number_of_cores

                        if cores == 1:
                            speedup_table_no_cores[workers].append(str(speedup))
                            efficiency_table_no_cores[workers].append(str(efficiency))
                        elif cores == 4:
                            speedup_table_cores[workers].append(str(speedup))
                            efficiency_table_cores[workers].append(str(efficiency))

                        plotfile.write("{} {} {} {} {} {} {}\n".format(workers, init_time, send_time, compute_time, total_time, avg_worker_waiting_time, avg_worker_computing_time))

                        if cores == 1 and workers == 1:
                            to_write = "{} {} {} {} {} {} {}\n".format(matrix_size, init_time, send_time, compute_time, total_time, avg_worker_waiting_time, avg_worker_computing_time)
                            print to_write
                            seqplotfile.write(to_write)
                            seq_table_line_template = "{matrix_size} & {init} & {send} & {comp} & {total} & {avg_worker_waiting} & {avg_worker_computing} \\\\ \hline"
                            seq_table_line = seq_table_line_template.replace("{init}", "{0:.3f}".format(init_time))
                            seq_table_line = seq_table_line.replace("{matrix_size}", "{}".format(matrix_size))
                            seq_table_line = seq_table_line.replace("{send}", "{0:.3f}".format(send_time))
                            seq_table_line = seq_table_line.replace("{comp}", "{0:.3f}".format(compute_time))
                            seq_table_line = seq_table_line.replace("{total}", "{0:.3f}".format(total_time))
                            seq_table_line = seq_table_line.replace("{avg_worker_waiting}", "{0:.3f}".format(avg_worker_waiting_time))
                            seq_table_line = seq_table_line.replace("{avg_worker_computing}", "{0:.3f}".format(avg_worker_computing_time))
                            seq_table_lines += seq_table_line
                            seq_table_lines += "\n"


                        table_line = table_line_template.replace("{workers}", str(workers))
                        table_line = table_line.replace("{cores}", "{0}".format(number_of_cores))
                        table_line = table_line.replace("{init}", "{0:.3f}".format(init_time))
                        table_line = table_line.replace("{send}", "{0:.3f}".format(send_time))
                        table_line = table_line.replace("{comp}", "{0:.3f}".format(compute_time))
                        table_line = table_line.replace("{total}", "{0:.3f}".format(total_time))
                        table_line = table_line.replace("{avg_worker_waiting}", "{0:.3f}".format(avg_worker_waiting_time))
                        table_line = table_line.replace("{avg_worker_computing}", "{0:.3f}".format(avg_worker_computing_time))
                        table_line = table_line.replace("{speedup}", "{0:.3f}".format(speedup))
                        table_line = table_line.replace("{efficiency}", "{0:.3f}".format(efficiency))

                        table_lines += table_line
                        table_lines += "\n"
                    table = table.replace("{lines}", table_lines)
                    tablefile.write(table)

            plot_file = "{}_{}.gp".format(matrix_size, cores)
            with open(plot_file, "w") as plotfile:
                plot = plot_template.replace("{matrix_size}", str(matrix_size))
                if cores == 4:
                    plot = plot.replace("{x-axis-label}", "Workers (4 Threads per Worker)")
                    plot = plot.replace("{cores_label}", "(4 Cores per Machine)")
                elif cores == 1:
                    plot = plot.replace("{x-axis-label}", "Workers (1 Thread per Worker)")
                    plot = plot.replace("{cores_label}", "(1 Core per Machine)")

                plot = plot.replace("{cores}", str(cores))
                plotfile.write(plot)
            os.system("gnuplot {}".format(plot_file))

with open(seq_table_file, "w") as seqtablefile:
    seq_table = seq_table_template.replace("{lines}", seq_table_lines)
    seqtablefile.write(seq_table)

os.system("gnuplot seq_plot.gp")


speedup_no_cores_data_file = "speedup_no_cores.aggregated"
with open(speedup_no_cores_data_file, "w") as speedupfile:
    for workers in speedup_table_no_cores:
        speedupfile.write(str(workers) + " ")
        speedupfile.write(" ".join(speedup_table_no_cores[workers]))
        speedupfile.write("\n")
os.system("gnuplot {}".format("speedup_no_cores.gp"))

speedup_cores_data_file = "speedup_cores.aggregated"
with open(speedup_cores_data_file, "w") as speedupfile:
    for workers in speedup_table_cores:
        speedupfile.write(str(workers) + " ")
        speedupfile.write(" ".join(speedup_table_cores[workers]))
        speedupfile.write("\n")
os.system("gnuplot {}".format("speedup_cores.gp"))

efficiency_no_cores_data_file = "efficiency_no_cores.aggregated"
with open(efficiency_no_cores_data_file, "w") as efficiencyfile:
    for workers in efficiency_table_no_cores:
        efficiencyfile.write(str(workers) + " ")
        efficiencyfile.write(" ".join(efficiency_table_no_cores[workers]))
        efficiencyfile.write("\n")
os.system("gnuplot {}".format("efficiency_no_cores.gp"))

efficiency_cores_data_file = "efficiency_cores.aggregated"
with open(efficiency_cores_data_file, "w") as efficiencyfile:
    for workers in efficiency_table_cores:
        efficiencyfile.write(str(workers) + " ")
        efficiencyfile.write(" ".join(efficiency_table_cores[workers]))
        efficiencyfile.write("\n")
os.system("gnuplot {}".format("efficiency_cores.gp"))

os.system("pdflatex report.tex")
