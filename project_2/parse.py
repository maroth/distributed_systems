import csv
from collections import defaultdict
from itertools import groupby



result = defaultdict(list)
with open("torus.txt", "r") as f:
    reader = csv.reader(f, delimiter="\t")
    for key, group in groupby(reader, key=lambda x: "{}:{}".format(x[0], x[1])):
        count = 0
        size = 0
        w = 0
        distSum = 0
        ackSum = 0
        stopSum = 0
        compTimeSum = 0
        totalTimeSum = 0
        for entry in group:
            count += 1
            size = entry[0]
            w = entry[1]
            distSum += int(entry[2])
            ackSum += int(entry[3])
            stopSum += int(entry[4])
            compTimeSum += float(entry[6])
            totalTimeSum += float(entry[6])
        distSum /= count
        ackSum /= count
        compTimeSum /= count
        totalTimeSum /= count
        out = "{} {} {} {} {} {} {}".format(size, w, distSum, ackSum, stopSum, compTimeSum, totalTimeSum)
        result[w].append(out)

for w, outs in result.iteritems():
    with open("torus_{}.aggregated".format(w), "w") as f:
        for out in outs:
            f.write(out)
            f.write("\n")
        

